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



#include <algorithm>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "config.h"
#include "model.h"

using namespace std;

Model::Model()
    : _upVector(0.0, 1.0, 0.0), _headingVector(0.0, 0.0, 1.0), _size(1.0),
      _scale(1.0), _connectionPoint(new BezierPoint(0.0, 0.0, 0.0)),
      _endPoint(new BezierPoint(0.0, 0.0, 0.0)), _c(NONE), _max(1.0, 1.0, 1.0),
      _min(-1.0, -1.0, -1.0), _useSelectSize(false), _hasChanged(false),
      _interpolateMerge(true), _showSplitable(false), _controlpoints(16),
      _patches(1) {
  _prepareColours();
}

Model::~Model() {
  _destroyData();
  delete _connectionPoint;
  delete _endPoint;
}

void Model::load(string filename) {
  // Rewrite me, I'm buggy.  I do not tolerate
  // malformed datafiles.

  _destroyData();

  ifstream in(filename.c_str());
  if (!in)
    return;

  bool sizefound = false;

  // bounding box
  if (!isalpha(in.peek())) {
    double minx, miny, minz, maxx, maxy, maxz;
    in >> minx >> maxx >> miny >> maxy >> minz >> maxz >> ws;

    _min.Set(minx, miny, minz);
    _max.Set(maxx, maxy, maxz);
  }

  int patchindex = 0;
  map<string, int> patchnames;
  multimap<int, string> attachmentsAL;
  multimap<int, string> attachmentsA;
  multimap<int, string> attachmentsAR;
  multimap<int, string> attachmentsL;
  multimap<int, string> attachmentsR;
  multimap<int, string> attachmentsBL;
  multimap<int, string> attachmentsB;
  multimap<int, string> attachmentsBR;

  while (!in.eof()) {
    string buf;
    in >> ws >> buf >> ws;

    if (buf == "PRECISION") {
      // can ignore this, it is not part of the specification
      getline(in, buf);
      in >> ws;
    }

    else if (buf == "CONTACT") {
      double x, y, z;
      in >> buf >> buf >> x >> buf >> y >> buf >> z >> ws;
      _connectionPoint->Set(x, y, z);
    }

    else if (buf == "END") {
      double x, y, z;
      in >> buf >> buf >> x >> buf >> y >> buf >> z >> ws;
      _endPoint->Set(x, y, z);
    }

    else if (buf == "HEADING") {
      double x, y, z;
      in >> buf >> x >> buf >> y >> buf >> z >> ws;
      _headingVector.Set(x, y, z);
    }

    else if (buf == "UP") {
      double x, y, z;
      in >> buf >> x >> buf >> y >> buf >> z >> ws;
      _upVector.Set(x, y, z);
    }

    else if (buf == "SIZE:") {
      in >> _size >> ws;
      sizefound = true;
    }

    else {
      // get the patch name
      string name(buf);
      patchnames[name] = patchindex;
      BezierPatch *patch = new BezierPatch();
      patch->name = name;

      // discard colour info if it exists
      if (in.peek() == 'T')
        in >> buf >> buf >> patch->top_colour >> buf >> patch->top_diffuse >>
            buf >> buf >> patch->bottom_colour >> buf >>
            patch->bottom_diffuse >> ws;

      // read in the patch connections;
      in >> buf >> buf >> ws;
      if (buf[0] != '~')
        attachmentsAL.insert(make_pair(patchindex, buf));

      in >> buf >> buf >> ws;
      if (buf[0] != '~')
        attachmentsA.insert(make_pair(patchindex, buf));

      in >> buf >> buf >> ws;
      if (buf[0] != '~')
        attachmentsAR.insert(make_pair(patchindex, buf));

      in >> buf >> buf >> ws;
      if (buf[0] != '~')
        attachmentsL.insert(make_pair(patchindex, buf));

      in >> buf >> buf >> ws;
      if (buf[0] != '~')
        attachmentsR.insert(make_pair(patchindex, buf));

      in >> buf >> buf >> ws;
      if (buf[0] != '~')
        attachmentsBL.insert(make_pair(patchindex, buf));

      in >> buf >> buf >> ws;
      if (buf[0] != '~')
        attachmentsB.insert(make_pair(patchindex, buf));

      in >> buf >> buf >> ws;
      if (buf[0] != '~')
        attachmentsBR.insert(make_pair(patchindex, buf));

      // read in the coordinates
      for (int i = 0; i < 16; i++) {
        double x, y, z;
        in >> x >> y >> z >> ws;
        BezierPoint *bp = new BezierPoint(x, y, z);
        bp->selectedForMerge = false;
        bp->patches.insert(patch);
        patch->points[i] = bp;
        _controlpoints.Add(bp);
      }

      _patches.Add(patch);
      patchindex++;
    }
    in >> ws;
  }

  // attach the patches
  {
    multimap<int, string>::iterator index;

    for (index = attachmentsAL.begin(); index != attachmentsAL.end(); index++)
      _attachAL(_patches[index->first], _patches[patchnames[index->second]]);
    for (index = attachmentsA.begin(); index != attachmentsA.end(); index++)
      _attachA(_patches[index->first], _patches[patchnames[index->second]]);
    for (index = attachmentsAR.begin(); index != attachmentsAR.end(); index++)
      _attachAR(_patches[index->first], _patches[patchnames[index->second]]);
    for (index = attachmentsL.begin(); index != attachmentsL.end(); index++)
      _attachL(_patches[index->first], _patches[patchnames[index->second]]);
    for (index = attachmentsR.begin(); index != attachmentsR.end(); index++)
      _attachR(_patches[index->first], _patches[patchnames[index->second]]);
    for (index = attachmentsBL.begin(); index != attachmentsBL.end(); index++)
      _attachBL(_patches[index->first], _patches[patchnames[index->second]]);
    for (index = attachmentsB.begin(); index != attachmentsB.end(); index++)
      _attachB(_patches[index->first], _patches[patchnames[index->second]]);
    for (index = attachmentsBR.begin(); index != attachmentsBR.end(); index++)
      _attachBR(_patches[index->first], _patches[patchnames[index->second]]);
  }

  if (!sizefound) {
    double x = _max.X() - _min.X();
    double y = _max.Z() - _min.Y();
    double z = _max.Y() - _min.Z();

    _scale = x;
    if (y > _scale)
      _scale = y;
    if (z > _scale)
      _scale = z;
  } else {
    _scale = _size;
  }
}

