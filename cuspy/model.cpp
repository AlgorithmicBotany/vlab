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
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <iostream>

#include <qgl.h>

#include "dynarray.h"
#include "geometry.h"
#include "model.h"
#include "glutils.h"
#include "file.h"
#include "drawflags.h"
#include "colors.h"

using std::string;

Model::Model()
    : DynArray<WorldPoint>(32), version(NOT_LOADED), _isClosed(true),
      _isEp(false), _samples(8), _size(1.0), _center(0.0, 0.0, 0.0),
      _hasChanged(false), name("noname") {

  _backgrdImage = false;
  _backgrdImageTranslate[0] = 0.f;
  _backgrdImageTranslate[1] = 0.f;

  PrepBSplineMatrix();

  WorldPoint wp(0.4, 0.4, 0.0);
  Add(wp);
  wp.Set(0.4, -0.4, 0.0);
  Add(wp);
  wp.Set(-0.4, -0.4, 0.0);
  Add(wp);
  wp.Set(-0.4, 0.4, 0.0);
  Add(wp);
}

Model::~Model() {}

void Model::Draw(int drawWhat, int retinaScale) const { _Draw(drawWhat, retinaScale); }

void Model::Load(ReadTextFile &src) {
  int num, dimensions;
  char buff[16];
  bool checkz = false, foundz = false;
  name = src.getname();

  double minx = 0.0, maxx = 0.0, miny = 0.0, maxy = 0.0;

  char c = fgetc(src);
  ungetc(c, src);

  if (isdigit(c)) {
    version = ORIGINAL;

    if (3 != fscanf(src, "%d %d %s\n", &num, &dimensions, buff)) {
      fprintf(stderr,
              "Error reading function: bad number, dimension or type\n");
      return;
    }

    if (!strcasecmp(buff, "closed"))
      _isClosed = true;
    else
      _isClosed = false;

    if (dimensions < 2) {
      fprintf(stderr, "Error: This program requires curves that have at least "
                      "two dimensions.\n");
      return;
    } else if (dimensions == 3) {
      checkz = true;
    } else if (dimensions > 3) {
      fprintf(stderr, "Warning: This program only handles 2D curves.  Data for "
                      "extra dimenstions will be discarded.");
    }

    Reset();
    for (int i = 0; i < num; i++) {
      float x, y, z;
      if (checkz) {
        if (3 != fscanf(src, "%f %f %f", &x, &y, &z)) {
          fprintf(stderr, "Error reading function\n");
          return;
        }

        if (z >= 0.00001 || z <= -0.00001)
          foundz = true;
      } else if (2 != fscanf(src, "%f %f", &x, &y)) {
        fprintf(stderr, "Error reading function\n");
        return;
      } else {
        // check for extra data on the line
        if ((char)fgetc(src) != '\n') {
          char dummy[256];
          fscanf(src, "%s\n", dummy);
        }
      }

      WorldPoint wp(x, y, 0.0);

      if (i == 0) {
        maxx = x;
        minx = x;
        maxy = y;
        miny = y;
      } else {
        if (x > maxx)
          maxx = x;
        if (x < minx)
          minx = x;
        if (y > maxy)
          maxy = y;
        if (y < miny)
          miny = y;
      }

      if (wp == _arr[_items - 1])
        _arr[_items - 1].incMultiplicity();
      else
        Add(wp);
    }

    if (foundz)
      fprintf(stderr, "Warning: This program only handles 2D curves.  Data for "
                      "extra dimenstions will be discarded.");

    fflush(stderr);
  } else if (c == 'c') {
    // check for cver 1 1
    int major = 0, minor = 0;
    if (2 != fscanf(src, "cver %d %d\n", &major, &minor)) {
      fprintf(stderr, "Error reading function: Bad version statement\n");
      return;
    }

    if (major == 1) {
      switch (minor) {
      case 1:
        version = v1_1;
        break;
      case 2:
        version = v1_2;
        break;
      case 3:
        version = v1_3;
        break;
      default:
        fprintf(stderr, "Error reading function: Bad version statement\n");
        return;
      }

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

      int num_total;
      if (2 != fscanf(src, "points: %d %d\n", &num, &num_total)) {
        fprintf(stderr, "Error reading function: Bad points statement\n");
        return;
      }

      if (1 != fscanf(src, "type: %s\n", buff)) {
        fprintf(stderr, "Error reading function: Bad type statement\n");
      }

      switch (version) {
      case v1_1:
        if (!strcasecmp(buff, "closed"))
          _isClosed = true;
        else
          _isClosed = false;
        break;
      case v1_2:
      case v1_3:
        if (strlen(buff) != 2) {
          fprintf(stderr, "Error reading function: Bad type.");
          return;
        }
        _isClosed = buff[0] == 'c' ? true : false;
        _isEp = buff[1] == 'e' ? true : false;
        break;
      default:
        break;
      }

      if (version == v1_3) {
        if (1 != fscanf(src, "samples: %d\n", &_samples)) {
          fprintf(stderr, "Error reading function: Bad number of samples.");
          return;
        }
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


      Reset();
      for (int i = 0; i < num; i++) {
        float x, y, z;
        int m;

        if (4 != fscanf(src, "%f %f %f %d\n", &x, &y, &z, &m)) {
          fprintf(stderr, "Error reading function: Bad control point\n");
          return;
        }
        //[PASCAL] forcing z coordinate to 0
        if (z != 0.) {
          std::cerr << "WARNING forcing z coordinate of Point: " << x << " "
                    << y << " "
                    << " " << z << " to 0" << std::endl;
          z = 0.;
        }

        WorldPoint p(x, y, z);
        p.setMultiplicity(m);

        if (i == 0) {
          maxx = x;
          minx = x;
          maxy = y;
          miny = y;
        } else {
          if (x > maxx)
            maxx = x;
          if (x < minx)
            minx = x;
          if (y > maxy)
            maxy = y;
          if (y < miny)
            miny = y;
        }

        Add(p);
      }
    } else {
      // bad version
      fprintf(stderr, "Error reading function: Unknown version\n");
    }
  } else if (c == 'v') {
    // check for version 1.4
    if (1 != fscanf(src, "version: %s\n", buff)) {
      fprintf(stderr, "Error reading function: Bad version statement\n");
      return;
    }

    if (!strcmp(buff, "1.4")) {
      // version v1_4 was found
      version = v1_4;
      _isClosed = false;

      int x, y, z;

      if (3 != fscanf(src, "contact: %d %d %d\n", &x, &y, &z)) {
        fprintf(stderr, "Error reading function: Bad contact statement\n");
        return;
      }
      if (3 != fscanf(src, "end: %d %d %d\n", &x, &y, &z)) {
        fprintf(stderr, "Error reading function: Bad end statement\n");
        return;
      }
      if (3 != fscanf(src, "heading: %d %d %d\n", &x, &y, &z)) {
        fprintf(stderr, "Error reading function: Bad heading statement\n");
        return;
      }
      if (3 != fscanf(src, "up: %d %d %d\n", &x, &y, &z)) {
        fprintf(stderr, "Error reading function: Bad up statement\n");
        return;
      }
      if (1 != fscanf(src, "size: %d\n", &x)) {
        fprintf(stderr, "Error reading function: Bad size statement\n");
        return;
      }
      if (1 != fscanf(src, "points: %d\n", &num)) {
        fprintf(stderr, "num = %d\n", num);
        fprintf(stderr, "Error reading function: Bad points statement\n");
        return;
      }
      if (0 != fscanf(src, "range: 0.0 1.0\n")) {
        fprintf(stderr, "Error reading function: Bad range statement\n");
        return;
      }
      if (0 != fscanf(src, "dimension: 4\n")) {
        fprintf(stderr, "Error reading function: Bad dimension statement\n");
        return;
      }
      if (0 != fscanf(src, "type: bspline\n")) {
        fprintf(stderr, "Error reading function: Bad type statement\n");
        return;
      }

      Reset();
      for (int i = 0; i < num; i++) {
        float x, y, z;
        int m;

        if (4 != fscanf(src, "%f %f %f %d\n", &x, &y, &z, &m)) {
          fprintf(stderr, "Error reading function: Bad control point\n");
          return;
        }

        WorldPoint p(x, y, z);
        p.setMultiplicity(m);

        if (i == 0) {
          maxx = x;
          minx = x;
          maxy = y;
          miny = y;
        } else {
          if (x > maxx)
            maxx = x;
          if (x < minx)
            minx = x;
          if (y > maxy)
            maxy = y;
          if (y < miny)
            miny = y;
        }

        Add(p);
      }
    } else {
      // bad version
      fprintf(stderr, "Error reading function: Unknown version\n");
    }
  } else {
    // unknown version
    fprintf(stderr, "Error reading function: Bad version statement\n");
  }

  double _sizex = maxx - minx;
  double _sizey = maxy - miny;
  if (_sizex < _sizey)
    _size = fabs(_sizey);
  else
    _size = fabs(_sizex);

  _center.Set(_sizex / 2 + minx, _sizey / 2 + miny, 0.0);

  _hasChanged = false;
}

void Model::Save(WriteTextFile &trg) {
  // get the number of points
  switch (version) {
  case ORIGINAL:
    //[PASCAL] the following code is commented out because it is not used anymore
    // however it's kept for historic reason.
    /*
    int points = 0;
    for (int i = 0; i < _items; i++)
      points += _arr[i].getMultiplicity();

    trg.PrintF("%d %d %s\n", points, 2, _isClosed ? "closed" : "open");

    for (int i = 0; i < _items; i++)
      for (int j = 0; j < _arr[i].getMultiplicity(); j++)
        trg.PrintF("%f %f\n", _arr[i].X(), _arr[i].Y());
    break;
    */
  case v1_1:
    /*
    int points = 0;
    for (int i = 0; i < _items; i++)
      points += _arr[i].getMultiplicity();

    trg.PrintF("cver 1 1\n");
    trg.PrintF("name: %s\n", name.c_str());
    trg.PrintF("points: %d %d\n", _items, points);
    trg.PrintF("type: %s\n", _isClosed ? "closed" : "open");

    for (int i = 0; i < _items; i++)
      trg.PrintF("%f %f %f %d\n", _arr[i].X(), _arr[i].Y(), _arr[i].Z(),
                 _arr[i].getMultiplicity());
    break;
    */
  case v1_2:
    /*
    int points = 0;
    for (int i = 0; i < _items; i++)
      points += _arr[i].getMultiplicity();

    trg.PrintF("cver 1 2\n");
    trg.PrintF("name: %s\n", name.c_str());
    trg.PrintF("points: %d %d\n", _items, points);
    trg.PrintF("type: %s%s\n", _isClosed ? "c" : "o", _isEp ? "e" : "r");

    for (int i = 0; i < _items; i++)
      trg.PrintF("%f %f %f %d\n", _arr[i].X(), _arr[i].Y(), _arr[i].Z(),
                 _arr[i].getMultiplicity());
    break;
    */
  case NOT_LOADED:
  case v1_3: {
    int points = 0;
    for (int i = 0; i < _items; i++)
      points += _arr[i].getMultiplicity();

    trg.PrintF("cver 1 3\n");
    trg.PrintF("name: %s\n", name.c_str());
    trg.PrintF("points: %d %d\n", _items, points);
    trg.PrintF("type: %s%s\n", _isClosed ? "c" : "o", _isEp ? "e" : "r");
    trg.PrintF("samples: %d\n", _samples);
    if (_backgrdImage) {
      trg.PrintF("background: %s\n", _backgrdImageFilename.c_str());
      trg.PrintF("backgroundXY: %f %f\n", _backgrdImageTranslate[0], _backgrdImageTranslate[1]);
    }
    for ( int i = 0; i < _items; i++)
      trg.PrintF("%f %f %f %d\n", _arr[i].X(), _arr[i].Y(), _arr[i].Z(),
                 _arr[i].getMultiplicity());
  } break;
  case v1_4: {
    trg.PrintF("version: 1.4\n");
    trg.PrintF("contact: 0 0 0\n");
    trg.PrintF("end: 0 0 0\n");
    trg.PrintF("heading: 0 1 0\n");
    trg.PrintF("up: 0 0 -1\n");
    trg.PrintF("size: 1\n");
    trg.PrintF("points: %d\n", _items);
    trg.PrintF("range: 0.0 1.0\n");
    trg.PrintF("dimension: 4\n");
    trg.PrintF("type: bspline\n");

    for (int i = 0; i < _items; i++)
      trg.PrintF("%f %f 0 %d\n", _arr[i].X(), _arr[i].Y(),
                 _arr[i].getMultiplicity());
  } break;
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

  for (int i = 0; i < _items; i++) {
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

  if (_isClosed)
    gls.Vertex(_arr[0]);
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

  if (_isEp) {
    if (_isClosed) {
      for (int j = 0; j < _arr[0].getMultiplicity(); ++j)
        ctrlpts.Add(_arr[0]);
    }

    glBegin(_isClosed ? GL_LINE_LOOP : GL_LINE_STRIP);

    {
      int num = ctrlpts.Count() * 12;
      for (int i = 0; i < num; i++) {
        WorldPoint wp = P(float(i) / num, ctrlpts);
        glVertex2f(wp.X(), wp.Y());
      }
      if (_isClosed)
        glVertex2f(ctrlpts[0].X(), ctrlpts[0].Y());
      else
        glVertex2f(ctrlpts[ctrlpts.Count() - 1].X(),
                   ctrlpts[ctrlpts.Count() - 1].Y());
    }

    glEnd();
  } else {
    GLlinestrip gls;

    if (_isClosed) {
      // Create the closed loop be adding the first three
      //  control points to the end of the list
      ctrlpts.Add(ctrlpts[0]);
      ctrlpts.Add(ctrlpts[1]);
      ctrlpts.Add(ctrlpts[2]);
    }

    int count = ctrlpts.Count();

    // The following is taken from "Computer Grapihics:
    //  Principles and Practice" (Foley et al.) section 11.2
    for (int i = 3; i < count; i++) {
      WorldPoint G[4] = {ctrlpts[i - 3], ctrlpts[i - 2], ctrlpts[i - 1],
                         ctrlpts[i]};

      for (double t = 0.0; t < 1.0; t += 0.1) {
        double x = Basis0(t) * G[0].X() + Basis1(t) * G[1].X() +
                   Basis2(t) * G[2].X() + Basis3(t) * G[3].X();
        double y = Basis0(t) * G[0].Y() + Basis1(t) * G[1].Y() +
                   Basis2(t) * G[2].Y() + Basis3(t) * G[3].Y();
        double z = Basis0(t) * G[0].Z() + Basis1(t) * G[1].Z() +
                   Basis2(t) * G[2].Z() + Basis3(t) * G[3].Z();

        gls.Vertex(WorldPoint(x, y, z));
      }
    }
  }
}

void Model::AddPoint(WorldPoint wp) {
  int num = 0;
  double dist = WorldLine(_arr[0], _arr[1]).DistanceTo(wp);

  for (int i = 1; i < _items - 1; i++) {
    double ldist = WorldLine(_arr[i], _arr[i + 1]).DistanceTo(wp);
    if (ldist < dist) {
      dist = ldist;
      num = i;
    }
  }

  if (_isClosed) {
    double ldist = WorldLine(_arr[_items - 1], _arr[0]).DistanceTo(wp);
    if (ldist < dist) {
      dist = ldist;
      num = _items - 1;
    }
  }

  _Insert(wp, num + 1);

  _hasChanged = true;
}

void Model::MovePoint(int i, WorldPoint wp) {
  assert(i >= 0);
  assert(i < _items);

  float z = _arr[i].Z();
  wp.setMultiplicity(_arr[i].getMultiplicity());
  wp.Z(z);
  _arr[i] = wp;

  _hasChanged = true;
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

void Model::PrepBSplineMatrix() {
  BSplineMat[0] = -1.0 / 6.0;
  BSplineMat[1] = 3.0 / 6.0;
  BSplineMat[2] = -3.0 / 6.0;
  BSplineMat[3] = 1.0 / 6.0;

  BSplineMat[4] = 3.0 / 6.0;
  BSplineMat[5] = -6.0 / 6.0;
  BSplineMat[6] = 3.0 / 6.0;
  BSplineMat[7] = 0.0 / 6.0;

  BSplineMat[8] = -3.0 / 6.0;
  BSplineMat[9] = 0.0 / 6.0;
  BSplineMat[10] = 3.0 / 6.0;
  BSplineMat[11] = 0.0 / 6.0;

  BSplineMat[12] = 1.0 / 6.0;
  BSplineMat[13] = 4.0 / 6.0;
  BSplineMat[14] = 1.0 / 6.0;
  BSplineMat[15] = 0.0 / 6.0;
}

double Model::Basis0(double t) const {
  return 1.0 / 6.0 * (-pow(t, 3) + 3 * pow(t, 2) - 3 * t + 1);
}

double Model::Basis1(double t) const {
  return 1.0 / 6.0 * (3 * pow(t, 3) - 6 * pow(t, 2) + 4);
}

double Model::Basis2(double t) const {
  return 1.0 / 6.0 * (-3 * pow(t, 3) + 3 * pow(t, 2) + 3 * t + 1);
}

double Model::Basis3(double t) const { return 1.0 / 6.0 * (pow(t, 3)); }

WorldPoint Model::P(double d, const DynArray<WorldPoint> &pts) const {
  const int n = pts.Count() - 1;
  const int t = 4;
  float u = d * (n - t + 2);
  WorldPoint sum;
  for (int k = 0; k <= n; ++k)
    sum += pts[k] * N(k, t, u, n);
  return sum;
}

double Model::N(int k, int t, float u, int n) const {
  if (t == 1)
    return Nk1(k, u, n);
  else
    return Nkt(k, t, u, n);
}

double Model::Nk1(int k, float u, int n) const {
  if (Uk(k, n) <= u && u < Uk(k + 1, n))
    return 1.0;
  return 0.0;
}

double Model::Nkt(int k, int t, float u, int n) const {
  double sum = 0.0;
  int div = Uk(k + t - 1, n) - Uk(k, n);
  if (div)
    sum = (u - Uk(k, n)) / div * N(k, t - 1, u, n);
  div = Uk(k + t, n) - Uk(k + 1, n);
  if (0 != div)
    sum += (Uk(k + t, n) - u) / div * N(k + 1, t - 1, u, n);
  return sum;
}

int Model::Uk(int j, int n) const {
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
