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




#ifndef __QT_SUPPORT_H
#define __QT_SUPPORT_H

#include <QString>

/* This ascii macro is modeled after qPrintable. The only
   difference is that ascii uses toAscii rather than toLocal8Bit

   Note:
      The returned value is a reference to a temporary and thus
      must be used only in the expression in which this macro
      is called.

      This is valid usage:
         printf("%s", ascii(string));

      This is invalid usage:
         char* asciiString = ascii(string);
         printf("%s", asciiString);
*/
#define ascii(string) QString(string).toAscii().constData()
#endif