void Model::save(string filename) {
  ofstream out(filename.c_str(), ios::out | ios::trunc);

  if (!out || !out.good() || out.bad())
    throw FileExc();

  double minx = 0.0, maxx = 0.0, miny = 0.0, maxy = 0.0, minz = 0.0, maxz = 0.0;

  for (int i = 0; i < _controlpoints.Count(); i++) {
    if (_controlpoints[i]->X() < minx)
      minx = _controlpoints[i]->X();
    if (_controlpoints[i]->X() > maxx)
      maxx = _controlpoints[i]->X();
    if (_controlpoints[i]->Y() < miny)
      miny = _controlpoints[i]->Y();
    if (_controlpoints[i]->Y() > maxy)
      maxy = _controlpoints[i]->Y();
    if (_controlpoints[i]->Z() < minz)
      minz = _controlpoints[i]->Z();
    if (_controlpoints[i]->Z() > maxz)
      maxz = _controlpoints[i]->Z();
  }

  out << minx << " " << maxx << "\t" << miny << " " << maxy << "\t" << minz
      << " " << maxz << endl
      << "CONTACT POINT  X: " << _connectionPoint->X()
      << " Y: " << _connectionPoint->Y() << " Z: " << _connectionPoint->Z()
      << endl
      << "END POINT      X: " << _endPoint->X() << " Y: " << _endPoint->Y()
      << " Z: " << _endPoint->Z() << endl
      << "HEADING        X: " << _headingVector.X()
      << " Y: " << _headingVector.Y() << " Z: " << _headingVector.Z() << endl
      << "UP             X: " << _upVector.X() << " Y: " << _upVector.Y()
      << " Z: " << _upVector.Z() << endl
      << "SIZE: " << _size << endl;

  for (int i = 0; i < _patches.Count(); i++) {
    //out << "Patch_" << i << endl;
    out << _patches[i]->name << endl;

    out.setf(ios::showpoint);
    out.precision(2);

    out << "TOP COLOR: " << _patches[i]->top_colour << ' '
        << "DIFFUSE: " << _patches[i]->top_diffuse << ' '
        << "BOTTOM COLOR: " << _patches[i]->bottom_colour << ' '
        << "DIFFUSE: " << _patches[i]->bottom_diffuse << endl;

    out.unsetf(ios::showpoint);
    out.precision(7);

    out << "AL: ";
    if (_patches[i]->aboveleft)
      out << "Patch_" << _patches.find(_patches[i]->aboveleft);
    else
      out << "~";

    out << " A: ";
    if (_patches[i]->above)
      out << "Patch_" << _patches.find(_patches[i]->above);
    else
      out << "~";

    out << " AR: ";
    if (_patches[i]->aboveright)
      out << "Patch_" << _patches.find(_patches[i]->aboveright);
    else
      out << "~";

    out << endl;
    out << "L: ";
    if (_patches[i]->left)
      out << "Patch_" << _patches.find(_patches[i]->left);
    else
      out << "~";

    out << " R: ";
    if (_patches[i]->right)
      out << "Patch_" << _patches.find(_patches[i]->right);
    else
      out << "~";

    out << endl;
    out << "BL: ";
    if (_patches[i]->belowleft)
      out << "Patch_" << _patches.find(_patches[i]->belowleft);
    else
      out << "~";

    out << " B: ";
    if (_patches[i]->below)
      out << "Patch_" << _patches.find(_patches[i]->below);
    else
      out << "~";

    out << " BR: ";
    if (_patches[i]->belowright)
      out << "Patch_" << _patches.find(_patches[i]->belowright);
    else
      out << "~";

    for (int j = 0; j < 16; j++) {
      if (!(j % 4))
        out << endl;
      out << _patches[i]->points[j]->X() << " " << _patches[i]->points[j]->Y()
          << " " << _patches[i]->points[j]->Z() << "\t";
    }
    out << endl;
  }

  _hasChanged = false;
}

double Model::findLargestWidth() {
  if (_controlpoints.Count() == 0)
    return 1.5;

  double maxx = 0.0;
  double minx = 0.0;
  double maxy = 0.0;
  double miny = 0.0;
  double maxz = 0.0;
  double minz = 0.0;

  for (int i = 0; i < _controlpoints.Count(); i++) {
    if (_controlpoints[i]->X() > maxx)
      maxx = _controlpoints[i]->X();
    if (_controlpoints[i]->X() < minx)
      minx = _controlpoints[i]->X();
    if (_controlpoints[i]->Y() > maxy)
      maxy = _controlpoints[i]->Y();
    if (_controlpoints[i]->Y() < miny)
      miny = _controlpoints[i]->Y();
    if (_controlpoints[i]->Z() > maxz)
      maxz = _controlpoints[i]->Z();
    if (_controlpoints[i]->Z() < minz)
      minz = _controlpoints[i]->Z();
  }

  return sqrt(pow(maxx - minx, 2) + pow(maxy - miny, 2) + pow(maxz - minz, 2));
}

void Model::createPatch() {
  _hasChanged = true;

  BezierPatch *patch = new BezierPatch();
  patch->name = "Patch_";
  patch->name += (QString().setNum(_patches.Count())).toStdString().c_str();

  BezierPoint *pt;

  pt = new BezierPoint(-0.75 * _scale, 0.75 * _scale, 0.0);
  _controlpoints.Add(pt);
  patch->points[0] = pt;
  pt->patches.insert(patch);
  pt->selectedForMerge = false;
  pt = new BezierPoint(-0.25 * _scale, 0.75 * _scale, 0.0);
  _controlpoints.Add(pt);
  patch->points[1] = pt;
  pt->patches.insert(patch);
  pt->selectedForMerge = false;
  pt = new BezierPoint(0.25 * _scale, 0.75 * _scale, 0.0);
  _controlpoints.Add(pt);
  patch->points[2] = pt;
  pt->patches.insert(patch);
  pt->selectedForMerge = false;
  pt = new BezierPoint(0.75 * _scale, 0.75 * _scale, 0.0);
  _controlpoints.Add(pt);
  patch->points[3] = pt;
  pt->patches.insert(patch);
  pt->selectedForMerge = false;

  pt = new BezierPoint(-0.75 * _scale, 0.25 * _scale, 0.0);
  _controlpoints.Add(pt);
  patch->points[4] = pt;
  pt->patches.insert(patch);
  pt->selectedForMerge = false;
  pt = new BezierPoint(-0.25 * _scale, 0.25 * _scale, 0.0);
  _controlpoints.Add(pt);
  patch->points[5] = pt;
  pt->patches.insert(patch);
  pt->selectedForMerge = false;
  pt = new BezierPoint(0.25 * _scale, 0.25 * _scale, 0.0);
  _controlpoints.Add(pt);
  patch->points[6] = pt;
  pt->patches.insert(patch);
  pt->selectedForMerge = false;
  pt = new BezierPoint(0.75 * _scale, 0.25 * _scale, 0.0);
  _controlpoints.Add(pt);
  patch->points[7] = pt;
  pt->patches.insert(patch);
  pt->selectedForMerge = false;

  pt = new BezierPoint(-0.75 * _scale, -0.25 * _scale, 0.0);
  _controlpoints.Add(pt);
  patch->points[8] = pt;
  pt->patches.insert(patch);
  pt->selectedForMerge = false;
  pt = new BezierPoint(-0.25 * _scale, -0.25 * _scale, 0.0);
  _controlpoints.Add(pt);
  patch->points[9] = pt;
  pt->patches.insert(patch);
  pt->selectedForMerge = false;
  pt = new BezierPoint(0.25 * _scale, -0.25 * _scale, 0.0);
  _controlpoints.Add(pt);
  patch->points[10] = pt;
  pt->patches.insert(patch);
  pt->selectedForMerge = false;
  pt = new BezierPoint(0.75 * _scale, -0.25 * _scale, 0.0);
  _controlpoints.Add(pt);
  patch->points[11] = pt;
  pt->patches.insert(patch);
  pt->selectedForMerge = false;

  pt = new BezierPoint(-0.75 * _scale, -0.75 * _scale, 0.0);
  _controlpoints.Add(pt);
  patch->points[12] = pt;
  pt->patches.insert(patch);
  pt->selectedForMerge = false;
  pt = new BezierPoint(-0.25 * _scale, -0.75 * _scale, 0.0);
  _controlpoints.Add(pt);
  patch->points[13] = pt;
  pt->patches.insert(patch);
  pt->selectedForMerge = false;
  pt = new BezierPoint(0.25 * _scale, -0.75 * _scale, 0.0);
  _controlpoints.Add(pt);
  patch->points[14] = pt;
  pt->patches.insert(patch);
  pt->selectedForMerge = false;
  pt = new BezierPoint(0.75 * _scale, -0.75 * _scale, 0.0);
  _controlpoints.Add(pt);
  patch->points[15] = pt;
  pt->patches.insert(patch);
  pt->selectedForMerge = false;

  _patches.Add(patch);
}

int Model::getPointCount() {
  return 1   // for the connection point
         + 1 // for the end point
         + _controlpoints.Count();
}

void Model::drawGrid() {
  glLineWidth(1.0);
  glColor4fv(_gridColour);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, _gridColour);
  double inc = _scale / 10.0;
  glBegin(GL_LINES);
  for (double i = -_scale; i < _scale; i += inc) {
    glVertex3f(i, 0.0, -_scale);
    glVertex3f(i, 0.0, _scale);
  }
  glVertex3f(_scale, 0.0, -_scale);
  glVertex3f(_scale, 0.0, _scale);

  for (double i = -_scale; i < _scale; i += inc) {
    glVertex3f(-_scale, 0.0, i);
    glVertex3f(_scale, 0.0, i);
  }
  glVertex3f(-_scale, 0.0, _scale);
  glVertex3f(_scale, 0.0, _scale);
  glEnd();
}

