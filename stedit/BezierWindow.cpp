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



#include "BezierWindow.h"

#include <QSizePolicy>
#include <QMessageBox>
#include <QFileDialog>
#include <QColorDialog>
#include <QInputDialog>
#include <QDesktopServices>
#include <QDebug>
#include <QTextBrowser>
#include <QPushButton>
#include <QDialogButtonBox>
#ifdef __APPLE__
#include "cocoabridge.h"
#endif

BezierWindow::BezierWindow(bool extendedFormatFlag, SavingMode savingMode, string surfaceName,
                           string textureName, bool showTextureWindow) {
#ifdef __APPLE__
  CocoaBridge::setAllowsAutomaticWindowTabbing(false);
#endif

  externalSelectSignal = false;
  _savingMode = savingMode;
  // Set up the widgets
  bezierEditor = new BezierEditor(this);
  bezierEditor->setContextMenuPolicy(Qt::CustomContextMenu);

  if (extendedFormatFlag)
    bezierEditor->forceExtendedFormat();
  connect(bezierEditor, SIGNAL(customContextMenuRequested(const QPoint &)),
          this, SLOT(menuize(const QPoint &)));

  setWireframeAct = new QAction("&Wireframe", this);
  readConfig();

  textureWindow = new TextureWindow(textureName,savingMode);

  turtleParametersWindow = new TurtleParametersWindow(this);
  turtleParametersWindow->setMaximumSize(
      10, 10); // This will ensure that the window is its minimum size

  adjacenciesWindow =
      new AdjacenciesWindow(bezierEditor->getSurfaceLoc(), this);

  translateWindow = new TranslateWindow(this);
  translateWindow->setMaximumSize(
      10, 10); // This will ensure that the window is its minimum size

  rotateWindow = new RotateWindow(this);
  rotateWindow->setMaximumSize(
      10, 10); // This will ensure that the window is its minimum size

  scaleWindow = new ScaleWindow(this);
  scaleWindow->setMaximumSize(
      10, 10); // This will ensure that the window is its minimum size


  for (int i = 0; i < 16; i++) {
    pointButtons[i] = new QPushButton(QString::number(i + 1));
    pointButtons[i]->setMaximumSize(40, 40);
    pointButtons[i]->setCheckable(true);
    pointButtons[i]->setAutoExclusive(true);
  }
  
  contactPointButton = new QPushButton("Contact");
  contactPointButton->setMaximumHeight(30);
  contactPointButton->setCheckable(true);
  connect(bezierEditor, SIGNAL(pointSelected(int, int)), this,
          SLOT(toggleButton(int, int)));
  connect(bezierEditor, SIGNAL(contactPointSelected()), this,
          SLOT(toggleContactButton()));

  connect(pointButtons[0], SIGNAL(toggled(bool)), this,
          SLOT(point1Toggled(bool)));
  connect(pointButtons[1], SIGNAL(toggled(bool)), this,
          SLOT(point2Toggled(bool)));
  connect(pointButtons[2], SIGNAL(toggled(bool)), this,
          SLOT(point3Toggled(bool)));
  connect(pointButtons[3], SIGNAL(toggled(bool)), this,
          SLOT(point4Toggled(bool)));
  connect(pointButtons[4], SIGNAL(toggled(bool)), this,
          SLOT(point5Toggled(bool)));
  connect(pointButtons[5], SIGNAL(toggled(bool)), this,
          SLOT(point6Toggled(bool)));
  connect(pointButtons[6], SIGNAL(toggled(bool)), this,
          SLOT(point7Toggled(bool)));
  connect(pointButtons[7], SIGNAL(toggled(bool)), this,
          SLOT(point8Toggled(bool)));
  connect(pointButtons[8], SIGNAL(toggled(bool)), this,
          SLOT(point9Toggled(bool)));
  connect(pointButtons[9], SIGNAL(toggled(bool)), this,
          SLOT(point10Toggled(bool)));
  connect(pointButtons[10], SIGNAL(toggled(bool)), this,
          SLOT(point11Toggled(bool)));
  connect(pointButtons[11], SIGNAL(toggled(bool)), this,
          SLOT(point12Toggled(bool)));
  connect(pointButtons[12], SIGNAL(toggled(bool)), this,
          SLOT(point13Toggled(bool)));
  connect(pointButtons[13], SIGNAL(toggled(bool)), this,
          SLOT(point14Toggled(bool)));
  connect(pointButtons[14], SIGNAL(toggled(bool)), this,
          SLOT(point15Toggled(bool)));
  connect(pointButtons[15], SIGNAL(toggled(bool)), this,
          SLOT(point16Toggled(bool)));
  connect(contactPointButton, SIGNAL(toggled(bool)), this,
          SLOT(contactToggled(bool)));
  pointButtons[0]->setChecked(true);
  connect(textureWindow, SIGNAL(textureEditorClosed(bool)), this,
          SLOT(textureEditorClosed(bool)));
  connect(adjacenciesWindow, SIGNAL(cancelChanges()), bezierEditor,
          SLOT(undo()));
  connect(adjacenciesWindow, SIGNAL(applied()), bezierEditor,
          SLOT(updateAdjacencies()));
  connect(textureWindow, SIGNAL(passTexture(QImage)), bezierEditor,
          SLOT(updateTexture(QImage)));
  connect(textureWindow, SIGNAL(textureLoaded(string)), bezierEditor,
          SLOT(updateTextureName(string)));

  connect(turtleParametersWindow, SIGNAL(updateContactPoint(Point)),
          bezierEditor, SLOT(setContactPoint(Point)));
  connect(bezierEditor, SIGNAL(updateContactPoint(Point)),
          turtleParametersWindow, SLOT(setContactPoint(Point)));
  connect(turtleParametersWindow, SIGNAL(updateEndPoint(Point)), bezierEditor,
          SLOT(setEndPoint(Point)));
  connect(turtleParametersWindow, SIGNAL(updateHeading(Vector3)), bezierEditor,
          SLOT(setHeading(Vector3)));
  connect(turtleParametersWindow, SIGNAL(updateUp(Vector3)), bezierEditor,
          SLOT(setUp(Vector3)));
  connect(turtleParametersWindow, SIGNAL(updateSize(double)), bezierEditor,
          SLOT(setSize(double)));

  connect(translateWindow, SIGNAL(update(Vector3)), bezierEditor,
          SLOT(translate(Vector3)));
  connect(rotateWindow, SIGNAL(update(Vector3, double)), bezierEditor,
          SLOT(rotate(Vector3, double)));
  connect(scaleWindow, SIGNAL(update(Vector3)), bezierEditor,
          SLOT(scale(Vector3)));
  connect(this, SIGNAL(updatingContMode(SavingMode)), textureWindow,
          SLOT(updateContinuousMode(SavingMode)));
  connect(textureWindow, SIGNAL(updatingContMode(SavingMode)), this,
          SLOT(updateContinuousMode(SavingMode)));

  // Create the X, Y, and Z coordinate spin boxes
  xbox = createSpinBox();
  ybox = createSpinBox();
  zbox = createSpinBox();
  connect(bezierEditor, SIGNAL(selectedPointX(double)), xbox,
          SLOT(setValue(double)));
  connect(bezierEditor, SIGNAL(selectedPointY(double)), ybox,
          SLOT(setValue(double)));
  connect(bezierEditor, SIGNAL(selectedPointZ(double)), zbox,
          SLOT(setValue(double)));
  connect(xbox, SIGNAL(editingFinished()), bezierEditor, SLOT(boxEdited()));
  connect(ybox, SIGNAL(editingFinished()), bezierEditor, SLOT(boxEdited()));
  connect(zbox, SIGNAL(editingFinished()), bezierEditor, SLOT(boxEdited()));

  symmetryBox = new QGroupBox("Symmetry:");
  symmetry14YBox = new QCheckBox("Horizontal");
  symmetry14XBox = new QCheckBox("1-4 X");
  symmetry113YBox = new QCheckBox("1-13 Y");
  symmetry113XBox = new QCheckBox("Vertical");
  connect(symmetry14YBox, SIGNAL(toggled(bool)), this,
          SLOT(set14XSymmetry(bool)));
  connect(symmetry14XBox, SIGNAL(toggled(bool)), this,
          SLOT(set14YSymmetry(bool)));
  connect(symmetry113YBox, SIGNAL(toggled(bool)), this,
          SLOT(set113XSymmetry(bool)));
  connect(symmetry113XBox, SIGNAL(toggled(bool)), this,
          SLOT(set113YSymmetry(bool)));

  /*
  addPatchButton = new QPushButton("Add", this);
#ifdef __APPLE__
  addPatchButton->setMaximumWidth(70);
#endif
  addPatchButton->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(addPatchButton, SIGNAL(pressed()), this, SLOT(addNewSquarePatch()));
  connect(addPatchButton, SIGNAL(customContextMenuRequested(const QPoint &)),
          this, SLOT(menuizeAddButton(const QPoint &)));
  deletePatchButton = new QPushButton("Delete", this);
#ifdef __APPLE__
  deletePatchButton->setMaximumWidth(70);
#endif
  connect(deletePatchButton, SIGNAL(pressed()), this,
          SLOT(deleteSelectedPatch()));
  duplicatePatchButton = new QPushButton("Duplicate", this);
#ifdef __APPLE__
  duplicatePatchButton->setMaximumWidth(90);
#endif
  connect(duplicatePatchButton, SIGNAL(pressed()), this,
          SLOT(duplicateSelectedPatch()));
  */
  patchList = new QListWidget(this);
#ifdef __APPLE__
  patchList->setMinimumHeight(76);
#else
  patchList->setMaximumHeight(140);
#endif
  connect(bezierEditor, SIGNAL(addPatchToList(string)), this,
          SLOT(addPatchToList(string)));
  connect(bezierEditor, SIGNAL(newSurface()), this, SLOT(clearPatchList()));
  connect(patchList, SIGNAL(itemSelectionChanged()), this, SLOT(selectPatch()));
  connect(patchList, SIGNAL(itemChanged(QListWidgetItem *)), this,
          SLOT(changePatchName(QListWidgetItem *)));
  connect(patchList, SIGNAL(itemClicked(QListWidgetItem *)), this,
          SLOT(changePatchVisibility()));
  connect(bezierEditor, SIGNAL(updatePatchSelection(int)), this,
          SLOT(selectPatchInList(int)));
  
  connect(bezierEditor, SIGNAL(continuousSave()), this, SLOT(saveSurface()));
  connect(textureWindow, SIGNAL(continuousSave()), this, SLOT(saveSurface()));

  // Set up the menus
  menuBar = new QMenuBar();
  fileMenu = new QMenu(("&File"), this);
  editMenu = new QMenu(("&Edit\1"), this);
  transformMenu = new QMenu(("&Transform"), this);
  flipMenu = new QMenu(("&Flip"), this);
  textureMenu = new QMenu(("&Texture"), this);
  showMenu = new QMenu(("&Show"), this);
  viewMenu = new QMenu(("&View\1"), this);
  preferencesMenu = new QMenu(("&Preferences"), this);
  helpMenu = new QMenu(("&Help"), this);
  addButtonContextMenu = new QMenu(("&Add"), this);

  // Load icons
  QIcon newIcon = style()->standardIcon(QStyle::SP_DialogResetButton);
  QIcon saveIcon = style()->standardIcon(QStyle::SP_DialogSaveButton);
  QIcon saveAsIcon = style()->standardIcon(QStyle::SP_DialogSaveButton);
  QIcon openIcon = style()->standardIcon(QStyle::SP_DialogOpenButton);
  QIcon undoIcon = style()->standardIcon(QStyle::SP_ArrowLeft);
  QIcon redoIcon = style()->standardIcon(QStyle::SP_ArrowRight);
  QIcon revertIcon = style()->standardIcon(QStyle::SP_BrowserReload);
  QIcon editTextureIcon = style()->standardIcon(QStyle::SP_ArrowForward);

  // Set up the menu actions
  fileMenu->addAction(newIcon, "&New", this, SLOT(newFile()),
                      QKeySequence(tr("CTRL+N")));
  fileMenu->addAction(saveIcon, "&Save", this, SLOT(saveSurface()),
                      QKeySequence(tr("CTRL+S")));
  fileMenu->addAction(saveAsIcon, "Save &As...", this, SLOT(saveSurfaceAs()));
  fileMenu->addAction(openIcon, "&Open", this, SLOT(loadSurface()),
                      QKeySequence(tr("CTRL+O")));
  revertAct = new QAction(revertIcon, "&Reload", this);
  connect(revertAct, SIGNAL(triggered()), this, SLOT(revert()));
  connect(bezierEditor, SIGNAL(canRevert(bool)), this,
          SLOT(enableRevert(bool)));
  revertAct->setEnabled(false);
  fileMenu->addAction(revertAct);
  fileMenu->addSeparator();
  fileMenu->addAction("&Preferences...", this, SLOT(preferences()),
                      QKeySequence(tr("CTRL+P")));
  ///////
  QMenu *modeMenu = fileMenu->addMenu("Refresh mode");

  _savingMenu_act = modeMenu->addAction("Explicit", this, SLOT(OffModeMenu()));
  _savingMenu_act->setCheckable(true);
  if (_savingMode == OFF)
    _savingMenu_act->setChecked(true);

  _savingTriggered_act =
      modeMenu->addAction("Triggered", this, SLOT(TriggeredModeMenu()));
  _savingTriggered_act->setCheckable(true);
  if (_savingMode == TRIGGERED)
    _savingTriggered_act->setChecked(true);

  _savingContinu_act =
      modeMenu->addAction("Continuous", this, SLOT(ContinuousModeMenu()));
  _savingContinu_act->setCheckable(true);
  if (_savingMode == CONTINUOUS)
    _savingContinu_act->setChecked(true);

  bezierEditor->setSavingMode(_savingMode);
  fileMenu->addMenu(modeMenu);
  fileMenu->addSeparator();
  fileMenu->addAction("&Exit", this, SLOT(close()), QKeySequence(tr("CTRL+Q")));

  undoAct = new QAction(undoIcon, "&Undo", this);
  redoAct = new QAction(redoIcon, "&Redo", this);
  connect(undoAct, SIGNAL(triggered()), this, SLOT(undo()));
  connect(redoAct, SIGNAL(triggered()), this, SLOT(redo()));
  connect(bezierEditor, SIGNAL(canUndo(bool)), this, SLOT(enableUndo(bool)));
  connect(bezierEditor, SIGNAL(canRedo(bool)), this, SLOT(enableRedo(bool)));
  undoAct->setShortcut(QKeySequence(tr("CTRL+Z")));
  redoAct->setShortcut(QKeySequence(tr("CTRL+Y")));
  undoAct->setEnabled(false);
  redoAct->setEnabled(false);
  editMenu->addAction(undoAct);
  editMenu->addAction(redoAct);
  editMenu->addSeparator();
  editMenu->addAction("&Contact Point...", this, SLOT(editTurtleParameters()),
                      QKeySequence(tr("CTRL+T")));
  //  editMenu->addAction("&Adjacencies...", this, SLOT(editAdjacencies()),
  //                    QKeySequence(tr("CTRL+SHIFT+A")));

  QAction *enforceAdjacencyAct = new QAction("&Enforce Adjacency", this);
  enforceAdjacencyAct->setCheckable(true);
  enforceAdjacencyAct->setChecked(true);
  connect(enforceAdjacencyAct, SIGNAL(toggled(bool)), bezierEditor,
          SLOT(setEnforceAdjacency(bool)));
  //editMenu->addAction(enforceAdjacencyAct);

  editMenu->addSeparator();
  editMenu->addMenu(transformMenu);

  transformMenu->addMenu(flipMenu);
  transformMenu->addAction("&Translate...", this, SLOT(translatePatch()),
                           QKeySequence(tr("CTRL+SHIFT+T")));
  transformMenu->addAction("&Rotate...", this, SLOT(rotatePatch()),
                           QKeySequence(tr("CTRL+SHIFT+R")));
  transformMenu->addAction("&Scale...", this, SLOT(scalePatch()),
                           QKeySequence(tr("CTRL+SHIFT+S")));

  flipMenu->addAction("&Horizontal", this, SLOT(flipPatchHorizontal()));
  flipMenu->addAction("&Vertical", this, SLOT(flipPatchVertical()));
  flipMenu->addAction("&Depthwise", this, SLOT(flipPatchDepth()));

  /*
  loadTextureAct = new QAction(openIcon, "&Open", this);
  loadTextureAct->setShortcut(QKeySequence(tr("CTRL+SHIFT+O")));
  connect(loadTextureAct, SIGNAL(triggered()), this, SLOT(loadTexture()));
  textureMenu->addAction(loadTextureAct);
  textureMenu->addSeparator();
  editTextureAct = new QAction(editTextureIcon, "&Edit\1", this);
  editTextureAct->setShortcut(QKeySequence(tr("CTRL+E")));
  connect(editTextureAct, SIGNAL(triggered()), this, SLOT(editTexture()));

  textureMenu->addAction(editTextureAct);
  textureMenu->addSeparator();
  */
  _rotateTextureCWAct = textureMenu->addAction("Rotate Texture CW", bezierEditor,
                         SLOT(rotateTextureCW()));
  _rotateTextureCCWAct = textureMenu->addAction("Rotate Texture CCW", bezierEditor,
                         SLOT(rotateTextureCCW()));
  flipTexHAct = new QAction("Flip Texture &Horizontal", this);
  flipTexHAct->setCheckable(true);
  connect(flipTexHAct, SIGNAL(toggled(bool)), bezierEditor,
          SLOT(flipTextureH(bool)));
  textureMenu->addAction(flipTexHAct);
  flipTexVAct = new QAction("Flip Texture &Vertical", this);
  flipTexVAct->setCheckable(true);
  connect(flipTexVAct, SIGNAL(toggled(bool)), bezierEditor,
          SLOT(flipTextureV(bool)));
  textureMenu->addAction(flipTexVAct);
  _rotateTextureCWAct->setEnabled(false);
  _rotateTextureCCWAct->setEnabled(false);
  flipTexHAct->setEnabled(false);
  flipTexVAct->setEnabled(false);

  QAction *showPointsAct = new QAction("&Points", this);
  showPointsAct->setCheckable(true);
  showPointsAct->setChecked(true);
  showPointsAct->setShortcut(QKeySequence(tr("CTRL+1")));
  connect(showPointsAct, SIGNAL(toggled(bool)), this,
          SLOT(showHidePoints(bool)));
  showMenu->addAction(showPointsAct);

  QAction *showLinesAct = new QAction("&Lines", this);
  showLinesAct->setCheckable(true);
  showLinesAct->setChecked(true);
  showLinesAct->setShortcut(QKeySequence(tr("CTRL+2")));
  connect(showLinesAct, SIGNAL(toggled(bool)), this, SLOT(showHideLines(bool)));
  showMenu->addAction(showLinesAct);

  QAction *showSurfaceAct = new QAction("&Surface", this);
  showSurfaceAct->setCheckable(true);
  showSurfaceAct->setChecked(true);
  showSurfaceAct->setShortcut(QKeySequence(tr("CTRL+3")));
  connect(showSurfaceAct, SIGNAL(toggled(bool)), this,
          SLOT(showHideSurface(bool)));
  showMenu->addAction(showSurfaceAct);

  QAction *showVectorsAct = new QAction("&Vectors", this);
  showVectorsAct->setCheckable(true);
  showVectorsAct->setChecked(true);
  showVectorsAct->setShortcut(QKeySequence(tr("CTRL+4")));
  connect(showVectorsAct, SIGNAL(toggled(bool)), this,
          SLOT(showHideVectors(bool)));
  showMenu->addAction(showVectorsAct);

  //viewMenu->addMenu(showMenu);
  setWireframeAct->setCheckable(true);
  setWireframeAct->setShortcut(QKeySequence(tr("CTRL+W")));
  connect(setWireframeAct, SIGNAL(toggled(bool)), this,
          SLOT(setWireframe(bool)));
  viewMenu->addAction(setWireframeAct);
  viewMenu->addSeparator();

  QAction *lockRotationAct = new QAction("Lock &Rotation", this);
  lockRotationAct->setCheckable(true);
  connect(lockRotationAct, SIGNAL(toggled(bool)), bezierEditor,
          SLOT(setRotationLocked(bool)));
  viewMenu->addAction(lockRotationAct);

  viewMenu->addAction(revertIcon, "&Reset View", this, SLOT(resetView()),
                      QKeySequence(tr("CTRL+R")));

  viewMenu->addAction("&Center", this, SLOT(centerCamera()),
                      QKeySequence(tr("CTRL+C")));

  centerAtContactAct = new QAction("Center at Contact", this);
  centerAtContactAct->setCheckable(true);
  connect(centerAtContactAct, SIGNAL(toggled(bool)), this,
          SLOT(centerAtContact(bool)));
  viewMenu->addAction(centerAtContactAct);
  connect(bezierEditor, SIGNAL(contactCenteredChanged(bool)), this,
          SLOT(setContactCentered(bool)));

  viewMenu->addSeparator();
  
  QMenu* resizeMenu = new QMenu("Resize");
  resizeMenu->addAction("&Point Size...", this, SLOT(changePointSize()));
  resizeMenu->addAction("&Line Width...", this, SLOT(changeLineWidth()));
  resizeMenu->addAction("&Wireframe Width...", this,
                             SLOT(changeWireframeWidth()));
  resizeMenu->addAction("&Subdivision Samples...", this,
                             SLOT(changeSubdivisionLevel()));

  viewMenu->addMenu(resizeMenu);

  colourMenu = new QMenu(("&Color"), this);

  colourMenu->addAction("&Background Color", this, SLOT(changeBgColour()));
  colourMenu->addAction("&Point Color", this, SLOT(changePointColour()));
  colourMenu->addAction("&Selected Point Color", this,
                        SLOT(changeSelectedPointColour()));
  colourMenu->addAction("&Line Color", this, SLOT(changeLineColour()));
  colourMenu->addAction("&Wireframe Color", this,
                        SLOT(changeWireframeColour()));
  colourMenu->addAction("&Vector Color", this, SLOT(changeVectorColour()));
  colourMenu->addAction("&Patch Color", this, SLOT(changePatchColour()));
  colourMenu->addAction("&Selected Patch Color", this,
                        SLOT(changeSelectedPatchColour()));
  colourMenu->addSeparator();
  colourMenu->addAction(revertIcon, "&Reset", this, SLOT(resetColours()));
  viewMenu->addMenu(colourMenu);
  viewMenu->addSeparator();
  viewMenu->addSeparator();

  /*
  helpMenu->addAction("&Controls", this, SLOT(showControlsHelp()),
                      QKeySequence(tr("F1")));
  helpMenu->addAction("&About", this, SLOT(showAbout()),
		      QKeySequence(tr("F2")));
#ifdef __APPLE__
  helpMenu->addAction("Stedit Help", this, SLOT(help()));
#endif
  helpMenu->addAction("Stedit Help (PDF)", this, SLOT(pdfHelp()));
  */
  QAction *qHelp=helpMenu->addAction("Quick help",this,SLOT(quickHelp()));
  qHelp->setEnabled(true);
  helpMenu->addAction("Tools manual", this, SLOT(pdfHelp()));

 
  addButtonContextMenu->addAction("&Square", this, SLOT(addNewSquarePatch()));
  addButtonContextMenu->addAction("&Triangle", this,
                                  SLOT(addNewTrianglePatch()));

  preferencesMenu->addAction(lockRotationAct);
  preferencesMenu->addMenu(transformMenu);
  preferencesMenu->addSeparator();
  preferencesMenu->addAction(revertIcon, "&Reset View", this, SLOT(resetView()),
                             QKeySequence(tr("CTRL+R")));
  preferencesMenu->addSeparator();
  
  preferencesMenu->addAction("&Center", this, SLOT(centerCamera()),
                             QKeySequence(tr("CTRL+C")));
  preferencesMenu->addAction(centerAtContactAct);
  preferencesMenu->addAction(revertAct);

  ///////
  QMenu *modeMenuPreferences = preferencesMenu->addMenu("Refresh mode");
  preferencesMenu->addSeparator();
  preferencesMenu->addAction("Quit", this, SLOT(close()), QKeySequence(tr("CTRL+Q")));

  _savingMenuP_act =
      modeMenuPreferences->addAction("Explicit", this, SLOT(OffModeMenu()));
  _savingMenuP_act->setCheckable(true);
  if (_savingMode == OFF)
    _savingMenuP_act->setChecked(true);

  _savingTriggeredP_act = modeMenuPreferences->addAction(
      "Triggered", this, SLOT(TriggeredModeMenu()));
  _savingTriggeredP_act->setCheckable(true);
  if (_savingMode == TRIGGERED)
    _savingTriggeredP_act->setChecked(true);

  _savingContinuP_act = modeMenuPreferences->addAction(
      "Continuous", this, SLOT(ContinuousModeMenu()));
  _savingContinuP_act->setCheckable(true);
  if (_savingMode == CONTINUOUS)
    _savingContinuP_act->setChecked(true);

  // Put the menus in the menu bar
  menuBar->addMenu(fileMenu);
  menuBar->addMenu(editMenu);
  //  menuBar->addMenu(textureMenu);
  menuBar->addMenu(viewMenu);
  menuBar->addMenu(helpMenu);
  QMenu *about = menuBar->addMenu("About");
  about->addAction("About", this, SLOT(showAbout()));

  // Create the layouts
  QHBoxLayout *mainLayout = new QHBoxLayout;
  QVBoxLayout *sidebarLayout = new QVBoxLayout;
  QGridLayout *pickerLayout = new QGridLayout;
  QHBoxLayout *buttonsLayout = new QHBoxLayout;
  QVBoxLayout *editorLayout = new QVBoxLayout;
  QGridLayout *spinBoxLayout = new QGridLayout;
  QGridLayout *checkBoxLayout = new QGridLayout;
  QFrame *spinBoxFrame = new QFrame;
  spinBoxFrame->setMaximumSize(200, 200);
  QFrame *editorFrame = new QFrame;
  editorFrame->setFrameShape(QFrame::Box);
  QFrame *pickerFrame = new QFrame;
#ifndef __APPLE__
  pickerFrame->setFrameShape(QFrame::Box);
#endif

  // Put together the layout
  mainLayout->setMenuBar(menuBar);
  mainLayout->addWidget(editorFrame);
  mainLayout->addLayout(sidebarLayout);
#ifdef __APPLE__
  mainLayout->setContentsMargins(10, 10, 10, 10);
  mainLayout->setSpacing(10);
#endif
  sidebarLayout->addWidget(pickerFrame, 0, Qt::AlignHCenter);
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      pickerLayout->addWidget(pointButtons[i + 4 * j], j, i);
    }
  }
  
