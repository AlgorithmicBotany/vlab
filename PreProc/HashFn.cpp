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
#include <string.h>
#include "Stab.h"
#include "ErrorHandler.h"

StabEl *AddListEntry(HashTable *head, char *id);
StabEl *NewListEntry(char *id);
StabEl *LookUpListEntry(HashTable *head, char *id);
void DeleteListEntry(HashTable *head, char *id);
void DeleteFromHashTable(HashTable **table, char *id);
void DeleteList(HashTable *head);
void PrintList(HashTable *head);
void FreeStabEl(StabEl *elem);
void AddtoEndofRepList(RepList *list, char *data, SegType type);

/****************************************************************************
 * Hash function
 * =============
 * Thanks to PJ Weinberger for the algorithm hashpjw.
 *
 * Taken from page 436 of _Compilers:_Principles,_Techniques_and_Tools_ by
 * A. Aho, R. Sethi and J. Ullman.
 ****************************************************************************/
int hashpjw(char *identifier) {
  char *p;
  unsigned h = 0, g = 0;

  for (p = identifier; *p != EOS; p++) {
    h = (h << 4) + (*p);
    // modified by Pavol Federl to get rid of a compiler warning
    g = h & 0xf0000000;
    if (g) {
      h = h ^ (g >> 24);
      h = h ^ g;
    }
  }

  return (h % HASH_TABLE_SIZE);
}

HashTable **CreateHashTable(void) {
  HashTable **table;
  int count;

  table = (HashTable **)calloc(HASH_TABLE_SIZE, sizeof(HashTable *));
  if (table == NULL) {
    perror("Calloc(CreateHashTable)");
    exit(-1);
  }

  for (count = 0; count < HASH_TABLE_SIZE; count++) {
    table[count] = (HashTable *)calloc(1, sizeof(HashTable));
    if (table[count] == NULL) {
      perror("Calloc(CreateHashTable)");
      exit(-1);
    }
  }

  return (table);
}

StabEl *AddToHashTable(HashTable **table, char *id) {
  return ((StabEl *)AddListEntry(table[hashpjw(id)], id));
}

StabEl *LookUpHashTable(HashTable **table, char *id) {
  return ((StabEl *)LookUpListEntry(table[hashpjw(id)], id));
}

void DeleteFromHashTable(HashTable **table, char *id) {
  DeleteListEntry(table[hashpjw(id)], id);
}

void PrintHashTable(HashTable **table) {
  int count = 0;

  for (count = 0; count < HASH_TABLE_SIZE; count++)
    if (table[count]->HashTable != NULL) {
      printf("Level %d: ", count);
      PrintList(table[count]);
    }
}

void DeleteHashTable(HashTable **table) {
  int count = 0;

  for (count = 0; count < HASH_TABLE_SIZE; count++) {
    DeleteList(table[count]);
    free(table[count]);
  }

  free(table);
}

StabEl *NewListEntry(char *id) {
  StabEl *newEntry;

  if ((newEntry = (StabEl *)calloc(1, sizeof(StabEl))) == NULL) {
    perror("Calloc(NewListEntry)");
    exit(-1);
  }

  newEntry->Id = (char *)strdup(id);
  return (newEntry);
}

StabEl *AddListEntry(HashTable *head, char *id) {
  StabEl *newEntry;

  /* Check for already defined identifier. */
  if (LookUpListEntry(head, id) != NULL)
    ErrorHandler(ERROR_MULTI_ID, id);

  newEntry = NewListEntry(id);

  /* Attach new entry to the front of list.*/
  newEntry->Next = head->HashTable;
  head->HashTable = newEntry;

  return (newEntry);
}

StabEl *LookUpListEntry(HashTable *head, char *id) {
  StabEl *stabTemp = head->HashTable;

  /* Traverse list and return StabEl if found. */
  while (stabTemp != NULL) {
    if (strcmp(stabTemp->Id, id))
      stabTemp = stabTemp->Next;
    else
      return (stabTemp);
  }

  return (stabTemp); /* Return NULL if not found. */
}

void DeleteListEntry(HashTable *head, char *id) {
  StabEl *stabTemp = head->HashTable;
  StabEl *del = NULL;

  if (stabTemp != NULL)
    if (!strcmp(stabTemp->Id, id)) {
      del = head->HashTable;
      head->HashTable = stabTemp->Next;
      FreeStabEl(del);
      return;
    }

  while ((stabTemp != NULL) && (stabTemp->Next != NULL)) {
    if (strcmp(stabTemp->Next->Id, id)) {
      stabTemp = stabTemp->Next;
    } else {
      del = stabTemp->Next;
      stabTemp->Next = del->Next;
      FreeStabEl(del);
      return;
    }
  }
}
void DeleteList(HashTable *head) {
  StabEl *temp = head->HashTable;

  while (temp != NULL) {
    head->HashTable = temp->Next;
    FreeStabEl(temp);
    temp = head->HashTable;
  }
}

void PrintList(HashTable *head) {
  StabEl *temp = head->HashTable;

  while (temp != NULL) {
    printf(" |%s| ", temp->Id);
    temp = temp->Next;
  }

  printf("\n");
}

void FreeStabEl(StabEl *elem) {
  RepList *list;
  RepList *temp;

  if (elem == NULL)
    return;
  if (elem->Id != NULL)
    free(elem->Id);

  list = elem->Parameters;
  while (list != NULL) {
    temp = list;
    list = temp->next;
    free(temp->data);
    free(temp);
  }

  list = elem->BrokenRep;
  while (list != NULL) {
    temp = list;
    list = temp->next;
    free(temp->data);
    free(temp);
  }

  list = elem->OrigRep;
  while (list != NULL) {
    temp = list;
    list = temp->next;
    free(temp->data);
    free(temp);
  }
}

void AddParameter(StabEl *stab, char *data) {
  stab->Number_of_params++;
  if (stab->Parameters == NULL) {
    stab->Parameters = (RepList *)calloc(1, sizeof(RepList));
    if (stab->Parameters == NULL) {
      perror("Calloc(AddParameter)");
      exit(-1);
    }
    stab->Parameters->data = (char *)strdup(data);
  } else {
    AddtoEndofRepList(stab->Parameters, data, Other);
  }
}

void AddReplacement(StabEl *stab, char *data, SegType type) {
  stab->Number_of_repsegs++;
  if (stab->OrigRep == NULL) {
    stab->OrigRep = (RepList *)calloc(1, sizeof(RepList));
    if (stab->OrigRep == NULL) {
      perror("Calloc(AddReplacement)");
      exit(-1);
    }
    stab->OrigRep->data = (char *)strdup(data);
    stab->OrigRep->type = type;
  } else {
    AddtoEndofRepList(stab->OrigRep, data, type);
  }
}

void AddtoEndofRepList(RepList *list, char *data, SegType type) {
  RepList *temp = list;

  if (temp != NULL)
    while (temp->next != NULL)
      temp = temp->next;

  if ((temp->next = (RepList *)calloc(1, sizeof(RepList))) == NULL) {
    perror("Calloc(AddtoEndofRepList)");
    exit(-1);
  }
  temp->next->data = (char *)strdup(data);
  temp->next->type = type;
}

void PrintRepList(RepList *list) {
  RepList *temp = list;

  while (temp != NULL) {

#if DEBUG < 1
    printf("%s", temp->data);
#else
    printf("\tReplist:  \"%s\"\n", temp->data);
#endif

    temp = temp->next;
  }
}
