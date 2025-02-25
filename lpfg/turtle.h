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



#ifndef __TURTLE_H__
#define __TURTLE_H__

#include "include/lparams.h"
#include "include/lintrfc.h"
#ifdef WIN32
#include "view.h"
#else
#include "viewLnx.h"
#endif

#include "tropismdata.h"
#include "polygon.h"
#include "gencyldata.h"
#include "surface.h"
#include "viewpos.h"
#include "glenv.h"

#include "terrain.h"
#include "povray.h"
#include "materialset.h"
#include "texturearr.h"
#include "psout.h"

class LstringIterator;
class Lstring;
class TropismArray;
class Mesh;
class Environment;
class Projection;
class ObjOutputStore;

class Turtle {
public:
  Turtle();
  virtual ~Turtle() {}
  virtual void F(float);
  virtual void f(float);
  virtual void G(float);
  virtual void g(float);
  virtual void Left(float);
  virtual void Right(float);
  virtual void Down(float);
  virtual void Up(float);
  virtual void RollR(float);
  virtual void RollL(float);
  virtual void IncColor();
  virtual void DecColor();
  virtual void SetColor(int);
  virtual void SetWidth(float);
  virtual void Label(const char *) const;
  virtual void Query(const LstringIterator &) const;
  virtual void Circle(float) const;
  virtual void CircleB(float) const;
  virtual void MoveTo3f(V3f v) { MoveTo(v.x, v.y, v.z); }
  virtual void MoveTo(float, float, float);
  virtual void MoveRel3f(V3f v) { MoveRel(v.x, v.y, v.z); }
  virtual void MoveRel(float, float, float);
  virtual void Sphere(float) const;
  virtual void SetHead(const float *);
  virtual void SetHeadOnly(V3f);
  virtual void Sphere0() const;
  virtual void Circle0() const;
  virtual void Line(V3f, V3f);
  virtual void LineTo(V3f);
  virtual void LineRel(V3f);
  virtual void Surface(int, float, float, float) const;
  virtual void RollToVert();
  virtual void SetElasticity(int, float);
  virtual void IncElasticity(int);
  virtual void DecElasticity(int);
  virtual void CurrentContour(int);
  virtual void StartGC();
  virtual void PointGC();
  virtual void EndGC();
  //virtual void MeshPoint();  // from mesh_unused.h/.cpp
  virtual void E1(float, const LstringIterator &);
  virtual void E2(float, float, const LstringIterator &);
  virtual void EA(int, const float *, const LstringIterator &);
  virtual void StartPolygon();
  virtual void EndPolygon();
  virtual void PolygonPoint();
  virtual void Rhombus(float, float) const;
  virtual void Triangle(float, float) const;
  virtual void BlendContours(int, int, float);
  virtual void CurrentTexture(int);
  virtual void TextureVCoeff(float);
  virtual void Orient() const;
  virtual void ScaleContour(float, float);
  virtual void Elasticity(float);
  virtual void STropismDirection(V3f);
  virtual void DSurface(SurfaceObj, bool) const;
  virtual void ContourSides(int);
  virtual void ContourNormal(V3f);
  virtual void InitSurface(int) const;
  virtual void SurfacePoint(int, int, int);
  virtual void DrawSurface(int) const;
  virtual void CircleFront(float) const;
  virtual void CircleFrontB(float) const;
  virtual void CircleFront0() const;
  virtual void SetUPrecision(int);
  virtual void SetVPrecision(int);
  virtual void BSurface(int, float, float, float) const;
  virtual void DBSurfaceS(BsurfaceObjS) const;
  virtual void DBSurfaceM(BsurfaceObjM) const;
  virtual void Camera(void);
  virtual void RotateXYZ(V3f, float);
  virtual void RotateHLU(V3f, float);
  virtual void Terrain(CameraPosition) const;
  virtual void PovRayStart(const char *, POVRayMeshMode);
  virtual void SetCoordinateSystem(float);
  virtual void Mesh(int, float, float, float) const;
  //virtual void AnimatedMesh(int, float, float) const;

  virtual void StartBranch();