#ifdef __APPLE__
  pickerLayout->addWidget(contactPointButton, 5, 0, 1, 4, Qt::AlignJustify);
#else
  pickerLayout->addWidget(contactPointButton, 5, 0, 1, 4);
#endif
  
  sidebarLayout->addWidget(new QLabel("Patches:"));
#ifdef __APPLE__
  sidebarLayout->addWidget(patchList, 0, Qt::AlignHCenter);
#else
  sidebarLayout->addWidget(patchList);
#endif
  //sidebarLayout->addLayout(buttonsLayout);
  //  buttonsLayout->addWidget(addPatchButton);
  //buttonsLayout->addWidget(duplicatePatchButton);
  //buttonsLayout->addWidget(deletePatchButton);
#ifdef __APPLE__
  buttonsLayout->setContentsMargins(0, 10, 0, 10);
  buttonsLayout->setSpacing(10);
#else
  sidebarLayout->addSpacerItem(
      new QSpacerItem(0, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));
#endif
  
  sidebarLayout->addWidget(symmetryBox);
  
  symmetryBox->setLayout(checkBoxLayout);
  
  checkBoxLayout->addWidget(symmetry14YBox, 0, 0);
  //checkBoxLayout->addWidget(symmetry14XBox, 0, 1);
  //checkBoxLayout->addWidget(symmetry113YBox, 0, 1);
  checkBoxLayout->addWidget(symmetry113XBox, 0, 1);
  sidebarLayout->addWidget(spinBoxFrame);
