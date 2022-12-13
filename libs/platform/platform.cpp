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
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace Vlab {
#ifdef ___PRETEND_TO_CLOSE_THIS_NAMESPACE_TO_FOOL_EMACS_INDENTATION___LOL___
}
#endif

void display_report_cerr(const Report &r) { std::cerr << r.get_log(); }

Report validate_environment(int, char **) {
  Report report;

  char *vlab_dir = getenv("VLABROOT");
  if (vlab_dir == NULL) {
    report.add_error("Environment variable VLABROOT not set.");
    return report;
  }

  // check for vlab config dir
  if (getenv("VLABCONFIGDIR") == NULL) {
    report.add_warning("VLABCONFIGDIR not set. Using $VLABDIR/config.");
    std::string config = vlab_dir;
    config += "/config";
    setenv("VLABCONFIGDIR", config.c_str(), 1);
  }

  if (getenv("VLABTMPDIR") == NULL) {
    report.add_warning("VLABTMPDIR not set. Using ~/.vlab/tmp");
    QString tmpPath = QString(QDir::homePath() + QString("/.vlab/tmp/"));
    setenv("VLABTMPDIR", tmpPath.toLatin1(), 1);
  }

  if (getenv("VLABBIN") == NULL) {
    report.add_warning("VLABBIN not set. Using $VLABDIR/bin.");
    std::string config = vlab_dir;
    config += "/bin";
    setenv("VLABBIN", config.c_str(), 1);
  }

  return report;
}

// returns the user's home directory
// - this is normally $HOME, but if it's not defined, we'll return root, i.e. /
static QString getUserHomeDir() {
  static QString cache;
  if (!cache.isNull())
    return cache;
  cache = QDir::homePath();
  return cache;
}

// returns the location where default config files are located
// - this is determined from the vlabroot path
static QString &getDefaultConfigDir() {
  static QString cache;
  if (!cache.isNull())
    return cache;
#ifdef VLAB_MACX
  cache = getVlabRoot() + "/Contents/Resources/config";
#elif defined(linux)
  cache = getVlabRoot() + "/config";
#else
  not_implemented();
#endif
  return cache;
}

// copies a file from src to dst if dst does not exist
// - if dst file alrady existed, return true
// - if file did not exit, attempty to copy it
//    then return 'true' of copy succeeded
//    otherwise return 'false'
static bool copyFileIfNotPresent(const QString &src, const QString &dst) {
  if (QFileInfo(dst).exists())
    return true;
  std::cerr << "platform: copying default config file\n"
            << "   from: " << src.toStdString() << "\n"
            << "     to: " << dst.toStdString() << "\n";
  return QFile::copy(src, dst);
}

const QString &getUserConfigDir(bool create) {
  static QString cache;
  if (!cache.isNull())
    return cache;

  if (0) {
    char *p = getenv("VLABCONFIGDIR");
    std::cerr << "vlab platform: getUserConfigDir called\n"
              << "   VLABCONFIGDIR = " << (p ? p : "<null>") << "\n";
  }

  QString homeDir = getUserHomeDir();
  cache = homeDir + "/.vlab";

  // check for presence of important files
  if (create) {
    static bool firstRun = true;
    if (firstRun) {
      firstRun = false;
      // attempt to make .vlab (we don't really care if it already existed)
      QDir(homeDir).mkdir(".vlab");
      // check for presence of files
      QString defaultConfigDir = getDefaultConfigDir();
      copyFileIfNotPresent(defaultConfigDir + "/object", cache + "/object");
#ifndef __APPLE__
      copyFileIfNotPresent(defaultConfigDir + "/lpfg.mak", cache + "/lpfg.mak");
#endif
    }
  }
  // set the environment variable VLABCONFIGDIR
  setenv("VLABCONFIGDIR", qPrintable(cache), 1);
  return cache;
}

