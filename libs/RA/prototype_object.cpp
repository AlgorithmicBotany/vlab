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

#include "dirList.h"
#include "prototype_object.h"
#include "utilities.h"
#include "xmemory.h"

static int _prototype_object(const char *object_path) {
  // change the current directory to the object
  if (chdir(object_path)) {
    fprintf(stderr,
            "prototype_object(%s):chdir() failed\n"
            "    -because: %s\n",
            object_path, strerror(errno));
    return -2;
  }

  // get the list of files in the object's directory
  char **list;
  int n_files = dirList(".", &list);

  // go through all the entries one by one and prototype them
  int result = 0;
  for (int i = 0; i < n_files; i++) {
    // skip .id file (in case we are stil using old vlab client..)
    if (strcmp(".id", list[i]) == 0)
      continue;
    // and skip uuid files
    if (strcmp(".uuid", list[i]) == 0)
      continue;

    // call stat(2) on this entry
    struct stat st;
    if (stat(list[i], &st)) {
      fprintf(stderr,
              "prototype_object(%s):stat(%s) failed\n"
              "    -because: %s\n",
              object_path, list[i], strerror(errno));
      result = -1;
      break;
    }
    // skip all non-regular files
    if (!S_ISREG(st.st_mode)) {
      continue;
    }

    //
    // --- compare the entry to the entry two levels up ---
    //
    char buff[4096];
    sprintf(buff, "../../%s", list[i]);
    // if files differ, goto the next file
    if (cmpfiles(buff, list[i])) {
      continue;
    }

    // delete the current entry
    if (unlink(list[i])) {
      fprintf(stderr,
              "prototype_object(%s):unlink(%s) failed\n"
              "    -because: %s\n",
              object_path, list[i], strerror(errno));
      result = -1;
      break;
    }

    // create a symbolic link
    if (symlink(buff, list[i])) {
      fprintf(stderr,
              "prototype_object(%s):symlink(%s) failed\n"
              "    -because: %s\n",
              object_path, list[i], strerror(errno));
      result = -1;
      break;
    }
  }

  // free the list of files
  {
    for (long i = 0; i < n_files; i++)
      xfree(list[i]);
    xfree(list);
  }

  // return the result
  return result;
}

int prototype_object(const char *object_path) {
  // remember the current path
  char old_path[4096];
  if (getcwd(old_path, sizeof(old_path)) == NULL) {
    fprintf(stderr, "prototype_object(%s):getcwd() failed: %s\n", object_path,
            strerror(errno));
    return -1;
  }

  int result = _prototype_object(object_path);

  // change to the old path
  if (chdir(old_path)) {
    fprintf(stderr,
            "prototype_object(%s):cannot chdir() back to %s.\n"
            "    -because: %s\n",
            object_path, old_path, strerror(errno));
    return -1;
  }

  return result;
}
