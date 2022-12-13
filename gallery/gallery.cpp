

#include "glwidget.h"
#include "gallery.h"
#include "cset.h"
#include "fset.h"
#include "set.h"
#include "contour.h"
#include "func.h"
#include "delete_recursive.h"
#include "createitemdlg.h"
#include "mainwindow.h"
#include <unistd.h>
#include <signal.h>


#include <QFileDialog>
#include <QGridLayout>
#include <QDesktopWidget>
#include <QScreen>
#include <QLabel>
#include <QTimer>
#include <QMenu>
#include <QApplication>
#include <QMessageBox>
#include <QMouseEvent>
#include <QInputDialog>
#include <QTextStream>
#include <iostream>
#include <string>
#include <directorywatcher.h>

#include "config.h"

Gallery::Gallery(QWidget *parent,int argc, char** argv):
  _directoryWatcher(NULL)
{

  setWindowFlags(Qt::WindowCloseButtonHint);
  QWidget *_topLevel = this;
#ifdef __APPLE__
  QWidget *p = dynamic_cast<QWidget *>(parent);
  while (p) {
    _topLevel = p;
    p = dynamic_cast<QWidget *>(_topLevel->parent());
  };
  MainWindow *mw = dynamic_cast<MainWindow *>(_topLevel);
#else
  QWidget *mw = this;
#endif

  
  // create and set up the timer for the IdleFunction
  _idleTimer = new QTimer(this);
  connect(_idleTimer, SIGNAL(timeout()), SLOT(Idle()));

  //this is the starting position of object when we call cuspy or funcedit
  _posx = 100;
  _posy = 100;

  char tempdir[] = "gal.XXXXXX";
  mkdtemp(tempdir);
  _tmpDir = std::string(tempdir);

  //Initialize window
  Config::readConfigFile();
 
  _itemWidth = Config::getItemWidth();
  _itemHeight = Config::getItemWidth();
  _margins = Config::getMargins();
  mw->setContentsMargins(_margins,_margins,_margins,_margins);

  // set the widget's properties
  _has_changed = false;
  setMenu();
  parseCommandLine(argc,argv);
  setSavingMode();
   
 
  _layout = new QGridLayout;
  int i = 0;
  _layout->setContentsMargins(0,0,0,0);
  QSpacerItem *hspacer = new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Minimum);
  QSpacerItem *hspacerTitle = new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Minimum);

  for (std::vector<Item*>::iterator it = _items.begin() ; it != _items.end(); ++it){
    addItemToLayout(*it);
     ++i;
  }
  _layout->addItem(hspacer, 0,1000, -1, 1);
  _layout->addItem(hspacerTitle, 1,1000, -1, 1);
  setLayout(_layout);

  //build size of the application
  QPoint originalPoint = QCursor::pos();
  int width = (_itemWidth + _margins)*_items.size() + _margins;
  QScreen *screen = QGuiApplication::screenAt(originalPoint);
  QList<QScreen *> screenList = QGuiApplication::screens();
  int maxwidth = screen->geometry().width();
  if (width + originalPoint.x() > maxwidth){
    originalPoint.setX(screen->geometry().x())  ;
    if (width > maxwidth)
      width = maxwidth;
  }
  int height = _itemHeight+ 2 * _margins + 25;

  resize(width, height);
  
  setFixedHeight(height); // prevent it from collapsing to zero immediately
  mw->setFixedHeight(height+SCROLLBARSIZE);
  QFileInfo fi(_galleryFilename);
  QString base = fi.baseName();
  std::string caption = "Gallery: ";
  caption += base.toStdString();
  mw->resize(width, height);
  mw->setWindowTitle(caption.c_str());
  mw->move(originalPoint);


}

Gallery::~Gallery(){

} // moved editor closing and delete_recursive to CleanUp()

void Gallery::CleanUp(){
  for (size_t i = 0; i <_items.size(); ++i){
    QProcess * process = _items[i]->getProcess();
    if (process != NULL) {
      process->close();
    }
  }
  delete_recursive(_tmpDir.c_str());
}

