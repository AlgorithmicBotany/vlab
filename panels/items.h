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



#ifndef ITEMS_H
#define ITEMS_H
#include <iostream>

#include <QString>
#include <QRect>
#include <QFontMetrics>
#include <vector>
#include <cstdio>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLFunctions_4_3_Core>

#define SLIDERLENGTH 128
#define SLIDERHEIGHT 13
#define BUTTONLENGTH 100
#define BUTTONHEIGHT 24
#define LABELHEIGHT 20
#define LABELLENGTH 200

#define OFF 0
#define ON 1
#define MONOSTABLE 2

#define LEFT 0
#define CENTER 1
#define RIGHT 2

class Page;
class Item;
class Slider;
class Button;
class Group;
class Label;
class Menu;

class Panel; // defined in panel.h
class GLWidget;

enum SavingMode { CONTINUOUS, TRIGGERED, NONE };

// a base class for panel items
class Item {
public:
  virtual ~Item(){};

  inline QString getName() { return name; };
  inline QString getType() { return type; };
  inline QString getMessage() { return message; };
  inline int getValue() { return value; };
  inline QRect *getContour() { return &contour; };
  inline GLfloat *getColour1() { return colour1; };
  inline GLfloat *getColour2() { return colour2; };
  inline Panel *getPanel() { return panel; };
  inline Page *getPage() { return pg; };
  inline void setSelected(bool S) { iamselect = S; };
  inline bool isSelected() { return iamselect; };

  virtual void setSelectColour(GLfloat *);
  virtual void setName(QString nm) { name = nm; };
  virtual void setMessage(QString ms) { message = ms; };
  virtual QPoint getOrigin() { return contour.topLeft(); };
  virtual QPoint getSize() {
    return QPoint(contour.width(), contour.height());
  };
  virtual void setColour1(GLfloat *c1);
  virtual void setColour2(GLfloat *c2);
  virtual void setSize(int w, int h);
  virtual void setValue(int);
  virtual void setMode(QString);

  virtual void aligntext() { return; };
  virtual void draw(GLWidget *) { return; }
  virtual void mousePress(QPoint) { return; };
  virtual void mouseMove(QPoint) { return; };
  virtual void mouseRelease(QPoint) { return; };
  virtual void moveBy(int, int) { return; };
  virtual void snap();

  virtual void processMessage();

  inline void setSavingMode(SavingMode s) { _savingMode = s; };
  virtual void resetToDefaultValue() { return; }
  virtual void saveDefaultValue() { return; }

protected:
  QRect contour;
  GLfloat colour1[3]; // for contour
  GLfloat colour2[3]; // for button,etc.
  GLfloat selcol[3];  // for selection in edit mode
  QString name;
  QString message;
  int value;
  int defaultValue;
  QString type;
  Page *pg;
  Panel *panel;
  bool iamselect;
  QString mode;
  int width, height;
  QPoint lastclick;
  QPoint preSnapPosition;
  SavingMode _savingMode;
  bool itemMoved;
};

class Page : public Item {
public:
  Page(QFont f, Panel *pan, QAction *id = NULL, QString nm = "",
       GLfloat *c = NULL, int x = -1, int y = -1, QString mess = "p <pg#>");
  Page(Page *I);

  inline Item *getItem(int i) { return items[i]; };
  inline int numItems() { return items.size(); };
  inline QAction *getID() { return menu_id; };
  inline void setID(QAction *id) { menu_id = id; };
  inline QFont panelfont() { return font; };
  inline std::vector<Item *> getSelectedItems() { return selectedItems; };
  inline Item *getItemWithActiveDialogue() { return itemWithActiveDialogue; };
  inline void setItemWithActiveDialogue(Item *I) { itemWithActiveDialogue = I; }

  void setSelectColour(GLfloat *);
  void getSelectColour(GLfloat *);
  inline GLfloat *getSelColour() { return selcol; };
  inline GLfloat *getColour1() { return colour1; };
  void setColour1(GLfloat *c1);


  void setName(QString);
  void setMode(QString);
  void setFont(QFont);
  void addItem(Item *);
  void deleteItem(Item *I);
  void draw(GLWidget *w);
  void mousePress(QPoint);
  void shiftMousePress(QPoint);
  void mouseDoublePress(QPoint);
  void mouseMove(QPoint);
  void mouseRelease(QPoint);
  void activate();

  void setSelectedItemsFromArea(QRect area);
  void clearSelectedItems();
  void addItemToSelectedItems(Item *I);
  void removeItemFromSelectedItems(Item *I);
  void updateAlignmentButtonsEnabled();

  void alignSelected(int, QString);

  void horizontalAlignLeft();
  void horizontalAlignCenter();
  void horizontalAlignRight();

  void distributeHorizontally();

  void verticalAlignBottom();
  void verticalAlignCenter();
  void verticalAlignTop();

  void distributeVertically();
  void flipItemsVertically();

  void moveSelectedItemsRight(int);
  void moveSelectedItemsUp(int);

  void resetToDefaultValue();
  void saveDefaultValue();

private:
  void drawDragSelectionBox();
  void drawSelectedItemsBox();
  void drawItemWithActiveDialogBox();

  void removeGroupedButtonsFromSelected();
  void updateSelectedItemsRect();

  QAction *menu_id;
  std::vector<Item *> items;
  // Item(s) selected for moving
  std::vector<Item *> selectedItems;
  // Item selected for editing
  Item *itemWithActiveDialogue;
  QFont font;
  int margin;

  QPoint _dragSelectOrigin;
  QPoint _dragSelectDestination;
  bool _isDragSelecting;
  QRect _selectedItemsRect;
};

