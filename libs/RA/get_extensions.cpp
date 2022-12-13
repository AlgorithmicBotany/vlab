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



#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "dirList.h"
#include "get_extensions.h"
#include "xmemory.h"
#include "xstring.h"

#ifdef VLAB_MACX
// according to OSX manual page for scandir, the filter takes a non-const
// pointer (true only for version 10.7 and under)
static int selectFiles(const dirent *e)
#else
// on linux, the pointer is const... go figure
static int selectFiles(const dirent *e)
#endif
{
  if (e->d_name[0] == '.')
    return 0;
  if (strcmp(e->d_name, "CVS") == 0)
    return 0;
  //    if( strcmp( e-> d_name, ".") == 0)
  //        return 0;
  //    if( strcmp( e-> d_name, "..") == 0)
  //        return 0;

  return 1;
}

/******************************************************************************
 *
 * return a list of extensions for an object (with their attributes),
 * and their number
 *
 * The format for each entry:
 *
 *    - name \0
 *    - real path \0
 *    - flag: (bitmapped)
 *           01 = is a link
 *           02 = is expandable
 */

int get_extensions(const char *obj_name, char **&list) {
  struct dirent **entry = NULL;
  int nFiles;

  // get the number of files into 'nFiles' and the entries into 'entry'
  {
    char dirname[4096];
    sprintf(dirname, "%s/ext", obj_name);
    nFiles = scandir(dirname, &entry, selectFiles, NULL);
  }
  if (nFiles <= 0)
    return nFiles;

  // prepare a result
  list = (char **)xmalloc(sizeof(char *) * nFiles);
  for (int i = 0; i < nFiles; i++) {
    // create a full name
    char fname[4096];
    sprintf(fname, "%s/ext/%s", obj_name, entry[i]->d_name);

    // determine the real path of this object into real_path
    char real_path[4096];
    {
      if (NULL == realpath(fname, real_path))
        strcpy(real_path, fname);
    }

    // calculate the flag
    char flag = 0;
    {
      // is this file a link?
      {
        struct stat buf;
        if (0 == lstat(fname, &buf))
          if (S_ISLNK(buf.st_mode))
            flag |= 0x01;
      }

      // is this directory expandable?
      {
        char buf[4096];
        sprintf(buf, "%s/ext/%s/ext/.", obj_name, entry[i]->d_name);
        if (0 < dirList(buf, NULL))
          flag |= 0x02;
      }
    }

    // allocate memory for this entry
    int len1 = xstrlen(entry[i]->d_name) + 1;
    int len2 = xstrlen(real_path) + 1;
    list[i] = (char *)xmalloc(len1 + len2 + 1);
    memcpy(list[i], entry[i]->d_name, len1);
    memcpy(list[i] + len1, real_path, len2);
    memcpy(list[i] + len1 + len2, &flag, 1);

    if (entry[i]) {
      free(entry[i]);
      entry[i] = NULL;
    }
  }
  if (entry) {
    free(entry);
    entry = NULL;
  }

  return nFiles;
}
