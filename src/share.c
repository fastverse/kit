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
  if (ptr->addr != NULL && ptr->addr != MAP_FAILED && ptr->STORAGE_SIZE > 0) {
    munmap(ptr->addr, ptr->STORAGE_SIZE);
  }
  if (ptr->STORAGE_ID != NULL) {
    shm_unlink(ptr->STORAGE_ID);
    free(ptr->STORAGE_ID);
  }
  if (ptr->length != NULL && ptr->length != MAP_FAILED) {
    munmap(ptr->length, 256);
  }
  if (ptr->LENGTH_ID != NULL) {
    shm_unlink(ptr->LENGTH_ID);
    free(ptr->LENGTH_ID);
  }
#endif
  R_Free(ptr);
  R_ClearExternalPtr(ext);
  if (verbose_finalizer) Rprintf("* Clear external pointer...OK\n");
}

/*
 *  Function to create data
 */

SEXP createMappingObjectR (SEXP MapObjectName, SEXP MapLengthName, SEXP DataObject, SEXP verboseArg) {
  if (TYPEOF(MapObjectName) != STRSXP || LENGTH(MapObjectName) != 1) {
    error("Argument 'MapObjectName' must be of type character and length 1.");
  }
  if (!IS_BOOL(verboseArg)) {
    error("Argument 'verbose' must be TRUE or FALSE.");
  }
  const bool verbose = asLogical(verboseArg);
  verbose_finalizer = verbose;
  const size_t len = LENGTH(DataObject);
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
  foo->hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, BUF_SIZE, pMN);
  foo->hMapLength = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 256, pML);
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
  foo->lpMapLength = (LPCTSTR) MapViewOfFile (foo->hMapLength, FILE_MAP_ALL_ACCESS, 0, 0, 256);
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
  CopyMemory((LPVOID)foo->lpMapAddress, RAW(DataObject), BUF_SIZE);
  CopyMemory((LPVOID)foo->lpMapLength, &len, sizeof(size_t));
