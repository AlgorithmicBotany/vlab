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




#ifndef __RA_FILE_H__
#define __RA_FILE_H__

#include <stdio.h>
#include <stdlib.h>

#include "RAconnection.h"

class RA_File
{
    
public:

    // -- methods --

    /* constructor */           RA_File ( void);
    /* destructor */           ~RA_File ();

    // -- variables --

    FILE *            local_file_stream ;
    char *              local_file_name ;
    char *             remote_file_name ;
    RA_Connection *          connection ;
    
private:

};
    

#endif
