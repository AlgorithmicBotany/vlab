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



/* MaterialWorld

   Definition of Class: MWeditor

   Last Modified by: Joanne
   On Date:
*/

#include <qcursor.h>
#include "mweditor.h"
#include <QBoxLayout>
#include <QCloseEvent>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QVBoxLayout>

using namespace Qt;

// ===================== Class: MWEditor ======================
// --------------------- Construction --------------------
MWEditor::MWEditor(QWidget *parent, const char *, Qt::WindowFlags f)
    : QWidget(parent, f) {
  setting = "jo is nice";
  edited = editing = off = false;

  CP = new ColourPick(this, "cp");

  QBoxLayout *top = new QVBoxLayout(this); // top level layout
  top->setMargin(3);
  QBoxLayout *panel = new QHBoxLayout();
  top->addLayout(panel);
  panel->setSpacing(8);
  panel->setMargin(3);
  QBoxLayout *button = new QHBoxLayout();
  top->addLayout(button);
  button->setSpacing(1);
  button->setMargin(3);
  QBoxLayout *display = new QVBoxLayout();
  panel->addLayout(display);
  display->setSpacing(1);
  QGridLayout *slide = new QGridLayout();
  panel->addLayout(slide);
  slide->setSpacing(3);
  slide->addItem(new QSpacerItem(150, 0), 0, 2);
  slide->addItem(new QSpacerItem(45, 0), 0, 0);
  slide->addItem(new QSpacerItem(70, 0), 0, 3);

  MD = new MaterialDisplay(this, "md");
  status = new QStatusBar(this);
  status->setSizeGripEnabled(false);
  display->addWidget(MD, 10);
  display->addWidget(status, 1);

  ambCS = new ColourSwatch(this, "ambCS");
  ambLB = new QLabel("Ambient", this);
  ambSL = new QSlider(Qt::Horizontal, this);
  ambSL->setObjectName("ambSL");
  ambSL->setMinimum(0);
  ambSL->setMaximum(1000);
  ambSL->setPageStep(125);
  ambSL->setValue(0);

  ambSL->setTickPosition(QSlider::TicksAbove);

  ambLE = new QLineEdit("0.000", this);
  ambLE->setMaximumWidth(85);

  slide->addWidget(ambCS, 0, 0);
  slide->addWidget(ambLB, 0, 1);
  slide->addWidget(ambSL, 0, 2);
  slide->addWidget(ambLE, 0, 3);

  difCS = new ColourSwatch(this, "difCS");
  difLB = new QLabel("Diffuse", this);
  difSL = new QSlider(Qt::Horizontal, this);
  difSL->setObjectName("difSL");
  difSL->setMinimum(0);
  difSL->setMaximum(1000);
  difSL->setPageStep(125);
  difSL->setValue(0);
  difSL->setTickPosition(QSlider::TicksAbove);
  difLE = new QLineEdit("0.000", this);
  difLE->setMaximumWidth(85);
  slide->addWidget(difCS, 1, 0);
  slide->addWidget(difLB, 1, 1);
  slide->addWidget(difSL, 1, 2);
  slide->addWidget(difLE, 1, 3);

  specCS = new ColourSwatch(this, "specCS");
  specLB = new QLabel("Specular", this);
  specSL = new QSlider(Qt::Horizontal, this);
  specSL->setObjectName("specSL");
  specSL->setMinimum(0);
  specSL->setMaximum(1000);
  specSL->setPageStep(125);
  specSL->setValue(0);
  specSL->setTickPosition(QSlider::TicksAbove);
  specLE = new QLineEdit("0.000", this);
  specLE->setMaximumWidth(85);
  slide->addWidget(specCS, 2, 0);
  slide->addWidget(specLB, 2, 1);
  slide->addWidget(specSL, 2, 2);
  slide->addWidget(specLE, 2, 3);

  emCS = new ColourSwatch(this, "emCS");
  emLB = new QLabel("Emissive", this);
  emSL = new QSlider(Qt::Horizontal, this);
  emSL->setObjectName("emSL");
  emSL->setMinimum(0);
  emSL->setMaximum(1000);
  emSL->setPageStep(125);
  emSL->setValue(0);

  emSL->setTickPosition(QSlider::TicksAbove);
  emLE = new QLineEdit("0.000", this);
  emLE->setMaximumWidth(85);
  slide->addWidget(emCS, 3, 0);
  slide->addWidget(emLB, 3, 1);
  slide->addWidget(emSL, 3, 2);
  slide->addWidget(emLE, 3, 3);

  shinLB = new QLabel("Shininess", this);
  shinSL = new QSlider(Qt::Horizontal, this);
  shinSL->setObjectName("shinSL");
  shinSL->setMinimum(0);
  shinSL->setMaximum(12800);
  shinSL->setPageStep(1280);
  shinSL->setValue(0);

  shinSL->setTickPosition(QSlider::TicksAbove);
  shinLE = new QLineEdit("0.00", this);
  shinLE->setMaximumWidth(85);
  slide->addWidget(shinLB, 4, 1);
  slide->addWidget(shinSL, 4, 2);
  slide->addWidget(shinLE, 4, 3);

  transLB = new QLabel("Transparency", this);
  transSL = new QSlider(Qt::Horizontal, this);
  ;
  transSL->setObjectName("transSL");
  transSL->setMinimum(0);
  transSL->setMaximum(1000);
  transSL->setPageStep(125);
  transSL->setValue(0);
  transSL->setTickPosition(QSlider::TicksAbove);

  transLE = new QLineEdit("0.000", this);
  transLE->setMaximumWidth(85);
  slide->addWidget(transLB, 5, 1);
  slide->addWidget(transSL, 5, 2);
  slide->addWidget(transLE, 5, 3);


  defaultPB = new QPushButton("Default", this);
  undoPB = new QPushButton("Undo", this);
  applyPB = new QPushButton("Apply", this);
  closePB = new QPushButton("Close", this);
  button->addWidget(defaultPB);
  button->addSpacing(20);
  button->addWidget(undoPB);
  button->addSpacing(20);
  button->addWidget(applyPB);
  button->addSpacing(20);
  button->addWidget(closePB);
  QObject::connect(defaultPB, SIGNAL(clicked()), this, SLOT(defaultM()));
  QObject::connect(undoPB, SIGNAL(clicked()), this, SLOT(undoM()));
  QObject::connect(applyPB, SIGNAL(clicked()), this, SLOT(applyM()));
  QObject::connect(closePB, SIGNAL(clicked()), this, SLOT(close()));

  // whoa, momma!
  QObject::connect(ambCS, SIGNAL(edit()), this, SLOT(editAmbColour()));
  QObject::connect(difCS, SIGNAL(edit()), this, SLOT(editDifColour()));
  QObject::connect(specCS, SIGNAL(edit()), this, SLOT(editSpecColour()));
  QObject::connect(emCS, SIGNAL(edit()), this, SLOT(editEmColour()));
  //-------- colour pick
  QObject::connect(this, SIGNAL(pickColour()), CP, SLOT(getColour()));
  QObject::connect(CP, SIGNAL(applyEvent(GLfloat *)), this,
                   SLOT(showPick(GLfloat *)));
  //--------
  QObject::connect(ambSL, SIGNAL(valueChanged(int)), this,
                   SLOT(updateAmb(int)));
  QObject::connect(difSL, SIGNAL(valueChanged(int)), this,
                   SLOT(updateDif(int)));
  QObject::connect(specSL, SIGNAL(valueChanged(int)), this,
                   SLOT(updateSpec(int)));
  QObject::connect(emSL, SIGNAL(valueChanged(int)), this, SLOT(updateEm(int)));
  QObject::connect(transSL, SIGNAL(valueChanged(int)), this,
                   SLOT(updateTrans(int)));
  QObject::connect(shinSL, SIGNAL(valueChanged(int)), this,
                   SLOT(updateShiny(int)));

  QObject::connect(ambSL, SIGNAL(sliderPressed()), this, SLOT(editingYes()));
  QObject::connect(difSL, SIGNAL(sliderPressed()), this, SLOT(editingYes()));
  QObject::connect(specSL, SIGNAL(sliderPressed()), this, SLOT(editingYes()));
  QObject::connect(emSL, SIGNAL(sliderPressed()), this, SLOT(editingYes()));
  QObject::connect(shinSL, SIGNAL(sliderPressed()), this, SLOT(editingYes()));
  QObject::connect(transSL, SIGNAL(sliderPressed()), this, SLOT(editingYes()));
  QObject::connect(ambSL, SIGNAL(sliderReleased()), this, SLOT(editingNo()));
  QObject::connect(difSL, SIGNAL(sliderReleased()), this, SLOT(editingNo()));
  QObject::connect(specSL, SIGNAL(sliderReleased()), this, SLOT(editingNo()));
  QObject::connect(emSL, SIGNAL(sliderReleased()), this, SLOT(editingNo()));
  QObject::connect(shinSL, SIGNAL(sliderReleased()), this, SLOT(editingNo()));
  QObject::connect(transSL, SIGNAL(sliderReleased()), this, SLOT(editingNo()));

  QObject::connect(ambSL, SIGNAL(actionTriggered(int)), this,
                   SLOT(editingYes()));
  QObject::connect(difSL, SIGNAL(actionTriggered(int)), this,
                   SLOT(editingYes()));
  QObject::connect(specSL, SIGNAL(actionTriggered(int)), this,
                   SLOT(editingYes()));
  QObject::connect(emSL, SIGNAL(actionTriggered(int)), this,
                   SLOT(editingYes()));
  QObject::connect(shinSL, SIGNAL(actionTriggered(int)), this,
                   SLOT(editingYes()));
  QObject::connect(transSL, SIGNAL(actionTriggered(int)), this,
                   SLOT(editingYes()));

  QObject::connect(ambLE, SIGNAL(returnPressed()), this, SLOT(editAmb()));
  QObject::connect(difLE, SIGNAL(returnPressed()), this, SLOT(editDif()));
  QObject::connect(specLE, SIGNAL(returnPressed()), this, SLOT(editSpec()));
  QObject::connect(emLE, SIGNAL(returnPressed()), this, SLOT(editEm()));
  QObject::connect(shinLE, SIGNAL(returnPressed()), this, SLOT(editShiny()));
  QObject::connect(transLE, SIGNAL(returnPressed()), this, SLOT(editTrans()));
  QObject::connect(ambCS, SIGNAL(copycolour(QString &)), this,
                   SLOT(copycolour(QString &)));
  QObject::connect(difCS, SIGNAL(copycolour(QString &)), this,
                   SLOT(copycolour(QString &)));
  QObject::connect(specCS, SIGNAL(copycolour(QString &)), this,
                   SLOT(copycolour(QString &)));
  QObject::connect(emCS, SIGNAL(copycolour(QString &)), this,
                   SLOT(copycolour(QString &)));
  QObject::connect(ambCS, SIGNAL(pastecolour(QString &)), this,
                   SLOT(pastecolour(QString &)));
  QObject::connect(difCS, SIGNAL(pastecolour(QString &)), this,
                   SLOT(pastecolour(QString &)));
  QObject::connect(specCS, SIGNAL(pastecolour(QString &)), this,
                   SLOT(pastecolour(QString &)));
  QObject::connect(emCS, SIGNAL(pastecolour(QString &)), this,
                   SLOT(pastecolour(QString &)));
  QObject::connect(this, SIGNAL(isEdited(bool)), this, SLOT(notify(bool)));

  resize(10, 10);
}

