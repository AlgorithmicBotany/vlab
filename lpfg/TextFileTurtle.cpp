/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#include <string>

#include "turtle.h"

TextFileTurtle::TextFileTurtle(FILE *fp) : _fp(fp) {}

TextFileTurtle::~TextFileTurtle() {}

void TextFileTurtle::F(float v) {
  fprintf(_fp, "F(%f)\n", v);
  Turtle::F(v);
}

void TextFileTurtle::f(float v) {
  fprintf(_fp, "f(%f)\n", v);
  Turtle::f(v);
}

void TextFileTurtle::G(float v) {
  fprintf(_fp, "G(%f)\n", v);
  Turtle::G(v);
}

void TextFileTurtle::g(float v) {
  fprintf(_fp, "g(%f)\n", v);
  Turtle::g(v);
}

void TextFileTurtle::Left(float v) {
  fprintf(_fp, "Left(%f)\n", v);
  Turtle::Left(v);
}

void TextFileTurtle::Right(float v) {
  fprintf(_fp, "Right(%f)\n", v);
  Turtle::Right(v);
}

void TextFileTurtle::Down(float v) {
  fprintf(_fp, "Down(%f)\n", v);
  Turtle::Down(v);
}

void TextFileTurtle::Up(float v) {
  fprintf(_fp, "Up(%f)\n", v);
  Turtle::Up(v);
}

void TextFileTurtle::RollR(float v) {
  fprintf(_fp, "RollR(%f)\n", v);
  Turtle::RollR(v);
}

void TextFileTurtle::RollL(float v) {
  fprintf(_fp, "RollL(%f)\n", v);
  Turtle::RollL(v);
}

void TextFileTurtle::IncColor() {
  fprintf(_fp, "IncColor\n");
  Turtle::IncColor();
}

void TextFileTurtle::DecColor() {
  fprintf(_fp, "DecColor\n");
  Turtle::DecColor();
}

void TextFileTurtle::SetColor(int color) {
  fprintf(_fp, "SetColor(%d)\n", color);
  Turtle::SetColor(color);
}

void TextFileTurtle::SetWidth(float v) {
  fprintf(_fp, "SetWidth(%f)\n", v);
  Turtle::SetWidth(v);
}

void TextFileTurtle::Label(const char *szLabel) const {
  fprintf(_fp, "Label(%s)\n", szLabel);
  Turtle::Label(szLabel);
}

void TextFileTurtle::Circle(float radius) const {
  fprintf(_fp, "Circle(%f)\n", radius);
  Turtle::Circle(radius);
}

void TextFileTurtle::MoveTo(float x, float y, float z) {
  fprintf(_fp, "MoveTo(%f, %f, %f)\n", x, y, z);
  Turtle::MoveTo(x, y, z);
}

void TextFileTurtle::MoveRel(float x, float y, float z) {
  fprintf(_fp, "MoveRel(%f, %f, %f)\n", x, y, z);
  Turtle::MoveRel(x, y, z);
}

void TextFileTurtle::Sphere(float radius) const {
  fprintf(_fp, "Sphere(%f)\n", radius);
  Turtle::Sphere(radius);
}

void TextFileTurtle::SetHead(const float *arr) {
  fprintf(_fp, "SetHead(%f, %f, %f, %f, %f, %f)\n", arr[0], arr[1], arr[2],
          arr[3], arr[4], arr[5]);
  Turtle::SetHead(arr);
}

void TextFileTurtle::SetHeadOnly(V3f v) {
  fprintf(_fp, "SetHeadOnly(%f, %f, %f)\n", v.x, v.y, v.z);
  Turtle::SetHeadOnly(v);
}

void TextFileTurtle::Sphere0() const {
  fprintf(_fp, "Sphere0\n");
  Turtle::Sphere0();
}

void TextFileTurtle::Circle0() const {
  fprintf(_fp, "Circle0\n");
  Turtle::Circle0();
}

void TextFileTurtle::Line(V3f v1, V3f v2) {
  fprintf(_fp, "Line(%f, %f, %f, %f, %f, %f)\n", v1.x, v1.y, v1.z, v2.x, v2.y,
          v2.z);
  Turtle::Line(v1, v2);
}

void TextFileTurtle::LineTo(V3f v) {
  fprintf(_fp, "LineTo(%f, %f, %f)\n", v.x, v.y, v.z);
  Turtle::LineTo(v);
}

