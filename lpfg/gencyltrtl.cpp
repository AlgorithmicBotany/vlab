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



#include "turtle.h"
#include "glenv.h"
#include "utils.h"
#include "contourarr.h"
#include "texturearr.h"
#include "glutils.h"
#include "drawparam.h"
#include "polygon.h"
#include "quaternion.h"

CylinderLineScreenTurtle::CylinderLineScreenTurtle(unsigned int glbase,
                                                   Vector3d vn,
                                                   GLDraw::Polygon *pPolygon,
                                                   void *pQ)
    : ScreenTurtle(glbase, vn, pPolygon, pQ), _CurrentContour(0),
      _ContourId2(0), _blender(0.0f), _allowBranchGC(false) {
  CurrentContour(0);
  _textureV = 0.0f;
  _textureVCoeff = 1.0f;
}

void CylinderLineScreenTurtle::F(float v) {
  gl.SetColor(_color);
  if (_gc.On()) {
    _GCF(v);
    _gc.Set(_Scale, _position, _up, _left, _heading, _CurrentContour,
            _ContourId2, _blender);
  } else
    _NormalF(v);
}

void CylinderLineScreenTurtle::CurrentContour(int id) {
  if (!contours.ValidId(id)) {
    Utils::Message("CurrentContour: Invalid id = %d. Module ignored.\n", id);
    return;
  }
  _CurrentContour = id;
  _blender = 0.0f;
  _UpdateContourDivisions(_CurrentContour);
}

void CylinderLineScreenTurtle::ContourSides(int cs) {
  if (GCStarted())
    Utils::Message(
        "ContourSides: called between StartGC and EndGC. Ignored.\n");
  else {
    ScreenTurtle::ContourSides(cs);
    if (_divisions != divUnspecified) {
      _UpdateContourDivisions(_CurrentContour);
      if (0.0f != _blender)
        _UpdateContourDivisions(_ContourId2);
    }
  }
}

void CylinderLineScreenTurtle::ContourNormal(V3f n) {
  _gc.SetNormal(Vector3d(n.x,n.y,n.z));
}

void CylinderLineScreenTurtle::_UpdateContourDivisions(size_t id) {
  if (_divisions != divUnspecified)
    contours.GetAccess(id).SetDivisions(_divisions);
  else if (contours.GetAccess(id).DivisionsSpecified())
    contours.GetAccess(id).SetDivisions(
        contours.GetAccess(id).OriginalDivisions());
  else
    // [PASCAL]
    // We have to set _divisions to cs+1 because glTriangleStip draws
    // 2*(_divisions-1) triangles if we give it 2*_divisions points.
    contours.GetAccess(id).SetDivisions(drawparams.ContourDivisions() + 1);
}

void CylinderLineScreenTurtle::_NormalF(float v) {
  const float bgnrot[16] = {_left.X(),    _left.Y(),    _left.Z(),    0.0f,
                            _up.X(),      _up.Y(),      _up.Z(),      0.0f,
                            _heading.X(), _heading.Y(), _heading.Z(), 0.0f,
                            0.0f,         0.0f,         0.0f,         1.0f};
  float bgnmtrx[16];
  {
    glPPM ppm;
    glLoadIdentity();
    glTranslatef(_position.X(), _position.Y(), _position.Z());
    glMultMatrixf(bgnrot);
    glScalef(0.5f * _ScaleP(), 0.5f * _ScaleQ(), 1.0f);
    glGetFloatv(GL_MODELVIEW_MATRIX, bgnmtrx);
  }

  Turtle::F(v);

  const float endrot[16] = {_left.X(),    _left.Y(),    _left.Z(),    0.0f,
                            _up.X(),      _up.Y(),      _up.Z(),      0.0f,
                            _heading.X(), _heading.Y(), _heading.Z(), 0.0f,
                            0.0f,         0.0f,         0.0f,         1.0f};
  float endmtrx[16];
  {
    glPPM ppm;
    glLoadIdentity();
    glTranslatef(_position.X(), _position.Y(), _position.Z());
    glMultMatrixf(endrot);
    glScalef(0.5f * _ScaleP(), 0.5f * _ScaleQ(), 1.0f);
    glGetFloatv(GL_MODELVIEW_MATRIX, endmtrx);
  }

  if (_TextureOn()) {
    glEnable(GL_TEXTURE_2D);
    textures.MakeActive(_CurrentTexture);
  }

  const float endtv = _textureV + v * _textureVCoeff;

  {
    GLprimitive glp(GL_TRIANGLE_STRIP);
    Vector3d vtx;
    const Contour &contour = contours.Get(_CurrentContour);
    for (size_t i = 0; i < contour.Divisions(); ++i) {
      if (drawparams.RenderMode() == DParams::rmShaded ||
          drawparams.RenderMode() == DParams::rmShadows) {
        vtx.Transform(contour.Normal(i), bgnrot);
        glNormal3fv(vtx);
      }
      vtx.Transform(contour.Vertex(i), bgnmtrx);

      if (_TextureOn())
        glTexCoord2f(i * 1.0f / (contour.Divisions() - 1), _textureV);

      glVertex3fv(vtx);
      if (drawparams.RenderMode() == DParams::rmShaded ||
          drawparams.RenderMode() == DParams::rmShadows) {
        vtx.Transform(contour.Normal(i), endrot);
        glNormal3fv(vtx);
      }
      vtx.Transform(contour.Vertex(i), endmtrx);

      if (_TextureOn())
        glTexCoord2f(i * 1.0f / (contour.Divisions() - 1), endtv);

      glVertex3fv(vtx);
    }
  }

  if (_TextureOn()) {
    glDisable(GL_TEXTURE_2D);
    glBindTexture(
        GL_TEXTURE_2D,
        0); // MC - Nov. 2016 - reset to default texture for shadows mode
    _textureV = endtv;
    if (_textureV > 1.0f)
      _textureV -= 1.0f;
  }
}