// user selected material from viewer/MW
void MWEditor::selectM(int i, Material m) {
  setWindowTitle("Medit : " + parentWidget()->windowTitle() + " : Material " +
                 QString::number(i));
  matcpy(&Moriginal, m);
  matcpy(&Mselect, m);
  updateMaterial();
  if (CP->isVisible())
    editAmbColour();
  edited = false;
  emit isEdited(false);
}

void MWEditor::notify(bool M) {
  if (M)
    status->showMessage("Modified");
  else
    status->showMessage("");
}

// user selected "edit" option
void MWEditor::edit() {
  undoPB->setDown(false);
  defaultPB->setDown(false);
  applyPB->setDown(false);
  undoPB->setDown(false);
  if (!this->isVisible()) {
    QPoint parentPosition = parentWidget()->pos();
    QPoint widgetPosition(parentPosition.x(), parentPosition.y() - 250);
    move(widgetPosition);
  }

  show();
}

// the buttons do this...
// applies the edit, and closes editor dialog
// edit is "set in stone"
void MWEditor::ok() {
  emit doneEvent(Mselect);
  edited = false;
  emit isEdited(false);
  hide();
  CP->hide();
}

// reverts to original and closes editor dialog (edit is lost)
void MWEditor::cancel() {
  emit doneEvent(Moriginal);
  edited = false;
  emit isEdited(false);
  CP->hide();
  hide();
}

