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



#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "RA.h"
#include "xmemory.h"

int main(int argc, char **argv) {
  if (argc != 4) {
    fprintf(stderr, "usage: %s host login password\n", argv[0]);
    exit(-1);
  }

  char *host_name = argv[1];
  char *user_name = argv[2];
  char *password = argv[3];

  char file_name[4096];

  RA_Connection *connection =
      RA::new_connection(host_name, user_name, password);

  if (connection == NULL) {
    RA::Error("main::new_connection()");
    exit(-1);
  }

  while (1) {
    fprintf(stderr, "Menu:\n"
                    "-----\n"
                    "\n"
                    "1.) Read file\n"
                    "2.) Remove file\n"
                    "3.) Symbolic link\n"
                    "4.) Stat file\n"
                    "5.) Get dir\n"
                    "6.) Fetch file\n"
                    "7.) Mkdir\n"
                    "8.) Rmdir\n"
                    "\n"
                    "0.) Quit\n"
                    "\n"
                    "Your choice? ");
    int choice;
    scanf("%d", &choice);

    switch (choice) {
    case 7: // mkdir
      fprintf(stderr, "mkdir: ");
      scanf("%s", file_name);
      if (RA::Mkdir(connection, file_name, 0755))
        fprintf(stderr, "mkdir failed\n");
      else
        fprintf(stderr, "mkdir successful\n");
      break;
    case 8: // rmdir
      fprintf(stderr, "rmdir: ");
      scanf("%s", file_name);
      if (RA::Rmdir(connection, file_name))
        fprintf(stderr, "rmdir failed\n");
      else
        fprintf(stderr, "rmdir successful\n");
      break;
    case 1: // read file
      fprintf(stderr, "File to read: ");
      scanf("%s", file_name);
      RA_File *fp = RA::Fopen(connection, file_name, "r");
      if (fp == NULL) {
        RA::Error("main:Fopen()");
        exit(-1);
      }

      while (1) {
        int c = RA::Fgetc(fp);

        if (c == EOF)
          break;

        printf("%c", c);
      }

      if (RA::Fclose(fp)) {
        RA::Error("main:Fclose()");
        exit(-1);
      }

      break;

    case 5: // get dir
      fprintf(stderr, "Directory: ");
      scanf("%s", file_name);
      char **list;
      int nfiles = RA::Get_dir(connection, file_name, &list);
      if (nfiles < 0) {
        fprintf(stderr, "Could not read directory '%s'\n", file_name);
        break;
      }
      printf("nfiles = %d\n", nfiles);
      for (int i = 0; i < nfiles; i++) {
        printf("  '%s'\n", list[i]);
        xfree(list[i]);
      }
      xfree(list);
      break;

    case 2: // remove file
      fprintf(stderr, "Remove: ");
      scanf("%s", file_name);
      if (0 != RA::Unlink(connection, file_name))
        fprintf(stderr, "File '%s:%s' not removed.\n", connection->host_name,
                file_name);
      else
        fprintf(stderr, "File '%s:%s' removed.\n", connection->host_name,
                file_name);
      break;
    case 3: // symbolic link
      fprintf(stderr, "Symbolic link src: ");
      scanf("%s", file_name);
      fprintf(stderr, "          link to: ");
      char fname2[4096];
      scanf("%s", fname2);
      if (0 != RA::Symlink(connection, file_name, fname2))
        fprintf(stderr, "Could not create link '%s' to '%s'\n", fname2,
                file_name);
      else
        fprintf(stderr, "Link '%s' -> '%s' created.\n", fname2, file_name);
      break;

    case 4: // stat file
      fprintf(stderr, "Get stat on: ");
      scanf("%s", file_name);
      RA_Stat_Struc buff;

      if (RA::Stat(connection, file_name, &buff) != 0) {
        RA::Error("main:Stat()");
        exit(-1);
      }

      if (buff.type == RA_REG_TYPE)
        printf("type      = REGULAR\n");
      else if (buff.type == RA_DIR_TYPE)
        printf("type      = DIRECTORY\n");
      else if (buff.type == RA_OTHER_TYPE)
        printf("type      = OTHER\n");
      else if (buff.type == RA_NOEXIST_TYPE)
        printf("type      = NOEXIST\n");

      if (buff.type != RA_NOEXIST_TYPE) {
        printf("is_link   = %d\n", buff.is_link);
        printf("readable  = %d\n", buff.readable);
        printf("writeable = %d\n", buff.writeable);
        printf("executable= %d\n", buff.executable);
      }

      break;
    case 6: // Fetch File
      fprintf(stderr, "Fetch file: ");
      scanf("%s", file_name);
      if (RA::Fetch_file(connection, file_name, "/var/tmp/testfile")) {
        RA::Error("Could not fetch a file.\n");
        break;
      }
      system("/bin/cat /var/tmp/testfile");
      break;
    }

    if (choice == 0)
      break;
  }

  RA::close_connection(connection);
  printf("\n");
}
