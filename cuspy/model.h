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

class ReadTextFile;
class WriteTextFile;

#include "dynarray.h"

class Model : public DynArray<WorldPoint>{
public:
  Model();
  ~Model();

  bool hasChanged() { return _hasChanged; }

  void Draw(int,int) const;
  void Load(ReadTextFile &);
  void Save(WriteTextFile &);

  void AddPoint(WorldPoint);
  void DeletePoint(int);
  int FindClosest(WorldPoint) const;
  void MovePoint(int, WorldPoint);
  void IncPointMultiplicity(int);

  void setClosed(bool closed) { _isClosed = closed; }
  bool isClosed() const { return _isClosed; }

  double size() { return _size; }
  WorldPoint center() { return _center; }

  std::string getName() const { return name; }
  unsigned int getSamples() const { return _samples; }
  void setName(std::string n) {
    name = n;
    _hasChanged = true;
  }
  void setSamples(unsigned int n){
    _samples = n;
    _hasChanged = true;
  }

  // Functions to add a background 
  void SetBackgroundImage(const std::string &name);
  void SetDimensions(float width, float height) { _backgrdImageWidth = width; _backgrdImageHeight = height; }
  bool backgrdImage() const { return _backgrdImage; }
  float backgrdImageWidth() const { return _backgrdImageWidth; }
  float backgrdImageHeight() const { return _backgrdImageHeight; }
  const float* backgrdTranslate() const { return _backgrdImageTranslate; }
  std::string const& Filename() const { return _backgrdImageFilename; }
  void Shift (float x, float y) { _backgrdImageTranslate[0] += x; _backgrdImageTranslate[1] += y; }
  // 'markers' are an experimental feature that will be considered for release in future versions
  //unsigned long NumMarkers (void) const { return _markers.size(); }
  //void AddMarker(const WorldPoint &wp);
  //void DelMarker(const WorldPoint &wp);
  //void MoveMarker(const WorldPoint &wp);
  //void SelectMarker(const WorldPoint &wp);
  //void DeselectMarker();
  //bool Selected(unsigned int i) const { return i == _selectedMarker; }
  //void ResetMarkers(void);
  //void UpdateMarkers(void);
  //const WorldPoint& GetPos (unsigned int i) const { return _markers[i]._pos; }
  //const WorldPoint& GetPosL (unsigned int i) const { return _markers[i]._posl; }
  //const WorldPoint& GetPosR (unsigned int i) const { return _markers[i]._posr; }
  //const std::string& GetStr (unsigned int i) const { return _markers[i]._str; }
  
  void TranslateAll (float x, float y);

  
private:
  enum { NOT_LOADED, ORIGINAL, v1_1, v1_2, v1_3, v1_4 } version;

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
  
  // 'markers' are an experimental feature that will be considered for release in future versions
  //struct Marker {
  //  WorldPoint _pos, _posl, _posr;
  //  float _u, _ul, _ur;
  //  double _t, _tl, _tr;
  //  int _Gi;
  //  std::string _str;
  //};
  
  //std::vector<Marker> _markers;
  //unsigned int _selectedMarker;
  //Marker _FindClosestNormalizedCoordinate(const WorldPoint&) const;

  /******************
     Curve stuff
  ******************/

  void PrepBSplineMatrix();
  double Basis0(double) const;
  double Basis1(double) const;
  double Basis2(double) const;
  double Basis3(double) const;

  WorldPoint P(double d, const DynArray<WorldPoint> &pts) const;
  double N(int k, int t, float u, int n) const;
  double Nk1(int k, float u, int n) const;
  double Nkt(int k, int t, float u, int n) const;
  int Uk(int j, int n) const;

  bool _isClosed;
  bool _isEp;
   unsigned int _samples;
  double BSplineMat[16];
  double _size;
  WorldPoint _center;

  mutable bool _hasChanged;

  mutable std::string name;
};

#endif
