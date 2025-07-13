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



#ifndef __CONTOUR_H__
#define __CONTOUR_H__

#include <vector>

#include "include/lparams.h"
#include "include/lintrfc.h"

#include "asrt.h"
#include "vector3d.h"
#include "lpfgparams.h"

class ReadTextFile;
template <int p, int q> class Matrix;
class Matrix4x3;

class Contour {
public:
  Contour();
  Contour(ReadTextFile &);
  bool isDefaultLoaded() { return defaultLoaded; }

  /*
          Getters;
  */
  const char *Name() const;
  bool Closed() const;
  float GetArcLen() const;

  // vertex data.
  const float *Vertex(size_t i) const;
  const float *Normal(size_t i) const;
  Vector3d GetVertex(size_t i) const;
  Vector3d GetNormal(size_t i) const;
  float GetX(float) const;
  float GetY(float) const;
  float GetZ(float) const;
  V3f GetV3fPoint(float) const;
  V3f GetV3fNormal(float) const;
  V2f GetV2fPoint(float) const;
  std::vector<Vector3d> GetCtrlPts();
  bool ValidPointId(size_t id) const;

  // division data
  size_t Divisions() const;
  size_t OriginalDivisions() const;
  bool DivisionsSpecified() const;

  // MC - Sept. 2016 - return the extent of the bounding box
  Vector3d GetMaxPt() const { return _maxPt; }
  Vector3d GetMinPt() const { return _minPt; }

  /*
          Setters;
  */
  void SetPoint(size_t id, float x, float y, float z);
  void SetDivisions(size_t d);

  /*
          Modifiers;
  */
  void Recalculate();
  void RecalculateDefault();
  void Reset();
  void Scale(float, float, float);

  void Dump() const;

protected:
  bool defaultLoaded;

  /*
          Create Curve;
  */
  void _Calculate(const std::vector<Vector3d> &);
  void _CalculateRegular(const std::vector<Vector3d> &);
  void _CalculateEndPoint(const std::vector<Vector3d> &);
  size_t _CalcPoints(Vector3d, Vector3d, Vector3d, Vector3d, size_t, size_t);
  void _CalcPoints2(const Vector3d &v0, const Vector3d &v1, const Vector3d &v2,
                    const Vector3d &v3, std::vector<Vector3d> &res,
                    std::vector<Vector3d> &nrm, int steps);

  /*
          Reparametrize Curve
  */
  void _Reparametrize(); // Do not use this.  Use _Reparametrize2
  void _Reparametrize2(std::vector<Vector3d> &pts, std::vector<Vector3d> &nrm);
  int _FindIx(float[], float, float &) const;

  void _Default();

  /*
          For Curve with end points.
  */
  static Vector3d _P(float, const std::vector<Vector3d> &);
  static float _N(size_t, size_t, float, size_t);
  static float _Nk1(size_t, float, size_t);
  static float _Nkt(size_t, size_t, float, size_t);
  static size_t _Uk(size_t, size_t);

  /*
          Helper functions;
  */
  void FillZeroNormals();

  static Vector3d _BSpline(const Matrix<1, 4> &, const Matrix4x3 &);
  static Vector3d _BSplineNormal(const Matrix<1, 4> &, const Matrix4x3 &);

  /*
          Loading a contour
  */
  bool _Load0101(ReadTextFile &);
  bool _Load0102(ReadTextFile &);
  bool _Load0103(ReadTextFile &);

  /*
          Variables
  */
  /*
          DivisionData: Contains data pertaining to the amount of divisions that
     are used to create the b-spline curve from the control points.
  */
  class DivisionData {
  public:
    DivisionData();
    size_t Divisions() const;
    bool Specified() const;
    size_t OrigDivisions() const;
    void SetOrigDivisions(size_t d);
    void SetDivisions(size_t d);

  private:
    size_t _divisions;
    size_t _origDiv;
    bool _specified;
  };

  typedef std::vector<Vector3d>::iterator Iter;
  typedef std::vector<Vector3d>::const_iterator Citer;
  enum ContourType { btRegular, btEndPoint };

  std::vector<Vector3d> _cpts; // the contour's control points.
  std::vector<Vector3d>
      _origCpts; // the contour's original unmodified control points.
  std::vector<Vector3d> _vertex; // the points for the b-spline curve
  std::vector<Vector3d>
      _normal; // the normals for each point of the b-spline curve.
  float _arcLen; // the curve's arclength

  bool _Closed;
  ContourType _type;
  char _Name[LPFGParams::ObjectNameLength + 1];

  DivisionData _ddata;

  Vector3d _maxPt, _minPt; // MC - Sept. 2016 - save the bounding box of the
                           // contour for view volume computation
};

/*
        BlendedContour: Defines how to interpolate from one contour to another.

                Requires that both curves have equal number of points describing
   the b-spline curve (_vertex[])
*/
class BlendedContour : public Contour {
public:
  void Blend(const Contour &, const Contour &, float);
};

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
