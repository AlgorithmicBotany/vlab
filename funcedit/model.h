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




#ifndef __MODEL_H__
#define __MODEL_H__

#include <string>
#include <vector>
#include "dynarray.h"

class ReadTextFile;
class WriteTextFile;

class Model : public DynArray<WorldPoint>
{
public:
  Model();
  ~Model();

  bool hasChanged() {return _hasChanged;}

  void Draw(int,int) const;
  void Load(ReadTextFile&);
  void Save(WriteTextFile&);

  void AddPoint(WorldPoint);
  void DeletePoint(int);
  int  FindClosest(WorldPoint) const;
  void MovePoint(int, WorldPoint);
  void IncPointMultiplicity(int);

  void setFlip(bool enable);
  bool isFlipped() {return flip;}

  std::string getName() const {return name;}
  unsigned int getSamples() const { return samples; }

  void setName(std::string s) const {name = s; _hasChanged = true;}
  void setSamples(unsigned int n){
    samples = n;
    _hasChanged = true;
  }

  WorldPoint getMax() {return max;}
  WorldPoint getMin() {return min;}


    // Functions to add a background 
  void SetBackgroundImage(const std::string &name);
  void SetDimensions(float width, float height) { _backgrdImageWidth = width; _backgrdImageHeight = height; }
  bool backgrdImage() const { return _backgrdImage; }
  float backgrdImageWidth() const { return _backgrdImageWidth; }
  float backgrdImageHeight() const { return _backgrdImageHeight; }
  const float* backgrdTranslate() const { return _backgrdImageTranslate; }
  std::string const& Filename() const { return _backgrdImageFilename; }
  void Shift (float x, float y) { _backgrdImageTranslate[0] += x; _backgrdImageTranslate[1] += y; }
  
  void TranslateAll (float x, float y);


private:
  void _Insert(WorldPoint, int);

  void _Draw(int,int) const;

  void _DrawPoints(int) const;
  void _DrawSegments() const;
  void _DrawCurve() const;

  /*********************
    Background stuff
  *********************/

  bool _backgrdImage;
  std::string _backgrdImageFilename;
  float _backgrdImageWidth, _backgrdImageHeight;
  float _backgrdImageTranslate[2];
  

/******************
   Curve stuff
******************/

  WorldPoint P(double) const;
  WorldPoint PMult(double,DynArray<WorldPoint>& ctrlpts) const;
  double N(int, int, double,int num) const;
  double Nk1(int, double, int num) const;
  double Nkt(int, int, double, int num) const;
  int Uk(int, int) const;
  mutable bool _hasChanged;

  enum {
    NOT_LOADED,
    ORIGINAL,
    v1_1
  } version;

  mutable std::string name;
  int samples;
  bool flip;
  

  WorldPoint max, min;
};

#endif
