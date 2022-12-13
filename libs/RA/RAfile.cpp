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



#include <assert.h>

#include "RAfile.h"
#include "xmemory.h"

/******************************************************************************
 *
 * RA_File constructor
 *
 *   - resets the variables
 *
 */

/* constructor */ RA_File::RA_File(void) {
  local_file_stream = NULL;
  local_file_name = NULL;
  remote_file_name = NULL;
  connection = NULL;
}

/******************************************************************************
 *
 * ~RA_File destructor
 *
 *   - releases memory
 *
 */

/* destructor */ RA_File::~RA_File() {
  if (local_file_name != NULL)
    xfree(local_file_name);
  if (remote_file_name != NULL)
    xfree(remote_file_name);

  assert(local_file_stream == NULL);
}
