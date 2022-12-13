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

#include <set>
#include <string>
#include <list>
#include <qgl.h>

#include "geometry.h"
#include "dynarray.h"

class Model {
 public:
  Model();
  ~Model();

  void load(std::string filename);
  void save(std::string filename);
  void createPatch();

  bool hasChanged() {return _hasChanged;}
  void useInterpolateMerge(bool on) {_interpolateMerge = on;}
  void showSplitablePoints(bool on) {_showSplitable = on;}

  int    getPointCount();
  double getSize() {return _size;}
  double getScale() {return _scale;}

  double findLargestWidth();

  // draw methods
  void drawGrid();
  void drawAxes();
  void drawUpVector();
  void drawHeadingVector();
  void drawConnectPoint();
  void drawEndPoint();
  void drawControlPoints();
  void drawWireModel();
  void drawSolidModel();
  void drawControlPoly();

  // exceptions
  class FileExc {};

  // model data
  class BezierPoint;
  class BezierPatch {
  public:
    BezierPatch();
    BezierPoint* points[16];

    BezierPatch* above;
    BezierPatch* left;
    BezierPatch* right;
    BezierPatch* below;
    BezierPatch* aboveleft;
    BezierPatch* aboveright;
    BezierPatch* belowleft;
    BezierPatch* belowright;

    // less than predciate for algorithms
    struct lt {
      bool operator() (BezierPatch* a, BezierPatch* b) const
      {return (a < b);}
    };

    bool highlight;

    std::string name;
    unsigned int top_colour;
    float top_diffuse;
    unsigned int bottom_colour;
    float bottom_diffuse;

    int index;
  };
  class BezierPoint : public WorldPoint {
  public:
    BezierPoint();
    BezierPoint(double, double, double);
    std::set<BezierPatch*, BezierPatch::lt> patches;

    bool selected;
    bool selectedForMerge;
    bool sticky;

    void operator=(BezierPoint& pb) {
      _x = pb._x;
      _y = pb._y;
      _z = pb._z;
    }
  };
  enum Continuity {
    NONE,
    G1,
    C1
  };

  void movePoint(BezierPoint*, WorldPoint);
  void movePatchesOfPoint(BezierPoint*, WorldPoint);
  void mergePatchesOfPoints(BezierPoint*, BezierPoint*);
  void splitPatchesOfPoint(BezierPoint*);
  void deletePatchesOfPoint(BezierPoint*);
  BezierPoint* getPoint(unsigned int id);


  void setContinuity(Continuity c) {_c = c;}

  std::list<std::string> getPatchNames();
  void updateName(int index, std::string newName);

  void highlightPatch(int index, bool on);
  void useSelectSize(bool on) {_useSelectSize = on;}
  
 protected:
  // colours
  void _prepareColours();
  GLfloat _gridColour[4];
  GLfloat _xAxisColour[4];
  GLfloat _yAxisColour[4];
  GLfloat _zAxisColour[4];
  GLfloat _upVectorColour[4];
  GLfloat _headingVectorColour[4];
  GLfloat _connectPointColour[4];
  GLfloat _endPointColour[4];
  GLfloat _controlPointsColour[4];
  GLfloat _wireModelColour[4];
  GLfloat _solidModelColour[4];
  GLfloat _stickyColour[4];
  GLfloat _splitColour[4];
  GLfloat _selectedColour[4];
  GLfloat _selectedForMergeColour[4];
  GLfloat _controlPolyColour[4];
  GLfloat _controlPolyInColour[4];
  GLfloat _controlPolyHLColour[4];
  GLfloat _wireHLColour[4];
  GLfloat _solidHLColour[4];

  // multipatch functions
  void _attachAL(BezierPatch*, BezierPatch*);
  void _attachA(BezierPatch*, BezierPatch*);
  void _attachAR(BezierPatch*, BezierPatch*);
  void _attachL(BezierPatch*, BezierPatch*);
  void _attachR(BezierPatch*, BezierPatch*);
  void _attachBL(BezierPatch*, BezierPatch*);
  void _attachB(BezierPatch*, BezierPatch*);
  void _attachBR(BezierPatch*, BezierPatch*);

  void _mergePoints(BezierPoint*, BezierPoint*);

  void _detachAL(BezierPatch*, BezierPatch*);
  void _detachA(BezierPatch*, BezierPatch*);
  void _detachAR(BezierPatch*, BezierPatch*);
  void _detachL(BezierPatch*, BezierPatch*);
  void _detachR(BezierPatch*, BezierPatch*);
  void _detachBL(BezierPatch*, BezierPatch*);
  void _detachB(BezierPatch*, BezierPatch*);
  void _detachBR(BezierPatch*, BezierPatch*);

  // model data
  void _fixColinear(BezierPoint*, BezierPoint*, BezierPoint*, WorldPoint);
  void _prepareEvaluator(BezierPatch&);

  WorldPoint   _upVector;
  WorldPoint   _headingVector;
  double       _size;
  double       _scale;
  BezierPoint* _connectionPoint;
  BezierPoint* _endPoint;
  Continuity   _c;
  WorldPoint   _max;
  WorldPoint   _min;

  bool _useSelectSize;
  bool _hasChanged;
  bool _interpolateMerge;
  bool _showSplitable;

  DynArray<BezierPoint*> _controlpoints;
  DynArray<BezierPatch*> _patches;

  // clean up routines
  void _destroyData();
  void _restoreData();
};

#endif
