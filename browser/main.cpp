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



#include <iostream>
#include <QApplication>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cassert>
#include <errno.h>
#include <signal.h>
#include <sstream>
#include <string.h>
#include <unistd.h>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QTextStream>
#include <QTimer>

#include "QTask_login.h"
#include "QTbrowser.h"
#include "dsprintf.h"
#include "graphics.h"
#include "main.h"
#include "newbrowser.h"
#include "parse_object_location.h"
#include "sgiFormat.h"
#include "utilities.h"
#include "xstring.h"
#include "xutils.h"

#include <QtPlugin>

#include "platform.h"
#include "qtFontUtils.h"

#ifdef __APPLE__
#include <errno.h>
#include <sys/sysctl.h>

#include "environment.h"
#ifdef USE_PLUGINS
#include <CoreServices/CoreServices.h>
#include <sys/wait.h>
#endif // USE_PLUGINS
#include "browserapp.h"
#endif

static const int DEBUG = 0;

// ********* prototypes **************

void usage(void);

// ********* global variables ********

SystemInfo sysInfo;

int start_browser(int argc, char **argv, bool cleanTMP = false);
int start_newbrowser_dlg();

/******************************************************************************
 *
 * this main function initializes the browser
 *
 *    - reads in the customization file
 *    - initializes the buttons
 *    - initializes the user interface
 *    - initializes the vlab daemon
 *
 */

#include "fixoofs.h"

int main(int argc, char **argv) {
  if (0) {
    std::string log = fixOofs(argv[1], true);
    return 0;
  }

#ifdef __APPLE__
  BrowserApp app(argc, argv);
#else
  QApplication app(argc, argv);
#endif
  setDefaultFont();
  // make sure no zombies are created when forked() processes exit
  signal(SIGCHLD, SIG_IGN);

  Vlab::setupVlabEnvironment();
  // update the bin directory

  Vlab::UpdateBinLog log = Vlab::updateBin();

#ifdef __APPLE__

  environment(argc, argv);

#ifdef USE_PLUGINS
  sysInfo.storePath = xstrdup(getenv("PATH"));

  sysInfo.pluginDir = xstrdup(getenv("VLABPLUGIN"));
  sysInfo.systemDir = xstrdup(getenv("VLABSYSTEM"));

#endif
#endif

  app.setApplicationName("Browser");
  app.setOrganizationDomain("algorithmicbotany.org");
  app.setOrganizationName("Algorithmic Botany");

  // copy the command line arguments to a safe place
  // --------------------------------------------------
  sysInfo.argc = argc;
  sysInfo.argv = (char **)xmalloc(sizeof(char *) * (argc + 1));
  int i;
  for (i = 0; i < argc; i++) {
    sysInfo.argv[i] = new char[1024];
    if (argv[i]) {
      strcpy(sysInfo.argv[i], argv[i]);
    } else {
      sysInfo.argv[i][0] = 0;
    }
  }
  sysInfo.argv[argc] = NULL;
  // 'store' the current working directory (because it
  // might change later)
  // --------------------------------------------------
  sysInfo.origCWD = getcwd(NULL, 4096);
  if (sysInfo.origCWD == NULL) {
    perror("getcwd()");
    sysInfo.origCWD = strdup("/");
  }

  // Count number of arguments
  int nb_args = 0;
  for (int i = 0; i < argc; ++i) {
    if (argv[i])
      ++nb_args;
  }

  if (nb_args > 1) {
    return start_browser(argc, argv, false);
  } else {

  display_splash();

    NewBrowserDlg dlg;
    dlg.setFocusPolicy(Qt::StrongFocus);
    dlg.setFocus();
    dlg.show();

    QTimer::singleShot(0, &dlg, SLOT(raise()));
    int answer = dlg.exec();
    if (answer == QDialog::Accepted) {
      QByteArray oofs = dlg.oofs().toLatin1();
      QList<QByteArray> ba_args = oofs.split(' ');
      char *args[5];
      int i = 0;
      args[i++] = argv[0];
      if (ba_args[0] == "-p") {
        args[i++] = ba_args[0].data();
        args[i++] = ba_args[1].data();
        args[i++] = ba_args[2].data();
      } else {
        args[i++] = ba_args[0].data();
      }
      args[i] = 0;

      char **copyargv = (char **)xmalloc(sizeof(char *) * 5);
      for (int j = 0; args[j]; ++j) {
        copyargv[j] = xstrdup(args[j]);
      }
      copyargv[i] = 0;
      if (DEBUG)
        fprintf(stderr, "Opening browser on oofs: %s\n", copyargv[1]);

      start_browser(i, copyargv, true);
    }
  }
}

