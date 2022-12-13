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



#include "dsprintf.h"
#include "xmemory.h"
#include "xstring.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

/* prototypes */
static void addChar(char **result, int *alloc, int *len, char c);

/******************************************************************************
 *
 * This function works exactly as sprintf, except that it allocates just
 * enough bytes for the resulting string.
 *
 */

char *dsprintf(const char *fmtStr, ...) {
  char *result;
  int fmtStrLen;
  int len;
  int alloc;
  unsigned int j;
  int i;
  char intParam;
  va_list argumentList;

  va_start(argumentList, fmtStr);

  /* at the beginning the result is empty string */
  len = 0;
  alloc = 100;
  result = (char *)xmalloc(sizeof(char) * alloc);
  result[0] = '\0';

  i = 0;
  fmtStrLen = xstrlen(fmtStr);
  while (i < fmtStrLen) {
    if (fmtStr[i] != '%') {
      addChar(&result, &alloc, &len, fmtStr[i]);
      i++;
      continue;
    }
    i++;
    if (fmtStr[i] == '%') {
      addChar(&result, &alloc, &len, '%');
      i++;
      continue;
    }
    if (fmtStr[i] == 's') {
      i++;
      const char *strParam = va_arg(argumentList, char *);
      if (strParam == NULL)
        strParam = "<NULL>";
      for (j = 0; j < xstrlen(strParam); j++)
        addChar(&result, &alloc, &len, strParam[j]);
      continue;
    }
    if (fmtStr[i] == 'd') {
      i++;
      intParam = va_arg(argumentList, int);
      char tmpStr[200];
      sprintf(tmpStr, "%d", intParam);
      for (j = 0; j < xstrlen(tmpStr); j++)
        addChar(&result, &alloc, &len, tmpStr[j]);
      continue;
    }
    addChar(&result, &alloc, &len, '%');
    addChar(&result, &alloc, &len, fmtStr[i]);
    i++;
  }

  result = (char *)xrealloc(result, sizeof(char) * (len + 1));
  result[len] = '\0';

  return result;
}

void addChar(char **result, int *alloc, int *len, char c) {
  if (*alloc <= (*len) + 1) {
    (*alloc) += 100;
    (*result) = (char *)realloc(*result, sizeof(char) * (*alloc));
  }
  (*result)[*len] = c;
  (*len) += 1;
}
