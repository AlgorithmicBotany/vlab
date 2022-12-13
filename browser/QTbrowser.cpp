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



#include <QFileDialog>
#include <QMenu>
#include <assert.h>
#include <errno.h>
#include <fstream>
#include <limits.h>
#include <QApplication>
#include <QCursor>
#include <QLayout>
#include <QMenuBar>
#include <stack>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <QFrame>
#include <QFontDialog>
#include <QStatusBar>

#include <QCloseEvent>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QImage>
#include <QInputDialog>
#include <QKeyEvent>
#include <QMainWindow>
#include <QMenu>
#include <QPixmap>
#include <QResizeEvent>
#include <QShortcut>
#include <QTextStream>
#include <QTimer>
#include <QUrl>
#include <QtGui> // lazy
#include <QTextBrowser>

using namespace Qt;

#include "FindDialog.h"
#include "QTGLbrowser.h"
#include "QTbrowser.h"
#include "about.h"
#include "buildTree.h"
#include "buttons.h"
#include "debug.h"
#include "dsprintf.h"
#include "font.h"
#include "graphics.h"
#include "main.h"
#include "openNode.h"
#include "xmemory.h"
#include "xutils.h"
#include "CustomizeDialog.h"
#include "FinderWidget.h"
#include "FixOofsDialog.h"
#include "Mem.h"
#include "dragDrop.h"
#include "dsprintf.h"
#include "newbrowser.h"
#include "nodeinfo.h"
#include "platform.h"
#include "vlab_help.h"
#include "xmemory.h"
#include "xstring.h"
#include "Export.h"
#include "Import.h"
#include "comm.h"
#include "qtsupport.h"
#include "resources.h"

#ifdef __APPLE__
#include "cocoabridge.h"

#include <errno.h>
#include <sys/sysctl.h>
#endif

#include <iostream>

// Constructor
QTbrowser::QTbrowser(QWidget *parent)
    : QMainWindow(parent),
      lastIconSize(-1), _customize_dialog(NULL),
      _idleIconLoaderInstalled(false),_findDialog(0) {
#ifdef __APPLE__
  CocoaBridge::setAllowsAutomaticWindowTabbing(false);
  char str[256];
  size_t size = sizeof(str);
  sysctlbyname("kern.osrelease", str, &size, NULL, 0);
  int version, x1, x2;
  sscanf(str, "%d.%d.%d", &version, &x1, &x2);
  if (version > 12) {
    // fix Mac OS X 10.9 (mavericks) font issue
    // https://bugreports.qt-project.org/browse/QTBUG-32789
    QFont::insertSubstitution(".Helvetica Neue DeskInterface", "Lucida Grande");
    QFont::insertSubstitution(".Lucida Grande UI", "Lucida Grande");
  }
#endif
  nb_of_objectDiagonals = 0;
  lastIconLoaded.start();
  sysInfo.mainForm = this;
  sysInfo.obj_posx = 100;
  sysInfo.obj_posy = 100;
  setFocusPolicy(Qt::StrongFocus);

  // Set main window dimension
  winWidth = DEFAULT_W;
  winHeight = DEFAULT_H;
  exportPath = QString();
  exportArchiveType = 1;

  resize(winWidth, winHeight);
  setWindowTitle(QString("Browser %1:%2")
                     .arg(sysInfo.connection->host_name)
                     .arg(sysInfo.oofs_dir_rp));

#ifndef __APPLE__
  setWindowIcon(QIcon(":/linux-icon.png"));
#endif
 
  // Create window status label
  statusBar()->showMessage("Browser is ready.");

  // Construct menus
  // ------------------------------
  //
  // Create file menu
  QMenu *fileMenu = new QMenu(this);
  fileMenu->addAction("New browser ...", this, SLOT(newBrowserCB()),
                      CTRL + Key_N);
  sysInfo.newShellButtonP = fileMenu;
  sysInfo.newShellButtonID = fileMenu->addAction(
      "Open shell", this, SLOT(openShellCB()), Qt::CTRL + Qt::Key_S);
  sysInfo.fileOpenButtonP = fileMenu;
  sysInfo.fileOpenButtonID =
      fileMenu->addAction("Open file", this, SLOT(fileopenCB()));
  if (sysInfo.connection->connection_type == RA_REMOTE_CONNECTION)
    sysInfo.fileOpenButtonID ->setEnabled(false);

#ifdef __APPLE__
  sysInfo.fileOpenConsoleP = fileMenu;
  sysInfo.fileOpenConsoleID =
      fileMenu->addAction("Open console", this, SLOT(fileopenconsoleCB()));
#endif
  fileMenu->addSeparator();
  sysInfo.importButtonP = fileMenu;
  sysInfo.importButtonID =
      fileMenu->addAction("Import", this, SLOT(importObject()), CTRL + Key_I);
  sysInfo.exportButtonP = fileMenu;
  sysInfo.exportButtonID =
      fileMenu->addAction("Export", this, SLOT(exportObject()), CTRL + Key_E);

  sysInfo.customizeButtonP = fileMenu;
  sysInfo.customizeButtonID =
      fileMenu->addAction("Preferences...", this, SLOT(customizeCB()));
  fileMenu->addSeparator();
  //  fileMenu->addAction("Update database", this, SLOT(checkDatabaseCB()));
  //  fileMenu->addSeparator();
  fileMenu->addAction("Exit", this, SLOT(close()), CTRL + Key_Q);

  // Create view menu
  QMenu *viewMenu = new QMenu(this);
  sysInfo.extButtonP = viewMenu;
  sysInfo.extButtonID =
      viewMenu->addAction("Show extensions", this, SLOT(showExtCB()));
  sysInfo.allExtButtonP = viewMenu;
  sysInfo.allExtButtonID =
      viewMenu->addAction("Show all extensions", this, SLOT(showAllExtCB()));
  viewMenu->addSeparator();
  sysInfo.iconButtonP = viewMenu;
  sysInfo.iconButtonID =
      viewMenu->addAction("Show icon", this, SLOT(showIconCB()));
  sysInfo.hideAllIcsButtonP = viewMenu;
  sysInfo.hideAllIcsButtonID =
      viewMenu->addAction("Hide all icons", this, SLOT(hideAllIconsCB()));
  sysInfo.showAllIcsButtonP = viewMenu;
  sysInfo.showAllIcsButtonID =
      viewMenu->addAction("Show all icons", this, SLOT(showAllIconsCB()));
  viewMenu->addSeparator();
  //  sysInfo.centreButtonP = viewMenu;
  // centreCB doesn't do anything
  //  sysInfo.centreButtonID =
  //    viewMenu->addAction("Centre object", this, SLOT(centreCB()));
  sysInfo.centreHyperlinkTargetButtonP = viewMenu;
  sysInfo.centreHyperlinkTargetButtonID = viewMenu->addAction(
      "Show hyperlink target", this, SLOT(centreHyperlinkTargetCB()),
      Qt::CTRL + Qt::Key_P);
  sysInfo.begTreeButtonP = viewMenu;
  sysInfo.begTreeButtonID =
      viewMenu->addAction("Begin tree here", this, SLOT(beginTreeCB()));
  viewMenu->addAction("Begin tree from root", this, SLOT(beginFromRootCB()));

  // Create object menu
  QMenu *objectMenu = new QMenu(this);
  sysInfo.addButtonP = objectMenu;
  sysInfo.addButtonID =
      objectMenu->addAction("New object", this, SLOT(add_object_menu_cb()));
  //  sysInfo.addHButtonP = objectMenu;
  //  sysInfo.addHButtonID = objectMenu->addAction("New hyperobject", this,
  //                                             SLOT(add_Hobject_menu_cb()));
  objectMenu->addSeparator();
  sysInfo.getButtonP = objectMenu;
  sysInfo.getButtonID = objectMenu->addAction("Get", this, SLOT(getObjectCB()));
  sysInfo.renameButtonP = objectMenu;
  sysInfo.renameButtonID =
      objectMenu->addAction("Rename", this, SLOT(rename_cb()), CTRL + Key_R);
  sysInfo.cutButtonP = objectMenu;
  sysInfo.cutButtonID = objectMenu->addAction("Cut", this, SLOT(cut_menu_cb()));
  sysInfo.copyNodeButtonP = objectMenu;
  sysInfo.copyNodeButtonID = objectMenu->addAction(
      "Copy object", this, SLOT(copy_node_menu_cb()), CTRL + Key_C);
  sysInfo.hypercopyNodeButtonP = objectMenu;
  sysInfo.hypercopyNodeButtonID = objectMenu->addAction(
      "Hypercopy object", this, SLOT(hypercopy_node_cb()), CTRL + Key_H);
  sysInfo.copySubtreeButtonP = objectMenu;
  sysInfo.copySubtreeButtonID = objectMenu->addAction(
      "Copy subtree", this, SLOT(copy_subtree_menu_cb()), SHIFT + CTRL + Key_C);
  //  sysInfo.hypercopySubtreeButtonP = objectMenu;
  // sysInfo.hypercopySubtreeButtonID =
  //    objectMenu->addAction("Hypercopy subtree", this,
  //                          SLOT(hypercopy_subtree_cb()), SHIFT + CTRL + Key_H);
  sysInfo.pasteButtonP = objectMenu;
  sysInfo.pasteButtonID =
      objectMenu->addAction("Paste", this, SLOT(paste_menu_cb()), CTRL + Key_V);
  sysInfo.deleteButtonP = objectMenu;
  sysInfo.deleteButtonID = objectMenu->addAction(
      "Delete", this, SLOT(delete_menu_cb()), CTRL + Key_K);
  objectMenu->addSeparator();
  sysInfo.links_buttonP = objectMenu;
  sysInfo.links_buttonID =
      objectMenu->addAction("Move h-links", this, SLOT(links_button_cb()));
  sysInfo.move_links = false;
  sysInfo.links_buttonID->setCheckable(true);
  sysInfo.links_buttonID->setChecked(false);

  // Create search menu
  // int keyID;

  QMenu *searchMenu = new QMenu(this);
  searchMenu->addAction("Find", this, SLOT(find_menu_cb()), CTRL + Key_F);
  QShortcut *keyBind = new QShortcut(QKeySequence(tr("\\")), this);
  connect(keyBind, SIGNAL(activated()), this, SLOT(find_menu_cb()));

  // Create help menu
  QMenu *helpMenu = new QMenu(this);
  helpMenu->addAction("About VLAB", this, SLOT(about_vlab_cb()));
  //  helpMenu->addAction("About Rayshade", this, SLOT(about_Rayshade_cb()));
  helpMenu->addAction("About Qt", qApp, SLOT(aboutQt()));
#ifndef __APPLE__
  helpMenu->addAction("Online help", this, SLOT(helpCB()));
#endif

  // Construct menubar
  QMenuBar *menubar;
#if defined(Q_WS_MAC)
  menubar = new QMenuBar(0);
#else
  menubar = menuBar();
#endif
  fileMenu->setTitle("&File");
  menubar->addMenu(fileMenu);
  viewMenu->setTitle("&View ");
  menubar->addMenu(viewMenu);
  objectMenu->setTitle("&Object");
  menubar->addMenu(objectMenu);
  searchMenu->setTitle("&Search");
  menubar->addMenu(searchMenu);

  QMenu *help = menubar->addMenu("&Help");
  QAction *qHelp = help->addAction("Quick help", this, SLOT(quickHelp()));
  help->addAction("Getting Started", this, SLOT(gettingStartedHelp()));
  help->addAction("Vlab framework", this, SLOT(newVlabHelp()));
  help->addAction("Cpfg manual", this, SLOT(newCpfgHelp()));
  help->addAction("Lpfg manual", this, SLOT(newLpfgHelp()));
  help->addAction("Tools manual", this, SLOT(vlabToolsHelp()));
  help->addAction("Environmental programs", this, SLOT(newEnviroHelp()));
  help->addAction("Rayshade", this, SLOT(rayshadeHelp()));
  qHelp->setEnabled(true);

  help->addAction("About VLAB", this, SLOT(about_vlab_cb()));
  help->addAction("About Qt", qApp, SLOT(aboutQt()));
  //help->addAction("About Rayshade", this, SLOT(about_Rayshade_cb()));

  _finderWidget = new FinderWidget(statusBar(), "finder");
  _finderWidget->hide();
  statusBar()->addWidget(_finderWidget);

  // if the connection is not local, hide the 'unavailable' buttons
  if (sysInfo.connection->connection_type != RA_LOCAL_CONNECTION) {
    sysInfo.fileOpenButtonID->setEnabled(false);
    sysInfo.newShellButtonID->setEnabled(false);
  }

  // create the customization dialog
  _customize_dialog = new CustomizeDialog(this);
  connect(_customize_dialog, SIGNAL(settingsChanged(const BrowserSettings &)),
          SLOT(applySettings(const BrowserSettings &)));
  connect(_customize_dialog,
          SIGNAL(settingsColorsChanged(const BrowserSettings &)),
          SLOT(applySettingsColors(const BrowserSettings &)));

  // read a new tree in
  get_new_tree(sysInfo.oofs_dir_rp);

  // no cut/copy data ready
  sysInfo.pasteReady = false;
  sysInfo.pasteLinkReady = false;
  // Create openGL widget display
  scroll = new QScrollArea();
  globj = new QTGLbrowser(this, scroll, "globj");
  notifier =
      new QSocketNotifier(sysInfo.vlabd->get_sock(), QSocketNotifier::Read);
  connect(notifier, SIGNAL(activated(int)), SLOT(inputReady(int)));

  // initialize the graphics
  graphics_init();

  applySettings(browserSettings());

  setAcceptDrops(true);

  scroll->setWidget(globj);
  QPalette pal;
  pal.setColor(QPalette::Window, QColor(0, 0, 0, 0));
  scroll->setPalette(pal);
  scroll->setBackgroundRole(QPalette::Window);
  scroll->setWidgetResizable(false);
  setCentralWidget(scroll);
  scroll->show();

#ifdef USE_PLUGINS
  fs_watcher.addPath(sysInfo.pluginDir);
  connect(&fs_watcher, SIGNAL(directoryChanged(const QString &)), &fs_timer,
          SLOT(start()));
  fs_timer.setInterval(200);
  fs_timer.setSingleShot(true);
  connect(&fs_timer, SIGNAL(timeout()), this, SLOT(pluginsChanged()));
#endif
}

