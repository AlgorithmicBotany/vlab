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



#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "RA.h"
#include "debug.h"
#include "dsprintf.h"
#include "xmemory.h"
#include "xstring.h"

// define the static variables of the class
RA_Error RA::error_code;
const char *RA::tmp_dir = "/var/tmp";
ProgressReporter *RA::progressReporter_ = NULL;

void RA::setProgressReporter(ProgressReporter *rep)
// ======================================================================
// use a new reporter
// ......................................................................
{
  progressReporter_ = rep;
}

void RA::setProgress(double val)
// ======================================================================
// This is used internally - to report on progress
// ......................................................................
{
  if (progressReporter_)
    progressReporter_->set(val);
}

/*****************************************************************************
 *
 * fopen
 *
 *      - open file for reading
 *      - copies the remote file to a local directory, and opens it
 *        there
 */

RA_File *RA::Fopen(RA_Connection *connection, const char *fname,
                   const char *type) {
  debug_printf("RA::Fopen( %s:%s).\n", connection->host_name, fname);
  assert(connection->connection_type != RA_NO_CONNECTION);

  RA_File *fp = new RA_File;

  // if the host we are connecting to is a local machine,
  // just open the file
  if (connection->connection_type == RA_LOCAL_CONNECTION) {
    // remember the file name
    fp->local_file_name = xstrdup(fname);
    // open the file for reading
    fp->local_file_stream = fopen(fname, type);
    // remember information about the host
    fp->connection = connection;
    // check for errors
    if (fp->local_file_stream == NULL) {
      delete fp;
      error_code = RA_FOPEN_FAILED;
      return NULL;
    }
    // return the file pointer
    return fp;
  }

  //
  // the file is on a remote server
  //

  // do we want to read, or write/modify the file?
  if (xstrcmp(type, "r") == 0) {
    // we only want to read the file

    fp->remote_file_name = xstrdup(fname);
    fp->connection = connection;

    // create a temporary file
    fp->local_file_name = dsprintf("%s/RA_TMP_FILEXXXXXX", tmp_dir);
    if (mkstemp(fp->local_file_name) < 0) {
      delete fp;
      error_code = RA_MKTEMP_FAILED;
      return NULL;
    }
    // copy the remote file into local space
    if (Fetch_file(connection, fname, fp->local_file_name)) {
      // error occured
      delete fp;
      error_code = RA_FETCH_FAILED;
      return NULL;
    }

    // fetch was successful, so just reopen the local file
    fclose(fp->local_file_stream);
    fp->local_file_stream = fopen(fp->local_file_name, type);
    if (fp->local_file_stream == NULL) {
      // error occured while reopening the local file
      delete fp;
      error_code = RA_FOPEN_FAILED;
      return NULL;
    }

    // return the file pointer
    return fp;
  }

  // we want to write a remote file

  debug_printf("RA::fopen(): Cannot write to a remote file yet.\n");

  delete fp;
  error_code = RA_OPERATION_NOT_SUPPORTED;
  return NULL;
}

FILE *RA::Fopen_read(RA_Connection *connection, const char *fname)
// ---------------------------------------------------------------------------
// - creates a local temporary file open for rw in binary mode, using
//   mkstemp()
// - downloads a remote file and writes it to the local file
// - returns result
// - when the file is fclose()-ed, it will be deleted automatically
// ---------------------------------------------------------------------------
{
  debug_printf("RA::Fopen_read( %s:%s).\n", connection->host_name, fname);
  assert(connection->connection_type != RA_NO_CONNECTION);

  // if the host we are connecting to is a local machine,
  // just open the file
  if (connection->connection_type == RA_LOCAL_CONNECTION) {
    FILE *fp = fopen(fname, "r");
    return fp;
    // check for errors
    if (fp == NULL) {
      error_code = RA_FOPEN_FAILED;
      return NULL;
    }
    // return the file pointer
    fseek(fp, 0, SEEK_SET);
    return fp;
  }

  //
  // the file is on a remote server
  //

  char *tmp_file = strdup("/tmp/chkdbaseXXXXXX");
  int res = mkstemp(tmp_file);
  if (res < 0) {
    perror("mkstemp():");
    error_code = RA_MKTEMP_FAILED;
    return NULL;
  }
  FILE *fp = fopen(tmp_file, "w+");
  if (fp == NULL) {
    error_code = RA_MKTEMP_FAILED;
    return NULL;
  }
  close(res);

  // make sure the file is erased when the program exists
  unlink(tmp_file);

  // copy the remote file into local space
  if (Fetch_file(connection, fname, fp)) {
    // error occured
    delete fp;
    error_code = RA_FETCH_FAILED;
    return NULL;
  }

  // return the file pointer
  fseek(fp, 0, SEEK_SET);
  return fp;
}

/******************************************************************************
 *
 * void error( char * str)
 *
 *   - print an error message to stderr, corresponding to error_code
 *
 */

void RA::Error(const char *err_msg) {
  fprintf(stderr, "%s: %s\n", err_msg, err_to_str());
}

/******************************************************************************
 *
 * int Fgetc( RA_File * fp)
 *
 *   - returns the next character from RA_File stream
 *
 */

