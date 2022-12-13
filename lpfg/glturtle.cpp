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


#define GL_SILENCE_DEPRECATION
#include "turtle.h"
#include "comlineparam.h"
#include "glutils.h"
#include "glenv.h"
#include "surfarr.h"
#include "bsurfarr.h"
#include "utils.h"
#include "texturearr.h"
#include "drawparam.h"
#include "BsurfaceObj.h"
#include "mesharr.h" // MC - Dec. 2020 - for drawing meshes
#ifdef WIN32
#include "fg_geometry.h"
#endif
#ifndef WIN32
#include "glwidget.h"
#endif
void ScreenTurtle::Label(const char *txt) const {

  if (!comlineparam.ColormapMode()) {
    glDisable(GL_LIGHTING);
  }
#ifndef WIN32

  Vector3d colors = gl.GetTextColor(_color);
  QColor qcolor((int)(colors[0] * 255), (int)(colors[1] * 255),
                (int)(colors[2] * 255));

#endif

#ifdef _WINDOWS
  if (!comlineparam.ColormapMode()) {
    gl.SetSolidColor(_color);
  } else
    gl.SetColor(_color);

  glRasterPos3fv(_position);
  glListBase(_glbase);
  glCallLists(static_cast<GLsizei>(strlen(txt)), GL_UNSIGNED_BYTE,
              (GLubyte *)txt);
#else

  DrawParams::Font font = drawparams.GetFont();
  const QFont qfont = drawparams.GetQFont();
  _glview->renderText(_position.X(), _position.Y(), _position.Z(), txt, qcolor,
                      qfont);

#endif // !_WINDOWS

  if (!comlineparam.ColormapMode()) {
    glEnable(GL_LIGHTING);
  }
}

void ScreenTurtle::Circle(float r) const {
  if (r <= 0.0)
    return;

  gl.SetColor(_color);

  glPPM ppm;

  glTranslatef(_position.X(), _position.Y(), _position.Z());

  const float rot[16] = {-_left.X(),   -_left.Y(),   -_left.Z(),   0.0f,
                         _heading.X(), _heading.Y(), _heading.Z(), 0.0f,
                         _up.X(),      _up.Y(),      _up.Z(),      0.0f,
                         0.0f,         0.0f,         0.0f,         1.0f};

  glMultMatrixf(rot);

  glNormal3f(0.0f, 0.0f, 1.0f);
  gl.Disk(0.0, r,
          _divisions == divUnspecified ? drawparams.ContourDivisions()
                                       : _divisions,
          1, _pQ);
}
void ScreenTurtle::CircleB(float r) const {
  if (r <= 0.0)
    return;

  gl.SetColor(_color);

  glPPM ppm;

  glTranslatef(_position.X(), _position.Y(), _position.Z());

  const float rot[16] = {-_left.X(),   -_left.Y(),   -_left.Z(),   0.0f,
                         _heading.X(), _heading.Y(), _heading.Z(), 0.0f,
                         _up.X(),      _up.Y(),      _up.Z(),      0.0f,
                         0.0f,         0.0f,         0.0f,         1.0f};

  glMultMatrixf(rot);

  glNormal3f(0.0f, 0.0f, 1.0f);

  if (drawparams.LineStyle() == DParams::lsCylinder) {
    glutSolidTorus(Width(), r,
                   _divisions == divUnspecified ? drawparams.ContourDivisions()
                                                : _divisions,
                   _divisions == divUnspecified ? drawparams.ContourDivisions()
                                                : _divisions);
  } else // LineStyle lsPixel and lsPolygon.
  {
    gl.Disk(r - Width() / 2.0f, r + Width() / 2.0f,
            _divisions == divUnspecified ? drawparams.ContourDivisions()
                                         : _divisions,
            1, _pQ);
  }
}