// revert back to original (when selection was made)
void MWEditor::revertM() {
  matcpy(&Mselect, Moriginal);
  updateMaterial();
  edited = false;
  emit isEdited(false);
}

// applies the edit to the selection
// we can revert back as long as a new selection is not made
void MWEditor::applyM() {
  emit applyEvent(Mselect);
  CP->resetInit();
  edited = false;
  emit triggered();
  emit isEdited(false);
}

// set selection to default material
void MWEditor::defaultM() {
  if (!Mselect.isD) {
    matcpy(&Mselect, Mdefault);
    updateMaterial();
    if (CP->isVisible()) {
      if (setting == "ambient") {
        CP->initColour(Mselect.ambvec);
      } else if (setting == "diffuse") {
        CP->initColour(Mselect.difvec);
      } else if (setting == "specular") {
        CP->initColour(Mselect.spevec);
      } else if (setting == "emissive") {
        CP->initColour(Mselect.emivec);
      }
    }
    edited = true;
    emit isEdited(true);
    emit applyEvent(Mselect);
  }
}

void MWEditor::undoM() {
  revertM();
  if (CP->isVisible()) {
    if (setting == "ambient") {
      CP->initColour(Mselect.ambvec);
    } else if (setting == "diffuse") {
      CP->initColour(Mselect.difvec);
    } else if (setting == "specular") {
      CP->initColour(Mselect.spevec);
    } else if (setting == "emissive") {
      CP->initColour(Mselect.emivec);
    }
  }
  emit applyEvent(Mselect);
}

