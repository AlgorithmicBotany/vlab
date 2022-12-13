/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#include "TextureWindow.h"
#include "about.h"
#include <QSizePolicy>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>
#include <QIcon>
#include <QFileDialog>
#include <QStyle>
#include <QColorDialog>
#include <QInputDialog>
#include <QImageWriter>
#include <QDesktopServices>
#include <QTextBrowser>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QTextStream>

#ifdef __APPLE__
#include "cocoabridge.h"
#endif

TextureWindow::TextureWindow(string textureName, SavingMode savingMode) {
#ifdef __APPLE__
  CocoaBridge::setAllowsAutomaticWindowTabbing(false);
#endif

  //setAttribute(Qt::WA_DeleteOnClose);
  textureEditor = new TextureEditor(this);
  textureEditor->setAttribute(Qt::WA_DeleteOnClose);
  textureEditor->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(textureEditor, SIGNAL(sendTexture(QImage)), this,
          SIGNAL(passTexture(QImage)));
  connect(textureEditor, SIGNAL(textureLoaded(string)), this,
          SIGNAL(textureLoaded(string)));
  connect(textureEditor, SIGNAL(currentOpenFile(string)), this,
          SLOT(updateActiveFile(string)));
  connect(textureEditor, SIGNAL(customContextMenuRequested(const QPoint &)),
          this, SLOT(menuize(const QPoint &)));

  textureDir = QString(textureName.c_str());
  if (!textureName.empty())
    textureEditor->preload(textureName);

  readConfig();

  // Set up the menus
  menuBar = new QMenuBar();
  fileMenu = new QMenu(("&File"), this);
  textureMenu = new QMenu(("&Texture"), this);
  projectMenu = new QMenu(("&Control points"), this);
  editMenu = new QMenu(("&Edit\1"), this);
  viewMenu = new QMenu(("&View\1"), this);
  preferencesMenu = new QMenu(("&Preferences"), this);
  colourMenu = new QMenu(("Color"), this);
  resizeMenu = new QMenu(("Resize"), this);
  bgColourMenu = new QMenu(("&Background"), this);
  pointColourMenu = new QMenu(("&Point"), this);
  lineColourMenu = new QMenu(("&Line"), this);
  showHideMenu = new QMenu(("&Show"), this);
  helpMenu = new QMenu(("&Help"), this);

  // Load icons
  QIcon newIcon = style()->standardIcon(QStyle::SP_DialogResetButton);
  QIcon saveIcon = style()->standardIcon(QStyle::SP_DialogSaveButton);
  QIcon saveAsIcon = style()->standardIcon(QStyle::SP_DialogSaveButton);
  QIcon openIcon = style()->standardIcon(QStyle::SP_DialogOpenButton);
  QIcon clearIcon = style()->standardIcon(QStyle::SP_TrashIcon);
  QIcon undoIcon = style()->standardIcon(QStyle::SP_ArrowLeft);
  QIcon redoIcon = style()->standardIcon(QStyle::SP_ArrowRight);
  QIcon revertIcon = style()->standardIcon(QStyle::SP_BrowserReload);
  QIcon captureIcon = style()->standardIcon(QStyle::SP_ComputerIcon);

  // Set up the menu actions
  textureMenu->addAction(saveIcon, "&Save", this, SLOT(save()),
                         QKeySequence(tr("CTRL+SHIFT+S")));
  textureMenu->addAction(saveAsIcon, "Save &As...", this, SLOT(saveAs()));
  textureMenu->addSeparator();
  textureMenu->addAction(openIcon, "&Open", this, SLOT(load()),
                         QKeySequence(tr("CTRL+SHIFT+O")));

  projectMenu->addAction(saveIcon, "&Save", this, SLOT(saveProject()),
                         QKeySequence(tr("CTRL+S")));
  projectMenu->addAction(saveAsIcon, "Save &As...", this,
                         SLOT(saveProjectAs()));
  projectMenu->addSeparator();
  projectMenu->addAction(openIcon, "&Open", this, SLOT(loadProject()),
                         QKeySequence(tr("CTRL+O")));

  fileMenu->addMenu(textureMenu);
  fileMenu->addMenu(projectMenu);
  fileMenu->addSeparator();
  fileMenu->addAction(newIcon, "&Reload", this, SLOT(newFile()),
                      QKeySequence(tr("CTRL+R")));
  revertAct = new QAction(revertIcon, "&Reload", this);
  connect(revertAct, SIGNAL(triggered()), this, SLOT(newFile()));
  //  connect(textureEditor, SIGNAL(canRevert(bool)), this,
  //        SLOT(enableRevert(bool)));
  revertAct->setEnabled(true);
  //fileMenu->addAction(revertAct);
  fileMenu->addSeparator();
  fileMenu->addAction("&Preferences...", this, SLOT(preferences()),
                      QKeySequence(tr("CTRL+P")));
  ///////
  QMenu *modeMenu = fileMenu->addMenu("Refresh mode");
  _savingMode = savingMode;
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

  textureEditor->setSavingMode(_savingMode);


  fileMenu->addSeparator();
  fileMenu->addAction("&Exit", this, SLOT(close()), QKeySequence(tr("CTRL+Q")));

  undoAct = new QAction(undoIcon, "&Undo", this);
  redoAct = new QAction(redoIcon, "&Redo", this);
  connect(undoAct, SIGNAL(triggered()), this, SLOT(undo()));
  connect(redoAct, SIGNAL(triggered()), this, SLOT(redo()));
  connect(textureEditor, SIGNAL(canUndo(bool)), this, SLOT(enableUndo(bool)));
  connect(textureEditor, SIGNAL(canRedo(bool)), this, SLOT(enableRedo(bool)));
  undoAct->setShortcut(QKeySequence(tr("CTRL+Z")));
  redoAct->setShortcut(QKeySequence(tr("CTRL+Y")));
  undoAct->setEnabled(false);
  redoAct->setEnabled(false);
  editMenu->addAction(undoAct);
  editMenu->addAction(redoAct);
  editMenu->addSeparator();
  editMenu->addAction("&Resize image", this, SLOT(resizeTexture()));
  editMenu->addAction("Rotate CW", textureEditor, SLOT(rotateCW()));
  editMenu->addAction("Rotate CCW", textureEditor, SLOT(rotateCCW()));
  editMenu->addAction("Flip &Horizontal", textureEditor, SLOT(flipH()));
  editMenu->addAction("Flip &Vertical", textureEditor, SLOT(flipV()));

  editMenu->addSeparator();
  editMenu->addAction(captureIcon, "&Capture", this, SLOT(capture()),
                      QKeySequence(tr("CTRL+C")));
  editMenu->addAction(clearIcon, "&Reset to captured", this, SLOT(reset()),
                      QKeySequence(tr("CTRL+R")));

  showPointsAct = new QAction("&Points", this);
  showPointsAct->setCheckable(true);
  showPointsAct->setChecked(true);
  showPointsAct->setShortcut(QKeySequence(tr("CTRL+1")));
  connect(showPointsAct, SIGNAL(toggled(bool)), this,
          SLOT(showHidePoints(bool)));
  showHideMenu->addAction(showPointsAct);

  showLinesAct = new QAction("&Lines", this);
  showLinesAct->setCheckable(true);
  showLinesAct->setChecked(true);
  showLinesAct->setShortcut(QKeySequence(tr("CTRL+2")));
  connect(showLinesAct, SIGNAL(toggled(bool)), this, SLOT(showHideLines(bool)));
  showHideMenu->addAction(showLinesAct);

  viewMenu->addAction(revertIcon, "&Reset", this, SLOT(resetView()));
  viewMenu->addMenu(showHideMenu);
  viewMenu->addSeparator();
  viewMenu->addMenu(resizeMenu);
  resizeMenu->addAction("&Point Size...", this, SLOT(changePointSize()));
  resizeMenu->addAction("&Line Width...", this, SLOT(changeLineWidth()));
  viewMenu->addMenu(colourMenu);
  colourMenu->addMenu(bgColourMenu);
  colourMenu->addMenu(pointColourMenu);
  colourMenu->addMenu(lineColourMenu);
  bgColourMenu->addAction("&Default", this, SLOT(changeBgDefault()));
  bgColourMenu->addAction("&Black", this, SLOT(changeBgBlack()));
  bgColourMenu->addAction("&White", this, SLOT(changeBgWhite()));
  bgColourMenu->addAction("&Custom...", this, SLOT(changeBgColour()));

  pointColourMenu->addAction("&Default", this, SLOT(changePointDefault()));
  pointColourMenu->addAction("&Red", this, SLOT(changePointRed()));
  pointColourMenu->addAction("&Green", this, SLOT(changePointGreen()));
  pointColourMenu->addAction("&Black", this, SLOT(changePointBlack()));
  pointColourMenu->addAction("&White", this, SLOT(changePointWhite()));
  pointColourMenu->addAction("&Custom...", this, SLOT(changePointColour()));
  colourMenu->addAction(revertIcon, "&Reset Defaults", this,
                             SLOT(resetColours()));


  lineColourMenu->addAction("&Default", this, SLOT(changeLineDefault()));
  lineColourMenu->addAction("&Red", this, SLOT(changeLineRed()));
  lineColourMenu->addAction("&Green", this, SLOT(changeLineGreen()));
  lineColourMenu->addAction("&Black", this, SLOT(changeLineBlack()));
  lineColourMenu->addAction("&White", this, SLOT(changeLineWhite()));
  lineColourMenu->addAction("&Custom...", this, SLOT(changeLineColour()));


  preferencesMenu->addAction("&Resize image", this, SLOT(resizeTexture()));
  preferencesMenu->addAction("Rotate CW", textureEditor, SLOT(rotateCW()));
  preferencesMenu->addAction("Rotate CCW", textureEditor, SLOT(rotateCCW()));
  preferencesMenu->addAction("Flip &Horizontal", textureEditor, SLOT(flipH()));
  preferencesMenu->addAction("Flip &Vertical", textureEditor, SLOT(flipV()));
  preferencesMenu->addSeparator();
  preferencesMenu->addAction(saveIcon, "&Save Texture", this, SLOT(save()));

  preferencesMenu->addAction(revertAct);

  //preferencesMenu->addMenu(bgColourMenu);
  //preferencesMenu->addMenu(pointColourMenu);
  //preferencesMenu->addMenu(lineColourMenu);
  //preferencesMenu->addSeparator();
  //preferencesMenu->addAction("&Point Size...", this, SLOT(changePointSize()));
  //preferencesMenu->addAction("&Line Width...", this, SLOT(changeLineWidth()));
  //preferencesMenu->addSeparator();
  QMenu *modeMenuPreferences = preferencesMenu->addMenu("Refresh mode");
  preferencesMenu->addSeparator();
  preferencesMenu->addAction("&Quit", this, SLOT(close()), QKeySequence(tr("CTRL+Q")));


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


  // Put the menus in the menu bar
  menuBar->addMenu(fileMenu);
  menuBar->addMenu(editMenu);
  menuBar->addMenu(viewMenu);
  menuBar->addMenu(helpMenu);
  QMenu *about = menuBar->addMenu("About");
  about->addAction("About", this, SLOT(showAbout()));

  // Create the layout
  QHBoxLayout *mainLayout = new QHBoxLayout;
  mainLayout->setContentsMargins(0, 0, 0, 0);

  // Put together the layout
  mainLayout->setMenuBar(menuBar);
  mainLayout->addWidget(textureEditor);
  textureEditor->setFocus();

  setLayout(mainLayout);
  updateActiveFile(textureName);

  // Initialize the lists of acceptable file formats, as provided by
  // QImageReader/Writer
  QList<QByteArray> loadFormatsList = QImageReader::supportedImageFormats();
  for (int i = 0; i < loadFormatsList.size(); i++) {
    loadFormats.append(QImageFormatStrings::getString(loadFormatsList.at(i)));
    loadFormats.append(" (*.");
    loadFormats.append(loadFormatsList.at(i));
    loadFormats.append(");;");
  }
  loadFormats.append("All Files (*.*);;");

  QList<QByteArray> saveFormatsList = QImageWriter::supportedImageFormats();
  for (int i = 0; i < saveFormatsList.size(); i++) {
    saveFormats.append(QImageFormatStrings::getString(saveFormatsList.at(i)));
    saveFormats.append(" (*.");
    saveFormats.append(saveFormatsList.at(i));
    saveFormats.append(");;");
  }
  saveFormats.append("All Files (*.*);;");

  projectFormats = "Warp Files (*.dtx);;All Files (*.*);;";
  connect(textureEditor, SIGNAL(continuousSave()), this, SLOT(save()));
}

