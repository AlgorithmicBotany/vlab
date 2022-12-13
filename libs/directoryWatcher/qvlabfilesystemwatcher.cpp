#include <QTimer>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "qvlabfilesystemwatcher.h"

namespace {
// timer for idle function
QTimer *idleTimer;
} // namespace

QVlabFileSystemWatcher *QVlabFileSystemWatcher::create(const QStringList &paths,
                                                       QObject *parent) {
  /* Open a kernel queue. */
  int kq;
  if ((kq = kqueue()) < 0) {
    fprintf(stderr, "Could not open kernel queue.  Error was %s.\n",
            strerror(errno));
    return 0;
  }
  return new QVlabFileSystemWatcher(kq, paths, parent);
}

QVlabFileSystemWatcher::QVlabFileSystemWatcher(const int kq,
                                               const QStringList &paths,
                                               QObject *)
    : _kq(kq) {

  _fileNb_to_monitor = paths.size();
  _events_to_monitor = new struct kevent[_fileNb_to_monitor];
  _event_data = new struct kevent[_fileNb_to_monitor];
  addPaths(paths);

  // create and set up the timer for the IdleFunction
  idleTimer = new QTimer(this);
  connect(idleTimer, SIGNAL(timeout()), SLOT(readFromKQueue()));

  /* Open a kernel queue. */
  if ((_kq = kqueue()) < 0) {
    fprintf(stderr, "Could not open kernel queue.  Error was %s.\n",
            strerror(errno));
  }

  idleTimer->start(10);
}

QVlabFileSystemWatcher::~QVlabFileSystemWatcher() {
  idleTimer->stop();

  close(_event_fd);
}

bool QVlabFileSystemWatcher::addPath(const QString &path) {
  if (path.isEmpty()) {
    qWarning("QVlabFileSystemWatcher::addPath: path is empty");
    return true;
  }

  _files << path;
  return _files.isEmpty();
}

QStringList QVlabFileSystemWatcher::addPaths(const QStringList &paths) {
  int fileNb = 0;

 
  QStringList::const_iterator constIterator;
  for (constIterator = paths.constBegin(); constIterator != paths.constEnd();
       ++constIterator) {

    _files << *constIterator;
    void *user_data;
    unsigned int vnode_events;

    std::string path = constIterator->toStdString();
    char *cstr = new char[path.size() + 1];
    strcpy(cstr, path.c_str());

    /*
      Open a file descriptor for the file/directory that you
      want to monitor.
    */
    _event_fd = open(cstr, O_EVTONLY);
    if (_event_fd <= 0) {
      fprintf(
          stderr,
          "The file %s could not be opened for monitoring.  Error was %s.\n",
          path.c_str(), strerror(errno));
    }

    /*
      The address in user_data will be copied into a field in the
      event.  If you are monitoring multiple files, you could,
      for example, pass in different data structure for each file.
      For this example, the path string is used.
    */

    user_data = cstr;

    /* Set up a list of events to monitor. */
    vnode_events = NOTE_WRITE;
    EV_SET(&(_events_to_monitor[fileNb]), _event_fd, EVFILT_VNODE,
           EV_ADD | EV_CLEAR, vnode_events, 0, user_data);
    ++fileNb;
  }

  return _files;
}

bool QVlabFileSystemWatcher::removePath(const QString &path) {
  if (path.isEmpty()) {
    qWarning("QVlabFileSystemWatcher::removePath: path is empty");
    return true;
  }

  QStringList paths = removePaths(QStringList(path));
  return paths.isEmpty();
}

QStringList QVlabFileSystemWatcher::removePaths(const QStringList &paths) {
  QStringList p = paths;
  return p;
}

QStringList QVlabFileSystemWatcher::directories() const {
  QStringList p;
  return p;
}

QStringList QVlabFileSystemWatcher::files() const { return _files; }

/* A simple routine to return a string for a set of flags. */
char *QVlabFileSystemWatcher::flagstring(int flags) {
  static char ret[512];
  const char *myOr = "";

  ret[0] = '\0'; // clear the string.
  if (flags & NOTE_DELETE) {
    strcat(ret, myOr);
    strcat(ret, "NOTE_DELETE");
    myOr = "|";
  }
  if (flags & NOTE_WRITE) {
    strcat(ret, myOr);
    strcat(ret, "NOTE_WRITE");
    myOr = "|";
  }
  if (flags & NOTE_EXTEND) {
    strcat(ret, myOr);
    strcat(ret, "NOTE_EXTEND");
    myOr = "|";
  }
  if (flags & NOTE_ATTRIB) {
    strcat(ret, myOr);
    strcat(ret, "NOTE_ATTRIB");
    myOr = "|";
  }
  if (flags & NOTE_LINK) {
    strcat(ret, myOr);
    strcat(ret, "NOTE_LINK");
    myOr = "|";
  }
  if (flags & NOTE_RENAME) {
    strcat(ret, myOr);
    strcat(ret, "NOTE_RENAME");
    myOr = "|";
  }
  if (flags & NOTE_REVOKE) {
    strcat(ret, myOr);
    strcat(ret, "NOTE_REVOKE");
    myOr = "|";
  }

  return ret;
}

void QVlabFileSystemWatcher::readFromKQueue() {
  if (!idleTimer->isActive()) {
    std::cerr << "idleTimer is active" << std::endl;
    return;
  }
  struct timespec timeout;

  /* Set the timeout to wake us every half second. */
  timeout.tv_sec = 0;  // 0 seconds
  timeout.tv_nsec = 0; // 500 milliseconds

  /* Handle events. */
  int num_files = _fileNb_to_monitor;
  int event_count = kevent(_kq, _events_to_monitor, num_files, _event_data,
                           num_files, &timeout);
  if ((event_count < 0) || (_event_data[0].flags == EV_ERROR)) {
    /* An error occurred. */
    fprintf(stderr, "An error occurred (event count %d).  The error was %s.\n",
            event_count, strerror(errno));
    idleTimer->stop();
    return;
  }
  if (event_count) {
    emit fileChanged(QString((char *)_event_data[0].udata));
  } else {
  }

  /* Reset the timeout.  In case of a signal interrruption, the
     values may change. */
  timeout.tv_sec = 0;  // 0 seconds
  timeout.tv_nsec = 0; // 500 milliseconds
}
