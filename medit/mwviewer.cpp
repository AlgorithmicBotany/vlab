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



/* Material World

   Implementation of Class: MWViewer

  */

#include <qcursor.h>
#include "mwviewer.h"
#include <QApplication>
#include <QMenu>
#include <QMouseEvent>
using namespace Qt;

// --------------------- Construction --------------------
MWViewer::MWViewer(QWidget *parent, const char *,Qt::WindowFlags f) : QOpenGLWidget(parent,f) {

  setMinimumWidth(269);
  setMinimumHeight(269);
  _savingMode = OFF;

  selectindex = 0;
  selectoffset = 1;
  pageindex = 0;
  pagenumber = 1;
  numpages = 16;
  selecting = false;
  selectable = true;
  enhance = false;
  pagesmooth = true;

  range = 1000;
  near = -3000;
  far = 2000;
  bgplane = -500;

  pg = 16;
  row = 4;
  base = 8;
  smoothness = 10;
  divisions = base + smoothness;
  setdim();

  // user should be able to edit these values in "preferences" file
  lightpos[0] = 1000.0; // x
  lightpos[1] = 1000.0; // y
  lightpos[2] = 2000.0; // z
  lightpos[3] = 0.0;    // directional

  lightmodel[0] = 1.0; // white ambient light
  lightmodel[1] = 1.0;
  lightmodel[2] = 1.0;
  lightmodel[3] = 1.0;

  // default is openGL default
  defmat.ambvec[0] = 0.2;
  defmat.ambvec[1] = 0.2;
  defmat.ambvec[2] = 0.2;
  defmat.ambvec[3] = 1.0;
  defmat.difvec[0] = 0.8;
  defmat.difvec[1] = 0.8;
  defmat.difvec[2] = 0.8;
  defmat.difvec[3] = 1.0;
  defmat.emivec[0] = 0.0;
  defmat.emivec[1] = 0.0;
  defmat.emivec[2] = 0.0;
  defmat.emivec[3] = 1.0;
  defmat.spevec[0] = 0.0;
  defmat.spevec[1] = 0.0;
  defmat.spevec[2] = 0.0;
  defmat.spevec[3] = 1.0;
  defmat.shiny = 0;
  defmat.trans = 0.0;
  defmat.isD = true;

  // initialize the copy material to default

  // for background
  background = "checkerboard";
   imageON = false;
  seed = time(0);

  menu = new QMenu(this);
  // a little editing menu
  _sv = menu->addAction("&Save", this, SLOT(save()), CTRL + Key_S);
  menu->addAction("Save As...", this, SLOT(saveas()));
  _rs = menu->addAction("Revert to Saved", this, SLOT(revertsaved()));
  menu->insertSeparator(_rs);
  QAction *act = 0;
  act = menu->addAction("M-Edit", this, SLOT(Medit()));
  menu->insertSeparator(act);
  menu->addAction("Cut", this, SLOT(cut()));
  menu->addAction("Copy", this, SLOT(copy()));

  _ps = menu->addAction("Paste", this, SLOT(paste()));
  _ps->setEnabled(false);
  menu->addAction("Insert", this, SLOT(insert()));
  menu->addAction("Interpolate", this, SLOT(interpolate()));
  act = menu->addAction("Set to Default", this, SLOT(defaultmat()));
  menu->insertSeparator(act);

  QMenu *modeMenu = menu->addMenu("Refresh mode");

  _savingMenu_act = modeMenu->addAction("Explicit", this, SLOT(ModeOff()));
  _savingMenu_act->setCheckable(true);
  if (_savingMode == OFF)
    _savingMenu_act->setChecked(true);

  _savingTriggered_act =
      modeMenu->addAction("Triggered", this, SLOT(TriggeredSavingMode()));
  _savingTriggered_act->setCheckable(true);
  if (_savingMode == TRIGGERED)
    _savingTriggered_act->setChecked(true);

  _savingContinu_act =
      modeMenu->addAction("Continuous", this, SLOT(ContinuousSavingMode()));
  _savingContinu_act->setCheckable(true);
  if (_savingMode == CONTINUOUS)
    _savingContinu_act->setChecked(true);

  menu->addSeparator();

  menu->addAction("Exit", parent, SLOT(close()), QKeySequence("Ctrl+Q"));

  emit modify(false);
  // Connections
  QObject::connect(this, SIGNAL(modify(bool)), this,
                   SLOT(saveInContinuouMode(bool)));
  QObject::connect(this, SIGNAL(changeSavingMode(SavingMode)), parent,
                   SLOT(setSavingMode(SavingMode)));

  QObject::connect(parent, SIGNAL(triggered()), this,
                   SLOT(saveInTriggeredMode()));
}

void MWViewer::saveInTriggeredMode() {
  if (_savingMode == TRIGGERED) {
    save();
  }
}

void MWViewer::saveInContinuouMode(bool modified) {
  if (!modified)
    return;
  if (_savingMode == CONTINUOUS) {
    save();
  }
}

void MWViewer::setSavingMode(SavingMode savingMode) {
  _savingMode = savingMode;
  _savingMenu_act->setChecked(false);
  _savingTriggered_act->setChecked(false);
  _savingContinu_act->setChecked(false);

  if (_savingMode == OFF)
    _savingMenu_act->setChecked(true);
  else if (_savingMode == TRIGGERED)
    _savingTriggered_act->setChecked(true);
  else if (_savingMode == CONTINUOUS)
    _savingContinu_act->setChecked(true);
  emit changeSavingMode(_savingMode);
}

