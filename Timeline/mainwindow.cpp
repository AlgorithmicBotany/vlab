#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "timeline.h"
#include "addpointdialog.h"
#include "deleteconfirm.h"
#include "editpointdialog.h"
#include "helpdialog.h"
#include "Preferences.h"
#include "about.h"

#include <directorywatcher.h>
#include "resources.h"
#include <QDesktopServices>
#include <QDir>


#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QFileDialog>
#include <QString>
#include <QScrollArea>
#include <QScreen>
#include <QMenu>
#include <QMenuBar>
#include <QDesktopWidget>
#include <QTimer>
#include <QMessageBox>
#include <fstream>
#include <stdio.h>
#include <cmath>
#include <QTextBrowser>
#include <QTextStream>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <iostream>
#include <QPushButton>

MainWindow::MainWindow(int argc, char *argv[], QWidget *parent)
  : QMainWindow(parent), ui(new Ui::MainWindow),_directoryWatcher(NULL) {
  _change = false;
  _new_save_pending = false;
  _selectShift = false;
  _selectedNb = 0;
  _mode = 0;
  std::string filename = "";
  _savingMode = OFF;
  int argNb = 1;
  while (argNb < argc) {
    if (argv[argNb][0] == '-') {
	
      if ((strcmp(argv[argNb], "--refreshMode") == 0) ||
          (strcmp(argv[argNb], "-rmode") == 0)) {
	++argNb;
	if (argNb > argc){
	  std::cout
            << "Wrong options: " << argv[0]
            << " - usage TimeLine filename [--refreshMode|-rmode cont|trig|exp]"
            << std::endl;
	  exit(EXIT_FAILURE);
	}
        const char *opt = argv[argNb];
        if ((strcmp(opt, "exp") == 0) || (strcmp(opt, "explicit") == 0))
          _savingMode = OFF;
        if ((strcmp(opt, "cont") == 0) || (strcmp(opt, "continuous") == 0))
          _savingMode = CONTINUOUS;
        if ((strcmp(opt, "trig") == 0) || (strcmp(opt, "triggered") == 0))
          _savingMode = TRIGGERED;
        ++argNb;

      } else {
        std::cout
            << "Wrong options: " << argv[0]
            << " - usage TimeLine filename [--refreshMode|-rmode cont|trig|exp]"
            << std::endl;
        exit(EXIT_FAILURE);
      }
    }
    else{
      filename = std::string(argv[argNb]);
      ++argNb;
    }
  }  
  ui->setupUi(this);
  // Create the timeline from command line params
  setPreferences();
  timeline = new Timeline(filename, window);
  timeline->show();
  timeline->setFocus();
  connect(window, SIGNAL(configChanged()), timeline, SLOT(loadConfig()));
  connect(timeline, SIGNAL(change()), this, SLOT(change()));

  scrollArea = new QScrollArea;
  scrollArea->setWidget(timeline);
  scrollArea->setWidgetResizable(true);
  setCentralWidget(scrollArea);
  scrollArea->resize(10,10);
  scrollArea->show();

  // create and set up the timer for the IdleFunction
  _idleTimer = new QTimer(this);
  connect(_idleTimer, SIGNAL(timeout()), SLOT(Idle()));


  setWindowTitle(tr("Timeline"));

  _selected = false;
  _selectedIndex = -1;


  createActions();
  createMenus();
  QList<QScreen *> screenList = QGuiApplication::screens();

  QScreen *screen = screenList.at(0);
  int h =  screen->geometry().height();
  if (timeline->yMax < h){
    this->resize(timeline->width() + 50, timeline->yMax + 80);
  }
  else
    this->resize(timeline->width() + 50, h / 2);

  if (_savingMode == CONTINUOUS) {
    _savingContinu_act->setChecked(true);
    _savingTriggered_act->setChecked(false);
    _savingMenu_act->setChecked(false);
    ContinuousSavingMode();
  } else if (_savingMode == TRIGGERED) {
    _savingContinu_act->setChecked(false);
    _savingTriggered_act->setChecked(true);
    _savingMenu_act->setChecked(false);
    TriggeredSavingMode();
  } else {
    _savingTriggered_act->setChecked(false);
    _savingContinu_act->setChecked(false);
    _savingMenu_act->setChecked(true);
    ModeOff();
  }
  _leftClick = false;
  timeline->createAxis();

}

MainWindow::~MainWindow() {
  delete timeline;
  delete ui;
}