void Gallery::setMenu(){

  _pContextMnu = new QMenu(this);

  _pContextMnu->addAction("&Save gallery", this, SLOT(saveAll()));
  _pContextMnu->addAction("Save gallery &as ...", this, SLOT(saveAllAs()));
  _pContextMnu->addSeparator();
  _pContextMnu->addAction("&Update all views", this, SLOT(reloadAll()));
  _pContextMnu->addSeparator();
  //QAction *actionLaunch = _pContextMnu->addAction("&Launch editor");
  //actionLaunch->setEnabled(false);
  
  //_pContextMnu->addSeparator();

  _pContextMnu->addAction("&Create new item ...", this, SLOT(createItem()));
   _actionDuplicate = _pContextMnu->addAction("&Duplicate item ...", this, SLOT(duplicateItem()));
  _actionDuplicate->setEnabled(false);
  _pContextMnu->addAction("&Load existing item ...", this, SLOT(loadItem()));
  _actionDelete = _pContextMnu->addAction("&Remove item",this, SLOT(removeItem()));
  _actionDelete->setEnabled(false);
  _pContextMnu->addSeparator();

  
  QMenu *continuousModeMenu = new QMenu(this);
  continuousModeMenu->setTitle("Refresh mode");
  _explicitMode = continuousModeMenu->addAction("Explicit", this,
						SLOT(SetExplicitMode(bool)));
  _triggeredMode = continuousModeMenu->addAction("Triggered", this,
						  SLOT(SetTriggeredMode(bool)));
  _continuousMode = continuousModeMenu->addAction("Continuous", this,
						  SLOT(SetContinuousMode(bool)));

  _continuousMode->setCheckable(true);
  _triggeredMode->setCheckable(true);
  _explicitMode->setCheckable(true);
  _pContextMnu->addMenu(continuousModeMenu);

  _pContextMnu->addSeparator();
  _pContextMnu->addAction("E&xit", this, SLOT(quit()));

}

void Gallery::quit(){
  int ret = QMessageBox::No;
  if (this->hasChanged()) {
    ret = QMessageBox::warning(
        this, "Save At Exit", "Save changes before exit?",
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
        QMessageBox::Cancel);
  

    switch (ret) {
    case QMessageBox::Yes:
      this->saveAll();
      break;
    case QMessageBox::No:
      _has_changed = false;
      break;
    case QMessageBox::Cancel:
    default:
      return ;
    }
  }
  close();
}

void Gallery::closeEvent(QCloseEvent *pEv) {
  // why isn't the MainWindow ptr saved as a private member?
  int ret = QMessageBox::No;

   QWidget *_topLevel = this;
#ifdef __APPLE__
  QWidget *p = dynamic_cast<QWidget *>(parent());
  while (p) {
    _topLevel = p;
    p = dynamic_cast<QWidget *>(_topLevel->parent());
  };
  MainWindow *mw = dynamic_cast<MainWindow *>(_topLevel);
#else
  QWidget *mw = this;
#endif
  mw->close();

  pEv->accept(); // default
}

void Gallery::changeSize(){

    //Initialize window
    Config::readConfigFile();
    
    _itemWidth = Config::getItemWidth();
    _itemHeight = Config::getItemWidth();
    _margins = Config::getMargins();

   for (std::vector<Item*>::iterator it = _items.begin() ; it != _items.end(); ++it){
     GLWidget* glwidget =  (*it)->getGLWidget();
     glwidget->setFixedSize(_itemWidth,_itemHeight);
   }

    int width = (_itemWidth + 2 * _margins)*_items.size();
    int height = _itemHeight+ 2 * _margins + 25;
    QWidget *_topLevel = this;
#ifdef __APPLE__
    QWidget *p = dynamic_cast<QWidget *>(parent());
    while (p) {
      _topLevel = p;
      p = dynamic_cast<QWidget *>(_topLevel->parent());
    };
    MainWindow *mw = dynamic_cast<MainWindow *>(_topLevel);

#else
    QWidget *mw = this;
#endif
    mw->setContentsMargins(_margins,_margins,_margins,_margins);
 
    mw->resize(width, height);

    mw->setFixedHeight(height + SCROLLBARSIZE);
    resize(width, height);
    
    setFixedHeight(height); // prevent it from collapsing to zero immediately
 
    //    std::cerr<<"Size of icons have changed, restart Gallery to apply the changes. "<<std::endl;
  }


void Gallery::setSavingMode(){
  _explicitMode->setChecked(true);
  _continuousMode->setChecked(false);

  if (_savingMode == CONTINUOUS) {
    _continuousMode->setChecked(true);
    _triggeredMode->setChecked(false);
    _explicitMode->setChecked(false);

    SetContinuousMode(true);
  } else if (_savingMode == TRIGGERED) {
    _continuousMode->setChecked(false);
    _triggeredMode->setChecked(true);
    _explicitMode->setChecked(false);
    SetTriggeredMode(true);
  } else {
    _explicitMode->setChecked(true);
    _triggeredMode->setChecked(false);
    _continuousMode->setChecked(false);
    SetExplicitMode(true);
  }

}

SavingMode Gallery::getSavingMode(){
  return _savingMode;
}

