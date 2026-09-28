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
#include <time.h>

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
  bool own_addr;
  bool own_length;
  uint64_t token;
  bool have_token;
#endif
  bool verbose;
};

static SEXP mapping_tag = NULL;

static SEXP getMappingTag(void) {
  if (mapping_tag == NULL) mapping_tag = install("kit_mapping");
  return mapping_tag;
}

#ifndef WIN32
static char *duplicateMapName(const char *name) {
  const size_t n = strlen(name);
  if (n == SIZE_MAX) return NULL;
  char *copy = (char *) R_Calloc(n + 1, char);
  if (copy != NULL) memcpy(copy, name, n + 1);
  return copy;
}

static int openMap(const char *name, bool *created) {
  int fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
  if (fd != -1) {
    *created = true;
    return fd;
  }
  if (errno != EEXIST) return -1;
  *created = false;
  return shm_open(name, O_RDWR, S_IRUSR | S_IWUSR);
}
#endif

#ifndef WIN32
#define TOKEN_OFFSET (sizeof(size_t))

static uint64_t mapping_token_counter = 0;

static uint64_t createMappingToken(void *address) {
  uint64_t token = (uint64_t) (uintptr_t) address;
  token ^= (uint64_t) getpid() << 32;
  token ^= ++mapping_token_counter;
  token ^= (uint64_t) time(NULL);
  return token == 0 ? 1 : token;
}

static bool mappingTokenMatches(const char *name, uint64_t token) {
  if (name == NULL) return false;
  const int fd = shm_open(name, O_RDONLY, S_IRUSR | S_IWUSR);
  if (fd == -1) return false;
  struct stat status;
  if (fstat(fd, &status) == -1 || status.st_size < (off_t) (TOKEN_OFFSET + sizeof(uint64_t))) {
    close(fd);
    return false;
  }
  void *mapping = mmap(NULL, 256, PROT_READ, MAP_SHARED, fd, 0);
  if (mapping == MAP_FAILED) {
    close(fd);
    return false;
  }
  uint64_t current_token;
  memcpy(&current_token, (const char *) mapping + TOKEN_OFFSET, sizeof(current_token));
  munmap(mapping, 256);
  close(fd);
  return current_token == token;
}
#endif

static void releaseObject(struct OBJECT *ptr, bool unlink_addr, bool unlink_length) {
#ifdef WIN32
  (void) unlink_addr;
  (void) unlink_length;
  if (ptr->lpMapAddress != NULL) UnmapViewOfFile(ptr->lpMapAddress);
  if (ptr->hMapFile != NULL && ptr->hMapFile != INVALID_HANDLE_VALUE) CloseHandle(ptr->hMapFile);
  if (ptr->lpMapLength != NULL) UnmapViewOfFile(ptr->lpMapLength);
  if (ptr->hMapLength != NULL && ptr->hMapLength != INVALID_HANDLE_VALUE) CloseHandle(ptr->hMapLength);
#else
  if (ptr->fd_addr >= 0) {
    const int fd_addr = ptr->fd_addr;
    ptr->fd_addr = -1;
    close(fd_addr);
  }
  if (ptr->fd_length >= 0) {
    const int fd_length = ptr->fd_length;
    ptr->fd_length = -1;
    close(fd_length);
  }
  if ((unlink_addr || unlink_length) && ptr->have_token &&
      !mappingTokenMatches(ptr->LENGTH_ID, ptr->token)) {
    unlink_addr = false;
    unlink_length = false;
  }
  if (ptr->addr != NULL && ptr->addr != MAP_FAILED && ptr->STORAGE_SIZE > 0) {
    munmap(ptr->addr, ptr->STORAGE_SIZE);
    ptr->addr = NULL;
  }
  if (ptr->STORAGE_ID != NULL) {
    if (unlink_addr) shm_unlink(ptr->STORAGE_ID);
    R_Free(ptr->STORAGE_ID);
    ptr->STORAGE_ID = NULL;
  }
  if (ptr->length != NULL && ptr->length != MAP_FAILED) {
    munmap(ptr->length, 256);
    ptr->length = NULL;
  }
  if (ptr->LENGTH_ID != NULL) {
    if (unlink_length) shm_unlink(ptr->LENGTH_ID);
    R_Free(ptr->LENGTH_ID);
    ptr->LENGTH_ID = NULL;
  }
#endif
  R_Free(ptr);
}

