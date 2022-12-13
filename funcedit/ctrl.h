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




#ifndef __CTRL_H__
#define __CTRL_H__

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLFunctions_4_3_Core>

#include <QOpenGLBuffer>

#include <QString>
#include <QMouseEvent>
#include <QCloseEvent>
#include <QAbstractButton>
#include <limits.h>

#include "geometry.h"
#include "gridview.h"
#include "model.h"
#include "functask.h"
#include "namedlg.h"
#include "sampleDlg.h"
#include "glutils.h"

class QWheelEvent;
class QMenu;


class Ctrl : public QOpenGLWidget, public GridView, protected QOpenGLFunctions {
  Q_OBJECT

enum SavingMode {CONTINUOUS,TRIGGERED, OFF};

   
 public:
  Ctrl(QWidget *parent, int argc, char** argv);
  ~Ctrl();

  void Load(const char*);
  int  FindClosest(WorldPoint) const;
  void AddPoint(WorldPoint);

  void DeletePoint(int);
  void MovePoint(int, WorldPoint);
  void IncPointMultiplicity(int);


 protected:
  void closeEvent(QCloseEvent* pEv);

  // following is used to render text
  inline GLint project(GLdouble objx, GLdouble objy, GLdouble objz,
		       const GLdouble model[16], const GLdouble proj[16],
		       const GLint viewport[4], GLdouble * winx, GLdouble * winy, GLdouble * winz);
  inline void transformPoint(GLdouble out[4], const GLdouble m[16], const GLdouble in[4]);
  void renderText(double x, double y, double z, const QString &str, const QColor color, const QFont & font = QFont());

  virtual void rtext(double x, double y, char* bf);
  virtual void rtext(double x, double y, const char *bf);

  void initializeGL();
  void resizeGL(int, int);

  void paintGL();
  void setupViewport(int width, int height);

  void mousePressEvent(QMouseEvent*);
  void mouseReleaseEvent(QMouseEvent*);
  void mouseMoveEvent(QMouseEvent*);
  void mouseDoubleClickEvent(QMouseEvent *pEv);

  void wheelEvent(QWheelEvent*);
  void keyPressEvent(QKeyEvent*);
  void keyReleaseEvent(QKeyEvent*);


 public slots:
  void FuncName();
  void SetSamples();
  void Reload();
  void Save();
  void SaveAs();
  void LoadBackGrdImage();

  void SaveInContinuousMode();
  void FlipView(bool checked);
  void ViewPoints(bool checked);
  void ViewSegments(bool checked);
  void ViewCurve(bool checked);
  void ViewLimits(bool checked);
  void ReloadColors();
  void Help();
  void About();
  void msgBoxClosed(QAbstractButton*);

  
  void ContinuousSavingMode();
  void TriggeredSavingMode();
  void ModeOff();

 public:
  QString     _caption;     // caption for the main window
  QMenu* _contextmenu; // The context menu
  bool hasMenuBar;
  QAction *flip_act;

 
  Model _FuncModel;
  char _fname[PATH_MAX+1];

  AddRemoveTask _AddRemoveTask;
  DragPointTask _DragPointTask;
  IncMultiplicityTask _IncMultiplicityTask;

  bool _default;


  NameDlg* pNDlg;
  SampleDlg *sampleDlg;


 private:
  SavingMode _savingMode;
  QAction *_savingContinu_act;
  QAction *_savingTriggered_act;
  QAction *_savingMenu_act;

  QAction *loadBackGrdImage_act;
  
  GLfilledCircle _circle;
  GLellipse _ellipse;

  QPixmap _backGrdImage;
  void _LoadBackGrdTexture(const QString &filename);
  void _DrawBackGrdTexture(void);

};

#else
  #error File already included
#endif
