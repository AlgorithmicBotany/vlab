#ifndef QVLABFILESYSTEMWATCHER_H
#define QVLABFILESYSTEMWATCHER_H
#include <QStringList>
#include <QSocketNotifier>

#include <QtCore/qobject.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>

#define NUM_EVENT_SLOTS 2
#define NUM_EVENT_FDS 2


class QVlabFileSystemWatcher : public QObject
{
    Q_OBJECT
 
public:
static QVlabFileSystemWatcher* create(const QStringList &paths,QObject *parent = NULL);
    QVlabFileSystemWatcher(const int kq,const QStringList &paths, QObject *parent = NULL);
    ~QVlabFileSystemWatcher();

    bool addPath(const QString &file);
    QStringList addPaths(const QStringList &files);
    bool removePath(const QString &file);
    QStringList removePaths(const QStringList &files);

    QStringList files() const;
    QStringList directories() const;

private slots:
    void readFromKQueue();

signals:
    void fileChanged(const QString &path);
    void directoryChanged(const QString &path);


 private:
    char *flagstring(int flags);

    QStringList _files;
    QStringList _directories;

struct kevent *_events_to_monitor;//[NUM_EVENT_FDS];
struct kevent *_event_data;//[NUM_EVENT_SLOTS];
    int _event_fd;
    int _kq;
    int _fileNb_to_monitor;




};




#endif // QVLABFILESYSTEMWATCHER_H