void MWViewer::ContinuousSavingMode() {
  _savingMode = CONTINUOUS;
  _savingContinu_act->setChecked(true);
  _savingTriggered_act->setChecked(false);
  _savingMenu_act->setChecked(false);
  emit changeSavingMode(_savingMode);
}

void MWViewer::TriggeredSavingMode() {
  _savingMode = TRIGGERED;
  _savingContinu_act->setChecked(false);
  _savingTriggered_act->setChecked(true);
  _savingMenu_act->setChecked(false);
  emit changeSavingMode(_savingMode);
}

void MWViewer::ModeOff() {
  _savingMode = OFF;
  _savingContinu_act->setChecked(false);
  _savingTriggered_act->setChecked(false);
  _savingMenu_act->setChecked(true);
  emit changeSavingMode(_savingMode);
}

// for the parent MW to get selected
void MWViewer::getselect(int *I, Material *M) {
  *I = selectindex;
  matcpy(M, materials[selectindex]);
}

// for the parent MW to turn select off
void MWViewer::selectOff(bool s) {
  selectable = !s;
  selectable = true;
}

void MWViewer::setSmoothness(int S) {
  smoothness = S;
  divisions = base + smoothness;
  setdim();
  update();
}

void MWViewer::setEnhance(bool E) {
  enhance = E;
  update();
}

void MWViewer::setPageSmooth(bool P) {
  pagesmooth = P;
  if (!pagesmooth)
    base = 4;
  else {
    switch (pg) {
    case 256:
      base = 4;
      break;
    case 64:
      base = 6;
      break;
    case 16:
      base = 8;
      break;
    case 1:
      base = 16;
      break;
    }
  }
  divisions = base + smoothness;
  update();
}

// --------------------- Slots --------------------
// File I/O
void MWViewer::read(QFile *f) {
  // init to default
  for (int i = 0; i < 256; i++) {
    matcpy(&materials[i], defmat);
  }

  // this should parse the input file and ensure that there are no errors
  // if error: emit error("error message here",code#);
  if (f) {
    // read in materials from file
    char *mat = new char[15];
    int id;
    GLfloat trans;

    f->open(QIODevice::Unbuffered | QIODevice::ReadOnly);

    while (!f->atEnd()) {
      f->read(mat, 15);
      id = (int)(unsigned char)mat[0];
      trans = (GLfloat)((unsigned char)mat[1]) / (GLfloat)255;
      materials[id].ambvec[0] = (GLfloat)((unsigned char)mat[2]) / (GLfloat)255;
      materials[id].ambvec[1] = (GLfloat)((unsigned char)mat[3]) / (GLfloat)255;
      materials[id].ambvec[2] = (GLfloat)((unsigned char)mat[4]) / (GLfloat)255;
      materials[id].ambvec[3] = 1 - trans;
      materials[id].difvec[0] = (GLfloat)((unsigned char)mat[5]) / (GLfloat)255;
      materials[id].difvec[1] = (GLfloat)((unsigned char)mat[6]) / (GLfloat)255;
      materials[id].difvec[2] = (GLfloat)((unsigned char)mat[7]) / (GLfloat)255;
      materials[id].difvec[3] = 1 - trans;
      materials[id].emivec[0] = (GLfloat)((unsigned char)mat[8]) / (GLfloat)255;
      materials[id].emivec[1] = (GLfloat)((unsigned char)mat[9]) / (GLfloat)255;
      materials[id].emivec[2] =
          (GLfloat)((unsigned char)mat[10]) / (GLfloat)255;
      materials[id].emivec[3] = 1 - trans;
      materials[id].spevec[0] =
          (GLfloat)((unsigned char)mat[11]) / (GLfloat)255;
      materials[id].spevec[1] =
          (GLfloat)((unsigned char)mat[12]) / (GLfloat)255;
      materials[id].spevec[2] =
          (GLfloat)((unsigned char)mat[13]) / (GLfloat)255;
      materials[id].spevec[3] = 1 - trans;
      materials[id].shiny = (GLfloat)((unsigned char)mat[14]);
      materials[id].trans = trans;
      materials[id].isD = false;
    }
    f->close();
    emit notice("Read OK");
  }
  selectrange(0, 1);
  emit modify(false);
  emit setdefault(defmat); // for editor

  emit setselect(selectindex, materials[selectindex]);
  //  update();
}