struct READ_MAPPING {
#ifdef WIN32
  HANDLE hMapFile;
  HANDLE hMapLength;
  LPCTSTR lpMapAddress;
  LPCTSTR lpMapLength;
#else
  int fd_addr;
  int fd_length;
  size_t addr_size;
  void *addr;
  void *length;
#endif
  const char *name_addr;
  const char *name_length;
  bool verbose;
  bool unlink_names;
  bool cleanup_failed;
  SEXP ans;
};

static void initReadMapping(struct READ_MAPPING *state) {
#ifdef WIN32
  state->hMapFile = NULL;
  state->hMapLength = NULL;
  state->lpMapAddress = NULL;
  state->lpMapLength = NULL;
#else
  state->fd_addr = -1;
  state->fd_length = -1;
  state->addr_size = 0;
  state->addr = NULL;
  state->length = NULL;
#endif
  state->name_addr = NULL;
  state->name_length = NULL;
  state->verbose = false;
  state->unlink_names = false;
  state->cleanup_failed = false;
  state->ans = R_NilValue;
}

static void releaseReadMapping(struct READ_MAPPING *state, bool unlink_names) {
#ifdef WIN32
  (void) unlink_names;
  if (state->lpMapAddress != NULL) {
    if (!UnmapViewOfFile(state->lpMapAddress)) state->cleanup_failed = true;
    state->lpMapAddress = NULL;
  }
  if (state->hMapFile != NULL && state->hMapFile != INVALID_HANDLE_VALUE) {
    if (!CloseHandle(state->hMapFile)) state->cleanup_failed = true;
    state->hMapFile = NULL;
  }
  if (state->lpMapLength != NULL) {
    if (!UnmapViewOfFile(state->lpMapLength)) state->cleanup_failed = true;
    state->lpMapLength = NULL;
  }
  if (state->hMapLength != NULL && state->hMapLength != INVALID_HANDLE_VALUE) {
    if (!CloseHandle(state->hMapLength)) state->cleanup_failed = true;
    state->hMapLength = NULL;
  }
#else
  if (state->addr != NULL && state->addr != MAP_FAILED && state->addr_size > 0) {
    if (munmap(state->addr, state->addr_size) == -1) state->cleanup_failed = true;
    state->addr = NULL;
  }
  if (state->length != NULL && state->length != MAP_FAILED) {
    if (munmap(state->length, 256) == -1) state->cleanup_failed = true;
    state->length = NULL;
  }
  if (state->fd_addr >= 0) {
    const int fd_addr = state->fd_addr;
    state->fd_addr = -1;
    if (close(fd_addr) == -1) state->cleanup_failed = true;
  }
  if (state->fd_length >= 0) {
    const int fd_length = state->fd_length;
    state->fd_length = -1;
    if (close(fd_length) == -1) state->cleanup_failed = true;
  }
  if (unlink_names) {
    if (state->name_addr != NULL) {
      if (shm_unlink(state->name_addr) == -1) state->cleanup_failed = true;
      state->name_addr = NULL;
    }
    if (state->name_length != NULL) {
      if (shm_unlink(state->name_length) == -1) state->cleanup_failed = true;
      state->name_length = NULL;
    }
  }
#endif
}

static void readMappingCleanup(void *data) {
  struct READ_MAPPING *state = (struct READ_MAPPING *) data;
  releaseReadMapping(state, state->unlink_names);
}

