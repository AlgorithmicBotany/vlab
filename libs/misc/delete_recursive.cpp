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



#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "debug.h"
#include "delete_recursive.h"
#include "dirList.h"
#include "xmemory.h"

//**************************************************************************
//
// delete fname and if fname is a directory, delete it recursively
//

int delete_recursive(const char *fname) {
  debug_printf("del( %s):\n", fname);

  // first try to apply a simple unlink
  int res = unlink(fname);

  // if unlink succeeded, we are done - return success
  if (res == 0)
    return 0;

  // unlink failed, let's see why
  debug_printf("   - unlink: %s\n", strerror(errno));

  // fname is a directory - delete all of its entries first
  if (errno == EPERM || errno == EISDIR) {
    // get all entries in the directory
    char **list;
    int n_list = dirList(fname, &list);
    if (n_list == -1) {
      debug_printf("   - dirList says no items could be read\n");
      return 1;
    }

    // go through all entries and delete them
    char full_name[4096];
    res = 0;
    for (long i = 0; i < n_list; i++) {
      sprintf(full_name, "%s/%s", fname, list[i]);
      res |= delete_recursive(full_name);
      xfree(list[i]);
      list[i] = NULL;
    }
    if (list)
      xfree(list);

    if (res) {
      debug_printf("   - failed to delete all entries.\n");
      return 2;
    }

    // now try to rmdir fname
    res = rmdir(fname);
    if (res) {
      debug_printf("   - failed to remove the directory\n");
      return 3;
    }

    // the directory is removed, continue with success
    return 0;
  }

  // return an error
  debug_printf("   - failed to remove the file\n");

  return 4;
}