  bool _TextureOn() const { return _CurrentTextureSet; }
  int getCurrentTexture() const { return _CurrentTexture; }

  Vector3d GetPosition() const { return _position; }
  Vector3d GetHeading() const { return _heading; }
  Vector3d GetUp() const { return _up; }
  Vector3d GetLeft() const { return _left; }
  void UpdateTropisms();
  void Dump() const;

#ifdef LINUX
  GLWidget *_glview;
#endif // LINUX

protected:
  void _AdjustTropisms(const TropismArray &);
  void _AdjustByOrthotropism(Vector3d);
  void _AdjustByTorque(const TropismArray &);

  virtual bool GCStarted() const { return false; }
  virtual void _UpdateContourDivisions(size_t) {}
  virtual void SuspendPolygon() {}
  virtual void ResumePolygon() {}
  virtual bool PolygonStarted() const { return false; }
  void _Bend(); // old (Jim's tropism)

  float Width() const { return _width; }
  float WidthUp() const { return _widthUp; }

  Vector3d _position;
  Vector3d _heading;
  Vector3d _up;
  Vector3d _left;
  // These two are required by tropisms.
  Vector3d _PrevHeading;
  Vector3d _PrevUp;
  int _CurrentTexture; //[PASCAL] - Sept 2017
  bool _CurrentTextureSet;
  float _textureV;
  float _textureVCoeff;

  int _color;

  const UVPrecision &GetUVPrecision() const { return _uvPrecision; }

  // MC - Feb. 2018 - add Cartesian coordinate system that can be aligned to
  // current turtle
  Vector3d _origin;
  Vector3d _xAxis;
  Vector3d _yAxis;
  Vector3d _zAxis;
  float _scaleCartesian;

private:
  void TRotate(Vector3d &, const Vector3d &, const Vector3d &, float,
               float) const;
  void ChDir(Vector3d &, Vector3d &, Vector3d &, float, float) const;

  float _width;
  float _widthUp; // MC - Sept. 2016 - added _widthUp, as contours can have
                  // different widths along Left and Up directions
  UVPrecision _uvPrecision;

protected:
  TropismData _TropismData;
  SimpleTropism _STropism;

  static const float epsilon;
};

class Volume;

class ViewTurtle : public Turtle {
public:
  ViewTurtle(Volume &vv, ViewPos &vp)
      : _volume(vv), _camera(vp), _polygonStarted(false), _genCylStarted(false),
        _leftGenCylPive(1.f), _leftGenCylNive(1.f), _upGenCylPive(1.f), _upGenCylNive(1.f),
        _prevGenCylWid(1.f), _prevGenCylWidUp(1.f) {}

  void F(float);
  void f(float);
  void operator=(const ViewTurtle &src) { Turtle::operator=(src); }
  void Circle(float) const;
  void CircleFront(float) const;
  void CircleB(float) const;
  void CircleFrontB(float) const;
  void Sphere(float) const;
  void Surface(int, float, float, float) const;
  void DSurface(SurfaceObj, bool) const;
  void BSurface(int, float, float, float) const;
  void DBSurfaceS(BsurfaceObjS) const;
  void DBSurfaceM(BsurfaceObjM) const;
  void Rhombus(float, float) const;
  void Triangle(float, float) const;
  void StartPolygon();
  void EndPolygon();
  void PolygonPoint();
  void Camera();
  void CurrentContour(int id); // MC - Sept. 2016 - added user-defined contours
                               // to view volume computation
  void BlendContours(int id1, int id2, float x); // and blended contours
  void StartGC();
  void EndGC();
  void Terrain(CameraPosition) const;
  void Mesh(int, float, float, float) const;
  //void AnimatedMesh(int, float, float) const;

protected:
  bool PolygonStarted() const { return _polygonStarted; }
  void SuspendPolygon() { _polygonStarted = false; }
  void ResumePolygon() { _polygonStarted = true; }

private:
  void AdaptSurf(::Surface &s) const;
  Volume &_volume;
  ViewPos &_camera;
  bool _polygonStarted, _genCylStarted;
  float _leftGenCylPive, _leftGenCylNive, _upGenCylPive, _upGenCylNive;
  float _prevGenCylWid, _prevGenCylWidUp;
};

