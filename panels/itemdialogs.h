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



#ifndef ITEMDIALOGS_H
#define ITEMDIALOGS_H

#include <qgl.h>
#include <QDialog>
#include <QWidget>
#include <QLayout>
#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QRadioButton>
#include <QGridLayout>
#include <QCloseEvent>
#include "items.h"
#include <QColorDialog>

class Panel;

class SliderDialog : public QWidget {
  Q_OBJECT

public:
  SliderDialog(QWidget *parent = NULL, Slider *I = NULL, const char *name = 0,
               Qt::WindowFlags f = Qt::Window);

public slots:
  void setSlider(Slider *);
  void update();
  void setName();
  void setMessage();
  void setValue(int);
  void setMinValue(int);
  void setMaxValue(int);
  void setOriginX(int);
  void setOriginY(int);
  void setColour1(GLfloat *);
  void setColour2(GLfloat *);
  void colour1ButtonClicked();
  void colour2ButtonClicked();
  void colour1SelectedFromDialog(QColor);
  void colour2SelectedFromDialog(QColor);
  void colour1RejectedFromDialog();
  void colour2RejectedFromDialog();
  void setTemplateMessage1(bool);
  void setTemplateMessage2(bool);


private:
  void connectUp();
  void disconnectUp();
  void closeEvent(QCloseEvent *);
  void resizeEvent(QResizeEvent *event);
  void moveEvent(QMoveEvent *event);

  Slider *slider;
  QGridLayout *G;
  QLineEdit *Name;
  QLineEdit *message;
  QSpinBox *val, *min, *max;
  QSpinBox *originX, *originY;
  QPushButton *colour1Button, *colour2Button;
  QPushButton *closebutton;
  bool connected;
  // In case the user presses 'Cancel' in the colour picker dialogue
  QColor previousColour1, previousColour2;
  // steps,stepsize,width,height
  QRadioButton *messageTemplate1;
  QRadioButton *messageTemplate2;
  bool open;
};

class ButtonDialog : public QWidget {
  Q_OBJECT

public:
  ButtonDialog(QWidget *parent = NULL, Button *I = NULL, const char *name = 0,
               Qt::WindowFlags f = Qt::Window);

  bool isButtonNameUnique(QString);

public slots:
  void setButton(Button *);
  void update();

private slots:
  void setName();
  void setMessage();
  void setValue(QListWidgetItem *);
  void setOriginX(int);
  void setOriginY(int);
  void setClear(bool);
  void setTemplateMessage1(bool);
  void setTemplateMessage2(bool);
  void setColour1(GLfloat *);
  void setColour2(GLfloat *);
  void setColour3(GLfloat *);
  void colour1ButtonClicked();
  void colour2ButtonClicked();
  void colour3ButtonClicked();
  void colour1SelectedFromDialog(QColor);
  void colour2SelectedFromDialog(QColor);
  void colour3SelectedFromDialog(QColor);
  void colour1RejectedFromDialog();
  void colour2RejectedFromDialog();
  void colour3RejectedFromDialog();

private:
  void connectUp();
  void disconnectUp();
  void closeEvent(QCloseEvent *);
  void calculateButtonNameAndExtension(QString *, QString *);
  void resizeEvent(QResizeEvent *event);
  void moveEvent(QMoveEvent *event);

  Button *button;
  QGridLayout *G;
  QLineEdit *Name;
  QLineEdit *message;
  QListWidget *val;
  QSpinBox *originX, *originY;
  QPushButton *colour1Button, *colour2Button, *colour3Button;
  QPushButton *closebutton;
  QRadioButton *clear;
  QRadioButton *messageTemplate1;
  QRadioButton *messageTemplate2;
   

  bool connected;
  // In case the user presses 'Cancel' in the colour picker dialogue
  QColor previousColour1, previousColour2, previousColour3;
  // width, height

  // This label will display the generated extension to the button's name in
  // case what the user entered as a name is not unique This is so that two
  // buttons with identical names can be distinct, so that button groups don't
  // break
  QLabel *nameExtension[2];
};