int RA::Fgetc(RA_File *fp) {
  debug_printf("RA::Fgetc().\n");
  assert(fp->connection->connection_type != RA_NO_CONNECTION);
  assert(fp->local_file_stream != NULL);

  return fgetc(fp->local_file_stream);
}

/******************************************************************************
 *
 * int Feof( RA_File * fp)
 *
 *   - returns feof( local_file_stream)
 *
 */

int RA::Feof(RA_File *fp) {
  debug_printf("RA::Feof().\n");
  assert(fp->connection->connection_type != RA_NO_CONNECTION);
  assert(fp->local_file_stream != NULL);

  return feof(fp->local_file_stream);
}

/******************************************************************************
 *
 * int Fclose( RA_File * fp)
 *
 *   - closes file associated with fp for reading
 *   - if the file originated from a remote machine, it will be removed
 *
 */

int RA::Fclose(RA_File *fp) {
  debug_printf("RA::Fclose().\n");
  assert(fp->connection->connection_type != RA_NO_CONNECTION);
  assert(fp->local_file_stream != NULL);

  int result = fclose(fp->local_file_stream);
  fp->local_file_stream = NULL;

  if (result != 0) {
    delete fp;
    error_code = RA_FCLOSE_FAILED;
    return 1;
  }

  // if the file is on a remote host, try to remove it
  if (fp->connection->connection_type == RA_REMOTE_CONNECTION) {
    int result = unlink(fp->local_file_name);
    if (result != 0) {
      delete fp;
      error_code = RA_UNLINK_FAILED;
      return 1;
    }
  }

  delete fp;

  return 0;
}

/******************************************************************************
 *
 * RA_Connection * RA::new_connection( const char * host_name,
 *                                     const char * login_name,
 *                                     const char * password);
 *
 * - establishes a new connection to host_name with login_name and password
 *
 * - returns pointer to the new connection or NULL (if failure)
 *
 */

RA_Connection *RA::new_connection(const char *host_name, const char *login_name,
                                  const char *password) {
  debug_printf("RA::new_connection( %s, %s, %s)\n", host_name,
               (login_name != NULL) ? login_name : "<NULL>",
               (password != NULL) ? password : "<NULL>");

  RA_Connection *result = new RA_Connection();

  int err = result->Connect(host_name, RA_PORT, login_name, password);
  // connection is opened
  if (err != 0) {
    // could not establish connection */
    delete result;
    if (err == 1)
      error_code = RA_INVALID_HOST_NAME;
    else if (err == 2)
      error_code = RA_INVALID_LOGIN;
    else if (err == 3)
      error_code = RA_SOCKET_ERROR;
    else if (err == 4)
      error_code = RA_SERVER_ERROR;
    else if (err == 5)
      error_code = RA_SERVER_ERROR;
    return NULL;
  }
  result->set_keep_connection_open(true); // connection established
  return result;
}

/******************************************************************************
 *
 * void RA:: close_connection ( RA_Connection * connection)
 *
 * - will close a connection
 *
 */

void RA::close_connection(RA_Connection *connection) {
  debug_printf("RA::close_connection().\n");

  assert(connection->connection_type != RA_NO_CONNECTION);

  // connection is "in remain open" state.

  connection->Disconnect_remain_open_connection();
  delete connection;
}

const char *RA::err_to_str(int code)
/*-------------------------------------------------.
| return a string version of the error code 'code' |
`-------------------------------------------------*/
{
  switch (code) {
  case RA_FOPEN_FAILED:
    return "fopen() failed.";
  case RA_OPERATION_NOT_SUPPORTED:
    return "operation requested not supported (yet).";
  case RA_FETCH_FAILED:
    return "failed to retrieve a file from remote host.";
  case RA_MKTEMP_FAILED:
    return "failed to create a temporary file.";
  case RA_FCLOSE_FAILED:
    return "failed to close a local file.";
  case RA_UNLINK_FAILED:
    return "failed to remove a local file.";
  case RA_INVALID_HOST_NAME:
    return "invalid host name.";
  case RA_INVALID_LOGIN:
    return "invalid login.";
  case RA_DELTREE_FAILED:
    return "deltree error.";
  case RA_WRITE_FAILED:
    return "write(2) error.";
  case RA_READ_FAILED:
    return "read(2) error.";
  case RA_SOCKET_ERROR:
    return "pipe error.";
  case RA_LOCAL_STAT_FAILED:
    return "local stat() failed.";
  case RA_REALPATH_FAILED:
    return "realpath() failed.";
  case RA_READLINK_FAILED:
    return "readlink() failed.";
  case RA_SERVER_ERROR:
    return "no response from server (maybe down?).";
  case RA_OPEN_FAILED:
    return "could not open local file.";
  case RA_PASTE_FAILED:
    return "Paste failed.";
  default:
    return "Invalid error code for err_to_str().";
  }
}

const char *RA::err_to_str(void)
/*--------------------------------------------.
| return a string version of the latest error |
`--------------------------------------------*/
{
  return err_to_str(error_code);
}

std::string RA::errorLog_;

void RA::appendErrorLog(const std::string &entry) { errorLog_ += entry; }

std::string RA::getErrorLog() {
  std::string res = errorLog_;
  errorLog_ = "";
  return res;
}