#else
  const char *pMN = CHAR(STRING_PTR_RO(MapObjectName)[0]);
  const char *pML = CHAR(STRING_PTR_RO(MapLengthName)[0]);
  foo->STORAGE_ID = strdup(pMN);
  foo->LENGTH_ID = strdup(pML);
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
    Rprintf("shm_open error, errno(%d): %s\n", errno, strerror(errno));
    if (foo->fd_addr != -1) close(foo->fd_addr);
    if (foo->fd_length != -1) close(foo->fd_length);
    free(foo->STORAGE_ID);
    free(foo->LENGTH_ID);
    R_ClearExternalPtr(ext);
    R_Free(foo);
    UNPROTECT(1);
    error("* Creating file mapping...ERROR");
  }
  if (verbose) Rprintf("* Creating file maping...OK\n");
  // Always size the objects: re-sharing the same name must reflect the new payload.
  if (ftruncate(foo->fd_addr, BUF_SIZE) == -1 || ftruncate(foo->fd_length, 256) == -1) {
    close(foo->fd_addr);
    close(foo->fd_length);
    free(foo->STORAGE_ID);
    free(foo->LENGTH_ID);
    R_ClearExternalPtr(ext);
    R_Free(foo);
    UNPROTECT(1);
    error("* Extend shared memory object...ERROR");
  }
  if (verbose) Rprintf("* Extend shared memory object...OK\n");
  foo->addr = mmap(NULL, BUF_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, foo->fd_addr, 0);
  foo->length = mmap(NULL, 256, PROT_READ | PROT_WRITE, MAP_SHARED, foo->fd_length, 0);
  if (foo->addr == MAP_FAILED || foo->length == MAP_FAILED) {
    if (foo->addr != MAP_FAILED) munmap(foo->addr, BUF_SIZE);
    if (foo->length != MAP_FAILED) munmap(foo->length, 256);
    close(foo->fd_addr);
    close(foo->fd_length);
    // Do not unlink here: the name may be shared; owner cleanup happens via finalizer.
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
  if (close(foo->fd_addr) == -1 || close(foo->fd_length) == -1) {
    munmap(foo->addr, BUF_SIZE);
    munmap(foo->length, 256);
    free(foo->STORAGE_ID);
    free(foo->LENGTH_ID);
    R_ClearExternalPtr(ext);
    R_Free(foo);
    UNPROTECT(1);
    error("* Closing file descriptors...ERROR");
  }
  foo->fd_addr = -1;
  foo->fd_length = -1;
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
  if (TYPEOF(MapObjectName) != STRSXP || LENGTH(MapObjectName) != 1) {
    error("Argument 'MapObjectName' must be of type character and length 1.");
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
  const char *pMN = CHAR(STRING_PTR_RO(MapObjectName)[0]);
  const char *pML = CHAR(STRING_PTR_RO(MapLengthName)[0]);
  int fd_addr = shm_open(pMN, O_RDONLY, S_IRUSR | S_IWUSR);
  int fd_length = shm_open(pML, O_RDONLY, S_IRUSR | S_IWUSR);
  if (fd_addr == -1 || fd_length == -1) {
    if (fd_addr != -1) close(fd_addr);
    if (fd_length != -1) close(fd_length);
#endif
    error("* Creating file mapping...ERROR");
  }
  if (verbose) Rprintf("* Creating file maping...OK\n");
#ifdef WIN32
  LPCTSTR lpMapLength = (LPCTSTR) MapViewOfFile (hMapLength, FILE_MAP_ALL_ACCESS, 0, 0, 256);
  if (lpMapLength == NULL) {
    CloseHandle(hMapFile);
    CloseHandle(hMapLength);
#else
  void *length = mmap(NULL, 256, PROT_READ, MAP_SHARED, fd_length, 0);
  if (length == MAP_FAILED) {
    close(fd_addr);
    close(fd_length);
#endif
    error("* Map view file (length)...ERROR");
  }
  if (verbose) Rprintf("* Map view file (length)...OK\n");
#ifdef WIN32
  size_t len = *(size_t*)lpMapLength;
  LPCTSTR lpMapAddress = NULL;
  if (len > 0) {
    lpMapAddress = (LPCTSTR) MapViewOfFile (hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, len*sizeof(Rbyte));
  }
  if (len == 0 || lpMapAddress == NULL) {
    UnmapViewOfFile(lpMapLength);
    CloseHandle(hMapFile);
    CloseHandle(hMapLength);
#else
  size_t len = *(size_t*)length;
  // Detach the length mapping as soon as the size is known; data mapping stays.
  if (munmap(length, 256) == -1) {
    close(fd_addr);
    close(fd_length);
    error("* Closing mapping file (length)...ERROR");
  }
  if (verbose) Rprintf("* Closing mapping file (length)...OK\n");
  if (close(fd_length) == -1) {
    close(fd_addr);
    error("* Closing file descriptor (length)...ERROR");
  }
  if (verbose) Rprintf("* Closing mapping handle (length)...OK\n");
  void *addr = NULL;
  if (len > 0) {
    addr = mmap(NULL, len*sizeof(Rbyte), PROT_READ, MAP_SHARED, fd_addr, 0);
  }
  if (len == 0 || addr == MAP_FAILED) {
    close(fd_addr);
#endif
    error("* Map view file (address)...ERROR");
  }
  if (verbose) Rprintf("* Map view file (address)...OK\n");
  // fds no longer needed once mappings are established.
  // Keep fd_addr open until after mmap; fd_length already closed on POSIX.
#ifdef WIN32
#else
  if (close(fd_addr) == -1) {
    munmap(addr, len*sizeof(Rbyte));
    error("* Closing file descriptor (address)...ERROR");
  }
#endif
  SEXP ans = PROTECT(allocVector(RAWSXP, len));
  if (verbose) Rprintf("* Create RAW Vector...OK\n");
#ifdef WIN32
  CopyMemory(RAW(ans), (Rbyte*)lpMapAddress, len*sizeof(Rbyte));
#else
  memcpy(RAW(ans), (Rbyte*)addr, len*sizeof(Rbyte));
#endif
  if (verbose) Rprintf("* Copy map memory...OK\n");

#ifdef WIN32
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
  if (munmap(addr, len*sizeof(Rbyte)) == -1) {
    UNPROTECT(1);
    error("* Closing mapping file (address)...ERROR");
  }
  if (verbose) Rprintf("* Closing mapping file (address)...OK\n");
  if (verbose) Rprintf("* Closing mapping handle (address)...OK\n");
#endif
  UNPROTECT(1);
  return ans;
}

/*
 *  Function to clear mapping object
 */

SEXP clearMappingObjectR  (SEXP ext, SEXP verboseArg) {
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