void TextFileTurtle::LineRel(V3f v) {
  fprintf(_fp, "LineRel(%f, %f, %f)\n", v.x, v.y, v.z);
  Turtle::LineRel(v);
}

void TextFileTurtle::Surface(int id, float sx, float sy, float sz) const {
  fprintf(_fp, "Surface(%d, %f, %f, %f)\n", id, sx, sy, sz);
  Turtle::Surface(id, sx, sy, sz);
}

void TextFileTurtle::RollToVert() {
  fprintf(_fp, "RollToVert\n");
  Turtle::RollToVert();
}

void TextFileTurtle::SetElasticity(int id, float value) {
  fprintf(_fp, "SetElasticity(%d, %f)\n", id, value);
  Turtle::SetElasticity(id, value);
}

void TextFileTurtle::IncElasticity(int id) {
  fprintf(_fp, "IncElasticity(%d)\n", id);
  Turtle::IncElasticity(id);
}

void TextFileTurtle::DecElasticity(int id) {
  fprintf(_fp, "DecElasticity(%d)\n", id);
  Turtle::DecElasticity(id);
}

void TextFileTurtle::CurrentContour(int id) {
  fprintf(_fp, "CurrentContour(%d)\n", id);
  Turtle::CurrentContour(id);
}

void TextFileTurtle::StartGC() {
  fprintf(_fp, "StartGC\n");
  Turtle::StartGC();
}

void TextFileTurtle::PointGC() {
  fprintf(_fp, "PointGC\n");
  Turtle::StartGC();
}

void TextFileTurtle::EndGC() {
  fprintf(_fp, "EndGC\n");
  Turtle::EndGC();
}

// from mesh_unused.h/.cpp
//void TextFileTurtle::MeshPoint() {
//  fprintf(_fp, "MeshPoint\n");
//  Turtle::MeshPoint();
//}

void TextFileTurtle::E1(float v, const LstringIterator &iterator) {
  fprintf(_fp, "E1(%f)\n", v);
  Turtle::E1(v, iterator);
}

void TextFileTurtle::E2(float v1, float v2, const LstringIterator &iterator) {
  fprintf(_fp, "E2(%f, %f)\n", v1, v2);
  Turtle::E2(v1, v2, iterator);
}

void TextFileTurtle::StartPolygon() {
  fprintf(_fp, "StartPolygon\n");
  Turtle::StartPolygon();
}

void TextFileTurtle::EndPolygon() {
  fprintf(_fp, "EndPolygon\n");
  Turtle::EndPolygon();
}

void TextFileTurtle::PolygonPoint() {
  fprintf(_fp, "PolygonPoint\n");
  Turtle::PolygonPoint();
}

void TextFileTurtle::Rhombus(float d1, float d2) const {
  fprintf(_fp, "Rhombus(%f, %f)\n", d1, d2);
  Turtle::Rhombus(d1, d2);
}

void TextFileTurtle::Triangle(float a, float b) const {
  fprintf(_fp, "Triangle(%f, %f)\n", a, b);
  Turtle::Triangle(a, b);
}

void TextFileTurtle::BlendContours(int id1, int id2, float weight) {
  fprintf(_fp, "BlendContours(%d, %d, %f)\n", id1, id2, weight);
  Turtle::BlendContours(id1, id2, weight);
}

void TextFileTurtle::CurrentTexture(int id) {
  fprintf(_fp, "CurrentTexture(%d)\n", id);
  Turtle::CurrentTexture(id);
}

void TextFileTurtle::TextureVCoeff(float v) {
  fprintf(_fp, "TextureVCoeff(%f)\n", v);
  Turtle::TextureVCoeff(v);
}

void TextFileTurtle::Orient() const {
  fprintf(_fp, "Orient\n");
  Turtle::Orient();
}

void TextFileTurtle::ScaleContour(float sw, float sh) {
  fprintf(_fp, "ScaleContour(%f, %f)\n", sw, sh);
  Turtle::ScaleContour(sw, sh);
}

void TextFileTurtle::Elasticity(float v) {
  fprintf(_fp, "Elasticity(%f)\n", v);
  Turtle::Elasticity(v);
}

void TextFileTurtle::STropismDirection(V3f dir) {
  fprintf(_fp, "STropismDirection(%f,%f,%f)\n", dir.x, dir.y, dir.z);
  Turtle::STropismDirection(dir);
}