// Destructor
QTbrowser::~QTbrowser() {
  if (globj)
    delete globj;
}

void QTbrowser::newVlabHelp() {
  QDir helpDir(getHelpDirectory());
  QDesktopServices::openUrl(
      QUrl::fromLocalFile(helpDir.filePath("VLABFramework.pdf")));
}

void QTbrowser::newCpfgHelp() {
  QDir helpDir(getHelpDirectory());
  QString absPath = helpDir.absolutePath();
  QDesktopServices::openUrl(
      QUrl::fromLocalFile(helpDir.filePath("CPFGManual.pdf")));
}

void QTbrowser::newLpfgHelp() {
  QDir helpDir(getHelpDirectory());
  QDesktopServices::openUrl(
      QUrl::fromLocalFile(helpDir.filePath("LPFGManual.pdf")));
}

void QTbrowser::newEnviroHelp() {
  QDir helpDir(getHelpDirectory());
  QDesktopServices::openUrl(
      QUrl::fromLocalFile(helpDir.filePath("EnviroManual.pdf")));
}
void QTbrowser::vlabToolsHelp() {
  QDir helpDir(getHelpDirectory());
  QDesktopServices::openUrl(
      QUrl::fromLocalFile(helpDir.filePath("VLABToolsManual.pdf")));
}

void QTbrowser::quickHelp(){
  QDir helpDir(getHelpDirectory());
  QString path = helpDir.filePath("Quick_Help/BrowserQuickHelp.html");
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
  msgBox->setWindowTitle("Browser: Quick Help");
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


void QTbrowser::gettingStartedHelp() {
  QDir helpDir(getHelpDirectory());
  QDesktopServices::openUrl(
      QUrl::fromLocalFile(helpDir.filePath("GettingStarted.pdf")));
}

void QTbrowser::rayshadeHelp() {
  QDir helpDir(getHelpDirectory());
  QDesktopServices::openUrl(
      QUrl::fromLocalFile(helpDir.filePath("rayshade.pdf")));
}

// Window close event handler
void QTbrowser::closeEvent(QCloseEvent *ev) {
  if (quitCB()) {
    ev->accept();
  } else {
    ev->ignore();
  }
}

void QTbrowser::newBrowserCB() {
  QByteArray userNameData = sysInfo.login_name.toLatin1();
  const char *userNameChar = userNameData.constData();

  QString path_str = QString("%1@%2:%3")
                         .arg(userNameChar)
                         .arg(sysInfo.host_name)
                         .arg(sysInfo.oofs_dir);
  NewBrowserDlg dlg(path_str, sysInfo.password, this);
  if (dlg.exec() == QDialog::Accepted) {
    newBrowser(dlg.oofs().toLatin1());
  }
}

#ifdef __APPLE__
void QTbrowser::fileopenconsoleCB() {
  QString cmd = "open /Applications/Utilities/Console.app";
  pid_t pid = vfork();
  if (pid == -1) {
    std::cerr << "browser: Cannot execute " << cmd.toStdString()
              << " because fork() failed'";
  } else if (pid == 0) {
    execlp("open", "open", "/Applications/Utilities/Console.app", NULL);
    std::cerr << "browser: Cannot execute " << cmd.toStdString()
              << " because execlp() failed'";
    _exit(0);
  } else {
    // Ok, nothing to do ...
  }
}
#else
void QTbrowser::fileopenconsoleCB() {
  // 2012.03.19 PBdR: Do nothing, this is the only way to have the signals
  // working correctly as moc will ignore preprocessing directives
}
#endif

// Set input message handler
void QTbrowser::inputReady(int) { handleMessages(); }

void QTbrowser::openShellCB()
// open a shell callback
{
  if (sysInfo.selNode == NULL)
    return;

  QString shell_cmd = browserSettings().shellCommandLine();

  // fork and exec the shell
  pid_t pid = fork();
  if (pid == -1) {
    char msg[4096];
    sprintf(msg,
            "Could not execute an external process.\n"
            "   - %s\n",
            strerror(errno));
    vlabxutils::infoBox(this, msg, "Error");
    return;
  }

  // parent returns
  if (pid != 0)
    return;

  // execute the shell in the selected node's directory
  chdir(sysInfo.selNode->name);
  QByteArray cmd = shell_cmd.toLatin1();
  execlp("/bin/bash", "/bin/bash", "-f", "-c", cmd.data(), NULL);

  // if execution gets to this point, execlp() has failed. report error
  fprintf(stderr, "Could not exec '%s': %s\n", "/bin/bash", strerror(errno));

  qApp->quit();
}

void QTbrowser::fileopenCB()
// callback for opening the file selection widget
{
  if (!sysInfo.selNode)
    return;

  QString dirname = sysInfo.selNode->name;
  QString fileName =
      QFileDialog::getOpenFileName(this, "File selection", dirname);

  if (!fileName.isEmpty()) {
    QString editor = browserSettings().editorCommandLine().arg(fileName);
    if (editor.isEmpty()) {
      vlabxutils::infoBox(this,
                          "Cannot determine which editor you would\n"
                          "like to use. Please set command line for\n"
                          "the editor from the preferences dialog box.",
                          "Warning");
      return;
    }
    system(editor.toLocal8Bit());
  }
}

void QTbrowser::customizeCB()
// calls the function custmize(), which will allow the user to interactively
// change the settings for the browser
{
  _customize_dialog->show();
  _customize_dialog->raise();
}

void QTbrowser::checkDatabaseCB()
// ---------------------------------------------------------------------------
// calls an external program 'checkdbase' to check the consistency of the
// database
// ---------------------------------------------------------------------------
{
  static FixOofsDialog *dialog = new FixOofsDialog(this);
  dialog->show();
  return;
  std::string log =
      RA::uuidTableReconcile(sysInfo.connection,
                             sysInfo.oofs_dir_rp, // the root of the database
                             sysInfo.oofs_dir_rp, // start reconcile at root
                             true,                // recursive
                             true);
  // tell browsers we changed UUID table
  // checking if connection is still alive
  if (sysInfo.connection->reconnect()) {
    vlabxutils::infoBox(sysInfo.mainForm,
                        "Can't check DataBase\n"
                        "Connection with raserver is down\n"
                        "Check your network connection\n",
                        "Error");
    return;
  }
  QByteArray userNameData = sysInfo.login_name.toLatin1();
  const char *userNameChar = userNameData.constData();

  sysInfo.vlabd->va_send_message(UUIDTABLECHANGED, "%s@%s:%s", userNameChar,
                                 sysInfo.host_name, sysInfo.oofs_dir);
  sysInfo.connection->Disconnect();

  QMessageBox msgBox;
  msgBox.setText("Results of database update");
  std::string log_mess = log + "\n";
  msgBox.setDetailedText(log_mess.c_str());
  msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard |
                            QMessageBox::Cancel);
  msgBox.setDefaultButton(QMessageBox::Save);
  msgBox.setSizeGripEnabled(true);
  int ret = msgBox.exec();
  if (ret == QMessageBox::Save) {
    QString fname = QString(sysInfo.database) + "/.fixoofs_log";
    QFile f(fname);
    if (f.open(QFile::WriteOnly)) {
      QTextStream out(&f);
      out << QString(log_mess.c_str()) << "\n";
    }
  }
}