TextureWindow::~TextureWindow() {}

void TextureWindow::updateContinuousMode(SavingMode mode) {
  if (mode == CONTINUOUS)
    ContinuousSavingMode();
  else if (mode == TRIGGERED)
    TriggeredSavingMode();
  else
    ModeOff();
}

void TextureWindow::ContinuousModeMenu() {
  ContinuousSavingMode();
  updatingContMode(CONTINUOUS);
}

void TextureWindow::TriggeredModeMenu() {
  TriggeredSavingMode();
  updatingContMode(TRIGGERED);
}

void TextureWindow::OffModeMenu() {
  ModeOff();
  updatingContMode(OFF);
}

void TextureWindow::ContinuousSavingMode() {
  textureEditor->setSavingMode(CONTINUOUS);
  _savingMode = CONTINUOUS;
  _savingContinu_act->setChecked(true);
  _savingTriggered_act->setChecked(false);
  _savingMenu_act->setChecked(false);

  _savingContinuP_act->setChecked(true);
  _savingTriggeredP_act->setChecked(false);
  _savingMenuP_act->setChecked(false);
}

void TextureWindow::TriggeredSavingMode() {
  textureEditor->setSavingMode(TRIGGERED);

  _savingMode = TRIGGERED;
  _savingContinu_act->setChecked(false);
  _savingTriggered_act->setChecked(true);
  _savingMenu_act->setChecked(false);

  _savingContinuP_act->setChecked(false);
  _savingTriggeredP_act->setChecked(true);
  _savingMenuP_act->setChecked(false);
}