static bool closeReadDescriptors(struct READ_MAPPING *state) {
#ifdef WIN32
  (void) state;
  return true;
#else
  bool success = true;
  if (state->fd_addr >= 0) {
    const int fd_addr = state->fd_addr;
    state->fd_addr = -1;
    if (close(fd_addr) == -1) success = false;
  }
  if (state->fd_length >= 0) {
    const int fd_length = state->fd_length;
    state->fd_length = -1;
    if (close(fd_length) == -1) success = false;
  }
  return success;
#endif
}

static void mapping_error(SEXP ext, struct OBJECT *ptr, bool unlink_addr,
                          bool unlink_length, const char *message) {
  R_ClearExternalPtr(ext);
  UNPROTECT(1);
  releaseObject(ptr, unlink_addr, unlink_length);
  Rf_error("%s", message);
}

static void map_finalizer(SEXP ext) {
  struct OBJECT *ptr = (struct OBJECT *) R_ExternalPtrAddr(ext);
  if (ptr == NULL) return;
  const bool verbose = ptr->verbose;
  R_ClearExternalPtr(ext);
#ifdef WIN32
  releaseObject(ptr, false, false);
#else
  releaseObject(ptr, ptr->own_addr, ptr->own_length);
#endif
  if (verbose) Rprintf("* Finalize...\n");
  if (verbose) Rprintf("* Clear external pointer...\n");
  if (verbose) Rprintf("* Clear external pointer...OK\n");
}

/*
 *  Function to create data
 */

SEXP createMappingObjectR (SEXP MapObjectName, SEXP MapLengthName, SEXP DataObject, SEXP verboseArg) {
  if (TYPEOF(MapObjectName) != STRSXP || XLENGTH(MapObjectName) != 1) {
    Rf_error("Argument 'MapObjectName' must be of type character and length 1.");
  }
  if (TYPEOF(MapLengthName) != STRSXP || XLENGTH(MapLengthName) != 1) {
    Rf_error("Argument 'MapLengthName' must be of type character and length 1.");
  }
  if (TYPEOF(DataObject) != RAWSXP) {
    Rf_error("Argument 'DataObject' must be of type raw vector.");
  }
  if (!IS_BOOL(verboseArg)) {
    Rf_error("Argument 'verbose' must be TRUE or FALSE.");
  }
  const bool verbose = asLogical(verboseArg);
  const R_xlen_t xlen = XLENGTH(DataObject);
  if (xlen < 0 || (uintmax_t) xlen > (uintmax_t) SIZE_MAX ||
      (uintmax_t) xlen > (uintmax_t) (SIZE_MAX / sizeof(Rbyte))) {
    Rf_error("* Data object is too large...ERROR");
  }
  const size_t len = (size_t) xlen;
  const size_t BUF_SIZE = len * sizeof(Rbyte);
  if (BUF_SIZE == 0) Rf_error("* Data object is empty...ERROR");
  if (verbose) Rprintf("* Data object size: %zu\n", BUF_SIZE);
  if (verbose) Rprintf("* Start mapping object...OK\n");

  struct OBJECT *foo = R_Calloc(1, struct OBJECT);
  if (foo == NULL) Rf_error("* Allocating mapping object...ERROR");
  foo->verbose = verbose;
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
  foo->own_addr = false;
  foo->own_length = false;
  foo->token = 0;
  foo->have_token = false;
  foo->STORAGE_SIZE = BUF_SIZE;
#endif
  SEXP ext = PROTECT(R_MakeExternalPtr(foo, getMappingTag(), R_NilValue));

#ifdef WIN32
  LPSTR pMN = (LPSTR) CHAR(STRING_PTR_RO(MapObjectName)[0]);
  LPSTR pML = (LPSTR) CHAR(STRING_PTR_RO(MapLengthName)[0]);
  const uint64_t buffer_size = (uint64_t) BUF_SIZE;
  foo->hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
                                    (DWORD) (buffer_size >> 32),
                                    (DWORD) (buffer_size & 0xFFFFFFFFu), pMN);
  foo->hMapLength = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 256, pML);
  if (foo->hMapFile == NULL || foo->hMapFile == INVALID_HANDLE_VALUE ||
      foo->hMapLength == NULL || foo->hMapLength == INVALID_HANDLE_VALUE) {
    mapping_error(ext, foo, false, false, "* Creating file mapping...ERROR");
  }
  if (verbose) Rprintf("* Creating file mapping...OK\n");
  foo->lpMapAddress = (LPCTSTR) MapViewOfFile(foo->hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, BUF_SIZE);
  foo->lpMapLength = (LPCTSTR) MapViewOfFile(foo->hMapLength, FILE_MAP_ALL_ACCESS, 0, 0, 256);
  if (foo->lpMapAddress == NULL || foo->lpMapLength == NULL) {
    mapping_error(ext, foo, false, false, "* Map view file...ERROR");
  }
  if (verbose) Rprintf("* Map view file...OK\n");
  CopyMemory((LPVOID) foo->lpMapAddress, RAW(DataObject), BUF_SIZE);
  CopyMemory((LPVOID) foo->lpMapLength, &len, sizeof(size_t));
