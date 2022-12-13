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



#include "paneledit.h"
#include "panel.h"
// Added by qt3to4:
#include <QGridLayout>
#include <QBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QCloseEvent>
using namespace Qt;

PanelEdit::PanelEdit(Panel *editMe) : QWidget(0, 0) {
  // This attribute will make this window close when the main window is closed
  setAttribute(Qt::WA_QuitOnClose, false);

  panel = editMe;

  QBoxLayout *B = new QVBoxLayout(this);


  QFrame *ff = new QFrame(this);
  B->addWidget(ff, 5);

  QGridLayout *GG = new QGridLayout(ff);
  GG->setVerticalSpacing(0);
  itembuttons = new QGroupBox("Add item to panel", this);
  button[0] = new QPushButton("Slider");
  button[1] = new QPushButton("Button");
  button[2] = new QPushButton("Label");
  button[3] = new QPushButton("Button group");
  button[3]->setEnabled(false);
  button[4] = new QPushButton("Menu item");
  button[5] = new QPushButton("Page");
  QGridLayout *gridLayout = new QGridLayout;
  for (int j = 0; j < 2; ++j) {
    for (int i = 0; i < 3; i++) {
      button[3 * j + i]->setAutoDefault(false);
      gridLayout->addWidget(button[3 * j + i], i, j);
    }
  }
  itembuttons->setLayout(gridLayout);
  GG->addWidget(itembuttons, 0, 0, 3, 3);

 Halignbuttons = new QGroupBox("Horizontal alignment", this);
  Halign[0] = new QPushButton("Left");
  Halign[1] = new QPushButton("Center");
  Halign[2] = new QPushButton("Right");

  // icons
  Halign[0]->setIcon(QIcon(":/icons/hLeft.png"));
  Halign[0]->setIconSize(QSize(32, 32));
  Halign[1]->setIcon(QIcon(":/icons/hCenter.png"));
  Halign[1]->setIconSize(QSize(32, 32));
  Halign[2]->setIcon(QIcon(":/icons/hRight.png"));
  Halign[2]->setIconSize(QSize(32, 32));

  QHBoxLayout *hbox = new QHBoxLayout;

  for (int i = 0; i < 3; i++) {
    Halign[i]->setAutoDefault(false);
    Halign[i]->setEnabled(false);
    hbox->addWidget(Halign[i]);
  }
  Halignbuttons->setLayout(hbox);
  GG->addWidget(Halignbuttons, 6, 0, 7, 3);

  Hspace = new QPushButton("Distribute horizontally");

  Hspace->setIcon(QIcon(":/icons/dHorizontal.png"));
  Hspace->setIconSize(QSize(32, 32));

  Hspace->setEnabled(false);
  GG->addWidget(Hspace, 13, 1);

  Valignbuttons = new QGroupBox("Vertical alignment", this);
  Valign[0] = new QPushButton("Bottom");
  Valign[1] = new QPushButton("Center");
  Valign[2] = new QPushButton("Top");

  // icons
  Valign[0]->setIcon(QIcon(":/icons/vBottom.png"));
  Valign[0]->setIconSize(QSize(32, 32));
  Valign[1]->setIcon(QIcon(":/icons/vCenter.png"));
  Valign[1]->setIconSize(QSize(32, 32));
  Valign[2]->setIcon(QIcon(":/icons/vTop.png"));
  Valign[2]->setIconSize(QSize(32, 32));

  QHBoxLayout *vbox = new QHBoxLayout;

  for (int i = 0; i < 3; i++) {
    Valign[i]->setAutoDefault(false);
    Valign[i]->setEnabled(false);
    vbox->addWidget(Valign[i]);
  }
  Valignbuttons->setLayout(vbox);

  GG->addWidget(Valignbuttons, 14, 0, 15, 3);

  Vspace = new QPushButton("Distribute vertically");

  Vspace->setIcon(QIcon(":/icons/dVertical.png"));
  Vspace->setIconSize(QSize(32, 32));

  Vspace->setEnabled(false);
  GG->addWidget(Vspace, 29, 1);

  vFlip = new QPushButton("Flip items vertically");
  GG->addWidget(vFlip, 30, 1);

  QFrame *f = new QFrame(this);
  B->addWidget(f, 1);

  QGridLayout *G = new QGridLayout(f);
  G->addItem(new QSpacerItem(0, 0), 0, 0);
  QLabel *nameLabel = new QLabel("Name: ", f);
  name = new QLineEdit(panel->getPanelName(), f);
  G->addWidget(nameLabel, 1, 0);
  G->addWidget(name, 1, 1);

  for (int i = 1; i < 6; i++)
    G->addItem(new QSpacerItem(0, name->height()), i, 0);

  QLabel *backgroundColourLabel = new QLabel("Background colour: ", f);

  backgroundColourButton = new QPushButton("", this);
  GLfloat *bgCol = panel->getBGColour();
  QColor c = QColor(bgCol[0] * 255.0, bgCol[1] * 255.0, bgCol[2] * 255.0);
  if (c.isValid()) {
    QString qss = QString("background-color: %1").arg(c.name());
    backgroundColourButton->setStyleSheet(qss);
  }

  G->addWidget(backgroundColourLabel, 1, 2, 1, 2);
  G->addWidget(backgroundColourButton, 1, 4);

  QLabel *fontLabel = new QLabel("Font: ", f);
  fontshow = new QLabel("default", f);
  fontshow->setStyleSheet("QLabel {background-color: white; color : grey;}");
  fontset = new QPushButton("Set", f);

  G->addWidget(fontLabel, 2, 0);
  G->addWidget(fontshow, 2, 1, 1, 3);
  G->addWidget(fontset, 2, 4);

  QLabel *widthLabel = new QLabel("Width: ", f);
  QLabel *heightLabel = new QLabel("Height: ", f);

  panelwidth = new QSpinBox(f);
  panelwidth->setMinimum(0);
  panelwidth->setMaximum(panel->getDesktopWidth());
  panelwidth->setSingleStep(1);
  panelheight = new QSpinBox(f);
  panelheight->setMinimum(0);
  panelheight->setMaximum(panel->getDesktopWidth());
  panelheight->setSingleStep(1);

  G->addWidget(widthLabel, 3, 0);
  G->addWidget(panelwidth, 3, 1);
  G->addWidget(heightLabel, 3, 2);
  G->addWidget(panelheight, 3, 3, 1, 1);

  sv = new QPushButton("Save");
  G->addWidget(sv, 5, 0);
  cl = new QPushButton("Close");
  G->addWidget(cl, 5, 1);

  this->setWindowTitle("Panel editor");

  resize(100, 100);
  hide();
}

