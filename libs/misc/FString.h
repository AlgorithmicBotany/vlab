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



#ifndef __FSTRING_H__
#define __FSTRING_H__

#include <Xm/Xm.h>

class FString {

public:
  /* constructor */ FString(const char *);
  /* destructor */ ~FString();
  XmString get_xmstring(void);
  void set(const char *);

private:
  char *data;
  Bool xmstring_prepared;
  XmString xmstring;
};

#endif