void Gallery::parseCommandLine(int argc, char **argv){
  int i = 1;
  _savingMode = OFF;
  while (i < argc) {
    if (strcmp(argv[i], "-rmode") == 0) {
      ++i;
      if ((strcmp(argv[i], "expl") == 0) || (strcmp(argv[i], "explicit") == 0))
        _savingMode = OFF;
      else if ((strcmp(argv[i], "cont") == 0) ||
               (strcmp(argv[i], "continuous") == 0))
        _savingMode = CONTINUOUS;
      else if ((strcmp(argv[i], "trig") == 0) ||
               (strcmp(argv[i], "triggered") == 0))
        _savingMode = TRIGGERED;
      else {
        std::cout << "Wrong options: " << argv[i]
                  << " - usage gallery [--refreshMode|-rmode cont|trig|exp] "
	  "[filename]"
                  << std::endl;
        exit(0);
      }
      ++i;
      continue;
    }
    _galleryFilename = QString(argv[i]);
    QFileInfo checkFile(_galleryFilename);
    // check if file exists and if yes: Is it really a file and no directory?
    if (checkFile.exists() && checkFile.isFile())
      loadFile(argv[i]);
    else {
      QMessageBox::warning(
			   this, "",
			   "File " + _galleryFilename +
			   " doesn't exist, a default gallery will be created.",
			   QMessageBox::Ok);
      // create a file with one default model and save it.
      QString ext = checkFile.completeSuffix();
      QFile file(_galleryFilename);
      if (ext.compare("cset") == 0){
	if (file.open(QIODevice::ReadWrite)) {
	  QTextStream stream(&file);
	  stream << "contourgalleryver 1 1\n";
	  stream << "items: 1\n";
	  stream << " cver 1 1\n";
	  stream << "name: noname\n";
	  stream << "points: 4 4\n";
	  stream << "type: closed\n";
	  stream << "0.4 0.4 0.0 1\n";
	  stream << "0.4 -0.4 0.0 1\n";
	  stream << "-0.4 -0.4 0.0 1\n";
	  stream << "-0.4 0.4 0.0 1\n";
	}
	file.close();
      }
      else  if (ext.compare("fset") == 0){
	if (file.open(QIODevice::ReadWrite)) {
	  QTextStream stream(&file);
	  stream << "funcgalleryver 1 1\n";
	  stream << "items: 1\n";
	  stream << "fver 1 1\n";
	  stream << "name: stem\n";
	  stream << "samples: 100\n";
	  stream << "flip: off\n";
	  stream << "points: 4\n";
	  stream << "0.000000 0.0\n";
	  stream << "0.4 0.0\n";
	  stream << "0.4 1.\n";
	  stream << "1.000000 1.\n";
	}
	file.close();
      }
      else {
        std::cout << "Wrong extension, file extension should be cset for contours or fset for functions"<< std::endl;
        exit(0);
      }
      loadFile(_galleryFilename.toStdString());

    }

    ++i;
  }

  if (argc == 1) // no file name  been provided
    {
      //setGalleryFileName();
      QString message = "No file name has been provided with the command line: \n";
      message += QString(" - usage:\n")
	+ QString("     gallery [--refreshMode|-rmode cont|trig|exp] filename\n")
	+ QString(" - .cset extension of filename will create a new contour gallery\n")
	+ QString(" - .fset extension oof filename will create a new function gallery\n");
      /*      QMessageBox::warning(
			   this, "",
			   message,
			   QMessageBox::Ok);
      exit(0);

      */
      bool ok;
      QStringList items;
      items << tr("Contour") << tr("Function");

      QString text = QInputDialog::getItem(this, tr("New Gallery"),
					   tr("No file name has been provided.\nChoose a type of gallery and a default gallery will be created.\n"), items, 0, false, &ok);
      if (!ok){
        exit(0);
      }
      if (text.compare("Contour") == 0){
	_galleryFilename = QString("noname.cset");
	QFile file(_galleryFilename);
	if (file.open(QIODevice::ReadWrite)) {
	  QTextStream stream(&file);
	  stream << "contourgalleryver 1 1\n";
	  stream << "items: 1\n";
	  stream << " cver 1 1\n";
	  stream << "name: noname\n";
	  stream << "points: 4 4\n";
	  stream << "type: closed\n";
	  stream << "0.4 0.4 0.0 1\n";
	  stream << "0.4 -0.4 0.0 1\n";
	  stream << "-0.4 -0.4 0.0 1\n";
	  stream << "-0.4 0.4 0.0 1\n";
	}
	file.close();
      }
      else  if (text.compare("Function") == 0){
	_galleryFilename = QString("noname.fset");
	QFile file(_galleryFilename);
	if (file.open(QIODevice::ReadWrite)) {
	  QTextStream stream(&file);
	  stream << "funcgalleryver 1 1\n";
	  stream << "items: 1\n";
	  stream << "fver 1 1\n";
	  stream << "name: noname\n";
	  stream << "samples: 100\n";
	  stream << "flip: off\n";
	  stream << "points: 4\n";
	  stream << "0.000000 0.0\n";
	  stream << "0.4 0.0\n";
	  stream << "0.4 1.\n";
	  stream << "1.000000 1.\n";
	}
	file.close();
      }
      loadFile(_galleryFilename.toStdString());
      return;
      
      char tempdir[] = "gal.XXXXXX";
      mkdtemp(tempdir);
      _tmpDir = std::string(tempdir);
    }


}