void ScreenTurtle::CircleFront(float r) const {
  Vector3d left = _heading % _ViewNormal;
  if (left.Length() < epsilon)
    left = _left % _ViewNormal;
  left.Normalize();
  Vector3d up = left % _ViewNormal;

  gl.SetColor(_color);

  glPPM ppm;

  glTranslatef(_position.X(), _position.Y(), _position.Z());
  const float rot[16] = {up.X(),
                         up.Y(),
                         up.Z(),
                         0.0f,
                         -left.X(),
                         -left.Y(),
                         -left.Z(),
                         0.0f,
                         _ViewNormal.X(),
                         _ViewNormal.Y(),
                         _ViewNormal.Z(),
                         0.0f,
                         0.0f,
                         0.0f,
                         0.0f,
                         1.0f};
  glMultMatrixf(rot);
  glNormal3f(0.0f, 0.0f, 1.0f);
  gl.Disk(0.0f, r,
          _divisions == divUnspecified ? drawparams.ContourDivisions()
                                       : _divisions,
          1, _pQ);
}
void ScreenTurtle::CircleFrontB(float r) const {
  Vector3d left = _heading % _ViewNormal;
  if (left.Length() < epsilon)
    left = _left % _ViewNormal;
  left.Normalize();
  Vector3d up = left % _ViewNormal;

  gl.SetColor(_color);

  glPPM ppm;

  glTranslatef(_position.X(), _position.Y(), _position.Z());
  const float rot[16] = {up.X(),
                         up.Y(),
                         up.Z(),
                         0.0f,
                         -left.X(),
                         -left.Y(),
                         -left.Z(),
                         0.0f,
                         _ViewNormal.X(),
                         _ViewNormal.Y(),
                         _ViewNormal.Z(),
                         0.0f,
                         0.0f,
                         0.0f,
                         0.0f,
                         1.0f};
  glMultMatrixf(rot);
  glNormal3f(0.0f, 0.0f, 1.0f);

  if (drawparams.LineStyle() == DParams::lsCylinder) {
    glutSolidTorus(Width(), r,
                   _divisions == divUnspecified ? drawparams.ContourDivisions()
                                                : _divisions,
                   _divisions == divUnspecified ? drawparams.ContourDivisions()
                                                : _divisions);
  } else // LineStyle lsPixel and lsPolygon.
  {
    gl.Disk(r - Width() / 2.0f, r + Width() / 2.0f,
            _divisions == divUnspecified ? drawparams.ContourDivisions()
                                         : _divisions,
            1, _pQ);
  }
}

void ScreenTurtle::Sphere(float r) const {
  if (r <= 0.0)
    return;

  gl.SetColor(_color);

  glPPM ppm;

  glTranslatef(_position.X(), _position.Y(), _position.Z());
  _Sphere(r);
}

void ScreenTurtle::_Sphere(float r) const {
  int numDiv =
      _divisions == divUnspecified ? drawparams.ContourDivisions() : _divisions;

  gl.Sphere(r, numDiv, (numDiv + 1) / 2, _pQ);
}

void ScreenTurtle::Surface(int id, float sx, float sy, float sz) const {
  if (!surfaces.ValidId(id)) {
    Utils::Message("Surface: Invalid id == %d. Module ignored.\n", id);
    return;
  }

  glPPM ppm;
  glOnOff nrmlz(GL_NORMALIZE);
  glTwoSidedLighting tsl;

  glTranslatef(_position.X(), _position.Y(), _position.Z());

  const float rot[16] = {-_left.X(),   -_left.Y(),   -_left.Z(),   0.0f,
                         _heading.X(), _heading.Y(), _heading.Z(), 0.0f,
                         _up.X(),      _up.Y(),      _up.Z(),      0.0f,
                         0.0f,         0.0f,         0.0f,         1.0f};

  glMultMatrixf(rot);

  gl.SetColor(_color);
  int oldTexture = surfaces.TextureId(id);
  if (_TextureOn()) {
    if (_CurrentTexture != -1)
      surfaces.SetTextureId(id, _CurrentTexture);
    else
      surfaces.DisableTexture(id);
  }

  if (surfaces.IsTextured(id)) {
    textures.MakeActive(surfaces.TextureId(id));
    glEnable(GL_TEXTURE_2D);
  }
  surfaces.Draw(id, sx, sy, sz, GetUVPrecision());
  // set textureId back to previous value (not sure it's necessary though)
  if (_TextureOn()) {
    surfaces.SetTextureId(id, oldTexture);
  }

  if ((surfaces.IsTextured(id)) || (_TextureOn())) {
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
  }
}