#else
  const char *pMN = CHAR(STRING_PTR_RO(MapObjectName)[0]);
  const char *pML = CHAR(STRING_PTR_RO(MapLengthName)[0]);
  foo->STORAGE_ID = duplicateMapName(pMN);
  foo->LENGTH_ID = duplicateMapName(pML);
  if (foo->STORAGE_ID == NULL || foo->LENGTH_ID == NULL) {
    mapping_error(ext, foo, false, false, "* Duplicating shared memory names...ERROR");
  }
  bool created_addr = false;
  bool created_length = false;
  foo->fd_addr = openMap(foo->STORAGE_ID, &created_addr);
  foo->fd_length = openMap(foo->LENGTH_ID, &created_length);
  foo->own_addr = created_addr;
  foo->own_length = created_length;
  if (foo->fd_addr == -1 || foo->fd_length == -1) {
    const int open_errno = errno;
    Rprintf("shm_open error, errno(%d): %s\n", open_errno, strerror(open_errno));
    mapping_error(ext, foo, created_addr, created_length, "* Creating file mapping...ERROR");
  }
  if (verbose) Rprintf("* Creating file mapping...OK\n");
  struct stat mapstat;
  if (fstat(foo->fd_addr, &mapstat) == -1) {
    mapping_error(ext, foo, created_addr, created_length, "* Inspect shared memory object...ERROR");
  }
  if (mapstat.st_size < 0) {
    mapping_error(ext, foo, created_addr, created_length, "* Invalid shared memory size...ERROR");
  }
  if (mapstat.st_size == 0) {
    if (ftruncate(foo->fd_addr, BUF_SIZE) == -1) {
      mapping_error(ext, foo, created_addr, created_length, "* Extend shared memory object (1)...ERROR");
    }
  } else if ((uintmax_t) mapstat.st_size < (uintmax_t) BUF_SIZE) {
    mapping_error(ext, foo, created_addr, created_length, "* Shared memory object is too small...ERROR");
  }
  if (fstat(foo->fd_length, &mapstat) == -1) {
    mapping_error(ext, foo, created_addr, created_length, "* Inspect shared memory length...ERROR");
  }
  if (mapstat.st_size < 0) {
    mapping_error(ext, foo, created_addr, created_length, "* Invalid shared memory length size...ERROR");
  }
  if (mapstat.st_size == 0) {
    if (ftruncate(foo->fd_length, 256) == -1) {
      mapping_error(ext, foo, created_addr, created_length, "* Extend shared memory object (2)...ERROR");
    }
  } else if ((uintmax_t) mapstat.st_size < 256u) {
    mapping_error(ext, foo, created_addr, created_length, "* Shared memory length is too small...ERROR");
  }
  if (verbose) Rprintf("* Extend shared memory object...OK\n");
  foo->addr = mmap(NULL, BUF_SIZE, PROT_WRITE, MAP_SHARED, foo->fd_addr, 0);
  foo->length = mmap(NULL, 256, PROT_WRITE, MAP_SHARED, foo->fd_length, 0);
  if (foo->addr == MAP_FAILED || foo->length == MAP_FAILED) {
    mapping_error(ext, foo, created_addr, created_length, "* Map view file...ERROR");
  }
  if (verbose) Rprintf("* Map view file...OK\n");
  if (created_addr && created_length) {
    foo->token = createMappingToken(foo);
    memcpy((char *) foo->length + TOKEN_OFFSET, &foo->token, sizeof(foo->token));
    foo->have_token = true;
  }
  int close_addr = close(foo->fd_addr);
  foo->fd_addr = -1;
  int close_length = close(foo->fd_length);
  foo->fd_length = -1;
  if (close_addr == -1 || close_length == -1) {
    mapping_error(ext, foo, created_addr, created_length, "* Closing file descriptors...ERROR");
  }
  memcpy(foo->addr, RAW(DataObject), BUF_SIZE);
  memcpy(foo->length, &len, sizeof(size_t));
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

