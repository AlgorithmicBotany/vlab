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



#ifndef __hash_h__
#define __hash_h__

typedef struct h_link H_LINK;

struct h_link {
  void *data;   /* the actual data */
  H_LINK *next; /* the next link */
};

typedef struct h_table HashTable;

struct h_table {
  /* a function that computes the hash value from the data */
  unsigned long int (*hashCompute)(void *data);
  /* a function that compares two data */
  int (*hashCompare)(void *data1, void *data2);
  /* a function that will destroy the 'data' */
  void (*dataDestroy)(void *data);
  /* the table of pointers to links */
  H_LINK **table;
  /* size of the hash table */
  unsigned long int size;
  /* number of the items in the table */
  unsigned long int count;
};

HashTable *hash_new(unsigned long n,
                    unsigned long int (*hashCompute)(void *data),
                    int (*hashCompare)(void *data1, void *data2),
                    void (*dataDestroy)(void *data));

void hash_destroy(HashTable *t);

int hash_add(HashTable *t, void *data);
int hash_exist(HashTable *t, void *data);
void hash_print(HashTable *t);

/* convenience function */
long unsigned int strHash(void *ptr);
int strCompare(void *str1, void *str2);
void strDestroy(void *str);

#endif