void Gallery::loadFile(std::string filename){
  // Determine the filetype by the extension. Probably want
  //  to replace this at a later time with recognition of the
  //  file contents.
  int index = filename.rfind(".");
  if (index < 1) {
    // Unknown file type
    std::cerr << filename
	      << " is an unknown file type.  This file will not be loaded." << std::endl;
    return;
  }

  std::string extension(filename, index + 1);
  
  if (extension == std::string("fset")) {
    _galleryType = FUNC;
    // It's a function set file
    Set *pSet = new FSet(this, filename);
    _set = pSet;
  }
  
  if (extension == std::string("cset")) {
    // It's a contour set file
    _galleryType = CON;
    Set *pSet = new CSet(this, filename);
    _set = pSet;
  }

}

void Gallery::setGalleryFileName() {
  QString filename = QFileDialog::getSaveFileName(NULL, QString("Save Gallery"));

  if (filename != QString::null) {
    _galleryFilename = filename;
  }
  QFileInfo fi(_galleryFilename);
  QString base = fi.baseName();
  std::string caption = "Gallery: ";
  caption += base.toStdString();
  QWidget *_topLevel = this;
#ifdef __APPLE__
  QWidget *p = dynamic_cast<QWidget *>(parent());
  while (p) {
    _topLevel = p;
    p = dynamic_cast<QWidget *>(_topLevel->parent());
  };
  MainWindow *mw = dynamic_cast<MainWindow *>(_topLevel);
#else
  QWidget *mw = this;
#endif

  mw->setWindowTitle(caption.c_str());
}

void Gallery::addItem(Item* pItem){
  _items.push_back(pItem);
}

void Gallery::reloadAll(){
  for (unsigned int i = 0; i < _items.size(); i++){
    if (_items[i]->isVisible()){
      bool reloadOk = _items[i]->reload();
      if (!reloadOk)
	return;
      QLayoutItem* glLayout = _layout->itemAtPosition(0,i);
      GLWidget* glWidget;
      if (_galleryType == FUNC)
	glWidget = dynamic_cast<FuncViewer *>(glLayout->widget());
      if (_galleryType == CON)
	glWidget = dynamic_cast<ConViewer *>(glLayout->widget());
      glWidget->update();
    }
  }
      
  if (_savingMode != OFF) {
    if (_galleryType == FUNC)
      saveFuncSet(_galleryFilename.toStdString().c_str());
    if (_galleryType == CON)
      saveConSet(_galleryFilename.toStdString().c_str());
    _has_changed = false;
  }
}
void Gallery::saveAll(){
  if (_galleryType == FUNC)
    saveFuncSet(_galleryFilename.toStdString().c_str());
  if (_galleryType == CON)
    saveConSet(_galleryFilename.toStdString().c_str());
  _has_changed = false;
  reloadAll();
}



void Gallery::saveAllAs() {
  setGalleryFileName();
  if (_galleryFilename != QString::null)
    saveAll();
  _has_changed = false;
}


void Gallery::saveFuncSet(std::string filename) {
  std::ofstream out(filename.c_str());
  int counter = 0;

  for (unsigned int i = 0; i < _items.size(); i++) {
    std::string name = _items[i]->getFileName();
    int index = name.rfind(".");
    std::string extension(name, index);
    if (_items[i]->isVisible())
      counter++;

    //    if (extension == std::string(".func"))
    //  counter++;
  }

  out << "funcgalleryver 1 1" << std::endl;
  out << "items: " << counter << std::endl;

  for (unsigned int i = 0; i < _items.size(); i++) {
    std::string name = _items[i]->getFileName();
    int index = name.rfind(".");
    std::string extension(name, index);

    if (!_items[i]->isVisible())
      continue;
    //    if (extension != std::string(".func"))
    //  continue;

    std::ifstream in(_items[i]->getFileName().c_str());


    while (!in.eof()) {
      std::string buffer;
      getline(in, buffer);
      bool empty_line = true;
      for (std::string::iterator j = buffer.begin(); j != buffer.end(); ++j)
        if (!std::isalpha(*j)) {
          empty_line = false;
          break;
        }
      if (!empty_line)
        out << buffer << std::endl;
    }
  }
}