static SEXP readMappingObject(void *data) {
  struct READ_MAPPING *state = (struct READ_MAPPING *) data;
  const char *pMN = state->name_addr;
  const char *pML = state->name_length;
#ifdef WIN32
  state->hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, pMN);
  state->hMapLength = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, pML);
  if (state->hMapFile == NULL || state->hMapFile == INVALID_HANDLE_VALUE ||
      state->hMapLength == NULL || state->hMapLength == INVALID_HANDLE_VALUE) {
    Rf_error("* Creating file mapping...ERROR");
  }
#else
  state->fd_addr = shm_open(pMN, O_RDONLY, S_IRUSR | S_IWUSR);
  state->fd_length = shm_open(pML, O_RDONLY, S_IRUSR | S_IWUSR);
  if (state->fd_addr == -1 || state->fd_length == -1) {
    Rf_error("* Creating file mapping...ERROR");
  }
#endif
  if (state->verbose) Rprintf("* Creating file mapping...OK\n");
#ifdef WIN32
  state->lpMapLength = (LPCTSTR) MapViewOfFile(state->hMapLength, FILE_MAP_ALL_ACCESS, 0, 0, 256);
  if (state->lpMapLength == NULL) {
    Rf_error("* Map view file (length)...ERROR");
  }
#else
  struct stat lengthstat;
  if (fstat(state->fd_length, &lengthstat) == -1 || lengthstat.st_size < 256) {
    Rf_error("* Invalid shared memory length size...ERROR");
  }
  state->length = mmap(NULL, 256, PROT_READ, MAP_SHARED, state->fd_length, 0);
  if (state->length == MAP_FAILED) {
    Rf_error("* Map view file (length)...ERROR");
  }
#endif
  if (state->verbose) Rprintf("* Map view file (length)...OK\n");
#ifdef WIN32
  size_t len = *(size_t *) state->lpMapLength;
#else
  size_t len = *(size_t *) state->length;
#endif
  if (len == 0 || (uintmax_t) len > (uintmax_t) R_XLEN_T_MAX ||
      (uintmax_t) len > (uintmax_t) (SIZE_MAX / sizeof(Rbyte))) {
    Rf_error("* Invalid shared data length...ERROR");
  }
#ifdef WIN32
  state->lpMapAddress = (LPCTSTR) MapViewOfFile(state->hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, len*sizeof(Rbyte));
  if (state->lpMapAddress == NULL) {
    Rf_error("* Map view file (address)...ERROR");
  }
