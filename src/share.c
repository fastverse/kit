/*
 * kit : Useful R Functions Implemented in C
 * Copyright (C) 2020-2025  Morgan Jacob
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "kit.h"
#include <string.h>
#include <stddef.h> // offsetof for the layout assertions below

/*
 *  Metadata stored at offset 0 of the SHM_META_SIZE-byte length segment.
 *  `len` stays first so segments written by older versions (which stored
 *  only `len`) still read correctly; their `gen` bytes are zero (even = ready).
 *
 *  Single-writer (owner) / multi-reader seqlock: odd `gen` means a write is
 *  in progress. Readers validate the header before AND after the payload
 *  copy, so misuse fails loudly instead of silently corrupting. No mutexes,
 *  no dependencies.
 *
 *  Contracts (also stated in ?shareData, enforced where cheap):
 *  - exactly one writer publishes a name at a time; a second writer seeing
 *    an odd generation fails loudly instead of interleaving payloads;
 *  - readers may run concurrently with one writer and with each other;
 *  - resizing a live name requires quiesced readers (same-size re-share is
 *    always safe); the owner handle must outlive all readers.
 *  Correct use needs no lock: share() returns only after the payload is
 *  published, so readers starting afterwards never see odd `gen`.
 *
 *  `gen` is 32-bit so header loads/stores are single-copy-atomic even on
 *  32-bit targets (mmap is page-aligned, hence naturally aligned).
 */

#define SHM_META_SIZE 256

struct SHM_META {
  size_t len;
  uint32_t gen;
};

// Layout guarantees: header fits the mapping, len stays first for back-compat.
typedef char shm_meta_size_check[(sizeof(struct SHM_META) <= SHM_META_SIZE) ? 1 : -1];
typedef char shm_meta_len_first_check[(offsetof(struct SHM_META, len) == 0) ? 1 : -1];

// Ordered header access. Every R toolchain (GCC, Clang, mingw) provides
// __atomic builtins; anything else falls back to plain accesses (still
// correct on strongly-ordered CPUs; the generation check fails loudly
// otherwise). The payload copy itself is never trusted, only validated.
#if defined(__GNUC__) || defined(__clang__)
static uint32_t shm_load_gen_acquire(const uint32_t *p) {
  return __atomic_load_n(p, __ATOMIC_ACQUIRE);
}
static void shm_store_gen_release(uint32_t *p, uint32_t v) {
  __atomic_store_n(p, v, __ATOMIC_RELEASE);
}
static size_t shm_load_len_acquire(const size_t *p) {
  return __atomic_load_n(p, __ATOMIC_ACQUIRE);
}
static void shm_store_len_release(size_t *p, size_t v) {
  __atomic_store_n(p, v, __ATOMIC_RELEASE);
}
#else
#warning "ordered shared-memory access unavailable: seqlock ordering best-effort only"
static uint32_t shm_load_gen_acquire(const uint32_t *p) { return *p; }
static void shm_store_gen_release(uint32_t *p, uint32_t v) { *p = v; }
static size_t shm_load_len_acquire(const size_t *p) { return *p; }
static void shm_store_len_release(size_t *p, size_t v) { *p = v; }
#endif

// strdup without feature-test-macro dependence (strict-ISO safe).
static char *shm_dup_string(const char *s) {
  size_t n = strlen(s) + 1;
  char *p = (char *) malloc(n);
  if (p != NULL) memcpy(p, s, n);
  return p;
}

/*
 *  Structure to hold Length and Address 
 *  of data to be shared in memory segment
 */

struct OBJECT {
#ifdef WIN32
  HANDLE hMapFile;
  HANDLE hMapLength;
  LPCTSTR lpMapAddress;
  LPCTSTR lpMapLength;
#else
  int fd_addr;
  int fd_length;
  size_t STORAGE_SIZE;
  void *addr;
  void *length;
  char *STORAGE_ID;
  char *LENGTH_ID;
#endif
};

/*
 *  Function to finalize memory map pointer
 */

static bool verbose_finalizer = false;

