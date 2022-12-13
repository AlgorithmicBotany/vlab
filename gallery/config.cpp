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



#include <cstdlib>
#include <climits>

#include <fstream>
#include <iostream>
#include <QString>
#include <platform.h>

#include "config.h"

using namespace std;

int Config::vsize = 20;
int Config::hsize = 50;
int Config::drag_red = 0;
int Config::drag_green = 0;
int Config::drag_blue = 0;
int Config::item_width = 158;
int Config::margins = 10;

void Config::readConfigFile(string fileName) {
  ifstream in(fileName.c_str());
  if (!in || !in.good() || in.eof()){
    std::cerr << "Unable to open config file, using default configuration"
	      << std::endl;
    return;
  }

  while (in && in.good() && !in.eof()) {
    string line;

    in >> ws >> line >> ws;
    if (line.empty())
      continue;

    if (line == string("VSize")) {
      in >> vsize >> ws;
    } else if (line == string("HSize")) {
      in >> hsize >> ws;
    } else if (line == string("DragColor")) {
      in >> drag_red >> drag_green >> drag_blue >> ws;
    } else if (line == string("ItemWidth")) {
      in >> item_width >> ws;
     } else if (line == string("Margins")) {
      in >> margins >> ws;
    } else {
      cerr << "Configuration error. Could not interpret the line: " << line
           << endl;
    }
  }
}

void Config::readConfigFile() {
  QString userConfigDir = "";
#ifdef __APPLE__
  userConfigDir = Vlab::getUserConfigDir(false);
#else
#endif
  
  char bf[PATH_MAX + 1];
  const char *cdir = userConfigDir.toStdString().c_str();
  if (NULL == cdir)
    return;
  else {
    strcpy(bf, cdir);
    strcat(bf, "/");
  }
  strcat(bf, "gallery.cfg");
  /*
  const char *p = getenv("VLABCONFIGDIR");
  if (p) {
    string configfile(p);
    configfile += "/gallery.cfg";
  */
  readConfigFile(bf);
    //}
}
