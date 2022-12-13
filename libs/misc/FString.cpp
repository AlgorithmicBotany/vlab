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



#include "FString.h"
#include "xmemory.h"
#include "xstring.h"
#include <stdio.h>

FString::FString(const char *ptr) {
  data = strdup(ptr);
  xmstring_prepared = False;
}

FString::~FString() {
  if (data != NULL)
    xfree(data);
  if (xmstring_prepared)
    XmStringFree(xmstring);
}

XmString FString::get_xmstring(void) {
  if (xmstring_prepared)
    return xmstring;

  xmstring = XmStringCreateLocalized(data);
  xmstring_prepared = True;
  return xmstring;
}

void FString::set(const char *ptr) {
  if (xmstring_prepared)
    XmStringFree(xmstring);
  xmstring_prepared = False;
  xfree(data);
  data = strdup(ptr);
}