#ifdef __APPLE__
  sidebarLayout->setContentsMargins(0, 0, 0, 0);
#endif
  editorLayout->addWidget(bezierEditor);
  QLabel *xlbl = new QLabel("X");
  QLabel *ylbl = new QLabel("Y");
  QLabel *zlbl = new QLabel("Z");
  xlbl->setMaximumSize(QSize(10, 10));
  ylbl->setMaximumSize(QSize(10, 10));
  zlbl->setMaximumSize(QSize(10, 10));
  spinBoxLayout->addWidget(xlbl, 0, 0);
  spinBoxLayout->addWidget(ylbl, 1, 0);
  spinBoxLayout->addWidget(zlbl, 2, 0);
  xbox->setValue(-1.5);
  ybox->setValue(1.5);
  spinBoxLayout->addWidget(xbox, 0, 1);
  spinBoxLayout->addWidget(ybox, 1, 1);
  spinBoxLayout->addWidget(zbox, 2, 1);
  connect(xbox, SIGNAL(editingFinished()), bezierEditor,
          SLOT(saveXYZValue()));
  connect(xbox, SIGNAL(valueChanged(double)), bezierEditor,
          SLOT(setXValue(double)));
  connect(ybox, SIGNAL(editingFinished()), bezierEditor,
          SLOT(saveXYZValue()));
  connect(ybox, SIGNAL(valueChanged(double)), bezierEditor,
          SLOT(setYValue(double)));
  connect(zbox, SIGNAL(editingFinished()), bezierEditor,
          SLOT(saveXYZValue()));
  connect(zbox, SIGNAL(valueChanged(double)), bezierEditor,
          SLOT(setZValue(double)));
  spinBoxFrame->setLayout(spinBoxLayout);
  editorFrame->setLayout(editorLayout);
  editorLayout->setContentsMargins(0, 0, 0, 0);
  pickerFrame->setLayout(pickerLayout);
