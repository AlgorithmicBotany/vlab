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



#include <iostream>
#include <string>

#include <cstdlib>

#include <qapplication.h>
#include <qlabel.h>
#include <qlayout.h>
#include <QFileDialog>
#include <qbitmap.h>
#include <qimage.h>
#include <qmessagebox.h>
#include <qsizepolicy.h>
#include <QBoxLayout>
#include <QCloseEvent>
#include <QHBoxLayout>
#include <QHideEvent>
#include <QShowEvent>
#include <QVBoxLayout>

#include "config.h"
#include "ctrl.h"
#include "icon.h"

using std::cerr;
using std::endl;
using std::list;
using std::string;

using namespace Qt;

Ctrl::Ctrl(int argc, char **argv)
    : _caption("Bezier Editor"), _savefilename("noname.s"),
      _pModel(new Model()), _defaultName(false) {
  // if we have a filename, load it the file

  if (argc > 1) {
    _pModel->load(argv[1]);
    _savefilename = argv[1];
    setWindowTitle(_caption + QString(": ") + _savefilename);
  } else {
    setWindowTitle(_caption);
    _defaultName = true;
  }

  // load the config options from file
  try {
    Config::readConfig();
   } catch (Config::ConfigErrorExc exc) {
    cerr << exc.message() << endl;
  } catch (...) {
    cerr << "Unknown config error occured" << endl;
  }

  // load the cursors
  _loadCursors();

  QGridLayout *ctrlLyt = new QGridLayout(this);

  // create the widgets

  _pReloadBtn = new QPushButton("Reload");
  ctrlLyt->addWidget(_pReloadBtn, 0, 0);
  _pSaveBtn = new QPushButton("Save");
  ctrlLyt->addWidget(_pSaveBtn, 1, 0);
  _pSaveAsBtn = new QPushButton("Save As");
  ctrlLyt->addWidget(_pSaveAsBtn, 2, 0);
  _pAddPatchBtn = new QPushButton("Add Patch");
  ctrlLyt->addWidget(_pAddPatchBtn, 3, 0);

  _pEditMode = new QGroupBox();
  QBoxLayout *modelayout = new QVBoxLayout();
  modelayout->setMargin(5);

  _pMovePointBtn = new QRadioButton("Move Point");
  modelayout->addWidget(_pMovePointBtn);

  _pMovePatchBtn = new QRadioButton("Move Patch");
  modelayout->addWidget(_pMovePatchBtn);
  _pSplitBtn = new QRadioButton("Split");
  modelayout->addWidget(_pSplitBtn);
  _pMergeBtn = new QRadioButton("Merge");
  modelayout->addWidget(_pMergeBtn);
  _pJoinToBtn = new QRadioButton("Join To ");
  modelayout->addWidget(_pJoinToBtn);
  _pDeleteBtn = new QRadioButton("Delete");
  modelayout->addWidget(_pDeleteBtn);
  _pMovePointBtn->setChecked(true);

  _pEditMode->setLayout(modelayout);
  ctrlLyt->addWidget(_pEditMode, 4, 0);

   _pCMode = new QGroupBox();
  QBoxLayout *cmodelayout = new QVBoxLayout();
  cmodelayout->setMargin(5);

  _pCOffBtn = new QRadioButton("No Continuity");
  cmodelayout->addWidget(_pCOffBtn);
  _pG1OnBtn = new QRadioButton("G1 Continuity");
  cmodelayout->addWidget(_pG1OnBtn);
  _pC1OnBtn = new QRadioButton("C1 Continuity");
  cmodelayout->addWidget(_pC1OnBtn);
  _pCOffBtn->setChecked(true);

  _pCMode->setLayout(cmodelayout);
  ctrlLyt->addWidget(_pCMode, 5, 0);

  _pSnapGrp = new QGroupBox();
  QHBoxLayout *layout = new QHBoxLayout;
  _pXY = new QPushButton("XY", _pSnapGrp);
  _pYZ = new QPushButton("YZ", _pSnapGrp);
  _pZX = new QPushButton("ZX", _pSnapGrp);

  layout->addWidget(_pXY);
  layout->addWidget(_pYZ);
  layout->addWidget(_pZX);
  _pSnapGrp->setLayout(layout);
  ctrlLyt->addWidget(_pSnapGrp, 6, 0);

  _pNamesLbx = new QListWidget();
  for (int i = 0; i < 5; i++)
    _pNamesLbx->addItem(" "); // set five rows for padding
  _pNamesLbx->setSelectionMode(QAbstractItemView::MultiSelection);

  ctrlLyt->addWidget(_pNamesLbx, 7, 0);

  _pHelpBtn = new QPushButton("Help");
  //ctrlLyt->addWidget(_pHelpBtn, 8, 0);

  _pAboutBtn = new QPushButton("About");
  // ctrlLyt->addWidget(_pAboutBtn, 9, 0);

  // connect the widgets
  connect(_pReloadBtn, SIGNAL(clicked()), SLOT(reload()));
  connect(_pSaveBtn, SIGNAL(clicked()), SLOT(save()));
  connect(_pSaveAsBtn, SIGNAL(clicked()), SLOT(saveas()));
  connect(_pAddPatchBtn, SIGNAL(clicked()), SLOT(addPatch()));
  connect(_pMovePointBtn, SIGNAL(clicked()), SLOT(setMovePoint()));
  connect(_pMovePatchBtn, SIGNAL(clicked()), SLOT(setMovePatch()));
  connect(_pSplitBtn, SIGNAL(clicked()), SLOT(setSplit()));
  connect(_pMergeBtn, SIGNAL(clicked()), SLOT(setMerge()));
  connect(_pJoinToBtn, SIGNAL(clicked()), SLOT(setJoinTo()));
  connect(_pDeleteBtn, SIGNAL(clicked()), SLOT(setDelete()));
  connect(_pCOffBtn, SIGNAL(clicked()), SLOT(setCOff()));
  connect(_pC1OnBtn, SIGNAL(clicked()), SLOT(setC1On()));
  connect(_pG1OnBtn, SIGNAL(clicked()), SLOT(setG1On()));
  connect(_pXY, SIGNAL(clicked()), SLOT(snapXY()));
  connect(_pYZ, SIGNAL(clicked()), SLOT(snapYZ()));
  connect(_pZX, SIGNAL(clicked()), SLOT(snapZX()));
  connect(_pNamesLbx, SIGNAL(itemClicked(QListWidgetItem *)),
          SLOT(updateHighlights()));
  connect(_pHelpBtn, SIGNAL(clicked()), SLOT(help()));
  connect(_pAboutBtn, SIGNAL(clicked()), SLOT(about()));

  // create the view windows
  _pPerspView = new PerspView(this, _pModel);
  _pPerspView->resize(QSize(520, 520));
  _pPerspView->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,
                                         QSizePolicy::MinimumExpanding));
  ctrlLyt->addWidget(_pPerspView, 0, 1, 10, 1);

 
  // set the default view options
  _pPerspView->GRID->setChecked(true);
  _pPerspView->AXES->setChecked(true);
  _pPerspView->WIRE_MODEL->setChecked(true);
  _pPerspView->CONNECTION_POINT->setChecked(true);
  _pPerspView->CONTROL_POINTS->setChecked(true);
  _pPerspView->CONTROL_POLY->setChecked(true);

  _pPerspView->useOrtho(true);
  setWindowIcon(QPixmap());
 
  // set the persp view to be square at minimum
  _pPerspView->setMinimumWidth(_pPerspView->height());

  _updatePatchNames();
  _pPerspView->firstResize = true;
  _pPerspView->show();
  updateViews();
}