void MainWindow::closeEvent(QCloseEvent * event) {
  int ret = QMessageBox::No;
  if (_change) {
    ret = QMessageBox::warning(
        this, "Save At Exit", "Save changes before exit?",
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
        QMessageBox::Cancel);
  

    switch (ret) {
    case QMessageBox::Yes:
      this->save();
      break;
    case QMessageBox::No:
      _change = false;
      break;
    case QMessageBox::Cancel:
    default:
      event->ignore();
      return ;
    }
  }
  // Turn off continuous mode before clean up.
  // Otherwise, timeline tries to save the .tset file
  // in this close event. This is because on Linux-based
  // systems directoryWatcher monitors for removal of files
  // and will emit a fileChanged signal during clean up.
  ModeOff();
  timeline->cleanUp();
}

void MainWindow::ContinuousSavingMode() {
  timeline->setSavingMode(CONTINUOUS);

  if ( _directoryWatcher){
    delete _directoryWatcher;
    _directoryWatcher = NULL;
  }
  if ( !_directoryWatcher) {
    QStringList ignoredPrefixes;
    QStringList ignoredSuffixes;
    std::string path = (QDir::currentPath()).toStdString() + "/" + timeline->getTmpDir();

    _directoryWatcher =
      new DirectoryWatcher(QString::fromStdString(path),
			   ignoredPrefixes << ".", ignoredSuffixes, this);

    connect(_directoryWatcher, SIGNAL(fileChanged(QString)), this,
            SLOT(RequestSave()));
  }

  _savingMode = CONTINUOUS;
  _savingContinu_act->setChecked(true);
  _savingTriggered_act->setChecked(false);
  _savingMenu_act->setChecked(false);
}

void MainWindow::TriggeredSavingMode() {
  timeline->setSavingMode(TRIGGERED);
  if ( !_directoryWatcher) {
    QStringList ignoredPrefixes;
    QStringList ignoredSuffixes;
    std::string path = (QDir::currentPath()).toStdString() + "/" + timeline->getTmpDir();
    _directoryWatcher =
      new DirectoryWatcher(QString::fromStdString(path),
			   ignoredPrefixes << ".", ignoredSuffixes, this);
    connect(_directoryWatcher, SIGNAL(fileChanged(QString)), this,
            SLOT(RequestSave()));
  }

  _savingMode = TRIGGERED;
  _savingContinu_act->setChecked(false);
  _savingTriggered_act->setChecked(true);
  _savingMenu_act->setChecked(false);
}

void MainWindow::ModeOff() {
  timeline->setSavingMode(OFF);
  if ( _directoryWatcher) {
    delete _directoryWatcher;
    _directoryWatcher = NULL;
  }

  _savingMode = OFF;
  _savingContinu_act->setChecked(false);
  _savingTriggered_act->setChecked(false);
  _savingMenu_act->setChecked(true);
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
  if (_mode == 0){ // execute mode
    mousePressEventInExecuteMode(event);
    int count = 0;
    for (Event *e : timeline->events) {
      if (e->startSelected || e->endSelected){
	count = 1;
	break;
      }
    }
    if (count > 0)
      deselectAllAct->setEnabled(true);
    else
      deselectAllAct->setEnabled(false);
      

    return;
  }
  
  
  QWidget *topWidget = QApplication::topLevelAt(this->mapToGlobal(QPoint()));
  topWidget->raise();

  prevMouseX = (float)event->pos().x();
  prevMouseY = (float)event->pos().y();
  prevMouse = QCursor::pos();
  hasMoved = false;

  
  if (event->button() == Qt::RightButton) {
    int index = _selectedIndex;
    _selectedIndex = timeline->getEventIndex(prevMouse);
  // right click on a timeline
    if (_selectedIndex != -1) {
      //std::cerr<<"deselect time lines"<<std::endl;
      deselectTimelines();
      _selectedIndex = timeline->getEventIndex(prevMouse);
      Event *event = timeline->events[_selectedIndex];
      _selected = true;
      event->setSelected(true);
      moveEventUpAct->setEnabled(true);
      moveEventDownAct->setEnabled(true);
      editEventAct->setEnabled(true);
      deselectAllAct->setEnabled(true);
      //std::cerr<<"Trying popup menu"<<std::endl;
      openFunctionAct->setEnabled(true);
      menu->exec(QCursor::pos());
      //std::cerr<<"Didn't crash"<<std::endl;
      return;
    }
    _selectedIndex = index;
    // right click outside timeline
    //check how many timelines are selected
    int count = timeline->nbSelected();
    if (count > 1){
      moveEventUpAct->setEnabled(false);
      moveEventDownAct->setEnabled(false);
      editEventAct->setEnabled(false);
      deselectAllAct->setEnabled(false);
    }
    if (count == 1){
      moveEventUpAct->setEnabled(true);
      moveEventDownAct->setEnabled(true);
      editEventAct->setEnabled(true);
      deselectAllAct->setEnabled(true);
   }

    //return;
  }
 if (_mode == 1){ // execute mode
    mousePressEventInEditMode(event);
    //return;
  }
 
  // left click on a timeline
  if (event->button() == Qt::LeftButton) {
    mouseLeftPressed = true;
    if (Qt::ShiftModifier == QApplication::keyboardModifiers()) {
      if (timeline->isPointSelected()) {
	timeline->deselectPoint();
	// check if there are still point selected
	return;
      }
      _selected = true;
      deselectAllAct->setEnabled(true);
      timeline->selectPoint();
      return;
    }
    //left click on a point
    if (timeline->isPoint()){
      if (!timeline->isPointSelected()) {
	deselectAll();
      }
      timeline->selectPoint();
      _selected = true;
      editEventAct->setEnabled(true);
      deselectAllAct->setEnabled(true);

      deleteSelectedAct->setEnabled(true);
      moveEventUpAct->setEnabled(true);
      moveEventDownAct->setEnabled(true);
      openFunctionAct->setEnabled(true);
	
    }  
    else{// no point selected
      deselectAll();
      _selected = false;
      editEventAct->setEnabled(false);
      deselectAllAct->setEnabled(false);
      deleteSelectedAct->setEnabled(false);
      moveEventUpAct->setEnabled(false);
      moveEventDownAct->setEnabled(false);
      openFunctionAct->setEnabled(false);

    }
    return;
  }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event) {
  if (Qt::ShiftModifier != QApplication::keyboardModifiers()) {
    if ((_mode == 0)&&!_selectShift){ // execute mode
 	deselectAll();
      }
    }

  if (_leftClick){
    _leftClick = false;
    return;
  }
  // Handles left and right mouse clicks for selection and deselection
  // Stores the position of the mouse

  if ((event->button() == Qt::RightButton)) {
    return;
  }
  // Handles mouse release events
  if (event->button() == Qt::LeftButton) {
    mouseLeftPressed = false;
  }
  timeline->releaseSelected();

  if ((_savingMode == TRIGGERED) || (_savingMode == CONTINUOUS)) {
    save();
  }

  return ;
}

