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



#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef SUNOS4
extern "C" {
char *rindex(const char *s, int c);
int strncasecmp(const char *s1, const char *s2, size_t n);
int strcasecmp(const char *s1, const char *s2);
}
#else
#include <strings.h>
#endif

#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "utilities.h"
#include "xmemory.h"
#include "xstring.h"

using namespace std;

/******************************************************************************
 *
 * this will extract the path from a file name ( by skipping all the
 * trailing slashes, and then skipping all the non-slashes, and then
 * skipping all the slashes again)
 *
 */

char *getPath(char *fileName) {
  int end;
  int i;
  char *result;

  printf("Extracting the path from %s\n", fileName);

  end = xstrlen(fileName) - 1;
  while ((fileName[end] == '/') && (end > 0))
    end--;
  while ((fileName[end] != '/') && (end > 0))
    end--;
  while ((fileName[end] == '/') && (end > 0))
    end--;
  result = (char *)xmalloc(sizeof(char) * (end + 1));
  for (i = 0; i <= end; i++)
    result[i] = fileName[i];
  result[i] = '\0';

  printf("Extracted: %s\n", result);
  return (result);
}

/******************************************************************************
 *
 * given a complete path name ('/foo/bar/weenie.gif'), returns just the
 * 'simple' name ('weenie.gif').  Note that it does not make a copy of
 * the name, so don't be modifying it...
 *
 */

char *getBaseName(char *fname) {
  char *basname;

  basname = rindex(fname, '/');
  if (!basname)
    basname = fname;
  else
    basname++;

  return basname;
}

std::string getBaseName2(const std::string &fname) {
  size_t t = fname.rfind('/');
  if (t == std::string::npos)
    return fname;
  else
    return std::string(fname, t + 1, fname.length());
}

void get_base_name(const char *fname, char *result) {
  const char *ptr = rindex(fname, '/');
  if (ptr == NULL)
    ptr = (char *)fname;
  else
    ptr++;

  memcpy(result, ptr, xstrlen(ptr) + 1);
}

/******************************************************************************
 *
 * removes the trailing /'s from the end of the string (except if the slash
 * happens to be the only character in the string, in which case it will
 * leave it there) by replacing all the trailing slashes with \0
 *
 */

void remove_trailing_slashes(char *name) {
  int end;

  end = xstrlen(name) - 1;
  while ((end > 0) && (name[end] == '/'))
    name[end--] = '\0';
}

/******************************************************************************
 *
 * convert the string into lower case
 *
 */

void lowerCase(char *str) {
  while (*str != '\0') {
    *str = tolower(*str);
    str++;
  }
}

/******************************************************************************
 *
 * returns True if str1 is substring of str2, and false otherwise
 *
 */

int isSubstring(char *str1, char *str2) {
  int i;
  int len1;
  int len2;

  len1 = xstrlen(str1);
  len2 = xstrlen(str2);

  for (i = 0; i <= len2 - len1; i++)
    if (strncasecmp(str1, str2 + i, len1) == 0)
      return 1;

  return 0;
}

/******************************************************************************
 *
 * string comparison routines good for qsort
 *
 */

int strptrcmp(const void *s1, const void *s2) {
  return xstrcmp(*((char **)s1), *((char **)s2));
}

int strptrcasecmp(const void *s1, const void *s2) {
  return strcasecmp(*((char **)s1), *((char **)s2));
}

int strptrnumcmp(const void *s1, const void *s2) {
  int n1, n2;

  sscanf(*((char **)s1), "%d", &n1);
  sscanf(*((char **)s2), "%d", &n2);
  if (n1 == n2)
    return 0;
  if (n1 < n2)
    return -1;
  else
    return 1;
}

/******************************************************************************
 *
 * will find out the length of the file
 *
 */

long getFileLength(const char *name) {

  struct stat buf;

  if (0 != stat(name, &buf))
    return -1;

  return (long)buf.st_size;
}

/******************************************************************************
 * cpfile(ifile, ofile)
 *	- copies file ifile1 into ofile
 *      - returns 0 = succes 1 = failure
 */