#ifdef __APPLE__
  pickerFrame->setMaximumSize(140, 180);
  pickerFrame->setMinimumSize(140, 180);
  pickerFrame->setFixedSize(140, 180);
  pickerFrame->setContentsMargins(0, 0, 0, 0);
  pickerFrame->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  pickerLayout->setHorizontalSpacing(7);
  pickerLayout->setVerticalSpacing(0);
#else
  pickerFrame->setMaximumSize(120, 150);
  pickerLayout->setSpacing(0);
#endif
  pickerLayout->setContentsMargins(0, 0, 0, 0);
  bezierEditor->setFocus();

  setLayout(mainLayout);
  std::string windowName = "Bezier Editor: ";
  std::string base_filename = surfaceName.substr(surfaceName.find_last_of("/\\") + 1);

  windowName += base_filename;
  if (surfaceName.empty())
    windowName += "noname.s";
  setWindowTitle(windowName.c_str());

  // Initialize the lists of acceptable file formats, as provided by
  // QImageReader/Writer
  QList<QByteArray> loadFormatsList = QImageReader::supportedImageFormats();
  for (int i = 0; i < loadFormatsList.size(); i++) {
    loadFormats.append(QImageFormatStrings::getString(loadFormatsList.at(i)));
    loadFormats.append(" (*.");
    loadFormats.append(loadFormatsList.at(i));
    loadFormats.append(");;");
  }

  surfaceFormats = "Bezier Surfaces (*.s)";
  if (!surfaceName.empty()){
    bool res = bezierEditor->initSurfaceFilename(surfaceName);
    if (!res){
      exit(0);
    }
  }

  if (!textureName.empty()){
    bool res = bezierEditor->initTextureFilename(textureName);
    if (!res){
      exit(0);
    }
  }
  if (surfaceName.empty()) {
    addNewSquarePatch();
    resetView();
  }

  if (showTextureWindow) {
    _rotateTextureCWAct->setEnabled(true);
    _rotateTextureCCWAct->setEnabled(true);
    flipTexHAct->setEnabled(true);
    flipTexVAct->setEnabled(true);
    
    editTexture();
    QPoint parentPosition = textureWindow->pos();
    QPoint widgetPosition(parentPosition.x() + 500, parentPosition.y());
    this->move(widgetPosition);
  }
  changePatchVisibility();
}

BezierWindow::~BezierWindow() {}

void BezierWindow::updateContinuousMode(SavingMode mode) {
  if (mode == CONTINUOUS)
    ContinuousSavingMode();
  else if (mode == TRIGGERED)
    TriggeredSavingMode();
  else
    ModeOff();
}

void BezierWindow::ContinuousModeMenu() {
  ContinuousSavingMode();
  emit updatingContMode(CONTINUOUS);
}

void BezierWindow::TriggeredModeMenu() {
  TriggeredSavingMode();
  emit updatingContMode(TRIGGERED);
}

void BezierWindow::OffModeMenu() {
  ModeOff();
  emit updatingContMode(OFF);
}

void BezierWindow::ContinuousSavingMode() {
  bezierEditor->setSavingMode(CONTINUOUS);
  _savingMode = CONTINUOUS;
  _savingContinu_act->setChecked(true);
  _savingTriggered_act->setChecked(false);
  _savingMenu_act->setChecked(false);
  _savingContinuP_act->setChecked(true);
  _savingTriggeredP_act->setChecked(false);
  _savingMenuP_act->setChecked(false);
}

void BezierWindow::TriggeredSavingMode() {
  bezierEditor->setSavingMode(TRIGGERED);
  _savingMode = TRIGGERED;
  _savingContinu_act->setChecked(false);
  _savingTriggered_act->setChecked(true);
  _savingMenu_act->setChecked(false);

  _savingContinuP_act->setChecked(false);
  _savingTriggeredP_act->setChecked(true);
  _savingMenuP_act->setChecked(false);
}

void BezierWindow::ModeOff() {
  bezierEditor->setSavingMode(OFF);
  if (textureWindow)
    textureWindow->setSavingMode(OFF);

  _savingMode = OFF;
  _savingContinu_act->setChecked(false);
  _savingTriggered_act->setChecked(false);
  _savingMenu_act->setChecked(true);

  _savingContinuP_act->setChecked(false);
  _savingTriggeredP_act->setChecked(false);
  _savingMenuP_act->setChecked(true);
}

QSize BezierWindow::sizeHint() const { return QSize(200, 200); }

// Prompts the user about saving unsaved modifications to the file, and returns
// the result
bool BezierWindow::surfaceModifiedPrompt() {
  QMessageBox box;
  if (bezierEditor->unsavedChanges) {
    box.setIcon(QMessageBox::Warning);
    box.setWindowTitle("Bezier Editor");
    box.setText("The surface has been modified");
    box.setInformativeText("Do you want to save your changes?");
    box.setStandardButtons(QMessageBox::Save | QMessageBox::Discard |
                           QMessageBox::Cancel);
    box.setDefaultButton(QMessageBox::Save);
    box.setEscapeButton(QMessageBox::Cancel);
    int ret = box.exec();
    switch (ret) {
    case QMessageBox::Save: // 'Save' was clicked
      return saveSurface();
    case QMessageBox::Discard: // 'Don't Save' was clicked
      return true;
    case QMessageBox::Cancel: // 'Cancel' was clicked
      return false;
    default: // This should never be reached
      return false;
    }
  }
  return true;
}

// Slots

// Resets the bezier editor to its initial state for creation of a new surface
void BezierWindow::newFile() {
  bool proceed = surfaceModifiedPrompt();
  if (proceed) {
    bezierEditor->clearSurfaceFilename();    
    bezierEditor->reset();
    addNewSquarePatch();
    resetView();
    turtleParametersWindow->close();
    toggleButton(0, 0);
    std::string windowName = "Bezier Editor: ";
    windowName += "noname.s";
    setWindowTitle(windowName.c_str());

  }
}

