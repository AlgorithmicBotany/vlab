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



#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string.h>

#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#include <string.h>
#endif

#include "colors.h"
#include "drawflags.h"
#include "dynarray.h"
#include "file.h"
#include "geometry.h"
#include "glutils.h"
#include "model.h"

using std::string;

Model::Model()
    : DynArray<WorldPoint>(32), _hasChanged(false), version(NOT_LOADED),
      name("noname"), samples(10), flip(false) {
  
  _backgrdImage = false;
  _backgrdImageTranslate[0] = 0.f;
  _backgrdImageTranslate[1] = 0.f;

  WorldPoint wp(0.0, 0.0, 0.0);
  Add(wp);
  wp.Set(1.0 / 3.0, 0.0, 0.0);
  Add(wp);
  wp.Set(2.0 / 3.0, 0.0, 0.0);
  Add(wp);
  wp.Set(1.0, 0.0, 0.0);
  Add(wp);
}

Model::~Model() {}

void Model::Draw(int drawWhat, int retinaScale) const { _Draw(drawWhat,retinaScale); }

void Model::Load(ReadTextFile &src) {
  char s[16];
  fscanf(src, "%s", s);
  name = src.getname();

  if (!strcmp(s, "range:")) {
    version = ORIGINAL;
    //remove extension if any in name
    size_t lastindex = name.find_last_of("."); 
    name = name.substr(0, lastindex); 
    float rmin, rmax;
    if (2 != fscanf(src, "%f %f\n", &rmin, &rmax)) {
      fprintf(stderr, "Error reading function: Bad range statement\n");
      return;
    }
  } else if (!strcmp(s, "fver")) {
    int major, minor;

    if (2 != fscanf(src, "%d %d\n", &major, &minor)) {
      fprintf(stderr, "Error reading function: Bad version statement\n");
      return;
    }

    if (major == 1 && minor == 1) {
      version = v1_1;

      char c = 0;
      name = string();
      if (1 != fscanf(src, "name: %c", &c)) {
	fprintf(stderr, "Error reading function: Bad function name\n");
	return;
      }

      while (c != '\n') {
	name += c;
	c = fgetc(src);
      }
      
      if (1 != fscanf(src, "samples: %d\n", &samples)) {
        fprintf(stderr, "Error reading function: Bad samples statement\n");
        return;
      }
      // read info on background image (if exists)
      // fgetc and ungetc are used to read in a char value and test it,
      // based on the result the background image is read 
      bool readImage = false;
      c = fgetc(src);
      if (c == 'b') {
        readImage = true;
      }
      ungetc(c, src);
      
      if (readImage) {
        char name[128];
        if (1 != fscanf(src, "background: %s\n", name)) {
          fprintf(stderr, "Error reading function: Bad background statement\n");
          return;
        }
        float x, y;
        if (2 != fscanf(src, "backgroundXY: %f %f\n", &x, &y)) {
          fprintf(stderr, "Error reading function: Bad backgroundXY statement\n");
          return;
        }
        _backgrdImage = true;
        _backgrdImageFilename = std::string(name);
        _backgrdImageTranslate[0] = x;
        _backgrdImageTranslate[1] = y;
      }


      char flip_buff[4];
      if (1 != fscanf(src, "flip: %s\n", flip_buff)) {
        fprintf(stderr, "Error reading function: Bad flip statement\n");
        return;
      }
      if (strcmp(flip_buff, "off"))
        flip = true;
    } else {
      fprintf(stderr, "Error reading function: Unknown func version\n");
      return;
    }
  } else {
    fprintf(stderr, "Error reading function: Bad function file\n");
    return;
  }

  int num;
  if (1 != fscanf(src, "points: %d\n", &num)) {
    fprintf(stderr, "Error reading function: Invalid points dtatement\n");
    return;
  }

  Reset();
  max.Set(0, 0, 0);
  min.Set(0, 0, 0);
  for (int i = 0; i < num; i++) {
    float x, y;
    int multiplicity = 1;
    string coordinate;
    char c = fgetc(src);
    while (c != '\n') {
      coordinate += c;
      c = fgetc(src);
    }
    coordinate += " ";
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
      fprintf(stderr, "Error reading function: Bad point coordinates\n");
      return;
    }
    x = std::stof(posVector[0]);
    y = std::stof(posVector[1]);
    if (sizeVec == 3)
      multiplicity = std::stof(posVector[2]);

    if (x > max.X())
      max.X(x);
    if (y > max.Y())
      max.Y(y);
    if (x < min.X())
      min.X(x);
    if (y < min.Y())
      min.Y(y);

    if (flip) {
      WorldPoint wp(y, x, 0.0,multiplicity);
      Add(wp);
    } else {
      WorldPoint wp(x, y, 0.0,multiplicity);
      Add(wp);
    }
  }

  _hasChanged = false;
}

