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



#include "cset.h"
#include "contour.h"
#include <cstdlib>
#include <fstream>
#include <list>
#include <unistd.h>

using namespace std;

CSet::CSet(Gallery *pGal, string filename)
    : Set(pGal, filename), version(NONE) {
  load();
}

CSet::~CSet() {
  // good place to destroy the temp directory?
  // fprintf(stderr,"Delete recursive tempdir = %s\n",tempdir);
  // nope... doesn't always get called
}

void CSet::cleanup() { delete_recursive(tmpDir.c_str()); }

void CSet::load() {
  ifstream in(filename.c_str());

  string buffer;

  // get the gallery version
  {
    in >> buffer >> ws;
    if (buffer != string("contourgalleryver"))
      throw FileReadExc("Unknown contour gallery format");

    int major, minor;
    in >> major >> minor >> ws;
    if ((major == 1) && (minor == 1))
      version = v1_1;
    else{
      std::cerr<<"Unknown contour gallery format"<<std::endl;
      return;
    }
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
      if (isalpha(buffer[0]) || isalpha(buffer[buffer.size() - 1]) ||
          (iscntrl(buffer[buffer.size() - 1]) &&
           isalpha(buffer[buffer.size() - 2]))) {

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
	    //char *tempname = new char[21];
	    //strcpy(tempname, tempdir);
        //strcpy(tempname + 10, "/gal.XXXXXX"); // causes buffer overflow
	unsigned int tempdirLen = (unsigned int)strlen(tempdir);
        char *tempname = new char[tempdirLen+11+1]; // 11 is the length of "/gal.XXXXXX"
        strcpy(tempname, tempdir);
        strcpy(tempname + tempdirLen, "/gal.XXXXXX");
        mkstemp(tempname);

        string outfilename = tempname;
        outfilename += ".con";

        delete[] tempname;
        out.open(outfilename.c_str());
        if (out.fail()) {
          std::cerr << "Fail opening "<<outfilename << std::endl;
	  return;
        }
        filenames.push_back(outfilename);
      }
      out << buffer << endl;
    }
    out.close();

    for (list<string>::iterator i = filenames.begin(); i != filenames.end();
         i++){
      ConItem *pItem = new ConItem(pGal,*i, pGal);
      pGal->addItem(pItem);
    }

    //      pGal->loadFile(*i, this, false);
  }
}

void CSet::saveCurrentSet() { //pGal->saveConSet(filename);
  std::cerr<<"SaveCurrent Set not yet implemented"<<std::endl;
}

