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



// UNIX includes
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

// VLAB includes
#include "debug.h"
#include "rename_object.h"
#include "uuid.h"
#include "xmemory.h"
#include "xstring.h"

/******************************************************************************
 *
 *  renames an object in oofs database and appropriately modifies .dbase
 *
 *  Input:
 *
 *       oofs_dir   - root object of the oofs database
 *       src_fname  - full name of the object
 *	 dst_fname  - full name of the renamed object
 *
 *	 * notice that oofs_dir HAS to be at the beginning of both src_fname
 *	   and dst_fname
 *
 *  Output:
 *
 *         0 indicates success, otherwise failure. Error codes:
 *
 *	   1: wrong arguments, i.e. oofs_dir is not at the beginning of
 *	      src_fname or dst_fname
 *
 *	   2: rename() failed
 *
 ****************************************************************************/

int rename_object(const char *oofs_dir, const char *src_fname,
                  const char *dst_fname) {
  debug_printf("rename_object( %s, %s, %s)\n", oofs_dir, src_fname, dst_fname);

  // check for preconditions (i.e. valid arguments:
  //      make sure that oofs_dir is at the beginning of both src and new
  int res = 0;
  if (!(xstrlen(src_fname) > xstrlen(oofs_dir)))
    res += 1;
  if (!(xstrlen(dst_fname) > xstrlen(oofs_dir)))
    res += 2;
  if (!(strncmp(oofs_dir, src_fname, xstrlen(oofs_dir)) == 0))
    res += 4;
  if (!(strncmp(oofs_dir, dst_fname, xstrlen(oofs_dir)) == 0))
    res += 8;
  if (res) {
    // report failure
    return 1;
  }

  // try to rename the object
  res = rename(src_fname, dst_fname);
  if (res) {
    // operation failed - report error
    return 2;
  }

  // figure out relative paths of source and destination
  int pos = xstrlen(oofs_dir) - 1;
  while (oofs_dir[pos] != '/')
    pos--;
  char src_rel[4096];
  strcpy(src_rel, src_fname + pos + 1);
  char dst_rel[4096];
  strcpy(dst_rel, dst_fname + pos + 1);

  debug_printf("object_rename(): src_rel = '%s'\n", src_rel);
  debug_printf("object_rename(): dst_rel = '%s'\n", dst_rel);

  // fix UUIDS
  uuidTableReconcile(oofs_dir, dst_fname, true, true);

  // report success
  return 0;
}