void MWViewer::write(QFile *f) {
  char *mat = new char[15];
  f->open(QIODevice::Unbuffered | QIODevice::WriteOnly | QIODevice::Truncate);
  for (int i = 0; i < 256; i++) {
    if (!materials[i].isD) {
      mat[0] = (char)(i);
      mat[1] = (char)(materials[i].trans * 255);
      mat[2] = (char)(materials[i].ambvec[0] * 255);
      mat[3] = (char)(materials[i].ambvec[1] * 255);
      mat[4] = (char)(materials[i].ambvec[2] * 255);
      mat[5] = (char)(materials[i].difvec[0] * 255);
      mat[6] = (char)(materials[i].difvec[1] * 255);
      mat[7] = (char)(materials[i].difvec[2] * 255);
      mat[8] = (char)(materials[i].emivec[0] * 255);
      mat[9] = (char)(materials[i].emivec[1] * 255);
      mat[10] = (char)(materials[i].emivec[2] * 255);
      mat[11] = (char)(materials[i].spevec[0] * 255);
      mat[12] = (char)(materials[i].spevec[1] * 255);
      mat[13] = (char)(materials[i].spevec[2] * 255);
      mat[14] = (char)materials[i].shiny;
      f->write(mat, 15);
    }
  }
  f->close();
  emit modify(false);
  emit notice("Write OK");
}

void MWViewer::newfile() { emit newfile_signal(); }

void MWViewer::newWindow() { emit newWindow_signal(); }

void MWViewer::load() { emit load_signal(); }

void MWViewer::loadWindow() { emit load_signal(); }

void MWViewer::save() { emit save_signal(); }

void MWViewer::saveas() { emit saveas_signal(); }

void MWViewer::revertsaved() { emit revertsaved_signal(); }

void MWViewer::setmodified_save(bool m) { _sv->setEnabled(m); }

void MWViewer::setmodified_save_as(bool m) { _rs->setEnabled(m); }

// Selecting
// opens dialog to select range
void MWViewer::rangeDialog() { emit getRange(selectindex, selectoffset); }

void MWViewer::selectpage() {
  selectindex = pageindex;
  selectoffset = pg;

  emit setselect(selectindex, materials[selectindex]);
  emit notice("Select " + QString::number(selectindex) + " to " +
              QString::number(selectindex + selectoffset - 1));
  update();
}

void MWViewer::selectall() {
  selectindex = 0;
  selectoffset = 256;

  emit setselect(selectindex, materials[selectindex]);
  emit notice("Select " + QString::number(selectindex) + " to " +
              QString::number(selectindex + selectoffset - 1));
  update();
}

void MWViewer::selectrange(int start, int offset) {
  bool change = false;
  // flip to correct page
  while (start < pageindex) {
    pageindex -= pg;
    pagenumber--;
    change = true;
  }
  while (start >= pageindex + pg) {
    pageindex += pg;
    pagenumber++;
    change = true;
  }
  if (change)
    emit pageflip(pageindex, pg);

  selectindex = start;
  selectoffset = offset;

  emit setselect(selectindex, materials[selectindex]);
  if (selectoffset > 1)
    emit notice("Select " + QString::number(selectindex) + " to " +
                QString::number(selectindex + selectoffset - 1));
  update();
}

void MWViewer::select(int index) {
  bool change = false;

  if ((selectindex != index)) {
    // flip to correct page
    while (index < pageindex) {
      pageindex -= pg;
      pagenumber--;
      change = true;
    }
    while (index >= pageindex + pg) {
      pageindex += pg;
      pagenumber++;
      change = true;
    }
    if (change)
      emit pageflip(pageindex, pg);

    // do what we must
    selectindex = index;
    if ((index + selectoffset) > 256)
      selectoffset = 256 - index;
    if (selectoffset > 1)
      emit notice("Select " + QString::number(selectindex) + " to " +
                  QString::number(selectindex + selectoffset - 1));
    emit setselect(selectindex, materials[selectindex]);
    update();
  }
}

// Actions on selected indices
void MWViewer::Medit() {
  emit edit(); // editor already knows selected material
}

void MWViewer::cut() {

  setCopied(selectoffset, selectindex);

  // shuffle tail end up
  int j = 0;
  for (int i = selectindex + selectoffset; i < 256; i++) {
    matcpy(&materials[selectindex + j], materials[i]);
    j++;
  }

  // fill in end with default
  for (int i = selectindex + j; i < 256; i++) {
    matcpy(&materials[i], defmat);
  }

  if (selectoffset == 1)
    emit notice("Cut " + QString::number(selectindex));
  else
    emit notice("Cut " + QString::number(selectindex) + " to " +
                QString::number(selectindex + selectoffset - 1));
  emit modify(true);
  update();
}

void MWViewer::insert() {
  int B;
  Material Mbuffer[256];
  B = getCopied(Mbuffer); // get copied into a material buffer

  // only put in as much as we can...
  if ((selectindex + B) > 255)
    B = 255 - selectindex + 1;

  // make room for new materials by shuffling
  // (some materials will fall off the end)
  int j = 0;
  for (int i = 255 - B; i >= selectindex; i--) {
    matcpy(&materials[255 - j], materials[i]);
    j++;
  }

  // insert the new materials
  for (int i = 0; i < B; i++) {
    matcpy(&materials[selectindex + i], Mbuffer[i]);
  }

  if (B == 1)
    emit notice("Insert " + QString::number(selectindex));
  else
    emit notice("Insert " + QString::number(selectindex) + " to " +
                QString::number(selectindex + B - 1));
  emit modify(true);
  update();
}

void MWViewer::copy() {

  setCopied(selectoffset, selectindex);

  if (selectoffset == 1)
    emit notice("Copy " + QString::number(selectindex));
  else
    emit notice("Copy " + QString::number(selectindex) + " to " +
                QString::number(selectindex + selectoffset - 1));
  //  emit modify(true);
}