void PanelEdit::buttonPressed() {
  QPushButton *btn_pressed = static_cast<QPushButton *>(sender());
  for (int i = 0; i < 6; ++i) {
    if (btn_pressed == button[i]) {
      panel->newItem(i);
    }
  }
  panel->getGLWidget()->update();
  panel->setModified();
}

void PanelEdit::HalignSelection() {
  QPushButton *btn_pressed = static_cast<QPushButton *>(sender());
  for (int i = 0; i < 3; ++i) {
    if (btn_pressed == Halign[i]) {
      panel->HalignSelection(i);
    }
  }
  panel->setModified();
}

void PanelEdit::ValignSelection() {
  QPushButton *btn_pressed = static_cast<QPushButton *>(sender());
  for (int i = 0; i < 3; ++i) {
    if (btn_pressed == Valign[i]) {
      panel->ValignSelection(i);
    }
  }
  panel->setModified();
}

void PanelEdit::connectAll() {
  connect(sv, SIGNAL(released()), this, SLOT(saveEditPanel()));
  connect(cl, SIGNAL(released()), this, SLOT(closeEditPanel()));
  for (int i = 0; i < 6; ++i)
    connect(button[i], SIGNAL(released()), this, SLOT(buttonPressed()));
  for (int i = 0; i < 3; ++i)
    connect(Halign[i], SIGNAL(released()), this, SLOT(HalignSelection()));
  for (int i = 0; i < 3; ++i)
    connect(Valign[i], SIGNAL(released()), this, SLOT(ValignSelection()));
  connect(name, SIGNAL(textChanged(const QString &)), this,
          SLOT(setPanelName()));
  connect(panelwidth, SIGNAL(valueChanged(int)), this,
          SLOT(updatePanelWidthFromSpinbox()));
  connect(panelheight, SIGNAL(valueChanged(int)), this,
          SLOT(updatePanelHeightFromSpinbox()));
  connect(fontset, SIGNAL(released()), panel, SLOT(setfont()));
  connect(backgroundColourButton, SIGNAL(clicked()), this,
          SLOT(backgroundColourButtonClicked()));
  connect(Hspace, SIGNAL(released()), panel,
          SLOT(horizontalDistributionSelected()));
  connect(Vspace, SIGNAL(released()), panel,
          SLOT(verticalDistributionSelected()));
  connect(vFlip, SIGNAL(released()), panel,
          SLOT(flipItemsVerticallySelected()));
}