#else
  struct stat addrstat;
  if (fstat(state->fd_addr, &addrstat) == -1 || addrstat.st_size < 0 ||
      (uintmax_t) addrstat.st_size < (uintmax_t) len) {
    Rf_error("* Shared data size is invalid...ERROR");
  }
  state->addr_size = len * sizeof(Rbyte);
  state->addr = mmap(NULL, state->addr_size, PROT_READ, MAP_SHARED, state->fd_addr, 0);
  if (state->addr == MAP_FAILED) {
    Rf_error("* Map view file (address)...ERROR");
  }
#endif
  if (state->verbose) Rprintf("* Map view file (address)...OK\n");
#ifndef WIN32
  if (!closeReadDescriptors(state)) {
    Rf_error("* Closing file descriptors...ERROR");
  }
#endif
  state->ans = PROTECT(allocVector(RAWSXP, len));
  if (state->verbose) Rprintf("* Create RAW Vector...OK\n");
#ifdef WIN32
  CopyMemory(RAW(state->ans), (Rbyte *) state->lpMapAddress, len*sizeof(Rbyte));
#else
  memcpy(RAW(state->ans), (Rbyte *) state->addr, len*sizeof(Rbyte));
#endif
  if (state->verbose) Rprintf("* Copy map memory...OK\n");
  state->unlink_names = true;
  return state->ans;
}

SEXP getMappingObjectR (SEXP MapObjectName, SEXP MapLengthName, SEXP verboseArg) {
  if (TYPEOF(MapObjectName) != STRSXP || XLENGTH(MapObjectName) != 1) {
    Rf_error("Argument 'MapObjectName' must be of type character and length 1.");
  }
  if (TYPEOF(MapLengthName) != STRSXP || XLENGTH(MapLengthName) != 1) {
    Rf_error("Argument 'MapLengthName' must be of type character and length 1.");
  }
  if (!IS_BOOL(verboseArg)) {
    Rf_error("Argument 'verbose' must be TRUE or FALSE.");
  }
  const bool verbose = asLogical(verboseArg);
  struct READ_MAPPING state;
  initReadMapping(&state);
  state.name_addr = CHAR(STRING_PTR_RO(MapObjectName)[0]);
  state.name_length = CHAR(STRING_PTR_RO(MapLengthName)[0]);
  state.verbose = verbose;
  SEXP ans = R_ExecWithCleanup(readMappingObject, &state, readMappingCleanup, &state);
  const bool cleanup_failed = state.cleanup_failed;
  if (cleanup_failed) {
    if (state.ans != R_NilValue) UNPROTECT(1);
    Rf_error("* Cleaning shared memory mapping...ERROR");
  }
  if (verbose) {
    Rprintf("* Closing mapping file (length)...OK\n");
    Rprintf("* Closing mapping handle (length)...OK\n");
    Rprintf("* Closing mapping file (address)...OK\n");
    Rprintf("* Closing mapping handle (address)...OK\n");
  }
  if (state.ans != R_NilValue) UNPROTECT(1);
  return ans;
}


/*
 *  Function to clear mapping object
 */

SEXP clearMappingObjectR  (SEXP ext, SEXP verboseArg) {
  if (TYPEOF(ext) != EXTPTRSXP || R_ExternalPtrTag(ext) != getMappingTag()) {
    Rf_error("Argument 'x' must be an external pointer like the one returned by shareData().");
  }
  if (!IS_BOOL(verboseArg)) {
    Rf_error("Argument 'verbose' must be TRUE or FALSE.");
  }
  struct OBJECT *ptr = (struct OBJECT *) R_ExternalPtrAddr(ext);
  if (ptr == NULL) {
    return ScalarLogical(FALSE);
  }
  ptr->verbose = asLogical(verboseArg);
  map_finalizer(ext);
  return ScalarLogical(TRUE);
}
