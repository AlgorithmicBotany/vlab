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



#ifndef __utilities_h__
#define __utilities_h__

#include <string>

void remove_trailing_slashes(char *name);
char *getBaseName(char *fname);
std::string getBaseName2(const std::string &str);
void get_base_name(const char *fname, char *result);
char *getPath(char *fileName);
void lowerCase(char *str);
int isSubstring(char *str1, char *str2);
int strptrcmp(const void *s1, const void *s2);
int strptrcasecmp(const void *s1, const void *s2);
int strptrnumcmp(const void *s1, const void *s2);
long getFileLength(const char *name);
int cpfile(const char *ifile, const char *ofile);
int cmpfiles(const char *file1, const char *file2);
char *extract_line(char *&ptr, long &size);

#endif /* #ifdef __utilities_h__ */