void ScreenTurtle::BSurface(int id, float sx, float sy, float sz) const {
  if (!bsurfaces.ValidId(id)) {
    Utils::Message("Surface: Invalid id == %d. Module ignored.\n", id);
    return;
  }

  glPPM ppm;
  glOnOff nrmlz(GL_NORMALIZE);
  glTwoSidedLighting tsl;

  glTranslatef(_position.X(), _position.Y(), _position.Z());

  const float rot[16] = {-_left.X(),   -_left.Y(),   -_left.Z(),   0.0f,
                         _heading.X(), _heading.Y(), _heading.Z(), 0.0f,
                         _up.X(),      _up.Y(),      _up.Z(),      0.0f,
                         0.0f,         0.0f,         0.0f,         1.0f};

  glMultMatrixf(rot);

  glScalef(sx, sy, sz);

  gl.SetColor(_color);

  b_wrapper &surface = bsurfaces.Get(id);

  if (surface.IsTextured()) {
    if (textures.Initialized(surface.TextureId())) {
      textures.MakeActive(surface.TextureId());
      glEnable(GL_TEXTURE_2D);
    } else
      return;
  }
  surface.Draw(GetUVPrecision());
  if (surface.IsTextured()) {
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
  }
}

void ScreenTurtle::DSurface(SurfaceObj s, bool trtl) const {
  glPPM ppm;
  glTwoSidedLighting tsl;
  glOnOff nrmlz(GL_NORMALIZE);
  if (trtl) {
    glTranslatef(_position.X(), _position.Y(), _position.Z());
    const float rot[16] = {-_left.X(),   -_left.Y(),   -_left.Z(),   0.0f,
                           _heading.X(), _heading.Y(), _heading.Z(), 0.0f,
                           _up.X(),      _up.Y(),      _up.Z(),      0.0f,
                           0.0f,         0.0f,         0.0f,         1.0f};
    glMultMatrixf(rot);
  }
  gl.SetColor(_color);
  Volume v;
  Patch p(s);
  p.Draw(Patch::tmNoTexture, v, GetUVPrecision().U(), GetUVPrecision().V());
}

void ScreenTurtle::DBSurfaceS(BsurfaceObjS s) const {

  glPPM ppm;
  glOnOff nrmlz(GL_NORMALIZE);
  glTwoSidedLighting tsl;

  glTranslatef(_position.X(), _position.Y(), _position.Z());

  const float rot[16] = {-_left.X(),   -_left.Y(),   -_left.Z(),   0.0f,
                         _heading.X(), _heading.Y(), _heading.Z(), 0.0f,
                         _up.X(),      _up.Y(),      _up.Z(),      0.0f,
                         0.0f,         0.0f,         0.0f,         1.0f};

  glMultMatrixf(rot);

  gl.SetColor(_color);

  b_wrapper surface(GetBsurface(s));

  surface.Draw(GetUVPrecision());
}

void ScreenTurtle::DBSurfaceM(BsurfaceObjM s) const {

  glPPM ppm;
  glOnOff nrmlz(GL_NORMALIZE);
  glTwoSidedLighting tsl;

  glTranslatef(_position.X(), _position.Y(), _position.Z());

  const float rot[16] = {-_left.X(),   -_left.Y(),   -_left.Z(),   0.0f,
                         _heading.X(), _heading.Y(), _heading.Z(), 0.0f,
                         _up.X(),      _up.Y(),      _up.Z(),      0.0f,
                         0.0f,         0.0f,         0.0f,         1.0f};

  glMultMatrixf(rot);

  gl.SetColor(_color);

  b_wrapper surface(GetBsurface(s));

  surface.Draw(GetUVPrecision());
}

void ScreenTurtle::ContourSides(int cs) {
  if (cs < LPFGParams::MinContourDivisions) {
    Utils::Message("ContourSides: value %d is less than the minimum allowed "
                   "(%d). Ignored.\n",
                   cs, LPFGParams::MinContourDivisions);
  } else if (cs > LPFGParams::MaxContourDivisions - 1) {
    Utils::Message("ContourSides: value %d is greater than the maximum allowed "
                   "(%d). Using maximum.\n",
                   cs, LPFGParams::MaxContourDivisions - 1);
    _divisions = LPFGParams::MaxContourDivisions;
  } else
    // We have to set _divisions to cs+1 because glTriangleStip draws
    // 2*(_divisions-1) triangles if we give it 2*_divisions points.
    //_divisions = cs+1;
    //[PASCAL] it looks this is true only for the general cylinder, I have
    // ported this to gencylturtle only
    _divisions = cs;
}

