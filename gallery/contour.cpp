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



#include <cmath>
#include <fstream>

#include "contour.h"


using namespace std;
using namespace Qt;

ConItem::ConItem(Gallery *pGal, std::string fileName, QWidget *parent,
                 const char *name, Qt::WindowFlags f)
    : Item(pGal, fileName, parent, name, f), version(NOT_LOADED),
      isClosed(false) {
  filename = fileName;
  tmpDir = pGal->getTmpDir();
  pEditor = new ConEditor();
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

  show();
}

bool ConItem::load() {

  int num = 0, dimensions = 0;
  bool checkz = false;

  ifstream in(filename.c_str());
  if (!in || !in.good() || in.eof())
    return false;

  if (in.peek() == 'c') {

    // check cver 1 1
    string buff, name;
    int major, minor;
    in >> buff >> major >> minor >> ws;

    if (major != 1)
      return false;

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
      return false;
    }

    in >> buff >> ws;
    getline(in, name);

    pNameTxt->setText(name.c_str());

    int num_total;
    in >> buff >> num >> num_total >> ws;

    string type;
    in >> buff >> type >> ws;

    endPointInterpolation = false;

    switch (version) {
    case v1_1:
      if (type == string("closed"))
        isClosed = true;
      else
        isClosed = false;
      break;
    case v1_2:
    case v1_3:
      if (type[0] == 'c')
        isClosed = true;
      else
        isClosed = false;
      if (type[1] == 'e')
        endPointInterpolation = true;
      break;
    default:
      break;
    }

    if (version == v1_3) {
      in >> buff >> samples >> ws;
    } else
      samples = 0;

    // read info on background image (if exists)
    if (in.peek() == 'b') {
      string name;
      in >> buff >> name >> ws;
      
      float x, y;
      in >> buff >> x >> y >> ws;
    }

    // 'markers' are an experimental feature that will be considered for release in future versions
    // check for markers
    //if (in.peek() == 'm') {
    //  int numMarkers;
    //  in >> buff >> numMarkers >> ws;
    //  int id;
    //  float u, x, y;
    //  for (int i = 0; i < numMarkers; i++)
    //    in >> id >> u >> x >> y >> ws;
    //}

    
    points = PtVec();
    static double minx = 0.0, maxx = 0.0, miny = 0.0, maxy = 0.0;

    for (int i = 0; i < num; i++) {
      Point pt;

      in >> pt.x >> pt.y >> pt.z >> pt.multiplicity >> ws;

      if (i == 0) {
        minx = pt.x;
        maxx = pt.x;
        miny = pt.y;
        maxy = pt.y;
      } else {
        if (pt.x < minx)
          minx = pt.x;
        if (pt.x > maxx)
          maxx = pt.x;
        if (pt.y < miny)
          miny = pt.y;
        if (pt.y > maxy)
          maxy = pt.y;
      }

      max.x = maxx;
      max.y = maxy;
      min.x = minx;
      min.y = miny;

      points.push_back(pt);
    }

    center.x = ((max.x - min.x) / 2) + min.x;
    center.y = ((max.y - min.y) / 2) + min.y;
  } else if (in.peek() == 'v') {
    // check version 1.4
    string buff;

    in >> buff >> buff >> ws;
    if (buff != string("1.4"))
      return false;

    version = v1_4;

    in >> buff >> buff >> buff >> buff  // contact
        >> buff >> buff >> buff >> buff // end
        >> buff >> buff >> buff >> buff // heading
        >> buff >> buff >> buff >> buff // up
        >> buff >> buff                 // size
        >> buff >> num                  // points
        >> buff >> buff >> buff         // range
        >> buff >> buff                 // dimension
        >> buff >> buff >> ws;          // type

    points = PtVec();
    static double minx = 0.0, maxx = 0.0, miny = 0.0, maxy = 0.0;

    for (int i = 0; i < num; i++) {
      Point pt;

      in >> pt.x >> pt.y >> pt.z >> pt.multiplicity >> ws;

      if (i == 0) {
        minx = pt.x;
        maxx = pt.x;
        miny = pt.y;
        maxy = pt.y;
      } else {
        if (pt.x < minx)
          minx = pt.x;
        if (pt.x > maxx)
          maxx = pt.x;
        if (pt.y < miny)
          miny = pt.y;
        if (pt.y > maxy)
          maxy = pt.y;
      }

      max.x = maxx;
      max.y = maxy;
      min.x = minx;
      min.y = miny;

      points.push_back(pt);
    }

    center.x = ((max.x - min.x) / 2) + min.x;
    center.y = ((max.y - min.y) / 2) + min.y;
  } else {
    // original version

    version = ORIGINAL;

    string closed;
    in >> num >> dimensions >> closed >> ws;
    if (closed == string("closed"))
      isClosed = true;

    if (dimensions < 2 || dimensions > 3)
      return false;
    if (dimensions == 3)
      checkz = true;

    points = PtVec();
    static double minx = 0.0, maxx = 0.0, miny = 0.0, maxy = 0.0;

    for (int i = 0; i < num; i++) {
      Point pt;

      in >> pt.x >> pt.y >> ws;

      if (points.size() > 1 && pt.x == points[points.size() - 1].x &&
          pt.y == points[points.size() - 1].y)
        points[points.size() - 1].multiplicity++;
      else
        points.push_back(pt);

      if (checkz) {
        double z;
        in >> z >> ws;
      }

      if (i == 0) {
        minx = pt.x;
        maxx = pt.x;
        miny = pt.y;
        maxy = pt.y;
      } else {
        if (pt.x < minx)
          minx = pt.x;
        if (pt.x > maxx)
          maxx = pt.x;
        if (pt.y < miny)
          miny = pt.y;
        if (pt.y > maxy)
          maxy = pt.y;
      }

      max.x = maxx;
      max.y = maxy;
      min.x = minx;
      min.y = miny;
    }

    center.x = ((max.x - min.x) / 2) + min.x;
    center.y = ((max.y - min.y) / 2) + min.y;
  }
  return true;
}