void MainWindow::mouseMoveEvent(QMouseEvent *event) {

  
  // Handles mouse move events
  // Find selected events
  if ((event->button() == Qt::RightButton)) {
    return;
  }

  if ((event->button() == Qt::MiddleButton)) {
    return;
  }

  /*
  if (timeline->isPoint()) {
    if ((Qt::ShiftModifier == QApplication::keyboardModifiers()))
      std::cerr<<"Mouse move with shift"<<std::endl;

  }
  */

  hasMoved = true;
  _change = true;
  // Update stored mouse position
  float dt = ((float)event->pos().x() - prevMouseX);
  float dty = ((float)event->pos().y() - prevMouseY);
  prevMouseX = (float)event->pos().x();
  if (mouseLeftPressed) {
    if ((_selected)&&(Qt::ShiftModifier != QApplication::keyboardModifiers())) {
      // Move selected points
      timeline->moveSelected(dt);
      hasMoved = true;

    } else if (Qt::ControlModifier == QApplication::keyboardModifiers()) {
      // No points selected,  zoom in-out
      prevMouseY = (float)event->pos().y();

      // Clamp movement to an integer distance
      int zoom = 1;

      timeline->xMax += zoom * dty;
      if (timeline->xMax < 1.0)
        timeline->xMax = 1.0;

      timeline->createAxis();
    }

    else {
      // No points selected, instead pan the axis
      float range = timeline->xMax - timeline->xMin;
      // Clamp movement to an integer distance
      float xMovement = (dt / this->size().width() * range);

      /*
      if (xMovement > 0.0 && xMovement < 1.0) {
        xMovement = 1.0;
      } else if (xMovement < 0.0 && xMovement > -1.0) {
        xMovement = -1.0;
      } else {
        xMovement = round(xMovement);
      }
      */
      timeline->xMin -= xMovement;
      timeline->xMax -= xMovement;
      timeline->createAxis();
    }
  }
  if (_savingMode == CONTINUOUS) {
    save();
  }
}

void MainWindow::wheelEvent(QWheelEvent *event) {
    if (Qt::ShiftModifier != QApplication::keyboardModifiers()) {
      timeline->yMin += event->delta() / 10;
      if (timeline->yMin < -timeline->yMax + 100)
	timeline->yMin = -timeline->yMax + 100;
      if (timeline->yMin > 0)
	timeline->yMin = 0;
      timeline->createAxis();
  } else {
    // Handle mouse wheel events
    float range = timeline->xMax - timeline->xMin;
    int counter = 0;
    while (range > 100) {
      range /= 10.0;
      counter++;
    }
    timeline->xMax += (event->delta() / 120) * (int)std::pow(10., int(counter));
    timeline->createAxis();
  }
}

