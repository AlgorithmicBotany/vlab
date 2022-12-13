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



#ifndef __Decl_h
#define __Decl_h
/* Defined in scanline.l. */
void Output(char *string);
void Outputnnl(char *string);
char *GetUnFname(char *);
void OutputFilePosLine(Stack_El *SE, FILE *out_fp);
int FileStackSize(FileStack *FS);
void FileStackInit(FileStack *FS, char *Filename);
void FileStackExit(FileStack *FS);
void FileStackPush(FileStack *FS, char *Filename);
void FileStackPushInputBuffer(FileStack *FS, char *buffer);
void FileStackPop(FileStack *FS);
void WriteDefine(StabEl *stab);
void ErrorHandler(int ERRORTYPE, char *string);

#endif