// set the default material
void MWEditor::setdefault(Material m) { matcpy(&Mdefault, m); }

// set the sliders etc. to show the material selected
void MWEditor::updateMaterial() {
  // calculate the colours and coefficients  (??????????????)
  Colour *c = new Colour();

  c->setrgb(Mselect.ambvec);
  amb = c->v();
  c->v(1.0);
  ambcol[0] = c->r();
  ambcol[1] = c->g();
  ambcol[2] = c->b();

  c->setrgb(Mselect.difvec);
  dif = c->v();
  c->v(1.0);
  difcol[0] = c->r();
  difcol[1] = c->g();
  difcol[2] = c->b();

  c->setrgb(Mselect.spevec);
  spec = c->v();
  c->v(1.0);
  speccol[0] = c->r();
  speccol[1] = c->g();
  speccol[2] = c->b();

  c->setrgb(Mselect.emivec);
  em = c->v();
  c->v(1.0);
  emcol[0] = c->r();
  emcol[1] = c->g();
  emcol[2] = c->b();

  shin = Mselect.shiny;
  trans = Mselect.trans;

  // display everything
  ambSL->setValue((int)(amb * 1000));
  ambCS->display(Mselect.ambvec, ambcol);
  difSL->setValue((int)(dif * 1000));
  difCS->display(Mselect.difvec, difcol);
  specSL->setValue((int)(spec * 1000));
  specCS->display(Mselect.spevec, speccol);
  emSL->setValue((int)(em * 1000));
  emCS->display(Mselect.emivec, emcol);
  shinSL->setValue((int)(shin * 100));
  transSL->setValue((int)(trans * 1000));
  MD->display(Mselect);
}

// catching changes to sliders
// if slider is being used, "editing" is true
void MWEditor::updateAmb(int a) {
  amb = (GLfloat)a / (GLfloat)1000;
  ambLE->setText(QString::number(amb, 'f', 3));
  if (editing) {

    Mselect.ambvec[0] = amb * ambcol[0];
    Mselect.ambvec[1] = amb * ambcol[1];
    Mselect.ambvec[2] = amb * ambcol[2];
    Mselect.isD = false;

    edited = true;
    emit isEdited(true);
    // to dynamically change in editor
    emit applyEvent(Mselect);

    MD->display(Mselect);
    ambCS->display(Mselect.ambvec, ambcol);
    if (setting == "ambient") {
      CP->updateIntensity(a);
    }
  }
}

void MWEditor::updateDif(int d) {
  dif = (GLfloat)d / (GLfloat)1000;
  difLE->setText(QString::number(dif, 'f', 3));
  if (editing) {
    Mselect.difvec[0] = dif * difcol[0];
    Mselect.difvec[1] = dif * difcol[1];
    Mselect.difvec[2] = dif * difcol[2];
    Mselect.isD = false;

    edited = true;
    emit isEdited(true);
    // to dynamically change in editor
    emit applyEvent(Mselect);

    MD->display(Mselect);
    difCS->display(Mselect.difvec, difcol);
    if (setting == "diffuse") {
      CP->updateIntensity(d);
    }
  }
}

void MWEditor::updateSpec(int s) {
  spec = (GLfloat)s / (GLfloat)1000;
  specLE->setText(QString::number(spec, 'f', 3));
  if (editing) {

    Mselect.spevec[0] = spec * speccol[0];
    Mselect.spevec[1] = spec * speccol[1];
    Mselect.spevec[2] = spec * speccol[2];
    Mselect.isD = false;

    edited = true;
    emit isEdited(true);
    // to dynamically change in editor
    emit applyEvent(Mselect);

    MD->display(Mselect);
    specCS->display(Mselect.spevec, speccol);
    if (setting == "specular") {
      CP->updateIntensity(s);
    }
  }
}

void MWEditor::updateEm(int e) {
  em = (GLfloat)e / (GLfloat)1000;
  emLE->setText(QString::number(em, 'f', 3));
  if (editing) {

    Mselect.emivec[0] = em * emcol[0];
    Mselect.emivec[1] = em * emcol[1];
    Mselect.emivec[2] = em * emcol[2];
    Mselect.isD = false;

    edited = true;
    emit isEdited(true);
    // to dynamically change in editor
    emit applyEvent(Mselect);

    MD->display(Mselect);
    emCS->display(Mselect.emivec, emcol);
    if (setting == "emissive") {
      CP->updateIntensity(e);
    }
  }
}

void MWEditor::updateShiny(int sh) {
  shin = (GLfloat)sh / (GLfloat)100;
  shinLE->setText(QString::number(shin, 'f', 3));
  if (editing) {

    Mselect.shiny = shin;
    Mselect.isD = false;

    edited = true;
    emit isEdited(true);
    // to dynamically change in editor
    emit applyEvent(Mselect);

    MD->display(Mselect);
  }
}