static void map_finalizer (SEXP ext) {
  if (verbose_finalizer) Rprintf("* Finalize...\n");
  if (NULL == R_ExternalPtrAddr(ext)) {
    return;
  }
  if (verbose_finalizer) Rprintf("* Clear external pointer...\n");
  struct OBJECT *ptr = (struct OBJECT*) R_ExternalPtrAddr(ext);
#ifdef WIN32
  if (ptr->lpMapAddress != NULL) UnmapViewOfFile(ptr->lpMapAddress);
  if (ptr->hMapFile != NULL && ptr->hMapFile != INVALID_HANDLE_VALUE) CloseHandle(ptr->hMapFile);
  if (ptr->lpMapLength != NULL) UnmapViewOfFile(ptr->lpMapLength);
  if (ptr->hMapLength != NULL && ptr->hMapLength != INVALID_HANDLE_VALUE) CloseHandle(ptr->hMapLength);
#else
  // fds are normally already closed (set to -1); close defensively in case a
  // future path registers the finalizer while still holding descriptors.
  if (ptr->fd_addr >= 0) close(ptr->fd_addr);
  if (ptr->fd_length >= 0) close(ptr->fd_length);
  if (ptr->addr != NULL && ptr->addr != MAP_FAILED && ptr->STORAGE_SIZE > 0) {
    munmap(ptr->addr, ptr->STORAGE_SIZE);
  }
  if (ptr->STORAGE_ID != NULL) {
    shm_unlink(ptr->STORAGE_ID);
    free(ptr->STORAGE_ID);
  }
  if (ptr->length != NULL && ptr->length != MAP_FAILED) {
    munmap(ptr->length, SHM_META_SIZE);
  }
  if (ptr->LENGTH_ID != NULL) {
    shm_unlink(ptr->LENGTH_ID);
    free(ptr->LENGTH_ID);
  }
#endif
  R_ClearExternalPtr(ext); // disarm first so GC re-entry is a safe no-op
  R_Free(ptr);
  if (verbose_finalizer) Rprintf("* Clear external pointer...OK\n");
}

/*
 *  Function to create data
 */

