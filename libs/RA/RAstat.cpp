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
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "RA.h"
#include "debug.h"
#include "dsprintf.h"
#include "xmemory.h"
#include "xstring.h"

/******************************************************************************
 *
 * find out statistics about a remote file
 *
 *   - returns: 0 = success (+ fills out the structure stat_struc)
 *              1 = failure
 *   - if stat_struc is NULL, only return status is returned
 */

int RA::Stat(RA_Connection *connection, const char *fname,
             RA_Stat_Struc *stat_struc) {
  debug_printf("RA::Stat( %s:%s)\n", connection->host_name, fname);

  assert(connection != NULL);
  assert(connection->connection_type != RA_NO_CONNECTION);

  if (connection->connection_type == RA_LOCAL_CONNECTION) {
    // the file is local - so use stat to find out the statistics
    struct stat buff;
    if (stat(fname, &buff) != 0) {
      if (stat_struc != NULL)
        stat_struc->type = RA_NOEXIST_TYPE;
      error_code = RA_LOCAL_STAT_FAILED;
      return 1;
    }
    if (stat_struc != NULL) {
      if (S_ISDIR(buff.st_mode))
        stat_struc->type = RA_DIR_TYPE;
      else if (S_ISREG(buff.st_mode))
        stat_struc->type = RA_REG_TYPE;
      else
        stat_struc->type = RA_OTHER_TYPE;
      // determine if the file is a link
      stat_struc->is_link = 0;
      if (lstat(fname, &buff) == 0)
        stat_struc->is_link = S_ISLNK(buff.st_mode);

      // determine the attributes of the file
      stat_struc->readable = !access(fname, R_OK);
      stat_struc->writeable = !access(fname, W_OK);
      /*stat_struc-> executable =
          (! access( fname, X_OK)) ||
          (! access( fname, EX_OK));*/
      stat_struc->executable = !access(fname, X_OK);
    }

    return 0;
  }

  //
  // REMOTE stat
  //
  if (connection->reconnect())
    return -1;
  // prepare the message
  Message request(RA_STAT_REQUEST, fname, xstrlen(fname) + 1);
  // send the message
  if (connection->messagePipe->send_message(request)) {
    error_code = RA_SOCKET_ERROR;
    connection->Disconnect();

    return -1;
  }

  // receive a reply
  Message *reply = connection->messagePipe->get_message(RA_STAT_RESPONSE);
  if (reply == NULL) {
    error_code = RA_SERVER_ERROR;
    connection->Disconnect();

    return -2;
  }

  // decode the reply
  char *result = reply->data;
  int result_valid =
      ((result[0] == 'd') || (result[0] == 'f') || (result[0] == 'n')) &&
      ((result[1] == 'l') || (result[1] == '-')) &&
      ((result[2] == 'r') || (result[2] == '-')) &&
      ((result[3] == 'w') || (result[3] == '-')) &&
      ((result[4] == 'x') || (result[4] == '-'));

  // if the result is not valid, report error
  if (!result_valid) {
    delete reply;
    error_code = RA_SERVER_ERROR;
    connection->Disconnect();

    return 1;
  }

  // decode the result
  if (stat_struc != NULL) {
    if (result[0] == 'd')
      stat_struc->type = RA_DIR_TYPE;
    else if (result[0] == 'f')
      stat_struc->type = RA_REG_TYPE;
    else if (result[0] == 'n')
      stat_struc->type = RA_NOEXIST_TYPE;

    /*
    stat_struc-> is_link = ( result[ 1] == 'l');
    stat_struc-> readable = (result[ 2] == 'r');
    stat_struc-> writeable = (result[ 3] == 'w');
    stat_struc-> executable = (result[ 4] == 'x');
    */
    stat_struc->is_link = result[1];
    stat_struc->readable = result[2];
    stat_struc->writeable = result[3];
    stat_struc->executable = result[4];
  }

  // free the response
  delete reply;
  connection->Disconnect();

  // return success
  return 0;
}
