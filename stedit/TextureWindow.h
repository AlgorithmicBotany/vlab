/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#pragma once

#include <QWidget>
#include <QDialog>
#include <QMenuBar>
#include <QMenu>
#include <QLabel>
#include <QAction>

#include <fstream>
#include <stdlib.h>
#include "TextureEditor.h"
#include "TextureEditorPreferencesDialog.h"
#include "QImageFormatStrings.h"
#include "resources.h"
#include "Globals.h"
#ifdef __APPLE__
#include "platform.h"
#endif
using namespace std;

class TextureEditor;

class TextureWindow : public QWidget {
  Q_OBJECT

public:
  TextureWindow(string textureName = "", SavingMode savingMode = OFF);
  ~TextureWindow();

  QSize sizeHint() const;
  void setSavingMode(SavingMode savingMode) { _savingMode = savingMode; }

signals:
  void textureEditorClosed(bool saved);
  void passTexture(QImage image);
  void textureLoaded(string filename);
  void continuousSave();
  void updatingContMode(SavingMode);

public slots:
  void enableUndo(bool value);
  void enableRedo(bool value);
  void enableRevert(bool value);
  void newFile();
  void initEditor();
  void useImage(string filename);
  void useImage(QImage image);
  void updateContinuousMode(SavingMode);
  void load();

private slots:
  void pdfHelp();
  void quickHelp();
  void help();
  void loadProject();
  void save();
  bool saveProject();
  void saveAs();
  bool saveProjectAs();
  void revert();
  void closeEvent(QCloseEvent *ce);
  void undo();
  void redo();
  void resizeTexture();
  void capture();
  void reset();
  void preferences();

  void changeBgColour();
  void changeBgDefault();
  void changeBgBlack();
  void changeBgWhite();
  void changePointColour();
  void changePointDefault();
  void changePointRed();
  void changePointGreen();
  void changePointBlack();
  void changePointWhite();
  void changeLineColour();
  void changeLineDefault();
  void changeLineRed();
  void changeLineGreen();
  void changeLineBlack();
  void changeLineWhite();

  void resetColours();
  void showHidePoints(bool value);
  void showHideLines(bool value);
  void resetView();
  void changePointSize();
  void changeLineWidth();
  void showControlsHelp();
  void showAbout();
  void menuize(const QPoint &pos);
  void updateActiveFile(string str);
  void readConfig();

  void ContinuousSavingMode();
  void TriggeredSavingMode();
  void ModeOff();
  void ContinuousModeMenu();
  void TriggeredModeMenu();
  void OffModeMenu();

private:
  QColor defaultBgColour;
  QColor defaultPointColour;
  QColor defaultLineColour;

  TextureEditor *textureEditor;

  // Menus
  QMenuBar *menuBar;
  QMenu *fileMenu;
  QMenu *textureMenu;
  QMenu *projectMenu;
  QMenu *editMenu;
  QMenu *viewMenu;
  QMenu *preferencesMenu;
  QMenu *lineColourMenu;
  QMenu *pointColourMenu;
  QMenu *bgColourMenu;
  QMenu *colourMenu;
  QMenu *resizeMenu;
  QMenu *showHideMenu;
  QMenu *helpMenu;

  // Actions that can be disabled
  QAction *undoAct;
  QAction *redoAct;
  QAction *revertAct;
  QAction *showPointsAct;
  QAction *showLinesAct;

  QAction *_savingContinu_act;
  QAction *_savingTriggered_act;
  QAction *_savingMenu_act;

  QAction *_savingContinuP_act;
  QAction *_savingTriggeredP_act;
  QAction *_savingMenuP_act;

  QString loadFormats; // QString of image formats that can be loaded
  QString saveFormats; // QString of image formats that can be saved
  QString textureDir;  // Directory to look for textures in
  QString
      projectFormats; // QString of project formats that can be loadedand saved
  QString projectDir; // Directory to look for projects in

  SavingMode _savingMode;
};