class ScreenTurtle : public Turtle {
public:
  ScreenTurtle(unsigned int glbase, Vector3d vn, GLDraw::Polygon *pPolygon,
               void *pQ)
      : _divisions(divUnspecified), _glbase(glbase), _ViewNormal(vn),
        _pPolygon(pPolygon), _pQ(pQ) { /*terrainData = NULL;*/
    _TropismData.resetToInitialTropism();
  }
  void Label(const char *) const;
  void Circle(float) const;
  void CircleB(float) const;
  void CircleFront(float) const;
  void CircleFrontB(float) const;
  void Sphere(float) const;
  void Surface(int, float, float, float) const;
  void DSurface(SurfaceObj, bool) const;
  void ContourSides(int);
  void BSurface(int, float, float, float) const;
  void DBSurfaceS(BsurfaceObjS) const;
  void DBSurfaceM(BsurfaceObjM) const;
  void StartPolygon();
  void EndPolygon();
  void PolygonPoint();
  void Rhombus(float, float) const;
  void Triangle(float, float) const;
  void Orient() const;
  void Terrain(CameraPosition) const;
  void Mesh(int, float, float, float) const;
  //void AnimatedMesh(int, float, float) const;

protected:
  bool PolygonStarted() const { return _pPolygon->Started(); }
  void SuspendPolygon() { _pPolygon->Suspend(); }
  void ResumePolygon() { _pPolygon->Resume(); }
  const Vector3d &ViewNormal() const { return _ViewNormal; }

  enum { divUnspecified = -1 };
  int _divisions;

private:
  void _Sphere(float) const;
  unsigned int _glbase;
  Vector3d _ViewNormal;
  GLDraw::Polygon *_pPolygon;
  void *_pQ;

};

class PixelLineScreenTurtle : public ScreenTurtle {
public:
  PixelLineScreenTurtle(unsigned int glbase, Vector3d vn,
                        GLDraw::Polygon *pPolygon, void *pQ)
      : ScreenTurtle(glbase, vn, pPolygon, pQ) {}
  void StartGC();
  void F(float);
};

class PolygonLineScreenTurtle : public ScreenTurtle {
public:
  PolygonLineScreenTurtle(unsigned int glbase, Vector3d vn,
                          GLDraw::Polygon *pPolygon, void *pQ)
      : ScreenTurtle(glbase, vn, pPolygon, pQ) {}
  void F(float);
  void StartGC();
};

class CylinderLineScreenTurtle : public ScreenTurtle {
public:
  CylinderLineScreenTurtle(unsigned int glbase, Vector3d vn,
                           GLDraw::Polygon *pPolygon, void *pQ);
  void F(float);
  void SetWidth(float);
  void CurrentContour(int);
  void BlendContours(int, int, float);
  void StartGC();
  void PointGC();
  void EndGC();
  void ScaleContour(float, float);
  void ContourSides(int);
  void ContourNormal(V3f);
  void StartBranch();

private:
  void _GCF(float);
  void _NormalF(float);
  void _CapGC();
  
  float _ScaleP() const { return _Scale.p; }
  float _ScaleQ() const { return _Scale.q; }

  void _UpdateContourDivisions(size_t);
  bool GCStarted() const { return _gc.On(); }

  GeneralizedCylinderData::Scale _Scale;
  int _CurrentContour;
  int _ContourId2;
  float _blender;
  bool _allowBranchGC;
  GeneralizedCylinderData _gc;

};

class EnvironmentTurtle : public Turtle {
public:
  EnvironmentTurtle(Lstring &lstring,
                    Environment *pEnv);
  void Query(const LstringIterator &) const;
  void operator=(const EnvironmentTurtle &src) { Turtle::operator=(src); }
  //void MeshPoint(); // from mesh_unused.h/.cpp
  bool InterpretedE() const { return _InterpretedE; }
  void E1(float, const LstringIterator &);
  void E2(float, float, const LstringIterator &);
  void EA(int, const float *, const LstringIterator &);

private:
  // [PASCAL] the following doesn't seem to be used
  //  Lstring &_lstring;
  Environment *_pEnvironment;
  bool _InterpretedE;
};