void ScreenTurtle::StartPolygon() {
  if (_pPolygon->Started())
    Utils::Message("SP: previous polygon not ended. Resetting polygon\n");
  _pPolygon->Start();
}

void ScreenTurtle::EndPolygon() {
  if (_pPolygon->Started()) {
    gl.SetColor(_color);
    _pPolygon->End();
  } else
    Utils::Message("EP: No polygon started. Module ignored.\n");
}

void ScreenTurtle::PolygonPoint() {
  if (_pPolygon->Started()) {
    if (!_pPolygon->Full())
      _pPolygon->AddVertex(_position, _up, _color);
    else
      Utils::Message("PP: Too many vertices in the polygon. Module ignored.\n");
  } else
    Utils::Message("PP: No polygon started. Module ignored.\n");
}

void ScreenTurtle::Rhombus(float a, float b) const {
  gl.SetColor(_color);

  GLprimitive quads(GL_QUADS);

  glNormal3fv(_up);
  Vector3d v = _position;
  glVertex3fv(v);
  v += _heading * a * 0.5f;
  v += _left * b * 0.5f;
  glVertex3fv(v);
  v = _position + _heading * a;
  glVertex3fv(v);
  v = _position + _heading * a * 0.5f;
  v -= _left * b * 0.5f;
  glVertex3fv(v);
}

void ScreenTurtle::Triangle(float a, float b) const {
  gl.SetColor(_color);

  GLprimitive triangles(GL_TRIANGLES);

  glNormal3fv(_up);
  Vector3d v(_position);
  v += _left * a * 0.5f;
  glVertex3fv(v);
  v = _position + _heading * b;
  glVertex3fv(v);
  v = _position - _left * a * 0.5f;
  glVertex3fv(v);
}

void ScreenTurtle::Orient() const {
  GLboolean light_on = glIsEnabled(GL_LIGHTING);
  if (light_on)
    glDisable(GL_LIGHTING);
  GLboolean depth_test = glIsEnabled(GL_DEPTH_TEST);
  if (depth_test)
    glDisable(GL_DEPTH_TEST);

  {
    GLprimitive lines(GL_LINES);

    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3fv(_position);
    Vector3d e = _position + _heading;
    glVertex3fv(e);

    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3fv(_position);
    e = _position + _left;
    glVertex3fv(e);

    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3fv(_position);
    e = _position + _up;
    glVertex3fv(e);
  }

  if (depth_test)
    glEnable(GL_DEPTH_TEST);
  if (light_on)
    glEnable(GL_LIGHTING);
}

void ScreenTurtle::Terrain(CameraPosition camPos) const{

  glTranslatef(_position.X(), _position.Y(), _position.Z());

  const float rot[16] = {-_left.X(),   -_left.Y(),   -_left.Z(),   0.0f,
                         _heading.X(), _heading.Y(), _heading.Z(), 0.0f,
                         _up.X(),      _up.Y(),      _up.Z(),      0.0f,
                         0.0f,         0.0f,         0.0f,         1.0f};

  glMultMatrixf(rot);

  if (terrainData != NULL) {
    gl.SetColor(_color);

    if (terrainData->IsTextured()) {
      if (textures.Initialized(terrainData->TextureId())) {
        textures.MakeActive(terrainData->TextureId());
        glEnable(GL_TEXTURE_2D);
      }
    }

    terrainData->draw(camPos);

    if (terrainData->IsTextured()) {
      glDisable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, 0);
    }
  }
}

void PixelLineScreenTurtle::F(float v) {
  gl.SetColor(_color);
  glLineWidth(Width());

  GLprimitive lines(GL_LINES);
  glVertex3fv(_position);
  Turtle::F(v);
  glVertex3fv(_position);
}