bool QTbrowser::quitCB()
// callback for the "QUIT" button
{
  QMessageBox msgBox;
  msgBox.addButton(QMessageBox::Yes);
  msgBox.addButton(QMessageBox::No);
  msgBox.setText("Are you sure you want to quit?");
  QPixmap logo(":/logo-small.png");
  logo = logo.scaled(96, 96, Qt::KeepAspectRatio, Qt::SmoothTransformation);
  msgBox.setIconPixmap(QPixmap(logo));
  int selection = msgBox.exec();
  if (selection == QMessageBox::Yes) {
    this->setWindowIcon(QPixmap(logo));
    notifier->setEnabled(false);
    vlab_close();
    fflush(stdout);
    fflush(stderr);
    return true;
  } else if (selection == QMessageBox::No) {
    return false;
  }
  return false;
}

void QTbrowser::centreHyperlinkTargetCB() {
  if (sysInfo.selNode == 0)
    return;
  if (!sysInfo.selNode->isHObj)
    return;
  if (!sysInfo.selNode->object_name)
    return;
  QByteArray userNameData = sysInfo.login_name.toLatin1();
  const char *userNameChar = userNameData.constData();

  std::string message = std::string(userNameChar) + "@" + sysInfo.host_name +
                        ":" + sysInfo.selNode->object_name;
  node_position(message.c_str());
  build_tree();
  sysInfo.mainForm->update_menus();
  centre_node(sysInfo.selNode);
  sysInfo.mainForm->updateDisplay();
}

void QTbrowser::showExtCB()
// callback for the button 'show extensions'
{
  if (sysInfo.selNode == NULL)
    return;
  if (sysInfo.selNode->nChildren > 0)
    hide_extensions(sysInfo.selNode);
  else {
    setCursor(WaitCursor);
    if (show_extensions(sysInfo.selNode) == -1)
      return;
    setCursor(ArrowCursor);
  }

  // refresh display etc
  build_tree();
  update_menus();
  centre_node(sysInfo.selNode);
  updateDisplay();
}

void QTbrowser::showAllExtCB()
// callback for the button 'show all extensions'
{
  if (sysInfo.selNode == NULL)
    return;
  // this could take a while, show a working dialog
  vlabxutils::tempBoxPopUp(topX(), topY(), this, "Working");
  // do the actual work
  if (show_all_extensions(sysInfo.selNode))
    return;
  // refresh display etc
  build_tree();
  update_menus();
  centre_node(sysInfo.selNode);
  updateDisplay();
  // pop down the working dialog
  vlabxutils::tempBoxPopDown(this);
}

/******************************************************************************
 *
 * show/hide icon callback
 *
 */
void QTbrowser::showIconCB() {
  if (sysInfo.selNode == NULL)
    return;
  if (sysInfo.selNode->iconShow == false) {
    showIcon(sysInfo.selNode);
  } else {
    hideIcon(sysInfo.selNode);
  }

  // update the menus
  update_menus();
  // rebuild the tree
  build_tree();
  updateDisplay();
}

void QTbrowser::hideAllIconsCB()
// hide all icons callback
{
  hideAllIcons_in_tree(sysInfo.selNode);

  /*** update the menus ***/
  update_menus();
  /* rebuild the tree */
  build_tree();
  updateDisplay();
}

void QTbrowser::showAllIconsCB()
// show all icons callback
{
  if (sysInfo.selNode == NULL)
    return;

  vlabxutils::tempBoxPopUp(topX(), topY(), this, "Reading in the icons...");

  showAllIcons_in_tree(sysInfo.selNode);
  // update the menus
  update_menus();
  // rebuild the tree
  build_tree();
  centre_node(sysInfo.selNode);
  // pop down the working dialog
  vlabxutils::tempBoxPopDown(this);
  updateDisplay();
}

void QTbrowser::centreCB()
// centre the object
{
  if (sysInfo.selNode == NULL)
    return;

  build_tree();
  centre_node(sysInfo.selNode);
  updateDisplay();
}

void QTbrowser::beginTreeCB()
// begin tree here / show parent
{
  if (sysInfo.selNode == NULL)
    return;

  if (sysInfo.selNode != sysInfo.beginTree) {
    /* we want to BEGIN TREE HERE */
    sysInfo.beginTree = sysInfo.selNode;
    /* rebuild the tree */
    build_tree();
    /* update menus */
    update_menus();
  } else {
    /* we want to show another parent */
    sysInfo.beginTree = sysInfo.selNode->parent;
    assert(sysInfo.beginTree != NULL);
    /* rebuild the tree */
    build_tree();
    /* update menus */
    update_menus();
  }
  updateDisplay();
}

/******************************************************************************
 *
 * begin tree from root
 *
 */
void QTbrowser::beginFromRootCB() {
  // checking if connection is still alive
  if (sysInfo.connection->reconnect()) {
    vlabxutils::infoBox(sysInfo.mainForm,
                        "Can't Begin Tree from here\n"
                        "Connection with raserver is down\n"
                        "Check your network connection\n",
                        "Error");
    return;
  }

  /* we want to BEGIN TREE HERE */
  sysInfo.beginTree = sysInfo.wholeTree;
  /* rebuild the tree */
  build_tree();
  /* update menus */
  update_menus();
  updateDisplay();
  sysInfo.connection->Disconnect();
}

/******************************************************************************
 *
 * callback function for 'get object'
 *
 */
void QTbrowser::getObjectCB() {
  std::string pwdString = sysInfo.password.toStdString();
  const char *pwd = pwdString.c_str();
  std::string loginString = sysInfo.login_name.toStdString();
  const char *login = loginString.c_str();

  if (sysInfo.selNode == NULL) {
    fprintf(stderr, "No object selected\n");
    return;
  }
  // checking if connection is still alive
  if (sysInfo.connection->reconnect()) {
    vlabxutils::infoBox(sysInfo.mainForm,
                        "Can't get object\n"
                        "Connection with raserver is down\n"
                        "Check your network connection\n",
                        "Error");
    return;
  }
  if (sysInfo.selNode->isHObj == 0) {
    // send the request to the vlabd - to invoke an object
    sysInfo.vlabd->va_send_message(
        GETOBJECT, "-rootdir %s -p '%s' -posx %d -posy %d %s@%s:%s",
        sysInfo.oofs_dir, sysInfo.password == QString::null ? "" : pwd,
        sysInfo.obj_posx, sysInfo.obj_posy, login, sysInfo.host_name,
        sysInfo.selNode->name);
  } else if (sysInfo.selNode->isHObj == 1) {
    // make sure that this node is associate with an object
    if (sysInfo.selNode->object_name == NULL)
      return;

    // send the request to the vlabd - to invoke an object
    sysInfo.vlabd->va_send_message(
        GETOBJECT, "-rootdir %s -p '%s'  -posx %d -posy %d %s@%s:%s",
        sysInfo.oofs_dir, sysInfo.password == QString::null ? "" : pwd,
        sysInfo.obj_posx, sysInfo.obj_posy, login, sysInfo.host_name,
        sysInfo.selNode->object_name);
  }
  if (sysInfo.obj_posx < 400) {
    sysInfo.obj_posx += 20;
    sysInfo.obj_posy += 20;
  } else {
    nb_of_objectDiagonals++;
    sysInfo.obj_posx = 20;
    sysInfo.obj_posy = nb_of_objectDiagonals * 20;
  }
  //  std::cerr << sysInfo.obj_posx << " - " << sysInfo.obj_posy << " - "
  //          << nb_of_objectDiagonals << std::endl;
  sysInfo.connection->Disconnect();
}

/******************************************************************************
 *
 * cut callbacks:
 *
 */
void QTbrowser::cut_menu_cb() {
  cut_cb();
  update_menus();
  updateDisplay();
}

/******************************************************************************
 *
 * callbacks for copy node
 *
 */
void QTbrowser::copy_node_menu_cb() { copy_node_cb(); }

/******************************************************************************
 *
 * callbacks for the copy subtree
 *
 */
void QTbrowser::copy_subtree_menu_cb() { copy_subtree_cb(); }

/******************************************************************************
 *
 * callbacks for paste
 *
 */
void QTbrowser::paste_menu_cb() { paste_cb(); }

/******************************************************************************
 *
 * delete callbacks:
 *
 */
void QTbrowser::delete_menu_cb() {
  delete_cb();
  update_menus();
  updateDisplay();
}

/******************************************************************************
 *
 * Add new object callback:
 *
 */
void QTbrowser::add_object_menu_cb() { new_object_cb(); }

/******************************************************************************
 *
 * Add new hyperobject callback:
 *
 */
void QTbrowser::add_Hobject_menu_cb() { new_Hobject_cb(); }

/******************************************************************************
 *
 * callback for the 'links_button'
 *
 */