void Model::drawAxes() {
  glLineWidth(2.0);
  glBegin(GL_LINES);
  glColor4fv(_xAxisColour);
  glVertex3f(0.0, 0.0, 0.0);
  glVertex3f(_scale / 2.0, 0.0, 0.0);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, _xAxisColour);

  glColor4fv(_yAxisColour);
  glVertex3f(0.0, 0.0, 0.0);
  glVertex3f(0.0, _scale / 2.0, 0.0);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, _yAxisColour);

  glColor4fv(_zAxisColour);
  glVertex3f(0.0, 0.0, 0.0);
  glVertex3f(0.0, 0.0, _scale / 2.0);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, _zAxisColour);
  glEnd();
}

void Model::drawUpVector() {
  glLineWidth(3.0);
  glColor4fv(_upVectorColour);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, _upVectorColour);
  glBegin(GL_LINES);
  glVertex3f(_connectionPoint->X(), _connectionPoint->Y(),
             _connectionPoint->Z());
  glVertex3f(_connectionPoint->X() + _upVector.X() * _scale,
             _connectionPoint->Y() + _upVector.Y() * _scale,
             _connectionPoint->Z() + _upVector.Z() * _scale);
  glEnd();
}

void Model::drawHeadingVector() {
  glLineWidth(3.0);
  glColor4fv(_headingVectorColour);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, _headingVectorColour);
  glBegin(GL_LINES);
  glVertex3f(_connectionPoint->X(), _connectionPoint->Y(),
             _connectionPoint->Z());
  glVertex3f(_connectionPoint->X() + _headingVector.X() * _scale,
             _connectionPoint->Y() + _headingVector.Y() * _scale,
             _connectionPoint->Z() + _headingVector.Z() * _scale);
  glEnd();
}

void Model::drawConnectPoint() {
  if (_useSelectSize)
    glPointSize(Config::getSelectionSize());
  else
    glPointSize(Config::getPointSize());
  glColor4fv(_connectPointColour);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, _connectPointColour);
  glLoadName(0);
  glBegin(GL_POINTS);
  glVertex3f(_connectionPoint->X(), _connectionPoint->Y(),
             _connectionPoint->Z());
  glEnd();
}

void Model::drawEndPoint() {
  if (_useSelectSize)
    glPointSize(Config::getSelectionSize());
  else
    glPointSize(Config::getPointSize());
  glColor4fv(_endPointColour);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, _endPointColour);
  glLoadName(1);
  glBegin(GL_POINTS);
  glVertex3f(_endPoint->X(), _endPoint->Y(), _endPoint->Z());
  glEnd();
}

void Model::drawControlPoints() {
  if (_useSelectSize)
    glPointSize(Config::getSelectionSize());
  else
    glPointSize(Config::getPointSize());
  glColor4fv(_controlPointsColour);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, _controlPointsColour);
  for (int i = 0; i < _controlpoints.Count(); i++) {
    glLoadName(i + 2);
    glBegin(GL_POINTS);
    if (_controlpoints[i]->selectedForMerge) {
      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, _selectedForMergeColour);
      glColor4fv(_selectedForMergeColour);
      glVertex3d(_controlpoints[i]->X(), _controlpoints[i]->Y(),
                 _controlpoints[i]->Z());
      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, _controlPointsColour);
      glColor4fv(_controlPointsColour);
    } else if (_controlpoints[i]->sticky) {
      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, _stickyColour);
      glColor4fv(_stickyColour);
      glVertex3d(_controlpoints[i]->X(), _controlpoints[i]->Y(),
                 _controlpoints[i]->Z());
      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, _controlPointsColour);
      glColor4fv(_controlPointsColour);
    } else if (_controlpoints[i]->selected) {
      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, _selectedColour);
      glColor4fv(_selectedColour);
      glVertex3d(_controlpoints[i]->X(), _controlpoints[i]->Y(),
                 _controlpoints[i]->Z());
      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, _controlPointsColour);
      glColor4fv(_controlPointsColour);
    } else if (_showSplitable && _controlpoints[i]->patches.size() > 1) {
      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, _splitColour);
      glColor4fv(_splitColour);
      glVertex3d(_controlpoints[i]->X(), _controlpoints[i]->Y(),
                 _controlpoints[i]->Z());
      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, _controlPointsColour);
      glColor4fv(_controlPointsColour);
    } else {
      glVertex3d(_controlpoints[i]->X(), _controlpoints[i]->Y(),
                 _controlpoints[i]->Z());
    }
    glEnd();
  }
}

void Model::drawWireModel() {
  glLineWidth(1.0);
  glPolygonMode(GL_FRONT, GL_LINE);

  for (int i = 0; i < _patches.Count(); i++) {
    if (_patches[i]->highlight) {
      glColor4fv(_wireHLColour);
      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, _wireHLColour);
    } else {
      glColor4fv(_wireModelColour);
      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, _wireModelColour);
    }

    _prepareEvaluator(*_patches[i]);
    glEvalMesh2(GL_FILL, 0, Config::getUDivs(), 0, Config::getVDivs());

    BezierPatch bp;
    bp.points[0] = _patches[i]->points[3];
    bp.points[1] = _patches[i]->points[2];
    bp.points[2] = _patches[i]->points[1];
    bp.points[3] = _patches[i]->points[0];

    bp.points[4] = _patches[i]->points[7];
    bp.points[5] = _patches[i]->points[6];
    bp.points[6] = _patches[i]->points[5];
    bp.points[7] = _patches[i]->points[4];

    bp.points[8] = _patches[i]->points[11];
    bp.points[9] = _patches[i]->points[10];
    bp.points[10] = _patches[i]->points[9];
    bp.points[11] = _patches[i]->points[8];

    bp.points[12] = _patches[i]->points[15];
    bp.points[13] = _patches[i]->points[14];
    bp.points[14] = _patches[i]->points[13];
    bp.points[15] = _patches[i]->points[12];

    _prepareEvaluator(bp);
    glEvalMesh2(GL_FILL, 0, Config::getUDivs(), 0, Config::getVDivs());
  }
  glPolygonMode(GL_FRONT, GL_FILL);
}

void Model::drawSolidModel() {
  glLineWidth(1.0);
  glColor4fv(_solidModelColour);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, _solidModelColour);

  for (int i = 0; i < _patches.Count(); i++) {
    if (_patches[i]->highlight) {
      glColor4fv(_solidHLColour);
      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, _solidHLColour);
    } else {
      glColor4fv(_solidModelColour);
      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, _solidModelColour);
    }

    _prepareEvaluator(*_patches[i]);
    glEvalMesh2(GL_FILL, 0, Config::getUDivs(), 0, Config::getVDivs());

    BezierPatch bp;
    bp.points[0] = _patches[i]->points[3];
    bp.points[1] = _patches[i]->points[2];
    bp.points[2] = _patches[i]->points[1];
    bp.points[3] = _patches[i]->points[0];

    bp.points[4] = _patches[i]->points[7];
    bp.points[5] = _patches[i]->points[6];
    bp.points[6] = _patches[i]->points[5];
    bp.points[7] = _patches[i]->points[4];

    bp.points[8] = _patches[i]->points[11];
    bp.points[9] = _patches[i]->points[10];
    bp.points[10] = _patches[i]->points[9];
    bp.points[11] = _patches[i]->points[8];

    bp.points[12] = _patches[i]->points[15];
    bp.points[13] = _patches[i]->points[14];
    bp.points[14] = _patches[i]->points[13];
    bp.points[15] = _patches[i]->points[12];

    _prepareEvaluator(bp);
    glEvalMesh2(GL_FILL, 0, Config::getUDivs(), 0, Config::getVDivs());
  }
}

