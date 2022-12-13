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
    : QObject(parent), ignorePrefixes_(ignorePrefixes),
      ignoreSuffixes_(ignoreSuffixes), dir_(path) {

  QStringList curFiles = filter(dir_.entryList(QDir::Files, QDir::Name));
  QStringList paths;
  foreach (const QString &s, curFiles) {
    paths << path + "/" + s;
  }

#ifdef __APPLE__
  watcher_ = QVlabFileSystemWatcher::create(paths);
  connect(watcher_, SIGNAL(directoryChanged(QString)),
          SLOT(handleFileChange(QString)));
  connect(watcher_, SIGNAL(directoryChanged(QString)),
          SLOT(handleDirectoryChange()));
#else
  watcher_ = new QFileSystemWatcher(this);
  watcher_->addPaths(paths);
  //watcher_->addPath(path);
  //updateWatchedFiles();
#endif

  connect(watcher_, SIGNAL(fileChanged(QString)), this,
          SIGNAL(fileChanged(QString)));
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

void DirectoryWatcher::handleFileChange(QString f) {
  DirectoryChange c = updateWatchedFiles();
  if (!c.removedFiles.isEmpty() || !c.addedFiles.isEmpty()) {
    //std::cerr << "FILE CHANGED : " << f.toStdString() << std::endl;
    emit fileChanged(f);
  }
}

void DirectoryWatcher::displayChangedFiles(QString f) {
  std::cerr << "DIRECTORY WATCHER -> FILE CHANGED : " << f.toStdString()
            << std::endl;
}