void TextureWindow::ModeOff() {
  textureEditor->setSavingMode(OFF);
  _savingMode = OFF;
  _savingContinu_act->setChecked(false);
  _savingTriggered_act->setChecked(false);
  _savingMenu_act->setChecked(true);

  _savingContinuP_act->setChecked(false);
  _savingTriggeredP_act->setChecked(false);
  _savingMenuP_act->setChecked(true);
}

QSize TextureWindow::sizeHint() const { return QSize(200, 200); }

// Slots

void TextureWindow::updateActiveFile(string str) {
  QString filename = str.c_str();
  if (filename.length() > 0) {
    setWindowTitle(
        QString("Texture Distorter - ")
            .append(filename.remove(0, filename.lastIndexOf("/") + 1)));
  } else {
    setWindowTitle("Texture Distorter");
  }
}

// Provides a dialog box to choose a texture to load
void TextureWindow::load() {
  if (textureEditor->unsavedChanges) {
    QMessageBox box;
    bool saved;
    box.setIcon(QMessageBox::Warning);
    box.setWindowTitle("Texture Distorter");
    box.setText("The texture has been modified");
    box.setInformativeText("Do you want to save your texture, continue without "
                           "saving, or discard changes?");
    box.setStandardButtons(QMessageBox::Save  |
                           QMessageBox::Discard | QMessageBox::Cancel);
    box.setDefaultButton(QMessageBox::Save);
    box.setEscapeButton(QMessageBox::Cancel);
    int ret = box.exec();
    switch (ret) {
    case QMessageBox::Ok: // Ok was clicked
      saved = true; // Act as if the texture was saved; i.e. keep it in the
                    // bezier editor
      save();
      break;
    case QMessageBox::Save: // Save was clicked
      saved = true;
      save();
      break;
    case QMessageBox::Discard: // Don't Save was clicked
      break;
    case QMessageBox::Cancel: // Cancel was clicked
      return;
      break;
    default: // should never be reached
      break;
    }
  }
  QString *formatString =
      new QString(" (*." + textureDir.section('.', -1) + ")");
  formatString->prepend(
      QImageFormatStrings::getString(textureDir.section('.', -1)));

  QString qfilename = QFileDialog::getOpenFileName(
      0, tr("Load Texture"), textureDir, loadFormats, formatString);

  if (!qfilename.isNull()) {
    textureDir = qfilename;
    string fname = qfilename.toStdString();
    textureEditor->load(fname);
  }
  textureEditor->unsavedChanges = false;

}