void Model::drawControlPoly() {
  glLineWidth(2.0);
  glColor4fv(_controlPolyColour);

  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, _controlPolyColour);
  for (int i = 0; i < _patches.Count(); i++) {

    if (_patches[i]->highlight) {
      glColor4fv(_controlPolyInColour);

      glBegin(GL_LINE_STRIP);
      glVertex3f(_patches[i]->points[4]->X(), _patches[i]->points[4]->Y(),
                 _patches[i]->points[4]->Z());
      glVertex3f(_patches[i]->points[5]->X(), _patches[i]->points[5]->Y(),
                 _patches[i]->points[5]->Z());
      glVertex3f(_patches[i]->points[6]->X(), _patches[i]->points[6]->Y(),
                 _patches[i]->points[6]->Z());
      glVertex3f(_patches[i]->points[7]->X(), _patches[i]->points[7]->Y(),
                 _patches[i]->points[7]->Z());
      glEnd();
      glBegin(GL_LINE_STRIP);
      glVertex3f(_patches[i]->points[8]->X(), _patches[i]->points[8]->Y(),
                 _patches[i]->points[8]->Z());
      glVertex3f(_patches[i]->points[9]->X(), _patches[i]->points[9]->Y(),
                 _patches[i]->points[9]->Z());
      glVertex3f(_patches[i]->points[10]->X(), _patches[i]->points[10]->Y(),
                 _patches[i]->points[10]->Z());
      glVertex3f(_patches[i]->points[11]->X(), _patches[i]->points[11]->Y(),
                 _patches[i]->points[11]->Z());
      glEnd();
      glBegin(GL_LINE_STRIP);
      glVertex3f(_patches[i]->points[1]->X(), _patches[i]->points[1]->Y(),
                 _patches[i]->points[1]->Z());
      glVertex3f(_patches[i]->points[5]->X(), _patches[i]->points[5]->Y(),
                 _patches[i]->points[5]->Z());
      glVertex3f(_patches[i]->points[9]->X(), _patches[i]->points[9]->Y(),
                 _patches[i]->points[9]->Z());
      glVertex3f(_patches[i]->points[13]->X(), _patches[i]->points[13]->Y(),
                 _patches[i]->points[13]->Z());
      glEnd();
      glBegin(GL_LINE_STRIP);
      glVertex3f(_patches[i]->points[2]->X(), _patches[i]->points[2]->Y(),
                 _patches[i]->points[2]->Z());
      glVertex3f(_patches[i]->points[6]->X(), _patches[i]->points[6]->Y(),
                 _patches[i]->points[6]->Z());
      glVertex3f(_patches[i]->points[10]->X(), _patches[i]->points[10]->Y(),
                 _patches[i]->points[10]->Z());
      glVertex3f(_patches[i]->points[14]->X(), _patches[i]->points[14]->Y(),
                 _patches[i]->points[14]->Z());
      glEnd();

      glColor4fv(_controlPolyHLColour);
    } else {
      glColor4fv(_controlPolyColour);
    }

    glBegin(GL_LINE_LOOP);
    glVertex3f(_patches[i]->points[0]->X(), _patches[i]->points[0]->Y(),
               _patches[i]->points[0]->Z());
    glVertex3f(_patches[i]->points[1]->X(), _patches[i]->points[1]->Y(),
               _patches[i]->points[1]->Z());
    glVertex3f(_patches[i]->points[2]->X(), _patches[i]->points[2]->Y(),
               _patches[i]->points[2]->Z());
    glVertex3f(_patches[i]->points[3]->X(), _patches[i]->points[3]->Y(),
               _patches[i]->points[3]->Z());
    glVertex3f(_patches[i]->points[7]->X(), _patches[i]->points[7]->Y(),
               _patches[i]->points[7]->Z());
    glVertex3f(_patches[i]->points[11]->X(), _patches[i]->points[11]->Y(),
               _patches[i]->points[11]->Z());
    glVertex3f(_patches[i]->points[15]->X(), _patches[i]->points[15]->Y(),
               _patches[i]->points[15]->Z());
    glVertex3f(_patches[i]->points[14]->X(), _patches[i]->points[14]->Y(),
               _patches[i]->points[14]->Z());
    glVertex3f(_patches[i]->points[13]->X(), _patches[i]->points[13]->Y(),
               _patches[i]->points[13]->Z());
    glVertex3f(_patches[i]->points[12]->X(), _patches[i]->points[12]->Y(),
               _patches[i]->points[12]->Z());
    glVertex3f(_patches[i]->points[8]->X(), _patches[i]->points[8]->Y(),
               _patches[i]->points[8]->Z());
    glVertex3f(_patches[i]->points[4]->X(), _patches[i]->points[4]->Y(),
               _patches[i]->points[4]->Z());
    glEnd();
  }
}

