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



#include "lists_same.h"
#include "xmemory.h"
#include "xstring.h"
#include <stdio.h>
#include <stdlib.h>

static int strptrcmp(const void *p1, const void *p2) {
  return xstrcmp(*(char **)p1, *(char **)p2);
}

/******************************************************************************
 *
 * will compare whether list1 and list2 contain the same strings
 *
 * Returns: 1 = if lists are the same
 *          0 = otherwise
 */

int lists_same(const char **list1, long n1, const char **list2, long n2) {
  // numbers have to be the same
  if (n1 != n2)
    return 0;

  char **t1 = (char **)xmalloc(sizeof(char *) * n1);
  char **t2 = (char **)xmalloc(sizeof(char *) * n1);

  // t1 = copy of list1, t2 = copy of list2
  int i;
  for (i = 0; i < n1; i++) {
    t1[i] = (char *)list1[i];
    t2[i] = (char *)list2[i];
  }

  // sort t1 and t2 alphabeticaly
  qsort(t1, n1, sizeof(char *), strptrcmp);
  qsort(t2, n1, sizeof(char *), strptrcmp);

  // make sure the lists are the same
  int result = 1;
  for (i = 0; i < n1; i++)
    if (xstrcmp(t1[i], t2[i]) != 0) {
      result = 0;
      break;
    }

  if (t1)
    xfree(t1);
  if (t2)
    xfree(t2);

  return result;
}