void MainWindow::resizeEvent(QResizeEvent *event) {
  // Handle window resize
  QMainWindow::resizeEvent(event);
  timeline->createAxis();
}

void MainWindow::newFile() {
  // Create an empty timeline when the user invokes New
  int argc = 0;
  char **argv = NULL;

  MainWindow *w = new MainWindow(argc, argv);
  w->show();
  return;

}

void MainWindow::open() {
  delete timeline;
  timeline = new Timeline;
  timeline->inputFile();
  setCentralWidget(timeline);
  timeline->show();
  timeline->createAxis();
  timeline->setFocus();
}

void MainWindow::save() {
  timeline->outputFile();
  _change =false;
}

void MainWindow::saveAs() {
  timeline->saveas();
  _change = false;
}


void MainWindow::addEvent() {
  // Create the dialog box to add an event
  addPointDialog *dialog = new addPointDialog;
  dialog->timeline = this->timeline;
  dialog->show();
  dialog->setWindowState(Qt::WindowState::WindowActive);
}

void MainWindow::openPopupMenu(QMouseEvent* event) {
  mousePressEvent(event);
}

void MainWindow::leftClick(int index){
  if (_mode == 0)
    return;
  
  _leftClick = true;
  prevMouse = QCursor::pos();

  _selectedIndex = timeline->getEventIndex(prevMouse);
  if (_selectedIndex != -1) {
    deselectTimelines();
    _selectedIndex = timeline->getEventIndex(prevMouse);

    Event *event = timeline->events[_selectedIndex];
    _selected = true;
    event->setSelected(true,_mode);

    moveEventUpAct->setEnabled(true);
    moveEventDownAct->setEnabled(true);
    editEventAct->setEnabled(true);
    deselectAllAct->setEnabled(true);
    return;
  }
}

void MainWindow::editEvent() {
  
  // Creates the dialog box to edit an event
  // Checks that only one event (the one to be edited) is selected

  //int index = timeline->getEventIndex(prevMouse);
  if (_selectedIndex != -1) {
    _change = true;
    Event *event = timeline->events[_selectedIndex];

    // Create the dialog box with default parameters from the selected event
    editpointdialog *dialog = new editpointdialog(_selectedIndex);
    dialog->timeline = this->timeline;
    dialog->startTime->setValue(event->startTime);
    dialog->endTime->setValue(event->endTime);
    dialog->start->setText(event->startLabel.text());
    dialog->end->setText(event->endLabel.text());
    dialog->name->setText(event->name.text());
    dialog->setEventColor(event->color());
    dialog->show();
  }
}

void MainWindow::openFunction() {
  // int index = timeline->getEventIndex(prevMouse);
  if (_selectedIndex != -1) {
    Event *event = timeline->events[_selectedIndex];
    QString buttonText = event->name.text();
    // If the function file does not exist, create it (this should never happen)
    std::string path =
        (QDir::currentPath()).toStdString() + "/" + timeline->getTmpDir() + "/";
    std::string name = path + buttonText.toStdString();

    std::ifstream myfile(name.c_str(), std::ifstream::in);
    if (!myfile.good()) {
      std::ofstream outfile(name.c_str(), std::ifstream::out);
      if (outfile.is_open()) {
        outfile << "fver 1 1\n";
        outfile << "name: " << buttonText.toStdString() << "\n";
        outfile << "samples: 20\n";
        outfile << "flip: off\n";
        outfile << "points: 4\n";
        outfile << "0.000000 0.000000\n";
        outfile << "0.333333 0.000000\n";
        outfile << "0.666667 0.000000\n";
        outfile << "1.000000 0.000000\n";
      } else {
        std::cerr << "Unable to open funcedit file" << std::endl;
      }
    }

    timeline->createFunceditProcess(buttonText);
  }
}

void MainWindow::deleteSelected() {
  // Create a dialog box for confirming deletes
  deleteConfirm *dialog = new deleteConfirm(QCursor::pos());
  dialog->timeline = this->timeline;
  dialog->show();
  dialog->setWindowState(Qt::WindowState::WindowActive);
}

void MainWindow::deleteEvent() {
  // Create a dialog box for confirming deletes
  //int index = timeline->getEventIndex(prevMouse);
  int index = _selectedIndex;

  deleteConfirm *dialog = new deleteConfirm(index);
  dialog->timeline = this->timeline;
  dialog->show();
  dialog->setWindowState(Qt::WindowState::WindowActive);
}