void QTbrowser::links_button_cb() {
  if (sysInfo.move_links)
    sysInfo.move_links = false;
  else
    sysInfo.move_links = true;
  // checking if connection is still alive
  if (sysInfo.connection->reconnect()) {
    vlabxutils::infoBox(sysInfo.mainForm,
                        "Can't link\n"
                        "Connection with raserver is down\n"
                        "Check your network connection\n",
                        "Error");
    return;
  }

  sysInfo.vlabd->send_message(MOVELINKS,
                              (char *)(sysInfo.move_links ? "1" : "0"));
  sysInfo.connection->Disconnect();

  update_menus();
}

// about vlab slot - displays the about dialog
void QTbrowser::about_vlab_cb() {
  vlab::about(this,"Browser");
}

// about vlab slot - displays the about dialog
void QTbrowser::about_Rayshade_cb() {
  vlab::aboutRayshade(this);
}

/******************************************************************************
 *
 * help callback(manual page)
 *
 */
void QTbrowser::helpCB() {
  char *docDirName = getenv("VLABDOCDIR");

  if (docDirName == NULL) {
    vlabxutils::infoBox(this, "You forgot to set up your VLABDOCDIR\nvariable!",
                        "Warning");
    return;
  }

  if (access(docDirName, X_OK) != 0) {
    vlabxutils::infoBox(this,
                        "The environment variable VLABDOCDIR\n"
                        "is not set correctly!\n"
                        "You have to set it to the directory\n"
                        "where the file 'VLABFramework.pdf' is located.",
                        "Error");
    return;
  }

  char url[4096];
  sprintf(url, "file:%s/VLABFramework.pdf", docDirName);
  if (vlab_help(url)) {
    vlabxutils::infoBox(this, "Cannot find netscape executable.", "Error");
    return;
  }
}

void QTbrowser::find_menu_cb() {
  if (_findDialog == NULL) {
    _findDialog = new FindDialog(this);
  }

  _findDialog->show();
}

/******************************************************************************
 *
 * this function will update the menus according to the current node selected
 *
 */
void QTbrowser::update_menus() {
  /* set the graphics type button */

  if (sysInfo.selNode == NULL) {
    /* special case - nothing is selected */
    sysInfo.fileOpenButtonID->setEnabled(false);
    sysInfo.newShellButtonID->setEnabled(false);
    sysInfo.extButtonID->setEnabled(false);
    sysInfo.allExtButtonID->setEnabled(false);
    sysInfo.iconButtonID->setEnabled(false);
    sysInfo.hideAllIcsButtonID->setEnabled(false);
    sysInfo.showAllIcsButtonID->setEnabled(false);
    //sysInfo.centreButtonID->setEnabled(false);
    sysInfo.begTreeButtonID->setEnabled(false);
    sysInfo.getButtonID->setEnabled(false);
    sysInfo.renameButtonID->setEnabled(false);
    sysInfo.cutButtonID->setEnabled(false);
    sysInfo.pasteButtonID->setEnabled(false);
    sysInfo.exportButtonID->setEnabled(false);
    sysInfo.importButtonID->setEnabled(false);
    sysInfo.copyNodeButtonID->setEnabled(false);
    sysInfo.copySubtreeButtonID->setEnabled(false);
    sysInfo.deleteButtonID->setEnabled(false);
    sysInfo.addButtonID->setEnabled(false);
    //    sysInfo.addHButtonID->setEnabled(false);
    sysInfo.hypercopyNodeButtonID->setEnabled(false);
    //    sysInfo.hypercopySubtreeButtonID->setEnabled(false);
    sysInfo.centreHyperlinkTargetButtonID->setEnabled(false);
  } else {
    if (sysInfo.connection->connection_type == RA_REMOTE_CONNECTION){
      sysInfo.fileOpenButtonID ->setEnabled(false);
      sysInfo.newShellButtonID->setEnabled(false);
    }
    else{
      sysInfo.fileOpenButtonID->setEnabled(true);
      sysInfo.newShellButtonID->setEnabled(true);
    }

    sysInfo.iconButtonID->setEnabled(true);
    sysInfo.getButtonID->setEnabled(true);
    sysInfo.renameButtonID->setEnabled(true);
    sysInfo.deleteButtonID->setEnabled(true);
    sysInfo.addButtonID->setEnabled(true);
    //    sysInfo.addHButtonID->setEnabled(true);
    sysInfo.cutButtonID->setEnabled(true);
    sysInfo.copyNodeButtonID->setEnabled(true);
    sysInfo.copySubtreeButtonID->setEnabled(true);
    sysInfo.exportButtonID->setEnabled(true);
    sysInfo.importButtonID->setEnabled(true);
    sysInfo.hideAllIcsButtonID->setEnabled(true);
    sysInfo.showAllIcsButtonID->setEnabled(true);
    //    sysInfo.centreButtonID->setEnabled(true);
    sysInfo.begTreeButtonID->setEnabled(true);

    if (sysInfo.selNode->isHObj != 0) {
      sysInfo.hypercopyNodeButtonID->setEnabled(false);
      //      sysInfo.hypercopySubtreeButtonID->setEnabled(false);
    } else {
      sysInfo.hypercopyNodeButtonID->setEnabled(true);
      //      sysInfo.hypercopySubtreeButtonID->setEnabled(false);
    }

    /* set the 'begin tree here'/'show parent' button */
    if (sysInfo.selNode == sysInfo.wholeTree)
      sysInfo.begTreeButtonID->setEnabled(false);
    else {
      if (sysInfo.selNode == sysInfo.beginTree)
        sysInfo.begTreeButtonID->setText("Show parent");
      else
        sysInfo.begTreeButtonID->setText("Begin tree here");
    }

    /* set the 'show [all]ext buttons' */
    if (sysInfo.selNode->expandable) {
      sysInfo.extButtonID->setEnabled(true);
      sysInfo.allExtButtonID->setEnabled(true);
    } else {
      sysInfo.extButtonID->setEnabled(false);
      sysInfo.allExtButtonID->setEnabled(false);
    }

    /* set the paste button */
    if ((sysInfo.pasteReady) || (sysInfo.pasteLinkReady))
      sysInfo.pasteButtonID->setEnabled(true);
    else
      sysInfo.pasteButtonID->setEnabled(false);

    /* set the show/hide extensions button */
    if (sysInfo.selNode->nChildren == 0)
      sysInfo.extButtonID->setText("Show extensions");
    else
      sysInfo.extButtonID->setText("Hide extensions");

    // update the show/hide icon menu
    if (sysInfo.selNode->iconShow == false)
      sysInfo.iconButtonID->setText("Show icon");
    else
      sysInfo.iconButtonID->setText("Hide icon");
    // update 'centre hyperlink target'
    if (sysInfo.selNode->isHObj && sysInfo.selNode->object_name)
      sysInfo.centreHyperlinkTargetButtonID->setEnabled(true);
    else
      sysInfo.centreHyperlinkTargetButtonID->setEnabled(false);
  }

  if (sysInfo.move_links)
    sysInfo.links_buttonID->setChecked(true);
  else
    sysInfo.links_buttonID->setChecked(false);
}

/******************************************************************************
 *
 * show_status will put the text supplied to the function into the
 * status label at the bottom of the browser window
 *
 */
void QTbrowser::show_status(char *cur_status) {
  statusBar()->showMessage(cur_status);
  update();
}

/******************************************************************************
 *
 * callbacks for rename
 *
 */
void QTbrowser::rename_cb() {
  // see if the user has a permission to rename the selected node
  if (!node_operation_allowed(sysInfo.selNode, OP_RENAME)) {
    vlabxutils::infoBox(
        this, "You do not have permissions to rename this object!", "Warning");
    return;
  }

  QString orig = sysInfo.selNode->baseName;
  if (sysInfo.selNode->isHObj != 0)
    orig = sysInfo.selNode->screenName;

  QString prev_style = this->styleSheet();
  this->setStyleSheet(prev_style + "\nQLineEdit { min-width: 250% }");
  bool ok;

  QString s = QInputDialog::getText(
      this, "Rename object", "Enter new name:", QLineEdit::Normal, orig, &ok);
  this->setStyleSheet(prev_style);
  if (!ok)
    return; // user canceled
  QByteArray sData = s.toLatin1();
  const char *sDataChar = sData.constData();

  char *newName = xstrdup(sDataChar);

  // make the name valid by replacing all invalid characters with '_'
  underscore_string(newName);
  if (!sysInfo.connection->check_connection()) {
    vlabxutils::infoBox(sysInfo.mainForm,
                        "Object can't be renamed\n"
                        "Connection with raserver is down\n"
                        "Check your network connection\n",
                        "Error");
    return;
  }

  if (sysInfo.selNode->isHObj == 0) {
    // create a new name for the node
    char new_full_name[4096];
    strcpy(new_full_name, sysInfo.selNode->name);
    char *ptr = strrchr(new_full_name, '/');
    assert(ptr != NULL);
    ptr++;
    strcpy(ptr, newName);

    // ask raserver to rename the node
    if (RA::Rename_object(sysInfo.connection,
                          sysInfo.oofs_dir_rp,   // the root of the database
                          sysInfo.selNode->name, // name of the src object
                          new_full_name)         // new name for the object
    ) {
      // rename failed
      vlabxutils::infoBox(
          this,
          "Rename failed.\n"
          "\n"
          "Most likely cause: object with this name already exists.",
          "Error");

      return;
    }

    // notify other vlab clients about renaming (and since we will catch
    // this message as well - the tree will get automatically updated
    if (sysInfo.connection->reconnect())
      return;
    QByteArray userNameData = sysInfo.login_name.toLatin1();
    const char *userNameChar = userNameData.constData();

    sysInfo.vlabd->va_send_message(RENAME, "%s@%s:%s,%s", userNameChar,
                                   sysInfo.host_name, sysInfo.selNode->name,
                                   newName);
    // tell browsers we changed .dbase
    sysInfo.vlabd->va_send_message(UUIDTABLECHANGED, "%s@%s:%s", userNameChar,
                                   sysInfo.host_name, sysInfo.oofs_dir);
    sysInfo.connection->Disconnect();
  } else if (sysInfo.selNode->isHObj == 1) {
    // do the actual renaming
    sysInfo.selNode->node_info.name(newName);
    if (!sysInfo.selNode->node_info.write()) {
      vlabxutils::popupInfoBox(sysInfo.mainForm, "Warning",
                               "Could not rename hyperobject %s.",
                               sysInfo.selNode->screenName);
    } else {
      tree_rename(sysInfo.selNode, newName);
      updateDisplay();
      // notify other vlab clients about renaming, except this does not seem to
      // be implemented in comm.c yet (TBD)
      if (sysInfo.connection->reconnect())
        return;
      QByteArray userNameData = sysInfo.login_name.toLatin1();
      const char *userNameChar = userNameData.constData();

      sysInfo.vlabd->va_send_message(HRENAME, "%s@%s:%s", userNameChar,
                                     sysInfo.host_name, sysInfo.selNode->name);
      sysInfo.connection->Disconnect();
    }

    update_menus();
    updateDisplay();
  }
}

