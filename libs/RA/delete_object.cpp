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



#include "delete_object.h"
#include "debug.h"
#include "delete_recursive.h"
#include "uuid.h"
#include "xmemory.h"
#include "xstring.h"
#include <stdio.h>
#include <stdlib.h>

/******************************************************************************
 *
 *  deletes an object in oofs database and appropriately modifies .dbase
 *
 *  Input:
 *
 *       oofs_dir   - root object of the oofs database
 *       fname  - full name of the object
 *
 *	 * notice that oofs_dir HAS to be at the beginning of fname
 *
 *  Output:
 *
 *         0 indicates success, otherwise failure. Error codes:
 *
 *	   1: wrong arguments, i.e. oofs_dir is not at the beginning of
 *	      src_fname or dst_fname
 *
 *	   2: delete() failed
 *
 *	   3: .dabase could not be initialized (most likely it could not
 *	      be read in)
 *
 *	   4: .dbase could not be modified (most likely write permissions
 *	      were not set)
 *
 ****************************************************************************/

int delete_object(const char *oofs_dir, const char *fname) {
  debug_printf("delete_object( %s, %s)\n", oofs_dir, fname);

  // check for preconditions (i.e. valid arguments:
  //      make sure that oofs_dir is at the beginning of both src and new
  int res = 0;
  if (!(xstrlen(fname) > xstrlen(oofs_dir)))
    res += 1;
  if (!(strncmp(oofs_dir, fname, xstrlen(oofs_dir)) == 0))
    res += 2;
  if (res) {
    // report failure
    return 1;
  }

  // delete the object from the filesystem
  res = delete_recursive(fname);
  if (res) {
    // operation failed - report error
    return 2;
  }

  // reconcile uuid table
  uuidTableReconcile(oofs_dir, fname, true, true);

  // report success
  return 0;
}
