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




#include <stdio.h>
#include <stdlib.h>
#include "getObject.h"

/******************************************************************************
 *
 * invoke an object
 *
 */

void getObject( char * object_parameters)
{
    char command[ 4096];
    char tmpStr[ 4096];
    char * objectBin;

    /* create the command line for object */
    objectBin = getenv( "VLABOBJECTBIN");
    if( objectBin == NULL)
    {
	fprintf( stderr, "getObject.c: The environment variable "
		 "VLABOBJECTBIN is not set.\nUsing default 'object'.\n");
	objectBin = (char*)"object";
	sprintf( tmpStr, "VLABOBJECTBIN=%s", objectBin);
	putenv( tmpStr);
    }
    
    sprintf( command, "%s %s &",
	     objectBin,
	     object_parameters);


    /* execute the line, end exit */
    system( command);
}
