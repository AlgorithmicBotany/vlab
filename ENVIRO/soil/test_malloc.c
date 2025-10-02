
#ifdef WIN32
#include "warningset.h"
#endif

#include <stdio.h>
/* #include <malloc.h> */
#include <stdlib.h>
#include <string.h>

#include "test_malloc.h"
//#include "generate.h"
#include "msg.h"


#define UNUSED    0
#define ALLOCATED 1

static char verbose = 0;

struct memory_item_type
{
  void *ptr;
  size_t size;
  char *descr;
  char flag;
};

typedef struct memory_item_type memory_item_type;

/* array of allocated pieces */
#define MAP_SIZE 1000
static memory_item_type *memory_map = NULL;
static unsigned long map_size = 0;

static unsigned long num_items = 0;

/**************************************************************************/
void test_free(void *ptr, char *descr)
{
  unsigned long i;

  if(verbose)
    printf("free %s\n",descr);

  for(i=0;i<num_items; i++)
    if(memory_map[i].ptr == ptr) 
      if(memory_map[i].flag == UNUSED) 
	  {
	printf("%s - has been freed already by %s!\n", descr,
		memory_map[i].descr);
	return;
      }
      else 
	  {
	memory_map[i].flag = UNUSED;
	memory_map[i].descr = descr;
	/* memory_map[i].ptr = 0; */
	free(ptr);
	return;
      }

  printf("%s - not found (free)!\n", descr);
}

/**************************************************************************/
static void *add_pointer(void *ptr, size_t size, char *descr)
{
  unsigned long i;

  /* store ptr - find the first unused slot */
  for(i=0;i<num_items; i++)
    if(memory_map[i].flag == UNUSED) 
	{
      memory_map[i].flag = ALLOCATED;
      memory_map[i].ptr = ptr;
      memory_map[i].descr = descr;
      memory_map[i].size = size;
      return ptr;
    }

  /* slot not found, increase num_items */

  if(num_items >= map_size) 
  {
    /* reallocate memory_map */
    if(map_size == 0) 
	{
      map_size += MAP_SIZE;
      memory_map = 
	(memory_item_type *)malloc(map_size*sizeof(memory_item_type));
    }
    else 
	{
      map_size += MAP_SIZE;
      memory_map = 
	(memory_item_type *)realloc(memory_map, 
				    map_size*sizeof(memory_item_type));
    }
  
    if(memory_map == NULL) 
	{
      printf("Cannot allocate memory for memory map!\n");
      exit(0);
    }
  }

  memory_map[num_items].flag = ALLOCATED;
  memory_map[num_items].ptr = ptr;
  memory_map[num_items].descr = descr;
  memory_map[num_items].size = size;

  num_items++;

  return ptr;
}

/**************************************************************************/
void *test_malloc(size_t size, char *descr)
{
  void *ptr;

  if(verbose)
    printf("malloc %s\n",descr);

  /* NULL is not stored */
  if((ptr=malloc(size)) == NULL)
    return ptr;

  return add_pointer(ptr, size, descr);
}



/**************************************************************************/
void *test_realloc(void *ptr, size_t size, char *descr)
{
  void *ptr2;
  unsigned long i;

  if(verbose)
    printf("realloc %s\n",descr);

  for(i=0;i<num_items; i++)
    if(memory_map[i].ptr == ptr) 
      if(memory_map[i].flag == UNUSED)
	printf("%s - has been freed already by %s!\n", descr,
		memory_map[i].descr);
      else 
	  {
	ptr2 = realloc(ptr, size);
	memory_map[i].ptr = ptr2;
	memory_map[i].descr = descr;
	memory_map[i].size = size;

	return ptr2;
      }

  printf("%s - not found (realloc)!\n", descr);

  return NULL;
}


/**************************************************************************/  
char *test_strdup(const char *s1, char *descr)
{
  void *ptr;

  if(verbose)
    printf("strdup %s\n",descr);

  /* NULL is not stored */
  if((ptr=(void*)strdup(s1)) == NULL)
    return (char*)ptr;

  return (char*)add_pointer(ptr, strlen((char*)ptr)+1, descr);
}


/**************************************************************************/  
void test_malloc_evaluate(void)
{
  unsigned long i,j;
  int count;
  unsigned long total = 0;
  FILE *fp=stderr;

#ifdef ORDER_TESTING
  char ok=1;
  fprintf(fp,"\nResults of malloc testing (in order of appearence):\n");

  for(i=0;i<num_items; i++)
    if(memory_map[i].flag == ALLOCATED) 
	{
      fprintf(fp," %s (size %ld)- not freed\n", 
	      memory_map[i].descr, memory_map[i].size);
      ok = 0;
    }
  if(ok)
    fprintf(fp," everything OK.\n");
#endif

  fprintf(fp,"\nResults of malloc testing (counted):\n");
  
  for(i=0;i<num_items; i++)
    if(memory_map[i].flag == ALLOCATED) 
	{
      /* find all other appearances */
      count = 1;
      total += memory_map[i].size;
      
      for(j=i+1;j<num_items; j++)
	if((memory_map[j].flag == ALLOCATED)&&
	   !strcmp(memory_map[i].descr, memory_map[j].descr)) 
	{
	  count++;
	  total += memory_map[j].size;
	  memory_map[j].flag = UNUSED;
	}
      
      fprintf(fp," %s (%d x %d)- not freed\n", 
	      memory_map[i].descr, count, memory_map[i].size);
    }

  if(total>0)
    fprintf(fp,"\nTotal bytes leaked: %ld\n", total);
  else
    fprintf(fp," everything OK.\n");
    
  if(memory_map)
    free(memory_map);
}
