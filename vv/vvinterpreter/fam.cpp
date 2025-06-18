#include "fam.hpp"
#ifdef FAM_THREAD

#include <stdio.h>
#include <iostream>
#include <qapplication.h>
#include <util/forall.hpp>
#include <QMutex>

using namespace std;

// Time to wait in the select
#define SELECT_WAIT 0 // seconds
#define SELECT_UWAIT 200000 // microseconds

FamThread::FamThread( QObject *parent )
  : QThread()
  , mutex( new QMutex() )
  , widget( parent )
  , valid( false )
  , stopped( false )
{
  if( FAMOpen2( &connection, "VVFamThread" ) )
    {
    perror( "Could not open connection to famd" );
    return;
    }
  valid = true;
}

void FamThread::registerFile( std::string filename )
{
  QMutexLocker locker( mutex );
  add_files.push_back( filename );
}

void FamThread::unregisterFile( std::string filename )
{
  QMutexLocker locker( mutex );
  remove_files.push_back( filename );
}

void FamThread::stop()
{
  stopped = true;
}

void FamThread::run()
{
  static int last_request = 1;
  // Allocation the event object for FAM
  FAMEvent event;
  if( !valid )
    {
    cerr << "Cannot run FAM thread: initialization went wrong" << endl;
    return;
    }
  int fam_fd;
  int retval;
  fd_set rdfs;
  struct timeval tv;
  // Get the file descriptor on which to wait for event with select
  fam_fd = FAMCONNECTION_GETFD( &connection );
  while( !stopped )
    {
    // Setup the select .. we are concerned only with the FAM fd
    FD_ZERO( &rdfs );
    FD_SET( fam_fd, &rdfs );
    tv.tv_sec = SELECT_WAIT;
    tv.tv_usec = SELECT_UWAIT;
    // Wait for a FAM event or for the timeout
    retval = select( fam_fd+1, &rdfs, NULL, NULL, &tv );
    if( retval < 0 )
      {
      perror( "Select failed: " );
      return;
      }
    else if( FD_ISSET( fam_fd, &rdfs ) )
      {
      // Something happened, unstack all the events at once and send them to 
      // the parent object
      while( FAMPending( &connection ) )
        {
        FAMNextEvent( &connection, &event );
        QApplication::postEvent( widget, new RereadEvent(event.filename) );
        }
      }
    // And now, (un)register the new files
    // The block is to ensure the mutex is correctly locked
      {
      QMutexLocker locker( mutex );
      forall( std::string filename, remove_files )
        {
        // Make sure the file is currently monitored
        if( file_monitors.find( filename ) != file_monitors.end() )
          {
          FAMRequest &request = file_monitors[ filename ];
          if( FAMCancelMonitor( &connection, &request ) )
            {
            cerr << "Could not cancel file monitoring of " << filename << endl;
            }
          else
            {
            file_monitors.erase( filename );
            }
          }
        }
      remove_files.clear();
      forall( std::string filename, add_files )
        {
        // Make sure the file is not already monitored
        if( file_monitors.find( filename ) == file_monitors.end() )
          {
          FAMRequest &request = file_monitors[ filename ];
          request.reqnum = last_request++;
          if( FAMMonitorFile2( &connection, filename.c_str(), &request ) )
            {
            cerr << "Could not monitor file " << filename << endl;
            }
          }
        }
      add_files.clear();
      }
    }
}

#endif