SEXP createMappingObjectR (SEXP MapObjectName, SEXP MapLengthName, SEXP DataObject, SEXP verboseArg) {
  if (TYPEOF(MapObjectName) != STRSXP || LENGTH(MapObjectName) != 1) {
    error("Argument 'MapObjectName' must be of type character and length 1.");
  }
  if (TYPEOF(MapLengthName) != STRSXP || LENGTH(MapLengthName) != 1) {
    error("Argument 'MapLengthName' must be of type character and length 1.");
  }
  if (TYPEOF(DataObject) != RAWSXP) {
    error("Argument 'DataObject' must be a raw vector.");
  }
  if (!IS_BOOL(verboseArg)) {
    error("Argument 'verbose' must be TRUE or FALSE.");
  }
  const bool verbose = asLogical(verboseArg);
  verbose_finalizer = verbose;
  const R_xlen_t xlen = XLENGTH(DataObject);
  if (xlen < 0 || (uint64_t) xlen > (uint64_t) SIZE_MAX) {
    error("* Data object is too large...ERROR");
  }
  const size_t len = (size_t) xlen;
  const size_t BUF_SIZE = len*sizeof(Rbyte);
  if (verbose) Rprintf("* Data object size: %zu\n",len*sizeof(Rbyte));
  if (verbose) Rprintf("* Start mapping object...OK\n");
  struct OBJECT *foo = R_Calloc(1, struct OBJECT);
#ifdef WIN32
  foo->hMapFile = NULL;
  foo->hMapLength = NULL;
  foo->lpMapAddress = NULL;
  foo->lpMapLength = NULL;
#else
  foo->fd_addr = -1;
  foo->fd_length = -1;
  foo->addr = NULL;
  foo->length = NULL;
  foo->STORAGE_ID = NULL;
  foo->LENGTH_ID = NULL;
  foo->STORAGE_SIZE = BUF_SIZE;
#endif
  SEXP ext = PROTECT(R_MakeExternalPtr(foo, R_NilValue, R_NilValue));
#ifdef WIN32
  LPSTR pMN = (LPSTR) CHAR(STRING_PTR_RO(MapObjectName)[0]);
  LPSTR pML = (LPSTR) CHAR(STRING_PTR_RO(MapLengthName)[0]);
  if (BUF_SIZE == 0) {
    R_ClearExternalPtr(ext);
    R_Free(foo);
    UNPROTECT(1);
    error("* Data object is empty...ERROR");
  }
  foo->hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
                                       (DWORD) (BUF_SIZE >> 32), (DWORD) (BUF_SIZE & 0xFFFFFFFFu), pMN);
  foo->hMapLength = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, SHM_META_SIZE, pML);
  if (foo->hMapFile == NULL || foo->hMapFile == INVALID_HANDLE_VALUE ||
      foo->hMapLength == NULL || foo->hMapLength == INVALID_HANDLE_VALUE) {
    if (foo->hMapFile != NULL && foo->hMapFile != INVALID_HANDLE_VALUE) CloseHandle(foo->hMapFile);
    if (foo->hMapLength != NULL && foo->hMapLength != INVALID_HANDLE_VALUE) CloseHandle(foo->hMapLength);
    R_ClearExternalPtr(ext);
    R_Free(foo);
    UNPROTECT(1);
    error("* Creating file mapping...ERROR");
  }
  if (verbose) Rprintf("* Creating file maping...OK\n");
  foo->lpMapAddress = (LPCTSTR) MapViewOfFile (foo->hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, BUF_SIZE);
  foo->lpMapLength = (LPCTSTR) MapViewOfFile (foo->hMapLength, FILE_MAP_ALL_ACCESS, 0, 0, SHM_META_SIZE);
  if (foo->lpMapAddress == NULL || foo->lpMapLength == NULL) {
    if (foo->lpMapAddress != NULL) UnmapViewOfFile(foo->lpMapAddress);
    if (foo->lpMapLength != NULL) UnmapViewOfFile(foo->lpMapLength);
    CloseHandle(foo->hMapFile);
    CloseHandle(foo->hMapLength);
    R_ClearExternalPtr(ext);
    R_Free(foo);
    UNPROTECT(1);
    error("* Map view file...ERROR");
  }
  if (verbose) Rprintf("* Map view file...OK\n");
  // Seqlock publish: mark writing, copy payload, then publish size + ready.
  // Exactly one writer per name: an odd generation means another writer is
  // mid-publish (or a previous one crashed) — fail loudly and recover with
  // clearShared() instead of interleaving two payloads silently.
  struct SHM_META *metaW = (struct SHM_META *) (void *) foo->lpMapLength;
  uint32_t gW = shm_load_gen_acquire(&metaW->gen);
  if (gW & 1u) {
    UnmapViewOfFile(foo->lpMapAddress);
    UnmapViewOfFile(foo->lpMapLength);
    CloseHandle(foo->hMapFile);
    CloseHandle(foo->hMapLength);
    R_ClearExternalPtr(ext);
    R_Free(foo);
    UNPROTECT(1);
    error("* Shared name is busy (concurrent writer?) — clearShared() to recover...ERROR");
  }
  shm_store_gen_release(&metaW->gen, gW + 1u); // odd: write in progress
  CopyMemory((LPVOID)foo->lpMapAddress, RAW(DataObject), BUF_SIZE);
  shm_store_len_release(&metaW->len, len);
  shm_store_gen_release(&metaW->gen, gW + 2u); // even: ready
