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




#include <sys/param.h>
#include "Stab.h"

#ifndef __Files_h
#define __Files_h
typedef struct _Stack_El {
  int LineNo;                   /* Current line number. */
  char inFile[MAXPATHLEN];      /* Input filename. */
  FILE *in_fp;                  /* Input stream. */
  YY_BUFFER_STATE yy_buf_state; /* Lex buffer. */

  HashTable **Stab; /* Symbol table for lexer. */

  struct _Stack_El *Next;
} Stack_El;

typedef struct _FileStack {
  int Number_of_files; /* Number of files on stack. */
  Stack_El *Head;      /* Pointer to stack. */
} FileStack;
#endif