void Gallery::saveConSet(std::string filename) {
  std::ofstream out(filename.c_str());
  int counter = 0;
  for (unsigned int i = 0; i < _items.size(); i++) {
    std::string name = _items[i]->getFileName();
    int index = name.rfind(".");
    std::string extension(name, index);
    // if (extension == std::string(".con"))
    if (_items[i]->isVisible())
      counter++;
  }
  
  out << "contourgalleryver 1 1" << std::endl;
  out << "items: " << counter << std::endl;

  for (unsigned int i = 0; i < _items.size(); i++) {
    std::string name = _items[i]->getFileName();
    int index = name.rfind(".");
    std::string extension(name, index);
    if (!_items[i]->isVisible())
      continue;
    // if (extension != std::string(".con"))
    //  continue;

    std::ifstream in(_items[i]->getFileName().c_str());


    while (!in.eof()) {
      std::string buffer;
      getline(in, buffer);
      bool empty_line = true;
      for (std::string::iterator j = buffer.begin(); j != buffer.end(); ++j)
        if (!isalpha(*j)) {
          empty_line = false;
          break;
        }
      if (!empty_line)
        out << buffer << std::endl;
    }
  }
}

void Gallery::resizeWindow(){
  QPoint originalPoint = this->pos();
  int width = (_itemWidth + _margins)*_items.size() + _margins;
  QScreen *screen = QGuiApplication::screenAt(originalPoint);
  QList<QScreen *> screenList = QGuiApplication::screens();
  int maxwidth = screen->geometry().width();
  if (width + originalPoint.x() > maxwidth){
    originalPoint.setX(screen->geometry().x())  ;
    if (width > maxwidth)
      width = maxwidth;
  }
  int height = _itemHeight+ 2 * _margins + 25;

  resize(width, height);
   QWidget *_topLevel = this;
#ifdef __APPLE__
  QWidget *p = dynamic_cast<QWidget *>(parent());
  while (p) {
    _topLevel = p;
    p = dynamic_cast<QWidget *>(_topLevel->parent());
  };
  MainWindow *mw = dynamic_cast<MainWindow *>(_topLevel);
#else
  QWidget *mw = this;
#endif
  mw->resize(width,height);
}

void Gallery::createItem(){
  std::string tmpDir;
  tmpDir = _tmpDir;
  std::string filename = tmpDir + "/dummy";

  std::string selection_name = "noname";
  CreateItemDlg::SELECTION selection = CreateItemDlg::FUNC;
  switch (_galleryType) {
  case CON:
    selection = CreateItemDlg::CON;
    break;
  case FUNC:
    selection = CreateItemDlg::FUNC;
    break;
  default:
    break;
  }
  CreateItemDlg *pDlg = new CreateItemDlg(this, 0, 0, 0, selection);
  int r = pDlg->exec();
  if (r == QDialog::Accepted) {
    selection_name =
      pDlg->getName(); // we name the item file with the name of the item
    
    filename = tmpDir + "/" + selection_name;
    
    QStringList files = QDir::current().entryList();
    if ((files.indexOf((filename + ".func").c_str()) != -1) ||
	(files.indexOf((filename + ".con").c_str()) != -1)) {
      QMessageBox::information(
			       this, "This item File already exists",
			       "The item name specified already exists.\nChange the item name or "
			       "delete the file attached to this name\n");
      

      
    }
    Item* newItem = nullptr;
    if (_galleryType == FUNC){
      newItem = new FuncItem(this,filename,selection_name); 
    }
    else if (_galleryType == CON){
      newItem = new ConItem(this,filename,selection_name); 
    }
    _has_changed = true;
    _items.push_back(newItem);
    addItemToLayout(newItem);

  }
  resizeWindow();
  if (_savingMode != OFF) {
    if (_galleryType == FUNC)
      saveFuncSet(_galleryFilename.toStdString().c_str());
    if (_galleryType == CON)
      saveConSet(_galleryFilename.toStdString().c_str());
    _has_changed = false;
    setSavingMode();
  }
}