void Model::movePoint(BezierPoint *bp, WorldPoint wp) {
  _hasChanged = true;

  if (bp == _connectionPoint || bp == _endPoint) {
    *bp += wp;
    return;
  }

  if (_c == NONE) {
    *bp += wp;
    return;
  }

  // handle the cases to enforce c1 continuity accross to neighbouring patches.
  set<BezierPatch *, BezierPatch::lt> patches = bp->patches;
  WorldPoint np(0, 0, 0);
  for (set<BezierPatch *, BezierPatch::lt>::iterator i = patches.begin();
       i != patches.end(); i++) {
    if ((*i)->aboveleft) {
      if (bp == (*i)->points[1])
        _fixColinear(bp, (*i)->points[0], (*i)->aboveleft->points[14], wp);
      else
        _fixColinear((*i)->points[1], (*i)->points[0],
                     (*i)->aboveleft->points[14], np);
      if (bp == (*i)->points[4])
        _fixColinear(bp, (*i)->points[0], (*i)->aboveleft->points[11], wp);
      else
        _fixColinear((*i)->points[4], (*i)->points[0],
                     (*i)->aboveleft->points[11], np);
    }
    if ((*i)->aboveright) {
      if (bp == (*i)->points[2])
        _fixColinear(bp, (*i)->points[3], (*i)->aboveright->points[13], wp);
      else
        _fixColinear((*i)->points[2], (*i)->points[3],
                     (*i)->aboveright->points[13], np);
      if (bp == (*i)->points[7])
        _fixColinear(bp, (*i)->points[3], (*i)->aboveright->points[8], wp);
      else
        _fixColinear((*i)->points[7], (*i)->points[3],
                     (*i)->aboveright->points[8], np);
    }
    if ((*i)->belowleft) {
      if (bp == (*i)->points[13])
        _fixColinear(bp, (*i)->points[12], (*i)->belowleft->points[2], wp);
      else
        _fixColinear((*i)->points[13], (*i)->points[12],
                     (*i)->belowleft->points[2], np);
      if (bp == (*i)->points[8])
        _fixColinear(bp, (*i)->points[12], (*i)->belowleft->points[7], wp);
      else
        _fixColinear((*i)->points[8], (*i)->points[12],
                     (*i)->belowleft->points[7], np);
    }
    if ((*i)->belowright) {
      if (bp == (*i)->points[14])
        _fixColinear(bp, (*i)->points[15], (*i)->belowright->points[1], wp);
      else
        _fixColinear((*i)->points[14], (*i)->points[15],
                     (*i)->belowright->points[1], np);
      if (bp == (*i)->points[11])
        _fixColinear(bp, (*i)->points[15], (*i)->belowright->points[4], wp);
      else
        _fixColinear((*i)->points[11], (*i)->points[15],
                     (*i)->belowright->points[4], wp);
    }

    if ((*i)->above) {
      if (bp == (*i)->points[4])
        _fixColinear(bp, (*i)->points[0], (*i)->above->points[8], wp);
      else
        _fixColinear((*i)->points[4], (*i)->points[0], (*i)->above->points[8],
                     np);
      if (bp == (*i)->points[5])
        _fixColinear(bp, (*i)->points[1], (*i)->above->points[9], wp);
      else
        _fixColinear((*i)->points[5], (*i)->points[1], (*i)->above->points[9],
                     np);
      if (bp == (*i)->points[6])
        _fixColinear(bp, (*i)->points[2], (*i)->above->points[10], wp);
      else
        _fixColinear((*i)->points[6], (*i)->points[2], (*i)->above->points[10],
                     np);
      if (bp == (*i)->points[7])
        _fixColinear(bp, (*i)->points[3], (*i)->above->points[11], wp);
      else
        _fixColinear((*i)->points[7], (*i)->points[3], (*i)->above->points[11],
                     np);
    }

    if ((*i)->below) {
      if (bp == (*i)->points[8])
        _fixColinear(bp, (*i)->points[12], (*i)->below->points[4], wp);
      else
        _fixColinear((*i)->points[8], (*i)->points[12], (*i)->below->points[4],
                     np);
      if (bp == (*i)->points[9])
        _fixColinear(bp, (*i)->points[13], (*i)->below->points[5], wp);
      else
        _fixColinear((*i)->points[9], (*i)->points[13], (*i)->below->points[5],
                     np);
      if (bp == (*i)->points[10])
        _fixColinear(bp, (*i)->points[14], (*i)->below->points[6], wp);
      else
        _fixColinear((*i)->points[10], (*i)->points[14], (*i)->below->points[6],
                     np);
      if (bp == (*i)->points[11])
        _fixColinear(bp, (*i)->points[15], (*i)->below->points[7], wp);
      else
        _fixColinear((*i)->points[11], (*i)->points[15], (*i)->below->points[7],
                     np);
    }

    if ((*i)->left) {
      if (bp == (*i)->points[1])
        _fixColinear(bp, (*i)->points[0], (*i)->left->points[2], wp);
      else
        _fixColinear((*i)->points[1], (*i)->points[0], (*i)->left->points[2],
                     np);
      if (bp == (*i)->points[5])
        _fixColinear(bp, (*i)->points[4], (*i)->left->points[6], wp);
      else
        _fixColinear((*i)->points[5], (*i)->points[4], (*i)->left->points[6],
                     np);
      if (bp == (*i)->points[9])
        _fixColinear(bp, (*i)->points[8], (*i)->left->points[10], wp);
      else
        _fixColinear((*i)->points[9], (*i)->points[8], (*i)->left->points[10],
                     np);
      if (bp == (*i)->points[13])
        _fixColinear(bp, (*i)->points[12], (*i)->left->points[14], wp);
      else
        _fixColinear((*i)->points[13], (*i)->points[12], (*i)->left->points[14],
                     np);
    }

    if ((*i)->right) {
      if (bp == (*i)->points[2])
        _fixColinear(bp, (*i)->points[3], (*i)->right->points[1], wp);
      else
        _fixColinear((*i)->points[2], (*i)->points[3], (*i)->right->points[1],
                     np);
      if (bp == (*i)->points[6])
        _fixColinear(bp, (*i)->points[7], (*i)->right->points[5], wp);
      else
        _fixColinear((*i)->points[6], (*i)->points[7], (*i)->right->points[5],
                     np);
      if (bp == (*i)->points[10])
        _fixColinear(bp, (*i)->points[11], (*i)->right->points[9], wp);
      else
        _fixColinear((*i)->points[10], (*i)->points[11], (*i)->right->points[9],
                     np);
      if (bp == (*i)->points[14])
        _fixColinear(bp, (*i)->points[15], (*i)->right->points[13], wp);
      else
        _fixColinear((*i)->points[14], (*i)->points[15],
                     (*i)->right->points[13], np);
    }
  }

  *bp += wp;
}

void Model::movePatchesOfPoint(BezierPoint *bp, WorldPoint wp) {
  if (bp == _connectionPoint || bp == _endPoint)
    return;

  _hasChanged = true;

  set<BezierPoint *> points;

  for (set<BezierPatch *, BezierPatch::lt>::iterator i = bp->patches.begin();
       i != bp->patches.end(); i++)
    for (int j = 0; j < 16; j++)
      points.insert((*i)->points[j]);

  if (_c == G1 || _c == C1) {
    Continuity c = _c;
    _c = NONE;

    for (set<BezierPoint *>::iterator i = points.begin(); i != points.end();
         i++)
      movePoint(*i, wp);

    WorldPoint np(0, 0, 0);
    _c = c;
    movePoint(bp, np);
  } else
    for (set<BezierPoint *>::iterator i = points.begin(); i != points.end();
         i++)
      movePoint(*i, wp);
}

void Model::mergePatchesOfPoints(BezierPoint *first, BezierPoint *second) {
  if (first == second) {
    cerr << "Attempt to merge a point with itself." << endl;
    return;
  }
  if (first == _connectionPoint || second == _connectionPoint ||
      first == _endPoint || second == _endPoint) {
    cerr << "Attempt to merge _connectionPoint or _endPoint." << endl;
    return;
  }

  _hasChanged = true;

  set<BezierPatch *, BezierPatch::lt> patches = first->patches;
  set<BezierPatch *, BezierPatch::lt> spatches = second->patches;

  for (set<BezierPatch *, BezierPatch::lt>::iterator i = patches.begin();
       i != patches.end(); i++) {
    for (set<BezierPatch *, BezierPatch::lt>::iterator j = spatches.begin();
         j != spatches.end(); j++) {
      if (first == (*i)->points[0])
        _attachAL(*i, *j);
      else if (first == (*i)->points[1] || first == (*i)->points[2])
        _attachA(*i, *j);
      else if (first == (*i)->points[3])
        _attachAR(*i, *j);
      else if (first == (*i)->points[4] || first == (*i)->points[8])
        _attachL(*i, *j);
      else if (first == (*i)->points[7] || first == (*i)->points[11])
        _attachR(*i, *j);
      else if (first == (*i)->points[12])
        _attachBL(*i, *j);
      else if (first == (*i)->points[13] || first == (*i)->points[14])
        _attachB(*i, *j);
      else if (first == (*i)->points[15])
        _attachBR(*i, *j);
    }
  }
}

void Model::splitPatchesOfPoint(BezierPoint *pt) {
  if (pt == _connectionPoint || pt == _endPoint) {
    cerr << "Attempt to split _connectionPoint." << endl;
    return;
  }

  if (pt->patches.size() <= 1) {
    return;
  }

  _hasChanged = true;

  set<BezierPatch *, BezierPatch::lt> patches = pt->patches;
  for (set<BezierPatch *, BezierPatch::lt>::iterator i = patches.begin();
       i != patches.end(); i++) {

    if (pt == (*i)->points[0]) {
      _detachAL(*i, (*i)->aboveleft);
      _detachA(*i, (*i)->above);
      _detachL(*i, (*i)->left);
    } else if (pt == (*i)->points[1] || pt == (*i)->points[2]) {
      _detachA(*i, (*i)->above);
     } else if (pt == (*i)->points[3]) {
      _detachAR(*i, (*i)->aboveright);
      _detachA(*i, (*i)->above);
      _detachR(*i, (*i)->right);
    } else if (pt == (*i)->points[4] || pt == (*i)->points[8]) {
      _detachL(*i, (*i)->left);
    } else if (pt == (*i)->points[7] || pt == (*i)->points[11]) {
      _detachR(*i, (*i)->right);
    } else if (pt == (*i)->points[12]) {
      _detachBL(*i, (*i)->belowleft);
      _detachL(*i, (*i)->left);
      _detachB(*i, (*i)->below);
    } else if (pt == (*i)->points[13] || pt == (*i)->points[14]) {
      _detachB(*i, (*i)->below);
     } else if (pt == (*i)->points[15]) {
      _detachBR(*i, (*i)->belowright);
      _detachR(*i, (*i)->right);
      _detachB(*i, (*i)->below);
    }
  }

  if (pt->patches.size() > 1) {
    for (set<BezierPatch *, BezierPatch::lt>::iterator i = pt->patches.begin();
         i != pt->patches.end(); i++) {

      if (i != pt->patches.begin()) {
        BezierPoint *npt = new BezierPoint();
        *npt = *pt;

        for (int k = 0; k < 16; k++) {
          if (pt == (*i)->points[k]) {
            (*i)->points[k] = npt;
            break;
          }
        }

        _controlpoints.Add(npt);
        npt->patches = pt->patches;
        npt->patches.erase(*(pt->patches.begin()));
        pt->patches.erase(*i);
      }
    }
  }

  _restoreData();
}