// paste 1 copied material over entire selection
// paste more copied materials, beginning at first selected
void MWViewer::paste() {
  int B;
  Material Mbuffer[256];
  B = getCopied(Mbuffer); // get copied into a material buffer

  // only put in as much as we can...
  // (this is not so user-friendly, but I am assuming the user knows what she/he
  // is doing)
  if ((selectindex + B) > 255)
    B = 255 - selectindex + 1;

  if ((B == 1) && (selectoffset > 1)) {
    for (int i = 0; i < selectoffset; i++) {
      matcpy(&materials[selectindex + i], Mbuffer[0]);
    }
  } else {
    for (int i = 0; i < B; i++) {
      matcpy(&materials[selectindex + i], Mbuffer[i]);
    }
  }
  if (B == 1)
    emit notice("Paste " + QString::number(selectindex));
  else
    emit notice("Paste " + QString::number(selectindex) + " to " +
                QString::number(selectindex + B - 1));

  emit setselect(selectindex, materials[selectindex]);
  emit modify(true);
  update();
}

void MWViewer::interpolate() {
  GLfloat scale[18]; // array of the scale values
  int interindex = selectindex + selectoffset - 1;

  if (selectoffset < 3)
    return; // need at least 3 to do interpolate

  // calculate the scale
  scale[0] =
      (materials[interindex].ambvec[0] - materials[selectindex].ambvec[0]) /
      (GLfloat)selectoffset;
  scale[1] =
      (materials[interindex].ambvec[1] - materials[selectindex].ambvec[1]) /
      (GLfloat)selectoffset;
  scale[2] =
      (materials[interindex].ambvec[2] - materials[selectindex].ambvec[2]) /
      (GLfloat)selectoffset;
  scale[3] =
      (materials[interindex].ambvec[3] - materials[selectindex].ambvec[3]) /
      (GLfloat)selectoffset;
  scale[4] =
      (materials[interindex].difvec[0] - materials[selectindex].difvec[0]) /
      (GLfloat)selectoffset;
  scale[5] =
      (materials[interindex].difvec[1] - materials[selectindex].difvec[1]) /
      (GLfloat)selectoffset;
  scale[6] =
      (materials[interindex].difvec[2] - materials[selectindex].difvec[2]) /
      (GLfloat)selectoffset;
  scale[7] =
      (materials[interindex].difvec[3] - materials[selectindex].difvec[3]) /
      (GLfloat)selectoffset;
  scale[8] =
      (materials[interindex].spevec[0] - materials[selectindex].spevec[0]) /
      (GLfloat)selectoffset;
  scale[9] =
      (materials[interindex].spevec[1] - materials[selectindex].spevec[1]) /
      (GLfloat)selectoffset;
  scale[10] =
      (materials[interindex].spevec[2] - materials[selectindex].spevec[2]) /
      (GLfloat)selectoffset;
  scale[11] =
      (materials[interindex].spevec[3] - materials[selectindex].spevec[3]) /
      (GLfloat)selectoffset;
  scale[12] =
      (materials[interindex].emivec[0] - materials[selectindex].emivec[0]) /
      (GLfloat)selectoffset;
  scale[13] =
      (materials[interindex].emivec[1] - materials[selectindex].emivec[1]) /
      (GLfloat)selectoffset;
  scale[14] =
      (materials[interindex].emivec[2] - materials[selectindex].emivec[2]) /
      (GLfloat)selectoffset;
  scale[15] =
      (materials[interindex].emivec[3] - materials[selectindex].emivec[3]) /
      (GLfloat)selectoffset;
  scale[16] = (materials[interindex].shiny - materials[selectindex].shiny) /
              (GLfloat)selectoffset;
  scale[17] = (materials[interindex].trans - materials[selectindex].trans) /
              (GLfloat)selectoffset;

  // in - ter - po - lat - ion, ya! a - cross - the - nat - ion, uh!
  for (int i = selectindex + 1; i < interindex; i++) {
    materials[i].ambvec[0] = materials[i - 1].ambvec[0] + scale[0];
    materials[i].ambvec[1] = materials[i - 1].ambvec[1] + scale[1];
    materials[i].ambvec[2] = materials[i - 1].ambvec[2] + scale[2];
    materials[i].ambvec[3] = materials[i - 1].ambvec[3] + scale[3];
    materials[i].difvec[0] = materials[i - 1].difvec[0] + scale[4];
    materials[i].difvec[1] = materials[i - 1].difvec[1] + scale[5];
    materials[i].difvec[2] = materials[i - 1].difvec[2] + scale[6];
    materials[i].difvec[3] = materials[i - 1].difvec[3] + scale[7];
    materials[i].spevec[0] = materials[i - 1].spevec[0] + scale[8];
    materials[i].spevec[1] = materials[i - 1].spevec[1] + scale[9];
    materials[i].spevec[2] = materials[i - 1].spevec[2] + scale[10];
    materials[i].spevec[3] = materials[i - 1].spevec[3] + scale[11];
    materials[i].emivec[0] = materials[i - 1].emivec[0] + scale[12];
    materials[i].emivec[1] = materials[i - 1].emivec[1] + scale[13];
    materials[i].emivec[2] = materials[i - 1].emivec[2] + scale[14];
    materials[i].emivec[3] = materials[i - 1].emivec[3] + scale[15];
    materials[i].shiny = materials[i - 1].shiny + scale[16];
    materials[i].trans = materials[i - 1].trans + scale[17];
    materials[i].isD = false;
  }
  emit modify(true);
  emit notice("Interpolate " + QString::number(selectindex) + " to " +
              QString::number(interindex));
  update();
  if (_savingMode == TRIGGERED)
    save();
}

