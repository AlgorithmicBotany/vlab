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
#include <QListWidget>
#include <QListWidgetItem>
#include <fstream>
#include <stdlib.h>
#include "BezierEditor.h"
#include "BezierEditorPreferencesDialog.h"
#include "PointPicker.h"
#include "TurtleParametersWindow.h"
#include "AdjacenciesWindow.h"
#include "TranslateWindow.h"
#include "RotateWindow.h"
#include "ScaleWindow.h"
#include "QImageFormatStrings.h"
#include "resources.h"
#ifdef __APPLE__
#include "platform.h"
#endif

using namespace std;

class BezierEditor;

class BezierWindow : public QWidget {
  Q_OBJECT

public:
  BezierWindow(bool extendedFormatFlag, SavingMode savingMode = OFF, string surfaceName = "",
               string textureName = "", bool showTextureWindow = false);
  ~BezierWindow();

  QSize sizeHint() const;
  SavingMode getSavingMode() { return _savingMode; }

public slots:
  void setContactCentered(bool value);
  void enableUndo(bool value);
  void enableRedo(bool value);
  void enableRevert(bool value);
  void updateContinuousMode(SavingMode);

signals:
  void continuousSave();
  void updatingContMode(SavingMode);

private slots:
  void pdfHelp();
  void quickHelp();
  void help();
  void newFile();
  void loadTexture();
  void loadSurface();
  bool saveSurface();
  bool saveSurfaceAs();
  void undo();
  void redo();
  void revert();
  void editTexture();
  void textureEditorClosed(bool saved);
  void closeEvent(QCloseEvent *ce);
  void changeBgColour();
  void changePointColour();
  void changeSelectedPointColour();
  void changeLineColour();
  void changeWireframeColour();
  void changeVectorColour();
  void changePatchColour();
  void changeSelectedPatchColour();
  void resetColours();
  void showHidePoints(bool value);
  void showHideLines(bool value);
  void showHideSurface(bool value);
  void showHideVectors(bool value);
  void resetView();
  void setWireframe(bool value);
  void changePointSize();
  void changeLineWidth();
  void changeWireframeWidth();
  void changeSubdivisionLevel();
  void centerCamera();
  void centerAtContact(bool value);
  void editTurtleParameters();
  void editAdjacencies();
  void flipPatchHorizontal();
  void flipPatchVertical();
  void flipPatchDepth();
  void translatePatch();
  void rotatePatch();
  void scalePatch();
  void showControlsHelp();
  void showAbout();
  void menuize(const QPoint &pos);
  void menuizeAddButton(const QPoint &pos);
  void addPatchToList(string name);
  void addNewSquarePatch();
  void addNewTrianglePatch();
  void deleteSelectedPatch();
  void duplicateSelectedPatch();
  void clearPatchList();
  void selectPatch();
  void selectPatchInList(int index);
  void changePatchName(QListWidgetItem *item);
  void changePatchVisibility();
  void preferences();
  void readConfig();

  void point1Toggled(bool checked);
  void point2Toggled(bool checked);
  void point3Toggled(bool checked);
  void point4Toggled(bool checked);
  void point5Toggled(bool checked);
  void point6Toggled(bool checked);
  void point7Toggled(bool checked);
  void point8Toggled(bool checked);
  void point9Toggled(bool checked);
  void point10Toggled(bool checked);
  void point11Toggled(bool checked);
  void point12Toggled(bool checked);
  void point13Toggled(bool checked);
  void point14Toggled(bool checked);
  void point15Toggled(bool checked);
  void point16Toggled(bool checked);
  void contactToggled(bool checked);
  void resetButtons(int button);
  void toggleButton(int i, int j);
  void toggleContactButton();

  void set14XSymmetry(bool checked);
  void set14YSymmetry(bool checked);
  void set113XSymmetry(bool checked);
  void set113YSymmetry(bool checked);

  void ContinuousSavingMode();
  void TriggeredSavingMode();
  void ModeOff();
  void ContinuousModeMenu();
  void TriggeredModeMenu();
  void OffModeMenu();

private:
  QDoubleSpinBox *createSpinBox();
  void closeAll();
  bool surfaceModifiedPrompt();
  QColor defaultBgColour;
  QColor defaultPointColour;
  QColor defaultSelectedPointColour;
  QColor defaultLineColour;
  QColor defaultWireframeColour;
  QColor defaultVectorColour;
  QColor defaultPatchColour;
  QColor defaultSelectedPatchColour;

  bool externalSelectSignal;

  // GL widgets and windows
  BezierEditor *bezierEditor;
  TextureWindow *textureWindow;
  TurtleParametersWindow *turtleParametersWindow;
  AdjacenciesWindow *adjacenciesWindow;
  TranslateWindow *translateWindow;
  RotateWindow *rotateWindow;
  ScaleWindow *scaleWindow;
  QPushButton *pointButtons[16];
  QPushButton *contactPointButton;

  // Menus
  QMenuBar *menuBar;
  QMenu *fileMenu;
  QMenu *editMenu;
  QMenu *transformMenu;
  QMenu *flipMenu;
  QMenu *textureMenu;
  QMenu *viewMenu;
  QMenu *preferencesMenu;
  QMenu *colourMenu;
  QMenu *showMenu;
  QMenu *helpMenu;
  QAction *_savingContinu_act;
  QAction *_savingTriggered_act;
  QAction *_savingMenu_act;

  QAction *_savingContinuP_act;
  QAction *_savingTriggeredP_act;
  QAction *_savingMenuP_act;

  QMenu *addButtonContextMenu; // The context menu on the add button that allows
                               // square or triangular patches

  // UI elements
  QDoubleSpinBox
      *xbox; // Spin boxes for specifying the position of the selected point
  QDoubleSpinBox *ybox;
  QDoubleSpinBox *zbox;

  QGroupBox *symmetryBox;
  QButtonGroup *group14;
  QButtonGroup *group113;
  QCheckBox *symmetry14YBox;
  QCheckBox *symmetry14XBox;
  QCheckBox *symmetry113YBox;
  QCheckBox *symmetry113XBox;

  QListWidget *patchList; // The list widget containing information about the
                          // patches in the surface
  QPushButton *addPatchButton;       // Button to add a patch
  QPushButton *deletePatchButton;    // Button to delete a patch
  QPushButton *duplicatePatchButton; // Button to duplicate a patch

  // Disablable QActions
  QAction *loadTextureAct;
  QAction *editTextureAct;
  QAction *_rotateTextureCWAct;
  QAction *_rotateTextureCCWAct;
  QAction *flipTexVAct;
  QAction *flipTexHAct;
  QAction *centerAtContactAct;
  QAction *undoAct;
  QAction *redoAct;
  QAction *revertAct;
  QAction *setWireframeAct;

  QString loadFormats; // QString of image formats that can be loaded
  QString textureDir;  // Directory to look for textures in
  QString
      surfaceFormats; // QString of project formats that can be loadedand saved
  QString surfaceDir; // Directory to look for projects in
  SavingMode _savingMode;
};