// Provides a dialog box to choose a texture to load
void BezierWindow::loadTexture() {
  QString *formatString =
      new QString(" (*." + textureDir.section('.', -1) + ")");
  formatString->prepend(
      QImageFormatStrings::getString(textureDir.section('.', -1)));

  QString qfilename = QFileDialog::getOpenFileName(
      0, tr("Load Texture"), textureDir, loadFormats, formatString);

  if (!qfilename.isNull()) {
    textureDir = qfilename;
    string fname = qfilename.toStdString();
    bezierEditor->loadTexture(fname.c_str());
    //    editTexture();
  }
  _rotateTextureCWAct->setEnabled(true);
  _rotateTextureCCWAct->setEnabled(true);
  flipTexHAct->setEnabled(true);
  flipTexVAct->setEnabled(true);

}

// Provides a dialog box to choose a surface to load
void BezierWindow::loadSurface() {
  bool proceed = surfaceModifiedPrompt();
  if (proceed) {
    QString qfilename = QFileDialog::getOpenFileName(
        0, tr("Load Surface"), surfaceDir, surfaceFormats);

    if (!qfilename.isNull()) {
      surfaceDir = qfilename;
      string fname = qfilename.toStdString();
      bezierEditor->load(fname);
      toggleButton(0, 0);
      turtleParametersWindow->close();
      std::string windowName = "Bezier Editor: ";
      std::string base_filename = fname.substr(fname.find_last_of("/\\") + 1);
      windowName += base_filename;
      setWindowTitle(windowName.c_str());

    }
  }
}
// Save the surface over the previously saved surface, or open 'save as' if
// there is none
bool BezierWindow::saveSurface() {
  if (bezierEditor->noSurfaceFilename())
    return saveSurfaceAs();
  else {
    bezierEditor->save();
    return true;
  }
}

// Provides a dialog box to save a new surface, or save over an old one
bool BezierWindow::saveSurfaceAs() {
  QString qfilename = QFileDialog::getSaveFileName(0, tr("Save Surface"),
                                                   surfaceDir, surfaceFormats);

  if (!qfilename.isNull()) {
    surfaceDir = qfilename;
    string fname = qfilename.toStdString();
    bezierEditor->save(fname);
    std::string windowName = "Bezier Editor: ";
    windowName += fname;
    setWindowTitle(windowName.c_str());
    return true;
  }
  return false;
}

// Reloads the previously loaded surface
void BezierWindow::revert() {
  if (!bezierEditor->noSurfaceFilename() && bezierEditor->unsavedChanges) {
    QMessageBox box;
    box.setWindowTitle("Revert Surface");
    box.setText(
        "Are you sure you want to revert to the most recent saved version?");
    box.setInformativeText("All unsaved changes will be lost");
    box.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    box.setDefaultButton(QMessageBox::Cancel);
    box.setEscapeButton(QMessageBox::Cancel);
    int ret = box.exec();
    switch (ret) {
    case QMessageBox::Ok: // Ok was clicked
      bezierEditor->reload();
      turtleParametersWindow->close();
      break;
    case QMessageBox::Cancel: // Cancel was clicked
      break;
    default: // should never be reached
      break;
    }
  }
}

// Perform an undo in the bezier editor and update the point picker to have the
// correct selected point
void BezierWindow::undo() {
  bezierEditor->undo();
  toggleButton(bezierEditor->getSelectedPointI(),
               bezierEditor->getSelectedPointJ());
}

// Perform a redo in the bezier editor and update the point picker to have the
// correct selected point
void BezierWindow::redo() {
  bezierEditor->redo();
  toggleButton(bezierEditor->getSelectedPointI(),
               bezierEditor->getSelectedPointJ());
}

void BezierWindow::enableUndo(bool value) { undoAct->setEnabled(value); }

void BezierWindow::enableRedo(bool value) { redoAct->setEnabled(value); }

// Opens the texture editor
void BezierWindow::editTexture() {
  if (textureWindow == NULL){
    textureWindow = new TextureWindow();
    connect(textureWindow, SIGNAL(textureEditorClosed(bool)), this,
	    SLOT(textureEditorClosed(bool)));
    connect(textureWindow, SIGNAL(passTexture(QImage)), bezierEditor,
	    SLOT(updateTexture(QImage)));
    connect(textureWindow, SIGNAL(textureLoaded(string)), bezierEditor,
	    SLOT(updateTextureName(string)));
    connect(textureWindow, SIGNAL(textureEditorClosed(bool)), this,
	    SLOT(textureEditorClosed(bool)));
    connect(textureWindow, SIGNAL(updatingContMode(SavingMode)), this,
	    SLOT(updateContinuousMode(SavingMode)));
    connect(this, SIGNAL(updatingContMode(SavingMode)), textureWindow,
	    SLOT(updateContinuousMode(SavingMode)));
    connect(textureWindow, SIGNAL(continuousSave()), this, SLOT(saveSurface()));
  }

  /*
  loadTextureAct->setEnabled(
      false); // Can't load a new texture while the texture editoris open; it
              // can be done from the editor
  editTextureAct->setEnabled(
      false); // Can't open the editor when it's already open
  */
  // textureMenu->setEnabled(false); // Can't use this while the texture editor
  // is open
  textureWindow->resize(500, 500);
  textureWindow->setSavingMode(_savingMode);
#ifdef __APPLE__  
  textureWindow->setWindowIcon(QIcon());
#endif
  textureWindow->show();
  if (bezierEditor->hasTexture()) {
    if (bezierEditor->noCurrentTexture()) {
      textureWindow->useImage(
          bezierEditor->getTextureName()); // Use the texture that is currently
                                           // on the surface
    } else {
      bezierEditor->saveCurrentTexture();
      if (!bezierEditor->noCurrentTexture())
        textureWindow->useImage((bezierEditor->getCurrentTexture()));
    }
  }
  textureWindow->initEditor();
}

// Sets up and opens a window for editing the turtle parameters
void BezierWindow::editTurtleParameters() {
  turtleParametersWindow->setContactPoint(
      bezierEditor
          ->getContactPoint()); // Get the initial values from the editor
  turtleParametersWindow->setEndPoint(bezierEditor->getEndPoint());
  turtleParametersWindow->setHeading(bezierEditor->getHeading());
  turtleParametersWindow->setUp(bezierEditor->getUp());
  turtleParametersWindow->setSize(bezierEditor->getSize());
  turtleParametersWindow->show();
}

// Sets up and opens a window for editing patch adjacencies
void BezierWindow::editAdjacencies() {
  bezierEditor->saveUndoState();
  adjacenciesWindow->setPatchList(bezierEditor->getPatchNames());
  adjacenciesWindow->reset();
  int ret = adjacenciesWindow->exec();
  if (ret) {
    bezierEditor->updateAdjacencies();
  } else {
    bezierEditor->undo();
  }
}

// Indicates that the texture editor has been closed, and whether or not it
// saved anything
void BezierWindow::textureEditorClosed(bool saved) {
  //loadTextureAct->setEnabled(true);
  //editTextureAct->setEnabled(true);
  _rotateTextureCWAct->setEnabled(false);
  _rotateTextureCCWAct->setEnabled(false);
  flipTexHAct->setEnabled(false);
  flipTexVAct->setEnabled(false);

  
  if (!saved) { // Revert back to the previously loaded texture if the changes
                // in the editor were discarded
    bezierEditor->restoreTexture();
    bezierEditor->reloadTexture();
  }
  disconnect(this, SIGNAL(updatingContMode(SavingMode)), textureWindow,
	    SLOT(updateContinuousMode(SavingMode)));
  disconnect(textureWindow, SIGNAL(textureEditorClosed(bool)), this,
	    SLOT(textureEditorClosed(bool)));
  disconnect(textureWindow, SIGNAL(passTexture(QImage)), bezierEditor,
	    SLOT(updateTexture(QImage)));
  disconnect(textureWindow, SIGNAL(textureLoaded(string)), bezierEditor,
	    SLOT(updateTextureName(string)));
  disconnect(textureWindow, SIGNAL(textureEditorClosed(bool)), this,
	    SLOT(textureEditorClosed(bool)));
  disconnect(textureWindow, SIGNAL(updatingContMode(SavingMode)), this,
	    SLOT(updateContinuousMode(SavingMode)));
  disconnect(textureWindow, SIGNAL(continuousSave()), this, SLOT(saveSurface()));

  textureWindow = NULL;
  this->raise();
  this-> activateWindow();
}

void BezierWindow::flipPatchHorizontal() {
  bezierEditor->flipPatchHorizontal();
}

void BezierWindow::flipPatchVertical() { bezierEditor->flipPatchVertical(); }

void BezierWindow::flipPatchDepth() { bezierEditor->flipPatchDepth(); }

// Resets and opens the translate window so that a new translation can be
// performed
void BezierWindow::translatePatch() {
  bezierEditor->saveUndoState();
  bezierEditor->resetTranslation();
  translateWindow->reset();
  translateWindow->exec();
}

// Resets and opens the rotation window so that a new rotation can be performed
void BezierWindow::rotatePatch() {
  bezierEditor->saveUndoState();
  bezierEditor->resetRotation();
  rotateWindow->reset();
  rotateWindow->exec();
}