// set all selected to default
void MWViewer::defaultmat() {
  for (int i = 0; i < selectoffset; i++) {
    matcpy(&materials[selectindex + i], defmat);
  }

  emit setselect(selectindex, materials[selectindex]);
  emit modify(true);
  if (selectoffset == 1)
    emit notice("Default " + QString::number(selectindex));
  else
    emit notice("Default " + QString::number(selectindex) + " to " +
                QString::number(selectindex + selectoffset - 1));
  update();
}

// set all selected to newly edited  material (no going back)
void MWViewer::setMaterial(Material m) {
  for (int i = 0; i < selectoffset; i++) {
    matcpy(&materials[selectindex + i], m);
  }

  emit setselect(selectindex, materials[selectindex]);
  emit modify(true);
  if (selectoffset == 1)
    emit notice("Edit " + QString::number(selectindex));
  else
    emit notice("Edit " + QString::number(selectindex) + " to " +
                QString::number(selectindex + selectoffset - 1));
  update();
}

// show the edit on selected (we are still able to revert back)
void MWViewer::showMaterial(Material m) {
  for (int i = 0; i < selectoffset; i++) {
    matcpy(&materials[selectindex + i], m);
  }
  emit modify(true);
  update();
}

// custom-set default material (not used yet)
void MWViewer::setDefault(Material m) {
  matcpy(&defmat, m);
  emit setdefault(m); // for editor
}

// Page Options
void MWViewer::xspage() {
  pg = 1;
  row = 1;
  numpages = 256;
  pagenumber = selectindex + 1;
  pageindex = selectindex;
  if (pagesmooth) {
    base = 16;
    divisions = base + smoothness;
  }
  setdim();
  emit pageflip(pageindex, pg);
  update();
}

void MWViewer::smpage() {
  pg = 16;
  row = 4;
  numpages = 16;
  pagenumber = (selectindex / pg) + 1;
  pageindex = (pagenumber - 1) * pg;
  if (pagesmooth) {
    base = 8;
    divisions = base + smoothness;
  }
  setdim();
  emit pageflip(pageindex, pg);
  update();
}

void MWViewer::mdpage() {
  pg = 64;
  row = 8;
  numpages = 4;
  pagenumber = (selectindex / pg) + 1;
  pageindex = (pagenumber - 1) * pg;
  if (pagesmooth) {
    base = 6;
    divisions = base + smoothness;
  }
  setdim();
  emit pageflip(pageindex, pg);
  update();
}

void MWViewer::lgpage() {
  pg = 256;
  row = 16;
  numpages = 1;
  pagenumber = 1;
  pageindex = 0;
  if (pagesmooth) {
    base = 4;
    divisions = base + smoothness;
  }
  setdim();
  emit pageflip(pageindex, pg);
  update();
}

void MWViewer::nextpage() {
  pageindex += pg;
  if (selecting)
    selectrange(selectindex, selectoffset + pg);
  emit pageflip(pageindex, pg);
  update();
}

void MWViewer::prevpage() {
  pageindex -= pg;
  if (selecting)
    selectrange(selectindex, selectoffset - pg);
  emit pageflip(pageindex, pg);
  update();
}

void MWViewer::firstpage() {
  pageindex = 0;
  emit pageflip(pageindex, pg);
  update();
}

void MWViewer::gotopage(int p) {
  pagenumber = p;
  pageindex = (pagenumber - 1) * pg;
  emit pageflip(pageindex, pg);
  update();
}

// will be implemented to allow the user to set a custom page size
void MWViewer::pagerange(int, int) {}

// show background image
void MWViewer::showBackground(bool b) {
  imageON = b;
  update();
  seed = time(0);
}

void MWViewer::enablePaste() { _ps->setEnabled(true); }