// Provides a dialog box to choose a project to load
void TextureWindow::loadProject() {
  QString qfilename = QFileDialog::getOpenFileName(0, tr("Load Project"),
                                                   projectDir, projectFormats);

  if (!qfilename.isNull()) {
    projectDir = qfilename;
    string fname = qfilename.toStdString();
    textureEditor->loadProject(fname);
  }
}

// Save the texture over the previously saved texture, or open save as if there
// is none
void TextureWindow::save() {
  if (textureEditor->noFilename())
    saveAs();
  else
    textureEditor->saveTexture();
  emit continuousSave();
}

// Save the project over the previously saved project, or open save as if there
// is none Return whether this was successful or not so that the program doesn't
// exit if the user cancels on save and exit
bool TextureWindow::saveProject() {
  if (textureEditor->noProjectFilename())
    return saveProjectAs();
  else {
    textureEditor->saveProject();
    return true;
  }
}

// Provides a dialog box to save a new texture, or save over an old one
void TextureWindow::saveAs() {
  QString *formatString =
      new QString(" (*." + textureDir.section('.', -1) + ")");
  formatString->prepend(
      QImageFormatStrings::getString(textureDir.section('.', -1)));

  QString qfilename = QFileDialog::getSaveFileName(
      0, tr("Save Texture"), textureDir, saveFormats, formatString);

  if (!qfilename.isNull()) {
    textureDir = qfilename;
    string fname = qfilename.toStdString();
    textureEditor->saveTexture(fname);
  }
}