int cpfile(const char *ifile, const char *ofile) {
  int f1, f2;
  unsigned char buffer[16384];
  int n;

  f1 = open(ifile, O_RDONLY);
  if (f1 == -1)
    return 1;

  struct stat results;

  stat(ifile, &results);

  /* in case the output file is a link, unlink it, otherwise
   * the open() would destroy the file this link points to */
  unlink(ofile);

  if (results.st_mode &
      S_IXUSR) // we assume if ifile is in x mode, it is also in rw
    f2 = open(ofile, O_WRONLY | O_CREAT | O_TRUNC,
              S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP |
                  S_IROTH | S_IWOTH | S_IXOTH);
  else if (results.st_mode &
           S_IWUSR) // else we assume if ifile is in w mode, it is also in r
    f2 = open(ofile, O_WRONLY | O_CREAT | O_TRUNC,
              S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
  else
    f2 = open(ofile, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IRGRP | S_IROTH);

  if (f2 == -1) {
    close(f1);
    return 1;
  }

  while ((n = read(f1, buffer, 16384)) > 0) {
    if (n != write(f2, buffer, n)) {
      perror("write()");
      close(f1);
      close(f2);
      return 1;
    }
  }
  close(f1);
  close(f2);
  return 0;
}

/******************************************************************************
 *
 * cmpfiles(char * file1, char * file2)
 *      - compares two files
 *      - returns:
 *		0 = same
 *	       >0 = different
 *             <0 = error
 */

int cmpfiles(const char *file1, const char *file2) {
  int f1;
  int f2;
  unsigned char buffer1[16384];
  unsigned char buffer2[16384];
  long n1;
  long n2;
  int c1;
  int c2;
  long index1;
  long index2;

  f1 = open(file1, O_RDONLY);
  if (f1 == -1) {
    return -1;
  }
  f2 = open(file2, O_RDONLY);
  if (f2 == -1) {
    close(f1);
    return -2;
  }

  n1 = 0;
  n2 = 0;
  index1 = 0;
  index2 = 0;
  do {
    if (n1 == index1) {
      n1 = read(f1, buffer1, 16384);
      if (n1 == -1) {
        close(f1);
        close(f2);
        return 1;
      }
      index1 = 0;
    }
    if (n2 == index2) {
      n2 = read(f2, buffer2, 16384);
      if (n2 == -1) {
        close(f1);
        close(f2);
        return 2;
      }
      index2 = 0;
    }

    if (n1 == 0)
      c1 = -1;
    else
      c1 = buffer1[index1++];
    if (n2 == 0)
      c2 = -1;
    else
      c2 = buffer2[index2++];

    if (c1 != c2) {
      close(f1);
      close(f2);
      return 3;
    }

    if ((c1 == -1) || (c2 == -1)) {
      close(f1);
      close(f2);
      return 0;
    }
  } while (c1 == c2);

  return 0;
}

/******************************************************************************
 *
 * will extract a string from memory pointed to by 'ptr', where the size
 * of the memory is in 'size', and the string is ended by '\n'. Then the
 * pointer is updated to point exactly after the new character, and
 * size is also updated. The string is terminated by '\0', and '\n' is
 * ignored.
 *
 * In case of an error (i.e. size = 0), NULL is returned, and ptr and size
 * are left unchanged.
 *
 * Returns: the string (can be freed using xfree())
 *
 */

char *extract_line(char *&ptr, long &size) {
  if (size <= 0) {
    return NULL;
  }

  // determine the length of the string
  long len = 0;
  while (len < size) {
    if (ptr[len] == '\n')
      break;
    len++;
  }

  // allocate memory for the result
  char *result = (char *)xmalloc(len + 1);
  result[len] = '\0';
  memcpy(result, ptr, len);

  // update ptr & size
  if (size <= len) {
    // we are at the end of the data - so there was no '\n'
    ptr += len;
    size = 0;
  } else {
    // account for the '\n' at the end of the string
    ptr += len + 1;
    size -= len + 1;
  }

  return result;
}