Ctrl::~Ctrl() { delete _pModel; }

void Ctrl::hideEvent(QHideEvent *) {
  if (isMinimized()) {
    if (!_pPerspView->isMinimized())
      _pPerspView->showMinimized();
  }
}

void Ctrl::showEvent(QShowEvent *) {
  if (isVisible()) {
    if (_pPerspView->isMinimized())
      _pPerspView->showNormal();
  }
}

void Ctrl::setMovePoint() {
  _pModel->showSplitablePoints(false);
  _pMovePointBtn->setChecked(true);

  _pPerspView->movePointMode();
  _pPerspView->setCursor(ArrowCursor);
  updateViews();
}

void Ctrl::setMovePatch() {
  _pModel->showSplitablePoints(false);
  _pMovePatchBtn->setChecked(true);

  _pPerspView->movePatchMode();
  _pPerspView->setCursor(movepatchCsr);
  updateViews();
}

void Ctrl::setSplit() {
  _pModel->showSplitablePoints(true);
  _pSplitBtn->setChecked(true);

  _pPerspView->splitMode();
  _pPerspView->setCursor(splitCsr);
  updateViews();
}

void Ctrl::setMerge() {
  _pModel->showSplitablePoints(false);
  _pMergeBtn->setChecked(true);
  _pModel->useInterpolateMerge(true);

  _pPerspView->mergeMode();
  _pPerspView->setCursor(mergeCsr);
  updateViews();
}