// Provides a dialog box to save a new project, or save over an old one
bool TextureWindow::saveProjectAs() {
  QString qfilename = QFileDialog::getSaveFileName(0, tr("Save Project"),
                                                   projectDir, projectFormats);

  if (!qfilename.isNull()) {
    projectDir = qfilename;
    string fname = qfilename.toStdString();
    textureEditor->saveProject(fname);
    return true;
  }
  return false;
}

// Reloads the previously loaded project
void TextureWindow::revert() {
  if (!textureEditor->noProjectFilename() && textureEditor->unsavedChanges) {
    QMessageBox box;
    box.setWindowTitle("Revert Texture");
    box.setText(
        "Are you sure you want to revert to the most recent saved version?");
    box.setInformativeText("All unsaved changes will be lost");
    box.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    box.setDefaultButton(QMessageBox::Cancel);
    box.setEscapeButton(QMessageBox::Cancel);
    int ret = box.exec();
    switch (ret) {
    case QMessageBox::Ok: // Ok was clicked
      textureEditor->reloadProject();
      break;
    case QMessageBox::Cancel: // Cancel was clicked
      break;
    default: // should never be reached
      break;
    }
  }
}

// Prompts the user to save the currently open project if changes have been made
// without saving
void TextureWindow::closeEvent(QCloseEvent *ce) {
  if (textureEditor->unsavedChanges) {
    QMessageBox box;
    bool saved;
    box.setIcon(QMessageBox::Warning);
    box.setWindowTitle("Texture Distorter");
    box.setText("The texture has been modified");
    box.setInformativeText("Do you want to save your texture, continue without "
                           "saving, or discard changes?");
    box.setStandardButtons(QMessageBox::Save | QMessageBox::Ok |
                           QMessageBox::Discard | QMessageBox::Cancel);
    box.setDefaultButton(QMessageBox::Save);
    box.setEscapeButton(QMessageBox::Cancel);
    int ret = box.exec();
    switch (ret) {
    case QMessageBox::Ok: // Ok was clicked
      saved = true; // Act as if the texture was saved; i.e. keep it in the
                    // bezier editor
      emit(textureEditorClosed(true));
      ce->accept();
      break;
    case QMessageBox::Save: // Save was clicked
      saved = saveProject();
      if (saved) {
        emit(textureEditorClosed(true));
        ce->accept();
      } else
        ce->ignore();
      break;
    case QMessageBox::Discard: // Don't Save was clicked
      emit(textureEditorClosed(false));
      ce->accept();
      break;
    case QMessageBox::Cancel: // Cancel was clicked
      ce->ignore();
      break;
    default: // should never be reached
      break;
    }
  } else
    emit(textureEditorClosed(false));
  //  std::cerr<<"Delete texture editor"<<std::endl;
  //delete(textureEditor);
  //textureEditor = NULL;
}

