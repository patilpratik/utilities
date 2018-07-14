/*
 * ptar.c
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

#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "ptar.h"

typedef struct
{
  char name[100];
  char mode[8];
  char owner[8];
  char group[8];
  char size[12];
  char mtime[12];
  char checksum[8];
  char type;
  char linkname[100];
  char _padding[255];
} ptar_raw_header_t;

static unsigned
round_up (unsigned n, unsigned incr)
{
  return n + (incr - n % incr) % incr;
}

static unsigned
checksum (const ptar_raw_header_t* rh)
{
  unsigned i;
  unsigned char *p = (unsigned char*) rh;
  unsigned res = 256;
  for (i = 0; i < offsetof(ptar_raw_header_t, checksum); i++)
    {
      res += p[i];
    }
  for (i = offsetof(ptar_raw_header_t, type); i < sizeof(*rh); i++)
    {
      res += p[i];
    }
  return res;
}

static int
tread (ptar_t *tar, void *data, unsigned size)
{
  int err = tar->read (tar, data, size);
  tar->pos += size;
  return err;
}

static int
twrite (ptar_t *tar, const void *data, unsigned size)
{
  int err = tar->write (tar, data, size);
  tar->pos += size;
  return err;
}

static int
write_null_bytes (ptar_t *tar, int n)
{
  int i, err;
  char nul = '\0';
  for (i = 0; i < n; i++)
    {
      err = twrite (tar, &nul, 1);
      if (err)
        {
          return err;
        }
    }
  return PTAR_ESUCCESS;
}

static int
raw_to_header (ptar_header_t *h, const ptar_raw_header_t *rh)
{
  unsigned chksum1, chksum2;

  /* If the checksum starts with a null byte we assume the record is NULL */
  if (*rh->checksum == '\0')
    {
      return PTAR_ENULLRECORD;
    }

  /* Build and compare checksum */
  chksum1 = checksum (rh);
  sscanf (rh->checksum, "%o", &chksum2);
  if (chksum1 != chksum2)
    {
      return PTAR_EBADCHKSUM;
    }

  /* Load raw header into header */
  sscanf (rh->mode, "%o", &h->mode);
  sscanf (rh->owner, "%o", &h->owner);
  sscanf (rh->size, "%o", &h->size);
  sscanf (rh->mtime, "%o", &h->mtime);
  h->type = rh->type;
  strcpy (h->name, rh->name);
  strcpy (h->linkname, rh->linkname);

  return PTAR_ESUCCESS;
}

static int
header_to_raw (ptar_raw_header_t *rh, const ptar_header_t *h)
{
  unsigned chksum;

  /* Load header into raw header */
  memset (rh, 0, sizeof(*rh));
  sprintf (rh->mode, "%o", h->mode);
  sprintf (rh->owner, "%o", h->owner);
  sprintf (rh->size, "%o", h->size);
  sprintf (rh->mtime, "%o", h->mtime);
  rh->type = h->type ? h->type : PTAR_TREG;
  strcpy (rh->name, h->name);
  strcpy (rh->linkname, h->linkname);

  /* Calculate and write checksum */
  chksum = checksum (rh);
  sprintf (rh->checksum, "%06o", chksum);
  rh->checksum[7] = ' ';

  return PTAR_ESUCCESS;
}

const char*
ptar_strerror (int err)
{
  switch (err)
    {
    case PTAR_ESUCCESS:
      return "success";
    case PTAR_EFAILURE:
      return "failure";
    case PTAR_EOPENFAIL:
      return "could not open";
    case PTAR_EREADFAIL:
      return "could not read";
    case PTAR_EWRITEFAIL:
      return "could not write";
    case PTAR_ESEEKFAIL:
      return "could not seek";
    case PTAR_EBADCHKSUM:
      return "bad checksum";
    case PTAR_ENULLRECORD:
      return "null record";
    case PTAR_ENOTFOUND:
      return "file not found";
    }
  return "unknown error";
}

int
ptar_close (ptar_t *tar)
{
  return tar->close (tar);
}

int
ptar_seek (ptar_t *tar, unsigned pos)
{
  int err = tar->seek (tar, pos);
  tar->pos = pos;
  return err;
}

int
ptar_rewind (ptar_t *tar)
{
  tar->remaining_data = 0;
  tar->last_header = 0;
  return ptar_seek (tar, 0);
}

int
ptar_next (ptar_t *tar)
{
  int err, n;
  ptar_header_t h;
  /* Load header */
  err = ptar_read_header (tar, &h);
  if (err)
    {
      return err;
    }
  /* Seek to next record */
  n = round_up (h.size, 512) + sizeof(ptar_raw_header_t);
  return ptar_seek (tar, tar->pos + n);
}

int
ptar_find (ptar_t *tar, const char *name, ptar_header_t *h)
{
  int err;
  ptar_header_t header;
  /* Start at beginning */
  err = ptar_rewind (tar);
  if (err)
    {
      return err;
    }
  /* Iterate all files until we hit an error or find the file */
  while ((err = ptar_read_header (tar, &header)) == PTAR_ESUCCESS)
    {
      if (!strcmp (header.name, name))
        {
          if (h)
            {
              *h = header;
            }
          return PTAR_ESUCCESS;
        }
      ptar_next (tar);
    }
  /* Return error */
  if (err == PTAR_ENULLRECORD)
    {
      err = PTAR_ENOTFOUND;
    }
  return err;
}

