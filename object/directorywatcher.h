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




#ifndef DIRECTORYWATCHER_H
#define DIRECTORYWATCHER_H

#include <QDir>
#include <QObject>
#include <QStringList>
class QFileSystemWatcher;
class QString;

/* Emits changed() signal whenever a file is added, removed or modified
   within the given path. Note that DirectoryWatcher does NOT watch
   subdirectories.

   This is essentially the behaviour of the QFileWatcher::directoryChanged
   signal as documented. However, as per QTBUG-6524, the documentation is
   wrong. QFileSystemWatcher does not emit any signal when a file within
   the watched directory is modified.
*/
class DirectoryWatcher : public QObject {
Q_OBJECT
public:
  DirectoryWatcher(const QString& path,
                   const QStringList& ignorePrefixes,
                   const QStringList& ignoreSuffixes = QStringList(),
                   QObject* parent = 0);
  virtual ~DirectoryWatcher();

signals:
  void changed();
  void fileChanged(QString);


private:
  struct DirectoryChange {
    QStringList addedFiles;
    QStringList removedFiles;
  };

private slots:
  void handleDirectoryChange();
  DirectoryChange updateWatchedFiles();
  void displayChangedFiles(QString f);


private:
  QStringList filter(const QStringList& input) const;
  DirectoryChange poll() const;

private:
  QFileSystemWatcher* watcher_;
  QStringList ignorePrefixes_;
  QStringList ignoreSuffixes_;
  QDir dir_;
};

#endif /* DIRECTORYWATCHER_H */