void CylinderLineScreenTurtle::_GCF(float v) {
  ASSERT(_gc.On());

  Turtle::F(v);
  float bgnmtrx[16];
  float endmtrx[16];
  {
    glPPM ppm;
    glLoadIdentity();
    glTranslatef(_gc.Position().X(), _gc.Position().Y(), _gc.Position().Z());
    {
      const float rot[16] = {_gc.Left().X(),
                             _gc.Left().Y(),
                             _gc.Left().Z(),
                             0.0f,
                             _gc.Up().X(),
                             _gc.Up().Y(),
                             _gc.Up().Z(),
                             0.0f,
                             _gc.Heading().X(),
                             _gc.Heading().Y(),
                             _gc.Heading().Z(),
                             0.0f,
                             0.0f,
                             0.0f,
                             0.0f,
                             1.0f};
      glMultMatrixf(rot);
    }
    glScalef(0.5f * _gc.ScaleP(), 0.5f * _gc.ScaleQ(), 1.0f);
    glGetFloatv(GL_MODELVIEW_MATRIX, bgnmtrx);
    glLoadIdentity();
    glTranslatef(_position.X(), _position.Y(), _position.Z());
    {
      const float rot[16] = {_left.X(),    _left.Y(),    _left.Z(),    0.0f,
                             _up.X(),      _up.Y(),      _up.Z(),      0.0f,
                             _heading.X(), _heading.Y(), _heading.Z(), 0.0f,
                             0.0f,         0.0f,         0.0f,         1.0f};
      glMultMatrixf(rot);
    }
    glScalef(0.5f * _ScaleP(), 0.5f * _ScaleQ(), 1.0f);
    glGetFloatv(GL_MODELVIEW_MATRIX, endmtrx);
  }

  const float bgnnrm[16] = {_gc.Left().X(),
                            _gc.Left().Y(),
                            _gc.Left().Z(),
                            0.0f,
                            _gc.Up().X(),
                            _gc.Up().Y(),
                            _gc.Up().Z(),
                            0.0f,
                            _gc.Heading().X(),
                            _gc.Heading().Y(),
                            _gc.Heading().Z(),
                            0.0f,
                            0.0f,
                            0.0f,
                            0.0f,
                            1.0f};
  const float endnrm[16] = {_left.X(),    _left.Y(),    _left.Z(),    0.0f,
                            _up.X(),      _up.Y(),      _up.Z(),      0.0f,
                            _heading.X(), _heading.Y(), _heading.Z(), 0.0f,
                            0.0f,         0.0f,         0.0f,         1.0f};

  if (_TextureOn()) {
    glEnable(GL_TEXTURE_2D);
    textures.MakeActive(_CurrentTexture);
  }

  //const float endtv = _textureV + v * _textureVCoeff;
  // MC - Dec. 2020, modified line above so that if F is called with v==0,
  // the 'v' coordinate is advanced by textureVCoeff (instead of by zero).
  // This was added to texture surfaces of revolution drawn using StartGC Right(ang) F(0) ... EndGC
  // but it may cause strange texturing otherwise...
  // Also, the condition is only checked in the case of _GCF() and not in _Normal(), and is
  // the same for the OBJ, POVRAY, and RAYSHADE turtles
  float endtv = _textureV + v * _textureVCoeff;
  if (v == 0.f)
    endtv =  _textureV + _textureVCoeff;

  {
    GLprimitive glp(GL_TRIANGLE_STRIP);
    Vector3d vtx;
    Vector3d normal; 
    const Contour &bgncntr =
        contours.Get1(_gc.ContourId(), _gc.ContourId2(), _gc.Blender());
    const Contour &endcntr =
        contours.Get2(_CurrentContour, _ContourId2, _blender);
    for (size_t i = 0; i < bgncntr.Divisions(); ++i) {

      if (drawparams.TaperedCylinders()) {
        // get normal at begin vertex
        Vector3d bgnV, bgnN;
        bgnN.Transform(bgncntr.Normal(i), bgnnrm);
        bgnV.Transform(bgncntr.Vertex(i), bgnmtrx);
        // get normal at end vertex
        Vector3d endV, endN;
        endN.Transform(endcntr.Normal(i), endnrm);
        endV.Transform(endcntr.Vertex(i), endmtrx);
        // compute vector between end and begin, and normalize it
        Vector3d vec1 = (endV - bgnV).Normalize();
        Vector3d bgnV2;
        if (i+1 < bgncntr.Divisions())
          bgnV2.Transform(bgncntr.Vertex(i+1), bgnmtrx);
        else // don't pick Vertex(0) because it is the same as Vertex(-1), pick Vertex(1)
          bgnV2.Transform(bgncntr.Vertex(1), bgnmtrx);
        Vector3d vec2 = (bgnV2 - bgnV).Normalize();

        vtx = (vec1 % vec2).Normalize();
        normal = vtx;
      } else {
        // set the normal
        vtx.Transform(bgncntr.Normal(i), bgnnrm);
        // check if CurrentNormal was specified
        if (_gc.changedNormal()) {
          const float *cntr_n = bgncntr.Normal(i);
          Vector3d perp (cntr_n[1], -cntr_n[0], 0.);
          Vector3d tangent;
          tangent.Transform(perp, bgnnrm);

          Vector3d n_curr = _gc.bgnNormal();
          Vector3d def(-1.f,0.f,0.f);
          float angle = acosf(def * n_curr); // assumes both vectors are normalized
          Vector3d up(0.f,1.f,0.f);
          if ((up * n_curr) <= 1e-4)
            angle *= -0.5f;
          else
            angle *= 0.5f;
          Quaternion q(cosf(angle), tangent.Normalize(sinf(angle)));
          vtx.RotateBy(q);
        }
      }
      glNormal3fv(vtx);

      if (_TextureOn())
        glTexCoord2f(i * 1.0f / (bgncntr.Divisions() - 1), _textureV);

      vtx.Transform(bgncntr.Vertex(i), bgnmtrx);
      glVertex3fv(vtx);

      // end point
      vtx.Transform(endcntr.Normal(i), endnrm);
      if (drawparams.TaperedCylinders()) {
        // // get normal at begin vertex
        // Vector3d bgnV, bgnN;
        // bgnN.Transform(bgncntr.Normal(i), bgnnrm);
        // bgnV.Transform(bgncntr.Vertex(i), bgnmtrx);
        // // get normal at end vertex -- QQQ -- don't know what the next vertex is yet!!!!!
        // Vector3d endV, endN;
        // endN.Transform(endcntr.Normal(i), endnrm);
        // endV.Transform(endcntr.Vertex(i), endmtrx);
        // // compute vector between end and begin, and normalize it
        // Vector3d vec1 = (endV - bgnV).Normalize();
        // Vector3d bgnV2;
        // int next_i = i+1 < bgncntr.Divisions() ? i + 1 : 0;
        // bgnV2.Transform(bgncntr.Vertex(next_i), bgnmtrx);
        // Vector3d vec2 = (bgnV2 - bgnV).Normalize();

        // vtx = (vec1 % vec2).Normalize();
        vtx = normal;
      } else {
        // check if CurrentNormal was specified
        if (_gc.changedNormal()) {
          const float *cntr_n = endcntr.Normal(i);
          Vector3d perp (cntr_n[1], -cntr_n[0], 0.);
          Vector3d tangent;
          tangent.Transform(perp, endnrm);

          Vector3d n_curr = _gc.endNormal();
          Vector3d def(-1.f,0.f,0.f);
          float angle = acosf(def * n_curr); // assumes both vectors are normalized
          Vector3d up(0.f,1.f,0.f);
          if ((up * n_curr) <= 1e-4)
            angle *= -0.5f;
          else
            angle *= 0.5f;
          Quaternion q(cosf(angle), tangent.Normalize(sinf(angle)));
          vtx.RotateBy(q);
        }
      }
      glNormal3fv(vtx);

      if (_TextureOn())
        glTexCoord2f(i * 1.0f / (bgncntr.Divisions() - 1), endtv);

      vtx.Transform(endcntr.Vertex(i), endmtrx);
      glVertex3fv(vtx);
    }
  }

  if (_TextureOn()) {
    glDisable(GL_TEXTURE_2D);
    // MC - Nov. 2016 - reset to default texture for shadows mode
    glBindTexture(GL_TEXTURE_2D,0);
    _textureV = endtv;
    if (_textureV > 1.0f)
      _textureV -= 1.0f;
  }
}

