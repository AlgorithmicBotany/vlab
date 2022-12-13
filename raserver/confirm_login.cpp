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



#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <unistd.h>

#include "confirm_login.h"
#include "dsprintf.h"
#include "xstring.h"
#include "xmemory.h"
#include "entry.h"
#include "raserver.h"

/******************************************************************************
 *
 * will confirm a login
 *
 *
 * - will check if the given pair 'login' & 'password' are in the database
 *
 * - returns:   0 - success
 *             !0 - error
 */

int confirm_login(const char *login, const char *password, char &read_only) {
  if (raserver_debug)
    fprintf(stderr,
            "confirm_login():\n"
            "\t   login = '%s'\n"
            "\tpassword = '%s'\n",
            login, password);

  char *pfile = dsprintf("%s/rapasswords", getenv("VLABCONFIGDIR"));

  // encrypt the passoword
  char *enc_password = crypt(password, SALT);

  if (raserver_debug)
    fprintf(stderr, "Encrypted password = %s\n", enc_password);

  // open password file for reading
  FILE *fp = fopen(pfile, "r");
  if (fp == NULL) {
    fprintf(stderr, "Could not open password file '%s'!\n", pfile);
    fprintf(stderr, "Please fix the problem and restart.\n");
    return -1;
  }

  // in this loop read in the entries one by one trying to find a
  // matching login & password
  int success = 0;
  while (1) {
    // read in an entry from the password file
    Entry e;
    if (e.read(fp))
      break;

    // check if this entry matches requested login
    if (xstrcmp(e.login, login) != 0)
      continue;
    if (xstrcmp(e.password, enc_password) != 0)
      continue;

    // entry matches login and password
    success = 1;
    read_only = e.read_only;
    break;
  }

  // close the password file
  fclose(fp);

  if (raserver_debug)
    fprintf(stderr, "raserver: login '%s' was %s.\n", login,
            success ? "confirmed" : "denied");

  // return the result
  if (success)
    return 0;
  else
    return 1;
}