// returns the path to the application executable
const QString &getApplicationPath() {
  static QString cache;
  // check cache first
  if (!cache.isNull())
    return cache;

  // determine the application file path
  QString appPath = QCoreApplication::applicationFilePath();

  // under both MacOSX (Leopard 10.5.8) and Linux (Scientific Linux
  // 5.3) the above call seems to return the absolute path of the
  // executable, with symbolic links already resolved. But the
  // documentation does not seem to guarantee this behavior, so
  // let's call canonical file path just in case :)
  appPath = QFileInfo(appPath).canonicalFilePath();

  // ok, now we should really have the right path. Cache it!
  cache = appPath;

  // done
  return cache;
}

// returns the root directory of the vlab installation
// - if VLABROOT environment is set, this is returned
// - otherwise it is derived from the directory where the executable is
//   by following it up, looking for the vlab root marker file
// - an exception is thrown if ROOT cannot be established
const QString &getVlabRoot() {
  //    static QString cache = "./";
  static QString cache;
  // check cache first
  if (!cache.isNull())
    return cache;
  // if VLABROOT environment is set, we are done
  char *env = getenv("VLABROOT");
  if (env) {
    cache = QString(env);
    return cache;
  }
  // determine the application file path
  QString appPath = getApplicationPath();

  // follow up the directory until the vlab root file marker is found
  QDir dir = QFileInfo(appPath).absoluteDir();
  while (1) {
    QString f = dir.path() + "/TopLevelMarker";
    if (QFileInfo(f).exists()) {
      cache = dir.path();
      break;
    }
    if (dir.cdUp())
      continue;
    std::cerr << "Cannot find VLAB's root :(\n";
    // throw "Cannot find VLAB's root";
    std::cerr << "returning :"
              << "./" << std::endl;
    cache = QString("./");
    return cache;
  }

  // done
  return cache;
}

// returns vlab tmp directory
const QString &getTmpDir() {
  static QString cache = QString(QDir::homePath() + QString("/.vlab/tmp/"));

  return cache;
}

static void _setenv(const std::string &env, const std::string &s,
                    bool overwrite = true) {
  setenv(env.c_str(), s.c_str(), overwrite ? 1 : 0);
}

void setupVlabEnvironment() {
  std::string vlabroot = getVlabRoot().toStdString();
  _setenv("VLABROOT", vlabroot);
  _setenv("VLABCONFIGDIR", getUserConfigDir().toStdString());
#ifdef VLAB_MACX
  _setenv("VLABSYSTEM", vlabroot + "/Contents/System");
  _setenv("VLABPLUGIN", vlabroot + "/Contents/Plug-ins");
  _setenv("VLABRESOURCES", vlabroot + "/Contents/Resources");
  _setenv("VLABBROWSERBIN", vlabroot + "/Contents/MacOS/browser");
#else
  _setenv("VLABSYSTEM", vlabroot + "/bin");
  _setenv("VLABPLUGIN", vlabroot + "/bin");
  _setenv("VLABRESOURCES", vlabroot + "/share");
  _setenv("VLABBROWSERBIN", vlabroot + "/bin/browser");
#endif
  _setenv("VLABDAEMONBIN", "vlabd");
  _setenv("VLABOBJECTBIN", "object");
  QString tmpPath = QString(QDir::homePath() + QString("/.vlab/tmp/"));
  _setenv("VLABTMPDIR", tmpPath.toStdString());

  _setenv("DISPLAY", ":0.0", false);
  std::string oldpath = getenv("PATH") ? getenv("PATH") : "";
#ifdef VLAB_MACX
  _setenv("PATH", vlabroot + "/Contents/MacOS/bin:" + vlabroot +
                      "/Contents/MacOS/dbin:" + oldpath);
#else
  _setenv("PATH", vlabroot + "/bin:" + oldpath);
#endif
}

// - determine where the dbin directory is
// - delete it's contents
// - get a list of all executables in .app bundles in Plug-ins and System
// directory
// - from this list make scripts in dbin