class LabelDialog : public QWidget {
  Q_OBJECT
public:
  LabelDialog(QWidget *parent = NULL, Label *I = NULL, const char *name = 0,
              Qt::WindowFlags f = Qt::Window);

public slots:
  void setLabel(Label *);
  void update();
  void setName();
  void setOriginX(int);
  void setOriginY(int);
  void setColour1(GLfloat *);
  void colour1ButtonClicked();
  void colour1SelectedFromDialog(QColor);
  void colour1RejectedFromDialog();

private:
  void connectUp();
  void disconnectUp();
  void closeEvent(QCloseEvent *);
  void resizeEvent(QResizeEvent *event);
  void moveEvent(QMoveEvent *event);

  Label *label;
  QGridLayout *G;
  QLineEdit *Name;
  QSpinBox *originX, *originY;
  QPushButton *colour1Button;
  QPushButton *closebutton;
  bool connected;
  // In case the user presses 'Cancel' in the colour picker dialogue
  QColor previousColour1;
  // width, height
};

class GroupDialog : public QWidget {
  Q_OBJECT
public:
  GroupDialog(QWidget *parent = NULL, Group *I = NULL, const char *name = 0,
              Qt::WindowFlags f = Qt::Window);

public slots:
  void setGroup(Group *);
  void update();
  void setOriginX(int);
  void setOriginY(int);
  void setColour1(GLfloat *);
  void colour1ButtonClicked();
  void colour1SelectedFromDialog(QColor);
  void colour1RejectedFromDialog();
  void removeSelect();

private:
  void connectUp();
  void disconnectUp();
  void closeEvent(QCloseEvent *);
  void resizeEvent(QResizeEvent *event);
  void moveEvent(QMoveEvent *event);

  Group *group;
  QGridLayout *G;
  QSpinBox *originX, *originY;
  QPushButton *colour1Button;
  QSpinBox *margin;
  QPushButton *closebutton;
  bool connected;
  QListWidget *buttonlist;
  // In case the user presses 'Cancel' in the colour picker dialogue
  QColor previousColour1;
};

class MenuDialog : public QWidget {
  Q_OBJECT

public:
  MenuDialog(QWidget *parent = NULL, Menu *I = NULL, const char *name = 0,
             Qt::WindowFlags f = Qt::Window);

public slots:
  void setMenu(Menu *);
  void update();
  void setName();
  void setMessage();
  void setTemplateMessage1(bool);
  void setTemplateMessage2(bool);


private:
  void connectUp();
  void disconnectUp();
  void closeEvent(QCloseEvent *);
  void resizeEvent(QResizeEvent *event);
  void moveEvent(QMoveEvent *event);

  Menu *menu;
  QGridLayout *G;
  QLineEdit *Name;
  QLineEdit *message;
  QPushButton *closebutton, *delbutton;
  bool connected;
  QRadioButton *messageTemplate1;
  QRadioButton *messageTemplate2;

};

class PageDialog : public QWidget {
  Q_OBJECT

public:
  PageDialog(QWidget *parent = NULL, Page *I = NULL, const char *name = 0,
             Qt::WindowFlags f = Qt::Window);

public slots:
  void setPage(Page *);
  void update();
  void setName();
  void setMessage();
  void setOriginX(int);
  void setOriginY(int);
  void setColour1(GLfloat *);
  void colour1ButtonClicked();
  void colour1SelectedFromDialog(QColor);
  void colour1RejectedFromDialog();

private:
  void connectUp();
  void disconnectUp();
  void closeEvent(QCloseEvent *);
  void resizeEvent(QResizeEvent *event);
  void moveEvent(QMoveEvent *event);
  Page *page;
  QGridLayout *G;
  QLineEdit *Name;
  QLineEdit *message;
  QSpinBox *originX, *originY;
  QPushButton *colour1Button;
  QPushButton *closebutton;
  bool connected;
  // In case the user presses 'Cancel' in the colour picker dialogue
  QColor previousColour1;
};

#endif