void MWEditor::updateTrans(int tr) {
  trans = (GLfloat)tr / (GLfloat)1000;
  transLE->setText(QString::number(trans, 'f', 3));
  if (editing) {

    Mselect.trans = trans;
    Mselect.ambvec[3] = 1 - trans;
    Mselect.difvec[3] = 1 - trans;
    Mselect.spevec[3] = 1 - trans;
    Mselect.emivec[3] = 1 - trans;
    Mselect.isD = false;

    edited = true;
    MD->display(Mselect);
    emit isEdited(true);
    // to dynamically change in editor
    emit applyEvent(Mselect);
  }
}

// catching chages to text field
void MWEditor::editAmb() {
  bool ok = false;
  GLfloat a;
  a = ambLE->text().toFloat(&ok);
  if ((ok) && ((a < 0) || (a > 1)))
    ok = false;
  if (!ok) {
    status->showMessage("0 <= Ambient <= 1", 2500);
    ambLE->setText(QString::number(amb, 'f', 3));
    return;
  }
  ambSL->setValue((int)(a * 1000));

  Mselect.ambvec[0] = amb * ambcol[0];
  Mselect.ambvec[1] = amb * ambcol[1];
  Mselect.ambvec[2] = amb * ambcol[2];
  Mselect.isD = false;

  edited = true;
  emit isEdited(true);
  // to dynamically change in editor
  emit applyEvent(Mselect);

  MD->display(Mselect);
  if (setting == "ambient") {
    CP->updateIntensity(ambSL->value());
  }
}

void MWEditor::editDif() {
  bool ok = false;
  GLfloat d;
  d = difLE->text().toFloat(&ok);
  if ((ok) && ((d < 0) || (d > 1)))
    ok = false;
  if (!ok) {
    status->showMessage("0 <= Diffuse <= 1", 2500);
    difLE->setText(QString::number(dif, 'f', 3));
    return;
  }
  difSL->setValue((int)(d * 1000));

  Mselect.difvec[0] = dif * difcol[0];
  Mselect.difvec[1] = dif * difcol[1];
  Mselect.difvec[2] = dif * difcol[2];
  Mselect.isD = false;

  edited = true;
  emit isEdited(true);
  // to dynamically change in editor
  emit applyEvent(Mselect);

  MD->display(Mselect);
  if (setting == "diffuse") {
    CP->updateIntensity(difSL->value());
  }
}

void MWEditor::editSpec() {
  bool ok = false;
  GLfloat s;
  s = specLE->text().toFloat(&ok);
  if ((ok) && ((s < 0) || (s > 1)))
    ok = false;
  if (!ok) {
    status->showMessage("0 <= Specular <= 1", 2500);
    specLE->setText(QString::number(spec, 'f', 3));
    return;
  }
  specSL->setValue((int)(s * 1000));

  Mselect.spevec[0] = spec * speccol[0];
  Mselect.spevec[1] = spec * speccol[1];
  Mselect.spevec[2] = spec * speccol[2];
  Mselect.isD = false;

  edited = true;
  emit isEdited(true);
  // to dynamically change in editor
  emit applyEvent(Mselect);

  MD->display(Mselect);
  if (setting == "specular") {
    CP->updateIntensity(specSL->value());
  }
}

void MWEditor::editEm() {
  bool ok = false;
  GLfloat e;
  e = emLE->text().toFloat(&ok);
  if ((ok) && ((e < 0) || (e > 1)))
    ok = false;
  if (!ok) {
    status->showMessage("0 <= Emissive <= 1", 2500);
    emLE->setText(QString::number(em, 'f', 3));
    return;
  }
  emSL->setValue((int)(e * 1000));

  Mselect.emivec[0] = em * emcol[0];
  Mselect.emivec[1] = em * emcol[1];
  Mselect.emivec[2] = em * emcol[2];
  Mselect.isD = false;

  edited = true;
  emit isEdited(true);
  // to dynamically change in editor
  emit applyEvent(Mselect);

  MD->display(Mselect);
  if (setting == "emissive") {
    CP->updateIntensity(emSL->value());
  }
}

void MWEditor::editShiny() {
  bool ok = false;
  GLfloat sh;
  sh = shinLE->text().toFloat(&ok);
  if ((ok) && ((sh < 0) || (sh > 128)))
    ok = false;
  if (!ok) {
    status->showMessage("0 <= Shininess <= 128", 2500);
    shinLE->setText(QString::number(shin, 'f', 3));
    return;
  }
  shinSL->setValue((int)(sh * 100));

  Mselect.shiny = shin;
  Mselect.isD = false;

  edited = true;
  emit isEdited(true);
  // to dynamically change in editor
  emit applyEvent(Mselect);

  MD->display(Mselect);
}