/******************************************************************************
 *
 * cut callbacks:
 *
 */
void QTbrowser::cut_cb() {
  std::string pwdString = sysInfo.password.toStdString();
  const char *pwd = pwdString.c_str();
  std::string loginString = sysInfo.login_name.toStdString();
  const char *login = loginString.c_str();

  // if no node is selected, return
  if (sysInfo.selNode == NULL)
    return;

  // we cannot cut a root
  if (sysInfo.selNode->parent == NULL) {
    vlabxutils::infoBox(this, "You cannot cut a root. It is too dangerous!!!",
                        "Warning");
    return;
  }

  // make sure that this operation is allowed
  if (!node_operation_allowed(sysInfo.selNode, OP_CUT)) {
    vlabxutils::infoBox(
        this, "You do not have permissions to 'cut' this object!", "Warning");
    return;
  }

  if (sysInfo.selNode->isHObj == 0) {
    // suspend all browsers & popup a temporary dialog
    if (sysInfo.connection->reconnect()) {
      vlabxutils::infoBox(sysInfo.mainForm,
                          "Can't cut\n"
                          "Connection with raserver is down\n"
                          "Check your network connection\n",
                          "Error");
      return;
    }
    sysInfo.vlabd->send_message(GETBUSY);
    sysInfo.connection->Disconnect();
    vlabxutils::tempBoxPopUp(topX(), topY(), this, "Cutting...");

    // prepare a DELETE message now, because after deletion the data will
    // be lost
    QByteArray userNameData = sysInfo.login_name.toLatin1();
    const char *userNameChar = userNameData.constData();

    char *del_message = dsprintf("%s@%s:%s", userNameChar, sysInfo.host_name,
                                 sysInfo.selNode->name);

    // prepare the paste message (we do it done now because after the node
    // is deleted, it would be more difficult to find the node_path for it)
    char paste_message[4096];
    char node_path[4096];

    node_get_relative_path(sysInfo.selNode, node_path);
    sprintf(paste_message, "%s %s@%s:%s %s", pwd, login, sysInfo.host_name,
            sysInfo.oofs_dir_rp, node_path);

    // cut the subtree
    int res = tree_cut(sysInfo.selNode);
    if (sysInfo.connection->reconnect()) {
      vlabxutils::infoBox(sysInfo.mainForm,
                          "Can't cut\n"
                          "Connection with raserver is down\n"
                          "Check your network connection\n",
                          "Error");
      return;
    }
    // hide the temporary dialog
    vlabxutils::tempBoxPopDown(this);
    // tell to all browsers that they shouldn't be busy
    sysInfo.vlabd->send_message(GETREADY);

    // check if the operation was successful
    if (res) {
      // send message: paste info is ready
      sysInfo.vlabd->send_message(PASTEREADY, paste_message);
      // send message: an object has been deleted
      sysInfo.vlabd->send_message(DELETE, del_message);
      // send a message: '.dbase' being modified
      QByteArray userNameData = sysInfo.login_name.toLatin1();
      const char *userNameChar = userNameData.constData();

      sysInfo.vlabd->va_send_message(UUIDTABLECHANGED, "%s@%s:%s", userNameChar,
                                     sysInfo.host_name, sysInfo.oofs_dir);
      // erase current selection
      sysInfo.selNode = NULL;
    } else {
      vlabxutils::infoBox(this, "Cut failed.", "Warning");
      sysInfo.connection->Disconnect();

      return;
    }
    sysInfo.connection->Disconnect();

    // get rid of the delete message
    xfree(del_message);
  } else if (sysInfo.selNode->isHObj == 1) {
    if (sysInfo.connection->reconnect()) {
      vlabxutils::infoBox(sysInfo.mainForm,
                          "Can't cut\n"
                          "Connection with raserver is down\n"
                          "Check your network connection\n",
                          "Error");
      return;
    }
    sysInfo.vlabd->send_message(GETBUSY);

    sysInfo.connection->Disconnect();
    QByteArray userNameData = sysInfo.login_name.toLatin1();
    const char *userNameChar = userNameData.constData();

    char *message = dsprintf("%s@%s:%s", userNameChar, sysInfo.host_name,
                             sysInfo.selNode->parent->name);
    int res = tree_cut(sysInfo.selNode);
    if (sysInfo.connection->reconnect()) {
      vlabxutils::infoBox(sysInfo.mainForm,
                          "Can't cut\n"
                          "Connection with raserver is down\n"
                          "Check your network connection\n",
                          "Error");
      return;
    }
    if (res) {
      /* paste is ready */
      sysInfo.vlabd->send_message(MPASTEREADY);
      sysInfo.vlabd->send_message(HDELETE, message);
    }
    // Prepare to paste just in case
    sysInfo.pasteLinkReady = true;
    sysInfo.pasteReady = false;
    sysInfo.paste_link_info.uuid = sysInfo.selNode->node_info.uuid();
    sysInfo.paste_link_info.dirNamex = sysInfo.selNode->baseName;

    sysInfo.selNode = NULL;
    update_menus();

    sysInfo.vlabd->send_message(GETREADY);
    updateDisplay();
    sysInfo.connection->Disconnect();
  }
}

/******************************************************************************
 *
 * callbacks for copy node
 *
 */
void QTbrowser::copy_node_cb() {

  std::string pwdString = sysInfo.password.toStdString();
  const char *pwd = pwdString.c_str();
  std::string loginString = sysInfo.login_name.toStdString();
  const char *login = loginString.c_str();

  // if no node is selected, return
  if (sysInfo.selNode == NULL)
    return;

  if (!sysInfo.connection->check_connection()) {
    vlabxutils::infoBox(sysInfo.mainForm,
                        "Can't copy\n"
                        "Connection with raserver is down\n"
                        "Check your network connection\n",
                        "Error");
    return;
  }

  if ((sysInfo.selNode->isHObj == 0) || (sysInfo.selNode->isHObj == 1)) {
    // display a message
    vlabxutils::tempBoxPopUp(topX(), topY(), this, "Copying node...");

    // copy the node
    int res = tree_copy_node(sysInfo.selNode);

    // pop down the temporary box
    vlabxutils::tempBoxPopDown(this);

    // check if the operation was successful
    if (res) {
      sysInfo.copyHyperobject = false;
      // tell the other browsers that paste data is present
      char node_path[4096];
      node_get_relative_path(sysInfo.selNode, node_path);
      sysInfo.vlabd->va_send_message(PASTEREADY, "%s %s@%s:%s %s", pwd, login,
                                     sysInfo.host_name, sysInfo.oofs_dir_rp,
                                     node_path);
    } else {
      vlabxutils::infoBox(this, "Could not copy the node.\n", "Warning");
    }
  } else {
    if (tree_copy_node(sysInfo.selNode)) {
      sysInfo.copyHyperobject = true;
      /* tell the other browser that paste data is present */
      sysInfo.vlabd->send_message(MPASTEREADY);
    }
  }

  update_menus();
  updateDisplay();
}

/******************************************************************************
 *
 * Slot for Export Object
 * 1) Open dialog to confirm export parameters
 * 2) Retrieve object from RA
 * 3) package and/or convert the format
 * 4) move the package to the destination
 * 5) clean up
 *
 */
void QTbrowser::exportObject() {

  // if no node is selected, return
  if (sysInfo.selNode == NULL)
    return;
  // 1) Open dialog to confirm export parameters
  Export *window = new Export(this, sysInfo.selNode,
                              QString(sysInfo.selNode->baseName), exportPath);
  QString savePath;
  // window->setTab(2);
  window->setAttribute(Qt::WA_DeleteOnClose,
                       false); // make sure we can get our information after the
                               // dialog is finished interacting with the user

  BrowserSettings bset; // let's get some browser-wide settings!!
  window->setRecursive(bset.recentExportRecursive());
  window->setFormat(bset.recentExportFormat()); // mac or windows?
  // we don't want to keep the same path ...
  //    window->setPaths(bset.recentExportPath());
  window->setType(bset.recentExportType()); // archive type?
  int result = window->exec();
  if (result == QDialog::Accepted) { // only proceed if the user did not press
                                     // cancel, obviously
    exportPath = window->getPath();
    exportArchiveType = window->getType();
    bset.setRecentExportFormat(
        window->getFormat()); // cache the last settings for the next use
    bset.setRecentExportRecursive(window->getRecursive());
    bset.setRecentExportType(window->getType());
  } else {
    return;
  }
  // 2->5) are handled within the window
  window->setAttribute(Qt::WA_DeleteOnClose, true);
  window->close();
}

/******************************************************************************
 *
 * Slot for Import Object
 * 1) Open dialog to confirm import parameters
 * 2) unpackage and/or convert the format
 * 3) archive this object
 * 4) transmit object to RA then unarchive
 * 5) prototype to find symlinks if asked
 * 6) clean up
 *
 */
