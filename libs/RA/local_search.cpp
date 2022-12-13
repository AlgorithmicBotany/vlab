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



#include "local_search.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <signal.h>
#include <stdlib.h>
#include <string>

struct Search {
  std::ifstream _stdProcess;
  FILE *pFile;
  bool is_open;
  fpos_t pos;
  std::string oofs;
  std::string start_path;
  std::string pattern;
  bool caseSensitive;
  bool exactMatch;
};

static Search &search() {
  static Search _search;
  return _search;
}

std::string upper_string(const std::string &str) {
  std::string upper;
  std::transform(str.begin(), str.end(), std::back_inserter(upper), toupper);
  return upper;
}

// checks if pattern is found in the object name of the path
bool patternMatch(const std::string &pattern, const std::string &path,
                  bool caseSensitive, bool exactMatch) {
  std::string objName = path.substr(path.find_last_of('/') + 1);
  size_t found;
  if (exactMatch) {
    found = objName.compare(pattern);
    if (found == 0)
      return true;
    return false;
  }
  if (caseSensitive) {
    found = objName.find(pattern);
    if (found != std::string::npos)
      return true;
    return false;
  } else {
    found = upper_string(objName).find(upper_string(pattern));
    if (found != std::string::npos)
      return true;
    return false;
  }
}

// checks if the path is a valid object path wrt to oofs
static bool is_valid_object_path(const std::string &path,
                                 const std::string &oofs) {
  // special case, path == oofs is valide
  if (path == oofs)
    return true;
  //  make sure path start with oofs
  if (((path + "/").find(oofs + "/")) != 0)
    return false;
  std::string rel = path.substr(oofs.length() + 1);
  // split by slashes
  // check...
  size_t pos = rel.find("/");
  int cpt = 0;
  while (pos != std::string::npos) {
    std::string str = rel.substr(0, pos);
    rel = rel.substr(pos + 1);
    if (cpt % 2 == 0)
      if (str.compare("ext"))
        return false;
    pos = rel.find("/");
    cpt++;
  }
  if (cpt % 2 == 0)
    return false;
   // // ok, looks valid enough :)
  return true;
}

void searchBegin(const std::string &oofs, const std::string &start_path,
                 const std::string &pattern, bool caseSensitive,
                 bool exactMatch) {
  Search &s = search();
  // kill running process
  //    searchEnd();
  // save the parameters in case we need them later
  s.oofs = oofs;
  s.start_path = start_path;
  s.pattern = pattern;
  s.caseSensitive = caseSensitive;
  s.exactMatch = exactMatch;
   std::string commandLine =
      "find " + start_path + " -type d -print > /tmp/find-results.tmp";
  if (s.is_open) {
    fclose(s.pFile);
    s.is_open = false;
  }
  remove("/tmp/find-results.tmp");
  signal(SIGCHLD, SIG_DFL);
  system(commandLine.c_str());
  s.pFile = fopen("/tmp/find-results.tmp", "r");
  s.is_open = true;
  if (s.pFile == NULL)
    std::cerr << "Error opening file" << std::endl;
  else {
    fgetpos(s.pFile, &s.pos);
  }
}

std::string searchContinue(bool blocking) {
  Search &s = search();
  s._stdProcess.close();

  fsetpos(s.pFile, &s.pos);
  char buffer[2048];

  while (1) {
    fgets(buffer, 2048, s.pFile);
    std::string stdLine;
    stdLine = std::string(buffer);
    if (feof(s.pFile)) {
      fclose(s.pFile);
      s.is_open = false;
      return "*";
    }
    // remove trailing \n
    if (stdLine.find('\n') != std::string::npos)
      stdLine.erase(stdLine.end() - 1, stdLine.end());
    // if this is not a valid object path (i.e. in case a user has put
    // subdirectories in the object...), reject it and try again
    std::string qoofs = s.oofs;

    if (!is_valid_object_path(stdLine, qoofs))
      continue;
    // if this line does not match the patter, get another line
    if (!patternMatch(s.pattern, stdLine, s.caseSensitive, s.exactMatch))
      continue;
    // otherwise return this search result
    //      std::cerr<<stdLine<<std::endl;
    fgetpos(s.pFile, &s.pos);
    return stdLine;
  }

  // at this point the process is still running, but there was not enough
  // input to be read into a line, what to do? Depends on whether we want
  // blocking search or non blocking
  if (!blocking)
    return "**";
  // if we can accept blocking, we will do a 1/2 second wait before we try again
  // s.p().waitForFinished(500);
}

void searchEnd() {
  // kill running process
  Search &s = search();
   if (s.is_open) {
    s.is_open = false;
    fclose(s.pFile);
  }
  remove("/tmp/find-results.tmp");
}
