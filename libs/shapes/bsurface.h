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




#ifndef __BSURFACE_H__
#define __BSURFACE_H__

#include <vector>
#include <string>
#include <stdio.h>
#include <sstream>

#include "point.h"
#include "matrix.h"
#include "../../lpfg/vector3d.h"

using std::vector;

namespace Shapes
{

typedef Point<double, 3> Point3d;
typedef Point<double, 2> Point2d;

class BSplineSurface
{
public:

  //Interface to LPFG
  void Load(const char* fname, double scale, int TextureId, int divU, int divV);
  void Reread();
  void BoundingBox(double &x1,double &y1,double &z1,double &x2,double &y2,double &z2);
  void Transform(Matrix M, double x, double y, double z);
  bool IsTextured() const
    { return _TextureId >= 0; }

  int TextureId() const
    { return _TextureId; }

  void Draw(int uDiv, int vDiv);
  void Rayshade(int uDiv, int vDiv, std::ofstream& target, bool textured, std::string filename);
  void PostScript(int uDiv, int vDiv, std::string& target, Vector3d clr);
  void GetQuad(int i, int j, Vector3d* vlist);

  void Precompute(int, int);
  bool Precomputed() { return precomputed; }

  void Recompute()
   {precomputed=false;}
	
  void GetControlMesh(int i, int j, double &x,double &y,double &z)
    {x=Control_points[0][i][j][0];y=Control_points[0][i][j][1];z=Control_points[0][i][j][2];}

  void SetControlMesh(int i, int j,double x, double y, double z)
    {Control_points[0][i][j][0]=x;Control_points[0][i][j][1]=y;Control_points[0][i][j][2]=z;}

  //Functions used in splineEdit and frameEdit

  //Constructors and Destructors
  BSplineSurface();
  ~BSplineSurface();
  BSplineSurface(const BSplineSurface &B);
	  
  //Save/Load the surface in fname
  bool  LoadPatch(const char *fname);
  bool  SavePatch(const char *fname);
  void Reset(int u,int v);	
  
  //Resizes the patch to have num_rows rows and num_cols columns using forward and reverse subdivision
  //It is assumed that the dimensions passed in can be created simply by using forward and reverse subdivision
  void Resize(unsigned int num_rows,unsigned int num_cols);

  //Returns first direvative of change in area at u,v
  float HasContracted(BSplineSurface *b, float u, float v);
  //Provides previous b-spline surface to perform the above comparison on when rendering
  void SetPrevious(BSplineSurface *b)
    {psurf = b;};
  void VisualizeContraction(bool c)
    {contraction=c;}
  
	  
  //Applys forward subdivision along the U-curves of the control points
  void DoubleU();
  //Applys reverse subdivision along the U-curves of the control points
  void HalveU();
  //Applys forward subdivision along the V-curves of the control points
  void DoubleV();
  //Applys reverse subdivision along the V-curves of the control points
  void HalveV();
	  
  //Toggles between wireframe and shaded renderings
  void IsShaded(bool shaded){wireframe=!shaded;};
  //Sets the degree of the surface, if degree!=3 then forward and reverse subdivision will not work
  //  void setdegree(int i){degree=i;};
  //Toggles between a shaded wireframe mesh and shaded polygons
  void ToggleShadedMesh(){s_mesh=false;};
  void TogglePolygonMesh(){s_mesh=true;};
  void ToggleNormals(){disp_normal=!disp_normal;};
  void ToggleControlMesh(){disp_c_mesh=!disp_c_mesh;};
  //Changes the colors used when rendering
  void SetColors(double c1[3],double c2[3],double c3[3]);
	  
  //Specifies which resolution of control points is being edited
  //returns false if level specified does not exist
  bool EditLevel(unsigned int i);
	  
  //Specifies the length of u and v steps to be used when rendering
  //the surface
  void SetPrecision(double u, double v){u_prec=u;v_prec=v;};
	  
  //Select a point for editing
  void SelectPoint(Point3d n, Matrix A, Point3d t);
  //Move the selected point
  void MovePoint(Point3d m, Matrix A, Point3d t);
	  
  //Select a region for editing
  void SelectRegion(vector<Point3d > s_reg, Matrix A, Point3d t);
  //Determine whether to_check fall in the contour defined by  shape
  bool InsideContour(vector<Point3d > shape, Point3d to_check);
	  
