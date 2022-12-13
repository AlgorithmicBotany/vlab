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



#include "func.h"
#include <cmath>
#include <fstream>
#include <iostream>

using namespace std;
using namespace Qt;


FuncItem::FuncItem(Gallery *pGal, std::string fileName, QWidget *parent,
                   const char *nam, Qt::WindowFlags f)
  :
  
  Item(pGal, fileName, parent, nam, f),  samples(0),
  flip(false), version(NOT_LOADED) {
  filename = fileName;
  name = fileName;

  pEditor = new FuncEditor();
  tmpDir = pGal->getTmpDir();
  _background[0] = 0.0;
  _background[1] = 0.0;
  _background[2] = 0.0;
  _curve[0] = 1.0;
  _curve[1] = 1.0;
  _curve[2] = 0.0;
  _visible = true;

    // load the config options from file
  try {
    Config::readConfigFile();
   } catch (Config::ConfigErrorExc exc) {
    cerr << exc.message() << endl;
  } catch (...) {
    cerr << "Unknown config error occured" << endl;
  }

  //  readConfigFile();
  //Initialize window
  _width = pGal->getItemWidth();
  _height = pGal->getItemHeight();

  load();
  pNameTxt->setText(this->name.c_str());
}

bool FuncItem::load() {
  ifstream in(filename.c_str());
  if (!in || in.eof() || !in.good()) {
    return false;
  }
  string buf;
  in >> buf >> ws;

  if (buf == string("fver")) {
    // A new version of the func file format
    int major = 0, minor = 0;
    in >> major >> minor >> ws;

    if (major == 1 && minor == 1) {
      version = v1_1;
      in >> buf;
      getline(in, name);
      pNameTxt->setText(name.c_str());

      in >> buf >> samples >> ws;
        
      // read info on background image (if exists)
      if (in.peek() == 'b') {
        string name;
        in >> buf >> name >> ws;
        
        float x, y;
        in >> buf >> x >> y >> ws;
      }
  
      // 'markers' are an experimental feature that will be considered for release in future versions
      // check for markers
      //if (in.peek() == 'm') {
      //  int numMarkers;
      //  in >> buf >> numMarkers >> ws;
      //  int id;
      //  float u, x, y;
      //  for (int i = 0; i < numMarkers; i++)
      //    in >> id >> u >> x >> y >> ws;
      //}

      in >> buf >> buf >> ws;
      if (buf.compare(string("on")) == 0)
        flip = true;
      else
        flip = false;
    } else
      return false;
  } else {
    // Original func file format
    version = ORIGINAL;

    float rmin = 0.0, rmax = 0.0;
    in >> rmin >> rmax >> ws;
  }

  int num = 0;
  in >> buf >> num >> ws;

  points = PtVec(num);

  double xmin = 0.0, xmax = 0.0, ymin = 0.0, ymax = 0.0;
  if (num == 0)
    return false;
  for (int i = 0; i < num; i++) {
    std::string coordinate;
    getline(in, coordinate);
    coordinate += " ";
    float x, y;
    int multiplicity = 1;    
    std::string delimiter = " ";
    size_t pos = 0;
    std::string token;
    std::vector<std::string> posVector;
    while ((pos = coordinate.find(delimiter)) != std::string::npos) {
      token = coordinate.substr(0, pos);
      if (token != "")
	posVector.push_back(token);
      coordinate.erase(0, pos + delimiter.length());
    }
    int sizeVec = posVector.size();
    if ((2 > sizeVec )||(3 < sizeVec)) {
      printf("error reading point\n");
      continue;
    }
    x = std::stof(posVector[0]);
    y = std::stof(posVector[1]);
    if (sizeVec == 3)
      multiplicity = std::stof(posVector[2]);
 


    
    
    Point pt;
    if (flip) {
      pt.y = x; pt.x =y; pt.multiplicity = multiplicity;
    } else{
      pt.x = x; pt.y =y; pt.multiplicity = multiplicity;
    }
    points[i] = pt;

    if (i == 0) {
      xmin = pt.x;
      xmax = pt.x;
      ymin = pt.y;
      ymax = pt.y;
    } else {
      if (pt.x < xmin)
        xmin = pt.x;
      if (pt.x > xmax)
        xmax = pt.x;
      if (pt.y < ymin)
        ymin = pt.y;
      if (pt.y > ymax)
        ymax = pt.y;
    }

    min.x = xmin;
    max.x = xmax;
    min.y = ymin;
    max.y = ymax;
  }
  center.x = ((max.x - min.x) / 2) + min.x;
  center.y = ((max.y - min.y) / 2) + min.y;
  return true;
}

void FuncItem::save() {
  ofstream out(filename.c_str());
  if (!out || !out.good())
    return;

  switch (version) {

  case ORIGINAL:
    out << "range: 0.0 1.0" << endl;
    break;

  case v1_1:
    out << "fver 1 1" << endl;
    out << "name: " << name << endl;
    out << "samples: " << samples << endl;
    
    out << "flip: " << (flip ? "on" : "off") << endl;
    break;

  default:
    return;
  }

  out << "points: " << points.size() << endl;
  for (unsigned int i = 0; i < points.size(); i++)
    if (flip)
      out << points[i].y << " " << points[i].x << " " << points[i].multiplicity << endl;
    else
      out << points[i].x << " " << points[i].y << " " << points[i].multiplicity << endl;
}



void FuncItem::readConfigFile() {
  const char *cdir = getenv("VLABCONFIGDIR");
  if (!cdir)
    return;

  string config = string(cdir);
  config += "/funcedit";

  ifstream in(config.c_str());
  if (!in || !in.good())
    return;

  while (in && !in.eof()) {
    string item;

    in >> item >> ws;
    if (item == string("background:"))
      in >> _background[0] >> _background[1] >> _background[2] >> ws;
    else if (item == string("curve:"))
      in >> _curve[0] >> _curve[1] >> _curve[2] >> ws;
  }
}

FuncItem::FuncItem(Gallery *pGal,std::string filename,std::string selection_name):
  Item(pGal, filename, pGal, selection_name.c_str()), samples(0),
  flip(false), version(NOT_LOADED) {
  name = selection_name;
  pEditor = new FuncEditor();
  tmpDir = pGal->getTmpDir();
    //Initialize window
  _width = pGal->getItemWidth();
  _height = pGal->getItemHeight();

  _visible = true;
  _background[0] = 0.0;
  _background[1] = 0.0;
  _background[2] = 0.0;
  _curve[0] = 1.0;
  _curve[1] = 1.0;
  _curve[2] = 0.0;

    // load the config options from file
  try {
    Config::readConfigFile();
   } catch (Config::ConfigErrorExc exc) {
    cerr << exc.message() << endl;
  } catch (...) {
    cerr << "Unknown config error occured" << endl;
  }

  //  readConfigFile();
  ofstream out;

  //   if (extension != string(".func"))
  //    filename += ".func";
    out.open(filename.c_str());
    out << "fver 1 1"
        << endl
        << "name: " << selection_name << endl
        << "samples: 20" << endl
        << "flip: off" << endl
        << "points: 4" << endl
        << "0.00 0.00" << endl
        << "0.33 0.00" << endl
        << "0.67 0.00" << endl
        << "1.00 0.00" << endl;
    pNameTxt->setText(this->name.c_str());
    load();
}
