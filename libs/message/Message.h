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




#ifndef __MESAGE_H__
#define __MESAGE_H__

#include <sys/types.h>
#include "Mem.h"

typedef long Code;

class Message
{
public:

    // --- methods --

    /* constructor */      Message ();
    /* constructor */      Message ( Code c,
				     const char * msg,
				     long len);
    /* constructor */      Message ( Code c,
				     const Mem & mem );

    /* destructor */      ~Message ();

    // --- variables ---

    Code                        code; // code of this message
    char *                      data; // the message
    long                      length; // length of the data

private:

};


#endif
