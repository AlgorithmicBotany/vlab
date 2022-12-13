#include <stdio.h>
#include <stdlib.h>

#include "get_field.h"
#include "xmemory.h"
#include "ctype.h"

/******************************************************************************
 *
 * read in an entry from file specified by 'fp'
 *
 * entry is defined as:
 *
 *    starting at the first non-blank character (after comments)
 *    ending with a ':' (colon)
 *
 * returns: the entry (caller frees it with xfree())
 *          if the result is NULL, EOF occured before a valid entry could
 *             be read in
 *
 */

char *get_field(FILE *fp) {
  int size_alloc = 1024;
  int size = 0;
  char *result = (char *)xmalloc(size_alloc);
  if (result == NULL) {
    fprintf(stderr, "raserver:get_field(): not enough memory.\n");
    return NULL;
  }

  // first skip all blanks and comments
  int c;
  while (1) {
    c = fgetc(fp);
    if (c == EOF) // if EOF, no more entries
    {
      xfree(result);
      return NULL;
    }

    else if (c == '#') // if comment, read until '\n'
    {
      while (1) {
        c = fgetc(fp);
        if (c == '\n')
          break;
        if (c == EOF) {
          xfree(result);
          return NULL;
        }
      }
    }

    else if (!isspace(c)) // if not a blank, entry begins here
      break;
  }

  // now read the entry until we find a colon
  while (c != ':') {
    if (size_alloc - 1 == size) {
      // need more memory for the result
      size_alloc *= 2;
      result = (char *)xrealloc(result, size_alloc);
      if (result == NULL) {
        fprintf(stderr, "raserver:get_field():2: not enough memory.\n");
        return NULL;
      }
    }

    // put the character into the result
    result[size] = c;
    size += 1;

    // read in another character
    c = fgetc(fp);
  } // while (c != ':')

  // now we have the result (valid)
  result[size] = '\0';

  return result;
}