void QTbrowser::importObject() {

  // if no node is selected, return
  if (sysInfo.selNode == NULL)
    return;

  // ImportExport* window = new ImportExport(this);
  Import *window = new Import(this, QString(sysInfo.selNode->name));
  window->setAttribute(Qt::WA_DeleteOnClose,
                       false); // make sure we can get our information after the
                               // dialog is finished interacting with the user
  BrowserSettings bset;        // let's get some browser-wide settings!!
  window->setFormat(bset.recentExportFormat());
  window->setType(bset.recentExportType());
  int result = window->exec();
  if (result == QDialog::Accepted) { // only proceed if the user did not press
                                     // cancel, obviously
    bset.setRecentExportFormat(
        window->getFormat()); // cache the last settings for the next use
    bset.setRecentExportType(window->getType());
  }
  window->setAttribute(Qt::WA_DeleteOnClose, true);
  window->close();
  char message[4096];
  QByteArray userNameData = sysInfo.login_name.toLatin1();
  const char *userNameChar = userNameData.constData();

  sprintf(message, "%s@%s:%s", userNameChar, sysInfo.host_name,
          sysInfo.selNode->name);

  if (sysInfo.connection->reconnect()) {
    vlabxutils::infoBox(sysInfo.mainForm,
                        "Can't export\n"
                        "Connection with raserver is down\n"
                        "Check your network connection\n",
                        "Error");
    return;
  }

  sysInfo.vlabd->send_message(UPDATE, message);

  sysInfo.connection->Disconnect();

  update_menus();
  updateDisplay();
}

/******************************************************************************
 *
 * callbacks for the copy subtree
 *
 */
void QTbrowser::copy_subtree_cb() {
  std::string pwdString = sysInfo.password.toStdString();
  const char *pwd = pwdString.c_str();
  std::string loginString = sysInfo.login_name.toStdString();
  const char *login = loginString.c_str();

  // if no node is selected, do not do anything
  if (sysInfo.selNode == NULL)
    return;
  if (!sysInfo.connection->check_connection()) {
    vlabxutils::infoBox(sysInfo.mainForm,
                        "Can't copy subtree\n"
                        "Connection with raserver is down\n"
                        "Check your network connection\n",
                        "Error");
    return;
  }

  // if the selected object is not a hyperobject
  if (sysInfo.selNode->isHObj == 0) {
    // display a message
    vlabxutils::tempBoxPopUp(topX(), topY(), this, "Copying subtree...");

    // copy the subtree
    int res = tree_copy(sysInfo.selNode);

    // pop down the temporary box
    vlabxutils::tempBoxPopDown(this);

    // check if the operation was successful
    if (res) {
      sysInfo.copyHyperobject = false;
      // tell the other browsers that paste data is present
      char node_path[4096];
      node_get_relative_path(sysInfo.selNode, node_path);
      sysInfo.vlabd->va_send_message(PASTEREADY, "%s %s@%s:%s %s", pwd, login,
                                     sysInfo.host_name, sysInfo.oofs_dir_rp,
                                     node_path);
    } else {
      std::string errMsg = "Copy subtree failed:\n" + sysInfo.getErrorLog();
      vlabxutils::infoBox(this, errMsg.c_str(), "Warning");
    }
  } else {
    if (tree_copy(sysInfo.selNode)) {
      sysInfo.copyHyperobject = true;
      sysInfo.vlabd->send_message(MPASTEREADY);
    }
  }

  update_menus();
  updateDisplay();
}

/******************************************************************************
 *
 * callbacks for paste
 *
 */
void QTbrowser::paste_cb() {
  if (sysInfo.selNode == NULL)
    return;

  if (sysInfo.pasteLinkReady) {
    hyperpaste_cb();
    return;
  }

  if (!sysInfo.pasteReady) {
    return;
  }

  if (!sysInfo.connection->check_connection()) {
    vlabxutils::infoBox(sysInfo.mainForm,
                        "Can't paste object\n"
                        "Connection with raserver is down\n"
                        "Check your network connection\n",
                        "Error");
    return;
  }

  if ((sysInfo.selNode->isHObj == 0) || (sysInfo.selNode->isHObj == 1)) {
    // If move_links is selected, the source of the data to be
    // pasted has to be the same as the destination, otherwise
    // this operation does not make sense.

    // [PASCAL] bug list from 2016, Jan 25th
    // this option should
    // not have any effect when moving objects between different oofs (since
    // h-links are never moved then). so we just disable temporarily the
    // move_links option.

    bool moveLinks = sysInfo.move_links;

    if (sysInfo.move_links) {

      if (strcmp(sysInfo.paste_info->host_name,
                 sysInfo.connection->host_name)) {
        moveLinks = false;
      }

      if (strcmp(sysInfo.paste_info->prefix, sysInfo.oofs_dir_rp)) {
        moveLinks = false;
      }
    }

    // prepare the name of the archive
    char archive_name[4096];
    sprintf(archive_name, "%s/data.ar", sysInfo.paste_dir);

    // prepare a full path of the source object
    char src_path[4096];
    fprintf(stderr, "sysInfo.paste_info->prefix: %s\n",
            sysInfo.paste_info->prefix);
    strcpy(src_path, sysInfo.paste_info->prefix);

    char *ptr = sysInfo.paste_info->node_path;

    while (1) {
      if (*ptr == '\0')
        break;
      if (*ptr == '/')
        break;
      ptr++;
    }
    strcat(src_path, ptr);

    //***********
    // if there is a directory in the destination with the same name,
    // try to come up with an alternative name
    char new_base_name[4096];
    char base_name[4096];
    char newDest[4096];
    // find the name of the object from the file '---FILENAME---'
    char nameName[4096];
    FILE *fp;
    int i, c;
    sprintf(nameName, "%s/---FILENAME---", sysInfo.paste_dir);
    fp = fopen(nameName, "r");
    // this little loop is needed because we need to read in
    // also names with spaces
    //
    i = 0;
    while (1 == 1) {
      c = fgetc(fp);
      // it should never be EOF, because there
      // should always be a '\n' at the end...
      if (c == EOF) {
        char msg[4096];
        sprintf(msg, "Cannot perform the paste operation \n"
                     "Buffer is empty.\n");
        vlabxutils::infoBox(this, msg, "Error");
        return;
      }

      if (c == '\n') // exit when read the whole name
        break;
      base_name[i] = c;
      i++;
    }
    base_name[i] = '\0';
    fclose(fp);

    sprintf(new_base_name, "%s", base_name);
    int count = 0;
    while (true) {
      sprintf(newDest, "%s/ext/%s", sysInfo.selNode->name, new_base_name);
      if (RA::Access(sysInfo.connection, newDest, F_OK) != 0) {
        // [PASTE] : the object doesn't exist, finish
        // drag/drop
        break; // the object doesn't exist, finish drag/drop
      }
      // be creative, and generate a new name
      count++;
      if (count == 1000) { // ridiculous
        vlabxutils::popupInfoBox(
            sysInfo.mainForm, "Error",
            "Could not finish paste operation, because\n"
            "could not create destination dir on try %d.\n",
            count);

        return;
      }
      sprintf(new_base_name, "%s_%d%d%d", base_name, count / 100,
              (count / 10) % 10, (count % 10));
    }
    fp = fopen(nameName, "w");
    //
    fprintf(fp, "%s", new_base_name);
    fclose(fp);
    // ************/

    vlabxutils::tempBoxPopUp(0, 0, this, "Pasting", "Browser");
    ProgressReporter pr(&vlabxutils::setProgress, 0, 1);
    RA::setProgressReporter(&pr);
    int res = RA::Paste_object(sysInfo.connection // RA connection
                               ,
                               sysInfo.oofs_dir_rp // database location
                               ,
                               sysInfo.selNode->name // destination
                               ,
                               archive_name // name of the local file where the
                                            // archive is stored
                               ,
                               src_path // the old name of the object
                               ,
                               moveLinks // whether the hyperlinks
                               // should follow the newly
                               // created object(s)
    );
    RA::setProgressReporter(NULL);
    vlabxutils::tempBoxPopDown(this);

    if (res) {
      char reason[4096];
      switch (res) {
      case -1:
        sprintf(reason, "Write access denied.");
        break;
      case -2:
        sprintf(reason, "Object already exists.");
        break;
      case -4:
        sprintf(reason, "Cannot open local archive.");
        break;
      case -5:
        sprintf(reason, "RAserver problems (resources?).");
        break;
      case -6:
        sprintf(reason, "Internal bug. Please contact developers.");
        break;
      case -7:
        sprintf(reason, ".dbase could not be properly modified.");
        break;
      default:
        sprintf(reason, "Unknown error.");
      }

      char err_msg[4096];
      sprintf(err_msg,
              "Could not perform the paste operation because:\n"
              "\n"
              "         %s\n"
              "\n"
              "because:\n"
              "\n"
              "         %s\n",
              RA::err_to_str(RA::error_code), reason);
      vlabxutils::infoBox(this, err_msg, "Warning");

      return;
    }

    // operation successful
    debug_printf("Paste_object() successful\n");
    if (sysInfo.connection->reconnect()) {
      vlabxutils::infoBox(sysInfo.mainForm,
                          "Can't paste object\n"
                          "Connection with raserver is down\n"
                          "Check your network connection\n",
                          "Error");
      return;
    }

    // send a message to vlabd about an object having been updated
    QByteArray userNameData = sysInfo.login_name.toLatin1();
    const char *userNameChar = userNameData.constData();

    sysInfo.vlabd->va_send_message(UPDATE, "%s@%s:%s", userNameChar,
                                   sysInfo.host_name, sysInfo.selNode->name);

    if (!sysInfo.copyHyperobject)
      sysInfo.vlabd->va_send_message(UUIDTABLECHANGED, "%s@%s:%s", userNameChar,
                                     sysInfo.host_name, sysInfo.oofs_dir);
    debug_printf("Disconnecting Paste\n");

    sysInfo.connection->Disconnect();

  } else if (sysInfo.selNode->isHObj == 1) {
    // the object where we are pasting is a hyper link
    if (!sysInfo.copyHyperobject) {
      vlabxutils::infoBox(
          sysInfo.mainForm,
          "Sorry, can not paste object under a hyperobject node.", "Error");
      return;
    }
    if (!node_operation_allowed(sysInfo.selNode, OP_PASTE)) {
      vlabxutils::infoBox(
          sysInfo.mainForm,
          "You do not have permissions to 'paste' to this object!", "Warning");
      return;
    }

    char message[4096];
    QByteArray userNameData = sysInfo.login_name.toLatin1();
    const char *userNameChar = userNameData.constData();

    sprintf(message, "%s@%s:%s", userNameChar, sysInfo.host_name,
            sysInfo.selNode->name);
  }
  sysInfo.pasteLinkReady = false;
  sysInfo.pasteReady = false;
  // update the visual appearance of browser
  update_menus();
  debug_printf("Menu updated\n");

  updateDisplay();
}

