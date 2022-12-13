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



#include "fset.h"
#include "func.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <list>
#include <unistd.h>

using namespace std;

FSet::FSet(Gallery *pGal, string filename)
    : Set(pGal, filename), version(NONE) {
  load();
}

FSet::~FSet() {}

void FSet::cleanup() { delete_recursive(tmpDir.c_str()); }

void FSet::load() {
  ifstream in(filename.c_str());

  string buffer;

  // get the gallery version
  {
    in >> buffer >> ws;
    if (buffer != string("funcgalleryver"))
      throw FileReadExc("Unknown function gallery format");

    int major, minor;
    in >> major >> minor >> ws;
    if ((major == 1) && (minor == 1))
      version = v1_1;
    else
      throw FileReadExc("Unknown function gallery format");
  }

  tmpDir = pGal->getTmpDir();
  const char* tempdir = tmpDir.c_str();

  {
    // get the number of items
    string buffer;
    int items;
    in >> buffer >> items >> ws;

    // copy out the functions to individual temporary files
    ofstream out;
    bool newfunc = true, readingpoints = false;

    while (!in.eof()) {
      in >> ws;
      getline(in, buffer);
      in >> ws;

      if (isalpha(buffer[0])) {
        if (readingpoints)
          newfunc = true;
        readingpoints = false;
      } else {
        readingpoints = true;
      }

      if (newfunc) {
        newfunc = false;
        if (out.is_open())
          out.close();

        char *tempname = new char[22];
        strcpy(tempname, tempdir);
        strcpy(tempname + 10, "/gal.XXXXXX");
        mkstemp(tempname);

        string outfilename = tempname;
        outfilename += ".func";
        delete[] tempname;

        out.open(outfilename.c_str());
        if (out.fail()){
          std::cerr << "Fail opening "<<outfilename << std::endl;
	  return;
	}
	
	filenames.push_back(outfilename);
      }

      out << buffer << std::endl;
    }
    out.close();

    for (list<string>::iterator i = filenames.begin(); i != filenames.end();
         i++){
      FuncItem *pItem = new FuncItem(pGal,*i, pGal);
      pGal->addItem(pItem);
    }

    //pGal->loadFile(*i, this, false);
  }
}

void FSet::saveCurrentSet() { //pGal->saveFuncSet(filename);
  std::cerr<<"SaveCurrent Set not yet implemented"<<std::endl;
}