// ------------------- Clipboard ----------------------
// read from clipboard into material buffer
int MWViewer::getCopied(Material *Mbuffer) {
  int bfsz, k = 0;

  QByteArray data(1 + (19 * 256), '\0');
  data = QApplication::clipboard()->mimeData()->data("text");

  bfsz = (int)((unsigned char)(data[k++]));
  if (bfsz == 0)
    bfsz = 256;
  for (int i = 0; i < bfsz; i++) {
    Mbuffer[i].ambvec[0] = (GLfloat)((unsigned char)data[k++]) / (GLfloat)255;
    Mbuffer[i].ambvec[1] = (GLfloat)((unsigned char)data[k++]) / (GLfloat)255;
    Mbuffer[i].ambvec[2] = (GLfloat)((unsigned char)data[k++]) / (GLfloat)255;
    Mbuffer[i].ambvec[3] = (GLfloat)((unsigned char)data[k++]) / (GLfloat)255;

    Mbuffer[i].difvec[0] = (GLfloat)((unsigned char)data[k++]) / (GLfloat)255;
    Mbuffer[i].difvec[1] = (GLfloat)((unsigned char)data[k++]) / (GLfloat)255;
    Mbuffer[i].difvec[2] = (GLfloat)((unsigned char)data[k++]) / (GLfloat)255;
    Mbuffer[i].difvec[3] = (GLfloat)((unsigned char)data[k++]) / (GLfloat)255;

    Mbuffer[i].emivec[0] = (GLfloat)((unsigned char)data[k++]) / (GLfloat)255;
    Mbuffer[i].emivec[1] = (GLfloat)((unsigned char)data[k++]) / (GLfloat)255;
    Mbuffer[i].emivec[2] = (GLfloat)((unsigned char)data[k++]) / (GLfloat)255;
    Mbuffer[i].emivec[3] = (GLfloat)((unsigned char)data[k++]) / (GLfloat)255;

    Mbuffer[i].spevec[0] = (GLfloat)((unsigned char)data[k++]) / (GLfloat)255;
    Mbuffer[i].spevec[1] = (GLfloat)((unsigned char)data[k++]) / (GLfloat)255;
    Mbuffer[i].spevec[2] = (GLfloat)((unsigned char)data[k++]) / (GLfloat)255;
    Mbuffer[i].spevec[3] = (GLfloat)((unsigned char)data[k++]) / (GLfloat)255;

    Mbuffer[i].trans = (GLfloat)((unsigned char)data[k++]) / (GLfloat)255;
    Mbuffer[i].shiny = (GLfloat)(unsigned char)data[k++];
    Mbuffer[i].isD = (bool)data[k++];
  }

  return bfsz;
}

// write material buffer to clipboard (as QByteArray)
void MWViewer::setCopied(int offset, int index) {

  int end = index + offset;
  int k = 0;

  QByteArray data(1 + (19 * 256), '\0');

  data[k++] = (unsigned char)(offset); // buffer size
  for (int i = index; i < end; i++) {
    data[k++] = (unsigned char)(materials[i].ambvec[0] * 255);
    data[k++] = (unsigned char)(materials[i].ambvec[1] * 255);
    data[k++] = (unsigned char)(materials[i].ambvec[2] * 255);
    data[k++] = (unsigned char)(materials[i].ambvec[3] * 255);

    data[k++] = (unsigned char)(materials[i].difvec[0] * 255);
    data[k++] = (unsigned char)(materials[i].difvec[1] * 255);
    data[k++] = (unsigned char)(materials[i].difvec[2] * 255);
    data[k++] = (unsigned char)(materials[i].difvec[3] * 255);

    data[k++] = (unsigned char)(materials[i].emivec[0] * 255);
    data[k++] = (unsigned char)(materials[i].emivec[1] * 255);
    data[k++] = (unsigned char)(materials[i].emivec[2] * 255);
    data[k++] = (unsigned char)(materials[i].emivec[3] * 255);

    data[k++] = (unsigned char)(materials[i].spevec[0] * 255);
    data[k++] = (unsigned char)(materials[i].spevec[1] * 255);
    data[k++] = (unsigned char)(materials[i].spevec[2] * 255);
    data[k++] = (unsigned char)(materials[i].spevec[3] * 255);

    data[k++] = (unsigned char)(materials[i].trans * 255);
    data[k++] = (unsigned char)materials[i].shiny;
    data[k++] = (unsigned char)materials[i].isD;
  }

  D = new QMimeData();
  D->setData("text", data);
  QApplication::clipboard()->setMimeData(D);
}

// --------------------- Rendering --------------------
void MWViewer::initializeGL() {
  glClearColor(0.0, 0.0, 0.0, 1.0);

  glShadeModel(GL_SMOOTH);
  glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lightmodel);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glCullFace(GL_BACK);
  glEnable(GL_CULL_FACE);

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);

  glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

  sphereobject = gluNewQuadric();
  gluQuadricDrawStyle(sphereobject, GLU_FILL);
  gluQuadricNormals(sphereobject, GLU_SMOOTH);
}

void MWViewer::resizeGL(int, int) { viewSetup(); }