void Gallery::duplicateItem(){
  Item *pItem = _selectedItem;
  Item* newItem = NULL;
  std::string filename = _tmpDir + "/dummy";
  std::string selection_name = "noname";
  CreateItemDlg::SELECTION selection = CreateItemDlg::FUNC;
  switch (_galleryType) {
  case CON:
    selection = CreateItemDlg::CON;
    break;
  case FUNC:
    selection = CreateItemDlg::FUNC;
    break;
  default:
    break;
  }
  CreateItemDlg *pDlg = new CreateItemDlg(this, 0, 0, 0, selection);
  int r = pDlg->exec();
  if (r == QDialog::Accepted) {
    selection_name =
      pDlg->getName(); // we name the item file with the name of the item
  }
  
  if (_galleryType == FUNC){
    newItem = new FuncItem(*dynamic_cast<FuncItem *>(pItem),selection_name);
    newItem->setName(selection_name);

  }
  else if (_galleryType == CON){
    newItem = new ConItem(*dynamic_cast<ConItem *>(pItem),selection_name);
    newItem->setName(selection_name);

  }
  else{
    std::cerr<<"DUPLICATE NOT YET IMPLEMENTED"<<std::endl;
    return;
  }
  _has_changed = true;
  _items.push_back(newItem);
  addItemToLayout(newItem);
  resizeWindow();
  if (_savingMode != OFF) {
    if (_galleryType == FUNC)
      saveFuncSet(_galleryFilename.toStdString().c_str());
    if (_galleryType == CON)
      saveConSet(_galleryFilename.toStdString().c_str());
    setSavingMode();

    _has_changed = false;
  }


 }

void Gallery::addItemToLayout(Item* it){
  GLWidget* openGL;
  if (_galleryType == CON){
    openGL = new ConViewer( this);
  }
  if (_galleryType == FUNC){
    openGL = new FuncViewer( this);
  }
  openGL->setItem(it);
  it->setGLWidget(openGL);
  QLabel *openGLLabel = (it)->getName();
  openGLLabel->setAlignment(Qt::AlignHCenter);
  unsigned int pos = positionOfItem(it);
  _layout->addWidget(openGL, 0, pos);
  _layout->addWidget(openGLLabel, 1, pos);
  QObject::connect(it, &Item::openMenu,this, &Gallery::activateItemMenu);
    //build size of the application
  this->adjustSize();

}


void Gallery::createItemWithoutDialog(){
  std::cerr<<"CREATE ITEM WITHOUT DIALOG NOT YET IMPLEMENTED"<<std::endl;
}

void Gallery::loadItem(){
  QString fileName;
  if (_galleryType == NONE)
    fileName = QFileDialog::getOpenFileName(this, tr("Open File"), ".",
                                            tr("Item (*.func *.con *.s)"));
  if (_galleryType == FUNC)
    fileName = QFileDialog::getOpenFileName(this, tr("Open File"), ".",
                                            tr("Item (*.func)"));
  if (_galleryType == CON)
    fileName = QFileDialog::getOpenFileName(this, tr("Open File"), ".",
                                            tr("Item (*.con)"));

  
  QFileInfo fi(fileName);
  if (!fi.exists())
    return;
  QString base = fi.baseName();

  Item* newItem ;
  if (_galleryType == FUNC){
    newItem = new FuncItem(this,fileName.toStdString(),this);
  }
  else if (_galleryType == CON){
    newItem = new ConItem(this,fileName.toStdString(),this); 
  }
  _has_changed = true;
  _items.push_back(newItem);
  addItemToLayout(newItem);
  resizeWindow();

}

void Gallery::activateItemMenu(Item* it){
  _selectedItem = it;
  _actionDelete->setEnabled(true);
  _actionDuplicate->setEnabled(true);

  _pContextMnu->raise();
  _pContextMnu->exec(QCursor::pos());
}


void Gallery::activateMenu(){
  if (_selectedItem != NULL){
      _actionDelete->setEnabled(true);
      _actionDuplicate->setEnabled(true);
  }
    _pContextMnu->raise();
    _pContextMnu->exec(QCursor::pos());
}

void Gallery::mousePressEvent(QMouseEvent* ev){
  QWidget *topWidget = QApplication::topLevelAt(this->mapToGlobal(QPoint()));
  if (topWidget != nullptr)
    topWidget->raise();
  unselectItems();
  _actionDelete->setEnabled(false);
  _actionDuplicate->setEnabled(false);

  // Activate menu
  if (ev->button() == Qt::RightButton) {
    activateMenu();
  }
  if (ev->button() == Qt::LeftButton) {
  }
}

void Gallery::mouseReleaseEvent(QMouseEvent*){
  //_actionDelete->setEnabled(false);
}
 
void Gallery::valueChanged(int ){
  std::cerr<<"VALUE CHANGED NOT YET IMPLEMENTED"<<std::endl;
}


