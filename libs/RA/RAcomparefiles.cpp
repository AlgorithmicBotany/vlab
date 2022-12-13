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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "RA.h"
#include "debug.h"
#include "dsprintf.h"
#include "utilities.h"
#include "xmemory.h"
#include "xstring.h"

/******************************************************************************
 *
 * int RA::Compare_files ( RA_Connection * connection1,
 *                         const char * fname1,
 *                         RA_Connection * connection2,
 *                         const char * fname2);
 *
 * - will compare two files (possibly on two different hosts)
 * - returns:  0 - files are the same
 *             1 - files differ
 *            <0 - error
 */

int RA::Compare_files(RA_Connection *connection1, const char *fname1,
                      RA_Connection *connection2, const char *fname2) {
  // pre-conditions
  assert(connection1 != NULL);
  assert(connection2 != NULL);
  assert(connection1->connection_type != RA_NO_CONNECTION);
  assert(connection2->connection_type != RA_NO_CONNECTION);

  debug_printf("RA::Compare_files( %s:%s, %s:%s)\n", connection1->host_name,
               fname1, connection2->host_name, fname2);

  // is connection1 local?
  if (connection1->connection_type == RA_LOCAL_CONNECTION) {
    // is connection2 also local?
    if (connection2->connection_type == RA_LOCAL_CONNECTION) {
      // both connections are local, so use 'cmpfiles()'
      return cmpfiles(fname1, fname2);
    }

    // *** connection2 is remote ***

    // create a temporary file
    char *tmp_file = dsprintf("/var/tmp/tmpf3XXXXXX");
    if (mkstemp(tmp_file) < 0) {
      error_code = RA_MKTEMP_FAILED;
      xfree(tmp_file);
      return -1;
    }

    // fetch the remote file to the temporary file
    if (Fetch_file(connection2, fname2, tmp_file)) {
      xfree(tmp_file);
      return -1;
    }

    // compare the temporary file with 'fname1'
    int result = cmpfiles(fname1, tmp_file);

    // free temporary storage
    unlink(tmp_file);
    xfree(tmp_file);

    // return result
    return result;
  }

  // *** connection1 is remote ***
  if (connection1->reconnect_and_remain_open())
    return -1;

  if (connection2->connection_type == RA_LOCAL_CONNECTION) {
    // connection2 is local

    // create a temporary file
    char *tmp_file = dsprintf("/var/tmp/tmpf3XXXXXX");
    if (mkstemp(tmp_file) < 0) {
      error_code = RA_MKTEMP_FAILED;
      xfree(tmp_file);
      connection1->Disconnect_remain_open_connection();
      return -1;
    }

    // fetch the remote file to the temporary file
    if (Fetch_file(connection1, fname1, tmp_file)) {
      xfree(tmp_file);
      connection1->Disconnect_remain_open_connection();
      return -1;
    }

    // compare the temporary file with 'fname2'
    int result = cmpfiles(fname2, tmp_file);

    // free temporary storage
    unlink(tmp_file);
    xfree(tmp_file);

    // return result
    connection1->Disconnect_remain_open_connection();
    return result;
  }

  // both connection1 and connection2 are remote
  if (connection2->reconnect_and_remain_open()) {
    connection1->Disconnect_remain_open_connection();
    return -1;
  }

  // are connection1 and connection2 the same? (to the same host
  // with the same username?)

  if (connection1->same_as(connection2) == 0) {
    // the connections are the same, so use a 'local comparison'
    // remotely

    debug_printf("compare_files::remote,same\n");

    // prepare a message with the local compare request
    Message request;
    request.code = RA_COMPFILE_REQUEST;
    request.length = xstrlen(fname1) + 1 + xstrlen(fname2) + 1;
    request.data = (char *)xmalloc(request.length);
    memcpy(request.data, fname1, xstrlen(fname1) + 1);
    memcpy(request.data + xstrlen(fname1) + 1, fname2, xstrlen(fname2) + 1);

    // send the message
    if (connection1->messagePipe->send_message(request)) {
      error_code = RA_SOCKET_ERROR;
      connection1->Disconnect_remain_open_connection();
      connection2->Disconnect_remain_open_connection();

      return -1;
    }

    // receive reply
    Message *reply =
        connection1->messagePipe->get_message(RA_COMPFILE_RESPONSE);
    if (reply == NULL) {
      error_code = RA_SERVER_ERROR;
      connection1->Disconnect_remain_open_connection();
      connection2->Disconnect_remain_open_connection();

      return -2;
    }

    // find out the result of the operation
    int result = 0;
    if (reply->data[0] == 'n')
      result = 1;
    else if (reply->data[0] == 'e')
      result = -1;

    delete reply;
    connection1->Disconnect_remain_open_connection();
    connection2->Disconnect_remain_open_connection();

    return result;
  }

  // both connection1 and connection2 are remote, but not on the same
  // host

  debug_printf("compare_files::remote,different\n");

  // create the first temporary file
  char *tmp_file1 = dsprintf("/var/tmp/tmpf3XXXXXX");
  if (mkstemp(tmp_file1) < 0) {
    error_code = RA_MKTEMP_FAILED;
    xfree(tmp_file1);
    connection1->Disconnect_remain_open_connection();
    connection2->Disconnect_remain_open_connection();

    return -1;
  }

  // fetch the first remote file into the first temporary file
  if (Fetch_file(connection1, fname1, tmp_file1)) {
    xfree(tmp_file1);
    connection1->Disconnect_remain_open_connection();
    connection2->Disconnect_remain_open_connection();

    return -1;
  }

  // create the second temproary file
  char *tmp_file2 = dsprintf("/var/tmp/tmpf3XXXXXX");
  if (mkstemp(tmp_file2) < 0) {
    error_code = RA_MKTEMP_FAILED;
    unlink(tmp_file1);
    xfree(tmp_file1);
    xfree(tmp_file2);
    connection1->Disconnect_remain_open_connection();
    connection2->Disconnect_remain_open_connection();

    return -1;
  }

  // fetch the second remote file into the second temporary file
  if (Fetch_file(connection2, fname2, tmp_file2)) {
    unlink(tmp_file1);
    xfree(tmp_file1);
    xfree(tmp_file2);
    connection1->Disconnect_remain_open_connection();
    connection2->Disconnect_remain_open_connection();

    return -1;
  }

  // compare the temporary files
  int result = cmpfiles(tmp_file1, tmp_file2);

  // free temporary storage
  unlink(tmp_file1);
  unlink(tmp_file2);
  xfree(tmp_file1);
  xfree(tmp_file2);

  // return result
  connection1->Disconnect_remain_open_connection();
  connection2->Disconnect_remain_open_connection();
  return result;
}
