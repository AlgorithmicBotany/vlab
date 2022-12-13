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



#ifndef PANEL_H
#define PANEL_H

#include <vector>
#include <cstdlib>
#include <iostream>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <QFileDialog>

#include <qwidget.h>
#include <QScrollArea>
#include <QPoint>
#include <QSize>
#include <QString>
#include <QFile>
#include <QApplication>
#include <QMainWindow>
#include <QMenu>
#include <QTextStream>
#include <QMessageBox>
#include <QFileDialog>
#include <QFontDialog>
#include <QFont>
#include <QCloseEvent>
#include <QMouseEvent>
#include <QDesktopWidget>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLFunctions_4_3_Core>
#include <QOpenGLBuffer>
#include "glwidget.h"
#include "items.h"

#define MINHIGH 0
#define MINLONG 0

class PanelEdit;
class SliderDialog;
class ButtonDialog;
class LabelDialog;
class GroupDialog;
class MenuDialog;
class PageDialog;
class QShowEvent;
class GLWidget;

// The Main Panel Widget
// has a vector of panel pages
class Panel : public QMainWindow {
  Q_OBJECT
public:
  Panel(int, char **, SavingMode savingMode = NONE);
  bool cantreadfile(){ return _cantread;}

  inline int getDesktopWidth() { return desktop->width(); };
  inline int getDesktopHeight() { return desktop->height(); };
  inline QString getPanelName() { return name; };
  inline int getPanelWidth() { return size.width(); };
  inline int getPanelHeight() { return size.height(); };
  GLfloat *getBGColour();
  inline bool isSnapping() { return snapon; };
  inline bool usingWindowsFormat() { return loadwindowsformat; };
  inline int getGridSize() {
    if (snapon)
      return gridsize;
    return 1;
  };
  inline int getDefaultGridSize() { return gridsize; };
  inline QFont getFont() { return font; };
  inline Page *getCurrPage() { return currpage; };
  QString panelName() { return name; };
  inline bool getModified() { return modified; };
  inline void setModified() { modified = true; };
  inline PanelEdit *getPanelEditor() { return editor; };
  inline QSize *getMainWindowSize() { return mainWindowSize; }
  inline QPoint *getMainWindowLocation() { return mainWindowLocation; }
  inline QScreen *getScreen() { return screen; }
  void resizeItemDialogs(QSize);
  void moveItemDialogs(QPoint);
  void resize(int w, int h) { size = QSize(w, h); }

  bool thereAreOpenDialogs();
  QString getMode() { return mode; }
  void setMode(QString m) { mode = m; }

  Page *getCurrentPage() { return currpage; }
  GLfloat *getGridVec() { return gridvec; }
  GLfloat *getHighVec() { return highvec; }

  void startMenu() { menu->exec(QCursor::pos()); }
  void keyPressed(QKeyEvent *);

  void mouseClicked(QMouseEvent *);
  void mouseDoubleClicked(QMouseEvent *);
  void mouseMoved(QMouseEvent *);
  void mouseReleased(QMouseEvent *);

  GLWidget *getGLWidget() { return _glWidget; }

  void closeEvent(QCloseEvent *ce);

  void update();

public slots:
  void setEdited(bool);
  void setPanelName(QString s);
  void setPanelWidth(int i);
  void setPanelHeight(int i);
  void setDoneEditing();
  void newItem(int);
  void HalignSelection(int);
  void horizontalDistributionSelected();
  void ValignSelection(int);
  void verticalDistributionSelected();
  void flipItemsVerticallySelected();
  void setBGColour(GLfloat *);
  void setupMenu();
  void deleteMenuItem();
  void deletePage();
  void deleteSelect();
  void cloneSelect();
  void copySelect();
  void pasteCopy();
  void editSelect();
  void editPage();
  void toggleSnap();
  void preferences();
  void itemDefaults();
  void setfont();
  void fontSelectedFromDialog(QFont);
  void fontRejectedFromDialog();
  void loadconfig();
  void saveconfig();
  void saveDefault();
  void reread();
  void restore();
  void reopen();
  void broadcast();
  void broadcastall();
  void load();
  void save();
  void saveslot(QString);
  void saveas();
  void pagemenuHandler(QAction *);
  void messmenuHandler(QAction *);
  void removeFromGroup(Button *);
  void help();
  void pdfHelp();
  void quickHelp();
  void about();
  void nothing();
  void execPanel();
  void editPanel();

  void ContinuousSavingMode();
  void TriggeredSavingMode();
  void ModeOff();
  void setDeviceRatio(int ratio) { deviceRatio = ratio; }
  int getDeviceRatio() { return deviceRatio; }
  void updateColourScheme();
  void updateOpenDialogs();

protected:
  bool parseargs(int, char **);
  QString read();
  QString write();
  int scrap();
  bool loadpanel(QString, int x = -1, int y = -1);
  bool isWindowsLoadFormat();
  void closeOpenDialogs();
  void setDialog();
  void moveSelectedItemsRight(int);
  void moveSelectedItemsUp(int);

  void showEvent(QShowEvent *ev);
  bool eventFilter(QObject *, QEvent *);

protected:
  std::vector<Page *> pages;
  Page *currpage;
  QMenu *menu, *pagemenu, *messmenu, *editmenu, *filemenu;
  std::vector<Menu *> menuitems;
  int charwidth, charheight;
  QString name, target;
  int bgcolour;
  Menu *selectmenu;
  QSize size;
  QPoint position;
  QDesktopWidget *desktop;
  QFont font,
      previousFont; // for when user cancels a font from dialog, we revert;
  QFile *file, *configfile;
  bool first, menufirst, modified, set;
  QAction *_save, *_edit, *_exec, *_load, *_saveas, *_restore, *_broad,
      *_broadall, *_reread, *_options, *_snap, *_delete, *_clone, *_edititem,
    *_paste, *_copy, *_editpage, *_filestuff, *_saveDefault;
  PanelEdit *editor;
  QString mode;
  bool fontset;
  SliderDialog *sliderdialog;
  ButtonDialog *buttondialog;
  LabelDialog *labeldialog;
  GroupDialog *groupdialog;
  MenuDialog *menudialog;
  PageDialog *pagedialog;
  bool adding, snapon, loadwindowsformat, savewindowsformat;
  int gridsize;
  int windowsxoffset, windowsyoffset;
  GLfloat bgColour[3];
  Label *lastlabel;
  Slider *lastslider;
  Button *lastbutton;
  Group *lastgroup;
  GLfloat gridvec[3];
  GLfloat highvec[3];

  SavingMode _savingMode;
  QAction *_savingContinu_act;
  QAction *_savingTriggered_act;
  QAction *_savingMenu_act;

  // Will store cmd line size and location params
  QSize *mainWindowSize;
  QPoint *mainWindowLocation;

  QScreen *screen;
  int deviceRatio;

  QMenu *pContextMnu;
  std::vector<Item *> copiedItems;

  GLWidget *_glWidget;
  bool _cantread;
};

#endif