void Gallery::SetContinuousMode(bool enable){
  if (enable && _directoryWatcher){
    delete _directoryWatcher;
    _directoryWatcher = NULL;
  }
  if (enable && !_directoryWatcher) {
    QStringList ignoredPrefixes;
    QStringList ignoredSuffixes;
    std::string path = (QDir::currentPath()).toStdString() + "/" + _tmpDir;

    _directoryWatcher =
      new DirectoryWatcher(QString::fromStdString(path),
			   ignoredPrefixes << ".", ignoredSuffixes, this);

    connect(_directoryWatcher, SIGNAL(fileChanged(QString)), this,
            SLOT(RequestReload()));
  }

  _continuousMode->setChecked(true);
  _triggeredMode->setChecked(false);
  _explicitMode->setChecked(false);
  _savingMode = CONTINUOUS;

  /*
    for (unsigned int i = 0; i < items.size(); ++i) {
    if (items[i]->getSavingMode() != CONTINUOUS) {
    items[i]->SetContinuousMode(true);
    }
    }
  */
}

// The following is not used it's kept for consistency
void Gallery::SetTriggeredMode(bool enable){
  if (enable && !_directoryWatcher) {
    QStringList ignoredPrefixes;
    QStringList ignoredSuffixes;
    std::string path = (QDir::currentPath()).toStdString() + "/" + _tmpDir;
    _directoryWatcher =
      new DirectoryWatcher(QString::fromStdString(path),
			   ignoredPrefixes << ".", ignoredSuffixes, this);
    connect(_directoryWatcher, SIGNAL(fileChanged(QString)), this,
            SLOT(RequestReload()));
  }
  _continuousMode->setChecked(false);
  _triggeredMode->setChecked(true);    
  _explicitMode->setChecked(false);
  _savingMode = TRIGGERED;

  /*
    for (unsigned int i = 0; i < items.size(); ++i) {
    if (items[i]->getSavingMode() != TRIGGERED) {
    items[i]->SetTriggeredMode(true);
    }
    }
  */
}

void Gallery::SetExplicitMode(bool enable){
  if (enable && _directoryWatcher) {
    delete _directoryWatcher;
    _directoryWatcher = NULL;
  }
  _continuousMode->setChecked(false);
  _triggeredMode->setChecked(false);
  _explicitMode->setChecked(true);
  _savingMode = OFF;

  /*
    for (unsigned int i = 0; i < items.size(); ++i) {
    if (items[i]->getSavingMode() != OFF)
    items[i]->SetExplicitMode(true);
    }
  */
}

void Gallery::RequestReload() {
  if (!_idleTimer->isActive()) {
    _idleTimer->setSingleShot(false);
    _idleTimer->start(0);
    reloadAll();

  } else {
    _new_reload_pending = true;
  }
}
 
void Gallery::Idle() {
  _idleTimer->stop();
  QWidget::setCursor(Qt::ArrowCursor);
  if (_new_reload_pending) {
    _new_reload_pending = false;
    reloadAll();
  }
}


void Gallery::moveItemLeft(Item *pItem) {

  
  _has_changed = true;
  //if (QCursor::pos().x() <= 0)
  //  scrollArea->scroll(_itemWidth, 0);

  int x = 0;

  unsigned int found = _items.size();
  for (unsigned int i = 0; i < _items.size(); i++) {
    if (_items[i] == pItem) {
      found = i;
      break;
    }
    x += int(_itemWidth * _items[i]->relativeSize());
  }
 

  if (x < this->mapFromGlobal(QCursor::pos()).x())
    return;
  
  if (found == _items.size())
    return;
  if (found == 0)
    return;

   //now look for the previous one which is not hidden
  unsigned int found_previous = 0;
   for (unsigned int i = found-1; i >=0; --i) {
     if (_items[i]->isVisible()) {
      found_previous = i;
      break;
    }
  }
  if(found_previous == _items.size())
    return;


  GLWidget* glWidgetLeft = glWidgetAtPosition(found_previous);
  GLWidget* glWidgetRight = glWidgetAtPosition(found);
  QLabel* labelLeft = labelAtPosition(found_previous);
  QLabel* labelRight = labelAtPosition(found);
  
  removeWidgetAtPosition(found);
  removeWidgetAtPosition(found_previous);
  _layout->addWidget(glWidgetLeft,0,found);
  _layout->addWidget(labelLeft,1,found);
  _layout->addWidget(glWidgetRight,0,found_previous);
  _layout->addWidget(labelRight,1,found_previous);
  //    this->adjustSize();


  Item *temp = _items[found];
  _items[found] = _items[found_previous];
  _items[found_previous] = temp;
  
}