void MainWindow::deselectAll() {
  _selectShift = false;
  // Deselect all events
  for (Event *e : timeline->events) {
    e->startSelected = false;
    e->endSelected = false;
    e->setSelected(false,_mode);
  }
  _selected = false;
  _selectedIndex = -1;

  editEventAct->setEnabled(false);
  deselectAllAct->setEnabled(false);
  deleteSelectedAct->setEnabled(false);
  moveEventUpAct->setEnabled(false);
  moveEventDownAct->setEnabled(false);
  openFunctionAct->setEnabled(false);
  timeline->update();
}

void MainWindow::deselectTimelines() {
  // Deselect all events
  for (Event *e : timeline->events) {
    e->setSelected(false,_mode);
  }
  _selected = false;
  _selectedIndex = -1;

  editEventAct->setEnabled(false);
  deselectAllAct->setEnabled(false);
  deleteSelectedAct->setEnabled(false);
  moveEventUpAct->setEnabled(false);
  moveEventDownAct->setEnabled(false);
  openFunctionAct->setEnabled(false);

  timeline->update();
}


void MainWindow::moveEventUp() {
  timeline->swapEvents(1);
  if (_selectedIndex < timeline->events.size()-1)
    _selectedIndex++;
}

void MainWindow::moveEventDown() {
  timeline->swapEvents(-1);
  if (_selectedIndex > 0)
    _selectedIndex--;
}