void ConItem::save() {
  ofstream out(filename.c_str());
  if (!out || !out.good())
    return; 

  int num = 0;

  switch (version) {
  case ORIGINAL:
    /*
    for (unsigned int i = 0; i < points.size(); i++)
      num += points[i].multiplicity;

    out << num << " " << 2 << " " << (isClosed ? "closed" : "open") << endl;

    for (unsigned int i = 0; i < points.size(); i++)
      for (unsigned int j = 0; j < points[i].multiplicity; j++)
        out << points[i].x << " " << points[i].y << endl;
    break;
    */

  case NOT_LOADED:
  case v1_1:
    /*
    for (unsigned int i = 0; i < points.size(); i++)
      num += points[i].multiplicity;

    out << "cver 1 1" << endl
        << "name: " << pNameTxt->text().toStdString().c_str() << endl
        << "points: " << points.size() << " " << num << endl
        << "type: " << (isClosed ? "closed" : "open") << endl;

    for (unsigned int i = 0; i < points.size(); i++)
      out << points[i].x << " " << points[i].y << " " << points[i].z << " "
          << points[i].multiplicity << endl;
    break;
    */
  case v1_2:
    /*
    for (unsigned int i = 0; i < points.size(); i++)
      num += points[i].multiplicity;

    out << "cver 1 2" << endl
        << "name: " << pNameTxt->text().toStdString().c_str() << endl
        << "points: " << points.size() << " " << num << endl
        << "type: " << (isClosed ? "c" : "o")
        << (endPointInterpolation ? "e" : "r") << endl;

    for (unsigned int i = 0; i < points.size(); i++)
      out << points[i].x << " " << points[i].y << " " << points[i].z << " "
          << points[i].multiplicity << endl;
    break;
    */
  case v1_3:
    for (unsigned int i = 0; i < points.size(); i++)
      num += points[i].multiplicity;

    out << "cver 1 3" << endl
        << "name: " << pNameTxt->text().toStdString().c_str() << endl
        << "points: " << points.size() << " " << num << endl
        << "type: " << (isClosed ? "c" : "o")
        << (endPointInterpolation ? "e" : "r") << endl
        << "samples: " << samples << endl;

    for (unsigned int i = 0; i < points.size(); i++)
      out << points[i].x << " " << points[i].y << " " << points[i].z << " "
          << points[i].multiplicity << endl;
    break;

  case v1_4:
    out << "version: 1.4" << endl
        << "contact: 0 0 0" << endl
        << "end: 0 0 0" << endl
        << "heading: 0 1 0" << endl
        << "up: 0 0 -1" << endl
        << "size 1" << endl
        << "points: " << points.size() << endl
        << "range: 0.0 1.0" << endl
        << "dimension: 4" << endl
        << "type: bspline" << endl;

    for (unsigned int i = 0; i < points.size(); i++)
      out << points[i].x << " " << points[i].y << " " << points[i].z << " "
          << points[i].multiplicity << endl;
    break;
  }
}
/*
ConViewer::ConViewer(QWidget *parent, const char *name, Qt::WindowFlags f)
    : Viewer(parent, name, f), scale(double()) {}

void ConViewer::initializeGL() {
  glClearColor(ConConfig::background[0], ConConfig::background[1],
               ConConfig::background[2], 1.0);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  int w = width();
  int h = height();
  double ratio = double(w) / double(h);
  {
    double n = fabs(dynamic_cast<ConItem *>(pItem)->max.x -
                    dynamic_cast<ConItem *>(pItem)->center.x);
    double s = fabs(dynamic_cast<ConItem *>(pItem)->min.x -
                    dynamic_cast<ConItem *>(pItem)->center.x);
    double e = fabs(dynamic_cast<ConItem *>(pItem)->max.y -
                    dynamic_cast<ConItem *>(pItem)->center.y);
    double w = fabs(dynamic_cast<ConItem *>(pItem)->min.y -
                    dynamic_cast<ConItem *>(pItem)->center.y);

    scale = n;
    if (s > scale)
      scale = s;
    if (e > scale)
      scale = e;
    if (w > scale)
      scale = w;

    scale *= 1.1; // add some padding
  }

  if (ratio > 1.0)
    glOrtho(dynamic_cast<ConItem *>(pItem)->center.x - scale * ratio,
            dynamic_cast<ConItem *>(pItem)->center.x + scale * ratio,
            dynamic_cast<ConItem *>(pItem)->center.y - scale,
            dynamic_cast<ConItem *>(pItem)->center.y + scale, -1.0, 1.0);
  else
    glOrtho(dynamic_cast<ConItem *>(pItem)->center.x - scale,
            dynamic_cast<ConItem *>(pItem)->center.x + scale,
            dynamic_cast<ConItem *>(pItem)->center.y - scale / ratio,
            dynamic_cast<ConItem *>(pItem)->center.y + scale / ratio, -1.0,
            1.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glViewport(0, 0, width(), height());
}

void ConViewer::resizeGL(int, int) {}

void ConViewer::paintGL() {
  initializeGL();

  int w = width();
  int h = height();
  double ratio = double(w) / double(h);

  float maxX, minX, maxY, minY;
  if (ratio > 1.0) {
    minX = dynamic_cast<ConItem *>(pItem)->center.x - scale * ratio;
    maxX = dynamic_cast<ConItem *>(pItem)->center.x + scale * ratio;
    minY = dynamic_cast<ConItem *>(pItem)->center.y - scale;
    maxY = dynamic_cast<ConItem *>(pItem)->center.y + scale;
  } else {
    minX = dynamic_cast<ConItem *>(pItem)->center.x - scale;
    maxX = dynamic_cast<ConItem *>(pItem)->center.x + scale;
    minY = dynamic_cast<ConItem *>(pItem)->center.y - scale / ratio;
    maxY = dynamic_cast<ConItem *>(pItem)->center.y + scale / ratio;
  }

  glClear(GL_COLOR_BUFFER_BIT);
  // drawing axis

  glColor3f(1, 0, 0);
  glBegin(GL_LINE_STRIP);
  glVertex2d(minX - 0.1 - _offSet, 0);
  glVertex2d(maxX + 1.1 - _offSet, 0);
  glEnd();
  glColor3f(0, 1, 0);
  glBegin(GL_LINE_STRIP);
  glVertex2d(0 - _offSet, minY - 0.1);
  glVertex2d(0 - _offSet, maxY + 1.1);
  glEnd();

  ConItem::PtVec ctrlpts;
  for (unsigned int i = 0; i < dynamic_cast<ConItem *>(pItem)->points.size();
       i++)
    for (unsigned int j = 0;
         j < dynamic_cast<ConItem *>(pItem)->points[i].multiplicity; j++)
      ctrlpts.push_back(dynamic_cast<ConItem *>(pItem)->points[i]);

  glColor3f(ConConfig::curve[0], ConConfig::curve[1], ConConfig::curve[2]);
  if (dynamic_cast<ConItem *>(pItem)->endPointInterpolation) {
    // Render with this end-point interpolation thing
    if (dynamic_cast<ConItem *>(pItem)->isClosed) {
      for (unsigned int j = 0;
           j < dynamic_cast<ConItem *>(pItem)->points.back().multiplicity; j++)
        ctrlpts.push_back(dynamic_cast<ConItem *>(pItem)->points.back());
    }

    unsigned int n = ctrlpts.size() * 12;

    glBegin(dynamic_cast<ConItem *>(pItem)->isClosed ? GL_LINE_LOOP
                                                     : GL_LINE_STRIP);
    for (unsigned int i = 0; i < n; ++i) {
      ConItem::Point p = P(double(i) / double(n), ctrlpts);
      glVertex2d(p.x, p.y);
    }
    if (dynamic_cast<ConItem *>(pItem)->isClosed) {
      glVertex2d(ctrlpts[0].x, ctrlpts[0].y);
    } else {
      glVertex2d(ctrlpts.back().x, ctrlpts.back().y);
    }
    glEnd();
  } else {
    // Render as a regular B-spline
    if (dynamic_cast<ConItem *>(pItem)->isClosed) {
      ctrlpts.push_back(ctrlpts[0]);
      ctrlpts.push_back(ctrlpts[1]);
      ctrlpts.push_back(ctrlpts[2]);
    }

    glBegin(GL_LINE_STRIP);
    for (unsigned int i = 3; i < ctrlpts.size(); i++) {
      ConItem::Point G[4] = {ctrlpts[i - 3], ctrlpts[i - 2], ctrlpts[i - 1],
                             ctrlpts[i]};

      for (double t = 0.0; t < 1.0; t += 0.1) {
        double x = Basis0(t) * G[0].x + Basis1(t) * G[1].x +
                   Basis2(t) * G[2].x + Basis3(t) * G[3].x;
        double y = Basis0(t) * G[0].y + Basis1(t) * G[1].y +
                   Basis2(t) * G[2].y + Basis3(t) * G[3].y;

        glVertex2d(x, y);
      }
    }
    glEnd();
  }

  glFlush();
}

double ConViewer::Basis0(double t) {
  return 1.0 / 6.0 * (-pow(t, 3) + 3 * pow(t, 2) - 3 * t + 1);
}

double ConViewer::Basis1(double t) {
  return 1.0 / 6.0 * (3 * pow(t, 3) - 6 * pow(t, 2) + 4);
}

double ConViewer::Basis2(double t) {
  return 1.0 / 6.0 * (-3 * pow(t, 3) + 3 * pow(t, 2) + 3 * t + 1);
}

double ConViewer::Basis3(double t) { return 1.0 / 6.0 * (pow(t, 3)); }

ConItem::Point ConViewer::P(double d, const ConItem::PtVec &pts) {
  const int n = pts.size() - 1;
  const int t = 4;
  float u = d * (n - t + 2);
  ConItem::Point sum;
  for (int k = 0; k <= n; ++k)
    sum += pts[k] * N(k, t, u, n);
  return sum;
}

double ConViewer::N(int k, int t, float u, int n) {
  if (t == 1)
    return Nk1(k, u, n);
  else
    return Nkt(k, t, u, n);
}

double ConViewer::Nk1(int k, float u, int n) {
  if (Uk(k, n) <= u && u < Uk(k + 1, n))
    return 1.0;
  return 0.0;
}

double ConViewer::Nkt(int k, int t, float u, int n) {
  double sum = 0.0;
  int div = Uk(k + t - 1, n) - Uk(k, n);
  if (div)
    sum = (u - Uk(k, n)) / div * N(k, t - 1, u, n);
  div = Uk(k + t, n) - Uk(k + 1, n);
  if (0 != div)
    sum += (Uk(k + t, n) - u) / div * N(k + 1, t - 1, u, n);
  return sum;
}

int ConViewer::Uk(int j, int n) {
  const int t = 4;
  if (j < t)
    return 0;
  if (j > n)
    return n - t + 2;
  return j - t + 1;
}
*/
void ConItem::readConfigFile() {
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


ConItem::ConItem(Gallery* pGal, std::string fileName,std::string selection_name) : Item(pGal, fileName, pGal, selection_name.c_str()), version(NOT_LOADED),
      isClosed(false) {
  filename = fileName;
  tmpDir = pGal->getTmpDir();
  pEditor = new ConEditor();
  _background[0] = 0.0;
  _background[1] = 0.0;
  _background[2] = 0.0;
  _curve[0] = 1.0;
  _curve[1] = 1.0;
  _curve[2] = 0.0;
  _visible = true;
  //Initialize window
  _width = pGal->getItemWidth();
  _height = pGal->getItemHeight();
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

  //    if (extension != string(".con"))
  //    filename += ".con";
  out.open(filename.c_str());
  out << "cver 1 3" << endl
      << "name: " << selection_name << endl
      << "points: 4 4" << endl
      << "type: cr" << endl
      << "samples: 8" << endl
      << "0.400000 0.400000 0.000000 1" << endl
      << "0.400000 -0.400000 0.000000 1" << endl
      << "-0.400000 -0.400000 0.000000 1" << endl
      << "-0.400000 0.400000 0.000000 1" << endl;
  load();
}
