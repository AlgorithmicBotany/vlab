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



#ifndef __HASH_H__
#define __HASH_H__

#ifdef __cplusplus
extern "C" {
#endif

int InitializeHashTable(void);
int FreeHashTable(void);
int FindObjectName(char *string, int length, char *name, int depth, TURTLE *tu,
                   char add);
int FindObjectIndex(char *name, TURTLE *tu);

void HashTablePassInitialize(void);
int GetHashTableItem(char **name, int *index, int *depth, TURTLE *tu);

#ifdef __cplusplus
}
#endif

#endif