// Resets and opens the scale window so that a new scale operation can be
// performed
void BezierWindow::scalePatch() {
  bezierEditor->saveUndoState();
  bezierEditor->resetScale();
  scaleWindow->reset();
  scaleWindow->exec();
}

// Asks for confirmation on exit
void BezierWindow::closeEvent(QCloseEvent *ce) {
  QMessageBox box;
  if (bezierEditor->unsavedChanges) {
    bool saved;
    box.setIcon(QMessageBox::Warning);
    box.setWindowTitle("Bezier Editor");
    box.setText("The surface has been modified");
    box.setInformativeText("Do you want to save your changes?");
    box.setStandardButtons(QMessageBox::Save | QMessageBox::Discard |
                           QMessageBox::Cancel);
    box.setDefaultButton(QMessageBox::Save);
    box.setEscapeButton(QMessageBox::Cancel);
    int ret = box.exec();
    switch (ret) {
    case QMessageBox::Save: // 'Save' was clicked
      saved = saveSurface();
      if (saved) {
        closeAll();
        ce->accept();
      } else
        ce->ignore();
      break;
    case QMessageBox::Discard: // 'Don't Save' was clicked
      closeAll();
      ce->accept();
      break;
    case QMessageBox::Cancel: // 'Cancel' was clicked
      ce->ignore();
      break;
    default: // This should never be reached
      break;
    }
  } else {
    box.setWindowTitle("Bezier Editor");
    box.setIcon(QMessageBox::Question);
    box.setText("Are you sure you want to quit?");
    box.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    box.setDefaultButton(QMessageBox::Ok);
    box.setEscapeButton(QMessageBox::Cancel);
    int choice = box.exec();
    switch (choice) {
    case QMessageBox::Ok: // 'Ok' was clicked
      closeAll();
      ce->accept();
      break;
    case QMessageBox::Cancel: // 'Cancel' was clicked
      ce->ignore();
      break;
    default: // This should never be reached
      break;
    }
  }
}

// Close all windows associated with the editor
void BezierWindow::closeAll() {
  if (textureWindow != NULL)
    textureWindow->close();
  turtleParametersWindow->close();
  translateWindow->close();
  rotateWindow->close();
  scaleWindow->close();
  adjacenciesWindow->close();
}

// Opens a colour picker to choose a new background colour
void BezierWindow::changeBgColour() {
  QColor colour = QColorDialog::getColor(
      QColor(bezierEditor->getBgColour().r * 255,
             bezierEditor->getBgColour().g * 255,
             bezierEditor->getBgColour().b * 255),
      this, "Pick Background Color", QColorDialog::DontUseNativeDialog);

  if (colour.isValid()) {
    bezierEditor->changeBgColour(Colour(colour));
  }
}

// Opens a colour picker to choose a new point colour
void BezierWindow::changePointColour() {
  QColor colour = QColorDialog::getColor(
      QColor(bezierEditor->getPointColour().r * 255,
             bezierEditor->getPointColour().g * 255,
             bezierEditor->getPointColour().b * 255),
      this, "Pick Point Color", QColorDialog::DontUseNativeDialog);

  if (colour.isValid()) {
    bezierEditor->changePointColour(Colour(colour));
  }
}

// Opens a colour picker to choose a new selected point colour
void BezierWindow::changeSelectedPointColour() {
  QColor colour = QColorDialog::getColor(
      QColor(bezierEditor->getSelectedPointColour().r * 255,
             bezierEditor->getSelectedPointColour().g * 255,
             bezierEditor->getSelectedPointColour().b * 255),
      this, "Pick Selected Point Color", QColorDialog::DontUseNativeDialog);

  if (colour.isValid()) {
    bezierEditor->changeSelectedPointColour(Colour(colour));
  }
}

// Opens a colour picker to choose a new line colour
void BezierWindow::changeLineColour() {
  QColor colour = QColorDialog::getColor(
      QColor(bezierEditor->getLineColour().r * 255,
             bezierEditor->getLineColour().g * 255,
             bezierEditor->getLineColour().b * 255),
      this, "Pick Line Color", QColorDialog::DontUseNativeDialog);

  if (colour.isValid()) {
    bezierEditor->changeLineColour(Colour(colour));
  }
}

// Opens a colour picker to choose a new colour for the wireframe lines
void BezierWindow::changeWireframeColour() {
  QColor colour = QColorDialog::getColor(
      QColor(bezierEditor->getWireframeColour().r * 255,
             bezierEditor->getWireframeColour().g * 255,
             bezierEditor->getWireframeColour().b * 255),
      this, "Pick Wireframe Color", QColorDialog::DontUseNativeDialog);

  if (colour.isValid()) {
    bezierEditor->changeWireframeColour(Colour(colour));
  }
}

// Opens a colour picker to choose a new vector colour
void BezierWindow::changeVectorColour() {
  QColor colour = QColorDialog::getColor(
      QColor(bezierEditor->getVectorColour().r * 255,
             bezierEditor->getVectorColour().g * 255,
             bezierEditor->getVectorColour().b * 255),
      this, "Pick Vector Color", QColorDialog::DontUseNativeDialog);

  if (colour.isValid()) {
    bezierEditor->changeVectorColour(Colour(colour));
  }
}

// Opens a colour picker to choose a new patch colour
void BezierWindow::changePatchColour() {
  QColor colour = QColorDialog::getColor(
      QColor(bezierEditor->getPatchColour().r * 255,
             bezierEditor->getPatchColour().g * 255,
             bezierEditor->getPatchColour().b * 255),
      this, "Pick Patch Color", QColorDialog::DontUseNativeDialog);

  if (colour.isValid()) {
    bezierEditor->changePatchColour(Colour(colour));
  }
}

// Opens a colour picker to choose a new selected patch colour
void BezierWindow::changeSelectedPatchColour() {
  QColor colour = QColorDialog::getColor(
      QColor(bezierEditor->getSelectedPatchColour().r * 255,
             bezierEditor->getSelectedPatchColour().g * 255,
             bezierEditor->getSelectedPatchColour().b * 255),
      this, "Pick Selected Patch Color", QColorDialog::DontUseNativeDialog);

  if (colour.isValid()) {
    bezierEditor->changeSelectedPatchColour(Colour(colour));
  }
}

// Resets all the colours to defaults
void BezierWindow::resetColours() { readConfig(); }

// Enables or dieables wireframe mode
void BezierWindow::setWireframe(bool value) {
  bezierEditor->setWireframe(value);
}

// Centers the camera and zooms to fit the surface's current position
void BezierWindow::centerCamera() { bezierEditor->centerCamera(); }

// Centers the camera around the surface's contact point
void BezierWindow::centerAtContact(bool value) {
  bezierEditor->centerAtContact(value);
}

// Resets the viewer to its initial state
void BezierWindow::resetView() { bezierEditor->resetView(); }

// Provides a dialog box for choosing a new point size
void BezierWindow::changePointSize() {
  bezierEditor->setPointSize(QInputDialog::getInt(
      0, "Set Point Size", "Size", bezierEditor->getPointSize(), 1, 20));
}

// Provides a dialog box for choosing a new line width
void BezierWindow::changeLineWidth() {
  bezierEditor->setLineWidth(QInputDialog::getInt(
      0, "Set Line Width", "Width", bezierEditor->getLineWidth(), 1, 20));
}

// Provides a dialog box for choosing a new width for the wireframe lines
void BezierWindow::changeWireframeWidth() {
  bezierEditor->setWireframeWidth(
      QInputDialog::getInt(0, "Set Wireframe Width", "Width",
                           bezierEditor->getWireframeWidth(), 1, 20));
}

// Provides a dialog box to choose the level of bezier subdivision
void BezierWindow::changeSubdivisionLevel() {
  bezierEditor->setSubdivisionSamples(
      QInputDialog::getInt(0, "Set Subdivision Samples", "Samples",
                           bezierEditor->getSubdivisionSamples(), 1, 40));
}

// Sets the visibility of the points to the given boolean value
void BezierWindow::showHidePoints(bool value) {
  bezierEditor->showHidePoints(value);
}

// Sets the visibility of the lines to the given boolean value
void BezierWindow::showHideLines(bool value) {
  bezierEditor->showHideLines(value);
}

// Sets the visibility of the surface to the given boolean value
void BezierWindow::showHideSurface(bool value) {
  bezierEditor->showHideSurface(value);
}

// Sets the visibility of the surface to the given boolean value
void BezierWindow::showHideVectors(bool value) {
  bezierEditor->showHideVectors(value);
}

void BezierWindow::setContactCentered(bool value) {
  centerAtContactAct->setChecked(value);
}