void CylinderLineScreenTurtle::_CapGC() {

    float bgnmtrx[16];
  
    glPPM ppm;
    glTranslatef(_gc.Position().X(), _gc.Position().Y(), _gc.Position().Z());
    
    const float bgnrot[16] = {_gc.Left().X(),
                             _gc.Left().Y(),
                             _gc.Left().Z(),
                             0.0f,
                             _gc.Up().X(),
                             _gc.Up().Y(),
                             _gc.Up().Z(),
                             0.0f,
                             _gc.Heading().X(),
                             _gc.Heading().Y(),
                             _gc.Heading().Z(),
                             0.0f,
                             0.0f,
                             0.0f,
                             0.0f,
                             1.0f};
    glMultMatrixf(bgnrot);
    
    glScalef(0.5f * _gc.ScaleP(), 0.5f * _gc.ScaleQ(), 1.0f);
    glGetFloatv(GL_MODELVIEW_MATRIX, bgnmtrx);

    Vector3d normal(0.0f, 0.0f, -1.0f);

    const Contour &bgncntr =
      contours.Get1(_gc.ContourId(), _gc.ContourId2(), _gc.Blender());

    // check if a concave cap should be generated
    if (drawparams.ConcavePolygons() == DParams::concaveOn) {
        GLDraw::Polygon polygon;
        glNormal3fv(normal);
        polygon.Start();
        for (size_t i = 0; i < bgncntr.Divisions()-1; ++i) {
          polygon.AddVertex(bgncntr.GetVertex(i),normal,_color);
        }
        polygon.End();
    } else {
        gl.SetColor(_color);
        GLprimitive glp(GL_TRIANGLE_FAN);
        glNormal3fv(normal);
        // compute centroid
        Vector3d centroid(0,0,0);
        for (size_t i = 0; i < bgncntr.Divisions()-1; ++i) {
          centroid += bgncntr.GetVertex(i);
        }
        centroid /= float(bgncntr.Divisions()-1);
        // draw polygon
        glVertex3fv(centroid);
        for (size_t i = 0; i < bgncntr.Divisions()-1; ++i) {
          glVertex3fv(bgncntr.Vertex(i));
          glVertex3fv(bgncntr.Vertex(i+1));
        }
    }

}