void MWViewer::viewSetup() {
  glViewport(0, 0, (GLsizei)width(), (GLsizei)height());
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  setdim(); // sets dimension variables, and sphere radius
  glOrtho(0, dimX + 1, 0, dimY + 1, near, far);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void MWViewer::paintGL() {
  GLfloat x, y, dx, dy;
  bool defnow = false;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glShadeModel(GL_SMOOTH);

  srand(seed); // for background

  x = 0;
  y = dimY;
  dx = celldimX / 2.0;
  dy = celldimY / 2.0;

  // background
  if (imageON) {
    glDisable(GL_LIGHTING);
    if (background == "bigsplash")
      backsplash(0, dimY, dimX, 0);
    else if (background == "bigneutralsplash")
      backsplash_N(0, dimY, dimX, 0);
  }

  for (int j = 0; j < row; j++) {
    for (int i = 0; i < row; i++) {

      // background
      if (imageON) {
        glDisable(GL_LIGHTING);
        if (background == "checkerboard")
          backcheck(x, y, x + (dx + dx), y - (dy + dy));
        else if (background == "splash")
          backsplash(x, y, x + (dx + dx), y - (dy + dy));
        else if (background == "neutralsplash")
          backsplash_N(x, y, x + (dx + dx), y - (dy + dy));
      }

      // highlight selected
      if ((selectindex <= pageindex + i * row + j) &&
          (selectindex + selectoffset > pageindex + i * row + j)) {
        glDisable(GL_LIGHTING);
        glLineWidth(1);
        glBegin(GL_LINE_LOOP);
        {
          glColor3f(0.95, 0.15, 0.15);
          glVertex3f(x, y, 0);
          glVertex3f((x + (dx + dx)), y, 0);
          glVertex3f((x + (dx + dx)), (y - (dy + dy)), 0);
          glVertex3f(x, (y - (dy + dy)), 0);
        }
        glEnd();
      }

      // draw our sphere(s)
      glPushMatrix();
      glEnable(GL_LIGHTING);

      glTranslatef(x + dx, y - dy, 0.0);
      glRotatef(78.0, 0.0, 1.0, 0.0);
      glRotatef(52.0, 1.0, 0.0, 0.0);

      // change materials as little as possible (assumes many default are
      // grouped together, say, at end)
      if (!defnow) {
        if (materials[pageindex + i * row + j].isD) {
          defnow = true;
          glMaterialfv(GL_FRONT, GL_AMBIENT, defmat.ambvec);
          glMaterialfv(GL_FRONT, GL_DIFFUSE, defmat.difvec);
          glMaterialfv(GL_FRONT, GL_SPECULAR, defmat.spevec);
          glMaterialfv(GL_FRONT, GL_EMISSION, defmat.emivec);
          glMaterialfv(GL_FRONT, GL_SHININESS, &defmat.shiny);
        }
      } else if (!materials[pageindex + i * row + j].isD)
        defnow = false;

      if (!defnow) {
        glMaterialfv(GL_FRONT, GL_AMBIENT,
                     materials[pageindex + i * row + j].ambvec);
        glMaterialfv(GL_FRONT, GL_DIFFUSE,
                     materials[pageindex + i * row + j].difvec);
        glMaterialfv(GL_FRONT, GL_SPECULAR,
                     materials[pageindex + i * row + j].spevec);
        glMaterialfv(GL_FRONT, GL_EMISSION,
                     materials[pageindex + i * row + j].emivec);
        glMaterialfv(GL_FRONT, GL_SHININESS,
                     &materials[pageindex + i * row + j].shiny);
      }

      // I think calling display list is faster
      // multiply by 1.5 to smooth horizon
      gluSphere(sphereobject, radius, divisions, divisions);

      glPopMatrix();

      x += celldimX;
    }
    x = 0;
    y -= celldimY;
  }
  glFlush(); // please, remember to always flush!
}

// background draw functions
void MWViewer::backsplash(GLfloat xmin, GLfloat ymin, GLfloat xmax,
                          GLfloat ymax) {
  GLfloat col[3];
  glBegin(GL_QUADS);
  {
    randomcolour(col);
    glColor3fv(col);
    glVertex3f(xmin, ymin, bgplane);

    randomcolour(col);
    glColor3fv(col);
    glVertex3f(xmax, ymin, bgplane);

    randomcolour(col);
    glColor3fv(col);
    glVertex3f(xmax, ymax, bgplane);

    randomcolour(col);
    glColor3fv(col);
    glVertex3f(xmin, ymax, bgplane);
  }
  glEnd();
}

void MWViewer::backsplash_N(GLfloat xmin, GLfloat ymin, GLfloat xmax,
                            GLfloat ymax) {
  GLfloat light[] = {0.9, 0.9, 0.9};
  GLfloat dark[] = {0.5, 0.5, 0.5};

  glBegin(GL_QUADS);
  {
    glColor3fv(light);
    glVertex3f(xmin, ymin, bgplane);

    glColor3fv(dark);
    glVertex3f(xmax, ymin, bgplane);

    glColor3fv(light);
    glVertex3f(xmax, ymax, bgplane);

    glColor3fv(dark);
    glVertex3f(xmin, ymax, bgplane);
  }
  glEnd();
}

void MWViewer::backcheck(GLfloat xmin, GLfloat ymin, GLfloat xmax,
                         GLfloat ymax) {
  GLfloat lightcheck[] = {0.9, 0.9, 0.9};
  GLfloat darkcheck[] = {0.5, 0.5, 0.5};
  GLfloat midX = (xmax - xmin) / 2.0;
  GLfloat midY = (ymax - ymin) / 2.0;

  glBegin(GL_QUADS);
  {
    glColor3fv(lightcheck);
    glVertex3f(xmin, ymin, bgplane);
    glVertex3f(xmin + midX, ymin, bgplane);
    glVertex3f(xmin + midX, ymin + midY, bgplane);
    glVertex3f(xmin, ymin + midY, bgplane);

    glVertex3f(xmin + midX, ymin + midY, bgplane);
    glVertex3f(xmax, ymin + midY, bgplane);
    glVertex3f(xmax, ymax, bgplane);
    glVertex3f(xmin + midX, ymax, bgplane);

    glColor3fv(darkcheck);
    glVertex3f(xmin + midX, ymin, bgplane);
    glVertex3f(xmax, ymin, bgplane);
    glVertex3f(xmax, ymin + midY, bgplane);
    glVertex3f(xmin + midX, ymin + midY, bgplane);

    glVertex3f(xmin, ymin + midY, bgplane);
    glVertex3f(xmin + midX, ymin + midY, bgplane);
    glVertex3f(xmin + midX, ymax, bgplane);
    glVertex3f(xmin, ymax, bgplane);
  }
  glEnd();
}

void MWViewer::randomcolour(GLfloat *col) {
  col[0] = (GLfloat)(rand() % 255) / (GLfloat)255;
  col[1] = (GLfloat)(rand() % 255) / (GLfloat)255;
  col[2] = (GLfloat)(rand() % 255) / (GLfloat)255;
}

void MWViewer::randomgrey(GLfloat *col) {
  GLfloat num = GLfloat(rand() % 255) / 255.0;
  col[0] = num;
  col[1] = num;
  col[2] = num;
}

// -------------------- Mouse Event Handling --------------------
void MWViewer::mousePressEvent(QMouseEvent *me) {
  QWidget *topWidget = QApplication::topLevelAt(this->mapToGlobal(QPoint()));
  if (topWidget != nullptr)
    topWidget->raise();

  if (LeftButton == me->button()) {
    if (selectable) {
      selecting = true;
      selectrange(selection_test(me->x(), me->y()), 1);
      emit setselect(selectindex, materials[selectindex]);
    }
  } else if (me->button() == RightButton)
    menu->exec(QCursor::pos());
}

void MWViewer::mouseDoubleClickEvent(QMouseEvent *me) {
  if (Qt::LeftButton == me->button())
    if (selectable)
      emit edit();
}

void MWViewer::mouseMoveEvent(QMouseEvent *me) {
  int s = selection_test(me->x(), me->y());
  if (selecting) {
    if (s > selectindex) {
      selectrange(selectindex, (s - selectindex) + 1);
    } else {
      selectrange(selectindex, 1);
    }
  }
}

void MWViewer::mouseReleaseEvent(QMouseEvent *) { selecting = false; }

int MWViewer::selection_test(int x, int y) {
  GLfloat xp = (GLfloat)x * (dimX / (GLfloat)width());
  GLfloat yp = (GLfloat)y * (dimY / (GLfloat)height());
  int r = (int)(yp / celldimY);
  int c = (int)(xp / celldimX);
  return (pageindex + c * row + r);
}

void MWViewer::setdim() {
  GLfloat scale;
  if (width() < height()) {
    scale = (GLfloat)height() / (GLfloat)width();
    dimX = range;
    dimY = range * scale;
    celldimX = dimX / (GLfloat)row;
    celldimY = dimY / (GLfloat)row;
    radius = celldimX * 0.444;
    if (enhance)
      divisions =
          (int)(((GLfloat)(base + smoothness) * (GLfloat)width() * 0.00375) +
                0.5);
  } else if (width() == height()) {
    dimX = range;
    dimY = range;
    celldimY = celldimX = dimX / (GLfloat)row;
    radius = celldimX * 0.444;
    if (enhance)
      divisions =
          (int)(((GLfloat)(base + smoothness) * (GLfloat)width() * 0.00375) +
                0.5);
  } else {
    scale = (GLfloat)width() / (GLfloat)height();
    dimX = range * scale;
    dimY = range;
    celldimX = dimX / (GLfloat)row;
    celldimY = dimY / (GLfloat)row;
    radius = celldimY * 0.444;
    if (enhance)
      divisions =
          (int)(((GLfloat)(base + smoothness) * (GLfloat)height() * 0.00375) +
                0.5);
  }
}

// =================== Extra Functions ============================
// copy material properties: &dst, src
void matcpy(Material *m1, Material m2) {
  for (int i = 0; i < 4; i++) {
    m1->ambvec[i] = m2.ambvec[i];
    m1->difvec[i] = m2.difvec[i];
    m1->emivec[i] = m2.emivec[i];
    m1->spevec[i] = m2.spevec[i];
  }
  m1->shiny = m2.shiny;
  m1->trans = m2.trans;
  m1->isD = m2.isD;
}

// compare materials for equality, true if equal
bool matcmp(Material m1, Material m2) {
  for (int i = 0; i < 4; i++) {
    if (!eq(m1.ambvec[i], m2.ambvec[i]))
      return false;
    if (!eq(m1.difvec[i], m2.difvec[i]))
      return false;
    if (!eq(m1.spevec[i], m2.spevec[i]))
      return false;
    if (!eq(m1.emivec[i], m2.emivec[i]))
      return false;
  }
  if (!eq(m1.shiny, m2.shiny))
    return false;
  if (!eq(m1.trans, m2.trans))
    return false;
  return true;
}

void matprint(Material M) {
  std::cerr << "\n"
            << M.ambvec[0] << "," << M.ambvec[1] << "," << M.ambvec[2] << "\n"
            << M.difvec[0] << "," << M.difvec[1] << "," << M.difvec[2] << "\n"
            << M.spevec[0] << "," << M.spevec[1] << "," << M.spevec[2] << "\n"
            << M.emivec[0] << "," << M.emivec[1] << "," << M.emivec[2] << "\n"
            << M.shiny << "\n"
            << M.trans << "\n"
            << (int)M.isD << "\n";
}

// eof: mwviewer.cc
