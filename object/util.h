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




#ifndef _VLABUTIL_
#define _VLABUTIL_
/*
 * Util:
 *	General utilities used by the virtual lab.
 *	These should be put into a library.
 *
 * getline(fp)
 *	- gets next non-blank line from the file pointed to by fp,
 *	  stores it using strdup(), and returns a pointer to it.
 *
 * getlineNC(fp)
 *	- uses the getline() to obtain a line that does not start
 *        with a ';'
 *
 * strdupn(s, n)
 * 	- stores n chars from buf in a permanent location,
 *	  and returns a pointer to it
 *	- uses the string function strdup()
 *
 * strcpyc(s1, s2, c)
 *	- looks for the first occurrence of c in s2, and
 *	  copies all chars preceding it from s2 to s1
 *	- puts a NULL char at the end of s1
 *	- if c is not in s2, all of s2 is copies to s1
 *	- uses the string functions strchr() and strncpy()
 *
 * strrcpyc(s1, s2, c)
 *	- looks for the last occurrence of c in s2, and
 *	  copies all chars preceding it from s2 to s1
 *	- puts a NULL char at the end of s1
 *	- if c is not in s2, returns NULL, else s1
 *	- uses the string functions strrchr() and strncpy()
 *
 * ntabs(line)
 *	- counts the number of tab chars at the beginning of line
 *
 * small_sleep:
 *	- wait delay/100 of a second 
 *	- unfortunately this is a "busy	while waiting" implementation.
 *
 * Lynn Mercer
 * October,1989 - August,1990
 */

/* function prototypes */
char  * getline(FILE *fp);
char  * getlineNC( FILE * fp);
char  * strdupn(char *s, int n);
char  * strcpyc(char *s1, char *s2, char c);
char  * strrcpyc(char *s1, char *s2, char c);
int 	ntabs(char line[]);
void 	small_sleep(int delay);
int 	get_time(void);
void	movecursor(long x, long y);
void 	hourglass(void);

#endif