void PanelEdit::disconnectAll() {
  disconnect(sv, SIGNAL(released()), this, SLOT(saveEditPanel()));
  disconnect(cl, SIGNAL(released()), this, SLOT(closeEditPanel()));
  for (int i = 0; i < 6; ++i)
    disconnect(button[i], SIGNAL(released()), this, SLOT(buttonPressed()));
  for (int i = 0; i < 3; ++i)
    disconnect(Halign[i], SIGNAL(released()), this, SLOT(HalignSelection()));
  for (int i = 0; i < 3; ++i)
    disconnect(Valign[i], SIGNAL(released()), this, SLOT(ValignSelection()));
  disconnect(name, SIGNAL(textChanged(const QString &)), this,
             SLOT(setPanelName()));
  disconnect(panelwidth, SIGNAL(valueChanged(int)), this,
             SLOT(updatePanelWidthFromSpinbox()));
  disconnect(panelheight, SIGNAL(valueChanged(int)), this,
             SLOT(updatePanelHeightFromSpinbox()));
  disconnect(fontset, SIGNAL(released()), panel, SLOT(setfont()));
  disconnect(backgroundColourButton, SIGNAL(clicked()), this,
             SLOT(backgroundColourButtonClicked()));
  disconnect(Hspace, SIGNAL(released()), panel,
             SLOT(horizontalDistributionSelected()));
  disconnect(Vspace, SIGNAL(released()), panel,
             SLOT(verticalDistributionSelected()));
  disconnect(vFlip, SIGNAL(released()), panel,
             SLOT(flipItemsVerticallySelected()));
}

void PanelEdit::updatePanel() {
  disconnectAll();

  // Update name field
  name->setText(panel->getPanelName());

  // Update colour field
  GLfloat *bgCol = panel->getBGColour();
  QColor c = QColor(bgCol[0] * 255.0, bgCol[1] * 255.0, bgCol[2] * 255.0);
  if (c.isValid()) {
    QString qss = QString("background-color: %1").arg(c.name());
    backgroundColourButton->setStyleSheet(qss);
  }

  // Update dimensions fields
  panelwidth->setValue(panel->getPanelWidth());
  panelheight->setValue(panel->getPanelHeight());

  // Update font field
  fontshow->setText(" " + panel->getFont().family() + ", " +
                    QString::number(panel->getFont().pointSize()) + "pt");

  connectAll();
}

void PanelEdit::editPanel() {
  update();
  show();
}

void PanelEdit::setPanelName() {
  panel->setPanelName(name->text());
  panel->setModified();
}

void PanelEdit::loadDefaults() {
  panel->getPanelEditor()->setAlignmentButttonsEnabled(false);
  panel->getPanelEditor()->setDistributionButtonsEnabled(false);
  panel->getPanelEditor()->setAddButtonGroupEnabled(false);
  panel->getGLWidget()->update();
  panel->setModified();
}

