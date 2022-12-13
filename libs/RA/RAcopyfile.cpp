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
#include <unistd.h>

#include "RA.h"
#include "debug.h"
#include "dsprintf.h"
#include "utilities.h"
#include "xmemory.h"
#include "xstring.h"

/******************************************************************************
 *
 * int RA::Copy_file( RA_Connection * src_connection,
 *                    const char * src_fname,
 *                    RA_Connection * dst_connection,
 *                    const char * dst_fname)
 *
 * - copies file from src to dst
 *
 * - returns:  0 = succes
 *            !0 = error
 *
 */

int RA::Copy_file(RA_Connection *src_connection, const char *src_fname,
                  RA_Connection *dst_connection, const char *dst_fname) {
  // preconditions
  assert(src_connection != NULL);
  assert(dst_connection != NULL);
  assert(src_connection->connection_type != RA_NO_CONNECTION);
  assert(dst_connection->connection_type != RA_NO_CONNECTION);

  debug_printf("RA::Copy_file( %s:%s, %s:%s)\n", src_connection->host_name,
               src_fname, dst_connection->host_name, dst_fname);

  debug_printf("RA::Copy_file( %s:%s, %s:%s)\n", src_connection->host_name,
               src_fname, dst_connection->host_name, dst_fname);

  if (src_connection->connection_type == RA_LOCAL_CONNECTION) {
    // src is local
    if (dst_connection->connection_type == RA_LOCAL_CONNECTION) {
      debug_printf("copy_files::local-> local\n");
      // both src and dst are local
      return cpfile(src_fname, dst_fname);
    }

    debug_printf("copy_files::local-> remote\n");
    // dst is remote
    return Put_file(src_fname, dst_connection, dst_fname);
  }

  // src is remote
  if (dst_connection->connection_type == RA_LOCAL_CONNECTION) {
    debug_printf("copy_files::remote-> local\n");
    // dst is local, so use Fetch_file
    return Fetch_file(src_connection, src_fname, dst_fname);
  }

  //
  // both src and dst are remote
  //

  // if both connections are to the same host, do a copy remotely
  if (src_connection->same_as(dst_connection) == 0) {
    if (src_connection->reconnect_and_remain_open())
      return 1;

    debug_printf("copy_files::remote-> remote (same)\n");

    // prepare a message with the local copy request
    Message request;
    request.code = RA_COPYFILE_REQUEST;
    request.length = xstrlen(src_fname) + 1 + xstrlen(dst_fname) + 1;
    request.data = (char *)xmalloc(request.length);
    memcpy(request.data, src_fname, xstrlen(src_fname) + 1);
    memcpy(request.data + xstrlen(src_fname) + 1, dst_fname,
           xstrlen(dst_fname) + 1);

    // send the message
    if (src_connection->messagePipe->send_message(request)) {
      error_code = RA_SOCKET_ERROR;
      src_connection->Disconnect_remain_open_connection();
      return 1;
    }

    // receive reply
    Message *reply =
        src_connection->messagePipe->get_message(RA_COPYFILE_RESPONSE);
    if (reply == NULL) {
      error_code = RA_SERVER_ERROR;
      src_connection->Disconnect_remain_open_connection();
      return 2;
    }

    // find the result of the operation
    int result = 0;
    if (reply->data[0] != 'y')
      result = 3;
    delete reply;
    src_connection->Disconnect_remain_open_connection();
    return result;
  }

  if (src_connection->reconnect_and_remain_open())
    return 1;
  if (dst_connection->reconnect_and_remain_open())
    return 1;

  // both connections are remote and different

  debug_printf("copy_files::remote-> remote (different)\n");

  // create a temp. file
  char *tmp_file = xstrdup("/var/tmp/tmpcp2XXXXXX");
  if (mkstemp(tmp_file) < 0) {
    xfree(tmp_file);
    error_code = RA_MKTEMP_FAILED;
    src_connection->Disconnect_remain_open_connection();
    dst_connection->Disconnect_remain_open_connection();

    return 1;
  }
  // copy the src to tmp. file
  if (0 != Fetch_file(src_connection, src_fname, tmp_file)) {
    xfree(tmp_file);
    src_connection->Disconnect_remain_open_connection();
    dst_connection->Disconnect_remain_open_connection();

    return 1;
  }

  // copy the tmp. file into dst. file
  if (0 != Put_file(tmp_file, dst_connection, dst_fname)) {
    unlink(tmp_file);
    xfree(tmp_file);
    src_connection->Disconnect_remain_open_connection();
    dst_connection->Disconnect_remain_open_connection();
    return 1;
  }

  unlink(tmp_file);
  xfree(tmp_file);
  src_connection->Disconnect_remain_open_connection();
  dst_connection->Disconnect_remain_open_connection();

  return 0;
}
