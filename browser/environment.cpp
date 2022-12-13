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



#include "platform.h"
#include <QIcon>
#include <cerrno>
#include <iostream>
#include <QApplication>
#include <QDir>
#include <QProcess>
#include <sys/param.h>
#include <unistd.h>

#ifdef USE_PLUGINS
#include <QString>
#include <QTextStream>

QTextStream err(stderr);


void changeDir(QDir &d, QString dirname) {
  if (!d.cd(dirname)) {
    err << "Error, cannot find subdirectory '" << dirname << "' in "
        << d.canonicalPath() << endl;
    abort();
  }
}
#endif

int environment(int , char **) {
  // [PASCAL] I am not sure we are using PLUGINS, in this case the following code should to be deleted
#ifndef USE_PLUGINS
  // fix up argv[0] so that it has absolute value
  if (argv[0][0] != '/') {
    char buff[MAXPATHLEN + 1];
    char *res = getcwd(buff, MAXPATHLEN);
    if (res != NULL) {
    } else {
      std::cerr << "browser: cannot getcwd(): " << strerror(errno) << "\n";
      return -1;
    }
    std::string s = std::string(buff) + "/" + argv[0];
    argv[0] = strdup(s.c_str());
  }
  //    QApplication app( argc, argv );
  QString aname = QFileInfo(argv[0]).fileName();

  // follow symbolic link to the actual executable
  QFileInfo wrapper_exec(qApp->applicationFilePath());
  while (wrapper_exec.isSymLink()) {
    QString lnk = wrapper_exec.readLink();
    if (lnk.isEmpty()) {
      std::cerr << "mac-wrapper: Cannot follow link.\n";
      exit(-1);
    }
    if (QFileInfo(lnk).isRelative())
      wrapper_exec = wrapper_exec.dirPath(true) + "/" + lnk;
    else
      wrapper_exec = lnk;
  }

  // determine the vlab directory
  QFileInfo vlabDir = wrapper_exec.dirPath(true) + "/..";

  // now get the appropriate application directory
  QString fap = vlabDir.absFilePath() + "/.binaries/" + aname +
                ".app/Contents/MacOS/" + aname;

  // set the vlab environment variables
  setenv("VLABROOT", vlabDir.absFilePath().ascii(), 1);
  setenv("VLABBIN", (vlabDir.absFilePath() + "/bin").ascii(), 0);
  setenv("VLABBROWSERBIN", (vlabDir.absFilePath() + "/browser").ascii(), 0);
  setenv("VLABDAEMONBIN", (vlabDir.absFilePath() + "/bin/vlabd").ascii(), 0);
  setenv("VLABOBJECTBIN", (vlabDir.absFilePath() + "/bin/object").ascii(), 0);
  setenv("VLABDOCDIR", (vlabDir.absFilePath() + "/docs").ascii(), 0);
  setenv("VLABTMPDIR", "~/.vlab/tmp", 0);
  setenv("LPFGPATH", vlabDir.absFilePath().ascii(), 0);
  setenv("DYLD_FRAMEWORK_PATH",
         vlabDir.absFilePath().ascii() + "../QtFrameworks/", 0);
  std::cerr << "DYLD_FRAMEWORK_PATH = "
            << vlabDir.absFilePath().ascii() + "../QtFrameworks/" << std::endl;

  char *path = getenv("PATH");
  if (path == NULL)
    setenv("PATH", (vlabDir.absFilePath() + "/bin").ascii(), 1);
  else
    setenv("PATH", (vlabDir.absFilePath() + "/bin:" + QString(path)).ascii(),
           1);

#else // USE_PLUGINS

  // determine the vlab directory
  QString vlabroot = Vlab::getVlabRoot();
  QDir vlabDir(vlabroot);
  //  std::cerr << "browser(environment.cpp): I think bundle is in:\n"
  //          << vlabDir.path().toStdString() << "\n";
  QDir resourcesDir(vlabDir);
  QDir systemDir(vlabDir);
  QDir pluginDir(vlabDir);

  changeDir(resourcesDir, "Contents");
  changeDir(resourcesDir, "Resources");

  changeDir(systemDir, "Contents");
  changeDir(systemDir, "System");

  changeDir(pluginDir, "Contents");
  changeDir(pluginDir, "Plug-ins");

  // set the vlab environment variables
  setenv("VLABROOT", vlabDir.absolutePath().toLatin1(), 1);
  setenv("VLABSYSTEM", systemDir.absolutePath().toLatin1(), 1);
  setenv("VLABPLUGIN", pluginDir.absolutePath().toLatin1(), 1);
  setenv("VLABRESOURCES", resourcesDir.absolutePath().toLatin1(), 1);
  setenv("VLABBROWSERBIN",
         vlabDir.absoluteFilePath("Contents/MacOS/browser").toLatin1(), 1);
  setenv("VLABDAEMONBIN", "vlabd", 1);
  setenv("VLABOBJECTBIN", "object", 1);
  QString tmpPath = QString(QDir::homePath() + QString("/.vlab/tmp/"));
  setenv("VLABTMPDIR", tmpPath.toLatin1(), 1);
  setenv("ENVRESOURCES", pluginDir.absolutePath().toLatin1(), 1);

#endif // USE_PLUGINS

  // set DISPLAY to :0.0, unless alread set to something
  setenv("DISPLAY", ":0.0", 0);

  return 0;
}