static const QString scriptString =
    //    "#!/bin/bash"
    "#!/usr/bin/env bash"
    "\n"
    "exe=\"%1\""
    "\n"
    "scriptSrc=\"$BASH_SOURCE\""
    "\n"
    "scriptDir=`dirname \"$scriptSrc\"`"
    "\n"
    "scriptDir=`(cd \"$scriptDir\" ; pwd -P)`"
    "\n"
    "fullexe=\"${scriptDir}/${exe}\""
    "\n"
    "exec \"$fullexe\" \"$@\""
    "\n";

UpdateBinLog updateBin() {
  UpdateBinLog log;
  QString vlabroot = getVlabRoot();
  QString contentsDir = vlabroot + "/Contents";
  QString vlabdbin = contentsDir + "/MacOS/dbin";
  // delete everything in dbin
  foreach (QFileInfo f,
           QDir(vlabdbin).entryInfoList(QDir::Files | QDir::System)) {
    // if this is not symbolic link, skip it
    if (!f.isSymLink())
      continue;
    std::cerr << "Deleting every symlink in dbin : "
              << f.fileName().toStdString() << std::endl;
    if (!QFile::remove(f.filePath())) {
      log.push_back("ERROR: failed to remove old binary script " +
                    f.fileName().toStdString());
    }
  }
  // compile a list of bundles
  std::vector<std::string> bundles;
  foreach (QFileInfo f, QDir(contentsDir + "/System")
                            .entryInfoList(QStringList("*.app"), QDir::Dirs)) {
    bundles.push_back("System/" + f.fileName().toStdString());
  }
  foreach (QFileInfo f, QDir(contentsDir + "/Plug-ins")
                            .entryInfoList(QStringList("*.app"), QDir::Dirs)) {
    bundles.push_back("Plug-ins/" + f.fileName().toStdString());
  }
  std::vector<std::string> execs;
  // add an executable for the browser
  execs.push_back("MacOS/browser");
  // add executables in the bundles
  for (size_t i = 0; i < bundles.size(); i++) {
    QDir dir(contentsDir + "/" + bundles[i].c_str() + "/Contents/MacOS");
    foreach (QFileInfo f, dir.entryInfoList(QDir::Executable | QDir::Files)) {
      execs.push_back(bundles[i] + "/Contents/MacOS/" +
                      f.fileName().toStdString());
    }
  }
  // add executables outside of bundles in System
  foreach (QFileInfo f,
           QDir(contentsDir + "/System").entryInfoList(QDir::Files)) {
    if (f.isExecutable())
      execs.push_back("System/" + f.fileName().toStdString());
  }
  // add executables outside of bundles in Plug-ins
  foreach (QFileInfo f,
           QDir(contentsDir + "/Plug-ins").entryInfoList(QDir::Files)) {
    if (f.isExecutable())
      execs.push_back("Plug-ins/" + f.fileName().toStdString());
  }

  // create scripts for the executables found
  for (size_t i = 0; i < execs.size(); i++) {
    QString prog = QFileInfo(execs[i].c_str()).fileName();
    // figure out src/dst
    QString src = QString("../../%1").arg(execs[i].c_str());
    QString dst = vlabdbin + "/" + prog;
    if (QFileInfo(dst).exists()) {
      log.push_back("ERROR: name conflict for :" + prog.toStdString());
      continue;
    }
    QFile script(dst);
    if (!script.open(QIODevice::WriteOnly)) {
      log.push_back("ERROR: could not create script for " + prog.toStdString());
      continue;
    }
    QTextStream out(&script);
    out << QString(scriptString).arg(src);
    // make script executable
    if (!script.setPermissions(script.permissions() | QFile::ExeUser |
                               QFile::ExeGroup | QFile::ExeOther)) {
      log.push_back("ERROR: failed to make script executable: " +
                    prog.toStdString());
    }
    script.close();
    log.push_back("INFO: made script for " + prog.toStdString());
  }
  return log;
}

} // namespace Vlab