void PanelEdit::closeEvent(QCloseEvent *ce) {

  if (panel->getModified()) {
    QMessageBox msgBox;
    //msgBox.setText("The panel has unsaved changes");
    /*
    msgBox.setInformativeText("Do you want to save your changes?");
    msgBox.setStandardButtons(QMessageBox::Yes |
			      QMessageBox::No |
                              QMessageBox::Cancel);
    
    msgBox.setDefaultButton(QMessageBox::Yes);
    int ret = msgBox.exec();
    */
    QMessageBox::StandardButton ret;
    ret= QMessageBox::question(this, "Test", "Do you want to save your changes?",
                                QMessageBox::Yes|QMessageBox::No |
                              QMessageBox::Cancel);
    switch (ret) {
    case QMessageBox::Yes:
      // Save was clicked
      saveEditPanel();
      break;
    case QMessageBox::No:
      // Don't Save was clicked
      break;
    case QMessageBox::Cancel:
      // Cancel was clicked
      ce->ignore();
      return;
    default:
      // should never be reached
      break;
    }
  }
  panel->setDoneEditing();
  panel->getGLWidget()->update();
}

void PanelEdit::saveEditPanel() { panel->save(); }

void PanelEdit::closeEditPanel() { this->close(); }

void PanelEdit::setAddButtonGroupEnabled(bool b) { button[3]->setEnabled(b); }
void PanelEdit::setAlignmentButttonsEnabled(bool b) {
  for (int i = 0; i < 3; i++) {
    Halign[i]->setEnabled(b);
    Valign[i]->setEnabled(b);
  }
}

void PanelEdit::setDistributionButtonsEnabled(bool b) {
  Hspace->setEnabled(b);
  Vspace->setEnabled(b);
}

void PanelEdit::updatePanelWidthFromSpinbox() {
  panel->setPanelWidth(panelwidth->value());
  QOpenGLWidget *w = panel->getGLWidget();
  w->resize(panelwidth->value(), w->height());
  //
  panel->getGLWidget()->update();
}

void PanelEdit::updatePanelHeightFromSpinbox() {
  panel->setPanelHeight(panelheight->value());
  QOpenGLWidget *w = panel->getGLWidget();
  w->resize(w->width(), panelheight->value());

  panel->getGLWidget()->update();
}

void PanelEdit::setBackgroundColour(GLfloat *col) {
  if (panel) {
    QColor c = QColor((int)(col[0] * 255.0), (int)(col[1] * 255.0),
                      (int)(col[2] * 255.0));
    if (c.isValid()) {
      QString qss = QString("background-color: %1").arg(c.name());
      backgroundColourButton->setStyleSheet(qss);
    }

    panel->setBGColour(col);
    panel->setEdited(true);
  }
  panel->getGLWidget()->update();
}

void PanelEdit::backgroundColourButtonClicked() {
  GLfloat *bgCol = panel->getBGColour();
  int currRed = int(bgCol[0] * 255.0);
  int currGreen = int(bgCol[1] * 255.0);
  int currBlue = int(bgCol[2] * 255.0);

  previousBackgroundColour = QColor(currRed, currGreen, currBlue);

  dialog = new QColorDialog(QColor(currRed, currGreen, currBlue));
  connect(dialog, SIGNAL(currentColorChanged(QColor)), this,
          SLOT(backgroundColourSelectedFromDialog(QColor)));
  connect(dialog, SIGNAL(rejected()), this,
          SLOT(backgroundColourRejectedFromDialog()));

  dialog->show();
}

void PanelEdit::backgroundColourSelectedFromDialog(QColor ret) {
  GLfloat c[3];

  if (ret.isValid()) {
    c[0] = float(ret.red()) / 255.0;
    c[1] = float(ret.green()) / 255.0;
    c[2] = float(ret.blue()) / 255.0;

    setBackgroundColour(c);
  }
  panel->getGLWidget()->update();
}

void PanelEdit::backgroundColourRejectedFromDialog() {
  GLfloat c[3];

  if (previousBackgroundColour.isValid()) {
    c[0] = float(previousBackgroundColour.red()) / 255.0;
    c[1] = float(previousBackgroundColour.green()) / 255.0;
    c[2] = float(previousBackgroundColour.blue()) / 255.0;

    setBackgroundColour(c);
  }
}