class POVRayTurtle : public Turtle {
public:
  POVRayTurtle(std::ofstream &trg, std::ofstream *surface_trg_arr,
               std::ofstream &layout);
  void F(float);
  void Sphere(float) const;
  void Surface(int, float, float, float) const;
  void Rhombus(float, float) const;
  void Triangle(float, float) const;
  void SetWidth(float);
  void CurrentContour(int);
  void BlendContours(int, int, float);
  void StartGC();
  void PointGC();
  void EndGC();
  void StartBranch();
  void ScaleContour(float, float);
  void Label(const char *) const;
  void Terrain(CameraPosition) const;
  void PovRayStart(const char *, POVRayMeshMode);

  void operator=(const POVRayTurtle &src);

  /// pre declaration of textures and surfaces.
  void textureDeclaration(std::ofstream &) const;
  void surfaceDeclaration(std::ofstream &) const;
  void terrainDeclaration(std::ofstream &) const;

private:
  void _SetColor() const;
  void _GCF(float);
  void _NormalF(float);
  float _ScaleP() const { return _Scale.p; }
  float _ScaleQ() const { return _Scale.q; }

  void itos(char *s, int s_len, int val) const;
  void ftos(char *s, int s_len, float val);

  GeneralizedCylinderData::Scale _Scale;
  int _CurrentContour;
  int _ContourId2;
  float _blender;
  bool _allowBranchGC;
  GeneralizedCylinderData _gc;

  std::ofstream &_trg; // Output stream for the branches and final object
  std::ofstream *_surface_trg_arr; // Output streams for all the surface objects
                                   // in the scene
  bool *_surface_trg_is_used; // Array of bools same size as _surface_trg_arr to
                              // tell where a file for surface output has been
                              // used or not. If not then we can remove it
  std::ofstream
      &_layoutTrg; // Output to the layout file to be used in the Terrain Editor
  char *fileAppend; // To ensure unique names across multiple files included in
                    // povray this is a unique string appended to every declared
                    // entity
  std::string
      oldFn; // used to add an entry to the layout file for the tree just read
  Vector3d basePos;       // basePosition of the model (first point in the tree)
  bool basePosSet;        // Flag to say that the basePos is valid or not
  float modelScaleFactor; // Used to scale the models before export to account
                          // for the terrain scale used in some programs

  Volume currentBB; // Used to calculate the bounding box information on a per
                    // plant basis

  POVRay::Mesh *mesh;
  POVRayMeshMode meshMode;
};

class RayshadeTurtle : public Turtle {
public:
  RayshadeTurtle(std::ofstream &target, const Projection &currentProjection,
                 GLEnv &glEnv, std::string fname);

  void operator=(const RayshadeTurtle &src);

  void F(float);
  void Sphere(float) const;
  void Circle(float) const;
  void CircleB(float) const;
  void CircleFront(float) const;
  void CircleFrontB(float) const;
  void Triangle(float, float) const;
  void Rhombus(float, float) const;
  void InstanceSurfaces() const;
  void Surface(int, float, float, float) const;
  void DSurface(SurfaceObj, bool) const;
  void BSurface(int, float, float, float) const;
  void DBSurfaceS(BsurfaceObjS) const;
  void DBSurfaceM(BsurfaceObjM) const;
  void StartPolygon();
  void EndPolygon();
  void PolygonPoint();
  void IncColor();
  void DecColor();
  void SetColor(int);
  void StartGC();
  void PointGC();
  void EndGC();
  void CurrentContour(int);
  void BlendContours(int, int, float);
  void SetWidth(float);
  void ScaleContour(float, float);
  void ContourSides(int);
  void ContourNormal(V3f);
  void StartBranch();

  void StartNewGrid(int *size, std::string fname) const;
  void CompleteGrid() const;
  void Finish(const Projection &currentProjection, std::string fname) const;

  void Mesh(int, float, float, float) const;
  //void AnimatedMesh(int, float, float) const;

protected:
  bool PolygonStarted() const { return _PolygonStarted; }
  void SuspendPolygon() { _PolygonStarted = false; }
  void ResumePolygon() { _PolygonStarted = true; }

private:
  void _NormalF(float);
  void _GCF(float);
  void _CapGC();