void MainWindow::quickHelp(){
  QDir helpDir(getHelpDirectory());
#ifndef __APPLE__
  helpDir.cd("Quick_Help");
#endif
  QString path = helpDir.filePath("TimelineQuickHelp.html");
  QFile f(path);
  if (!f.open(QFile::ReadOnly | QFile::Text)) {
    std::cerr<<"Path: "<< path.toStdString() << " doesn't exist"<<std::endl;
    return;
  }
  QTextStream in(&f);
  QString message = in.readAll();
  QTextBrowser *tb = new QTextBrowser(this);
  tb->setOpenExternalLinks(true);
  tb->setHtml(message);

  QDialog *msgBox = new QDialog;
  msgBox->setWindowTitle("Timeline: Quick Help");
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

void MainWindow::pdfHelp() {
  QDir helpDir(getHelpDirectory());
  QDesktopServices::openUrl(
      QUrl::fromLocalFile(helpDir.filePath("VLABToolsManual.pdf")));
}


void MainWindow::help() {
  // Show the help dialog box
  helpdialog *dialog = new helpdialog;
  dialog->timeline = this->timeline;
    
  dialog->show();
  dialog->setWindowState(Qt::WindowState::WindowActive);
}

void MainWindow::reloadConfig() { timeline->loadConfig(); }

void MainWindow::selectAllLeftOfCursor() { timeline->selectAll(); }

void MainWindow::createActions() {
  // Create all the menu actions and link them to the menu buttons
  
  saveAct = new QAction(tr("&Save"), this);
  //saveAct->setShortcuts(QKeySequence::Save);
  saveAct->setStatusTip(tr("Save the timeline"));
  connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

  newAct = new QAction(tr("&New"), this);
  //newAct->setShortcuts(QKeySequence::New);
  newAct->setStatusTip(tr("Create a new timeline"));
  connect(newAct, SIGNAL(triggered()), this, SLOT(newFile()));

  openAct = new QAction(tr("&Open..."), this);
  //openAct->setShortcuts(QKeySequence::Open);
  openAct->setStatusTip(tr("Open an existing timeline"));
  connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

  addEventAct = new QAction(tr("&Add timeline"), this);
  //addEventAct->setShortcut(tr("A"));
  addEventAct->setStatusTip("Add a timeline");
  connect(addEventAct, SIGNAL(triggered()), this, SLOT(addEvent()));

  editEventAct = new QAction(tr("&Edit timeline"), this);
  //editEventAct->setShortcut(tr("E"));
  editEventAct->setStatusTip("Edit a timeline");
  editEventAct->setEnabled(false);
  connect(editEventAct, SIGNAL(triggered()), this, SLOT(editEvent()));

  deleteSelectedAct = new QAction(tr("&Delete selected timeline"), this);
  // deleteSelectedAct->setShortcut(tr("D"));
  deleteSelectedAct->setStatusTip("Delete selected timeline");
  deleteSelectedAct->setEnabled(false);
  connect(deleteSelectedAct, SIGNAL(triggered()), this, SLOT(deleteSelected()));

  deleteEventAct = new QAction(tr("Delete timeline"), this);
  deleteEventAct->setStatusTip("Delete timeline");
  connect(deleteEventAct, SIGNAL(triggered()), this, SLOT(deleteEvent()));

  deselectAllAct = new QAction(tr("&Deselect all"), this);
  // deselectAllAct->setShortcut(tr("X"));
  deselectAllAct->setStatusTip("Deselect all timelines");
  connect(deselectAllAct, SIGNAL(triggered()), this, SLOT(deselectAll()));
  deselectAllAct->setEnabled(false);
  /*
  moveEventUpAct = new QAction(tr("&Move selected timeline up"), this);
  moveEventUpAct->setShortcut(Qt::Key_Up);
  moveEventUpAct->setStatusTip("Move selected timeline up");
  moveEventUpAct->setEnabled(false);
  connect(moveEventUpAct, SIGNAL(triggered()), this, SLOT(moveEventDown()));
  addAction(moveEventUpAct);

  moveEventDownAct = new QAction(tr("&Move selected timeline down"), this);
  moveEventDownAct->setShortcut(Qt::Key_Down);
  moveEventDownAct->setStatusTip("Move selected timeline down");
  moveEventDownAct->setEnabled(false);
  addAction(moveEventDownAct);
  
  connect(moveEventDownAct, SIGNAL(triggered()), this, SLOT(moveEventUp()));
  */
  selectAllRightOfCursorAct =
      new QAction(tr("&Select all left of cursor"), this);
  // selectAllRightOfCursorAct->setShortcut(tr("S"));
  selectAllRightOfCursorAct->setStatusTip("Select all left of cursor");
  connect(selectAllRightOfCursorAct, SIGNAL(triggered()), this,
          SLOT(selectAllLeftOfCursor()));

  /*
  helpAct = new QAction(tr("&Help"), this);
  helpAct->setShortcut(tr("H"));
  helpAct->setStatusTip("Help");
  connect(helpAct, SIGNAL(triggered()), this, SLOT(help()));
  */


  reloadConfigAct = new QAction(tr("&Reload config"), this);
  // reloadConfigAct->setShortcut(tr("R"));
  reloadConfigAct->setStatusTip("Reload config.txt file");

  openFunctionAct = new QAction("Open function", this);
  connect(openFunctionAct, SIGNAL(triggered()), this, SLOT(openFunction()));
  openFunctionAct->setEnabled(false);

  connect(reloadConfigAct, SIGNAL(triggered()), this, SLOT(reloadConfig()));

  connect(timeline, SIGNAL(nameRightClick(QMouseEvent*)), this, SLOT(openPopupMenu(QMouseEvent*)));
  connect(timeline, SIGNAL(nameLeftClick(int)), this, SLOT(leftClick(int)));
  connect(timeline, SIGNAL(eventDoubleClicked()), this, SLOT(editEvent()));

  exitAct = new QAction(tr("Quit"), this);
  //  exitAct->setShortcuts(QKeySequence::Quit);
  exitAct->setStatusTip(tr("Exit the application"));
  connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

  // new action created for new edit/execute mode
  saveAsAct = new QAction(tr("Save as ..."), this);
  connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));
  // refresh mode menu
  _savingMenu_act = new QAction("Explicit",this);
  connect(_savingMenu_act,SIGNAL(triggered()), this, SLOT(ModeOff()));
  _savingMenu_act->setCheckable(true);
  if (_savingMode == OFF)
    _savingMenu_act->setChecked(true);
  
  _savingTriggered_act = new QAction("Triggered",this);
  connect(_savingTriggered_act,SIGNAL(triggered()), this, SLOT(TriggeredSavingMode()));
  _savingTriggered_act->setCheckable(true);
  if (_savingMode == TRIGGERED)
    _savingTriggered_act->setChecked(true);
  _savingContinu_act = new QAction("Continuous",this);
  connect(_savingContinu_act,SIGNAL(triggered()), this, SLOT(ContinuousSavingMode()));
  _savingContinu_act->setCheckable(true);
  if (_savingMode == TRIGGERED)
    _savingContinu_act->setChecked(true);

  editAct = new QAction("Edit",this);
  connect(editAct,SIGNAL(triggered()),this,SLOT(setToEditMode()));
  executeAct = new QAction("Execute",this);
  connect(executeAct,SIGNAL(triggered()),this,SLOT(setToExecuteMode()));
  
}

