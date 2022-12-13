#ifdef WIN32
#include "warningset.h"

#include <windows.h>
#endif

#include <stdio.h>

#ifdef WIN32
#include <stdarg.h>
#else /* UNIX */
#include <vararg.h>
#endif

#ifndef WIN32 /* UNIX (console version) */

void Message(const char *format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
}

#else /* Windows version */

void AddMsg(const char *);

FILE *fpLog = NULL;

void Message(const char *format, ...) {
  static char buf[256];

  va_list args;
  va_start(args, format);
  _vsnprintf(buf, 255, format, args);
  va_end(args);

  if (NULL == fpLog) {
    fpLog = fopen("cpfg.log", "wt");
    if (NULL != fpLog)
      setbuf(fpLog, NULL);
  }

  if (NULL != fpLog)
    fputs(buf, fpLog);

  AddMsg(buf);
}

#endif
