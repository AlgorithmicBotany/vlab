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




#ifndef __FUNC_H__
#define __FUNC_H__

#include <vector>
#include <fstream>
#include <iostream>

#include "item.h"

// function item
class FuncItem : public Item {

 public:
  FuncItem(Gallery* pGal, std::string filename, QWidget* parent, const char* name = 0, Qt::WindowFlags f = 0);

  FuncItem(Gallery* pGal, std::string filename, std::string name);

  FuncItem(const FuncItem &c, std::string n){
    pGallery = c.getGallery();
      //Initialize window
    _width = pGallery->getItemWidth();
    _height = pGallery->getItemHeight();
    

    pEditor = new FuncEditor();
    _visible = true;
    name = n;
    pNameTxt = new QLabel(name.c_str(),c.getGallery());
    samples = c.Samples();
    flip = c.Flip();
    points = c.Points();
    center = c.Center();
    max = c.Max();
    min = c.Min();
    tmpDir = c.getTmpDir();
    //now change file name and copy files into new filename
    char *tempname = new char[21];
    strcpy(tempname, tmpDir.c_str());
    strcpy(tempname + 10, "/gal.XXXXXX");
    mkstemp(tempname);
    
    filename = tempname;
    filename += ".fun";
    std::ofstream  dst(filename);
    dst << "fver 1 1" << std::endl;
    dst << "name: " << name << std::endl;
    dst << "samples: " << samples << std::endl;
    dst << "flip: " << (flip ? "on" : "off") << std::endl;
    dst << "points: " << points.size() << std::endl;
    for (unsigned int i = 0; i < points.size(); i++)
      if (flip)
      dst << points[i].y << " " << points[i].x << " " << points[i].multiplicity << std::endl;
    else
      dst << points[i].x << " " << points[i].y << " " << points[i].multiplicity << std::endl;
    
    

    _background[0] = c.Background(0);
    _background[1] = c.Background(1);
    _background[2] = c.Background(2);
    _curve[0] = c.Curve(0);
    _curve[1] = c.Curve(1);
    _curve[2] = c.Curve(2);

  }

  std::string Name()const{ return name;}
  int Samples()const{ return samples;}
  bool Flip()const{ return flip;}
  PtVec Points()const{ return points;}
  Point Center()const{return center;}
  Point Min()const{return min;}
  Point Max()const {return max;}
  std::string getTmpDir() const { return tmpDir;}
  double* Background() { return _background;}
  double* Curve(){return _curve;}
  double Background(const int i) const { return _background[i];}
  double Curve(const int i) const{return _curve[i];}

  void readConfigFile();


  bool load();
  void save();

private:
  PtVec points;
  Point center, max, min;
  std::string tmpDir;

  // format 1.1 features
  int samples;
  bool flip;
  double _background[3];
  double _curve[3];


  enum {
    NOT_LOADED,
    ORIGINAL,
    v1_1
  } version;
};



#endif
