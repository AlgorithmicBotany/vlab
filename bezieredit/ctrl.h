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

#include <list>

#include <qwidget.h>
#include <qstring.h>
#include <qpushbutton.h>
#include <QButtonGroup>
#include <QGroupBox>
#include <qradiobutton.h>
#include <qcursor.h>
#include <QListWidget>
#include <QShowEvent>
#include <QHideEvent>
#include <QCloseEvent>
#include <limits.h>

#include "model.h"
#include "perspview.h"

class Ctrl : public QWidget {
  Q_OBJECT

 public:
  Ctrl(int argc, char** argv);
  ~Ctrl();

  void closeEvent(QCloseEvent*);
  void updateViews();
  void updateNames() {_updatePatchNames();}

   bool quitCB();


 protected:
  void hideEvent(QHideEvent*);
  void showEvent(QShowEvent*);

 public slots:
  void reload();
  void save();
  void saveas();
  void addPatch();

 private slots:
  void setMovePoint();
  void setMovePatch();
  void setSplit();
  void setMerge();
  void setJoinTo();
  void setDelete();
  void setCOff();
  void setG1On();
  void setC1On();
  void snapXY();
  void snapYZ();
  void snapZX();
  void updateHighlights();
  void help();
  void about();
  void updatePatchName(QListWidgetItem*);

 private:
  void _loadCursors();
  void _updatePatchNames();

  QString _caption;
  QString _savefilename;

  QPushButton* _pReloadBtn;
  QPushButton* _pSaveBtn;
  QPushButton* _pSaveAsBtn;
  QPushButton* _pAddPatchBtn;

  QGroupBox* _pEditMode;
  QRadioButton* _pMovePointBtn;
  QRadioButton* _pMovePatchBtn;
  QRadioButton* _pSplitBtn;
  QRadioButton* _pMergeBtn;
  QRadioButton* _pJoinToBtn;
  QRadioButton* _pDeleteBtn;

  QGroupBox* _pCMode;
  QRadioButton* _pCOffBtn;
  QRadioButton* _pG1OnBtn;
  QRadioButton* _pC1OnBtn;

  QGroupBox*    _pSnapGrp;
  QPushButton*  _pXY;
  QPushButton*  _pYZ;
  QPushButton*  _pZX;

  QListWidget* _pNamesLbx;

  QPushButton* _pHelpBtn;
  QPushButton* _pAboutBtn;

  QCursor mergeCsr;
  QCursor splitCsr;
  QCursor deleteCsr;
  QCursor movepatchCsr;

  Model*     _pModel;
  PerspView* _pPerspView;

  static const int WBUF = 25;
  static const int HBUF = 50;

  std::list<std::string> _patchNames;

  bool _defaultName;
};

#endif