void Model::deletePatchesOfPoint(BezierPoint *pt) {
  if (pt == _connectionPoint || pt == _endPoint) {
    cerr << "Attempt to delete _connectionPoint." << endl;
    return;
  }

  _hasChanged = true;

  set<BezierPatch *, BezierPatch::lt> patches = pt->patches;
  for (set<BezierPatch *, BezierPatch::lt>::iterator i = patches.begin();
       i != patches.end(); i++) {

    if ((*i)->aboveleft)
      (*i)->aboveleft->belowright = NULL;
    if ((*i)->aboveright)
      (*i)->aboveright->belowleft = NULL;
    if ((*i)->belowleft)
      (*i)->belowleft->aboveright = NULL;
    if ((*i)->belowright)
      (*i)->belowright->aboveleft = NULL;

    if ((*i)->above)
      (*i)->above->below = NULL;
    if ((*i)->left)
      (*i)->left->right = NULL;
    if ((*i)->right)
      (*i)->right->left = NULL;
    if ((*i)->below)
      (*i)->below->above = NULL;

    for (int j = 0; j < 16; j++) {
      if (!(*i)->points[j])
        continue;

      (*i)->points[j]->patches.erase(*i);

      if (!(*i)->points[j]->patches.size()) {
        _controlpoints.Erase((*i)->points[j]);

        // This might be possible to replace with some sort of smart pointer
        for (int k = j + 1; k < 16; k++)
          if ((*i)->points[k] == (*i)->points[j])
            (*i)->points[k] = NULL;

        delete (*i)->points[j];
        (*i)->points[j] = NULL;
      }
    }

    _patches.Erase(*i);
    delete *i;
  }

  _restoreData();
}

list<string> Model::getPatchNames() {
  list<string> names;
  for (int i = 0; i < _patches.Count(); i++)
    names.push_back(_patches[i]->name);
  return names;
}

void Model::updateName(int index, const std::string newName){
  _patches[index]->name = newName;
}

void Model::highlightPatch(int index, bool on) {
  if (index < _patches.Count())
    _patches[index]->highlight = on;
}

void Model::_attachAL(BezierPatch *first, BezierPatch *second) {
  // AL of first attaches to BR or second

  if (!first || !second)
    return;
  if ((first->aboveleft || second->belowright) &&
      (first->aboveleft != second->belowright))
    return;

  if (_interpolateMerge) {
    *(first->points[0]) += *(second->points[15]);
    *(first->points[0]) *= 0.5;
  } else
    *(first->points[0]) = *(second->points[15]);

  _mergePoints(first->points[0], second->points[15]);

}

void Model::_attachA(BezierPatch *first, BezierPatch *second) {
  // A of first attached to B of second

  if (!first || !second)
    return;
  if ((first->above || second->below) && (first->above != second->below))
    return;

  if (_interpolateMerge) {
    *(first->points[0]) += *(second->points[12]);
    *(first->points[1]) += *(second->points[13]);
    *(first->points[2]) += *(second->points[14]);
    *(first->points[3]) += *(second->points[15]);
    *(first->points[0]) *= 0.5;
    *(first->points[1]) *= 0.5;
    *(first->points[2]) *= 0.5;
    *(first->points[3]) *= 0.5;
  } else {
    *(first->points[0]) = *(second->points[12]);
    *(first->points[1]) = *(second->points[13]);
    *(first->points[2]) = *(second->points[14]);
    *(first->points[3]) = *(second->points[15]);
  }

  _mergePoints(first->points[0], second->points[12]);
  _mergePoints(first->points[1], second->points[13]);
  _mergePoints(first->points[2], second->points[14]);
  _mergePoints(first->points[3], second->points[15]);

  first->above = second;
  second->below = first;

}

void Model::_attachAR(BezierPatch *first, BezierPatch *second) {
  // AR of first attached to BL of second

  if (!first || !second)
    return;
  if ((first->aboveright || second->belowleft) &&
      (first->aboveright != second->belowleft))
    return;

  if (_interpolateMerge) {
    *(first->points[3]) += *(second->points[12]);
    *(first->points[3]) *= 0.5;
  } else {
    *(first->points[3]) = *(second->points[12]);
  }

  _mergePoints(first->points[3], second->points[12]);

 }

void Model::_attachL(BezierPatch *first, BezierPatch *second) {
  // L of first attached to R of second

  if (!first || !second)
    return;
  if ((first->left || second->right) && (first->left != second->right))
    return;

  if (_interpolateMerge) {
    *(first->points[0]) += *(second->points[3]);
    *(first->points[4]) += *(second->points[7]);
    *(first->points[8]) += *(second->points[11]);
    *(first->points[12]) += *(second->points[15]);
    *(first->points[0]) *= 0.5;
    *(first->points[4]) *= 0.5;
    *(first->points[8]) *= 0.5;
    *(first->points[12]) *= 0.5;
  } else {
    *(first->points[0]) = *(second->points[3]);
    *(first->points[4]) = *(second->points[7]);
    *(first->points[8]) = *(second->points[11]);
    *(first->points[12]) = *(second->points[15]);
  }

  _mergePoints(first->points[0], second->points[3]);
  _mergePoints(first->points[4], second->points[7]);
  _mergePoints(first->points[8], second->points[11]);
  _mergePoints(first->points[12], second->points[15]);

  first->left = second;
  second->right = first;
 }

void Model::_attachR(BezierPatch *first, BezierPatch *second) {
  if (!_interpolateMerge) {
    BezierPoint temp;
    temp = *(second->points[0]);
    *(second->points[0]) = *(first->points[3]);
    *(first->points[3]) = temp;
    temp = *(second->points[4]);
    *(second->points[4]) = *(first->points[7]);
    *(first->points[7]) = temp;
    temp = *(second->points[8]);
    *(second->points[8]) = *(first->points[11]);
    *(first->points[11]) = temp;
    temp = *(second->points[12]);
    *(second->points[12]) = *(first->points[15]);
    *(first->points[15]) = temp;
  }

  // R of first attached to L of second
  _attachL(second, first);
}

void Model::_attachBL(BezierPatch *first, BezierPatch *second) {
  if (!_interpolateMerge) {
    BezierPoint temp = *(second->points[3]);
    *(second->points[3]) = *(first->points[12]);
    *(first->points[12]) = temp;
  }

  // BL of first attached to AR of second
  _attachAR(second, first);
}

void Model::_attachB(BezierPatch *first, BezierPatch *second) {
  if (!_interpolateMerge) {
    BezierPoint temp;
    temp = *(second->points[0]);
    *(second->points[0]) = *(first->points[12]);
    *(first->points[12]) = temp;
    temp = *(second->points[1]);
    *(second->points[1]) = *(first->points[13]);
    *(first->points[13]) = temp;
    temp = *(second->points[2]);
    *(second->points[2]) = *(first->points[14]);
    *(first->points[14]) = temp;
    temp = *(second->points[3]);
    *(second->points[3]) = *(first->points[15]);
    *(first->points[15]) = temp;
  }

  // B of first attached to A of second
  _attachA(second, first);
}

void Model::_attachBR(BezierPatch *first, BezierPatch *second) {
  if (!_interpolateMerge) {
    BezierPoint temp = *(second->points[0]);
    *(second->points[0]) = *(first->points[15]);
    *(first->points[15]) = temp;
  }

  // BR of first attached to AL of second
  _attachAL(second, first);
}