void Gallery::moveItemRight(Item *pItem) {
  _has_changed = true;
  unsigned int found = _items.size();

  int x = int(_itemWidth * _items[0]->relativeSize());

  for (unsigned int i = 0; i < _items.size(); i++) {
    if (_items[i] == pItem) {
      found = i;
      break;
    }
    x += int(_itemWidth * _items[i]->relativeSize());
  }
  
  if (x > this->mapFromGlobal(QCursor::pos()).x())
    return;
  if(found == _items.size())
    return;
  if (found == _items.size() - 1)
    return;

  //now look for the next one which is not hidden
  unsigned int found_next= _items.size();
   for (unsigned int i = found+1; i < _items.size(); i++) {
     if (_items[i]->isVisible()) {
      found_next = i;
      break;
    }
  }
  if (found_next == _items.size())
    return;


  GLWidget* glWidgetLeft = glWidgetAtPosition(found);
  GLWidget* glWidgetRight = glWidgetAtPosition(found_next);
  QLabel* labelLeft = labelAtPosition(found);
  QLabel* labelRight = labelAtPosition(found_next);
  
  removeWidgetAtPosition(found);
  removeWidgetAtPosition(found_next);

  _layout->addWidget(glWidgetRight,0,found);
  _layout->addWidget(labelRight,1,found);
  _layout->addWidget(glWidgetLeft,0,found_next);
  _layout->addWidget(labelLeft,1,found_next);
  //  this->adjustSize();

  Item *temp = _items[found];
  _items[found] = _items[found_next];
  _items[found_next] = temp;

}

//return a copy of glwidget at positioni
GLWidget* Gallery::glWidgetAtPosition(unsigned int i){
  QLayoutItem* glLayout = _layout->itemAtPosition(0,i);
  GLWidget* glWidget = nullptr;
  if (_galleryType == CON){
    //glWidget = new ConViewer(dynamic_cast<ConViewer*>(glLayout->widget()));
    glWidget =  dynamic_cast<ConViewer*>(_items[i]->getGLWidget());
  }
  if (_galleryType == FUNC){
    //glWidget  = new FuncViewer(dynamic_cast<FuncViewer*>(glLayout->widget()));
    glWidget =  dynamic_cast<FuncViewer*>(_items[i]->getGLWidget());
  }

  return glWidget;
}

//return a copy of label at positioni
QLabel* Gallery::labelAtPosition(unsigned int i){
  return _items[i]->getName();
  
  QLayoutItem* labelLayout = _layout->itemAtPosition(1,i);
  QLabel* label = dynamic_cast<QLabel*>(labelLayout->widget());
  QLabel* copyLabel = new QLabel(label->text(),this);
  copyLabel->setAlignment(Qt::AlignHCenter);
  return copyLabel;
}

unsigned int Gallery::positionOfItem(Item* pItem){
  for (unsigned int i = 0; i < _items.size(); ++i){
    if (_items[i] == pItem){
      return i;
    }
  }
  return _items.size()-1;
}

void Gallery::removeWidgetAtPosition(unsigned int i){
      QLayoutItem* glLayout = _layout->itemAtPosition(0,i);
      QWidget* glWidget = glLayout->widget();
      QLayoutItem* labelLayout = _layout->itemAtPosition(1,i);
      QWidget* label = labelLayout->widget();
      _layout->removeWidget(glWidget);
      //glWidget->hide();
      _layout->removeWidget(label);
      //label->hide();
}

void Gallery::removeItem() {
  _has_changed = true;
  unsigned int foundPosition = 0;
  // remove all the widgets to the right of the selected ite
  for (unsigned int i = _items.size()-1; i >= 0; --i){
      QLayoutItem* glLayout = _layout->itemAtPosition(1,i);

    _layout->removeWidget(_items[i]->getGLWidget());
    _items[i]->getGLWidget()->hide();
    _layout->removeWidget(_items[i]->getName());
    _items[i]->getName()->hide();
    if (_items[i] == _selectedItem){
      foundPosition = i;
      //_items[i]->hide();
      break;
    }
  }
  

  //delete the widget
  _items.erase(_items.begin()+foundPosition);

  //delete _selectedItem;
  _selectedItem = _items[_items.size()-1];
  if (foundPosition < _items.size())
    _selectedItem = _items[foundPosition];
  _selectedItem = nullptr;
  // put all the widgets back in the layout
  for (unsigned int i = foundPosition; i < _items.size(); ++i){
    _layout->addWidget(_items[i]->getGLWidget(), 0, i);
    _layout->addWidget(_items[i]->getName(), 1, i);
     _items[i]->getGLWidget()->show();
     _items[i]->getName()->show();
  }

    //build size of the application
  resizeWindow();
  this->adjustSize();
  if (_savingMode != OFF) {
    if (_galleryType == FUNC)
      saveFuncSet(_galleryFilename.toStdString().c_str());
    if (_galleryType == CON)
      saveConSet(_galleryFilename.toStdString().c_str());
    setSavingMode();

    _has_changed = false;
  }
  
}
