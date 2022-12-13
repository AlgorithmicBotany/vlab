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



#include "xstring.h"
#include "xmemory.h"
#include <string>

/*****************************************************************************
 *
 * wrapper for strlen() - will return 0, if s == NULL
 *
 */

size_t xstrlen(const char *s) {
  if (s == NULL)
    return 0;

  return strlen(s);
}

/*****************************************************************************
 *
 * wrapper for strcmp - will handle NULL pointers
 *
 */

int xstrcmp(const char *s1, const char *s2) {
  if (s1 == s2)
    return 0;

  if (s1 == NULL)
    return -1;

  if (s2 == NULL)
    return 1;

  return strcmp(s1, s2);
}

/******************************************************************************
 *
 * wrapper for xstrdup
 *
 *    - uses xmalloc for memory allocation
 *    - handles NULL pointers (returns NULL)
 *
 */

char *xstrdup(const char *s) {
  if (s == NULL)
    return NULL;

  int len = strlen(s);
  char *result = (char *)xmalloc(len + 1);
  memcpy(result, s, len + 1);
  return result;
}

/******************************************************************************
 *
 * return a position of the character 'c' in a string, or -1 if NOT FOUND
 *
 */

int xstrpos(const char *s, char c) {
  if (s == NULL)
    return -1;

  char *ptr = (char *)s;
  int pos = 0;
  while (*ptr != '\0') {
    if (*ptr == c)
      return pos;
    ptr++;
    pos++;
  }
  return -1;
}

/******************************************************************************
 *
 * Just like xstrdup, but copies 'len' bytes
 *
 */

char *xmemdup(const char *src, int len) {
  if (src == NULL)
    return NULL;

  char *dst = (char *)xmalloc(len);
  memcpy(dst, src, len);
  return dst;
}

/******************************************************************************
 *
 * reallocate memory for string and append character to it
 *
 */

void xstrappend(char *&str, char c) {
  int n = xstrlen(str);
  str = (char *)xrealloc(str, n + 2);
  str[n] = c;
  str[n + 1] = '\0';
}

/******************************************************************************
 *
 * return the next token from a string (similar to 'strtok()'), but don't
 * actually overwrite the string.
 *
 */

char *get_token(char *str, char separator) {
  static char *last_ptr = NULL;

  // if the argument is NULL, start where we left off last time
  if (str == NULL)
    str = last_ptr;
  if (str == NULL)
    return NULL;

  // look for the next separator or end of string
  int index = 0;
  while (str[index] != separator && str[index] != '\0')
    index++;

  // if we are at the end of the string, set last_ptr to NULL, otherwise
  // set it to the position of the separator + 1
  if (str[index] == separator)
    last_ptr = str + index + 1;
  else
    last_ptr = NULL;

  // replace the separator with a NULL pointer
  char old = str[index];
  str[index] = '\0';
  char *result = xstrdup(str);
  str[index] = old;

  return result;
}

/******************************************************************************
 *
 * almost the same thing as strtok, except if there is more than one
 * separator in a row, it will return empty strings...
 *
 */

char *fstrtok(char *str, char separator) {
  static char *last_ptr = NULL;

  if (str == NULL)
    str = last_ptr;
  if (str == NULL)
    return NULL;
  last_ptr = str;

  // update last_ptr
  while ((*last_ptr != '\0') && (*last_ptr != separator))
    last_ptr++;
  if (*last_ptr != '\0') {
    *last_ptr = '\0';
    last_ptr++;
  }

  return str;
}

void underscore_string(char *str)
// ---------------------------------------------------------------------------
// replace all bad characters with underscores
// bad characters are: all spaces, slashes smaller than 32 and bigger than 127
// ---------------------------------------------------------------------------
{
  for (size_t i = 0; i < strlen(str); i++) {
    unsigned char c = *(unsigned char *)(&str[i]);
    if (c == ' ')
      str[i] = '_';
    else if (c == '/')
      str[i] = '_';
    else if (c < 32 || c > 127)
      str[i] = '_';
  }
}

std::string validateObjectName(const std::string &s)
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
{
  static const std::string illegalChars = " /<>:\"\\|?*";
  std::string res = s;
  for (size_t i = 0; i < s.size(); i++) {
    unsigned char c = (unsigned char)(s[i]);
    // anything not in range 33-126
    if (c < 33 || c > 126)
      c = '_';
    // spaces, slashes and reserved windows(tm) characters
    else if (illegalChars.find_first_of(c) != std::string::npos)
      c = '_';
    res[i] = c;
  }
  return res;
}