int start_browser(int argc, char **argv, bool cleanTMP) {
  // figure out the temporary directory for vlab
  // --------------------------------------------------
  sysInfo.tmpDir = xstrdup(getenv("VLABTMPDIR"));
  if (sysInfo.tmpDir == NULL) {
    QString tmpPath = QString(QDir::homePath() + QString("/.vlab/tmp/"));
    sysInfo.tmpDir = xstrdup(tmpPath.toLatin1());
  }
  // if temp dir does not exist, create it.
  QDir dir(sysInfo.tmpDir);
  if (!dir.exists()) {
    dir.mkpath(sysInfo.tmpDir);
  } else {
    // temp dir exists; if it should be cleaned but isn't empty, ask user what to do
    if (cleanTMP && !dir.isEmpty()) {
      QMessageBox msgBox;
      msgBox.setIcon(QMessageBox::Warning);
      msgBox.setText("An existing temporary folder for vlab was detected. Would you like to save or discard it?");
      msgBox.setInformativeText("Discarding the folder will remove all files within open objects. You will lose any unsaved work!");
      // write out the names of the objects contained in the temp folder
      QString name_filter("VL*"); // all objects start with 'VL', see MakeTemp() in object.cpp
      QString folder_names = dir.entryList(QStringList(name_filter), QDir::Dirs | QDir::NoDotAndDotDot).join("\n");
      QString folder_str = "Object folders to be deleted:\n" + folder_names;
      msgBox.setDetailedText(folder_str);
      msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
      msgBox.setDefaultButton(QMessageBox::Save);
      int ret = msgBox.exec();
      if (ret == QMessageBox::Cancel) {
        // exit and don't do anything
        exit(-1);
      } else if (ret == QMessageBox::Discard) {
        QDir dir(sysInfo.tmpDir);
        bool a = dir.removeRecursively();
        if (!dir.exists()) {
          a = dir.mkpath(sysInfo.tmpDir);
        }
      } // else ... keep the temp folder
    }
  }

  // figure out the directory where we'll store data
  // for paste
  // --------------------------------------------------
  sprintf(sysInfo.paste_dir, "%s/cutCopyPaste%d", sysInfo.tmpDir,
          (int)getuid());
  sysInfo.paste_info = new PasteInfo;

  // remember our process id
  // --------------------------------------------------
  sysInfo.pid = getpid();

  // by default - forking is enabled
  // --------------------------------------------------
  sysInfo.noFork_ = false;

  // parse the location arguments
  // --------------------------------------------------
  sysInfo.host_name = xstrdup("UNDEFINED_HOSTNAME");
  sysInfo.oofs_dir = xstrdup("UNDEFINED_OBJNAME");
  sysInfo.database = NULL;
  assert(argc > 0);
  int i;
  for (i = 1; i < argc; i++) {
    if (argv[i] == 0)
      continue;
    // === password switch ===
    if (xstrcmp(argv[i], "-p") == 0) {
      i++;
      // password specification
      if (i == argc || !(sysInfo.password.isNull())) {
        // no more arguments after '-p', or
        // password has already been specified
        usage();
        exit(-1);
      }
      sysInfo.password = xstrdup(argv[i]);
      continue;
    }

    if (xstrcmp(argv[i], "-nofork") == 0) {
      sysInfo.noFork_ = true;
      continue;
    }

    // invalid switch
    if (argv[i][0] == '-') {
      usage();
      exit(-1);
    }

    // database specification
    if (sysInfo.database != NULL) {
      usage();
      exit(-1);
    }
    sysInfo.database = argv[i];
  }
  // set a default password if no password was specified
  if (sysInfo.password.isNull()) {
    sysInfo.password = xstrdup("");
  }

  // has database been specified?
  if (sysInfo.database == NULL) {
    // use the default database in $VLABROOT/oofs
#ifdef USE_PLUGINS
    sysInfo.database = dsprintf("%s/oofs", getenv("VLABRESOURCES"));
#else
    sysInfo.database = dsprintf("%s/oofs", getenv("VLABROOT"));
#endif
  }

  // parse the location of the object
  char *c_login_name = NULL;
  parse_object_location(sysInfo.database, &c_login_name, &sysInfo.host_name,
                        &sysInfo.oofs_dir);
  sysInfo.login_name = c_login_name;
  BrowserSettings qset;
  qset.setLastOofs(sysInfo.database, sysInfo.password);
  if (DEBUG)
    fprintf(stderr, "sysInfo.database = %s\n", sysInfo.database);

  // if the oofs directory location is not an absolute path, make
  // it an absolute path
  if (sysInfo.oofs_dir == NULL) {
    fprintf(stderr, "browser: oofs directory not specified.\n");
    exit(-1);
  }
  if (sysInfo.oofs_dir[0] != '/') {
    sysInfo.oofs_dir = dsprintf("%s/%s", sysInfo.origCWD, sysInfo.oofs_dir);
  }

  // the user gets the shell back (the parrent quits, the child runs)
  // Next Change by Pascal Ferraro otherwise does not work on Mac OS 10.5
  //     if (! sysInfo.noFork_)
  //       if( fork() != 0)
  // 	exit( 0);

  // Initialize main application widget
  // - make sure this goes after the fork(), otherwiser it might not work
  //   (it sure does not on Mac OS X - tiger)


  // initialize connection to VLAB daemon
  if (vlab_open()) {
    fprintf(stderr, "browser: Failed to activate a connection to VLAB "
                    "daemon.\n"
                    "         Please restart the program!\n");
    exit(-1);
  }

  // establish a connection to the database
  sysInfo.connection = RA::new_connection(
      sysInfo.host_name, sysInfo.login_name.toStdString().c_str(),
      sysInfo.password.toStdString().c_str());
  while (sysInfo.connection == NULL) {
    // what error occured?
    if (RA::error_code != RA_INVALID_LOGIN) {
      RA::Error("browser: new_connection()");
      exit(-1);
    }

    // it was just a login error - reconfirm login and password
    QTask_login ask_login(sysInfo.login_name.toStdString().c_str(),
                          sysInfo.password.toStdString().c_str());
    if (ask_login.exec() != QDialog::Accepted) {
      exit(1);
    }
    ask_login.getResponse(sysInfo.login_name, sysInfo.password);
    sysInfo.connection = RA::new_connection(
        sysInfo.host_name, sysInfo.login_name.toStdString().c_str(),
        sysInfo.password.toStdString().c_str());
  }

  // determine the realpath of the database root
  char *rp = NULL;
  if (RA::Realpath(sysInfo.connection, sysInfo.oofs_dir, rp)) {
    RA::Error("Cannot determine location of the database root");
    exit(-1);
  }

  if (DEBUG)
    fprintf(stderr, "database root realpath = '%s'\n", rp);

  // set the tree to be empty
  sysInfo.wholeTree = NULL;
  sysInfo.beginTree = NULL;
  // no node is selected
  sysInfo.selNode = NULL;
  // remove any trailing slashes from the filename
  remove_trailing_slashes(rp);
  // if a directory was specified and try to read it in
  strcpy(sysInfo.oofs_dir_rp, rp);
  // initialize the buttons on the screen
  initButtons(&sysInfo.buttons);

  // initialize the QT interface
  sysInfo.mainForm = new QTbrowser();
  sysInfo.mainForm->setObjectName("browser");

#ifdef __APPLE__
  sysInfo.mainForm->setWindowIcon(QPixmap()); // use qt3support
#endif

  sysInfo.mainForm->show();
  QTimer::singleShot(0, sysInfo.mainForm, SLOT(raise()));

  // ask all browsers to send you information about 'paste' availability

  sysInfo.vlabd->send_message(ISPASTEREADY);

  // ask all browsers to send you information about the status of the
  // move/leave h-links toggle
  sysInfo.move_links = true;
  sysInfo.vlabd->send_message(RESENDMOVELINKS);


  // make sure that a paste directory exists and we can write to it
  int paste_dir_error = 0;
  mkdir(sysInfo.paste_dir, 0755);
  struct stat st;

  if (stat(sysInfo.paste_dir, &st)) {
    paste_dir_error = 1;
  } else {
    if (!S_ISDIR(st.st_mode)) {
      paste_dir_error = 1;
    } else {
      if (access(sysInfo.paste_dir, W_OK))
        paste_dir_error = 1;
    }
  }

  if (paste_dir_error) {
    char tmp_msg[4096+256];
    sprintf(tmp_msg,
            "Having troubles creating temporary directory\n"
            "needed to perform cut/copy/paste and drag/drop\n"
            "operations.\n"
            "The name of the directory is:\n"
            "\n"
            "        %s\n"
            "\n"
            "Withouth this directory you will most likely not\n"
            "be able to perform these operations.\n",
            sysInfo.paste_dir);
    vlabxutils::infoBox(sysInfo.mainForm, tmp_msg, "Warning");
  }
  // Keeping connection in stand by
  sysInfo.connection->Disconnect_remain_open_connection();

  return qApp->exec();
}