  float _ScaleP() const { return _Scale.p; }
  float _ScaleQ() const { return _Scale.q; }

  std::ofstream
      &_target; // Output stream for the final object being written to a file.
  void PrintMaterial(GLEnv &glEnv, int x);
  void PrintSurface(::Surface s, float sx, float sy, float sz) const;
  void PrintHeader(const Projection &currentProjection, GLEnv &glEnv);
  void PrintBBOX(const Projection &currentProjection, std::string fname);
  void PrintSurfaceUse() const;

  GeneralizedCylinderData::Scale _Scale;
  int _CurrentContour;
  int _ContourId2;
  float _blender;
  bool _allowBranchGC;
  GeneralizedCylinderData _gc;

  std::vector<Vector3d> polygonPoints;
  bool _PolygonStarted;

  static const float epsilon;
};

class ObjTurtle : public Turtle {
public:
  ObjTurtle(ObjOutputStore &);
  void F(float);
  void Circle(float) const;
  void CircleB(float) const;
  void CircleFront(float) const;
  void CircleFrontB(float) const;
  void Sphere(float) const;
  void Surface(int, float, float, float) const;
  void DSurface(SurfaceObj, bool) const;
  void BSurface(int, float, float, float) const;
  void DBSurfaceS(BsurfaceObjS) const;
  void DBSurfaceM(BsurfaceObjM) const;
  void ContourSides(int);
  void ContourNormal(V3f);
  void StartPolygon();
  void EndPolygon();
  void PolygonPoint();
  void Rhombus(float, float) const;
  void Triangle(float, float) const;
  void SetWidth(float);
  void CurrentContour(int);
  void BlendContours(int, int, float);
  void CurrentTexture(int);
  void StartGC();
  void PointGC();
  void EndGC();
  void StartBranch();
  void IncColor();
  void DecColor();
  void SetColor(int);
  void ScaleContour(float, float);
  void Mesh(int, float, float, float) const;
  //void AnimatedMesh(int, float, float) const;

  void operator=(const ObjTurtle &src);

protected:
  bool PolygonStarted() const { return _PolygonStarted; }
  void SuspendPolygon() { _PolygonStarted = false; }
  void ResumePolygon() { _PolygonStarted = true; }

private:
  void _GCF(float);
  void _NormalF(float);
  float _ScaleP() const { return _Scale.p; }
  float _ScaleQ() const { return _Scale.q; }
  void _CapGC(bool endCap);

  GeneralizedCylinderData::Scale _Scale;
  int _CurrentContour;
  int _ContourId2;
  float _blender;
  bool _allowBranchGC;
  GeneralizedCylinderData _gc;

  ObjOutputStore &_trg;

  std::vector<Vector3d> polygonPoints;
  bool _PolygonStarted;

  enum { divUnspecified = -1 };
  int _divisions;
};

class PostscriptTurtle : public Turtle {
public:
  PostscriptTurtle(std::ostream &trg, PsOutputStore &st, Volume,
                   const Projection &currentProjection,
                   DParams::ProjectionMode mode);
  void operator=(const PostscriptTurtle &src);

  void F(float);
  void Circle(float) const;
  void CircleB(float) const;
  void CircleFront(float) const;
  void CircleFrontB(float) const;
  void Sphere(float) const;
  void StartPolygon();
  void EndPolygon();
  void PolygonPoint();
  void StartGC();
  void PointGC();
  void EndGC();
  void StartBranch();
  void CurrentContour(int);
  void BlendContours(int, int, float);
  void SetWidth(float);
  void ScaleContour(float, float);
  void ContourSides(int);
  void ContourNormal(V3f);
  void Rhombus(float, float) const;
  void Triangle(float, float) const;
  void Orient() const;
  void Label(const char *) const;
  void Surface(int, float, float, float) const;
  void DSurface(SurfaceObj, bool) const;
  void BSurface(int id, float sx, float sy, float sz) const;
  void PrintSurface(::Surface, float, float, float) const;
  void Finish();
protected:
  bool PolygonStarted() const { return _polygonStarted; }
  void SuspendPolygon() { _polygonSuspended = true; }
  void ResumePolygon() { _polygonSuspended = false; }

private:
  Vector3d _TransformPoint(Vector3d) const;
  float _TransformSize(float) const;

