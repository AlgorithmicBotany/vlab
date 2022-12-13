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



#ifndef __Stab_h
#define __Stab_h

/*
 * Size of the hash table for the symbol table entry.
 */
#define HASH_TABLE_SIZE 211
#define EOS '\0'

typedef enum _segtype { Other, Id } SegType;

typedef struct _replist {
  SegType type; /* Type of segment:  { Other, Id }. */
  char *data;   /* Holds the value of the list type. */
  char *value;  /* Holds the value to replace with. */
  struct _replist *next;
} RepList;

typedef struct _stab_el {
  char *Id;

  int Number_of_repsegs;
  RepList *OrigRep;

  int Number_of_params;
  RepList *Parameters;

  int Number_of_broken;
  RepList *BrokenRep;

  struct _stab_el *Next;
} StabEl;

typedef struct _hashtable {
  StabEl *HashTable;
} HashTable;
#endif
