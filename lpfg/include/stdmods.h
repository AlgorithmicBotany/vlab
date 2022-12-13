#ifndef __STDMODS_H__
#define __STDMODS_H__

module SB() = 0;
module EB() = 1;
module F(float) = 2;
module f(float) = 3;
module Left(float) = 4;
module Right(float) = 5;
module Down(float) = 6;
module Up(float) = 7;
module RollL(float) = 8;
module RollR(float) = 9;
module IncColor() = 10;
module DecColor() = 11;
module SetColor(int) = 12;
module SetWidth(float) = 13;
module Label(__lc_Text) = 14;
module TurnAround() = 15;
module Cut() = 16;
module GetPos(float, float, float) = 17;
module GetHead(float, float, float) = 18;
module GetLeft(float, float, float) = 19;
module GetUp(float, float, float) = 20;
module Circle(float) = 21;
module MoveTo(float, float, float) = 22;
module Sphere(float) = 23;
module SetHead(float, float, float, float, float, float) = 24;
module Sphere0() = 25;
module Circle0() = 26;
module Line2f(V2tf, V2tf) = 27;
module Line2d(V2td, V2td) = 28;
module Line3f(V3tf, V3tf) = 29;
module Line3d(V3td, V3td) = 30;
module LineTo2f(V2tf) = 31;
module LineTo2d(V2td) = 32;
module LineTo3f(V3tf) = 33;
module LineTo3d(V3td) = 34;
module LineRel2f(V2tf) = 35;
module LineRel2d(V2td) = 36;
module LineRel3f(V3tf) = 37;
module LineRel3d(V3td) = 38;
module Surface(int, float) = 39;
module RollToVert() = 40;
module SetElasticity(int, float) = 41;
module IncElasticity(int) = 42;
module DecElasticity(int) = 43;
module CurrentContour(int) = 44;
module StartGC() = 45;
module PointGC() = 46;
module EndGC() = 47;
module Mesh(int,float) = 48;
module E1(float) = 49;
module MoveTo2f(V2tf) = 50;
module MoveTo2d(V2td) = 51;
module MoveTo3f(V3tf) = 52;
module MoveTo3d(V3td) = 53;
module E2(float, float) = 54;
module SP() = 55;
module EP() = 56;
module PP() = 57;
module Rhombus(float, float) = 58;
module Triangle(float, float) = 59;
module MouseIns() = 60;
module BlendedContour(int, int, float) = 61;
module CurrentTexture(int) = 62;
module TextureVCoeff(float) = 63;
module Orient() = 64;
module ScaleContour(float, float) = 65;
module Elasticity(float) = 66;
module DSurface(SurfaceObj) = 67;
module G(float) = 68;
module g(float) = 69;
module MouseInsPos(MouseStatus) = 70;
module Surface3(int, float, float, float) = 71;
module ContourSides(int) = 72;
module InitSurface(int) = 73;
module SurfacePoint(int, int, int) = 74;
module DrawSurface(int) = 75;
module Propensity(float) = 76;
module CircleFront(float) = 77;
module CircleFront0() = 78;
module SetUPrecision(int) = 79;
module SetVPrecision(int) = 80;
module LineTo(float, float, float) = 81;
module BSurface(int, float) = 82;
module MoveRel2f(V2tf) = 83;
module MoveRel2d(V2td) = 84;
module MoveRel3f(V3tf) = 85;
module MoveRel3d(V3td) = 86;
module DBSurfaceS(BsurfaceObjS) = 87;
module DBSurfaceM(BsurfaceObjM) = 88;
module Camera() = 89;
module EA20(EA20Array) = 90;
module Rotate(V3tf, float) = 91;
module RotateHLU(V3tf, float) = 92;
module RotateXYZ(V3tf, float) = 93;
module Terrain(CameraPosition) = 94;
module PovRayStart(__lc_Text, POVRayMeshMode) = 95;
module SetHead3f(V3tf) = 96;
module SetTropismDirection3f(V3tf) = 97;
module CircleB(float) = 98;
module CircleFrontB(float) = 99;
module SetCoordinateSystem(float) = 100;
module Mesh3(int, float, float, float) = 101;

#endif