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



#ifndef __PARSE_OBJECT_LOCATION_H__
#define __PARSE_OBJECT_LOCATION_H__

#include <string>
const std::string LOCAL_HOST_STR = "localhost";

// Class for parsing an object location
//
// Parsing is done in the constructor. It parses a string specifying the
// object's location in the format:
//      [[login@]hostname:]path
// success of parsing should be checked using
//      valid()
// the parts of the object name can then be accessed using
//      username(), hostname() and path()
// if not specified, the defaults are:
//      username() = current unix username
//      localhost() = localhost (LOCAL_HOST_STR)
class ObjectLocation {
public:
  // default constructor - makes an invalid object
  ObjectLocation();
  // constructor - parses from argument
  ObjectLocation(const std::string &loc);
  const std::string &username() const { return _username; }
  const std::string &hostname() const { return _hostname; }
  const std::string &path() const { return _path; }
  std::string &path() { return _path; }
  bool valid() const { return _valid; }

private:
  std::string _username, _hostname, _path;
  bool _valid;
};

// this function offers some of the functionality as the class above.
// But it uses old style c pointers, YUCK!!!
// Please avoid using it. It's here only for the old code that I don't have
// time to change (Pavol)
void parse_object_location(const char *str, char **login_name_ptr,
                           char **host_name_ptr, char **object_name_ptr);
void parse_object_location(std::string str, char **login_name_ptr,
                           char **host_name_ptr, char **object_name_ptr);

#endif