#else
  const char *pMN = CHAR(STRING_PTR_RO(MapObjectName)[0]);
  const char *pML = CHAR(STRING_PTR_RO(MapLengthName)[0]);
  foo->STORAGE_ID = shm_dup_string(pMN);
  foo->LENGTH_ID = shm_dup_string(pML);
  if (foo->STORAGE_ID == NULL || foo->LENGTH_ID == NULL) {
    if (foo->STORAGE_ID != NULL) free(foo->STORAGE_ID);
    if (foo->LENGTH_ID != NULL) free(foo->LENGTH_ID);
    R_ClearExternalPtr(ext);
    R_Free(foo);
    UNPROTECT(1);
    error("* Duplicating shared memory names...ERROR");
  }
  if (BUF_SIZE == 0) {
    free(foo->STORAGE_ID);
    free(foo->LENGTH_ID);
    R_ClearExternalPtr(ext);
    R_Free(foo);
    UNPROTECT(1);
    error("* Data object is empty...ERROR");
  }
  foo->fd_addr = shm_open(foo->STORAGE_ID, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
  foo->fd_length = shm_open(foo->LENGTH_ID, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
  if (foo->fd_addr == -1 || foo->fd_length == -1) {
    int open_errno = errno;
    Rprintf("shm_open error, errno(%d): %s\n", open_errno, strerror(open_errno));
    if (foo->fd_addr != -1) close(foo->fd_addr);
    if (foo->fd_length != -1) close(foo->fd_length);
    // No unlink here: pre-existing names may belong to a live owner.
    free(foo->STORAGE_ID);
    free(foo->LENGTH_ID);
    R_ClearExternalPtr(ext);
    R_Free(foo);
    UNPROTECT(1);
    error("* Creating file mapping...ERROR");
  }
  if (verbose) Rprintf("* Creating file maping...OK\n");
  // Size the objects. Same-size re-share reuses them (payload + seqlock
  // generation are simply republished). A size change recreates them: on some
  // platforms (e.g. macOS) ftruncate fails on fds from re-opening an existing
  // object, so the stale objects are unlinked and fresh (truncatable) ones are
  // created. In-flight readers of the old objects keep reading the old
  // payload; only new opens see the new one.
  //
  // Orphan discipline: only objects this call effectively created (empty/new)
  // are unlinked again on later failures; pre-existing live objects are never
  // unlinked by us. Recover leftovers with clearShared().
  struct stat st_addr, st_len;
  if (fstat(foo->fd_addr, &st_addr) == -1 || fstat(foo->fd_length, &st_len) == -1) {
    int st_errno = errno;
    Rprintf("stat error, errno(%d): %s\n", st_errno, strerror(st_errno));
    close(foo->fd_addr);
    close(foo->fd_length);
    free(foo->STORAGE_ID);
    free(foo->LENGTH_ID);
    R_ClearExternalPtr(ext);
    R_Free(foo);
    UNPROTECT(1);
    error("* Stat shared memory object...ERROR");
  }
  bool fresh_addr = (st_addr.st_size == 0);
  bool fresh_len = (st_len.st_size == 0);
  if (st_addr.st_size < 0 || (size_t) st_addr.st_size != BUF_SIZE ||
      st_len.st_size < 0 || (size_t) st_len.st_size != SHM_META_SIZE) {
    close(foo->fd_addr);
    close(foo->fd_length);
    shm_unlink(foo->STORAGE_ID);
    shm_unlink(foo->LENGTH_ID);
    foo->fd_addr = shm_open(foo->STORAGE_ID, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    foo->fd_length = shm_open(foo->LENGTH_ID, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (foo->fd_addr == -1 || foo->fd_length == -1) {
      int reopen_errno = errno;
      Rprintf("shm_open error, errno(%d): %s\n", reopen_errno, strerror(reopen_errno));
      // Both names were just unlinked by us, so anything opened here is ours.
      if (foo->fd_addr != -1) { close(foo->fd_addr); shm_unlink(foo->STORAGE_ID); }
      if (foo->fd_length != -1) { close(foo->fd_length); shm_unlink(foo->LENGTH_ID); }
      free(foo->STORAGE_ID);
      free(foo->LENGTH_ID);
      R_ClearExternalPtr(ext);
      R_Free(foo);
      UNPROTECT(1);
      error("* Recreating file mapping...ERROR");
    }
    // Recreated objects are ours: unlink them again if sizing fails below.
    fresh_addr = true;
    fresh_len = true;
    if (ftruncate(foo->fd_addr, BUF_SIZE) == -1 || ftruncate(foo->fd_length, SHM_META_SIZE) == -1) {
      int ft_errno = errno;
      Rprintf("ftruncate error, errno(%d): %s\n", ft_errno, strerror(ft_errno));
      close(foo->fd_addr);
      close(foo->fd_length);
      shm_unlink(foo->STORAGE_ID);
      shm_unlink(foo->LENGTH_ID);
      free(foo->STORAGE_ID);
      free(foo->LENGTH_ID);
      R_ClearExternalPtr(ext);
      R_Free(foo);
      UNPROTECT(1);
      error("* Extend shared memory object...ERROR");
    }
  }
  if (verbose) Rprintf("* Extend shared memory object...OK\n");
  foo->addr = mmap(NULL, BUF_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, foo->fd_addr, 0);
  foo->length = mmap(NULL, SHM_META_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, foo->fd_length, 0);
  if (foo->addr == MAP_FAILED || foo->length == MAP_FAILED) {
    int mm_errno = errno;
    Rprintf("mmap error, errno(%d): %s\n", mm_errno, strerror(mm_errno));
    if (foo->addr != MAP_FAILED && foo->addr != NULL) munmap(foo->addr, BUF_SIZE);
    if (foo->length != MAP_FAILED && foo->length != NULL) munmap(foo->length, SHM_META_SIZE);
    close(foo->fd_addr);
    close(foo->fd_length);
    if (fresh_addr) shm_unlink(foo->STORAGE_ID);
    if (fresh_len) shm_unlink(foo->LENGTH_ID);
    foo->addr = NULL;
    foo->length = NULL;
    free(foo->STORAGE_ID);
    free(foo->LENGTH_ID);
    R_ClearExternalPtr(ext);
    R_Free(foo);
    UNPROTECT(1);
    error("* Map view file...ERROR");
  }
  if (verbose) Rprintf("* Map view file...OK\n");
  int cerr_addr = close(foo->fd_addr);
  int cerr_len = close(foo->fd_length);
  foo->fd_addr = -1;
  foo->fd_length = -1;
  if (cerr_addr == -1 || cerr_len == -1) {
    int cl_errno = errno;
    Rprintf("close error, errno(%d): %s\n", cl_errno, strerror(cl_errno));
    munmap(foo->addr, BUF_SIZE);
    munmap(foo->length, SHM_META_SIZE);
    if (fresh_addr) shm_unlink(foo->STORAGE_ID);
    if (fresh_len) shm_unlink(foo->LENGTH_ID);
    free(foo->STORAGE_ID);
    free(foo->LENGTH_ID);
    R_ClearExternalPtr(ext);
    R_Free(foo);
    UNPROTECT(1);
    error("* Closing file descriptors...ERROR");
  }
  // Seqlock publish: mark writing, copy payload, then publish size + ready.
  // Exactly one writer per name: an odd generation means another writer is
  // mid-publish (or a previous one crashed) — fail loudly and recover with
  // clearShared() instead of interleaving two payloads silently. No unlink
  // here: the names may belong to that live writer.
  struct SHM_META *meta = (struct SHM_META *) foo->length;
  uint32_t g = shm_load_gen_acquire(&meta->gen);
  if (g & 1u) {
    munmap(foo->addr, BUF_SIZE);
    munmap(foo->length, SHM_META_SIZE);
    free(foo->STORAGE_ID);
    free(foo->LENGTH_ID);
    R_ClearExternalPtr(ext);
    R_Free(foo);
    UNPROTECT(1);
    error("* Shared name is busy (concurrent writer?) — clearShared() to recover...ERROR");
  }
  shm_store_gen_release(&meta->gen, g + 1u); // odd: write in progress
  memcpy(foo->addr, RAW(DataObject), BUF_SIZE);
  shm_store_len_release(&meta->len, len);
  shm_store_gen_release(&meta->gen, g + 2u); // even: ready
#endif
  if (verbose) Rprintf("* Copy memory...OK\n");
  R_RegisterCFinalizerEx(ext, map_finalizer, TRUE);
  if (verbose) Rprintf("* Register finalizer...OK\n");
  UNPROTECT(1);
  return ext;
}

/*
 *  Function to retrieve data
 */

SEXP getMappingObjectR (SEXP MapObjectName, SEXP MapLengthName, SEXP verboseArg) {
  if (TYPEOF(MapObjectName) != STRSXP || LENGTH(MapObjectName) != 1) {
    error("Argument 'MapObjectName' must be of type character and length 1.");
  }
  if (TYPEOF(MapLengthName) != STRSXP || LENGTH(MapLengthName) != 1) {
    error("Argument 'MapLengthName' must be of type character and length 1.");
  }
  if (!IS_BOOL(verboseArg)) {
    error("Argument 'verbose' must be TRUE or FALSE.");
  }
  const bool verbose = asLogical(verboseArg);
#ifdef WIN32
  LPSTR pMN = (LPSTR) CHAR(STRING_PTR_RO(MapObjectName)[0]);
  LPSTR pML = (LPSTR) CHAR(STRING_PTR_RO(MapLengthName)[0]);
  HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, pMN);
  HANDLE hMapLength = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, pML);
  if (hMapFile == NULL || hMapFile == INVALID_HANDLE_VALUE ||
      hMapLength == NULL || hMapLength == INVALID_HANDLE_VALUE) {
    if (hMapFile != NULL && hMapFile != INVALID_HANDLE_VALUE) CloseHandle(hMapFile);
    if (hMapLength != NULL && hMapLength != INVALID_HANDLE_VALUE) CloseHandle(hMapLength);
#else
  // Open the header first, then the payload: with the writer's
  // unlink(data)-then-unlink(meta) / create(data)-then-create(meta) order,
  // every outcome is then a consistent (old,old)/(new,new) pair or a clean
  // ENOENT — a stale-payload/new-header mix is unreachable (see below).
  const char *pMN = CHAR(STRING_PTR_RO(MapObjectName)[0]);
  const char *pML = CHAR(STRING_PTR_RO(MapLengthName)[0]);
  int fd_length = shm_open(pML, O_RDONLY);
  int fd_addr = shm_open(pMN, O_RDONLY);
  if (fd_length == -1 || fd_addr == -1) {
    if (fd_length != -1) close(fd_length);
    if (fd_addr != -1) close(fd_addr);
#endif
    error("* Creating file mapping...ERROR");
  }
  if (verbose) Rprintf("* Creating file maping...OK\n");
  size_t len_a = 0;
#ifndef WIN32
  size_t map_len = 0;
#endif
  uint32_t gen_a = 0;
#ifdef WIN32
  LPCTSTR lpMapLength = (LPCTSTR) MapViewOfFile (hMapLength, FILE_MAP_ALL_ACCESS, 0, 0, SHM_META_SIZE);
  if (lpMapLength == NULL) {
    CloseHandle(hMapFile);
    CloseHandle(hMapLength);
#else
  // The length object must already hold a full header: a writer that crashed
  // between shm_open and ftruncate leaves size 0, whose mapping would SIGBUS
  // on dereference below.
  struct stat st_len0;
  if (fstat(fd_length, &st_len0) == -1 ||
      st_len0.st_size < (off_t) sizeof(struct SHM_META)) {
    close(fd_length);
    close(fd_addr);
    error("* Map view file (length)...ERROR");
  }
  void *length = mmap(NULL, SHM_META_SIZE, PROT_READ, MAP_SHARED, fd_length, 0);
  if (length == MAP_FAILED) {
    close(fd_length);
    close(fd_addr);
    error("* Map view file (length)...ERROR");
  }
#endif
  if (verbose) Rprintf("* Map view file (length)...OK\n");
  // Seqlock: observe the header before the payload copy. An odd generation
  // means a writer is mid-publish, so fail fast instead of reading torn data.
  // Generation is loaded first: the writer publishes len before the final
  // generation bump, so gen-first narrows the retry window.
  struct SHM_META *metaR = NULL;
#ifdef WIN32
  metaR = (struct SHM_META *) (void *) lpMapLength;
  gen_a = shm_load_gen_acquire(&metaR->gen);
  len_a = shm_load_len_acquire(&metaR->len);
  if (gen_a & 1u) {
    UnmapViewOfFile(lpMapLength);
    CloseHandle(hMapFile);
    CloseHandle(hMapLength);
#else
  metaR = (struct SHM_META *) length;
  gen_a = shm_load_gen_acquire(&metaR->gen);
  len_a = shm_load_len_acquire(&metaR->len);
  if (gen_a & 1u) {
    munmap(length, SHM_META_SIZE);
    close(fd_addr);
    close(fd_length);
#endif
    error("* Shared data is being written, please retry...ERROR");
  }
  if (verbose) Rprintf("* Generation check...OK\n");
#ifdef WIN32
  LPCTSTR lpMapAddress = NULL;
  if (len_a > 0) {
    lpMapAddress = (LPCTSTR) MapViewOfFile (hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, len_a*sizeof(Rbyte));
  }
  if (len_a == 0 || lpMapAddress == NULL) {
    UnmapViewOfFile(lpMapLength);
    CloseHandle(hMapFile);
    CloseHandle(hMapLength);
#else
  // Clamp the data mapping to the object size observed right now: if a writer
  // concurrently re-shares a smaller payload, mapping the stale (larger) size
  // could SIGBUS on access. The seqlock re-check after the copy turns any such
  // race into a clean error. Concurrent writers must use unique map names.
  map_len = len_a*sizeof(Rbyte);
  struct stat addrstat;
  if (len_a == 0 || fstat(fd_addr, &addrstat) == -1) {
    munmap(length, SHM_META_SIZE);
    close(fd_addr);
    close(fd_length);
    error("* Map view file (address)...ERROR");
  }
  if ((size_t) addrstat.st_size < map_len) map_len = (size_t) addrstat.st_size;
  // Fail fast when the clamp already proves a concurrent size change: this
  // also avoids attempting a huge allocation for a garbage/torn length.
  if (map_len != len_a*sizeof(Rbyte)) {
    munmap(length, SHM_META_SIZE);
    close(fd_addr);
    close(fd_length);
    error("* Shared data changed during read, please retry...ERROR");
  }
  // fd_length stays open: the header is re-read after the copy to validate.
  if (munmap(length, SHM_META_SIZE) == -1) {
    close(fd_addr);
    close(fd_length);
    error("* Closing mapping file (length)...ERROR");
  }
  if (verbose) Rprintf("* Closing mapping file (length)...OK\n");
  void *addr = NULL;
  if (map_len > 0) {
    addr = mmap(NULL, map_len, PROT_READ, MAP_SHARED, fd_addr, 0);
  }
  if (map_len == 0 || addr == MAP_FAILED) {
    close(fd_addr);
    close(fd_length);
#endif
    error("* Map view file (address)...ERROR");
  }
  if (verbose) Rprintf("* Map view file (address)...OK\n");
  // The data mapping is established; its fd can go. fd_length is still needed
  // for the post-copy header revalidation below.
#ifdef WIN32
#else
  if (close(fd_addr) == -1) {
    munmap(addr, map_len);
    close(fd_length);
    error("* Closing file descriptor (address)...ERROR");
  }
#endif
  SEXP ans = PROTECT(allocVector(RAWSXP, len_a));
  if (verbose) Rprintf("* Create RAW Vector...OK\n");
#ifdef WIN32
  CopyMemory(RAW(ans), (Rbyte*)lpMapAddress, len_a*sizeof(Rbyte));
#else
  memcpy(RAW(ans), (Rbyte*)addr, map_len);
#endif
  if (verbose) Rprintf("* Copy map memory...OK\n");

#ifdef WIN32
  // Seqlock revalidation: the header must be unchanged since the pre-copy
  // read. Acquire loads are ordered after the payload copy above.
  uint32_t gen_b = shm_load_gen_acquire(&metaR->gen);
  size_t len_b = shm_load_len_acquire(&metaR->len);
  bool torn_win = ((gen_a != gen_b) || (gen_b & 1u) || (len_a != len_b));
  if (!UnmapViewOfFile(lpMapLength)) {
    UnmapViewOfFile(lpMapAddress);
    CloseHandle(hMapFile);
    CloseHandle(hMapLength);
    UNPROTECT(1);
    error("* Closing mapping file (length)...ERROR");
  }
  if (verbose) Rprintf("* Closing mapping file (length)...OK\n");
  if (!CloseHandle(hMapLength)) {
    UnmapViewOfFile(lpMapAddress);
    CloseHandle(hMapFile);
    UNPROTECT(1);
    error("* Closing mapping handle (length)...ERROR");
  }
  if (verbose) Rprintf("* Closing mapping handle (length)...OK\n");
  if (torn_win) {
    UnmapViewOfFile(lpMapAddress);
    CloseHandle(hMapFile);
    UNPROTECT(1);
    error("* Shared data changed during read, please retry...ERROR");
  }

  if (!UnmapViewOfFile(lpMapAddress)) {
    CloseHandle(hMapFile);
    UNPROTECT(1);
    error("* Closing mapping file (address)...ERROR");
  }
  if (verbose) Rprintf("* Closing mapping file (address)...OK\n");
  if (!CloseHandle(hMapFile)) {
    UNPROTECT(1);
    error("* Closing mapping handle (address)...ERROR");
  }
  if (verbose) Rprintf("* Closing mapping handle (address)...OK\n");
#else
  // Non-destructive read: unmap + close only. Lifetime stays with the owner
  // handle from shareData(); use clearData() to shm_unlink().
  if (munmap(addr, map_len) == -1) {
    close(fd_length);
    UNPROTECT(1);
    error("* Closing mapping file (address)...ERROR");
  }
  if (verbose) Rprintf("* Closing mapping file (address)...OK\n");
  // Seqlock revalidation against the pre-copy header.
  void *length2 = mmap(NULL, SHM_META_SIZE, PROT_READ, MAP_SHARED, fd_length, 0);
  if (length2 == MAP_FAILED) {
    close(fd_length);
    UNPROTECT(1);
    error("* Map view file (length)...ERROR");
  }
  struct SHM_META *metaR2 = (struct SHM_META *) length2;
  uint32_t gen_b = shm_load_gen_acquire(&metaR2->gen);
  size_t len_b = shm_load_len_acquire(&metaR2->len);
  bool torn_posix = ((gen_a != gen_b) || (gen_b & 1u) || (len_a != len_b) ||
                     (map_len != len_a*sizeof(Rbyte)));
  if (munmap(length2, SHM_META_SIZE) == -1) {
    close(fd_length);
    UNPROTECT(1);
    error("* Closing mapping file (length)...ERROR");
  }
  if (close(fd_length) == -1) {
    UNPROTECT(1);
    error("* Closing mapping handle (length)...ERROR");
  }
  if (verbose) Rprintf("* Closing mapping handle (length)...OK\n");
  if (torn_posix) {
    UNPROTECT(1);
    error("* Shared data changed during read, please retry...ERROR");
  }
#endif
  UNPROTECT(1);
  return ans;
}

/*
 *  Function to unlink a mapping by name (orphan recovery without owner handle)
 */

SEXP unlinkMappingObjectR (SEXP MapObjectName, SEXP MapLengthName, SEXP verboseArg) {
  if (TYPEOF(MapObjectName) != STRSXP || LENGTH(MapObjectName) != 1) {
    error("Argument 'MapObjectName' must be of type character and length 1.");
  }
  if (TYPEOF(MapLengthName) != STRSXP || LENGTH(MapLengthName) != 1) {
    error("Argument 'MapLengthName' must be of type character and length 1.");
  }
  if (!IS_BOOL(verboseArg)) {
    error("Argument 'verbose' must be TRUE or FALSE.");
  }
  const bool verbose = asLogical(verboseArg);
#ifdef WIN32
  // Named file mappings vanish with the last open handle; there is no name
  // to unlink. Probe only, so the call stays harmless cross-platform.
  LPSTR pMN = (LPSTR) CHAR(STRING_PTR_RO(MapObjectName)[0]);
  LPSTR pML = (LPSTR) CHAR(STRING_PTR_RO(MapLengthName)[0]);
  HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, pMN);
  HANDLE hMapLength = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, pML);
  if (hMapFile != NULL && hMapFile != INVALID_HANDLE_VALUE) CloseHandle(hMapFile);
  if (hMapLength != NULL && hMapLength != INVALID_HANDLE_VALUE) CloseHandle(hMapLength);
  if (verbose) Rprintf("* Unlink by name is a no-op on Windows...OK\n");
  return ScalarLogical(FALSE);
#else
  const char *pMN = CHAR(STRING_PTR_RO(MapObjectName)[0]);
  const char *pML = CHAR(STRING_PTR_RO(MapLengthName)[0]);
  bool unlinked = false;
  if (shm_unlink(pMN) == 0) {
    unlinked = true;
  } else if (errno != ENOENT) {
    error("* Unlink shared data...ERROR");
  }
  if (shm_unlink(pML) == 0) {
    unlinked = true;
  } else if (errno != ENOENT) {
    error("* Unlink shared data length...ERROR");
  }
  if (verbose) Rprintf("* Unlink by name (removed=%d)...OK\n", unlinked ? 1 : 0);
  return unlinked ? ScalarLogical(TRUE) : ScalarLogical(FALSE);
#endif
}

/*
 *  Function to clear mapping object
 */

SEXP clearMappingObjectR  (SEXP ext, SEXP verboseArg) {
  if (TYPEOF(ext) != EXTPTRSXP) {
    error("Argument 'x' must be an external pointer like the one returned by shareData().");
  }
  if (!IS_BOOL(verboseArg)) {
    error("Argument 'verbose' must be TRUE or FALSE.");
  }
  verbose_finalizer = asLogical(verboseArg);
  if (NULL == R_ExternalPtrAddr(ext)) {
    return ScalarLogical(FALSE);
  }
  map_finalizer(ext);
  return ScalarLogical(TRUE);
}