void MainWindow::createEditMenu(){
  newEditMenu = new QMenu("Edit",this);
  newEditMenu->addAction(addEventAct);
  newEditMenu->addSeparator();
  moveEventUpAct = newEditMenu->addAction(tr("&Move selected timeline up"),this,SLOT(moveEventDown()),Qt::Key_Up);
  addAction(moveEventUpAct);
  moveEventUpAct->setEnabled(false);
  moveEventUpAct->setShortcutVisibleInContextMenu(true);

  moveEventDownAct = newEditMenu->addAction(tr("&Move selected timeline down"),this,SLOT(moveEventUp()),QKeySequence(Qt::Key_Down));
  addAction(moveEventDownAct);
  moveEventDownAct->setEnabled(false);
  moveEventDownAct->setShortcutVisibleInContextMenu(true);


//  newEditMenu->addAction(moveEventDownAct);
  newEditMenu->addAction(deleteSelectedAct);
  newEditMenu->addSeparator();
  newEditMenu->addAction(openFunctionAct);
  newEditMenu->addSeparator();
  newEditMenu->addAction(executeAct);
  newEditMenu->addSeparator();
  newEditMenu->addAction(saveAct);
  newEditMenu->addAction(saveAsAct);
  newEditMenu->addSeparator();
  newEditMenu->addAction(exitAct);

  menu = new QMenu("menu", this);
  menu->addAction(openFunctionAct);
  menu->addAction(deleteEventAct);
  menu->addAction(editEventAct);


}

void MainWindow::createExecuteMenu(){
  executeMenu = new QMenu("Execute",this);
  executeMenu->addAction(saveAct);
  executeMenu->addAction(saveAsAct);
  executeMenu->addSeparator();
  executeMenu->addAction(deselectAllAct);
  executeMenu->addAction(editAct);
  executeMenu->addSeparator();
  executeMenu->addMenu(modeMenu);
  executeMenu->addSeparator();
  executeMenu->addAction(exitAct);

}

void MainWindow::createBarMenu(){
  // Create the top screen menu heirarchy
  barFileMenu = menuBar()->addMenu(tr("&File"));
  barFileMenu->addAction(saveAct);
  barFileMenu->addAction(saveAsAct);
  modeMenu = barFileMenu->addMenu("Refresh mode");
  modeMenu->addAction( _savingMenu_act);
  modeMenu->addAction( _savingTriggered_act);
  modeMenu->addAction( _savingContinu_act);
  QAction *preferences = barFileMenu->addAction("Preferences...");
  connect(preferences, SIGNAL(triggered()), this, SLOT(editPreferencesCB()));

  QMenu *help = menuBar()->addMenu("Help");
  QAction *qHelp=help->addAction("Quick help",this,SLOT(quickHelp()));
  qHelp->setEnabled(true);
  help->addAction("Tools manual", this, SLOT(pdfHelp()));

  QMenu *about = menuBar()->addMenu("About");
  QAction* aboutAct = about->addAction(tr("&About"), this,SLOT(about()));

}

void MainWindow::about(){
  vlab::about(this,"Timeline");
}

void MainWindow::createMenus() {

  createBarMenu();
  createExecuteMenu();
  createEditMenu();
  return;

  
  // Create the top screen menu heirarchy
  fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(newAct);
  fileMenu->addAction(openAct);
  fileMenu->addAction(saveAct);

  editMenu = menuBar()->addMenu(
      tr("&Edit\1")); // \1 to remove additional menu from Apple
  editMenu->addAction(addEventAct);
  editMenu->addAction(editEventAct);
  editMenu->addAction(deleteSelectedAct);
  editMenu->addAction(deselectAllAct);
  editMenu->addAction(moveEventUpAct);
  editMenu->addAction(moveEventDownAct);

  helpMenu = menuBar()->addMenu(tr("&Help"));
  helpMenu->addAction(helpAct);

  QAction *preferences = fileMenu->addAction("Preferences...");
  connect(preferences, SIGNAL(triggered()), this, SLOT(editPreferencesCB()));

  this->addAction(selectAllRightOfCursorAct);

  popupMenu = new QMenu("popupMenu", this);
  popupMenu->addAction(saveAct);
  popupMenu->addAction(newAct);
  popupMenu->addAction(openAct);
  popupMenu->addSeparator();

  popupMenu->addAction(addEventAct);
  popupMenu->addAction(deleteSelectedAct);
  popupMenu->addAction(deselectAllAct);
  popupMenu->addAction(moveEventUpAct);
  popupMenu->addAction(moveEventDownAct);

  QMenu *modeMenu = popupMenu->addMenu("Refresh mode");
  _savingMenu_act = modeMenu->addAction("Explicit", this, SLOT(ModeOff()));
  _savingMenu_act->setCheckable(true);
  if (_savingMode == OFF)
    _savingMenu_act->setChecked(true);

  _savingTriggered_act =
      modeMenu->addAction("Triggered", this, SLOT(TriggeredSavingMode()));
  _savingTriggered_act->setCheckable(true);
  if (_savingMode == TRIGGERED)
    _savingTriggered_act->setChecked(true);

  _savingContinu_act =
      modeMenu->addAction("Continuous", this, SLOT(ContinuousSavingMode()));
  _savingContinu_act->setCheckable(true);
  if (_savingMode == CONTINUOUS)
    _savingContinu_act->setChecked(true);
  popupMenu->addSeparator();

  popupMenu->addAction(exitAct);

  // Set the actions of the right click context menu
  menu = new QMenu("menu", this);
  menu->addAction(openFunctionAct);
  menu->addAction(deleteEventAct);
  menu->addAction(editEventAct);
}