/******************************************************************************
 *
 * callbacks for delete
 *
 */
void QTbrowser::delete_cb() {
  // if no node is selected, return
  if (sysInfo.selNode == NULL)
    return;

  // top level object cannot be removed
  if (sysInfo.selNode->parent == NULL) {
    vlabxutils::infoBox(
        this, "You cannot delete a root. It is too dangerous!!!", "Warning");
    return;
  }

  // check if the user has a proper permission to remove the selected node
  if (!node_operation_allowed(sysInfo.selNode, OP_DELETE)) {
    vlabxutils::infoBox(
        this, "You do not have permissions to delete this object!", "Warning");
    return;
  }

  if (!sysInfo.connection->check_connection()) {
    vlabxutils::infoBox(sysInfo.mainForm,
                        "Can't delete object\n"
                        "Connection with raserver is down\n"
                        "Check your network connection\n",
                        "Error");
    return;
  }

  // ask user to confirm
  std::string msg = "<qt>Do you really want to delete this object?<br>"
                    "<font color=darkblue><center>";
  if (sysInfo.selNode->isHObj)
    msg += sysInfo.selNode->screenName;
  else
    msg += sysInfo.selNode->baseName;
  msg += "</font></center>";
  if (sysInfo.selNode->expandable)
    msg += "<br><font color=darkred>It has extensions.</font>";
  if (!vlabxutils::askYesNo(this, msg, "Please confirm")) {
    return;
  }

  // if the tree is being displayed from the node that is being deleted,
  // show the tree from one level up
  if (sysInfo.beginTree == sysInfo.selNode)
    sysInfo.beginTree = sysInfo.selNode->parent;

  if (sysInfo.selNode->isHObj == 0) {
    // show up a temporary dialog & suspend all other browsers
    vlabxutils::tempBoxPopUp(topX(), topY(), this, "Deleting...");
    sysInfo.vlabd->send_message(GETBUSY);
    // ask RAserver to delete the object
    int res = RA::Delete_object(sysInfo.connection, sysInfo.oofs_dir_rp,
                                sysInfo.selNode->name);
    // tell to all browsers that they shouldn't be busy
    if (sysInfo.connection->reconnect())
      return;
    sysInfo.vlabd->send_message(GETREADY);
    vlabxutils::tempBoxPopDown(this);

    // check result of the delete operation
    if (res) {
      vlabxutils::infoBox(this, "Delete unsuccessful.", "Error");
      sysInfo.vlabd->send_message(GETREADY);
      sysInfo.connection->Disconnect();
      return;
    }

    // inform everybody else that an object has been deleted
    QByteArray userNameData = sysInfo.login_name.toLatin1();
    const char *userNameChar = userNameData.constData();

    sysInfo.vlabd->va_send_message(DELETE, "%s@%s:%s", userNameChar,
                                   sysInfo.host_name, sysInfo.selNode->name);
    // unselect the node
    sysInfo.selNode = NULL;
    // send a message about '.dbase' being modified
    sysInfo.vlabd->va_send_message(UUIDTABLECHANGED, "%s@%s:%s", userNameChar,
                                   sysInfo.host_name, sysInfo.oofs_dir);
    sysInfo.connection->Disconnect();
  } else if (sysInfo.selNode->isHObj == 1) {
    QByteArray userNameData = sysInfo.login_name.toLatin1();
    const char *userNameChar = userNameData.constData();

    vlabxutils::tempBoxPopUp(topX(), topY(), this, "Deleting...");
    // make all browsers look busy
    sysInfo.vlabd->send_message(GETBUSY);

    char *message = dsprintf("%s@%s:%s", userNameChar, sysInfo.host_name,
                             sysInfo.selNode->name);
    if (sysInfo.beginTree == sysInfo.selNode)
      sysInfo.beginTree = sysInfo.selNode->parent;
    if (tree_delete(sysInfo.selNode))
      sysInfo.vlabd->send_message(HDELETE, message);
    // to all browsers: 'get ready'
    sysInfo.vlabd->send_message(GETREADY);
    free(message);
    vlabxutils::tempBoxPopDown(this);
    sysInfo.selNode = NULL;
    update_menus();
  }

  updateDisplay();
}

void QTbrowser::new_object_cb()
// ---------------------------------------------------------------------------
// create a new object as a child of the selected object
// ---------------------------------------------------------------------------
{
  if (sysInfo.selNode == NULL)
    return;

  if (sysInfo.selNode->isHObj) {
    vlabxutils::infoBox(this, "Cannot create a new object under hyper-object.",
                        "Error");
    return;
  }

  if (!sysInfo.connection->check_connection()) {
    vlabxutils::infoBox(sysInfo.mainForm,
                        "Can't create a new object\n"
                        "Connection with raserver is down\n"
                        "Check your network connection\n",
                        "Error");
    return;
  }

  // ask for the name of the new object
  QTStringDialog *dialog =
      new QTStringDialog("Enter object name:", "New Object", "", this);
  dialog->exec();
  char *newName = dialog->getResult();
  if (newName == NULL)
    return;

  // make sure the filename does not contain any spaces or other bad
  // characters - by replacing them with underscores
  underscore_string(newName);

  // make sure 'ext' directory exists (if it doesn't, create it)
  char ext_dir[PATH_MAX + 1];
  sprintf(ext_dir, "%s/ext", sysInfo.selNode->name);
  RA::Mkdir(sysInfo.connection, ext_dir);
  if (RA::Access(sysInfo.connection, ext_dir, F_OK) != 0) {
    vlabxutils::infoBox(this, "Cannot create a directory for extensions.",
                        "Error");
    return;
  }

  // construct a path to the new object in 'new_dir' and then create it
  char *new_dir = dsprintf("%s/%s", ext_dir, newName);
  if (RA::Mkdir(sysInfo.connection, new_dir)) {
    char *msg = dsprintf("Can't create the directory:\n  %s", new_dir);
    vlabxutils::infoBox(this, msg, "Error");
    xfree(msg);
    xfree(new_dir);
    return;
  }

  // create a dummy 'specifications' file
  char *specs_fname = dsprintf("%s/specifications", new_dir);
  char *buf = dsprintf(
      "description.txt\nignore:\n*\nDescription:\n\tEDIT description.txt\n");
  RA::Write_file(sysInfo.connection, specs_fname, buf, xstrlen(buf));
  xfree(buf);
  xfree(specs_fname);

  // create a dummy 'REMOVEME' file
  char *tmp_fname = dsprintf("%s/description.txt", new_dir);
  buf = dsprintf(
      "This is a dummy description file.\nThis file should be edited.\n");
  RA::Write_file(sysInfo.connection, specs_fname, buf, xstrlen(buf));
  xfree(buf);
  xfree(tmp_fname);

  // figure out where the relative path of the new object starts
  long pos = xstrlen(sysInfo.oofs_dir_rp) - 1;
  while (sysInfo.oofs_dir_rp[pos] != '/')
    pos--;

  // send a message to vlabd about an object having been updated
  QByteArray userNameData = sysInfo.login_name.toLatin1();
  const char *userNameChar = userNameData.constData();

  sysInfo.vlabd->va_send_message(UPDATE, "%s@%s:%s", userNameChar,
                                 sysInfo.host_name, sysInfo.selNode->name);

  sysInfo.vlabd->va_send_message(UUIDTABLECHANGED, "%s@%s:%s", userNameChar,
                                 sysInfo.host_name, sysInfo.oofs_dir);

  xfree(new_dir);

  // update the visual appearance of browser
  update_menus();
  updateDisplay();
}

void QTbrowser::new_Hobject_cb()
// ---------------------------------------------------------------------------
// create a new hyper-object as a child of the selected node
// ---------------------------------------------------------------------------
{
  if (sysInfo.selNode == NULL)
    return;

  if (!sysInfo.connection->check_connection()) {
    vlabxutils::infoBox(sysInfo.mainForm,
                        "Can't create a new hyperobject\n"
                        "Connection with raserver is down\n"
                        "Check your network connection\n",
                        "Error");
    return;
  }

  // ask for the name of the new object
  QTStringDialog *dialog =
      new QTStringDialog("Enter object name:", "New Object", "", this);
  dialog->exec();
  char *newName = dialog->getResult();
  if (newName == NULL)
    return;

  // make sure the filename does not contain any spaces or other bad
  // characters - by replacing them with underscores
  underscore_string(newName);

  // make sure 'ext' directory exists (if it doesn't, create it)
  char ext_dir[PATH_MAX + 1];
  sprintf(ext_dir, "%s/ext", sysInfo.selNode->name);
  RA::Mkdir(sysInfo.connection, ext_dir);
  if (RA::Access(sysInfo.connection, ext_dir, F_OK) != 0) {
    vlabxutils::infoBox(this, "Cannot create a directory for extensions.",
                        "Error");
    return;
  }

  // construct a path to the new object in 'new_dir' and then create it
  char *baseStr = genName(sysInfo.selNode);
  char new_dir[PATH_MAX + 1];
  sprintf(new_dir, "%s/ext/%s", sysInfo.selNode->name, baseStr);
  if (RA::Mkdir(sysInfo.connection, new_dir, 0755)) {
    char *msg = dsprintf("Can't create the directory:\n  %s", new_dir);
    vlabxutils::infoBox(this, msg, "Error");
    xfree(msg);
    xfree(baseStr);
    return;
  }

  // create a dummy node file
  NodeInfo nodeInfo(sysInfo.connection, std::string(new_dir) + "/node", QUuid(),
                    newName);
  nodeInfo.write();
  QByteArray userNameData = sysInfo.login_name.toLatin1();
  const char *userNameChar = userNameData.constData();

  sysInfo.vlabd->va_send_message(UPDATE, "%s@%s:%s", userNameChar,
                                 sysInfo.host_name, sysInfo.selNode->name);

  xfree(baseStr);

  // update the visual appearance of browser
  update_menus();
  updateDisplay();
}

