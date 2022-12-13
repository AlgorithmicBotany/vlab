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



/*
 * Labutil:
 *	Special-purpose utilities used by the virtual laboratory.
 *	Some of these routines use the general-purpose utilities
 *	in util.c
 *
 * spsearch(sp, specs)
 *	- searches through the tool spec hierarchy given 'specs'
 *	  of the form 'spec/spec/...'
 *	- begins at sp in the hierarchy
 *	- called recursively for each level
 *	- returns a pointer to the next level, or NULL if not found
 *
 * gtsearch(gtools, line)
 *	- searches an array of generic tools for the name at the
 *	  beginning of line
 *	- returns the array index, or -1 if the name is not found
 *
 * Lynn Mercer
 * October, 1989
 */

#include <stdio.h>
#include <string.h>

#include "labutil.h"

struct spec *spsearch(struct spec *sp, char *specs) {
  char specline[STRLEN];
  char *token, *remainder;
  struct spec *s;

  if (specs == NULL)
    return (sp);
  strcpy(specline, specs);
  token = strtok(specline, "/");
  for (s = sp; s != NULL; s = s->nextspec)
    if (strcmp(s->sname, token) == 0) {
      remainder = strtok(NULL, " ");
      if (remainder == NULL)
        return (s);
      else
        return (spsearch(s->subitems, remainder));
    }
  return (NULL);
}

int gtsearch(struct gtools gtools[], char *line) {
  char gtline[STRLEN];
  char *gtname;
  int t;

  strcpy(gtline, line);
  gtname = strtok(gtline, " \t");
  if (gtname == NULL)
    return -1;
  for (t = 0; gtools[t].tname != NULL; t++)
    if (strcmp(gtools[t].tname, gtname) == 0)
      return (t);
  return -1;
}