void MWEditor::editTrans() {
  bool ok = false;
  GLfloat tr;
  tr = transLE->text().toFloat(&ok);
  if ((ok) && ((tr < 0) || (tr > 1)))
    ok = false;
  if (!ok) {
    status->showMessage("0 <= Transparency <= 1", 2500);
    transLE->setText(QString::number(trans, 'f', 3));
    return;
  }
  transSL->setValue((int)(tr * 1000));

  Mselect.trans = trans;
  Mselect.ambvec[3] = 1 - trans;
  Mselect.difvec[3] = 1 - trans;
  Mselect.spevec[3] = 1 - trans;
  Mselect.emivec[3] = 1 - trans;
  Mselect.isD = false;

  edited = true;
  emit isEdited(true);
  // to dynamically change in editor
  emit applyEvent(Mselect);

  MD->display(Mselect);
}

// catching chages to colour swatch
// edit current colour at current intensity
void MWEditor::editAmbColour() {
  setting = "ambient";

  cancol[0] = Mselect.ambvec[0];
  cancol[1] = Mselect.ambvec[1];
  cancol[2] = Mselect.ambvec[2];

  CP->initColour(Mselect.ambvec);
  QString cap = windowTitle();
  cap.remove(0, 8);
  CP->setWindowTitle("Colour Pick : " + cap + " : " + setting);
  emit pickColour();
}

void MWEditor::editDifColour() {
  setting = "diffuse";

  cancol[0] = Mselect.difvec[0];
  cancol[1] = Mselect.difvec[1];
  cancol[2] = Mselect.difvec[2];

  CP->initColour(Mselect.difvec);
  QString cap = windowTitle();
  cap.remove(0, 8);
  CP->setWindowTitle("Colour Pick : " + cap + " : " + setting);
  emit pickColour();
}

void MWEditor::editSpecColour() {
  setting = "specular";

  cancol[0] = Mselect.spevec[0];
  cancol[1] = Mselect.spevec[1];
  cancol[2] = Mselect.spevec[2];

  CP->initColour(Mselect.spevec);
  QString cap = windowTitle();
  cap.remove(0, 8);
  CP->setWindowTitle("Colour Pick : " + cap + " : " + setting);
  emit pickColour();
}

void MWEditor::editEmColour() {
  setting = "emissive";

  cancol[0] = Mselect.emivec[0];
  cancol[1] = Mselect.emivec[1];
  cancol[2] = Mselect.emivec[2];

  CP->initColour(Mselect.emivec);
  QString cap = windowTitle();
  cap.remove(0, 8);
  CP->setWindowTitle("Colour Pick : " + cap + " : " + setting);
  emit pickColour();
}

// copy/paste full intensity
void MWEditor::copycolour(QString &name) {
  if (name == "ambCS") {
    copycol[0] = ambcol[0];
    copycol[1] = ambcol[1];
    copycol[2] = ambcol[2];
  }
  if (name == "difCS") {
    copycol[0] = difcol[0];
    copycol[1] = difcol[1];
    copycol[2] = difcol[2];
  }
  if (name == "specCS") {
    copycol[0] = speccol[0];
    copycol[1] = speccol[1];
    copycol[2] = speccol[2];
  }
  if (name == "emCS") {
    copycol[0] = emcol[0];
    copycol[1] = emcol[1];
    copycol[2] = emcol[2];
  }
}

void MWEditor::pastecolour(QString &name) {
  if (name == "ambCS") {
    ambcol[0] = copycol[0];
    ambcol[1] = copycol[1];
    ambcol[2] = copycol[2];
    Mselect.ambvec[0] = amb * ambcol[0];
    Mselect.ambvec[1] = amb * ambcol[1];
    Mselect.ambvec[2] = amb * ambcol[2];
    ambCS->display(Mselect.ambvec, ambcol);
  }
  if (name == "difCS") {
    difcol[0] = copycol[0];
    difcol[1] = copycol[1];
    difcol[2] = copycol[2];
    Mselect.difvec[0] = dif * difcol[0];
    Mselect.difvec[1] = dif * difcol[1];
    Mselect.difvec[2] = dif * difcol[2];
    difCS->display(Mselect.difvec, difcol);
  }
  if (name == "specCS") {
    speccol[0] = copycol[0];
    speccol[1] = copycol[1];
    speccol[2] = copycol[2];
    Mselect.spevec[0] = spec * speccol[0];
    Mselect.spevec[1] = spec * speccol[1];
    Mselect.spevec[2] = spec * speccol[2];
    specCS->display(Mselect.spevec, speccol);
  }
  if (name == "emCS") {
    emcol[0] = copycol[0];
    emcol[1] = copycol[1];
    emcol[2] = copycol[2];
    Mselect.emivec[0] = em * emcol[0];
    Mselect.emivec[1] = em * emcol[1];
    Mselect.emivec[2] = em * emcol[2];
    emCS->display(Mselect.emivec, emcol);
  }
  edited = true;
  emit isEdited(true);
  // to dynamically change in editor
  emit applyEvent(Mselect);

  MD->display(Mselect);
}

