/*
 * PTrace.h
 *  Module     :
 *  Description:
 *  Input      :
 *  Output     :
 *  Created on : 09-Jul-2018
 *  Author     : pratik <patil.pratik.r@gmail.com>
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

#ifndef INCLUDE_PTRACE_H_
#define INCLUDE_PTRACE_H_
#include <time.h>
#include <string.h>

/*
 * [Logging Levels]
 * It defines 4 levels of logging.
 *    1) NO_LOG : it will not log anything
 *    2) ERROR_LEVEL : Used to mark errors.
 *    3) INFO_LEVEL  : Used to log information
 *    4) DEBUG_LEVEL : Used to log info to assist in debugging
 *
 *     You can define LOG_LEVEL to any one of the above. DEBUG_LEVEL has the highest level i.e if
 *     LOG_LEVEL is defined to DEBUG_LEVEL, all other logs will also be recorded.
 *     This will control information that logged.
 *
 * [Log Files]
 * ERR_STREAM macro is used to define the stream where errors will be logged
 * If this macro is not defined, errors will be put to stderr.
 *
 * Following could be the way of using this
 *
 * #define LOG_LEVEL ERROR_LEVEL
 * #ifdef ERR_STREAM
 * #undef ERR_STREAM
 * #endif
 * #define LOG_FILE_PATH "error.log"
 * #define ERR_STREAM fopen(LOG_FILE_PATH, "a+")
 *  ...
 *  PTrace(ERROR_LEVEL, "Failed to open file : %s, Error : %d", filename, err=errno);
 *
 */
static inline char *
timenow ();

#define _FILE strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__

#define NO_LOG          0x00
#define ERROR_LEVEL     0x01
#define INFO_LEVEL      0x02
#define DEBUG_LEVEL     0x03

#define NEWLINE     "\n"
const char* LOG_TAG[] =
  { "", "ERROR", "INFO", "DEBUG" };

#ifndef ERR_STREAM
#define ERR_STREAM stderr
#endif
#ifndef LOG_LEVEL
#define LOG_LEVEL   DEBUG_LEVEL
#endif

#define PRINT_FUNCTION(stream, format, ...)      fprintf(stream, format, __VA_ARGS__)
#define LOG_FMT             "%s | %-7s | %-15s | %s:%d | "
#define LOG_ARGS(logLevel)   timenow(), LOG_TAG[logLevel], _FILE, __FUNCTION__, __LINE__


#define PTrace(logLevel, message, args...) if(NO_LOG != logLevel && logLevel <= LOG_LEVEL) PRINT_FUNCTION(ERR_STREAM, LOG_FMT message NEWLINE, LOG_ARGS(logLevel), ## args)

static inline char*
timenow ()
{
  static char buffer[64];
  time_t rawtime;
  struct tm *timeinfo;

  time (&rawtime);
  timeinfo = localtime (&rawtime);

  strftime (buffer, 64, "%Y-%m-%d %H:%M:%S", timeinfo);

  return buffer;
}

#endif /* INCLUDE_PTRACE_H_ */