void Model::Save(WriteTextFile &trg) {
  switch (version) {
  case ORIGINAL:
    //[PASCAL] the following code is commented out because it is not used anymore
    // however it's kept for historic reason.

    //trg.PrintF("range: 0.0 1.0\n");
    //break;

  case NOT_LOADED:
  case v1_1:
    trg.PrintF("fver 1 1\n");
    trg.PrintF("name: %s\n", name.c_str());
    trg.PrintF("samples: %d\n", samples);
    if (_backgrdImage) {
      trg.PrintF("background: %s\n", _backgrdImageFilename.c_str());
      trg.PrintF("backgroundXY: %f %f\n", _backgrdImageTranslate[0], _backgrdImageTranslate[1]);
    }
    if (flip)
      trg.PrintF("flip: on\n");
    else
      trg.PrintF("flip: off\n");
    break;

  default:
    return;
  }

  trg.PrintF("points: %d\n", Count());
  for (int i = 0; i < Count(); i++){
    double x =  _arr[i].X();
    double y =  _arr[i].Y();
    if (flip){
      double tmp = x;
      x = y;
      y = tmp;
    }
    int multiplicity = _arr[i].getMultiplicity();
    trg.PrintF("%f %f %d\n",x,y,multiplicity);
  }
  _hasChanged = false;
}

void Model::SetBackgroundImage(std::string const &name)
{
  _backgrdImage = true;
  _backgrdImageFilename = name;
  //_backgrdImageWidth = width;
  //_backgrdImageHeight = height;
  _backgrdImageTranslate[0] = 0.f;
  _backgrdImageTranslate[1] = 0.f;

  _hasChanged = true;
}



void Model::_Draw(int drawWhat, int retinaScale) const {
  if (DrawSegments & drawWhat)
    _DrawSegments();
  if (DrawCurve & drawWhat)
    _DrawCurve();
  if (DrawPoints & drawWhat)
    _DrawPoints(retinaScale);
}

void Model::_DrawPoints(int retinaScale) const {

  glColor3dv(GetColor(Points));
  glPointSize(GetSize(PointSize)*retinaScale);
  GLpoints glp;
  for (int i = 0; i < _items; i++){
     switch (_arr[i].getMultiplicity()) {
    case 1:
      glColor3d(1.0, 1.0, 1.0);
      break;
    case 2:
      glColor3d(1.0, 1.0, 0.0);
      break;
    case 3:
      glColor3d(0.0, 1.0, 1.0);
      break;
    default:
      glColor3d(1.0, 0.0, 0.0);
      break;
    }
   glp.Vertex(_arr[i]);
  }
}

void Model::_DrawSegments() const {
  glColor3dv(GetColor(Segments));

  glLineWidth(GetSize(SegmentWidth));
  GLlinestrip gls;
  for (int i = 0; i < _items; i++)
    gls.Vertex(_arr[i]);
}

void Model::_DrawCurve() const {
  glColor3dv(GetColor(Curve));
  glLineWidth(GetSize(CurveWidth));
  // Make a list of control points.  Using _items * 3
  //  will guarantee that we will not need to grow the array.
  //  Since we do this lots, it's good to save time here.
  DynArray<WorldPoint> ctrlpts(_items * 3);

  for (int i = 0; i < _items; i++)
    for (int j = 0; j < _arr[i].getMultiplicity(); j++)
      ctrlpts.Add(_arr[i]);

  
  int num = 6 * ctrlpts.Count();

  GLlinestrip gls;
  for (int i = 0; i < num; i++) {
    WorldPoint wp = PMult(double(i) / num,ctrlpts);
    gls.Vertex(wp);
  }
  gls.Vertex(ctrlpts[ctrlpts.Count() - 1]);
}

void Model::AddPoint(WorldPoint wp) {
  int i = 0;

  if (flip)
    while (_arr[i].Y() < wp.Y())
      i++;
  else
    while (_arr[i].X() < wp.X())
      i++;

  _Insert(wp, i);

  _hasChanged = true;
}

