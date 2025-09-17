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



#ifndef __VLAB_VERSION_H__
#define __VLAB_VERSION_H__

#include "version.h"
#include <sstream>
#include <string>
#ifdef __APPLE__
#include <QCoreApplication>
#include <QDate>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <iostream>
#endif

namespace vlab {

int version_major() { return 5; }

int version_minor() { return 1; }

int version_minor_minor() { return 0; }

int build_number() {
  return 3614; // this build number is changed manually 
}

std::string build_date_string() {
  // Date of build is set manually
  return "August 12th, 2025";
}

std::string version_string() {
  std::ostringstream os;
  // os << version_major() << "." << version_minor() << "."
  //   << version_minor_minor();
  os << version_major() << "." << version_minor();
  return os.str();
}

std::string build_info() {
  std::ostringstream os;
  os << "build #" << build_number() << " " << build_date_string();
#ifdef __APPLE__
  QString path = QCoreApplication::applicationFilePath();
  QProcess proc;
  proc.start("file", QStringList() << path);
  proc.waitForFinished();
  QString result = QString::fromLocal8Bit(proc.readAllStandardOutput());
  QStringList lines = result.split("\n");
  bool has32 = false, has64 = false;
  for (int i = 0; i < lines.size(); ++i) {
    if (lines[i].contains("i386"))
      has32 = true;
    else if (lines[i].contains("x86_64"))
      has64 = true;
    else if (lines[i].contains("arm64"))
      has64 = true;
  }
  /*
  os << " for ";
  if (has32 && has64)
    os << "32/64 bit";
  else if (has32)
    os << "32 bit";
  else if (has64)
    os << "64 bit";
  else
    os << "unknown";
  os << " architecture";
  */
#endif
  return os.str();
}

} // namespace vlab

#endif