int
ptar_read_header (ptar_t *tar, ptar_header_t *h)
{
  int err;
  ptar_raw_header_t rh;
  /* Save header position */
  tar->last_header = tar->pos;
  /* Read raw header */
  err = tread (tar, &rh, sizeof(rh));
  if (err)
    {
      return err;
    }
  /* Seek back to start of header */
  err = ptar_seek (tar, tar->last_header);
  if (err)
    {
      return err;
    }
  /* Load raw header into header struct and return */
  return raw_to_header (h, &rh);
}

int
ptar_read_data (ptar_t *tar, void *ptr, unsigned size)
{
  int err;
  /* If we have no remaining data then this is the first read, we get the size,
   * set the remaining data and seek to the beginning of the data */
  if (tar->remaining_data == 0)
    {
      ptar_header_t h;
      /* Read header */
      err = ptar_read_header (tar, &h);
      if (err)
        {
          return err;
        }
      /* Seek past header and init remaining data */
      err = ptar_seek (tar, tar->pos + sizeof(ptar_raw_header_t));
      if (err)
        {
          return err;
        }
      tar->remaining_data = h.size;
    }
  /* Read data */
  err = tread (tar, ptr, size);
  if (err)
    {
      return err;
    }
  tar->remaining_data -= size;
  /* If there is no remaining data we've finished reading and seek back to the
   * header */
  if (tar->remaining_data == 0)
    {
      return ptar_seek (tar, tar->last_header);
    }
  return PTAR_ESUCCESS;
}

int
ptar_write_header (ptar_t *tar, const ptar_header_t *h)
{
  ptar_raw_header_t rh;
  /* Build raw header and write */
  header_to_raw (&rh, h);
  tar->remaining_data = h->size;
  return twrite (tar, &rh, sizeof(rh));
}

int
ptar_write_file_header (ptar_t *tar, const char *name, unsigned size)
{
  ptar_header_t h;
  /* Build header */
  memset (&h, 0, sizeof(h));
  strcpy (h.name, name);
  h.size = size;
  h.type = PTAR_TREG;
  h.mode = 0664;
  /* Write header */
  return ptar_write_header (tar, &h);
}

int
ptar_write_dir_header (ptar_t *tar, const char *name)
{
  ptar_header_t h;
  /* Build header */
  memset (&h, 0, sizeof(h));
  strcpy (h.name, name);
  h.type = PTAR_TDIR;
  h.mode = 0775;
  /* Write header */
  return ptar_write_header (tar, &h);
}

int
ptar_write_data (ptar_t *tar, const void *data, unsigned size)
{
  int err;
  /* Write data */
  err = twrite (tar, data, size);
  if (err)
    {
      return err;
    }
  tar->remaining_data -= size;
  /* Write padding if we've written all the data for this file */
  if (tar->remaining_data == 0)
    {
      return write_null_bytes (tar, round_up (tar->pos, 512) - tar->pos);
    }
  return PTAR_ESUCCESS;
}

int
ptar_finalize (ptar_t *tar)
{
  /* Write two NULL records */
  return write_null_bytes (tar, sizeof(ptar_raw_header_t) * 2);
}

/*
 * function for no posix systems
 */
#ifndef POSIX_SYSTEM
static int file_write(ptar_t *tar, const void *data, unsigned size)
  {
    unsigned res = fwrite(data, 1, size, tar->stream);
    return (res == size) ? PTAR_ESUCCESS : PTAR_EWRITEFAIL;
  }

static int file_read(ptar_t *tar, void *data, unsigned size)
  {
    unsigned res = fread(data, 1, size, tar->stream);
    return (res == size) ? PTAR_ESUCCESS : PTAR_EREADFAIL;
  }

static int file_seek(ptar_t *tar, unsigned offset)
  {
    int res = fseek(tar->stream, offset, SEEK_SET);
    return (res == 0) ? PTAR_ESUCCESS : PTAR_ESEEKFAIL;
  }

static int file_close(ptar_t *tar)
  {
    fclose(tar->stream);
    return PTAR_ESUCCESS;
  }

int ptar_open(ptar_t *tar, const char *filename, const char *mode)
  {
    int err;
    ptar_header_t h;

    /* Init tar struct and functions */
    memset(tar, 0, sizeof(*tar));
    tar->write = file_write;
    tar->read = file_read;
    tar->seek = file_seek;
    tar->close = file_close;

    /* Assure mode is always binary */
    if ( strchr(mode, 'r') ) mode = "rb";
    if ( strchr(mode, 'w') ) mode = "wb";
    if ( strchr(mode, 'a') ) mode = "ab";
    /* Open file */
    tar->stream = fopen(filename, mode);
    if (!tar->stream)
      {
        return PTAR_EOPENFAIL;
      }
    /* Read first header to check it is valid if mode is `r` */
    if (*mode == 'r')
      {
        err = ptar_read_header(tar, &h);
        if (err != PTAR_ESUCCESS)
          {
            ptar_close(tar);
            return err;
          }
      }

    /* Return ok */
    return PTAR_ESUCCESS;
  }