void Model::MovePoint(int i, WorldPoint wp) {
  assert(i >= 0);
  assert(i < _items);

  if (flip) {
    if (0 == i) {
      wp.Y(0.0);
    } else if (_items - 1 == i) {
      wp.Y(1.0);
    } else {
      if (wp.Y() < _arr[i - 1].Y())
        wp.Y(_arr[i - 1].Y());
      else if (wp.Y() > _arr[i + 1].Y())
        wp.Y(_arr[i + 1].Y());
    }
  } else {
    if (0 == i) {
      wp.X(0.0);
    } else if (_items - 1 == i) {
      wp.X(1.0);
    } else {
      if (wp.X() < _arr[i - 1].X())
        wp.X(_arr[i - 1].X());
      else if (wp.X() > _arr[i + 1].X())
        wp.X(_arr[i + 1].X());
    }
  }
  
  wp.setMultiplicity(_arr[i].getMultiplicity());

  _arr[i] = wp;

  _hasChanged = true;
}

void Model::setFlip(bool enable) {
  flip = enable;
  for (int i = 0; i < _items; i++)
    _arr[i].Set(_arr[i].Y(), _arr[i].X(), 0);
}

void Model::_Insert(WorldPoint wp, int n) {
  assert(n >= 0);
  assert(n <= _items);
  if (_size == _items)
    _Grow();
  for (int i = _items; i > n; i--)
    _arr[i] = _arr[i - 1];
  _arr[n] = wp;
  _items++;
}

int Model::FindClosest(WorldPoint wp) const {
  assert(_items > 0);
  int toret = 0;
  double mindist = wp.DistanceTo(_arr[0]);
  for (int i = 1; i < _items; i++) {
    double dist = wp.DistanceTo(_arr[i]);
    if (dist < mindist) {
      mindist = dist;
      toret = i;
    }
  }

  return toret;
}

void Model::DeletePoint(int n) {
  assert(n >= 0);
  assert(n < _items);

  // do not allow deletion of end points
  if (n == 0)
    return;
  if (n == _items - 1)
    return;

  for (int i = n; i < _items - 1; i++)
    _arr[i] = _arr[i + 1];

  _items--;

  _hasChanged = true;
}

void Model::IncPointMultiplicity(int n) {
  assert(n >= 0);
  assert(n < _items);

  _arr[n].incMultiplicity();
  if (_arr[n].getMultiplicity() > 3)
    _arr[n].setMultiplicity(1);

  _hasChanged = true;
}

WorldPoint Model::PMult(double d,DynArray<WorldPoint>& ctrlpts) const {
  assert(d >= 0.0);
  assert(d <= 1.0);
  const int n = ctrlpts.Count() - 1;
  const int t = 4;
  double u = d * (n - t + 2);
  WorldPoint sum;
  for (int k = 0; k <= n; k++) {
    double coeff = N(k, t, u,n);
    sum += ctrlpts[k] * coeff;
  }
  return sum;
}


WorldPoint Model::P(double d) const {
  assert(d >= 0.0);
  assert(d <= 1.0);
  const int n = Count() - 1;
  const int t = 4;
  double u = d * (n - t + 2);
  WorldPoint sum;
  for (int k = 0; k <= n; k++) {
    double coeff = N(k, t, u,n);
    sum += _arr[k] * coeff;
  }
  return sum;
}

double Model::N(int k, int t, double u, int num) const {
  double res = 0.0;
  if (1 == t)
    res = Nk1(k, u,num);
  else
    res = Nkt(k, t, u,num);
  return res;
}

double Model::Nk1(int k, double u, int num) const {
  if (Uk(k,num) <= u) {
    if (u < Uk(k + 1,num))
      return 1.0;
  }
  return 0.0;
}

double Model::Nkt(int k, int t, double u, int num) const {
  double sum = 0.0;
  int div = Uk(k + t - 1,num) - Uk(k,num);
  if (0 != div)
    sum = (u - Uk(k,num)) / div * N(k, t - 1, u,num);

  div = Uk(k + t,num) - Uk(k + 1,num);
  if (0 != div)
    sum += (Uk(k + t,num) - u) / div * N(k + 1, t - 1, u,num);

  return sum;
}

int Model::Uk(int j,int num) const {
  const int n = num ;
  const int t = 4;
  if (j < t)
    return 0;
  if (j > n)
    return n - t + 2;
  return j - t + 1;
}



void Model::TranslateAll(float x, float y) {
  WorldPoint t(x,y,0.f);
  for (int i = 0; i < _items; i++) {
    _arr[i] += t;
  }  
  _hasChanged = true;
}