void Ctrl::setJoinTo() {
  _pModel->showSplitablePoints(false);
  _pJoinToBtn->setChecked(true);
  _pModel->useInterpolateMerge(false);

  _pPerspView->mergeMode();
  _pPerspView->setCursor(mergeCsr);
  updateViews();
}

void Ctrl::setDelete() {
  _pModel->showSplitablePoints(false);
  _pDeleteBtn->setChecked(true);

  _pPerspView->deleteMode();
  _pPerspView->setCursor(deleteCsr);
  updateViews();
}

void Ctrl::setCOff() {
  _pModel->showSplitablePoints(false);
  _pCOffBtn->setChecked(true);
  _pModel->setContinuity(Model::NONE);
}

void Ctrl::setG1On() {
  _pG1OnBtn->setChecked(true);
  _pModel->setContinuity(Model::G1);
}

void Ctrl::setC1On() {
  _pC1OnBtn->setChecked(true);
  _pModel->setContinuity(Model::C1);
}

void Ctrl::snapXY() {
  _pPerspView->snapToXY();
  _pPerspView->update();
}

void Ctrl::snapYZ() {
  _pPerspView->snapToYZ();
  _pPerspView->update();
}

void Ctrl::snapZX() {
  _pPerspView->snapToZX();
  _pPerspView->update();
}

bool Ctrl::quitCB() {
  int ret = QMessageBox::No;

  if (_pModel->hasChanged()) {
    ret = QMessageBox::warning(this, "Save At Exit",
                               "Save changes before exit?", QMessageBox::Yes,
                               QMessageBox::No, QMessageBox::Cancel);
  }

  switch (ret) {
  case QMessageBox::Yes:
    try {
      _pModel->save(_savefilename.toStdString().c_str());
      _pPerspView->close();
    } catch (Model::FileExc) {
      QMessageBox::critical(this, "Error",
                            QString("Could not save to the file ") +
                                _savefilename);
    }
    return true;
  case QMessageBox::No:
    return true;
    break;
  default:
    return false;
  }
}

void Ctrl::closeEvent(QCloseEvent*) {
}

void Ctrl::updateViews() { _pPerspView->update(); }

void Ctrl::reload() {
  _pModel->load(_savefilename.toStdString().c_str());
  _defaultName = false;
  _pPerspView->firstResize = true;
  setCOff();
  _updatePatchNames();
  updateViews();
}

void Ctrl::save() {
  if (_defaultName)
    saveas();
  else {
    try {
      _pModel->save(_savefilename.toStdString().c_str());
    } catch (Model::FileExc) {
      QMessageBox::critical(this, "Error",
                            QString("Could not save to the file ") +
                                _savefilename);
    }
    updateViews();
  }
}