  void _NormalF(float);
  void _GCF(float);
  void _CapGC();

  PsOutputStore &_store;
  std::ostream &_trg;
  float _scale;
  Vector3d _translate;
  Vector3d _translateClipping;
  float _rotation[16];

  bool _polygonStarted, _polygonSuspended;
  std::vector<Vector3d> _polygonPoints;

  float _ScaleP() const { return _Scale.p; }
  float _ScaleQ() const { return _Scale.q; }

  GeneralizedCylinderData::Scale _Scale;
  int _CurrentContour;
  int _ContourId2;
  float _blender;
  bool _allowBranchGC;
  GeneralizedCylinderData _gc;

};

class NumericCheckTurtle : public Turtle {
public:
  void F(float);
  void f(float);
  void Left(float);
  void Right(float);
  void Down(float);
  void Up(float);
  void RollR(float);
  void RollL(float);
  void MoveTo(float, float, float);
  void MoveRel(float, float, float);
  void SetHead(const float *);
  void SetHeadOnly(V3f);
  void Line(V3f, V3f);
  void LineTo(V3f);
  void LineRel(V3f);
  void Sphere(float) const;
  void RotateXYZ(V3f, float);
  void RotateHLU(V3f, float);

private:
  void _Check(float, const char *) const;
};

class TextFileTurtle : public Turtle {
public:
  TextFileTurtle(FILE *fp);
  ~TextFileTurtle();
  void F(float);
  void f(float);
  void G(float);
  void g(float);
  void Left(float);
  void Right(float);
  void Down(float);
  void Up(float);
  void RollR(float);
  void RollL(float);
  void IncColor();
  void DecColor();
  void SetColor(int);
  void SetWidth(float);
  void Label(const char *) const;
  void Circle(float) const;
  void MoveTo(float, float, float);
  void MoveRel(float, float, float);
  void Sphere(float) const;
  void SetHead(const float *);
  void SetHeadOnly(V3f);
  void Sphere0() const;
  void Circle0() const;
  void Line(V3f, V3f);
  void LineTo(V3f);
  void LineRel(V3f);
  void Surface(int, float, float, float) const;
  void RollToVert();
  void SetElasticity(int, float);
  void IncElasticity(int);
  void DecElasticity(int);
  void CurrentContour(int);
  void StartGC();
  void PointGC();
  void EndGC();
  //void MeshPoint(); // from mesh_unused.h/.cpp
  void E1(float, const LstringIterator &);
  void E2(float, float, const LstringIterator &);
  void StartPolygon();
  void EndPolygon();
  void PolygonPoint();
  void Rhombus(float, float) const;
  void Triangle(float, float) const;
  void BlendContours(int, int, float);
  void CurrentTexture(int);
  void TextureVCoeff(float);
  void Orient() const;
  void ScaleContour(float, float);
  void Elasticity(float);
  void STropismDirection(V3f);
  void DSurface(SurfaceObj, bool) const;
  void ContourSides(int);
  void ContourNormal(V3f);
  void InitSurface(int) const;
  void SurfacePoint(int, int, int);
  void DrawSurface(int) const;
  void CircleFront(float) const;
  void CircleFront0() const;
  void SetUPrecision(int);
  void SetVPrecision(int);
  void BSurface(int, float, float, float) const;
  void DBSurfaceS(BsurfaceObjS) const;
  void DBSurfaceM(BsurfaceObjM) const;
  void Camera(void);
  void EA(int, const float *, const LstringIterator &);
  void RotateXYZ(V3f, float);
  void RotateHLU(V3f, float);
  void Terrain(CameraPosition) const;
  void PovRayStart(const char *, POVRayMeshMode);
  void Mesh(int, float, float, float) const;
  //void AnimatedMesh(int, float, float) const;

private:
  FILE *_fp;
};

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
