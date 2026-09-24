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
    close(ptr->fd_addr);
    ptr->fd_addr = -1;
  }
  if (ptr->fd_length >= 0) {
    close(ptr->fd_length);
    ptr->fd_length = -1;
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

#ifdef WIN32
static void releaseReadLength(HANDLE hMapFile, HANDLE hMapLength, LPCTSTR lpMapLength) {
  if (lpMapLength != NULL) UnmapViewOfFile(lpMapLength);
  if (hMapFile != NULL && hMapFile != INVALID_HANDLE_VALUE) CloseHandle(hMapFile);
  if (hMapLength != NULL && hMapLength != INVALID_HANDLE_VALUE) CloseHandle(hMapLength);
}
#else
static void releaseReadLength(int fd_addr, int fd_length, void *length) {
  if (length != NULL && length != MAP_FAILED) munmap(length, 256);
  if (fd_addr >= 0) close(fd_addr);
  if (fd_length >= 0) close(fd_length);
}
#endif

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
  releaseObject(ptr, true, true);
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
  int close_addr = close(foo->fd_addr);
  if (close_addr == 0) foo->fd_addr = -1;
  int close_length = close(foo->fd_length);
  if (close_length == 0) foo->fd_length = -1;
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
#ifdef WIN32
  LPSTR pMN = (LPSTR) CHAR(STRING_PTR_RO(MapObjectName)[0]);
  LPSTR pML = (LPSTR) CHAR(STRING_PTR_RO(MapLengthName)[0]);
  HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, pMN);
  HANDLE hMapLength = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, pML);
  if (hMapFile == INVALID_HANDLE_VALUE || hMapLength == INVALID_HANDLE_VALUE) {
    releaseReadLength(hMapFile, hMapLength, NULL);
#else
  const char *pMN = CHAR(STRING_PTR_RO(MapObjectName)[0]);
  const char *pML = CHAR(STRING_PTR_RO(MapLengthName)[0]);
  int fd_addr = shm_open(pMN, O_RDONLY, S_IRUSR | S_IWUSR);
  int fd_length = shm_open(pML, O_RDONLY, S_IRUSR | S_IWUSR);
  if (fd_addr == -1 || fd_length == -1) {
    releaseReadLength(fd_addr, fd_length, NULL);
#endif
    Rf_error("* Creating file mapping...ERROR");
  }
  if (verbose) Rprintf("* Creating file mapping...OK\n");
#ifdef WIN32
  LPCTSTR lpMapLength = (LPCTSTR) MapViewOfFile (hMapLength, FILE_MAP_ALL_ACCESS, 0, 0, 256);
  if (lpMapLength == NULL) {
    releaseReadLength(hMapFile, hMapLength, lpMapLength);
#else
  struct stat lengthstat;
  if (fstat(fd_length, &lengthstat) == -1 || lengthstat.st_size < 256) {
    releaseReadLength(fd_addr, fd_length, NULL);
    Rf_error("* Invalid shared memory length size...ERROR");
  }
  void *length = mmap(NULL, 256, PROT_READ, MAP_SHARED, fd_length, 0);
  if (length == MAP_FAILED) {
    shm_unlink(pML);
    releaseReadLength(fd_addr, fd_length, length);
#endif
    Rf_error("* Map view file (length)...ERROR");
  }
  if (verbose) Rprintf("* Map view file (length)...OK\n");
#ifdef WIN32
  size_t len = *(size_t*)lpMapLength;
  if (len == 0 || (uintmax_t) len > (uintmax_t) R_XLEN_T_MAX ||
      (uintmax_t) len > (uintmax_t) (SIZE_MAX / sizeof(Rbyte))) {
    releaseReadLength(hMapFile, hMapLength, lpMapLength);
    Rf_error("* Invalid shared data length...ERROR");
  }
  LPCTSTR lpMapAddress = (LPCTSTR) MapViewOfFile (hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, len*sizeof(Rbyte));
  if (lpMapAddress == NULL) {
    releaseReadLength(hMapFile, hMapLength, lpMapLength);
    Rf_error("* Map view file (address)...ERROR");
  }
#else
  size_t len = *(size_t*)length;
  if (len == 0 || (uintmax_t) len > (uintmax_t) R_XLEN_T_MAX ||
      (uintmax_t) len > (uintmax_t) (SIZE_MAX / sizeof(Rbyte))) {
    releaseReadLength(fd_addr, fd_length, length);
    Rf_error("* Invalid shared data length...ERROR");
  }
  struct stat addrstat;
  if (fstat(fd_addr, &addrstat) == -1 || addrstat.st_size < 0 ||
      (uintmax_t) addrstat.st_size < (uintmax_t) len) {
    releaseReadLength(fd_addr, fd_length, length);
    Rf_error("* Shared data size is invalid...ERROR");
  }
  void *addr = mmap(NULL, len*sizeof(Rbyte), PROT_READ, MAP_SHARED, fd_addr, 0);
  if (addr == MAP_FAILED) {
    shm_unlink(pMN);
    releaseReadLength(fd_addr, fd_length, length);
    Rf_error("* Map view file (address)...ERROR");
  }
#endif
  if (verbose) Rprintf("* Map view file (address)...OK\n");
#ifndef WIN32
  int close_addr = close(fd_addr);
  if (close_addr == 0) fd_addr = -1;
  int close_length = close(fd_length);
  if (close_length == 0) fd_length = -1;
  if (close_addr == -1 || close_length == -1) {
    releaseReadLength(fd_addr, fd_length, length);
    Rf_error("* Closing file descriptors...ERROR");
  }
#endif
  SEXP ans = PROTECT(allocVector(RAWSXP, len));
  if (verbose) Rprintf("* Create RAW Vector...OK\n");
#ifdef WIN32
  CopyMemory(RAW(ans), (Rbyte*)lpMapAddress, len*sizeof(Rbyte));
#else
  memcpy(RAW(ans), (Rbyte*)addr, len*sizeof(Rbyte)); // maybe need +1
#endif
  if (verbose) Rprintf("* Copy map memory...OK\n");
  
#ifdef WIN32
  if (!UnmapViewOfFile(lpMapLength)) {
#else
  if (munmap(length, 256) == -1) {
#endif
    Rf_error("* Closing mapping file (length)...ERROR");
  }
  if (verbose) Rprintf("* Closing mapping file (length)...OK\n");
#ifdef WIN32
  if (!CloseHandle(hMapLength)) {
#else
  if (shm_unlink(pML) == -1) {
#endif
    Rf_error("* Closing mapping handle (length)...ERROR");
  }
  if (verbose) Rprintf("* Closing mapping handle (length)...OK\n");
  
#ifdef WIN32
  if (!UnmapViewOfFile(lpMapAddress)) {
#else
  if (munmap(addr, len*sizeof(Rbyte)) == -1) {
#endif
    Rf_error("* Closing mapping file (address)...ERROR");
  }
  if (verbose) Rprintf("* Closing mapping file (address)...OK\n");
#ifdef WIN32
  if (!CloseHandle(hMapFile)) {
#else
  if (shm_unlink(pMN) == -1) {
#endif
    Rf_error("* Closing mapping handle (address)...ERROR");
  }
  if (verbose) Rprintf("* Closing mapping handle (address)...OK\n");
  UNPROTECT(1);
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