void Ctrl::saveas() {
  QString filename = QFileDialog::getSaveFileName(this, QString("."), "*.s");
  if (!filename.isEmpty()) {
    _defaultName = false;
    _savefilename = filename;
    save();
  }
}

void Ctrl::addPatch() {
  _pModel->createPatch();
  _updatePatchNames();
  updateViews();
}

void Ctrl::updateHighlights() {
  for (int i = 0; i < _pNamesLbx->count(); i++){
    _pModel->highlightPatch(i, (_pNamesLbx->item(i))->isSelected());
  }
  updateViews();
}


void Ctrl::help() {
  char msg[] = "Mouse Commands:\n"
               "  Rotate View - Shift + Left Button\n"
               "  Zoom View - Ctrl + Left Button \n"
               "  Pan View - Alt + Left Button \n"
               "\n"
                "Sticky Point:\n"
               "  Set/Unset sticky point - double click on point.\n";

  QMessageBox *mb =
      new QMessageBox("Help", QString(msg), QMessageBox::Information, 1, 0, 0);
  QPixmap icon(":icon.png");
  mb->setIconPixmap(icon.scaled(icon.width() / 2, icon.height() / 2,
                                Qt::IgnoreAspectRatio,
                                Qt::SmoothTransformation));
  mb->setButtonText(1, "OK");
  mb->exec();

  delete mb;
}

void Ctrl::about() {
  char msg[] = "Bezieredit\n"
               "\n"
               "by Colin Smith\n"
               "August 2001\n"
               "\n"
               "Dept. of Computer Science\n"
               "University of Calgary";

  QMessageBox *mb =
      new QMessageBox("About", QString(msg), QMessageBox::Information, 1, 0, 0);
  QPixmap icon(":icon.png");
  mb->setIconPixmap(icon.scaled(icon.width() / 2, icon.height() / 2,
                                Qt::IgnoreAspectRatio,
                                Qt::SmoothTransformation));

  mb->setButtonText(1, "OK");
  mb->exec();

  delete mb;
}