void Model::_mergePoints(BezierPoint *first, BezierPoint *second) {
  if (first == second)
    return;

  set_union(first->patches.begin(), first->patches.end(),
            second->patches.begin(), second->patches.end(),
            inserter(first->patches, first->patches.begin()),
            BezierPatch::lt());

  for (set<BezierPatch *, BezierPatch::lt>::iterator i = first->patches.begin();
       i != first->patches.end(); i++) {
    for (int j = 0; j < 16; j++) {
      if ((*i)->points[j] == second)
        (*i)->points[j] = first;
    }
  }

  _controlpoints.Erase(second);
  delete second;
}

void Model::_detachAL(BezierPatch *first, BezierPatch *second) {
  if (!first || !second)
    return;

  if (first->points[0] == second->points[15]) {
    second->points[15] = new BezierPoint();
    *(second->points[15]) = *(first->points[0]);
    second->points[15]->patches = first->points[0]->patches;
    _controlpoints.Add(second->points[15]);
  }
  first->points[0]->patches.erase(second);
  second->points[15]->patches.erase(first);

  first->aboveleft = NULL;
  second->belowright = NULL;

  splitPatchesOfPoint(first->points[0]);
  splitPatchesOfPoint(second->points[15]);
}

void Model::_detachA(BezierPatch *first, BezierPatch *second) {
  if (!first || !second)
    return;

  if (!second->belowleft && !second->left) {
    if (first->points[0] == second->points[12]) {
      second->points[12] = new BezierPoint();
      *(second->points[12]) = *(first->points[0]);
      second->points[12]->patches = first->points[0]->patches;
      _controlpoints.Add(second->points[12]);
    }
    first->points[0]->patches.erase(second);
    second->points[12]->patches.erase(first);
  }

  if (first->points[1] == second->points[13]) {
    second->points[13] = new BezierPoint();
    *(second->points[13]) = *(first->points[1]);
    second->points[13]->patches = first->points[1]->patches;
    _controlpoints.Add(second->points[13]);
  }
  first->points[1]->patches.erase(second);
  second->points[13]->patches.erase(first);

  if (first->points[2] == second->points[14]) {
    second->points[14] = new BezierPoint();
    *(second->points[14]) = *(first->points[2]);
    second->points[14]->patches = first->points[2]->patches;
    _controlpoints.Add(second->points[14]);
  }
  first->points[2]->patches.erase(second);
  second->points[14]->patches.erase(first);

  if (!second->belowright && !second->right) {
    if (first->points[3] == second->points[15]) {
      second->points[15] = new BezierPoint();
      *(second->points[15]) = *(first->points[3]);
      second->points[15]->patches = first->points[3]->patches;
      _controlpoints.Add(second->points[15]);
    }
    first->points[3]->patches.erase(second);
    second->points[15]->patches.erase(first);
  }

  first->above = NULL;
  second->below = NULL;
}

void Model::_detachAR(BezierPatch *first, BezierPatch *second) {
  if (!first || !second)
    return;

  if (first->points[3] == second->points[12]) {
    second->points[12] = new BezierPoint();
    *(second->points[12]) = *(first->points[3]);
    second->points[12]->patches = first->points[3]->patches;
    _controlpoints.Add(second->points[12]);
  }
  first->points[3]->patches.erase(second);
  second->points[12]->patches.erase(first);

  first->aboveright = NULL;
  second->belowleft = NULL;

  splitPatchesOfPoint(first->points[3]);
  splitPatchesOfPoint(second->points[12]);
}

void Model::_detachL(BezierPatch *first, BezierPatch *second) {
  if (!first || !second)
    return;

  if (!second->aboveleft && !second->above) {
    if (first->points[0] == second->points[3]) {
      second->points[3] = new BezierPoint();
      *(second->points[3]) = *(first->points[0]);
      second->points[3]->patches = first->points[0]->patches;
      _controlpoints.Add(second->points[3]);
    }
    first->points[0]->patches.erase(second);
    second->points[3]->patches.erase(first);
  }

  if (first->points[4] == second->points[7]) {
    second->points[7] = new BezierPoint();
    *(second->points[7]) = *(first->points[4]);
    second->points[7]->patches = first->points[4]->patches;
    _controlpoints.Add(second->points[7]);
  }
  first->points[4]->patches.erase(second);
  second->points[7]->patches.erase(first);

  if (first->points[8] == second->points[11]) {
    second->points[11] = new BezierPoint();
    *(second->points[11]) = *(first->points[8]);
    second->points[11]->patches = first->points[8]->patches;
    _controlpoints.Add(second->points[11]);
  }
  first->points[8]->patches.erase(second);
  second->points[11]->patches.erase(first);

  if (!second->belowleft && !second->below) {
    if (first->points[12] == second->points[15]) {
      second->points[15] = new BezierPoint();
      *(second->points[15]) = *(first->points[12]);
      second->points[15]->patches = first->points[12]->patches;
      _controlpoints.Add(second->points[15]);
    }
    first->points[12]->patches.erase(second);
    second->points[15]->patches.erase(first);
  }

  first->left = NULL;
  second->right = NULL;
 }

void Model::_detachR(BezierPatch *first, BezierPatch *second) {
  _detachL(second, first);
}

void Model::_detachBL(BezierPatch *first, BezierPatch *second) {
  _detachAR(second, first);
}

void Model::_detachB(BezierPatch *first, BezierPatch *second) {
  _detachA(second, first);
}

void Model::_detachBR(BezierPatch *first, BezierPatch *second) {
  _detachAL(second, first);
}

void Model::_fixColinear(BezierPoint *first, BezierPoint *middle,
                         BezierPoint *second, WorldPoint change) {
  WorldPoint d1(first->X(), first->Y(), first->Z());
  d1 += change;

  d1 -= *middle;
  d1 *= -1;
  if (_c == G1) {
    d1 /= d1.magnitude();
    d1 *= (*second - *middle).magnitude();
  }
  d1 += *middle;

  second->Set(d1.X(), d1.Y(), d1.Z());
}

