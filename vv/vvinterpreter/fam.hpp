#ifndef __FAM_HPP__
#define __FAM_HPP__

#ifdef FAM_THREAD

#include <QThread>
#include <QEvent>
#include <string>
#include <map>
#include <fam.h>

class QMutex;

struct RereadEvent : public QEvent
{
  RereadEvent( std::string fn ) : QEvent( static_cast<QEvent::Type>(QEvent::User+1) ), filename( fn ) {}
  virtual ~RereadEvent() {}

  // File that was modified: the path is always absolute !
  std::string filename;
};

class FamThread : public QThread
{
public:
  /**
   * \c parent is the QObject that will receive the RereadEvent
   */
  FamThread( QObject* parent );

  /**
   * Register a new file for monitoring.
   *
   * As the library is not declared thread-safe, the actual registration will 
   * take place in the FAM thread. That's why there is no return value. An 
   * error will be printed on stderr from the FAM thread.
   *
   * \warning the path has to be absolute
   */
  void registerFile( std::string filename );

  /**
   * Unregister a new file for monitoring
   *
   * As the library is not declared thread-safe, the actual unregistration will 
   * take place in the FAM thread. That's why there is no return value. An 
   * error will be printed on stderr from the FAM thread.
   *
   * \warning the path has to be absolute
   */
  void unregisterFile( std::string filename );


  /**
   * Ask the thread to stop: to be called when exiting the interpreter
   */
  void stop();

protected:
  void run();

  QMutex *mutex;
  std::map<std::string,FAMRequest> file_monitors;
  std::list<std::string> add_files;
  std::list<std::string> remove_files;
  FAMConnection connection;
  QObject *widget;

  /**
   * Prevent the thread to launch if the connection to FAM did not work
   */
  bool valid;

  /**
   * Tells if the user requested the thread to be stopped
   */
  bool stopped;
};

#endif

#endif // FAM_HPP

