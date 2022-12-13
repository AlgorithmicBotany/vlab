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



#include <stdio.h>
#include <stdlib.h>

#include "hash.h"
#include "xmemory.h"
#include "xstring.h"

/******************************************************************************
 *
 * this function will create a hash table
 *
 * n = number of slots in the hash table (should be ~1.4 * number of
 *                                        anticipated items)
 */

HashTable *hash_new(unsigned long n,
                    unsigned long int (*hashCompute)(void *data),
                    int (*hashCompare)(void *data1, void *data2),
                    void (*dataDestroy)(void *data)) {
  HashTable *t;
  long unsigned int i;

  if (n < 1)
    return NULL; /* doesn't make sense to create
                  * tables of size 0 */

  t = (HashTable *)xmalloc(sizeof(HashTable));

  t->hashCompute = hashCompute;
  t->hashCompare = hashCompare;
  t->dataDestroy = dataDestroy;
  t->size = n;
  t->table = (H_LINK **)xmalloc(n * sizeof(H_LINK *));

  for (i = 0; i < n; i++)
    t->table[i] = (H_LINK *)NULL;

  return t;
}

/******************************************************************************
 *
 * this will find out whether 'data' is in the hash table or not
 *
 */

int hash_exist(HashTable *t, void *data) {
  unsigned long int hValue;
  H_LINK *ptr;

  hValue = t->hashCompute(data) % t->size;

  ptr = t->table[hValue];
  while (ptr != NULL) {
    if (t->hashCompare(ptr->data, data))
      return (1 == 1); /* hit */
    else
      ptr = ptr->next;
  }

  return (0 == 1); /* does not exist */
}

/******************************************************************************
 *
 * first it will check if the data is in the table. If it is, it will
 * return False, otherwise the data is inserted into the table
 * and the return value is True. Remember that only the pointer to the
 * data is actually stored, so the user should not change the data.
 *
 */

int hash_add(HashTable *t, void *data) {
  unsigned long int hValue;
  H_LINK *ptr;

  hValue = t->hashCompute(data) % t->size;

  ptr = t->table[hValue];
  while (ptr != NULL) {
    if (t->hashCompare(ptr->data, data))
      return (1 == 0);
    else
      ptr = ptr->next;
  }

  /* the data is not in the table yet, so create another
   * item and insert it into the table (at the beginning of
   * the linked list)*/

  ptr = (H_LINK *)xmalloc(sizeof(H_LINK));
  ptr->data = data;
  ptr->next = t->table[hValue];
  t->table[hValue] = ptr;

  t->count += 1; /* one more items in the table */

  return (1 == 1);
}

/******************************************************************************
 *
 * destroy the hash table
 *
 */

void hash_destroy(HashTable *t) {
  unsigned long i;
  H_LINK *ptr;
  H_LINK *nextPtr;

  for (i = 0; i < t->size; i++) {
    ptr = t->table[i];
    while (ptr != NULL) {
      nextPtr = ptr->next;
      t->dataDestroy(ptr->data);
      xfree(ptr);
      ptr = nextPtr;
    }
  }

  xfree(t);
}

/******************************************************************************
 *
 * print all the elements in the hash table
 *
 */

void hash_print(HashTable *t) {
  H_LINK *ptr;
  long unsigned int i;

  for (i = 0; i < t->size; i++) {
    printf("[%4ld]\n ", i);
    ptr = t->table[i];
    while (ptr != NULL) {
      printf("        %s\n", (char *)ptr->data);
      ptr = ptr->next;
    }
  }
}

/******************************************************************************
 *
 * this function computes a hash value of a string
 *
 */

long unsigned int strHash(void *ptr) {
  unsigned int i;
  char *str = (char *)ptr;

  long unsigned int hValue;

  hValue = 12345;

  for (i = 0; i < xstrlen(str); i++)
    hValue = (hValue << 7) + (hValue >> 25) + 54321 + str[i];

  return hValue;
}

/******************************************************************************
 *
 * convenience function for comparing two strings
 *
 */

int strCompare(void *str1, void *str2) {

  return (xstrcmp((char *)str1, (char *)str2) == 0);
}

/******************************************************************************
 *
 * convenience function for destroying a string
 *
 */

void strDestroy(void *str) { xfree((char *)str); }