void PixelLineScreenTurtle::StartGC() {
  Utils::Message("StartGC: Pixel line mode. Module ignored.\n");
}

void PolygonLineScreenTurtle::F(float v) {
  Vector3d left = _heading % ViewNormal();
  if (left.Length() < epsilon)
    // This can only happen if the turtle is heading into or out of the screen;
    // in this case, we want to draw nothing
    return;
  left.Normalize(0.5f * Width());

  // Using a class object "tsl" to set the light model in OpenGL and unset it
  // with destructor does not work as expected on all operating systems/OpenGL
  // implementations. For example, on Ubuntu 18, the two sided light model is
  // not unset before the next object is drawn! It is safer to explicitly call
  // the OpenGL functions at the start and end of the function:
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
  gl.SetColor(_color);
  GLprimitive quads(GL_QUADS);

  Vector3d vrt = _position - left;
  glVertex3fv(vrt);
  vrt = _position + left;
  glVertex3fv(vrt);

  Turtle::F(v);
  Vector3d newLeft = _heading % ViewNormal();
  // What do we do if the new turtle direction is directly out of the screen?
  // Just using the old left seems the easiest.
  if (newLeft.Length() > epsilon)
    left = newLeft;
  left.Normalize(0.5f * Width());

  vrt = _position + left;
  glVertex3fv(vrt);
  vrt = _position - left;
  glVertex3fv(vrt);

  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
}

void PolygonLineScreenTurtle::StartGC() {
  Utils::Message("StartGC: Polygon line mode. Module ignored.\n");
}

void ScreenTurtle::Mesh(int meshId, float sx, float sy, float sz) const
{
	// check if mesh exists
	if (!meshes.ValidMeshId(meshId))
	{
		Utils::Message("Mesh: Invalid id == %d. Module ignored.\n", meshId);
		return;
	}

	gl.SetColor(_color);

   	if (meshes.IsMeshTextured(meshId)) {
	    int texId = meshes.MeshTextureId(meshId);
	    if (_CurrentTexture != -1)
		    texId = _CurrentTexture;
	    if (texId != -1) {
		    textures.MakeActive(texId);
		    glEnable(GL_TEXTURE_2D);
	    }
    }

    glPPM ppm; // push current model-view matrix
    glOnOff nrmlz(GL_NORMALIZE);
    glTwoSidedLighting tsl;

    // set model transformation matrix
    const float model_mat[16] =
    {
    -_left.X(),		-_left.Y(),		-_left.Z(),		0.0f,
    _heading.X(),	_heading.Y(),	_heading.Z(),	0.0f,
    _up.X(),		_up.Y(),		_up.Z(),		0.0f,
    _position.X(),	_position.Y(),	_position.Z(),	1.0f
    };

    glMultMatrixf(model_mat);

    meshes.DrawMesh(meshId, sx, sy, sz);

	if (meshes.IsMeshTextured(meshId))
	{
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

}
/*
void ScreenTurtle::AnimatedMesh(int meshId, float scale, float t) const
{
	// check if mesh exists
	if (!meshes.ValidAnimatedMeshId(meshId))
	{
		Utils::Message("Animated Mesh: Invalid id == %d. Module ignored.\n", meshId);
		return;
	}

	gl.SetColor(_color);

	int texId = meshes.AnimatedMeshTextureId(meshId);
	if (_CurrentTexture != -1)
		texId = _CurrentTexture;
	if (texId != -1)
	{
		textures.MakeActive(texId);
		glEnable(GL_TEXTURE_2D);
	}

    glPPM ppm; // push current model-view matrix
    glOnOff nrmlz(GL_NORMALIZE);
    glTwoSidedLighting tsl;

    // set model transformation matrix
    const float model_mat[16] =
    {
    -_left.X(),		-_left.Y(),		-_left.Z(),		0.0f,
    _heading.X(),	_heading.Y(),	_heading.Z(),	0.0f,
    _up.X(),		_up.Y(),		_up.Z(),		0.0f,
    _position.X(),	_position.Y(),	_position.Z(),	1.0f
    };

    glMultMatrixf(model_mat);

    meshes.DrawAnimatedMesh(meshId, scale, t);

	if (meshes.IsAnimatedMeshTextured(meshId))
	{
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}
*/