void MWEditor::showPick(GLfloat *col) {
  Colour *c = new Colour();
  if (setting == "ambient") {
    c->setrgb(col);
    Mselect.ambvec[0] = c->r();
    Mselect.ambvec[1] = c->g();
    Mselect.ambvec[2] = c->b();
    amb = c->v();
    c->v(1.0);
    ambcol[0] = c->r();
    ambcol[1] = c->g();
    ambcol[2] = c->b();
    ambCS->display(Mselect.ambvec, ambcol);
    ambSL->setValue((int)(amb * 1000));
  }
  if (setting == "diffuse") {
    c->setrgb(col);
    Mselect.difvec[0] = c->r();
    Mselect.difvec[1] = c->g();
    Mselect.difvec[2] = c->b();
    dif = c->v();
    c->v(1.0);
    difcol[0] = c->r();
    difcol[1] = c->g();
    difcol[2] = c->b();
    difCS->display(Mselect.difvec, difcol);
    difSL->setValue((int)(dif * 1000));
  }
  if (setting == "specular") {
    c->setrgb(col);
    Mselect.spevec[0] = c->r();
    Mselect.spevec[1] = c->g();
    Mselect.spevec[2] = c->b();
    spec = c->v();
    c->v(1.0);
    speccol[0] = c->r();
    speccol[1] = c->g();
    speccol[2] = c->b();
    specCS->display(Mselect.spevec, speccol);
    specSL->setValue((int)(spec * 1000));
  }
  if (setting == "emissive") {
    c->setrgb(col);
    Mselect.emivec[0] = c->r();
    Mselect.emivec[1] = c->g();
    Mselect.emivec[2] = c->b();
    em = c->v();
    c->v(1.0);
    emcol[0] = c->r();
    emcol[1] = c->g();
    emcol[2] = c->b();
    emCS->display(Mselect.emivec, emcol);
    emSL->setValue((int)(em * 1000));
  }
  if (!matcmp(Mselect, Mdefault))
    Mselect.isD = false;
  if (!matcmp(Mselect, Moriginal)) {
    edited = true;
    emit isEdited(true);
    // to dynamically change in editor
    emit applyEvent(Mselect);
  }
  MD->display(Mselect);
}

// upkeeping flags
void MWEditor::editingYes() { editing = true; }

void MWEditor::editingNo() {
  editing = false;
  emit triggered();
}

void MWEditor::closeEvent(QCloseEvent *) {
  edited = false;
  emit isEdited(false);
  CP->close();
  hide();
}

// ===================== Class: MaterialDisplay ======================
// --------------------- Construction --------------------
MaterialDisplay::MaterialDisplay(QWidget *parent, const char *name)
    : QOpenGLWidget(parent) {
  setMinimumWidth(150);
  setMinimumHeight(150);

  range = 1000;
  near = -3000;
  far = 2000;

  radius = 375;

  lightpos[0] = 1000.0; // x
  lightpos[1] = 1000.0; // y
  lightpos[2] = 2000.0; // z
  lightpos[3] = 0.0;    // directional

  lightmodel[0] = 1.0; // white ambient light
  lightmodel[1] = 1.0;
  lightmodel[2] = 1.0;
  lightmodel[3] = 1.0;
}

void MaterialDisplay::display(Material mat) {
  matcpy(&M, mat);
  update();
}

// setting lights
void MaterialDisplay::lightup(GLfloat *lp, GLfloat *lm) {
  lightpos[0] = lp[0];
  lightpos[1] = lp[1];
  lightpos[2] = lp[2];
  lightpos[3] = lp[3];

  lightmodel[0] = lm[0];
  lightmodel[1] = lm[1];
  lightmodel[2] = lm[2];
  lightmodel[3] = lm[3];
}

// -------------------- Rendering --------------------
void MaterialDisplay::initializeGL() {
  glClearColor(0.0, 0.0, 0.0, 1.0);

  glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lightmodel);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_BLEND);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_DEPTH_TEST);

  sphereobject = gluNewQuadric();
  gluQuadricDrawStyle(sphereobject, GLU_FILL);
  gluQuadricNormals(sphereobject, GLU_SMOOTH);

  spheredrawlist = glGenLists(1);
  glNewList(spheredrawlist, GL_COMPILE);
  { gluSphere(sphereobject, radius, 80, 80); }
  glEndList();
}

