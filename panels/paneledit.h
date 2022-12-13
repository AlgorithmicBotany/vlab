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



#ifndef PANELEDIT_H
#define PANELEDIT_H

#include <QSpinBox>
#include <QLineEdit>
#include <QLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QGroupBox>
#include <QCloseEvent>
#include "panel.h"

#ifdef __APPLE__
#include <GLUT/glut.h>
#include <OpenGL/glu.h>
#else
#include <GL/glut.h>
#include <GL/glu.h>
#endif
#include <QWidget>
#include <QDialog>
#include <QCursor>
#include <QMouseEvent>
#include <QGridLayout>
#include <QColorDialog>
#include <iostream>

class Panel;

class PanelEdit : public QWidget {
  Q_OBJECT

public:
  PanelEdit(Panel *editMe = 0);

public:

public slots:
  void updatePanel();
  void editPanel();
  void setPanelName();
  void loadDefaults();
  void saveEditPanel();
  void closeEditPanel();

  void buttonPressed();
  void HalignSelection();
  void ValignSelection();

  void setAddButtonGroupEnabled(bool b);
  void setAlignmentButttonsEnabled(bool b);
  void setDistributionButtonsEnabled(bool b);

  void updatePanelWidthFromSpinbox();
  void updatePanelHeightFromSpinbox();

  void setBackgroundColour(GLfloat *col);
  void backgroundColourButtonClicked();
  void backgroundColourSelectedFromDialog(QColor);
  void backgroundColourRejectedFromDialog();

private:
  void disconnectAll();
  void connectAll();
  void closeEvent(QCloseEvent *);
  bool readDefaults(QFile *);

  Panel *panel;
  QLineEdit *name;
  QLabel *colours;
  QPushButton *fontset;
  QLabel *fontshow;
  QSpinBox *panelwidth, *panelheight;
  QPushButton *sv, *cl, *button[6], *Halign[3], *Valign[3], *Hspace, *Vspace,
      *backgroundColourButton, *vFlip;
  // In case the user presses 'Cancel' in the background colour picker dialogue
  QColor previousBackgroundColour;
  QGroupBox *itembuttons, *Halignbuttons, *Valignbuttons;
  QLabel *indexlabel;
  QColorDialog *dialog;
};

#endif
