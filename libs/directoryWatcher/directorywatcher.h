#ifndef DIRECTORYWATCHER_H
#define DIRECTORYWATCHER_H

#include <QDir>
#include <QObject>
#include <QStringList>

#ifdef __APPLE__
#include "qvlabfilesystemwatcher.h"
#endif

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
   void handleFileChange(QString f);
  void displayChangedFiles(QString f);


private:
  QStringList filter(const QStringList& input) const;
  DirectoryChange poll() const;

private:

#ifdef __APPLE__
  QVlabFileSystemWatcher* watcher_;
#else
  QFileSystemWatcher* watcher_;
#endif

  QStringList ignorePrefixes_;
  QStringList ignoreSuffixes_;
  QDir dir_;
};

#endif /* DIRECTORYWATCHER_H */