int start_newbrowser_dlg() {
  NewBrowserDlg dlg;
  int answer = dlg.exec();
  if (answer == QDialog::Accepted) {
    QByteArray oofs = dlg.oofs().toLatin1();
    printf("New dlg box: Opening browser on oofs: '%s'\n", oofs.data());
    newBrowser(oofs, false);
  }
  return 0;
}

int vlab_open(void)
// ---------------------------------------------------------------------------
// - attempts to start a communication channel with VLABD. If vlabd is not
//   running, it will try to start it.
// - returns: 0 = success, otherwise FAILURE
// ---------------------------------------------------------------------------
{
  // connect to vlabd
  if (DEBUG)
    fprintf(stderr, "browser: connecting to vlabd.\n");
  sysInfo.vlabd = new VlabD();
  if (DEBUG)
    fprintf(stderr, "browser: initialize connection to vlabd\n");
  if (sysInfo.vlabd->init()) {
    fprintf(stderr, "browser: cannot contact vlabd: %s\n",
            sysInfo.vlabd->get_errstr());
    return -1;
  }
  if (DEBUG)
    fprintf(stderr, "browser: connected to vlabd.\n");

  // send in the registration string
  std::ostringstream buff;
  buff << (int)getpid() << ",browser," << UPDATE << "," << RENAME << ","
       << DELETE << "," << GETBUSY << "," << GETREADY << "," << PASTEREADY
       << "," << ISPASTEREADY << "," << POSITIONOBJ << "," << RESENDMOVELINKS
       << "," << MOVELINKS << "," << UUIDTABLECHANGED << "," << REFRESHICON;

  if (sysInfo.vlabd->send_message(REGISTER, buff.str().c_str())) {
    // could not register
    fprintf(stderr, "browser: could not register with vlabd: %s\n",
            sysInfo.vlabd->get_errstr());
    return -1;
  }
 
  // everything went ok - return success
  return 0;
}

