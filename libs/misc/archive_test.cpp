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



#include "Archive.h"
#include "xstring.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// prototypes
static void usage(const char *pname, int exit_code);

int main(int argc, char **argv) {
  int recursive = 1;
  char dir_name[4096];
  char archive_name[4096];

  if (argc < 2)
    usage(argv[0], -1);
  if (strncmp(argv[1], "-h", 2) == 0)
    usage(argv[0], 0);

  if (xstrcmp("c", argv[1]) == 0) {
    if (argc < 4)
      usage(argv[0], -1);
    if (xstrcmp(argv[2], "-nr") == 0) {
      recursive = 1;
      if (argc < 5)
        usage(argv[0], -1);

      strcpy(archive_name, argv[3]);
      strcpy(dir_name, argv[4]);
    } else {
      strcpy(archive_name, argv[2]);
      strcpy(dir_name, argv[3]);
    }

    // create the archive
    fprintf(stderr, "Packing %s%s into %s\n",
            recursive ? "" : "(non-recursively) ", dir_name, archive_name);

    Archive ar;
    ar.set_destination(Archive::AR_FILE, archive_name);
    if (ar.pack(dir_name, recursive))
      fprintf(stderr, "\nArchive failed. Errors:\n\n%s", ar.get_errors());
    else
      fprintf(stderr, "Done.\n");
  } else if (xstrcmp("l", argv[1]) == 0) {
    if (argc < 3)
      usage(argv[0], -1);

    strcpy(archive_name, argv[2]);

    fprintf(stderr, "Listing archive %s:\n\n", archive_name);

    Archive ar;
    if (ar.list(archive_name))
      fprintf(stderr, "\nArchive failed. Errors:\n\n%s", ar.get_errors());
    else
      fprintf(stderr, "\nDone.\n");
  } else if (xstrcmp("x", argv[1]) == 0) {
    if (argc < 4)
      usage(argv[1], -1);

    strcpy(archive_name, argv[2]);
    strcpy(dir_name, argv[3]);

    fprintf(stderr, "Extracting archive %s into %s.\n", archive_name, dir_name);

    Archive ar;
    int res = ar.unpack(archive_name, dir_name);
    if (res)
      fprintf(stderr, "\nArchive failed. Errors:\n\n%s", ar.get_errors());
    else
      fprintf(stderr, "\nSuccess.\n");
  } else {
    usage(argv[0], -1);
  }

  return 0;
}

static void usage(const char *pname, int exit_code) {
  /*-------------------------------------------------------------.
  | archive c [-nr] archive_name dir_name                        |
  | archive l archive_name                                       |
  | archive x archive_name dir_name                              |
  `-------------------------------------------------------------*/

  fprintf(stderr, "Usage:  %s c [-nr] archive_name dir_name\n", pname);
  fprintf(stderr, "        %s l archive_name\n", pname);
  fprintf(stderr, "        %s x archive_name dir_name\n", pname);
  fprintf(stderr, "        %s -h[elp]\n", pname);

  exit(exit_code);
}