void BezierWindow::showControlsHelp() {
  QMessageBox::about(this, "Controls",
                     "<table>"
                     "<tr><td>Left Click</td><td><pre>   </pre></td>"
                     "<td>Rotate surface view</td></tr>"
                     "<tr><td>Ctrl+Left</td><td><pre>   </pre></td>"
                     "<td>Move control points</td></tr>"
                     "<tr><td>Shift+Left</td><td><pre>   </pre></td>"
                     "<td>Pan surface view</td></tr>"
                     "<tr><td>Middle Click</td><td><pre>   </pre></td>"
                     "<td>Zoom surface view</td></tr>"
                     "<tr><td>Mouse Wheel</td><td><pre>   </pre></td>"
                     "<td>Zoom surface view</td></tr>"
                     "<tr><td>Ctrl+1</td><td><pre>   </pre></td>"
                     "<td>Show/hide points</td></tr>"
                     "<tr><td>Ctrl+2</td><td><pre>   </pre></td>"
                     "<td>Show/hide lines</td></tr>"
                     "<tr><td>Ctrl+3</td><td><pre>   </pre></td>"
                     "<td>Show/hide surface</td></tr>"
                     "<tr><td>Ctrl+4</td><td><pre>   </pre></td>"
                     "<td>Show/hide vectors</td></tr>"
                     "<tr><td>Ctrl+W</td><td><pre>   </pre></td>"
                     "<td>Toggle wireframe view</td></tr>"
                     "<tr><td>Ctrl+C</td><td><pre>   </pre></td>"
                     "<td>Center camera on surface</td></tr>"
                     "<tr><td>Ctrl+R</td><td><pre>   </pre></td>"
                     "<td>Reset the view</td></tr>"
                     "<tr><td>Ctrl+Shift+O</td><td><pre>   </pre></td>"
                     "<td>Load a texture</td></tr>"
                     "<tr><td>Ctrl+E</td><td><pre>   </pre></td>"
                     "<td>Open texture editor</td></tr>"
                     "<tr><td>Ctrl+T</td><td><pre>   </pre></td>"
                     "<td>Open turtle parameters window</td></tr>"
                     "<tr><td>Ctrl+Shift+A</td><td><pre>   </pre></td>"
                     "<td>Open adjacencies editor</td></tr>"
                     "<tr><td>Ctrl+Shift+T</td><td><pre>   </pre></td>"
                     "<td>Open translation dialog</td></tr>"
                     "<tr><td>Ctrl+Shift+R</td><td><pre>   </pre></td>"
                     "<td>Open rotation dialog</td></tr>"
                     "<tr><td>Ctrl+Shift+S</td><td><pre>   </pre></td>"
                     "<td>Open scaling dialog</td></tr>"
                     "<tr><td>Ctrl+Z</td><td><pre>   </pre></td>"
                     "<td>Undo last change</td></tr>"
                     "<tr><td>Ctrl+Y</td><td><pre>   </pre></td>"
                     "<td>Redo last undone change</td></tr>"
                     "<tr><td>Ctrl+N</td><td><pre>   </pre></td>"
                     "<td>Create a new surface</td></tr>"
                     "<tr><td>Ctrl+O</td><td><pre>   </pre></td>"
                     "<td>Open a surface file</td></tr>"
                     "<tr><td>Ctrl+S</td><td><pre>   </pre></td>"
                     "<td>Save the surface file</td></tr>"
                     "<tr><td>Ctrl+P</td><td><pre>   </pre></td>"
                     "<td>Edit preferences</td></tr>"
                     "<tr><td>Ctrl+Q</td><td><pre>   </pre></td>"
                     "<td>Exit program</td></tr></table>");
}

void BezierWindow::showAbout() {
  QMessageBox::about(this, "About",
                     "Bezier Editor\n\n"
                     "Author:\t"
                     "Mark Koleszar\n"
                     "Date:\t"
                     "May 2010");
}

// Makes the view menu appear on a right click in the bezier editor
void BezierWindow::menuize(const QPoint &pos) {
  // QMenu *menu = new QMenu;
  // menu->addAction(tr("Ashorrkj"), this, SLOT(showAbout()));
  preferencesMenu->exec(bezierEditor->mapToGlobal(pos));
}

// Makes the view menu appear on a right click in the bezier editor
void BezierWindow::menuizeAddButton(const QPoint &pos) {
  addButtonContextMenu->exec(addPatchButton->mapToGlobal(pos));
}

// Adds a patch to the patch list
void BezierWindow::addPatchToList(string name) {
  QListWidgetItem *temp = new QListWidgetItem(QString(name.c_str()));
  //temp->setCheckState(Qt::Checked);
  temp->setFlags(temp->flags() | Qt::ItemIsEditable);
  patchList->addItem(temp);
  if (patchList->count() > 1) {
    symmetry14YBox->setChecked(false);
    symmetry14YBox->setEnabled(false);
    symmetry14XBox->setChecked(false);
    symmetry14XBox->setEnabled(false);
    symmetry113YBox->setChecked(false);
    symmetry113YBox->setEnabled(false);
    symmetry113XBox->setChecked(false);
    symmetry113XBox->setEnabled(false);
  }
}

// Changes which patch is selected in the editor
void BezierWindow::selectPatch() {
  int index = patchList->currentRow();
  if (index >= 0)
    bezierEditor->selectPatch(index);
}

// Changes which patch is selected in the list
void BezierWindow::selectPatchInList(int index) {
  patchList->setCurrentRow(index);
}

// Removes all the patches from the patch list
void BezierWindow::clearPatchList() {
  patchList->clear();
  patchList->setCurrentRow(0);
  symmetry14YBox->setEnabled(true);
  symmetry14XBox->setEnabled(true);
  symmetry113YBox->setEnabled(true);
  symmetry113XBox->setEnabled(true);
}

// Changes the name of a patch when the text is changed, after ensuring that the
// name is valid
void BezierWindow::changePatchName(QListWidgetItem *item) {
  QStringList names = bezierEditor->getPatchNames();
  QString name =
      item->text().simplified().replace(' ', "_"); // No whitespace allowed
  if (name.isEmpty())
    ; 
  else if (names.contains(name))
    ;
  else {
    bezierEditor->changePatchName(name.toStdString(), patchList->currentRow());
    item->setText(name);
    return;
  }
  item->setText(names.at(patchList->currentRow()));
}

// Toggles the visibility of the selected patch based on the state of its check
// box
void BezierWindow::changePatchVisibility() {
  for (int i = 0; i < patchList->count(); ++i){
    bezierEditor->changePatchVisibility(false,i);
  }
  bezierEditor->changePatchVisibility(true,patchList->currentRow());

  //bezierEditor->changePatchVisibility(item->checkState() == Qt::Checked,
  //                                    patchList->currentRow());
  
}

// Adds a new square patch in the bezier editor
void BezierWindow::addNewSquarePatch() {
  bezierEditor->addNewPatch(Patch::Square);
}

// Adds a new triangular patch in the bezier editor
void BezierWindow::addNewTrianglePatch() {
  bezierEditor->addNewPatch(Patch::Triangle);
}

// Deletes the selected patch from the list and informs the editor to delete the
// actual patch
void BezierWindow::deleteSelectedPatch() {
  if (patchList->count() > 0) {
    bezierEditor->deleteSelectedPatch();
    QListWidgetItem *item = patchList->takeItem(patchList->currentRow());
    delete item;
    selectPatch();
    if (patchList->count() <= 1) {
      symmetry14YBox->setEnabled(true);
      symmetry14XBox->setEnabled(true);
      symmetry113YBox->setEnabled(true);
      symmetry113XBox->setEnabled(true);
    }
  }
}

// Duplicates the selected patch in the bezier editor
void BezierWindow::duplicateSelectedPatch() {
  bezierEditor->duplicateSelectedPatch();
}

// Open the preferences dialog
void BezierWindow::preferences() {
  BezierEditorPreferencesDialog *bepd = new BezierEditorPreferencesDialog(this);
  connect(bepd, SIGNAL(applied()), this, SLOT(readConfig()));
  bepd->setMaximumSize(1, 1);
  int ret = bepd->exec();
  if (ret)
    readConfig();
}