void MaterialDisplay::resizeGL(int, int) {
  glViewport(0, 0, (GLsizei)width(), (GLsizei)height());
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  GLfloat scale;

  if (width() < height()) {
    scale = (GLfloat)height() / (GLfloat)width();
    xmin = -0.5 * range;
    xmax = 0.5 * range;
    ymin = -0.5 * range * scale;
    ymax = 0.5 * range * scale;
  } else if (width() == height()) {
    xmin = -0.5 * range;
    xmax = 0.5 * range;
    ymin = -0.5 * range;
    ymax = 0.5 * range;
  } else {
    scale = (GLfloat)width() / (GLfloat)height();
    xmin = -0.5 * range * scale;
    xmax = 0.5 * range * scale;
    ymin = -0.5 * range;
    ymax = 0.5 * range;
  }

  glOrtho(xmin, xmax, ymin, ymax, near, far);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void MaterialDisplay::paintGL() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // traditional background for showing transparency
  glDisable(GL_LIGHTING);
  GLfloat lightcheck[] = {0.9, 0.9, 0.9};
  GLfloat darkcheck[] = {0.5, 0.5, 0.5};

  glBegin(GL_QUADS);
  {
    glColor3fv(lightcheck);
    glVertex3f(xmin, ymax, -500);
    glVertex3f(0, ymax, -500);
    glVertex3f(0, 0, -500);
    glVertex3f(xmin, 0, -500);

    glVertex3f(0, 0, -500);
    glVertex3f(xmax, 0, -500);
    glVertex3f(xmax, ymin, -500);
    glVertex3f(0, ymin, -500);

    glColor3fv(darkcheck);
    glVertex3f(0, ymax, -500);
    glVertex3f(xmax, ymax, -500);
    glVertex3f(xmax, 0, -500);
    glVertex3f(0, 0, -500);

    glVertex3f(xmin, 0, -500);
    glVertex3f(0, 0, -500);
    glVertex3f(0, ymin, -500);
    glVertex3f(xmin, ymin, -500);
  }
  glEnd();

  // draw our sphere
  glPushMatrix();
  glEnable(GL_LIGHTING);
  glShadeModel(GL_SMOOTH);

  glRotatef(90.0, 0.0, 1.0, 0.0);
  glRotatef(45.0, 1.0, 0.0, 0.0);

  glMaterialfv(GL_FRONT, GL_AMBIENT, M.ambvec);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, M.difvec);
  glMaterialfv(GL_FRONT, GL_SPECULAR, M.spevec);
  glMaterialfv(GL_FRONT, GL_EMISSION, M.emivec);
  glMaterialfv(GL_FRONT, GL_SHININESS, &M.shiny);

  glCallList(spheredrawlist);

  glPopMatrix();

  glFlush();
}

// ===================== Class: ColourSwatch ======================
// --------------------- Construction --------------------
ColourSwatch::ColourSwatch(QWidget *parent, const char *name)
    : QOpenGLWidget(parent) {
  enabled = true;
  menu = new QMenu(this);
  menu->addAction("Edit", this, SLOT(editcol()));
  menu->addAction("Copy", this, SLOT(copy()));
  menu->addAction("Paste", this, SLOT(paste()));
  mynameis = name;
}

// -------------------- Slots -------------------
void ColourSwatch::display(GLfloat *curr, GLfloat *full) {
  currColour[0] = curr[0];
  currColour[1] = curr[1];
  currColour[2] = curr[2];
  fullColour[0] = full[0];
  fullColour[1] = full[1];
  fullColour[2] = full[2];
  update();
}

void ColourSwatch::setDisabled(bool e) { enabled = !e; }

void ColourSwatch::copy() { emit copycolour(mynameis); }

void ColourSwatch::paste() { emit pastecolour(mynameis); }

void ColourSwatch::editcol() { emit edit(); }

// -------------------- Rendering --------------------
void ColourSwatch::initializeGL() { glClearColor(0, 0, 0, 0); }

void ColourSwatch::resizeGL(int, int) {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  gluOrtho2D(0, (GLdouble)width(), 0, (GLdouble)height());

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glViewport(0, 0, (GLint)width(), (GLint)height());
}

void ColourSwatch::paintGL() {
  glClear(GL_COLOR_BUFFER_BIT);

  int line = width() / 3;
  line *= 2;

  glColor3fv(currColour);
  glBegin(GL_POLYGON);
  {
    glVertex2i(1, 1);
    glVertex2i(line - 1, 1);
    glVertex2i(line - 1, height() - 1);
    glVertex2i(1, height() - 1);
  }
  glEnd();

  glColor3fv(fullColour);
  glBegin(GL_POLYGON);
  {
    glVertex2i(line, 1);
    glVertex2i(width() - 1, 1);
    glVertex2i(width() - 1, height() - 1);
    glVertex2i(line, height() - 1);
  }
  glEnd();
}

// -------------------- Mouse Event Handling --------------------
void ColourSwatch::mousePressEvent(QMouseEvent *me) {
  if (enabled) {
    if (me->button() == Qt::LeftButton)
      emit edit();
    else if (me->button() == RightButton)
      menu->exec(QCursor::pos());
  }
}