void vlab_close(void)
// ---------------------------------------------------------------------------
// vlab_close() - Tell the vlab daemon that I am quitting.
// ---------------------------------------------------------------------------
{
  // close the RA connection
  // Tell the vlabd that I am quitting
  if (sysInfo.connection->reconnect())
    return;
  sysInfo.vlabd->send_message(REMOVE);
  sysInfo.connection->Disconnect();
}

void usage(void)
// ---------------------------------------------------------------------------
// print the usage of the browser
// ---------------------------------------------------------------------------
{
  fprintf(stderr, "Usage: browser [-p password] dirname [-nofork] \n");
}


void newBrowser(QByteArray oofs_spec, bool do_fork)
// open a new browser
{
  // I am sick of compiler warnings because do_fork is not used on apple...
  // Cheers, Pavol
#ifdef __APPLE__
  if (do_fork)
    do_fork = false;
#endif

  // Find out where to look for the browser executable, and put the
  // result into browser_bin. The executable must be specified by
  // BROWSERBIN.
  char *browser_bin = getenv("VLABBROWSERBIN");
  if (browser_bin == NULL) {
    vlabxutils::infoBox(0,
                        "Your $VLABBROWSERBIN is not set. Cannot run "
                        "browser.",
                        "Error");
    return;
  }

  // fork off a process
#ifndef __APPLE__
  if (do_fork) {
#endif
    pid_t pid = fork();
    if (pid == -1) {
      char msg[4096];
      sprintf(msg,
              "Could not execute an external process.\n"
              "   - %s\n",
              strerror(errno));
      vlabxutils::infoBox(0, msg, "Error");
      return;
    }

    // the parent returns
    if (pid != 0)
      return;
#ifndef __APPLE__
  }
#endif

  QList<QByteArray> ba_args = oofs_spec.split(' ');
  char *args[5];
  int i = 0;
  args[i++] = browser_bin;
  if (ba_args[0] == "-p") {
    args[i++] = ba_args[0].data();
    args[i++] = ba_args[1].data();
    args[i++] = ba_args[2].data();
  } else {
    args[i++] = ba_args[0].data();
  }
  args[i] = 0;

  if (DEBUG) {
    fprintf(stderr, "Executing: %s\nWith args:", browser_bin);
    for (int j = 0; args[j]; ++j) {
      fprintf(stderr, "'%s'\n", args[j]);
    }
  }

  // the child executes browser process
  execv(browser_bin, args);

  // if execution gets to this point, execlp() has failed. report error
  fprintf(stderr, "Could not exec '%s': %s\n", browser_bin, strerror(errno));

  qApp->quit();
}

