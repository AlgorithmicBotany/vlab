#ifndef __TEST_MALLOC_H__
#define __TEST_MALLOC_H__

#ifdef TEST_MALLOC

void *test_malloc(size_t size, char *descr);
void *test_realloc(void *ptr, size_t size, char *descr);
void test_free(void *ptr,  char *descr);
char *test_strdup(const char *s1, char *descr);

void test_malloc_evaluate(void);

#define Malloc test_malloc
#define Realloc test_realloc
#define Free test_free
#define Strdup test_strdup

#else

#define Malloc(size, descr) malloc(size)
#define Realloc(ptr, size, descr) realloc(ptr, size)
#define Free(ptr, descr) free(ptr)
#ifdef WIN32
#define Strdup(s1, descr) _strdup(s1)
#else
#define Strdup(s1, descr) strdup(s1)
#endif

#endif

#else
  #error File already included
#endif