class Slider : public Item {
public:
  Slider(Page *p, Panel *pan, SavingMode savingMode, QString nm = "Slider",
         GLfloat *c1 = NULL, GLfloat *c2 = NULL, int x = 0, int y = 0,
         int minval = 0, int maxval = 10, int val = 5,
         QString mess = "d <name> %d <scale>", int w = -1, int h = -1);
  Slider(Slider *);
  ~Slider();

  inline int getMinValue() { return min; };
  inline int getMaxValue() { return max; };
  inline int getDefaultValue() { return defaultVal; };
  void setValue(int);
  void setMinValue(int);
  void setMaxValue(int);
  void setDefaultValue(int);
  void setName(QString);
  void initValue(int v, int mn, int mx, int sz = 1);
  void setMode(QString);
  void resetPalette();
  void aligntext();
  void draw(GLWidget *w);
  void mousePress(QPoint);
  void mouseMove(QPoint);
  void mouseRelease(QPoint);
  void moveBy(int dx, int dy);
  QPoint getOrigin() { return emptyContour.topLeft(); }

  void resetToDefaultValue();
  void saveDefaultValue();

private:
  void initSlider(Page *p, Panel *pan, QString nm = "Slider",
                  GLfloat *c1 = NULL, GLfloat *c2 = NULL, int x = 0, int y = 0,
                  int minval = 0, int maxval = 2, int val = 1,
                  QString mess = "d <name> %d <scale>", int w = -1, int h = -1);

  int min, max, defaultVal;
  int steps;
  int stepsize;
  int stepwidth;
  Label *titlelabel, *valuelabel;
  QRect slidebox;
  QRect emptyContour;

  bool sliding;
};

class Button : public Item {
public:
  Button(Page *p, Panel *pan, QString nm = "Button", GLfloat *c1 = NULL,
         GLfloat *c2 = NULL, int x = 0, int y = 0, int val = 0,
         QString mess = "d <name> %d <scale>", GLfloat *c3 = NULL,
         bool tricolor = false, int w = -1, int h = -1);
  Button(Button *);
  ~Button();

  inline int status() { return value; };
  inline Label *getLabel() { return label; };
  inline Group *getGroup() { return group; };
  inline GLfloat *getColour3() { return colour3; };
  inline void setTransparent(bool T) { tri = !T; };
  inline bool isTransparent() { return !tri; };
  void setName(QString);
  void setColour1(GLfloat *c1);
  void setColour3(GLfloat *c3);
  void setSize(int, int);
  void setMode(QString);
  void resetPalette();
  void aligntext();
  void draw(GLWidget *w);
  void mousePress(QPoint);
  void mouseMove(QPoint);
  void mouseRelease(QPoint);
  void moveBy(int dx, int dy);

  void toggle(); // on <-> off
  void groupIn(Group *);

  QString getExtension();
  void setDefaultValue(int type){
    defaultValue = type;
  }
  void resetToDefaultValue() {   value = defaultValue; };
  void saveDefaultValue() {   defaultValue = value; };
  QPoint getOrigin() { return buttonBox.topLeft(); }

private:
  void initButton(Page *p, Panel *pan, QString nm = "Button",
                  GLfloat *c1 = NULL, GLfloat *c2 = NULL, int x = 0, int y = 0,
                  int val = 0, QString mess = "d <name> %d <scale>",
                  GLfloat *c3 = NULL, bool tricolor = false, int w = -1,
                  int h = -1);
  void calculateButtonName(QString);
  bool isButtonNameUnique(QString);

  Group *group;
  Label *label;
  GLfloat colour3[3]; // tri-colour button
  bool tri, down;
  QRect buttonBox;
};

class Group : public Item {
public:
  Group(Page *p, Panel *pan, GLfloat *c1 = NULL, QString nm = "", int w = -1,
        int h = -1);
  Group(Group *);
  ~Group();

  inline int getNumButtons() { return buttons.size(); };
  inline Button *getButtonAt(int i) { return buttons[i]; };
  inline int getNumNames() { return names.size(); };
  inline QString getNameAt(int i) { return names[i]; };

  void addButtonName(QString);
  void addButton(Button *);
  void setButtons(std::vector<Button *>);
  void deleteButton(Button *);
  bool containsButton(Button *);
  void unset();
  void draw(GLWidget *w);
  void mousePress(QPoint);
  void mouseMove(QPoint);
  void mouseRelease(QPoint);
  void moveBy(int dx, int dy);
  void realign();

private:
  void initGroup(Page *p, Panel *pan, GLfloat *c1 = NULL, QString nm = "",
                 int w = -1, int h = -1);

  std::vector<Button *> buttons;
  std::vector<QString> names;
  int margin;
};

class Label : public Item {
public:
  Label(Page *p, Panel *pan, QString nm = "Label", GLfloat *c1 = NULL,
        int x = 0, int y = 0, int w = -1, int h = -1);
  Label(Label *);

  void setName(QString);
  void setSize(int w, int h);
  void draw(GLWidget *w);
  void aligntext();
  void mousePress(QPoint);
  void mouseMove(QPoint);
  void mouseRelease(QPoint);
  void moveBy(int dx, int dy);

private:
  void initLabel(Page *p, Panel *pan, QString nm = "Label", GLfloat *c1 = NULL,
                 int x = 0, int y = 0, int w = -1, int h = -1);

  int xras, yras;
};

// one menu for all pages
class Menu : public Item {
public:
  Menu(Panel *pan, QAction *id = NULL, QString nm = "",
       QString mess = "d <name> %d <scale>");

  inline QAction *getID() { return menu_id; };
  inline void setID(QAction *id) { menu_id = id; };

  void activate();

private:
  QAction *menu_id;
};
#endif