void Model::_prepareColours() {
  _gridColour[0] = Config::getGridColour()[0];
  _gridColour[1] = Config::getGridColour()[1];
  _gridColour[2] = Config::getGridColour()[2];
  _gridColour[3] = Config::getGridColour()[3];

  _xAxisColour[0] = Config::getXAxisColour()[0];
  _xAxisColour[1] = Config::getXAxisColour()[1];
  _xAxisColour[2] = Config::getXAxisColour()[2];
  _xAxisColour[3] = Config::getXAxisColour()[3];

  _yAxisColour[0] = Config::getYAxisColour()[0];
  _yAxisColour[1] = Config::getYAxisColour()[1];
  _yAxisColour[2] = Config::getYAxisColour()[2];
  _yAxisColour[3] = Config::getYAxisColour()[3];

  _zAxisColour[0] = Config::getZAxisColour()[0];
  _zAxisColour[1] = Config::getZAxisColour()[1];
  _zAxisColour[2] = Config::getZAxisColour()[2];
  _zAxisColour[3] = Config::getZAxisColour()[3];

  _upVectorColour[0] = Config::getUpVectorColour()[0];
  _upVectorColour[1] = Config::getUpVectorColour()[1];
  _upVectorColour[2] = Config::getUpVectorColour()[2];
  _upVectorColour[3] = Config::getUpVectorColour()[3];

  _headingVectorColour[0] = Config::getHeadingColour()[0];
  _headingVectorColour[1] = Config::getHeadingColour()[1];
  _headingVectorColour[2] = Config::getHeadingColour()[2];
  _headingVectorColour[3] = Config::getHeadingColour()[3];

  _connectPointColour[0] = Config::getConnectionPointColour()[0];
  _connectPointColour[1] = Config::getConnectionPointColour()[1];
  _connectPointColour[2] = Config::getConnectionPointColour()[2];
  _connectPointColour[3] = Config::getConnectionPointColour()[3];

  _endPointColour[0] = Config::getEndPointColour()[0];
  _endPointColour[1] = Config::getEndPointColour()[0];
  _endPointColour[2] = Config::getEndPointColour()[0];
  _endPointColour[3] = Config::getEndPointColour()[0];

  _controlPointsColour[0] = Config::getControlPointsColour()[0];
  _controlPointsColour[1] = Config::getControlPointsColour()[1];
  _controlPointsColour[2] = Config::getControlPointsColour()[2];
  _controlPointsColour[3] = Config::getControlPointsColour()[3];

  _wireModelColour[0] = Config::getWireColour()[0];
  _wireModelColour[1] = Config::getWireColour()[1];
  _wireModelColour[2] = Config::getWireColour()[2];
  _wireModelColour[3] = Config::getWireColour()[3];

  _solidModelColour[0] = Config::getSolidColour()[0];
  _solidModelColour[1] = Config::getSolidColour()[1];
  _solidModelColour[2] = Config::getSolidColour()[2];
  _solidModelColour[3] = Config::getSolidColour()[3];

  _stickyColour[0] = Config::getStickyColour()[0];
  _stickyColour[1] = Config::getStickyColour()[1];
  _stickyColour[2] = Config::getStickyColour()[2];
  _stickyColour[3] = Config::getStickyColour()[3];

  _splitColour[0] = Config::getSplitColour()[0];
  _splitColour[1] = Config::getSplitColour()[1];
  _splitColour[2] = Config::getSplitColour()[2];
  _splitColour[3] = Config::getSplitColour()[3];

  _selectedColour[0] = Config::getSelectedColour()[0];
  _selectedColour[1] = Config::getSelectedColour()[1];
  _selectedColour[2] = Config::getSelectedColour()[2];
  _selectedColour[3] = Config::getSelectedColour()[3];

  _selectedForMergeColour[0] = Config::getSelectedForMergeColour()[0];
  _selectedForMergeColour[1] = Config::getSelectedForMergeColour()[1];
  _selectedForMergeColour[2] = Config::getSelectedForMergeColour()[2];
  _selectedForMergeColour[3] = Config::getSelectedForMergeColour()[3];

  _controlPolyColour[0] = Config::getControlPolyColour()[0];
  _controlPolyColour[1] = Config::getControlPolyColour()[1];
  _controlPolyColour[2] = Config::getControlPolyColour()[2];
  _controlPolyColour[3] = Config::getControlPolyColour()[3];

  _controlPolyInColour[0] = Config::getControlPolyInColour()[0];
  _controlPolyInColour[1] = Config::getControlPolyInColour()[1];
  _controlPolyInColour[2] = Config::getControlPolyInColour()[2];
  _controlPolyInColour[3] = Config::getControlPolyInColour()[3];

  _controlPolyHLColour[0] = Config::getControlPolyHLColour()[0];
  _controlPolyHLColour[1] = Config::getControlPolyHLColour()[1];
  _controlPolyHLColour[2] = Config::getControlPolyHLColour()[2];
  _controlPolyHLColour[3] = Config::getControlPolyHLColour()[3];

  _wireHLColour[0] = Config::getWireHLColour()[0];
  _wireHLColour[1] = Config::getWireHLColour()[1];
  _wireHLColour[2] = Config::getWireHLColour()[2];
  _wireHLColour[3] = Config::getWireHLColour()[3];

  _solidHLColour[0] = Config::getSolidHLColour()[0];
  _solidHLColour[1] = Config::getSolidHLColour()[1];
  _solidHLColour[2] = Config::getSolidHLColour()[2];
  _solidHLColour[3] = Config::getSolidHLColour()[3];
}

void Model::_prepareEvaluator(BezierPatch &p) {
  GLfloat coordList[16][3];

  for (int i = 0; i < 16; i++) {
    coordList[i][0] = p.points[i]->X();
    coordList[i][1] = p.points[i]->Y();
    coordList[i][2] = p.points[i]->Z();
  }

  glMap2f(GL_MAP2_VERTEX_3, 0, 1, 3, 4, 0, 1, 12, 4, &coordList[0][0]);

  glMapGrid2f(Config::getUDivs(), 0.0, 1.0, Config::getVDivs(), 0.0, 1.0);
}

void Model::_restoreData() {
  vector<BezierPatch *> tmpPatches;
  vector<BezierPoint *> tmpPoints;

  list<pair<int, int>> attAL;
  list<pair<int, int>> attA;
  list<pair<int, int>> attAR;
  list<pair<int, int>> attL;

  for (int i = 0; i < _patches.Count(); i++)
    _patches[i]->index = i;

  for (int i = 0; i < _patches.Count(); i++) {
    if (_patches[i]->aboveleft)
      attAL.push_back(make_pair(i, _patches[i]->aboveleft->index));
    if (_patches[i]->above)
      attA.push_back(make_pair(i, _patches[i]->above->index));
    if (_patches[i]->aboveright)
      attAR.push_back(make_pair(i, _patches[i]->aboveright->index));
    if (_patches[i]->left)
      attL.push_back(make_pair(i, _patches[i]->left->index));
  }

  for (int i = 0; i < _patches.Count(); i++) {
    BezierPatch *bp = new BezierPatch();
    bp->highlight = _patches[i]->highlight;
    bp->name = _patches[i]->name;
    tmpPatches.push_back(bp);

    for (int j = 0; j < 16; j++) {
      BezierPoint *pt = new BezierPoint();
      pt->patches.insert(bp);
      *pt = *(_patches[i]->points[j]);
      tmpPoints.push_back(pt);
      bp->points[j] = pt;
    }
  }

  for (int i = 0; i < _controlpoints.Count(); i++)
    delete _controlpoints[i];
  _controlpoints.Reset();

  for (int i = 0; i < _patches.Count(); i++)
    delete _patches[i];
  _patches.Reset();

  for (unsigned int i = 0; i < tmpPatches.size(); i++)
    _patches.Add(tmpPatches[i]);
  for (unsigned int i = 0; i < tmpPoints.size(); i++)
    _controlpoints.Add(tmpPoints[i]);

  for (list<pair<int, int>>::iterator i = attAL.begin(); i != attAL.end(); i++)
    _attachAL(_patches[(*i).first], _patches[(*i).second]);
  for (list<pair<int, int>>::iterator i = attA.begin(); i != attA.end(); i++)
    _attachA(_patches[(*i).first], _patches[(*i).second]);
  for (list<pair<int, int>>::iterator i = attAR.begin(); i != attAR.end(); i++)
    _attachAR(_patches[(*i).first], _patches[(*i).second]);
  for (list<pair<int, int>>::iterator i = attL.begin(); i != attL.end(); i++)
    _attachL(_patches[(*i).first], _patches[(*i).second]);
}

void Model::_destroyData() {
  _upVector.Set(0.0, 1.0, 0.0);
  _headingVector.Set(0.0, 0.0, 1.0);
  _size = 1.0;
  _connectionPoint->Set(0.0, 0.0, 0.0);
  _endPoint->Set(0.0, 0.0, 0.0);
  _c = NONE;
  _max.Set(1.0, 1.0, 1.0);
  _min.Set(-1.0, -1.0, -1.0);

  for (int i = 0; i < _controlpoints.Count(); i++)
    delete _controlpoints[i];
  _controlpoints.Reset();

  for (int i = 0; i < _patches.Count(); i++)
    delete _patches[i];
  _patches.Reset();

  _hasChanged = false;
}

Model::BezierPoint::BezierPoint()
    : WorldPoint(), selected(false), selectedForMerge(false), sticky(false) {}

Model::BezierPoint::BezierPoint(double x, double y, double z)
    : WorldPoint(x, y, z), selected(false), selectedForMerge(false),
      sticky(false) {}

Model::BezierPatch::BezierPatch()
    : above(NULL), left(NULL), right(NULL), below(NULL), aboveleft(NULL),
      aboveright(NULL), belowleft(NULL), belowright(NULL), highlight(false),
      top_colour(), top_diffuse(), bottom_colour(), bottom_diffuse() {
  for (int i = 0; i < 16; i++)
    points[i] = NULL;
}

Model::BezierPoint *Model::getPoint(unsigned int id) {
  if (id == 0)
    return _connectionPoint;
  else if (id == 1)
    return _endPoint;
  return _controlpoints[id - 2];
}
