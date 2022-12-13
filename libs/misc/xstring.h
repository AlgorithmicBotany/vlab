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



#ifndef __XSTRING_H__
#define __XSTRING_H__

#include <string.h>
#include <string>

size_t xstrlen(const char *s);
int xstrcmp(const char *s1, const char *s2);
char *xstrdup(const char *s);

int xstrpos(const char *s, char c);
char *xmemdup(const char *src, int len);
void xstrappend(char *&str, char c);
char *get_token(char *str, char separator);

char *fstrtok(char *str, char separator);

void underscore_string(char *str);

// Takes an object name (most likely user input) and returns a valid object
// name. Basically this function  replaces all bad characters with underscores.
// Bad characters for VLAB are:
//  <space> vlab does not deal nicely with spaces at this moment
//  <forward slash> cannot be part of a unix filename
//  <>:"/\|?* illegal windows characters
std::string validateObjectName(const std::string &s);

#endif