void BezierWindow::readConfig() {
  ifstream inFile;
#ifdef __APPLE__
  QString userConfigDir = Vlab::getUserConfigDir();
  userConfigDir.append("/stedit-bezier.cfg");
  inFile.open(userConfigDir.toStdString().c_str());
#else
  inFile.open("stedit-bezier.cfg");
#endif
  if (inFile.is_open()) {
    int val;
    int r, g, b;

    inFile >> r >> g >> b;
    defaultBgColour = QColor(r, g, b);
    bezierEditor->changeBgColour(
        Colour((double)r / 255.0, (double)g / 255.0, (double)b / 255.0));
    inFile >> r >> g >> b;
    defaultPointColour = QColor(r, g, b);
    bezierEditor->changePointColour(
        Colour((double)r / 255.0, (double)g / 255.0, (double)b / 255.0));
    inFile >> r >> g >> b;
    defaultSelectedPointColour = QColor(r, g, b);
    bezierEditor->changeSelectedPointColour(
        Colour((double)r / 255.0, (double)g / 255.0, (double)b / 255.0));
    inFile >> r >> g >> b;
    defaultLineColour = QColor(r, g, b);
    bezierEditor->changeLineColour(
        Colour((double)r / 255.0, (double)g / 255.0, (double)b / 255.0));
    inFile >> r >> g >> b;
    defaultWireframeColour = QColor(r, g, b);
    bezierEditor->changeWireframeColour(
        Colour((double)r / 255.0, (double)g / 255.0, (double)b / 255.0));
    inFile >> r >> g >> b;
    defaultVectorColour = QColor(r, g, b);
    bezierEditor->changeVectorColour(
        Colour((double)r / 255.0, (double)g / 255.0, (double)b / 255.0));
    inFile >> r >> g >> b;
    defaultPatchColour = QColor(r, g, b);
    bezierEditor->changePatchColour(
        Colour((double)r / 255.0, (double)g / 255.0, (double)b / 255.0));
    inFile >> r >> g >> b;
    defaultSelectedPatchColour = QColor(r, g, b);
    bezierEditor->changeSelectedPatchColour(
        Colour((double)r / 255.0, (double)g / 255.0, (double)b / 255.0));

    inFile >> val;
    bezierEditor->setPointSize(val);
    inFile >> val;
    bezierEditor->setContactPointSize(val);
    inFile >> val;
    bezierEditor->setLineWidth(val);
    inFile >> val;
    bezierEditor->setWireframeWidth(val);
    inFile >> val;
    bezierEditor->setSubdivisionSamples(val);
    inFile >> val;
    bezierEditor->setLinearInterpolation(val);
    inFile >> val;
    bezierEditor->setWireframe(val);
    setWireframeAct->setCheckable(true);
    setWireframeAct->setChecked(val);
    inFile >> val;
    bezierEditor->setProjection(val);

    inFile.close();
  } else {
    bezierEditor->changeBgColour(Colour());
    bezierEditor->changePointColour(Colour(1, 0, 0));
    bezierEditor->changeLineColour(Colour(1, 0, 0));
    bezierEditor->changeWireframeColour(Colour(102, 102, 153));
    bezierEditor->changeVectorColour(Colour(153, 0, 204));
    bezierEditor->changeSelectedPatchColour(Colour(1, 0, 0));
    bezierEditor->setPointSize(DEFAULT_POINT_SIZE);
    bezierEditor->setLineWidth(DEFAULT_LINE_WIDTH);
    bezierEditor->setWireframeWidth(DEFAULT_WIREFRAME_WIDTH);
    bezierEditor->setSubdivisionSamples(DEFAULT_SUBDIVISION_SAMPLES);
  }
}

// Point selection button slots
void BezierWindow::point1Toggled(bool checked) {
  if (checked && !externalSelectSignal) {
    resetButtons(1);
    bezierEditor->selectPoint(0, 0);
  }
}
void BezierWindow::point2Toggled(bool checked) {
  if (checked && !externalSelectSignal) {
    resetButtons(2);
    bezierEditor->selectPoint(1, 0);
  }
}
void BezierWindow::point3Toggled(bool checked) {
  if (checked && !externalSelectSignal) {
    resetButtons(3);
    bezierEditor->selectPoint(2, 0);
  }
}
void BezierWindow::point4Toggled(bool checked) {
  if (checked && !externalSelectSignal) {
    resetButtons(4);
    bezierEditor->selectPoint(3, 0);
  }
}
void BezierWindow::point5Toggled(bool checked) {
  if (checked && !externalSelectSignal) {
    resetButtons(5);
    bezierEditor->selectPoint(0, 1);
  }
}
void BezierWindow::point6Toggled(bool checked) {
  if (checked && !externalSelectSignal) {
    resetButtons(6);
    bezierEditor->selectPoint(1, 1);
  }
}
void BezierWindow::point7Toggled(bool checked) {
  if (checked && !externalSelectSignal) {
    resetButtons(7);
    bezierEditor->selectPoint(2, 1);
  }
}
void BezierWindow::point8Toggled(bool checked) {
  if (checked && !externalSelectSignal) {
    resetButtons(8);
    bezierEditor->selectPoint(3, 1);
  }
}
void BezierWindow::point9Toggled(bool checked) {
  if (checked && !externalSelectSignal) {
    resetButtons(9);
    bezierEditor->selectPoint(0, 2);
  }
}
void BezierWindow::point10Toggled(bool checked) {
  if (checked && !externalSelectSignal) {
    resetButtons(10);
    bezierEditor->selectPoint(1, 2);
  }
}
void BezierWindow::point11Toggled(bool checked) {
  if (checked && !externalSelectSignal) {
    resetButtons(11);
    bezierEditor->selectPoint(2, 2);
  }
}
void BezierWindow::point12Toggled(bool checked) {
  if (checked && !externalSelectSignal) {
    resetButtons(12);
    bezierEditor->selectPoint(3, 2);
  }
}
void BezierWindow::point13Toggled(bool checked) {
  if (checked && !externalSelectSignal) {
    resetButtons(13);
    bezierEditor->selectPoint(0, 3);
  }
}
void BezierWindow::point14Toggled(bool checked) {
  if (checked && !externalSelectSignal) {
    resetButtons(14);
    bezierEditor->selectPoint(1, 3);
  }
}
void BezierWindow::point15Toggled(bool checked) {
  if (checked && !externalSelectSignal) {
    resetButtons(15);
    bezierEditor->selectPoint(2, 3);
  }
}
void BezierWindow::point16Toggled(bool checked) {
  if (checked && !externalSelectSignal) {
    resetButtons(16);
    bezierEditor->selectPoint(3, 3);
  }
}
void BezierWindow::contactToggled(bool checked) {
  if (checked && !externalSelectSignal) {
    resetButtons(17);
    bezierEditor->selectContactPoint();
  }
}

void BezierWindow::resetButtons(int button) {
  for (int i = 0; i < 16; i++) {
    //if (button - 1 != i){
      pointButtons[i]->setChecked(false);
      pointButtons[i]->update();

      //}
  }
  if (button != 17){
    contactPointButton->setChecked(false);
    contactPointButton->update();
  }
}

void BezierWindow::toggleButton(int i, int j) {
  resetButtons(0);
  externalSelectSignal = true;
  pointButtons[i + 4 * j]->setChecked(true);
  pointButtons[i]->update();

  externalSelectSignal = false;
}

void BezierWindow::toggleContactButton() {
  resetButtons(0);
  externalSelectSignal = true;
  contactPointButton->setChecked(true);
  contactPointButton->update();

  externalSelectSignal = false;
}

void BezierWindow::set14XSymmetry(bool checked) {
  if (checked){
    symmetry14XBox->setChecked(false);
    symmetry113YBox->setChecked(false);
  }
  bezierEditor->setSymmetricalEditing(true, true, checked);
}

void BezierWindow::set14YSymmetry(bool checked) {
  if (checked)
    symmetry14YBox->setChecked(false);
  bezierEditor->setSymmetricalEditing(true, false, checked);
}

void BezierWindow::set113XSymmetry(bool checked) {
  if (checked)
    symmetry113XBox->setChecked(false);
  bezierEditor->setSymmetricalEditing(false, true, checked);
}

void BezierWindow::set113YSymmetry(bool checked) {
  if (checked){
    symmetry113YBox->setChecked(false);
    symmetry14XBox->setChecked(false);
  }
  bezierEditor->setSymmetricalEditing(false, false, checked);
}

// Auxilliary functions

// Creates a QDoubleSpinBox with steps of 0.1 and a range from -10 000 to 10 000
QDoubleSpinBox *BezierWindow::createSpinBox() {
  QDoubleSpinBox *box = new QDoubleSpinBox();
  box->setSingleStep(0.1);
  box->setRange(-10000, 10000);
  box->setDecimals(6);
  return box;
}

// Enables or disables the revert function, at signals indicating whether or not
// there is unsaved data to revert
void BezierWindow::enableRevert(bool value) { revertAct->setEnabled(value); }

void BezierWindow::help() {
  QDir helpDir(getHelpDirectory());
  QDesktopServices::openUrl(
      QUrl::fromLocalFile(helpDir.filePath("VLABToolsManual.pdf")));
}

void BezierWindow::pdfHelp() {
  QDir helpDir(getHelpDirectory());
  QDesktopServices::openUrl(
      QUrl::fromLocalFile(helpDir.filePath("VLABToolsManual.pdf")));
}

void BezierWindow::quickHelp(){
  QDir helpDir(getHelpDirectory());
#ifndef __APPLE__
  helpDir.cd("Quick_Help");
#endif
  QString path = helpDir.filePath("SteditBezierQuickHelp.html");
  QFile f(path);
  if (!f.open(QFile::ReadOnly | QFile::Text)) {
    std::cerr<<"Path: "<<path.toStdString()<<" doesn't exist"<<std::endl;
    return;
  }
  QTextStream in(&f);
  QString message = in.readAll();
  QTextBrowser *tb = new QTextBrowser(this);
  tb->setOpenExternalLinks(true);
  tb->setHtml(message);

  QDialog *msgBox = new QDialog;
  msgBox->setWindowTitle("Stedit: Quick Help");
  msgBox->setWindowFlags(Qt::Dialog);
  msgBox->setModal(false);
  QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok);
  QPushButton* okBtn = bb->button(QDialogButtonBox::Ok);
  connect(okBtn, SIGNAL(clicked()),msgBox,SLOT(close()));
  QVBoxLayout *layout = new QVBoxLayout;
  layout->addWidget(tb);
  layout->addWidget(bb);
  msgBox->setLayout(layout);
  msgBox->resize(400,300);
 
  msgBox->show();
}
