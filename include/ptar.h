/*
 * ptar.h
 *  Module     :
 *  Description:
 *  Input      :
 *  Output     :
 *  Created on : 27-May-2018
 *  Author     : pratik
 *  License     :
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 3 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. 
 */

#ifndef INCLUDE_PTAR_H_
#define INCLUDE_PTAR_H_

// For POSIX systems, mmap is used.
// If cross-compiling for non POSIX system, make sure to remove following macro
#ifdef __unix__
#define POSIX_SYSTEM
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef ERR_STREAM
#undef ERR_STREAM
#endif
#define LOG_FILE_PATH "error.log"
#define ERR_STREAM fopen(LOG_FILE_PATH, "a+")

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <ptrace.h>
#define PTAR_VERSION "0.1.0"

#ifndef offsetof
#define offsetof(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))
#endif
  enum
  {
    PTAR_ESUCCESS = 0,
    PTAR_EFAILURE = -1,
    PTAR_EOPENFAIL = -2,
    PTAR_EREADFAIL = -3,
    PTAR_EWRITEFAIL = -4,
    PTAR_ESEEKFAIL = -5,
    PTAR_EBADCHKSUM = -6,
    PTAR_ENULLRECORD = -7,
    PTAR_ENOTFOUND = -8
  };

  enum
  {
    PTAR_TREG = '0',
    PTAR_TLNK = '1',
    PTAR_TSYM = '2',
    PTAR_TCHR = '3',
    PTAR_TBLK = '4',
    PTAR_TDIR = '5',
    PTAR_TFIFO = '6'
  };

  typedef struct
  {
    unsigned mode;
    unsigned owner;
    unsigned size;
    unsigned mtime;
    unsigned type;
    char name[100];
    char linkname[100];
  } ptar_header_t;

  typedef struct ptar_t ptar_t;

  struct ptar_t
  {
    int
    (*read) (ptar_t *tar, void *data, unsigned size);
    int
    (*write) (ptar_t *tar, const void *data, unsigned size);
    int
    (*seek) (ptar_t *tar, unsigned pos);
    int
    (*close) (ptar_t *tar);
    void *stream;
    unsigned pos;
    unsigned remaining_data;
    unsigned last_header;
  };

  struct mmap_info
  {
    int fd;
    unsigned char *data;
    unsigned size;
  };

  int
  ptar_close (ptar_t *tar);

  int
  ptar_seek (ptar_t *tar, unsigned pos);
  int
  ptar_rewind (ptar_t *tar);
  int
  ptar_next (ptar_t *tar);
  int
  ptar_find (ptar_t *tar, const char *name, ptar_header_t *h);
  int
  ptar_read_header (ptar_t *tar, ptar_header_t *h);
  int
  ptar_read_data (ptar_t *tar, void *ptr, unsigned size);

  int
  ptar_write_header (ptar_t *tar, const ptar_header_t *h);
  int
  ptar_write_file_header (ptar_t *tar, const char *name, unsigned size);
  int
  ptar_write_dir_header (ptar_t *tar, const char *name);
  int
  ptar_write_data (ptar_t *tar, const void *data, unsigned size);
  int
  ptar_finalize (ptar_t *tar);

#ifdef POSIX_SYSTEM
  int
  ptar_open_mapped (ptar_t *tar, const char *filename);
  int
  ptar_get_mapped (ptar_t *tar, const char *filename, const void **data);
  int
  ptar_get_pointer (ptar_t *tar, const void **ptr);
  int
  ptar_open (ptar_t *tar, const char *filename, const int mode);
#else
int ptar_open(ptar_t *tar, const char *filename, const char *mode);
#endif

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_PTAR_H_ */