// called by other components to update the display
void QTbrowser::updateDisplay() {
  int w = sysInfo.beginTree->getTreeRect().width();
  int h = sysInfo.beginTree->getTreeRect().height();
  int new_width = width();
  if (w > new_width)
    new_width = w;
  int new_height = height();
  if (h > new_height)
    new_height = h;

  globj->resize(QSize(new_width, new_height));
  globj->redrawBackBuffer();

  this->update();
}

// Window resize event
void QTbrowser::resizeEvent(QResizeEvent *ev) {
  winWidth = ev->size().width();
  winHeight = ev->size().height();
  updateDisplay();
}

// X position of top modal window
int QTbrowser::topX() { return x() + 100; }

// Y position of top modal window
int QTbrowser::topY() { return y() + 100; }

void QTbrowser::hypercopy_node_cb() {
  // if no node is selected, return
  if (sysInfo.selNode == NULL)
    return;

  if (sysInfo.selNode->isHObj == 0) {
    int res = tree_hypercopy_node(sysInfo.selNode);
    if (res)
      sysInfo.copyHyperobject = false;
    else
      vlabxutils::infoBox(this, "Could not hypercopy the node.\n", "Warning");
  } else if (sysInfo.selNode->isHObj == 1) {
    sysInfo.pasteLinkReady = true;
    sysInfo.pasteReady = false;
    sysInfo.paste_link_info.uuid = sysInfo.selNode->node_info.uuid();
    sysInfo.paste_link_info.dirNamex = sysInfo.selNode->baseName;
  }
  update_menus();
}

void QTbrowser::hyperpaste_cb() {
  if (sysInfo.selNode == NULL)
    return;

  tree_hyperpaste(sysInfo.selNode);

  // send a message to vlabd about an object having been updated
  if (sysInfo.connection->reconnect()) {
    vlabxutils::infoBox(sysInfo.mainForm,
                        "Can't paste hyperobject\n"
                        "Connection with raserver is down\n"
                        "Check your network connection\n",
                        "Error");
    return;
  }
  QByteArray userNameData = sysInfo.login_name.toLatin1();
  const char *userNameChar = userNameData.constData();

  sysInfo.vlabd->va_send_message(UPDATE, "%s@%s:%s", userNameChar,
                                 sysInfo.host_name, sysInfo.selNode->name);
  sysInfo.connection->Disconnect();

  // update the visual appearance of browser
  update_menus();
  updateDisplay();
}

void QTbrowser::hypercopy_subtree_cb() {
  // TODO: evil grin ;)
}

char *QTbrowser::genName(NODE *node) {
  int success = 0;
  int count = 0;
  char *base = NULL;
  char *tmp = NULL;

  while (!success) {
    base = dsprintf("%s_%d", node->baseName, count);
    tmp = dsprintf("%s/ext/%s", node->name, base);
    if (RA::Access(sysInfo.connection, tmp, F_OK) != 0)
      break;
    xfree(base);
    xfree(tmp);
    count++;
  }

  return base;
}

void QTbrowser::keyPressEvent(QKeyEvent *ev) {
  ev->accept();
  if ((ev->key() == Key_Up) && (ev->modifiers() & Qt::ControlModifier))
    globj->adjustChild(1);
  else if ((ev->key() == Key_Down) && (ev->modifiers() & Qt::ControlModifier))
    globj->adjustChild(0);
  else if (ev->key() == '/') {
    _finderWidget->activate();
  }
}

const BrowserSettings &QTbrowser::browserSettings() {
  return _customize_dialog->get();
}

void QTbrowser::applySettings(const BrowserSettings &bset) {
  // get info about the font
  QFontMetrics fm(bset.get(BrowserSettings::TextFont).value<QFont>());
  sysInfo.fontWidth = fm.width("W");
  sysInfo.fontHeight = fm.height();

  build_set_compacting(bset.get(BrowserSettings::Compacting).toBool());
  build_set_center_parent(bset.get(BrowserSettings::CenterParent).toBool());

  if (sysInfo.fontHeight < box_height)
    selectionHeight = box_height;
  else
    selectionHeight = sysInfo.fontHeight;

  int iconSize = bset.get(BrowserSettings::IconSize).toInt();
  if (lastIconSize != iconSize) {
    // update the sizes of all icons in the tree
    lastIconSize = iconSize;
    std::stack<NODE *> todo;
    todo.push(sysInfo.wholeTree);
    while (!todo.empty()) {
      NODE *node = todo.top();
      todo.pop();
      if (node->iconShow) {
        hideIcon(node);
        showIcon(node);
      }
      for (int i = 0; i < node->nChildren; i++)
        todo.push(node->child[i]);
    }
  }
  build_tree();
  update_menus();
  updateDisplay();
}

void QTbrowser::applySettingsColors(const BrowserSettings&) {
  updateDisplay();
}

const QPixmap &QTbrowser::defaultIcon(int size)
// returns a pixmap for a default icon
// if size != -1 (default), then an unscaled icon is returned
// otherwise the icon is scaled so that its max. size is size
{
  static QPixmap orig = QPixmap(":/default-icon.png");
  static QPixmap cached;
  static int cachedSize;
  // if size is negative, return unscaled icon
  if (size < 0)
    return orig;
  if (cached.isNull() || size != cachedSize) {
    QImage img = orig.toImage();
    cached = orig.scaled(size, size, Qt::KeepAspectRatio);
    cachedSize = size;
  }
  return cached;
}

const QPixmap &QTbrowser::hourglassIcon(int size)
// returns a pixmap for an hourglass
// if size != -1 (default), then an unscaled icon is returned
// otherwise the icon is scaled so that its max. size is size
{
  static QPixmap orig = QPixmap(":/hourglass.png");
  static QPixmap cached;
  static int cachedSize;
  // if size is negative, return unscaled icon
  if (size < 0)
    return orig;
  if (cached.isNull() || size != cachedSize) {
    QImage img = orig.toImage();
    cached = orig.scaled(size, size, Qt::KeepAspectRatio);
    cachedSize = size;
  }
  return cached;
}

bool QTbrowser::loadArbitraryIcon(const QRect &view, bool &inview)
// picks one of the icons and loads it
// if something was loaded, returns true, otherwise false
{
  int size = browserSettings().get(BrowserSettings::IconSize).toInt();
  // cache the center of the view
  QPoint viewCenter = view.center();

  // find an icon that needs to be loaded
  NODE *bestNode = NULL;
  int bestDist = -1;

  long numberOfIconsToLoad = 0;
  inview = false;
  std::stack<NODE *> todo;
  todo.push(sysInfo.beginTree);
  while (!todo.empty()) {
    NODE *node = todo.top();
    todo.pop();
    if (node->iconShow && node->isIconLoading()) {
      int dist = (viewCenter - node->getTopLeft()).manhattanLength();
      if (bestNode == NULL || bestDist > dist) {
        bestNode = node;
        bestDist = dist;
        inview = view.intersects(node->getNodeRect());
        // cannot have the code below if we want to display proper progress
        // if ( inview ) break; // no need to look for more
      }
      numberOfIconsToLoad++;
    }
    for (int i = 0; i < node->nChildren; i++)
      todo.push(node->child[i]);
  }
  if (bestNode != NULL) {
    loadIcon(bestNode, size);
    statusBar()->showMessage(
        QString("Loading icons: %1 remaining...").arg(numberOfIconsToLoad));
    if (lastIconLoaded.elapsed() > 1500) {
      updateDisplay();
      lastIconLoaded.start();
    }
    return true;
  } else {
    updateDisplay();
    statusBar()->showMessage(QString("Icons are now loaded."), 3000);
    return false;
  }
}

void QTbrowser::idleIconLoader()
// slot: tries to load one icon from the tree
// sets up another single shot timer to call itself if it
// thinks there might be more icons to be loaded
{
  // allow idle icon loader to be installed again
  _idleIconLoaderInstalled = false;

  QTime t;
  t.restart();

  // get the current view

  QRect view = globj->contentsRect();

  bool schedule_again = true;
  bool update_display = false;
  while (1) {
    bool inview;
    if (loadArbitraryIcon(view, inview)) {
      update_display |= inview;
    } else {
      schedule_again = false;
      break;
    }
    // if we have spent less than 0.1 seconds in this idle call, we might as
    // well try to load more icons...
    if (t.elapsed() > 100)
      break;
  }

  if (schedule_again) {
    scheduleIdleIconLoader(true, 1);
  } else {
    scheduleIdleIconLoader(false);
  }
  if (update_display)
    updateDisplay();
}

void QTbrowser::scheduleIdleIconLoader(bool yes, int delay)
// schedule an event to process reading icons but make sure only one at a time
//
{
  if (yes) {
    if (_idleIconLoaderInstalled) {
      return;
    } else {
      QTimer::singleShot(delay, this, SLOT(idleIconLoader()));
      _idleIconLoaderInstalled = true;
      return;
    }
  } else {
    _idleIconLoaderInstalled = false;
    return;
  }
}

void QTbrowser::dragEnterEvent(QDragEnterEvent *ev) {
  if (ev->mimeData()->hasUrls())
    ev->acceptProposedAction();
}

void QTbrowser::dropEvent(QDropEvent *ev) {
  if (ev->mimeData()->hasUrls()) {
    QUrl url = ev->mimeData()->urls()[0];
    interpretFile(url.path());
  }
  ev->acceptProposedAction();
}

void QTbrowser::showEvent(QShowEvent *ev) { QMainWindow::showEvent(ev); }

#ifdef USE_PLUGINS
void QTbrowser::pluginsChanged() {
  Vlab::UpdateBinLog log = Vlab::updateBin();
}
#else
void QTbrowser::pluginsChanged() {
  // 2012.03.19 PBdR: Do nothing, this is the only way to have the signals
  // working correctly as moc will ignore preprocessing directives
}
#endif