void TextureWindow::undo() { textureEditor->undo(); }

void TextureWindow::redo() { textureEditor->redo(); }

void TextureWindow::capture() { textureEditor->capture(); }

void TextureWindow::resizeTexture() { textureEditor->resizeTexture(); }

void TextureWindow::reset() { textureEditor->reset(); }

// Reset everything to its initial state so that a new texture can be created
void TextureWindow::newFile() {
  if (textureEditor->unsavedChanges) {
    QMessageBox box;
    bool saved;
    box.setIcon(QMessageBox::Warning);
    box.setWindowTitle("Texture Distorter");
    box.setText("The texture has been modified");
    box.setInformativeText("Do you want to save your texture, continue without "
                           "saving, or discard changes?");
    box.setStandardButtons(QMessageBox::Save |
                           QMessageBox::Discard | QMessageBox::Cancel);
    box.setDefaultButton(QMessageBox::Save);
    box.setEscapeButton(QMessageBox::Cancel);
    int ret = box.exec();
    switch (ret) {
    case QMessageBox::Save: // Save was clicked
      saved = saveProject();
      break;
    case QMessageBox::Cancel: // Cancel was clicked
      return;
      break;
    case QMessageBox::Discard: // Don't Save was clicked
    default: // should never be reached
      break;
    }
  }
  textureEditor->clearProjectFilename();
  textureEditor->reset();
  resetColours();
  textureEditor->reloadTexture();
  textureEditor->clearFilename(); // Reset the filename so that saving doesn't
                                  // overwrite anything
  textureEditor->unsavedChanges = false;
}

// Reset everything to its initial state so that a new texture can be created
void TextureWindow::initEditor() {
  // doesn't overwrite anything
    
  textureEditor->clearProjectFilename();
  textureEditor->reset();
  resetColours();
  textureEditor->unsavedChanges = false;
  textureEditor->show();
}

// Load the image with the given filename, at a signal from the bezier editor
void TextureWindow::useImage(string filename) {
  textureEditor->load(filename);
  textureDir = QString(filename.c_str());
}

void TextureWindow::useImage(QImage image) { textureEditor->load(image); }

void TextureWindow::showHidePoints(bool value) {
  textureEditor->showHidePoints(value);
}

void TextureWindow::showHideLines(bool value) {
  textureEditor->showHideLines(value);
}