void Ctrl::_loadCursors() {
  QImage splitimg(split_png_width, split_png_height, QImage::Format_RGB16);
  for (int y = 0; y < split_png_width; y++) {
    for (int x = 0; x < split_png_height; x++) {
      QRgb rgb = qRgb(split_png[(y * split_png_width + x) * 4 + 0],
                      split_png[(y * split_png_width + x) * 4 + 1],
                      split_png[(y * split_png_width + x) * 4 + 2]);

      splitimg.setPixel(y, x, rgb);
    }
  }

  QImage splitimgmsk(split_png_width, split_png_height, QImage::Format_RGB16);
  for (int y = 0; y < split_png_width; y++) {
    for (int x = 0; x < split_png_height; x++) {
      QRgb rgb = qRgb(split_png_alpha[(y * split_png_width + x) * 4],
                      split_png_alpha[(y * split_png_width + x) * 4],
                      split_png_alpha[(y * split_png_width + x) * 4]);

      splitimgmsk.setPixel(y, x, rgb);
    }
  }

  QBitmap splitpx;
  splitpx.convertFromImage(splitimg);
  QBitmap splitmsk;
  splitmsk.convertFromImage(splitimgmsk);
  splitCsr = QCursor();

  QImage mergeimg(merge_png_width, merge_png_height, QImage::Format_RGB16);
  for (int y = 0; y < merge_png_width; y++) {
    for (int x = 0; x < merge_png_height; x++) {
      QRgb rgb = qRgb(merge_png[(y * merge_png_width + x) * 4 + 0],
                      merge_png[(y * merge_png_width + x) * 4 + 1],
                      merge_png[(y * merge_png_width + x) * 4 + 2]);

      mergeimg.setPixel(y, x, rgb);
    }
  }

  QImage mergeimgmsk(merge_png_width, merge_png_height, QImage::Format_RGB16);
  for (int y = 0; y < merge_png_width; y++) {
    for (int x = 0; x < merge_png_height; x++) {
      QRgb rgb = qRgb(merge_png_alpha[(y * merge_png_width + x) * 4],
                      merge_png_alpha[(y * merge_png_width + x) * 4],
                      merge_png_alpha[(y * merge_png_width + x) * 4]);

      mergeimgmsk.setPixel(y, x, rgb);
    }
  }

  QBitmap mergepx;
  mergepx.convertFromImage(mergeimg);
  QBitmap mergemsk;
  mergemsk.convertFromImage(mergeimgmsk);
  mergeCsr = QCursor();

  QImage deleteimg(delete_png_width, delete_png_height, QImage::Format_RGB16);
  for (int y = 0; y < delete_png_width; y++) {
    for (int x = 0; x < delete_png_height; x++) {
      QRgb rgb = qRgb(delete_png[(y * delete_png_width + x) * 4 + 0],
                      delete_png[(y * delete_png_width + x) * 4 + 1],
                      delete_png[(y * delete_png_width + x) * 4 + 2]);

      deleteimg.setPixel(y, x, rgb);
    }
  }

  QImage deleteimgmsk(delete_png_width, delete_png_height,
                      QImage::Format_RGB16);
  for (int y = 0; y < delete_png_width; y++) {
    for (int x = 0; x < delete_png_height; x++) {
      QRgb rgb = qRgb(delete_png_alpha[(y * delete_png_width + x) * 4],
                      delete_png_alpha[(y * delete_png_width + x) * 4],
                      delete_png_alpha[(y * delete_png_width + x) * 4]);

      deleteimgmsk.setPixel(y, x, rgb);
    }
  }

  QBitmap deletepx;
  deletepx.convertFromImage(deleteimg);
  QBitmap deletemsk;
  deletemsk.convertFromImage(deleteimgmsk);
  deleteCsr = QCursor();
  QImage movepatchimg(movepatch_png_width, movepatch_png_height,
                      QImage::Format_RGB16);
  for (int y = 0; y < movepatch_png_width; y++) {
    for (int x = 0; x < movepatch_png_height; x++) {
      QRgb rgb = qRgb(movepatch_png[(y * movepatch_png_width + x) * 4 + 0],
                      movepatch_png[(y * movepatch_png_width + x) * 4 + 1],
                      movepatch_png[(y * movepatch_png_width + x) * 4 + 2]);

      movepatchimg.setPixel(y, x, rgb);
    }
  }

  QImage movepatchimgmsk(movepatch_png_width, movepatch_png_height,
                         QImage::Format_RGB16);
  for (int y = 0; y < movepatch_png_width; y++) {
    for (int x = 0; x < movepatch_png_height; x++) {
      QRgb rgb = qRgb(movepatch_png_alpha[(y * movepatch_png_width + x) * 4],
                      movepatch_png_alpha[(y * movepatch_png_width + x) * 4],
                      movepatch_png_alpha[(y * movepatch_png_width + x) * 4]);

      movepatchimgmsk.setPixel(y, x, rgb);
    }
  }

  QBitmap movepatchpx;
  movepatchpx.convertFromImage(movepatchimg);
  QBitmap movepatchmsk;
  movepatchmsk.convertFromImage(movepatchimgmsk);
  movepatchCsr = QCursor();
}

void Ctrl::_updatePatchNames() {
  _pNamesLbx->clear();

  _patchNames = _pModel->getPatchNames();
  for (list<string>::iterator i = _patchNames.begin(); i != _patchNames.end();
       i++){
    QListWidgetItem *item = new QListWidgetItem((*i).c_str());
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    _pNamesLbx->addItem(item);
    connect(_pNamesLbx, SIGNAL(itemChanged(QListWidgetItem *)), SLOT(updatePatchName(QListWidgetItem *)));

  }
}

void Ctrl::updatePatchName(QListWidgetItem *item){
  int index = _pNamesLbx->row(item);
  std::string text = item->text().toStdString();
  _pModel->updateName(index,text);
}
