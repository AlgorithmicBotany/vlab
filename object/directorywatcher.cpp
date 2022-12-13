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



#include "directorywatcher.h"
#include <QFileSystemWatcher>
#include <QString>
#include <algorithm>
#include <iostream>
#include <iterator>

namespace {
QStringList setDifference(const QStringList &first, const QStringList &second) {
  QStringList difference;
  std::set_difference(first.constBegin(), first.constEnd(), second.constBegin(),
                      second.constEnd(), std::back_inserter(difference));
  return difference;
}
} // namespace

DirectoryWatcher::DirectoryWatcher(const QString &path,
                                   const QStringList &ignorePrefixes,
                                   const QStringList &ignoreSuffixes,
                                   QObject *parent)
    : QObject(parent), watcher_(new QFileSystemWatcher(this)),
      ignorePrefixes_(ignorePrefixes), ignoreSuffixes_(ignoreSuffixes),
      dir_(path) {
  watcher_->addPath(path);
  updateWatchedFiles();
  connect(watcher_, SIGNAL(directoryChanged(QString)),
          SLOT(handleDirectoryChange()));
  connect(watcher_, SIGNAL(fileChanged(QString)), SIGNAL(fileChanged(QString)));
  connect(watcher_, SIGNAL(fileChanged(QString)),
          SLOT(displayChangedFiles(QString)));
}

DirectoryWatcher::~DirectoryWatcher() {}

QStringList DirectoryWatcher::filter(const QStringList &input) const {
  // nested loops are not elegant, but the ignore lists should be small
  QStringList l;
  foreach (const QString &s, input) {
    bool ok = true;
    foreach (const QString &f, ignorePrefixes_) {
      if (s.startsWith(f)) {
        ok = false;
        break;
      }
    }
    foreach (const QString &f, ignoreSuffixes_) {
      if (s.endsWith(f)) {
        ok = false;
        break;
      }
    }
    if (ok) {
      l << s;
    }
  }
  return l;
}

DirectoryWatcher::DirectoryChange DirectoryWatcher::poll() const {
  QStringList curFiles = filter(dir_.entryList(QDir::Files, QDir::Name));
  QStringList prevFiles = watcher_->files();
  qSort(prevFiles);

  QStringList addedFiles = setDifference(curFiles, prevFiles);
  QStringList removedFiles = setDifference(prevFiles, curFiles);
  DirectoryChange change = {addedFiles, removedFiles};
  return change;
}

DirectoryWatcher::DirectoryChange DirectoryWatcher::updateWatchedFiles() {
  DirectoryChange c = poll();
  if (!c.removedFiles.isEmpty()) {
    watcher_->removePaths(c.removedFiles);
  }
  if (!c.addedFiles.isEmpty()) {
    watcher_->addPaths(c.addedFiles);
  }
  return c;
}

void DirectoryWatcher::handleDirectoryChange() {
  DirectoryChange c = updateWatchedFiles();
  if (!c.removedFiles.isEmpty() || !c.addedFiles.isEmpty()) {
    emit changed();
  }
}

void DirectoryWatcher::displayChangedFiles(QString ) {
}