void TextureWindow::showControlsHelp() {
  QMessageBox::about(
      this, "Controls",
      // sorry, can't use cellspacing here since it adds vertical space
      // and to force the alignment we have to make a ghost column... ahh HTML,

      "<table>"
      "<tr><td>Ctrl Click</td><td><pre>   </pre></td>"
      "<td>Add control points</td></tr>"
      "<tr><td>Left Click</td><td><pre>   </pre></td>"
      "<td>Select and drag control points</td></tr>"
      "<tr><td>Ctrl+1</td><td><pre>   </pre></td>"
      "<td>Show/hide control points</td></tr>"
      "<tr><td>Ctrl+2</td><td><pre>   </pre></td>"
      "<td>Show/hide lines</td></tr>"
      "<tr><td>Ctrl+Z</td><td><pre>   </pre></td>"
      "<td>Undo last added point</td></tr>"
      "<tr><td>Ctrl+Y</td><td><pre>   </pre></td>"
      "<td>Redo last added point</td></tr>"
      "<tr><td>Ctrl+R</td><td><pre>   </pre></td>"
      "<td>Reset texture and control points</td></tr>"
      "<tr><td>Ctrl+C</td><td><pre>   </pre></td>"
      "<td>Capture current texture and reset control points</td></tr>"
      "<tr><td>Ctrl+N</td><td><pre>   </pre></td>"
      "<td>Start a new project file</td></tr>"
      "<tr><td>Ctrl+O</td><td><pre>   </pre></td>"
      "<td>Open a project file</td></tr>"
      "<tr><td>Ctrl+S</td><td><pre>   </pre></td>"
      "<td>Save the project file</td></tr>"
      "<tr><td>Ctrl+Shift+O</td><td><pre>   </pre></td>"
      "<td>Open a texture file</td></tr>"
      "<tr><td>Ctrl+Shift+S</td><td><pre>   </pre></td>"
      "<td>Save the texture file</td></tr>"
      "<tr><td>Ctrl+P</td><td><pre>   </pre></td>"
      "<td>Edit preferences</td></tr>"
      "<tr><td>Ctrl+Q</td><td><pre>   </pre></td>"
      "<td>Exit program</td></tr>"
      "</table>");
}

void TextureWindow::showAbout() {
  vlab::about(this,"Stedit");
  return;
  QMessageBox::about(this, "About",
                     "Texture Distorter\n\n"
                     "Author:\t"
                     "Mark Koleszar\n"
                     "Date:\t"
                     "May 2010");
}

// Opens a colour picker to choose a new background colour
void TextureWindow::changeBgColour() {
  QColor colour = QColorDialog::getColor(
      textureEditor->getBgColour().toQColor(), this, "Pick Background Color",
      QColorDialog::DontUseNativeDialog);
  if (colour.isValid())
    textureEditor->changeBgColour(Colour(colour));
}

void TextureWindow::changeBgDefault() {
  textureEditor->changeBgColour(defaultBgColour);
}
void TextureWindow::changeBgBlack() {
  textureEditor->changeBgColour(Colour(0, 0, 0));
}
void TextureWindow::changeBgWhite() {
  textureEditor->changeBgColour(Colour(1, 1, 1));
}

// Opens a colour picker to choose a new point colour
void TextureWindow::changePointColour() {
  QColor colour = QColorDialog::getColor(
      textureEditor->getPointColour().toQColor(), this, "Pick Point Color",
      QColorDialog::DontUseNativeDialog);
  if (colour.isValid())
    textureEditor->changePointColour(Colour(colour));
}

void TextureWindow::changePointDefault() {
  textureEditor->changePointColour(defaultPointColour);
}
void TextureWindow::changePointRed() {
  textureEditor->changePointColour(Colour(1, 0, 0));
}
void TextureWindow::changePointGreen() {
  textureEditor->changePointColour(Colour(0, 1, 0));
}
void TextureWindow::changePointBlack() {
  textureEditor->changePointColour(Colour(0, 0, 0));
}
void TextureWindow::changePointWhite() {
  textureEditor->changePointColour(Colour(1, 1, 1));
}

// Opens a colour picker to choose a new line colour
void TextureWindow::changeLineColour() {
  QColor colour = QColorDialog::getColor(
      textureEditor->getLineColour().toQColor(), this, "Pick Line Color",
      QColorDialog::DontUseNativeDialog);
  if (colour.isValid())
    textureEditor->changeLineColour(Colour(colour));
}

void TextureWindow::changeLineDefault() {
  textureEditor->changeLineColour(defaultLineColour);
}
void TextureWindow::changeLineRed() {
  textureEditor->changeLineColour(Colour(1, 0, 0));
}
void TextureWindow::changeLineGreen() {
  textureEditor->changeLineColour(Colour(0, 1, 0));
}
void TextureWindow::changeLineBlack() {
  textureEditor->changeLineColour(Colour(0, 0, 0));
}
void TextureWindow::changeLineWhite() {
  textureEditor->changeLineColour(Colour(1, 1, 1));
}

// Resets all the colours to defaults
void TextureWindow::resetColours() { readConfig(); }