  int SelectedLevel()
    {return selected_level;}
  vector<int*> SelectedIndicies()
    {return selected_region;}
  vector<vector<vector<Point3d > > >* ControlPoints()
    {return &Control_points;}


  //Move the selected region
  void  MoveRegion(Point3d dis);
  //Apply the bending tool to the selected region
  //void RotateRegion(Point2d c_rot,double theta,double r_dis, Matrix A);
	  
  //renders the control points as circles
  void DisplayControlPoints(Matrix A, Point3d t);
  //renders the surface based on options contained in the class
  void DisplaySurface();
  //draws a chircel at x,y,z with radius w
  void DrawCircle(double x, double y, double z, double w);
	  
  //Functions working on the parameter range (1.0,n_u+degree-1.0)x(1.0,n_v+degree-1.0)
  //evaluates the b-spline surface at (u,v)
  Point3d Eval(double u, double v);
  //evaluates the u-curve at u
  Point3d EvalU(double u, int m);
  //evaluates the normal at u,v
  Point3d NormalEval(double u, double v); 
  //evaluates F(u,v)/du
  const Point3d PartialU(double u, double v);
  //evaluates F(u,v)/dv
  const Point3d PartialV(double u, double v);
  //Locates the closest point to q on the surface using p0 as
  //an initial guess
  bool PointInversion(Point2d &surf_p, Point2d p0, Point3d q);
	  
  //The same functionality as above, but for the parameter range (0,1)x(0,1)
  Point3d EvalN(double u, double v);
  Point3d NormalEvalN(double u, double v); 
  Point3d PartialUN(double u, double v);
  Point3d PartialVN(double u, double v);
       
  bool PointInversionN(Point2d &surf_p, Point2d p0, Point3d q);
  double ConvertU(double u) { return (u*(n_u-degree) )+3.0; }
  double ConvertV(double v) { return (v*(n_v-degree) )+3.0; }
	  
  //Parameterized representation of the curve along the lines
  //(0,0)(0,1),(0,1)(1,1),(1,1)(1,0),(1,0)(0,0) in the order given
  //this traces out the boundary of the b-spline patch, u varies between
  //0 and 1
  Point3d ContourEval(double u);

  Point2d ContourInverse(double u);
  
  BSplineSurface Interpolate(const BSplineSurface &BS1, const BSplineSurface &BS2, double alpha, int num_rows, int num_cols);
  unsigned int LowestResDim(int i);
  BSplineSurface LowestRes();
	  
  void UpdateFootprint();
  void RenderSupport(unsigned int i,unsigned int j,unsigned int level);
  void RenderFootprint();
  void CalcFootprint(unsigned int i,unsigned  int j,unsigned int level);
  void CalcPreImage(int i, int j, unsigned int level);

  void  RenderLine(int i1, int j1, int level1, int i2, int j2, int level2);

  BSplineSurface& operator=(const BSplineSurface& B);
  BSplineSurface operator+(const BSplineSurface& B);
  BSplineSurface operator-(const BSplineSurface& B);
  BSplineSurface operator*(const double c);
  
  int DimU(){return n_u;};
  int DimV(){return n_v;};

  int DivU() {return _divU;}
  int DivV() {return _divV;}

private:
  //	UVPrecision _uvPrecision;
  inline void Set(int r, int c, Point3d v)
    { Control_points[0][r][c] = v; }
  inline Point3d Get(int r, int c) const
    { return Control_points[0][r][c]; }

  std::string _fname;
  double _scale;

  int _TextureId;
	
  int PtId(int s, int t) const
  {
    return s+(_divU+1)*t;
  }

  int _divU, _divV;
  vector<Point3d > _vrtx;
  vector<Point3d > _nrml;
  vector<float > _ctract; 
  float c_min,c_max;

  bool precomputed;
  std::string _f_name;
	
  bool s_mesh;
  bool wireframe;
  bool lines;
  bool contraction;
  bool disp_normal;
  bool disp_c_mesh;
  bool periodic;
	
  unsigned int selected[2];
  int selected_level;
  int n_u,n_v;

  double sel_point[3];
  double unsel_point[3];
  double poly_line[3];
  int degree;

  vector< vector< vector< Point3d > > > Control_points;
  vector< vector< vector< Point3d > > > Details;

  vector< int* > footprint;
  vector< int* > selected_region;
  int selected_bb[4];

  double u_prec,v_prec;  

  BSplineSurface *psurf;
/*
  void Update_Mres();
 */

};
} // namespace Shapes

using Shapes::BSplineSurface;

#endif
