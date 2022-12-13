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



// Implementes globMatch (string pattern, string str)
//
// - compares the pattern to a string
//
//  Understands the following patterns:
//
//       *       any substring
//       ?       any single character
//       \x      match x
//       x       match x
//       [a-z]   any single character in the range a-z
//       [^a-z]  any single character not in the range a-z

#include "globMatch.h"

bool globMatch(const std::string &pattern, const std::string &str) {
  size_t plen = pattern.length();
  size_t slen = str.length();

  // trivial case: if pattern string is empty
  if (plen == 0)
    return (slen == 0);

  // setup pointers to beginning of each string
  size_t pp = 0;
  size_t sp = 0;
  // this flag is used to determine between '[' and '[^'
  bool neg;
  bool match;

  // process each character in the pattern
  while (1) {
    // if we matched everyithing in the pattern,
    // the result depends on whether we exhausted the string
    if (pp >= plen)
      return sp >= slen;
    // figure out what the next character in the pattern is
    switch (pattern[pp]) {
    case '\\':
      // unexpected end of string
      if (pp + 1 >= plen)
        return false;
      pp++;
      // if the string buffer is empty, we have no match
      if (sp >= slen)
        return false;
      // otherwiser we compare
      if (pattern[pp] != str[sp])
        return false;
      pp++;
      sp++;
      break;
    case '?':
      // if the string buffer is empty, we have no match
      if (sp >= slen)
        return false;
      pp++;
      sp++;
      break;
    case '*':
      // try to eat 0-all characters and answer recursively
      for (size_t i = 0; i <= slen - sp; i++) {
        if (globMatch(std::string(pattern, pp + 1, plen),
                      std::string(str, sp + i, slen)))
          return true;
      }
      return false;
    case '[':
      neg = false;
      pp++;
      if (pp >= plen)
        return false;
      if (pattern[pp] == '^') {
        neg = true;
        pp++;
      }
      match = false;
      // process all characters
      while (1) {
        // if we are at the end of the pattern, we
        // are done (unexpeced end of string)
        if (pp >= plen) {
          return false;
        }
        // if we find the closing ] we are also done
        if (pattern[pp] == ']') {
          pp++;
          break;
        }
        // otherwise we try the match
        if (pattern[pp] == str[sp])
          match = true;
        // in any case, we advance the pattern pointer
        pp++;
      }
      sp++;
      if (match == neg)
        return false;
      break;
    default:
      // if the string is empty, we have no match
      if (sp >= slen)
        return false;
      // otherwise the two characters must match
      if (pattern[pp] != str[sp])
        return false;
      pp++;
      sp++;
      break;
    }
  }
}
