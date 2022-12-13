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



#include <string.h>
#include <sys/dir.h>
#include <sys/types.h>
#include <unistd.h>

#include "dirList.h"
#include "xmemory.h"
#include "xstring.h"

/******************************************************************************
 *
 * find out the entries in a directory  (except '.' and '..')
 *
 * return number of entries  (negative number means error)
 *
 *     -1 = directory could not be opened
 *
 *
 */

int dirList(const char *dirname, char ***dlist) {
  // no enries
  int n_entries = 0;
  if (dlist != NULL)
    *dlist = NULL;

  // open the directory
  DIR *dir = opendir(dirname);
  if (dir == NULL)
    return -1;

  // read the entries one by one
  while (1) {
    // get the next entry from the directory
    direct *entry = readdir(dir);

    // if there are no more entries, we are done
    if (entry == NULL)
      break;

    // skip "." and ".." entries
    if (xstrcmp(entry->d_name, ".") == 0)
      continue;
    if (xstrcmp(entry->d_name, "..") == 0)
      continue;

    n_entries++;
    if (dlist != NULL) {
      // adjust memory for result
      *dlist = (char **)xrealloc(*dlist, sizeof(char *) * n_entries);

      // add the entry at the end of result
      (*dlist)[n_entries - 1] = xstrdup(entry->d_name);
    }
  }

  // close the directory - no longer needed
  closedir(dir);

  // return the number of entries found
  return n_entries;
}

// same as above - but using vectors and strings
std::vector<std::string> dirList2(const std::string &dirName) {
  std::vector<std::string> result;

  // open the directory
  DIR *dir = opendir(dirName.c_str());
  if (dir == NULL)
    return result;

  // read the entries one by one
  while (1) {
    // get the next entry from the directory
    direct *entry = readdir(dir);

    // if there are no more entries, we are done
    if (entry == NULL)
      break;

    // skip "." and ".." entries
    if (strcmp(entry->d_name, ".") == 0)
      continue;
    if (strcmp(entry->d_name, "..") == 0)
      continue;

    result.push_back(entry->d_name);
  }

  // close the directory - no longer needed
  closedir(dir);

  // return the number of entries found
  return result;
}
