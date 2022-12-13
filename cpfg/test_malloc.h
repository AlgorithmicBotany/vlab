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



#ifndef __TEST_MALLOC_H__
#define __TEST_MALLOC_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef TEST_MALLOC

void *test_malloc(size_t size, const char *file, int line);
void *test_realloc(void *ptr, size_t size, const char *file, int line);

void test_free(void **ptr, const char *file, int line);
char *test_strdup(const char *s1, const char *file, int line);

void test_malloc_evaluate(void);

#define Malloc(size) test_malloc(size, __FILE__, __LINE__)
#define Realloc(ptr, size) test_realloc(ptr, size, __FILE__, __LINE__)
#define Free(ptr) test_free(&(ptr), __FILE__, __LINE__)
#define Strdup(s1) test_strdup(s1, __FILE__, __LINE__)

#else

#define Malloc(size) malloc(size)
#define Realloc(ptr, size) realloc(ptr, size)
#define Free(ptr) free(ptr)
#ifdef WIN32
#define Strdup(s1) _strdup(s1)
#else
#define Strdup(s1) strdup(s1)
#endif

#endif

#ifdef __cplusplus
}
#endif

#else
#error File already included
#endif