void TextFileTurtle::DSurface(SurfaceObj surface, bool trtl) const {
  fprintf(_fp, "DSurface(surface, %s)\n", trtl ? "true" : "false");
  Turtle::DSurface(surface, trtl);
}

void TextFileTurtle::ContourSides(int sides) {
  fprintf(_fp, "ContourSides(%d)\n", sides);
  Turtle::ContourSides(sides);
}

void TextFileTurtle::InitSurface(int id) const {
  fprintf(_fp, "InitSurface(%d)\n", id);
  Turtle::InitSurface(id);
}

void TextFileTurtle::SurfacePoint(int n, int r, int c) {
  fprintf(_fp, "SurfcePoint(%d, %d, %d)\n", n, r, c);
  Turtle::SurfacePoint(n, r, c);
}

void TextFileTurtle::DrawSurface(int id) const {
  fprintf(_fp, "DrawSurface(%d)\n", id);
  Turtle::DrawSurface(id);
}

void TextFileTurtle::CircleFront(float v) const {
  fprintf(_fp, "CircleFront(%f)\n", v);
  Turtle::CircleFront(v);
}

void TextFileTurtle::CircleFront0() const {
  fprintf(_fp, "CircleFront0\n");
  Turtle::CircleFront0();
}

void TextFileTurtle::SetUPrecision(int u) {
  fprintf(_fp, "SetUPrecision(%d)\n", u);
  Turtle::SetUPrecision(u);
}

void TextFileTurtle::SetVPrecision(int v) {
  fprintf(_fp, "SetVPrecision(%d)\n", v);
  Turtle::SetVPrecision(v);
}

void TextFileTurtle::BSurface(int id, float sx, float sy, float sz) const {
  fprintf(_fp, "BSurface(%d, %f, %f, %f)\n", id, sx, sy, sz);
  Turtle::BSurface(id, sx, sy, sz);
}

void TextFileTurtle::DBSurfaceS(BsurfaceObjS surface) const {
  fprintf(_fp, "DBSurfaceS(surface)\n");
  Turtle::DBSurfaceS(surface);
}

void TextFileTurtle::DBSurfaceM(BsurfaceObjM surface) const {
  fprintf(_fp, "DBSurfaceM(surface)\n");
  Turtle::DBSurfaceM(surface);
}

void TextFileTurtle::Camera() {
  fprintf(_fp, "Camera\n");
  Turtle::Camera();
}

void TextFileTurtle::EA(int iNumParams, const float *arr,
                        const LstringIterator &iterator) {
  fprintf(_fp, "EA(%d, ", iNumParams);
  for (int iParam = 0; iParam < iNumParams; ++iParam) {
    fprintf(_fp, "%f", arr[iParam]);
    if (iParam < iNumParams - 1)
      fprintf(_fp, ", ");
  }
  fprintf(_fp, ")\n");
  Turtle::EA(iNumParams, arr, iterator);
}

void TextFileTurtle::RotateXYZ(V3f axis, float angle) {
  fprintf(_fp, "RotateXYZ(%f, %f, %f, %f)\n", axis.x, axis.y, axis.z, angle);
  Turtle::RotateXYZ(axis, angle);
}

void TextFileTurtle::RotateHLU(V3f axis, float angle) {
  fprintf(_fp, "RotateHLU(%f, %f, %f, %f)\n", axis.x, axis.y, axis.z, angle);
  Turtle::RotateHLU(axis, angle);
}

void TextFileTurtle::Terrain(CameraPosition camera) const {
  fprintf(_fp, "Terrain(camera)\n");
  Turtle::Terrain(camera);
}

void TextFileTurtle::PovRayStart(const char *szString,
                                 POVRayMeshMode mode) {
  fprintf(_fp, "POVRayStart(%s, mode)\n", szString);
  Turtle::PovRayStart(szString, mode);
}

void TextFileTurtle::Mesh(int id, float sx, float sy, float sz) const {
  fprintf(_fp, "Mesh(%d, %f, %f, %f)\n", id, sx, sy, sz);
  Turtle::Mesh(id,sx,sy,sz);
}
/*
void TextFileTurtle::AnimatedMesh(int id, float scale, float t) const {
  fprintf(_fp, "AnimatedMesh(%d, %f, %f)\n", id, scale, t);
  Turtle::AnimatedMesh(id,scale,t);
}
*/
