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



/*
 * Util:
 *	General utilities used by the virtual lab.
 *	These should be put into a library.
 *
 * getline(fp)
 *	- gets next non-blank line from the file pointed to by fp,
 *	  stores it using strdup(), and returns a pointer to it.
 *
 * getlineNC(fp)
 *	- uses the getline() to obtain a line that does not start
 *        with a ';'
 *
 * strdupn(s, n)
 * 	- stores n chars from buf in a permanent location,
 *	  and returns a pointer to it
 *	- uses the string function strdup()
 *
 * strcpyc(s1, s2, c)
 *	- looks for the first occurrence of c in s2, and
 *	  copies all chars preceding it from s2 to s1
 *	- puts a NULL char at the end of s1
 *	- if c is not in s2, all of s2 is copies to s1
 *	- uses the string functions strchr() and strncpy()
 *
 * strrcpyc(s1, s2, c)
 *	- looks for the last occurrence of c in s2, and
 *	  copies all chars preceding it from s2 to s1
 *	- puts a NULL char at the end of s1
 *	- if c is not in s2, returns NULL, else s1
 *	- uses the string functions strrchr() and strncpy()
 *
 * ntabs(line)
 *	- counts the number of tab chars at the beginning of line
 *
 * movecursor(x,y)
 *	- moves the cursor to given coordinates
 *
 * hourglass()
 *	- changes the cursor to an hour glass
 *	- to change back to arrow: setcursor(0,0,0)
 *
 * small_sleep:
 *	- wait delay/100 of a second
 *	- unfortunately this is a "busy	while waiting" implementation.
 *
 * Lynn Mercer
 * October,1989 - August,1990
 */

#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "util.h"
#include "xmemory.h"

#define STRLEN 1000

char *getline(FILE *fp) {
  char buf[STRLEN];
  int len;
  char *b;

  do {
    if (fgets(buf, STRLEN, fp) == NULL)
      return (NULL);
    len = strlen(buf);
    buf[len - 1] = (char)NULL; /* remove NL */
  } while (len <= 1);
  b = (char *)xmalloc(strlen(buf) + 1);
  memcpy(b, buf, strlen(buf) + 1);
  return (b);
}

char *getlineNC(FILE *fp) {
  char *result;

  while (1 == 1) {
    result = getline(fp);
    if (result == NULL)
      return NULL;
    if (result[0] != ';') {
      return result;
    }

    xfree(result);
  }
}

char *strdupn(char *s, int n) {
  char buf[STRLEN];
  char *b;

  strncpy(buf, s, n);
  buf[n] = (char)NULL;
  b = (char *)xmalloc(strlen(buf) + 1);
  memcpy(b, buf, strlen(buf) + 1);
  return (b);
}

char *strcpyc(char *s1, char *s2, char c) {
  char *cptr;
  int n;

  if ((cptr = strchr(s2, c)) == NULL)
    strcpy(s1, s2);
  else {
    n = cptr - s2;
    strncpy(s1, s2, n);
    s1[n] = (char)NULL;
  }
  return (s1);
}

char *strrcpyc(char *s1, char *s2, char c) {
  char *cptr;
  int n;

  if ((cptr = strrchr(s2, c)) == NULL)
    return (NULL);
  n = cptr - s2;
  strncpy(s1, s2, n);
  s1[n] = (char)NULL;
  return (s1);
}

int ntabs(char line[]) {
  int t;

  for (t = 0; line[t] == '\t'; t++)
    ;
  return (t);
}

#ifdef DONT_COMPILE

void small_sleep(int delay) {
  int end, current;

  end = get_time();
  end += delay;

  while (1) {
    current = get_time();
    if (current > end)
      break;
  }
}

/*
 * get_time()
 *	- retrieve the time in units of 1/100 of a second
 */

int get_time(void) {
  struct timeval t;

  gettimeofday(&t, 0);
  return (t.tv_usec / 10000 + t.tv_sec * 100);
}

#endif
