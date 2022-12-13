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
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "Archive.h"
#include "debug.h"
#include "paste_object.h"
#include "prototype_object.h"
#include "utilities.h"
#include "uuid.h"
#include "xmemory.h"
#include "xstring.h"

/******************************************************************************
 *
 *  paste an object in oofs database and appropriately modify .dbase
 *
 *  Input:
 *
 *        oofs_dir   - root directory of the database
 *        dst_fname  - location of the directory where to paste the data
 *        archive_name - name of the file holding the archive data
 *        old_path   - the original location of the objects (needed if
 *                     move_links is set)
 *        move_links - this determines how UUIDs are handled. If set, the
 *                     pasted, the objecsts that are pasted will keep their
 *UUIDs and the originals will have their UUIDs wiped. If unset, the reverse
 *will happen.
 *
 *  Output:
 *                   0 = success
 *                  -1 = write access denied
 *                  -2 = object already exists
 *                  -4 = cannot open archive
 *                  -5 = raserver has resources (or other) problems
 *                  -6 = bad arguments (oofs_dir must be at the beginning
 *                       of both dst_fname and old_path)
 *
 ****************************************************************************/

int paste_object(const char *oofs_dir, const char *dst_fname,
                 const char *archive_name, const char *old_path,
                 char move_links) {
  debug_printf("paste_object( oofs_dir     = %s\n,"
               "              dst_fname    = %s\n,"
               "              archive_name = %s,\n"
               "              old_path     = %s,\n"
               "              move_links   = %s)\n",
               oofs_dir, dst_fname, archive_name, old_path,
               move_links ? "YES" : "NO");

  // check for preconditions (i.e. valid arguments:
  //      make sure that oofs_dir is at the beginning of both dst_fname
  //      and old_path
  {
    int res = 0;

    char tmp[4096];
    sprintf(tmp, "%s/", oofs_dir);
    if (strcmp(oofs_dir, dst_fname))
      if (strlen(tmp) > strlen(dst_fname) ||
          strncmp(tmp, dst_fname, strlen(tmp)) != 0)
        res = 1;
    if (move_links)
      if (strcmp(oofs_dir, old_path))
        if (strlen(tmp) > strlen(old_path) ||
            strncmp(tmp, old_path, strlen(tmp)) != 0)
          res = 1;
    if (res) {
      // report failure - bad arguments
      fprintf(stderr,
              "paste_object(): bad arguments.\n"
              "     oofs_dir  = %s\n"
              "     dst_fname = %s\n"
              "     old_path  = %s\n"
              "\n"
              "     oofs_dir must be at the beginning of dst_fname and\n"
              "     old_path but it is not!!!\n",
              oofs_dir, dst_fname, old_path);
      return -6;
    }
  }

  // make sure that the destination object has an 'ext' directory
  char dst[4096];
  sprintf(dst, "%s/ext", dst_fname);
  if (access(dst, F_OK))
    if (mkdir(dst, 0755)) {
      debug_printf("paste_object(): cannot create an ext directory for\n"
                   "                object %s.\n"
                   "                Reason: %s.\n",
                   dst_fname, strerror(errno));
      return -1; // write denied
    }

  // figure out object's basename
  char base_name[4096];
  sprintf(base_name, "%s", getBaseName((char *)old_path));

  // figure out the new object's name
  char obj_name[4096];
  sprintf(obj_name, "%s/ext/%s", dst_fname, base_name);

  // if such object already exists, return an error
  if (access(obj_name, F_OK) == 0) {
    debug_printf("paste_object(): object already exists (%s)\n", obj_name);
    return -2;
  }

  // dearchive the archive
  Archive ar;
  if (ar.unpack(archive_name, dst)) {
    // dearchive failed - report error
    fprintf(stderr,
            "paste_object(): archive could not be unpacked.\n"
            "    reason: %s\n",
            ar.get_errors());
    return -1;
  }

  // prototype the object
  prototype_object(obj_name);

  if (move_links) {
    // if move links is requested, we reconcile so that the priority is given
    // the the UUIDs that start with the path of the pasted subtree. The result
    // of this is that the UUIDs in the old tree will be deleted, and the UUIDs
    // in the pasted tree will be inserted into the table.
    uuidTableReconcile(oofs_dir, dst_fname, true, false);
  } else {
    // if move links was not requested, we reconcile with priority
    // given to the UUIDs already in the table. This will result in
    // UUIDs being removed from the pasted objects because the existing ones
    // are still valid.
    uuidTableReconcile(oofs_dir, dst_fname, true, true);
  }

  // report success
  debug_printf("paste_object(): success.\n");
  return 0;
}
