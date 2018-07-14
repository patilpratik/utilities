/*
 * ptar_test.cpp
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

#include <limits.h>
#include "gtest/gtest.h"

#include "ptar.h"

namespace
{
  TEST(Open, CanOpenFile )
  {
    ptar_t tar;

#ifdef POSIX_SYSTEM
    /* Open archive for reading */
    remove("test.tar");
    EXPECT_TRUE(PTAR_ESUCCESS != ptar_open(&tar, "test.tar", PROT_READ));
    /* Open archive for writing */
    EXPECT_TRUE(PTAR_ESUCCESS == ptar_open(&tar, "test.tar", PROT_WRITE));
#else
    /* Open archive for reading */
    remove("test.tar");
    EXPECT_TRUE(PTAR_ESUCCESS != ptar_open(&tar, "test.tar", "r"));
    /* Open archive for writing */
    EXPECT_TRUE(PTAR_ESUCCESS == ptar_open(&tar, "test.tar", "w"));

#endif
    ptar_finalize(&tar);
    ptar_close (&tar);
  }

  TEST(Finalize, CanFinalizeTar)
  {
    ptar_t tar;
#ifdef POSIX_SYSTEM
    ptar_open (&tar, "test.tar", PROT_WRITE);
#else
    ptar_open(&tar,"test.tar","w");
#endif
    EXPECT_TRUE(PTAR_ESUCCESS == ptar_finalize(&tar));
    EXPECT_TRUE(PTAR_ESUCCESS == ptar_close (&tar));
  }
  TEST(WriteData, CanWriteData)
      {
        ptar_t tar;
        const char *str1 = "Hello world";
        const char *str2 = "Goodbye world";
    #ifdef POSIX_SYSTEM
        EXPECT_TRUE(PTAR_ESUCCESS == ptar_open(&tar, "test.tar", PROT_WRITE));
    #else
        EXPECT_TRUE(PTAR_ESUCCESS == ptar_open(&tar, "test.tar", "w"));
    #endif
        /* Write strings to files `test1.txt` and `test2.txt` */
        EXPECT_TRUE(PTAR_ESUCCESS == ptar_write_file_header (&tar, "test1.txt", strlen (str1)));
        EXPECT_TRUE(PTAR_ESUCCESS == ptar_write_data (&tar, str1, strlen (str1)));
        EXPECT_TRUE(PTAR_ESUCCESS == ptar_write_file_header (&tar, "test2.txt", strlen (str2)));
        EXPECT_TRUE(PTAR_ESUCCESS == ptar_write_data (&tar, str2, strlen (str2)));

        /* Finalize -- this needs to be the last thing done before closing */
        ptar_finalize (&tar);

        /* Close archive */
        ptar_close (&tar);
        /* Open archive for writing */
      }
      TEST(ReadData, CanReadDataFromTar)
      {
        ptar_t tar;
        ptar_header_t h;
        const char *str1 = "Hello world";
        const char *str2 = "Goodbye world";
        char *p;

        /* Open archive for reading */
    #ifdef POSIX_SYSTEM
        ptar_open (&tar, "test.tar", PROT_READ);
    #else
        ptar_open(&tar, "test.tar", "r");
    #endif
        /* Load and print contents of file "test1.txt" */
        ptar_find (&tar, "test1.txt", &h);
        p = (char*) calloc (1, h.size + 1);
        ptar_read_data (&tar, p, h.size);
        ASSERT_STREQ(str1, p);
        free (p);

        /* Close archive */
        ptar_close (&tar);
      }
}
