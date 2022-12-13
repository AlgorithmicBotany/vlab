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




#ifndef __CON_H__
#define __CON_H__

#include <vector>
#include <fstream>
#include <iostream>
#include "item.h"


// contour item
class ConItem : public Item {

 public:

  ConItem(Gallery* pGal, std::string filename, QWidget* parent, const char* name = 0, Qt::WindowFlags f = 0);
    ConItem(Gallery* pGal, std::string filename,std::string name);


  ConItem(const ConItem &c, std::string n){
    pGallery = c.getGallery();
      //Initialize window
    _width = pGallery->getItemWidth();
    _height = pGallery->getItemHeight();
    pEditor = new ConEditor();
    _visible = true;
    name = n;
    pNameTxt = new QLabel(n.c_str(),c.getGallery());
    samples = c.Samples();
    points = c.Points();
    center = c.Center();
    max = c.Max();
    min = c.Min();
    isClosed = c.IsClosed();
    endPointInterpolation = c.EndPointInterpolation();
    tmpDir = c.getTmpDir();
    //now change file name and copy files into new filename
    char *tempname = new char[21];
    strcpy(tempname, tmpDir.c_str());
    strcpy(tempname + 10, "/gal.XXXXXX");
    mkstemp(tempname);
    
    filename = tempname;
    filename += ".con";
    std::ofstream  dst(filename);
    int num = 0;
    for (unsigned int i = 0; i < points.size(); i++)
	num += points[i].multiplicity;
    
    dst << "cver 1 3" << std::endl
	<< "name: " << pNameTxt->text().toStdString().c_str() << std::endl
        << "points: " << points.size() << " " << num << std::endl
        << "type: " << (isClosed ? "c" : "o")
        << (endPointInterpolation ? "e" : "r") << std::endl
        << "samples: " << samples << std::endl;
    
    for (unsigned int i = 0; i < points.size(); i++)
      dst << points[i].x << " " << points[i].y << " " << points[i].z << " "
          << points[i].multiplicity << std::endl;
    

    _background[0] = c.Background(0);
    _background[1] = c.Background(1);
    _background[2] = c.Background(2);
    _curve[0] = c.Curve(0);
    _curve[1] = c.Curve(1);
    _curve[2] = c.Curve(2);
    
 
   }
  

  std::string Name()const{ return name;}
  int Samples()const{ return samples;}
  PtVec Points()const{ return points;}
  Point Center()const{return center;}
  Point Min()const{return min;}
  Point Max()const {return max;}
  bool IsClosed() const {return isClosed;}
  bool EndPointInterpolation() const {return endPointInterpolation;}
  std::string getTmpDir() const { return tmpDir;}
  bool load();
  void save();
  Point Max(){ return max;}
  double* Background(){ return _background;}
  double* Curve() {return _curve;}
  double Background(const int i) const { return _background[i];}
  double Curve(const int i) const{return _curve[i];}

  void readConfigFile();

 private:
  enum {
    NOT_LOADED,
    ORIGINAL,
    v1_1,
    v1_2,
    v1_3,
    v1_4
  } version;


  PtVec points;
  bool  isClosed;
  bool  endPointInterpolation;
  std::string tmpDir;
  unsigned int samples;

  Point center;
  Point max;
  Point min;

  double _background[3];
  double _curve[3];
};

#endif