void CylinderLineScreenTurtle::StartGC() {
  if (_allowBranchGC) {
    _gc.End();
    _allowBranchGC = false;
  }
  if (!_gc.On()) {
    _gc.Start();
    _gc.Set(_Scale, _position, _up, _left, _heading, _CurrentContour,
            _ContourId2, _blender);
    if (drawparams.CappedCylinders() == DParams::cappedCylindersOn)
        _CapGC();
  } else
    Utils::Message("StartGC: cylinder already started. Module ignored.\n");
}

void CylinderLineScreenTurtle::PointGC() {
  if (_gc.On()) {
    _gc.Set(_Scale, _position, _up, _left, _heading, _CurrentContour,
            _ContourId2, _blender);
  } else
    Utils::Message("PointGC: cylinder not started. Module ignored.\n");
}

void CylinderLineScreenTurtle::EndGC() {
  if (_gc.On()) {
    if (drawparams.CappedCylinders() == DParams::cappedCylindersOn)
      _CapGC();
    _gc.End();
  }
  else
    Utils::Message("EndGC: cylinder not started. Module ignored.\n");
}

void CylinderLineScreenTurtle::BlendContours(int id1, int id2, float b) {
  if (!contours.ValidId(id1)) {
    Utils::Message("BlendContours: Invalid contour id = %d. Module ignored.\n",
                   id1);
    return;
  }
  if (!contours.ValidId(id2)) {
    Utils::Message("BlendContours: Invalid contour id = %d. Module ignored.\n",
                   id2);
    return;
  }
  if (b < 0.0f) {
    Utils::Message("BlendContours: Blending factor %f < 0.0, assumed 0.0\n");
    b = 0.0f;
  } else if (b > 1.0f) {
    Utils::Message("BlendContours: Blending factor %f > 1.0, assumed 1.0\n");
    b = 1.0f;
  }
  _CurrentContour = id1;
  _ContourId2 = id2;
  _blender = b;
  _UpdateContourDivisions(_CurrentContour);
  if (b > 0.0f)
    _UpdateContourDivisions(_ContourId2);
}

void CylinderLineScreenTurtle::SetWidth(float w) { ScaleContour(w, w); }

void CylinderLineScreenTurtle::ScaleContour(float p, float q) {
  Turtle::ScaleContour(p, q);
  _Scale.p = p;
  _Scale.q = q;
}

void CylinderLineScreenTurtle::StartBranch() {
  if (_gc.On() && _allowBranchGC == false) {
    _allowBranchGC = true;
  }
}