/******************************************************************************
 *
 * callback for the 'edit preferences'
 *
 */

void MainWindow::setPreferences() {

  QString userConfigDir = "";
#ifdef __APPLE__
  userConfigDir = Vlab::getUserConfigDir(false);
#endif

  char bf[PATH_MAX + 1];
  const char *cdir = userConfigDir.toStdString().c_str();
  if (NULL == cdir)
    return;
  else {
    strcpy(bf, cdir);
    strcat(bf, "/");
  }
  strcat(bf, "timeline.cfg");

  QString filePreferences = QString(bf);
  window = new Preferences(this, filePreferences);
  window->setModal(false);
  window->loadConfig();
}

void MainWindow::editPreferencesCB() {

  window->show();
}



void MainWindow::RequestSave() {
  if (!_idleTimer->isActive()) {
    _idleTimer->setSingleShot(false);
    _idleTimer->start(0);
    save();

  } else {
    _new_save_pending = true;
  }
}

void MainWindow::Idle() {
  _idleTimer->stop();
  QWidget::setCursor(Qt::ArrowCursor);
  if (_new_save_pending) {
    _new_save_pending = false;
    save();
  }
}

void MainWindow::setToExecuteMode(){
  int ret = QMessageBox::No;
  if (_change) {
    ret = QMessageBox::warning(
        this, "Save At Exit", "Save changes before switching to the execute mode?",
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
        QMessageBox::Cancel);
    switch (ret) {
    case QMessageBox::Yes:
      _change = false;
      this->save();
      break;
    case QMessageBox::No:
      break;
    case QMessageBox::Cancel:
    default:
      return ;
    }
  }
  deselectAll();
  _mode = 0;

  timeline->setMode(_mode);
}

void MainWindow::setToEditMode(){
  _mode = 1;
  timeline->setMode(_mode);
}

void MainWindow::mousePressEventInExecuteMode(QMouseEvent* event){
  QWidget *topWidget = QApplication::topLevelAt(this->mapToGlobal(QPoint()));
  topWidget->raise();

  prevMouseX = (float)event->pos().x();
  prevMouseY = (float)event->pos().y();
  prevMouse = QCursor::pos();
  hasMoved = false;
  if (event->button() == Qt::RightButton) {
      executeMenu->exec(QCursor::pos());
    return;
  }
  // left click on a timeline
  if (event->button() == Qt::LeftButton) {
    mouseLeftPressed = true;
    
    if (Qt::ShiftModifier == QApplication::keyboardModifiers()) {
      _selectShift = true;
      if (timeline->isPointSelected()) {	
	_selected = false;
	_selectedNb--;
	timeline->deselectPoint();
	// check if there are still point selected
	if (_selectedNb == 0)
	  _selectShift = false;
	
	return;
      }
      _selected = true;
      _selectedNb++;
      timeline->selectPoint();
      return;
    }

    //left click on a point
    if (timeline->isPoint()){
      if (_selectShift){
	if (!timeline->isPointSelected()) {	
	  _selected = false;
	  return;
	}
	if (timeline->isPointSelected()) {	
	  _selected = true;
	  return;
	}

      }

      if (timeline->isPointSelected()) {
	deselectAll();
	_selected = false;
	return;
      }
      deselectAll();
      timeline->selectPoint();
      _selected = true;
      return;
    }
    // no point selected
    deselectAll();
    _selected = false;
    return;
  }
}


void MainWindow::mousePressEventInEditMode(QMouseEvent* event){
  QWidget *topWidget = QApplication::topLevelAt(this->mapToGlobal(QPoint()));
  topWidget->raise();
  

  prevMouseX = (float)event->pos().x();
  prevMouseY = (float)event->pos().y();
  prevMouse = QCursor::pos();
  hasMoved = false;
  if (event->button() == Qt::RightButton) {
    // right click outside timeline
    //check how many timelines are selected
    int count = timeline->nbSelected();
    if (count == 0){
      moveEventUpAct->setEnabled(false);
      moveEventDownAct->setEnabled(false);
      deleteSelectedAct->setEnabled(false);
      openFunctionAct->setEnabled(false);

    }
    if (count == 1){
      moveEventUpAct->setEnabled(true);
      moveEventDownAct->setEnabled(true);
      deleteSelectedAct->setEnabled(true);
      openFunctionAct->setEnabled(true);

    }
    newEditMenu->exec(QCursor::pos());
    return;
  }
}