#endif

#ifdef POSIX_SYSTEM
/*
 * functions using mmap for POSIX systems.
 *
 */
static int
file_write (ptar_t *tar, const void *data, unsigned size)
{
  if (NULL != tar->stream)
    {
      memcpy (((struct mmap_info*) (tar->stream))->data + tar->pos, data, size);
      return PTAR_ESUCCESS;
    }
  return PTAR_EWRITEFAIL;
}

static int
file_read (ptar_t *tar, void *data, unsigned size)
{
  if (NULL != tar->stream)
    {
      memcpy (data, ((struct mmap_info*) tar->stream)->data + tar->pos, size);
      return PTAR_ESUCCESS;
    }
  return PTAR_EREADFAIL;
}

static int
file_seek (ptar_t *tar, unsigned offset)
{
  if ( NULL != tar->stream)
    {
      tar->pos = offset;
      if (tar->pos > ((struct mmap_info*) tar->stream)->size)
        {
          return PTAR_ESEEKFAIL;
        }
      return PTAR_ESUCCESS;
    }
  return PTAR_ESEEKFAIL;
}

static int
file_close (ptar_t *tar)
{
  if ( NULL != tar->stream)
    {
      munmap (((struct mmap_info*) tar->stream)->data,
              ((struct mmap_info*) tar->stream)->size);
      close (((struct mmap_info*) tar->stream)->fd);
      return PTAR_ESUCCESS;
    }
  return PTAR_EFAILURE;
}
int
fileModeMapper (const int mode)
{
  if ( PROT_READ == mode)
    return O_RDONLY;
  if (PROT_WRITE == mode)
    return (O_RDWR |O_CREAT);

}
int
ptar_open (ptar_t *tar, const char *filename, const int mode)
{
  int err;
  ptar_header_t h;
  struct stat st ={ 0 };
  struct mmap_info *info ={ 0 };
  int len = 0 ;

  /* Init tar struct and functions */
  memset (tar, 0, sizeof(*tar));
  tar->write = file_write;
  tar->read = file_read;
  tar->seek = file_seek;
  tar->close = file_close;

  /* Open file */
  info = malloc (sizeof(struct mmap_info));

  /* Assure that file opened with the correct mode */

  info->fd = open (filename, fileModeMapper(mode),S_IRUSR |S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
  if (info->fd == -1)
    {
      PTrace(ERROR_LEVEL, "Failed to open file : %s, Error : %d", filename, err=errno);
      ptar_close (tar);
      return PTAR_EOPENFAIL;
    }
  /* Get file info */
  fstat (info->fd, &st);

  /* validate file. Empty file needs to be truncated to right memory */
  if (st.st_size != 0)
    {
      len = st.st_size;
    }
  else
    {
      len = sysconf (_SC_PAGESIZE);
      if (0 != ftruncate (info->fd, len))
        PTrace(ERROR_LEVEL, "Failed to truncate empty file : %s, Error : %d", filename, err = errno);
    }


  /* Map file memory */
  info->data = mmap (
                     NULL,
                     len,
                     mode,
                     MAP_SHARED,
                     info->fd, 0);
  if (MAP_FAILED == info->data)
    {
      tar->stream = NULL;
      PTrace(ERROR_LEVEL, "mmap failed with err : %d", err = errno);
    }
  else
    {
      info->size = st.st_size;
      tar->stream = info;
      err = ptar_read_header (tar, &h);
      /* Read first header to check it is valid if mode is `r` */
      /* PTAR_ENULLRECORD is checked to handle creation of tar. i.e. when tar is empty*/
      if (err != PTAR_ESUCCESS && err != PTAR_ENULLRECORD)
        {
//          ptar_close(tar);
          return err;
        }
      ptar_rewind (tar);
    }

  /* Return ok */
  PTrace(INFO_LEVEL, "File %s is opened successfully.", filename);
  return PTAR_ESUCCESS;
}

int
ptar_get (ptar_t *tar, const char* filename, const void **ptr)
{
  int err;
  ptar_header_t h;

  /* Rewind file */
  ptar_rewind (tar);
  /* Find file point */
  while ((err = ptar_read_header (tar, &h)) == PTAR_ESUCCESS)
    {
      if (!strcmp (h.name, filename))
        {
          tar->pos += sizeof(ptar_raw_header_t);
          break;
        }
      ptar_next (tar);
    }
  /* Return mapped file pointer */
  *ptr = ((struct mmap_info*) tar->stream)->data + tar->pos;
  return PTAR_ESUCCESS;
}

int
ptar_get_pointer (ptar_t *tar, const void **ptr)
{
  tar->pos += sizeof(ptar_raw_header_t);
  /* Return pointer to data after header */
  *ptr = ((struct mmap_info*) tar->stream)->data + tar->pos;
  return PTAR_ESUCCESS;
}

#endif
