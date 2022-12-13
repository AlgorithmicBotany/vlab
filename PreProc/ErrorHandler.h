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



#ifndef __ErrorHandler_h
#define __ErrorHandler_h

void ErrorHandler(int ERRORTYPE, char *string);

#define ERROR_INCLUDE_NOFILE 0
#define ERROR_NO_PARAMS 1
#define ERROR_IDENT_HASPARAMS 2
#define ERROR_IDENT_EXTRAPARAMS 3
#define ERROR_IDENT_TOOFEWPARAMS 4
#define ERROR_MACRO_NEWLINE 5
#define ERROR_OPENINGFILE 6
#define ERROR_MULTI_ID 7

#define ERRORS 8
#endif
