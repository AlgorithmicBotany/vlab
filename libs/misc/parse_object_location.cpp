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



#include "parse_object_location.h"

#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "xmemory.h"
#include "xstring.h"

using namespace std;

// constructor for ObjectLocation
ObjectLocation::ObjectLocation() { _valid = false; }
ObjectLocation::ObjectLocation(const string &ploc) {
  // remove trailing slash from the location
  string loc = ploc;
  if (ploc.size() > 0) {
    if (loc[ploc.size() - 1] == '/')
      loc = loc.substr(0, ploc.size() - 1);
  }

  // make sure there is at most one ':'
  size_t colonPos = loc.find_first_of(':');
  if (colonPos != loc.find_last_of(':')) {
    _valid = false;
    return;
  }
  // make sure there is at most one '@'
  size_t atPos = loc.find_first_of('@');
  if (atPos != loc.find_last_of('@')) {
    _valid = false;
    return;
  }
  // make sure that if there is '@', there is also ':'
  if (atPos != loc.npos && colonPos == loc.npos) {
    _valid = false;
    return;
  }
  // make sure that '@' is always before ':'
  if (atPos != loc.npos && colonPos != loc.npos && atPos > colonPos) {
    _valid = false;
    return;
  }
  // three cases to handle:
  // 1.) neither ':' or '@' is given
  if (atPos == loc.npos && colonPos == loc.npos) {
    _path = loc;
  }
  // 2.) only ':' is given
  else if (atPos == loc.npos) {
    _hostname = loc.substr(0, colonPos);
    _path = loc.substr(colonPos + 1);
  }
  // 3.) both '@' and ':' are given
  else {
    _username = loc.substr(0, atPos);
    _hostname = loc.substr(atPos + 1, colonPos - atPos - 1);
    _path = loc.substr(colonPos + 1);
  }
  // assign default username
  if (_username.empty()) {
    struct passwd *pw = getpwuid(getuid());
    if (pw != 0)
      _username = pw->pw_name;
  }
  if (_hostname.empty()) {
    _hostname = LOCAL_HOST_STR;
  }
  _valid = true;
}

/******************************************************************************
 *
 * (c) Pavol Federl - June 13 1996
 *
 * will parse the object location and extract:
 *
 * - user_name   (default = getpwuid())
 * - host_name   (default = "localhost")
 * - object_name (default = NULL)
 *
 */

void parse_object_location(const char *str, char **login_name_ptr,
                           char **host_name_ptr, char **object_name_ptr) {
  // get the default login_name (from the password file)
  char *login_name = xstrdup("undefined_vlab_user");
  struct passwd *pswd = getpwuid(getuid());
  if (pswd != NULL) {
    xfree(login_name);
    login_name = xstrdup(pswd->pw_name);
  }

  // LOCAL_HOST_STR is going to be the default computer
  char *host_name = xstrdup(LOCAL_HOST_STR.c_str());

  // the object name will be NULL
  char *object_name = NULL;

  // first find out where first '@', ':' and '/' are
  int l = xstrlen(str);
  int at_pos = l + 1;
  int col_pos = l + 1;
  int slash_pos = l + 1;

  // go from the right and search for all the above characters
  int i;
  for (i = l; i >= 0; i--) {
    if (str[i] == '@')
      at_pos = i;
    else if (str[i] == ':')
      col_pos = i;
    else if (str[i] == '/')
      slash_pos = i;
  }

  // do we have a user name?
  if ((at_pos < col_pos) && (col_pos < slash_pos)) {
    xfree(login_name);
    login_name = (char *)xmalloc(at_pos + 1);
    memcpy(login_name, str, at_pos);
    login_name[at_pos] = '\0';

    str = str + at_pos + 1;
    l -= at_pos + 1;
    col_pos -= at_pos + 1;
    slash_pos -= at_pos + 1;
  }

  // do we have a host name?
  if (col_pos < slash_pos) {
    xfree(host_name);
    host_name = (char *)xmalloc(col_pos + 1);
    memcpy(host_name, str, col_pos);
    host_name[col_pos] = '\0';

    str = str + col_pos + 1;
    l -= col_pos + 1;
    slash_pos -= col_pos + 1;
  }

  // and finaly, copy the object name
  //    object_name = xstrdup( str);
  object_name = strdup(str);

  // if the last character of the object name is '/', change it to '\0'

  while (object_name[0] != '\0' && object_name[strlen(object_name) - 1] == '/')
    object_name[strlen(object_name) - 1] = '\0';

  *login_name_ptr = login_name;
  *host_name_ptr = host_name;
  *object_name_ptr = object_name;
}

void parse_object_location(std::string str, char **login_name_ptr,
                           char **host_name_ptr, char **object_name_ptr) {
  parse_object_location(str.c_str(), login_name_ptr, host_name_ptr,
                        object_name_ptr);
}