// Provides a dialog box for choosing a new point size
void TextureWindow::changePointSize() {
  textureEditor->setPointSize(QInputDialog::getInt(
      0, "Set Point Size", "Size", textureEditor->getPointSize(), 1, 20));
}

// Provides a dialog box for choosing a new line width
void TextureWindow::changeLineWidth() {
  textureEditor->setLineWidth(QInputDialog::getInt(
      0, "Set Line Width", "Width", textureEditor->getLineWidth(), 1, 20));
}

void TextureWindow::resetView() {
  showPointsAct->setChecked(true);
  showLinesAct->setChecked(true);
  textureEditor->resetView();
}

// Open the preferences dialog
void TextureWindow::preferences() {
  TextureEditorPreferencesDialog *tepd =
      new TextureEditorPreferencesDialog(this);
  connect(tepd, SIGNAL(applied()), this, SLOT(readConfig()));
  tepd->setMaximumSize(1, 1);
  int ret = tepd->exec();
  if (ret)
    readConfig();
}

void TextureWindow::readConfig() {
  ifstream inFile;
#ifdef __APPLE__
  QString userConfigDir = Vlab::getUserConfigDir();
  userConfigDir.append("/stedit-warp.cfg");
  inFile.open(userConfigDir.toStdString().c_str());
#else
  inFile.open("stedit-warp.cfg");
#endif
  if (inFile.is_open()) {
    int val;
    int r, g, b;

    inFile >> r >> g >> b;
    defaultBgColour = QColor(r, g, b);
    textureEditor->changeBgColour(
        Colour((double)r / 255.0, (double)g / 255.0, (double)b / 255.0));
    inFile >> r >> g >> b;
    defaultPointColour = QColor(r, g, b);
    textureEditor->changePointColour(
        Colour((double)r / 255.0, (double)g / 255.0, (double)b / 255.0));
    inFile >> r >> g >> b;
    defaultLineColour = QColor(r, g, b);
    textureEditor->changeLineColour(
        Colour((double)r / 255.0, (double)g / 255.0, (double)b / 255.0));
    inFile >> val;
    textureEditor->setPointSize(val);
    inFile >> val;
    textureEditor->setLineWidth(val);
    inFile >> val;
    textureEditor->setLinearInterpolation(val);

    inFile.close();
  } else {
    textureEditor->changeBgColour(Colour());
    textureEditor->changePointColour(Colour(1, 0, 0));
    textureEditor->changeLineColour(Colour(1, 0, 0));
    textureEditor->setPointSize(DEFAULT_POINT_SIZE);
    textureEditor->setLineWidth(DEFAULT_LINE_WIDTH);
  }
}

// Public slots

// Enables or disables the undo function, at signals indicating whether or not
// undoing is possible
void TextureWindow::enableUndo(bool value) { undoAct->setEnabled(value); }

// Enables or disables the redo function, at signals indicating whether or not
// redoing is possible
void TextureWindow::enableRedo(bool value) { redoAct->setEnabled(value); }

// Enables or disables the revert function, at signals indicating whether or not
// there is unsaved data to revert
void TextureWindow::enableRevert(bool value) { revertAct->setEnabled(value); }

// Makes the view menu appear on a right click in the texture editor
void TextureWindow::menuize(const QPoint &pos) {
  preferencesMenu->exec(textureEditor->mapToGlobal(pos));
}
void TextureWindow::help() {
  //  OpenHelpBook();
  QDir helpDir(getHelpDirectory());
  QDesktopServices::openUrl(
      QUrl::fromLocalFile(helpDir.filePath("VLABToolsManual.pdf")));
}

void TextureWindow::pdfHelp() {
 QDir helpDir(getHelpDirectory());
#ifdef __APPLE__
  QDesktopServices::openUrl(
      QUrl::fromLocalFile(helpDir.filePath("VLABToolsManual.pdf")));
#else
  QDesktopServices::openUrl(
      QUrl::fromLocalFile(helpDir.filePath("../docs/cpfg/VLABToolsManual.pdf")));
#endif
}

void TextureWindow::quickHelp(){
  QDir helpDir(getHelpDirectory());
  QString path = helpDir.filePath("SteditWarpQuickHelp.html");
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
