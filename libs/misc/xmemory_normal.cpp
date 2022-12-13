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



#include <algorithm>
#include <assert.h>
#include <cstdlib>
#include <list>
#include <stdio.h>

#include "xmemory.h"

/******************************************************************************
 *
 * my malloc routine with error handling
 */

void *xmalloc(long int n) {
  if (n < 0) {
    fprintf(stderr, "xmalloc( %ld):invalid request\n", n);
    exit(-1);
  }

  void *mem;

  if (n == 0)
    mem = NULL;
  else {
    mem = (void *)malloc(n);

    if (mem == NULL) {
      fprintf(stderr, "xmalloc( %ld): Run out of memory\n", n);
      exit(-1);
    }
  }

  return (mem);
}

/******************************************************************************
 *
 * my realloc routine with error handling
 */

void *xrealloc(void *ptr, long int n) {
  if (n < 0) {
    fprintf(stderr, "xrealloc( %ld):invalid request\n", n);
    exit(-1);
  }

  void *mem;

  if (n == 0) {
    mem = NULL;
  } else {

    if (ptr == NULL)
      mem = malloc(n);
    else
      mem = (void *)realloc(ptr, n);

    if (mem == NULL) {
      fprintf(stderr, "xrealloc( %ld): Run out of memory\n", n);
      exit(-1);
    }
  }

  return (mem);
}

/******************************************************************************
 *
 * my xfree will keep track of the total memory used...
 */

void xfree(void *ptr) {
  if (ptr)
    free(ptr);
}
