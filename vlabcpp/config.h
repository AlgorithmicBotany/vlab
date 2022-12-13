/* ******************************************************************** *
   Copyright (C) 1990-2022 University of Calgary
  
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
  
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * ******************************************************************** */




/* Define if you have a working <inttypes.h> header file.  */
/*#define HAVE_INTTYPES_H 1*/

/* Whether malloc must be declared even if <stdlib.h> is included.  */
/* #undef NEED_DECLARATION_MALLOC */

/* Whether realloc must be declared even if <stdlib.h> is included.  */
/* #undef NEED_DECLARATION_REALLOC */

/* Whether calloc must be declared even if <stdlib.h> is included.  */
/* #undef NEED_DECLARATION_CALLOC */

/* Whether free must be declared even if <stdlib.h> is included.  */
/* #undef NEED_DECLARATION_FREE */

/* Whether index must be declared even if <stdlib.h> is included.  */
/* #define NEED_DECLARATION_INDEX 1 */

/* Whether rindex must be declared even if <stdlib.h> is included.  */
/* #define NEED_DECLARATION_RINDEX 1 */

/* Whether getenv must be declared even if <stdlib.h> is included.  */
/* #undef NEED_DECLARATION_GETENV */

/* Whether sbrk must be declared even if <stdlib.h> is included.  */
/* #undef NEED_DECLARATION_SBRK */

/* Define if you have the ANSI C header files.  */
/* #define STDC_HEADERS 1 */

/* Define if `sys_siglist' is declared by <signal.h>.  */
/* #undef SYS_SIGLIST_DECLARED */

/* Define if you can safely include both <sys/time.h> and <time.h>.  */
/*#define TIME_WITH_SYS_TIME 1*/

/* Define if you have the bcmp function.  */
/* #define HAVE_BCMP 1 */

/* Define if you have the bcopy function.  */
/* #define HAVE_BCOPY 1 */

/* Define if you have the bzero function.  */
/* #define HAVE_BZERO 1 */

/* Define if you have the getrlimit function.  */
/* #define HAVE_GETRLIMIT 1 */

/* Define if you have the index function.  */
/* #define HAVE_INDEX 1 */

/* Define if you have the kill function.  */
/* #define HAVE_KILL 1 */

/* Define if you have the popen function.  */
/* #define HAVE_POPEN 1 */

/* Define if you have the putenv function.  */
/* #define HAVE_PUTENV 1 */

/* Define if you have the rindex function.  */
/* #define HAVE_RINDEX 1 */

/* Define if you have the setrlimit function.  */
/* #define HAVE_SETRLIMIT 1 */

/* Define if you have the strerror function.  */
/* #define HAVE_STRERROR 1 */

/* Define if you have the sysconf function.  */
/* #define HAVE_SYSCONF 1 */

/* Define if you have the vprintf function.  */
/* #define HAVE_VPRINTF 1 */

/* Define if you have the <fcntl.h> header file.  */
/* #define HAVE_FCNTL_H 1 */

/* Define if you have the <limits.h> header file.  */
/* #define HAVE_LIMITS_H 1 */

/* Define if you have the <stddef.h> header file.  */
/* #define HAVE_STDDEF_H 1 */

/* Define if you have the <stdlib.h> header file.  */
/* #define HAVE_STDLIB_H 1 */

/* Define if you have the <string.h> header file.  */
/* #define HAVE_STRING_H 1 */

/* Define if you have the <strings.h> header file.  */
/* #define HAVE_STRINGS_H 1 */

/* Define if you have the <sys/file.h> header file.  */
/* #define HAVE_SYS_FILE_H 1 */

/* Define if you have the <sys/param.h> header file.  */
/* #define HAVE_SYS_PARAM_H 1 */

/* Define if you have the <sys/resource.h> header file.  */
/*#define HAVE_SYS_RESOURCE_H 1*/

/* Define if you have the <sys/time.h> header file.  */
/*#define HAVE_SYS_TIME_H 1*/

/* Define if you have the <sys/times.h> header file.  */
/*#define HAVE_SYS_TIMES_H 1*/

/* Define if you have the <time.h> header file.  */
/* #define HAVE_TIME_H 1 */

/* Define if you have the <unistd.h> header file.  */
/*#define HAVE_UNISTD_H 1*/

#ifdef VLAB_LINUX
#define STDC_HEADERS 1
#define HAVE_BCMP 1
#define HAVE_BCOPY 1
#define HAVE_BZERO 1
#define HAVE_GETRLIMIT 1
#define HAVE_INDEX 1
#define HAVE_KILL 1
#define HAVE_POPEN 1
#define HAVE_PUTENV 1
#define HAVE_RINDEX 1
#define HAVE_SETRLIMIT 1
#define HAVE_STRERROR 1
#define HAVE_SYSCONF 1
#define HAVE_VPRINTF 1
#define HAVE_FCNTL_H 1
#define HAVE_LIMITS_H 1
#define HAVE_STDDEF_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRING_H 1
#define HAVE_STRINGS_H 1
#define HAVE_SYS_FILE_H 1
#define HAVE_SYS_PARAM_H 1
#define HAVE_TIME_H 1
#endif /* VLAB_LINUX */

#ifdef VLAB_MACX
#define STDC_HEADERS 1
#define HAVE_BCMP 1
#define HAVE_BCOPY 1
#define HAVE_BZERO 1
#define HAVE_GETRLIMIT 1
#define HAVE_INDEX 1
#define HAVE_KILL 1
#define HAVE_POPEN 1
#define HAVE_PUTENV 1
#define HAVE_RINDEX 1
#define HAVE_SETRLIMIT 1
#define HAVE_STRERROR 1
#define HAVE_SYSCONF 1
#define HAVE_VPRINTF 1
#define HAVE_FCNTL_H 1
#define HAVE_LIMITS_H 1
#define HAVE_STDDEF_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRING_H 1
#define HAVE_STRINGS_H 1
#define HAVE_SYS_FILE_H 1
#define HAVE_SYS_PARAM_H 1
#define HAVE_TIME_H 1
#endif /* VLAB_MACX */

#ifdef _WIN32
#define STDC_HEADERS 1
#define HAVE_GETRLIMIT 1
#define HAVE_KILL 1
#define HAVE_POPEN 1
#define HAVE_PUTENV 1
#define HAVE_SETRLIMIT 1
#define HAVE_STRERROR 1
#define HAVE_SYSCONF 1
#define HAVE_VPRINTF 1
#define HAVE_FCNTL_H 1
#define HAVE_LIMITS_H 1
#define HAVE_STDDEF_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRING_H 1
#define HAVE_STRINGS_H 1
#define HAVE_SYS_FILE_H 1
#define HAVE_SYS_PARAM_H 1
#define HAVE_TIME_H 1
#endif /* _WIN32 */

#define BITS_PER_WORD 32
#define BITS_PER_UNIT 32

#define TARGET_BELL '\a'
#define TARGET_BS '\b'
#define TARGET_FF '\f'
#define TARGET_NEWLINE '\n'
#define TARGET_CR '\r'
#define TARGET_TAB '\t'
#define TARGET_VT '\v'

#define PREFIX "\\"


/* We use this so we don't get shackled with a bunch of
 * gcc include paths */
#define INCLUDE_DEFAULTS


#define FATAL_EXIT_CODE 33
#define SUCCESS_EXIT_CODE 0

