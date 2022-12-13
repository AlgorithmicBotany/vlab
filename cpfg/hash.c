#ifdef WIN32
#include "warningset.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "interpret.h"
#include "generate.h"

#include "test_malloc.h"
#include "log.h"

void MyExit(int status);

#define HASHTABLESIZE 8192

struct turtle_items {
  float line_width;
  float scale_factor;
  int color_index;
  int color_index_back;
  int texture;
};

typedef struct turtle_items turtle_items;

struct hash_item {
  struct hash_item *next_item;
  struct hash_item *previous_item; /* to create a link list in the order
of object specification */
  int depth;
  turtle_items turtle;
  char *data;
  char *name;
  short length;
  short index; /* to differentiate the same modules with different turtle
information */
};

typedef struct hash_item hash_item;

static hash_item *table[HASHTABLESIZE];

hash_item *last_item;
hash_item *first_item;

static hash_item *pass_item;

/***************************************************************************/
void HashTablePassInitialize(void) { pass_item = first_item; }

/***************************************************************************/
int GetHashTableItem(char **string, int *index, int *depth, TURTLE *tu) {
  hash_item *ptr;

  if (pass_item == NULL)
    return 0;

  ptr = pass_item;
  pass_item = pass_item->previous_item;

  tu->line_width = ptr->turtle.line_width;
  tu->scale_factor = ptr->turtle.scale_factor;
  tu->color_index = ptr->turtle.color_index;
  tu->color_index_back = ptr->turtle.color_index_back;
  tu->texture = ptr->turtle.texture;

  *depth = ptr->depth;
  *index = ptr->index;
  *string = ptr->data;

  return ptr->length;
}

/***************************************************************************/
int HashFunction(char *name, int length) {
  int i, res;

  res = name[0];

  for (i = length - 1; i >= 2; i = -2)
    res ^= (name[i] << 6) + name[i - 1];

  return res & (HASHTABLESIZE - 1);
}

/***************************************************************************/
int InitializeHashTable(void) {
  int i;

  for (i = 0; i < HASHTABLESIZE; i++)
    table[i] = NULL;

  first_item = last_item = NULL;
  return 0;
}

/***************************************************************************/
int FreeHashTable(void) {
  int i;
  hash_item *ptr, *ptr2;

  for (i = 0; i < HASHTABLESIZE; i++) {
    ptr = table[i];

    while (ptr != NULL) {
      if (ptr->data != NULL) {
        Free(ptr->data);
        ptr->data = NULL;
      }

      Free(ptr->name);
      ptr->name = NULL;

      ptr2 = ptr;
      ptr = ptr->next_item;
      Free(ptr2);
      ptr2 = NULL;
    }
  }

  InitializeHashTable();
  return 0;
}

/***************************************************************************/
int FindObjectName(char *string, int length, char *name, int depth, TURTLE *tu,
                   char add) {
  int i, o, index;
  hash_item **ptr, *ptr2;
  extern VIEWPARAM viewparam;
  turtle_items turtle;

  turtle.line_width = tu->line_width;
  turtle.scale_factor = tu->scale_factor;
  turtle.color_index = tu->color_index;
  turtle.color_index_back = tu->color_index_back;
  turtle.texture = tu->texture;

  index = 0;

  i = HashFunction(name, (int)(strlen(name)));

  ptr = &table[i];

  while (*ptr != NULL) {
    if ((o = strcmp(name, (*ptr)->name)) == 0) {
      /* string found */

      if (viewparam.objects_include_turtle) {
        /* have to compare the turtle also */

        if (turtle.line_width == (*ptr)->turtle.line_width &&
            turtle.scale_factor == (*ptr)->turtle.scale_factor &&
            turtle.color_index == (*ptr)->turtle.color_index &&
            turtle.color_index_back == (*ptr)->turtle.color_index_back &&
            turtle.texture == (*ptr)->turtle.texture)
          /* even turtles are the same */
          return 1;
        else
          index++;
      } else
        return 1;
    }

    if (o > 0) {
      /* string is bigger than 'name' */
      if (add) {
        /* add the string before the item (*ptr) */
        ptr2 = *ptr;
        if ((*ptr = Malloc(sizeof(hash_item))) == NULL) {
          Message("Not enough memory.\n");
          MyExit(-1);
        }

        (*ptr)->next_item = ptr2;
        (*ptr)->data = string;
        (*ptr)->length = length;
        (*ptr)->name = name;
        (*ptr)->depth = depth;
        (*ptr)->index = index;
        (*ptr)->turtle = turtle;

        if (first_item == NULL) {
          first_item = last_item = *ptr;
        } else {
          last_item->previous_item = *ptr;
          last_item = *ptr;
        }
        last_item->previous_item = NULL;
      }

      return 0;
    }
    ptr = &(*ptr)->next_item;
  }

  if (add) {
    /* add the string to the end */
    if ((*ptr = Malloc(sizeof(hash_item))) == NULL) {
      Message("Not enough memory.\n");
      MyExit(-1);
    }

    (*ptr)->next_item = NULL;
    (*ptr)->data = string;
    (*ptr)->name = name;
    (*ptr)->length = length;
    (*ptr)->depth = depth;
    (*ptr)->index = index;
    (*ptr)->turtle = turtle;

    if (first_item == NULL) {
      first_item = last_item = *ptr;
    } else {
      last_item->previous_item = *ptr;
      last_item = *ptr;
    }
    last_item->previous_item = NULL;
  }

  return 0;
}

/***************************************************************************/
int FindObjectIndex(char *name, TURTLE *tu) {
  int i, o, index;
  hash_item **ptr;
  extern VIEWPARAM viewparam;
  turtle_items turtle;

  turtle.line_width = tu->line_width;
  turtle.scale_factor = tu->scale_factor;
  turtle.color_index = tu->color_index;
  turtle.color_index_back = tu->color_index_back;
  turtle.texture = tu->texture;

  index = 0;

  i = HashFunction(name, (int)(strlen(name)));

  ptr = &table[i];

  while (*ptr != NULL) {
    if ((o = strcmp(name, (*ptr)->name)) == 0) {
      /* string found */

      if (viewparam.objects_include_turtle) {
        /* have to compare the turtle also */

        if (turtle.line_width == (*ptr)->turtle.line_width &&
            turtle.scale_factor == (*ptr)->turtle.scale_factor &&
            turtle.color_index == (*ptr)->turtle.color_index &&
            turtle.color_index_back == (*ptr)->turtle.color_index_back &&
            turtle.texture == (*ptr)->turtle.texture)
          /* even turtles are the same */
          return index;
        else
          index++;
      } else
        return 0;
    }

    if (o > 0) {
      /* string is bigger than 'name' */
      return -1;
    }
    ptr = &(*ptr)->next_item;
  }

  return -1;
}