void interpretFile(QString filename) {
  QTextStream out(stdout);
  QByteArray oofs;
  out << "Interpreting file: " << filename << endl;
  QFileInfo fi(filename);
  if (fi.isDir()) {
    oofs = filename.toLatin1();
  } else {
    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly)) {
      QMessageBox::critical(
          0, "Error while opening OOFS",
          QString("Cannot open OOFS description file '%1'").arg(filename));
    }
    oofs = f.readAll();
  }
  newBrowser(oofs);
}

// --------------------------------------------------------------------------
//
// display the splash screen
//
//

void display_splash(void) {
  // get the binary directory for vlab
  char *path = getenv("VLABBIN");
  char command[4096];
  if (path == NULL)
    sprintf(command, "vlab-splash 3000");
  else
    sprintf(command, "%s/vlab-splash 3000", path);

  // run the command
  int err = system(command);
  if (err && errno != ECHILD)
  // For some reason, we always get error ECHILD here under Linux
  // (possibly because the process keeps running after system() returns?)
  // Thus, we ignore it.
  {
    fprintf(stderr, "vlabd: when running vlab-splash I got error = %d\n", err);
    fprintf(stderr, "       executed command = %s\n", command);
    fprintf(stderr, "       errno = %d (%s)\n", errno, strerror(errno));
    return;
  }
}
