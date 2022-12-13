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
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "RA.h"
#include "debug.h"
#include "dsprintf.h"
#include "utilities.h"
#include "xmemory.h"
#include "xstring.h"

/******************************************************************************
 *
 * int Fetch_file( RA_Host * host,
 *                 const char * remote_fname,
 *                 const char * local_fname,
 *
 *
 *   - copies a remote file to a local file
 *
 *   - returns:  0 = success
 *               1 = failure
 *
 */

int RA::Fetch_file(RA_Connection *connection, const char *remote_fname,
                   const char *local_fname) {
  assert(connection->connection_type != RA_NO_CONNECTION);

  debug_printf("RA::Fetch_file( %s:%s, %s)\n", connection->host_name,
               remote_fname, local_fname);

  // if the connection is local, do a local copy
  if (connection->connection_type == RA_LOCAL_CONNECTION) {
    return cpfile(remote_fname, local_fname);
  }
  debug_printf(" message before reconnecting : %s \n", remote_fname);

  //
  // the connection is to a remote host
  //
  if (connection->reconnect())
    return 1;

  // create a request message

  debug_printf("create a request message : %s \n", remote_fname);
  Message request;
  request.code = RA_FETCH_FILE_REQUEST;
  request.data = (char *)xstrdup(remote_fname);
  request.length = xstrlen(remote_fname) + 1;

  // send the message
  debug_printf("send the request message : %s - %s\n", remote_fname,
               request.data);
  if (connection->messagePipe->send_message(request)) {
    error_code = RA_FETCH_FAILED;
    connection->Disconnect();
    return 1;
  }

  // wait for the reply
  debug_printf(" wait for the reply\n");
  Message *response =
      connection->messagePipe->get_message(RA_FETCH_FILE_RESPONSE);
  if (response == NULL) {
    error_code = RA_FETCH_FAILED;
    connection->Disconnect();

    return 1;
  }

  // check if the content of the file arrived
  debug_printf("check if the content of the file arrived : %s\n",
               response->data);
  if (response->length < 1) {
    error_code = RA_FETCH_FAILED;
    delete response;
    connection->Disconnect();

    return 1;
  }

  // check the first character of the response
  debug_printf("check the first character of the response : %s\n",
               response->data);
  if (response->data[0] != 'y') {
    error_code = RA_FETCH_FAILED;
    delete response;
    connection->Disconnect();
    return 1;
  }

  // open the destination file (local file)
  debug_printf(" open the destination file (local file)\n");
  int fd = open(local_fname, O_WRONLY | O_CREAT | O_TRUNC,
                S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
  if (fd == -1) {
    perror("RA::Fetch_File:open()");
    delete response;
    error_code = RA_FETCH_FAILED;
    connection->Disconnect();
    return 1;
  }

  // write the received date into this file
  debug_printf("write the received date into this file\n");
  if (response->length - 1 !=
      write(fd, response->data + 1, response->length - 1)) {
    error_code = RA_FETCH_FAILED;
    close(fd);
    perror("RA::Fetch_File:write()");
    delete response;
    connection->Disconnect();
    return 1;
  }

  // close the local file
  close(fd);
  connection->Disconnect();

  // return success
  return 0;
}

/******************************************************************************
 *
 * int Fetch_file( RA_Host * host,
 *                 const char * remote_fname,
 *                 FILE * fp)
 *
 *
 *   - copies a remote file to a local file (given by descriptor)
 *
 *   - returns:  0 = success
 *               1 = failure
 *
 */

int RA::Fetch_file(RA_Connection *connection, const char *remote_fname,
                   FILE *fp) {
  assert(connection->connection_type != RA_NO_CONNECTION);

  debug_printf("RA::Fetch_file( %s:%s, FILE *)\n", connection->host_name,
               remote_fname);

  // if the connection is local, do a local open
  if (connection->connection_type == RA_LOCAL_CONNECTION) {
    int fd = open(remote_fname, O_RDONLY);
    if (fd == -1)
      return 1;
    long n;
    while (1) {
      char buff[4096];
      n = read(fd, buff, sizeof(buff));
      if (n <= 0)
        break;
      if (1 != fwrite(buff, n, 1, fp)) {
        error_code = RA_FETCH_FAILED;
        return 1;
      }
    }
    close(fd);
    fseek(fp, 0, SEEK_SET);
    return 0;
  }

  //
  // the connection is to a remote host
  //
  if (connection->reconnect())
    return 1;

  // create a request message
  Message request;
  request.code = RA_FETCH_FILE_REQUEST;
  request.data = (char *)xstrdup(remote_fname);
  request.length = xstrlen(remote_fname) + 1;

  // send the message
  if (connection->messagePipe->send_message(request)) {
    error_code = RA_FETCH_FAILED;
    connection->Disconnect();

    return 1;
  }

  // wait for the reply
  Message *response =
      connection->messagePipe->get_message(RA_FETCH_FILE_RESPONSE);
  if (response == NULL) {
    error_code = RA_FETCH_FAILED;
    connection->Disconnect();
    return 1;
  }

  // check if the content of the file arrived
  if (response->length < 1) {
    error_code = RA_FETCH_FAILED;
    connection->Disconnect();
    delete response;
    return 1;
  }

  // check the first character of the response
  if (response->data[0] != 'y') {
    error_code = RA_FETCH_FAILED;
    connection->Disconnect();
    delete response;
    return 1;
  }

  // write the result to the local file
  if (1 != fwrite(response->data + 1, response->length - 1, 1, fp)) {
    error_code = RA_FETCH_FAILED;
    perror("RA::Fetch_File:fwrite()");
    delete response;
    connection->Disconnect();

    return 1;
  }
  fseek(fp, 0, SEEK_SET);

  // return success
  connection->Disconnect();

  return 0;
}
