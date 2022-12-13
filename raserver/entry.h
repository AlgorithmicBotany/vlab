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



#ifndef __ENTRY_H__
#define __ENTRY_H__

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "xmemory.h"
#include "xstring.h"

#ifdef VLAB_LINUX
// this is needed to use 'crypt()' function
#include <unistd.h>
#endif

class Entry {
public:
  char *login;
  char *password;
  char read_only;
  char valid;

  Entry(void) : login(NULL), password(NULL), read_only(0), valid(0) {}
  Entry(const char *l, const char *p, char read_only) {
    login = xstrdup(l);
    password = xstrdup(p);
    this->read_only = read_only;
    valid = 1;
  }

  ~Entry() {
    if (valid) {
      xfree(login);
      xfree(password);
    }
  }

  int read(FILE *fp) {
    valid = 0;
    // get a line from the file
    char buff[4096];
    char *ptr = fgets(buff, sizeof(buff), fp);
    if (ptr == NULL)
      return -1;
    if (strlen(buff) == 0)
      return -1;

    // get rid of the last '\n'
    if (buff[strlen(buff) - 1] == '\n')
      buff[strlen(buff) - 1] = '\0';

    // get login
    login = fstrtok(buff, ':');
    if (login == NULL)
      return -1;
    // get password
    password = fstrtok(NULL, ':');
    if (password == NULL)
      return -1;
    // get write permissions
    char *tmp = fstrtok(NULL, ':');
    if (tmp == NULL) {
      // this is an old password file, consider the user to have
      // no write permissions
      read_only = 1;
    } else {
      if (tmp[0] == 'y')
        read_only = 0;
      else
        read_only = 1;
    }

    // allocate memory for the fields
    login = xstrdup(login);
    password = xstrdup(password);
    valid = 1;

    return 0;
  }

  int write(FILE *fp) {
    assert(valid);
    if (login != NULL && password != NULL) {
      if (0 >
          fprintf(fp, "%s:%s:%s:\n", login, password, read_only ? "n" : "y")) {
        return -1;
      }
      return 0;
    }

    return -1;
  }
};

#endif
