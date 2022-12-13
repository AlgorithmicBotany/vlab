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



#include <Mem.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "Archive.h"
#include "RA.h"
#include "debug.h"
#include "xmemory.h"
#include "xstring.h"

/******************************************************************************
 *
 *  static int Archive_object ( RA_Connection * connection,
 *				const char * oofs_dir,
 *				const char * object_name,
 *				const char * local_file,
 *				int recursive);
 *
 * - creates an archive of the object specified in object_name into a file
 *   specified by local_file
 *
 * - connection:   specifies connection to the raserver
 *   oofs_dir:     location of the database
 *   object_name:  full name of the object
 *   recursive:    if set then all children of the object are also archived
 *
 * - result:   0 = success, otherwise failure
 *             in case of failure, errorLog will have more verbose
 *                 description of the error
 *
 */
#define __DEBUG__ 0

int RA::Archive_object(RA_Connection *connection, const char *oofs_dir,
                       const char *object_name, const char *local_file,
                       int recursive) {
  assert(connection != NULL);
  assert(connection->connection_type != RA_NO_CONNECTION);

  debug_printf("RA::Archive_object( %s:%s %s %s %s)\n", connection->host_name,
               oofs_dir, object_name, local_file,
               recursive ? "recursive" : "single");

  /*-----------------.
  | LOCAL connection |
  `-----------------*/

  // if the connection is local, do the operation localy
  if (connection->connection_type == RA_LOCAL_CONNECTION) {
    Archive ar(progressReporter_);
    ar.set_destination(Archive::AR_FILE, local_file);
    int res = ar.pack(object_name, recursive);
    if (res) {
      // operation failed - report error
      error_code = RA_ARCHIVE_FAILED;
      fprintf(stderr, "Local archive operation unsuccessful:\n\n%s\n",
              ar.get_errors());
      appendErrorLog(ar.get_errors());
      return 1;
    }

    // report success
    return 0;
  }

  // REMOTE connection
  // ======================================================================
  //
  // send a message to the server requesting an ARCHIVE operation

  Mem data;
  data.append(oofs_dir, xstrlen(oofs_dir) + 1);
  data.append(object_name, xstrlen(object_name) + 1);
  char rec = recursive ? '1' : '0';
  data.append(&rec, 1);
  if (connection->reconnect())
    return 1;

  Message request(RA_ARCHIVE_OBJECT_REQUEST, (char *)data.data, data.size);
  setProgress(0.01);
  if (connection->messagePipe->send_message(request)) {
    error_code = RA_SOCKET_ERROR;
    appendErrorLog("communication error - could not send message "
                   "to RAserver\n");
    connection->Disconnect();

    return 1;
  }

  setProgress(0.02);
  // wait for the response
  Message *response =
      connection->messagePipe->get_message(RA_ARCHIVE_OBJECT_RESPONSE);
  if (response == NULL) {
    error_code = RA_SERVER_ERROR;
    debug_printf("communication error - cannot receive data from RAserver\n");
    appendErrorLog("communication error - cannot receive data from "
                   "RAserver\n");
    connection->Disconnect();

    return 1;
  }

  setProgress(0.03);
  // report a failure
  if (response->length < 1) {
    error_code = RA_SERVER_ERROR;
    debug_printf("received a NULL message from RAserver\n");

    appendErrorLog("internal communication error:\n"
                   "\t- received a NULL message from RAserver\n");
    delete response;
    connection->Disconnect();

    return 1;
  }
  if (response->data[0] == 'n') {
    if (response->length >= 3 && response->data[1] == 0 &&
        response->data[response->length - 1] == 0) {
      // this is a valid negative response - extract the error
      error_code = RA_ARCHIVE_FAILED;
      appendErrorLog(response->data + 2);
    } else {
      // this is not a valid response - indicate that to
      // the user
      error_code = RA_SERVER_ERROR;
      debug_printf("received an invalid negative response from RAserver\n");
      appendErrorLog("internal communication error:\n"
                     "\t- received an invalid negative response from "
                     "RAserver - maybe old version???\n");
    }
    delete response;
    connection->Disconnect();

    return 1;
  }

  // is the response 'success'?
  if (response->length != 1 + sizeof(u_int32_t)) {
    error_code = RA_TYPE_MISMATCH;
    debug_printf("received an invalid negative response from raserver\n");

    appendErrorLog("internal communication error:\n"
                   "\t- received an invalid response from raserver\n");
    delete response;
    connection->Disconnect();

    return 1;
  }

  // extract the length of the archive
  size_t archive_length;
  memcpy(&archive_length, response->data + 1, sizeof(u_int32_t));
  archive_length = ntohl(archive_length);
  if (__DEBUG__)
    fprintf(stderr, "RA::Archive_object(): archive_length=%ld\n",
            archive_length);
  delete response;

  setProgress(0.04);
  // create a local file where the archive will be stored
  int fd = open(local_file, O_WRONLY | O_CREAT | O_TRUNC,
                S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
  if (fd == -1) {
    error_code = RA_ARCHIVE_FAILED;
    appendErrorLog("Cannot create " + std::string(local_file) + "\n");
    fprintf(stderr, "Cannot create %s.\n", local_file);
    return -1;
  }

  // receive messages of type ARCHIVE_BLOCK and write their contents to
  // the local file, until archive_length bytes have been received
  size_t n_written = 0;

  while (n_written < archive_length) {
    setProgress(0.95 * n_written / archive_length + 0.04);
    // receive a message of type ARCHIVE_BLOCK
    if (__DEBUG__)
      fprintf(stderr, "Waiting for a message...");

    Message *bl = connection->messagePipe->get_message(RA_ARCHIVE_BLOCK);

    if (__DEBUG__)
      fprintf(stderr, "received.\n");
    if (bl == NULL) {
      error_code = RA_SERVER_ERROR;
      appendErrorLog("Communication error - transmission from RAserver "
                     "ended unexpectedly\n");
      connection->Disconnect();

      return -1;
    }

    if (__DEBUG__)
      fprintf(stderr, "Received block with %ld bytes, total %ld\n", bl->length,
              n_written);

    // write the contents of this message to the file
    long remain = bl->length;
    long p = 0;
    while (remain > 0) {
      long len = write(fd, bl->data + p, remain);
      if (len <= 0) {
        appendErrorLog(
            "Cannot create an archive in " + std::string(local_file) +
            " because:\n\t- because system reported: " + strerror(errno) +
            "\n");
        fprintf(stderr,
                "Cannot create an archive in %s because:\n"
                "\t%s\n",
                local_file, strerror(errno));
        error_code = RA_ARCHIVE_FAILED;
        close(fd);
        connection->Disconnect();

        return -1;
      }
      remain -= len;
      p += len;
    }
    n_written += bl->length;
  }
  connection->Disconnect();

  // close the local file
  close(fd);

  setProgress(1);

  // return the result
  return 0;
}

/******************************************************************************
 *
 *  static int Dearchive_object ( RA_Connection * connection,
 *				  const char * local_file,
 *			  	  const char * oofs_dir,
 *				  const char * destination)
 *
 * - dearchives a specified archive (local_file) into destination
 *
 * - result:   0 = success, otherwise failure
 *
 */

int RA::Dearchive_object(RA_Connection *connection, const char *local_file,
                         const char *oofs_dir, const char *destination) {
  assert(connection != NULL);
  assert(connection->connection_type != RA_NO_CONNECTION);

  debug_printf("RA::Dearchive_object( %s -> %s:%s[%s])\n", local_file,
               connection->host_name, oofs_dir, destination);

  /*-----------------.
  | LOCAL connection |
  `-----------------*/

  // if the connection is local, do the operation localy
  if (connection->connection_type == RA_LOCAL_CONNECTION) {
    Archive ar(progressReporter_);
    if (ar.unpack(local_file, destination)) {
      // operation failed - report error
      error_code = RA_DEARCHIVE_FAILED;
      fprintf(stderr, "Local dearchive operation unsuccessful:\n\n%s\n",
              ar.get_errors());
      return 1;
    }

    // report success
    return 0;
  }

  /*------------------.
  | REMOTE connection |
  `------------------*/
  if (connection->reconnect())
    return 1;

  // figure out the length of the archive
  size_t archive_size = 0;
  {
    struct stat st;
    if (stat(local_file, &st)) {
      fprintf(stderr, "RA::Dearchive_object(): cannot stat(2) %s.\n\t-%s\n",
              local_file, strerror(errno));
      error_code = RA_LOCAL_STAT_FAILED;
      connection->Disconnect();
      return 1;
    }
    archive_size = st.st_size;
  }

  // convert the archive_size into a network format
  size_t n_archive_size = htonl(archive_size);

  // compose a message to the server requesting a DEARCHIVE operation
  //
  //  - this message contains the database name, destination and
  //    the length (network format) of the archive which will follow
  //
  Mem data;
  data.append_string0(oofs_dir);
  data.append_string0(destination);
  data.append_byte(0);
  data.append_ulong(n_archive_size);
  Message request(RA_DEARCHIVE_OBJECT_REQUEST, (char *)data.data, data.size);

  // send the message to the server

  connection->messagePipe->send_message(request);
  // connection-> Disconnect();

  //
  // now send the whole file in blocks of 16K
  // ========================================
  //

  // open the archive
  int fd = open(local_file, O_RDONLY);
  if (fd == -1) {
    fprintf(stderr, "RA::Dearchive_object(): cannot open %s.\n\t-%s\n",
            local_file, strerror(errno));
    error_code = RA_OPEN_FAILED;
    connection->Disconnect();
    return 1;
  }

  // send chunks of 16KB until we are done
  char buff[16384];
  long remains = archive_size;
  while (remains > 0) {
    // get the next 16KB (or whatever is remaining) from the archive
    long len = read(fd, buff, sizeof(buff));
    if (len <= 0) {
      fprintf(stderr, "RA::Dearchive_object(): cannot read(2) %s.\n\t-%s\n",
              local_file, strerror(errno));
      error_code = RA_READ_FAILED;
      connection->Disconnect();
      return 1;
    }

    // create a message with this data
    Message m(RA_ARCHIVE_BLOCK, buff, len);
    //	if (connection-> reconnect()) return 1;

    connection->messagePipe->send_message(m);

    // update the remaining count
    if (len > remains) {
      fprintf(stderr, "RA::Dearchive_object(): weird error - I was able to\n"
                      "                        read more bytes out of the\n"
                      "                        archive than the original \n"
                      "                        length of the archive...\n\n");
      break;
    }
    remains -= len;
  }

  // close the input file
  close(fd);

  // wait for the response

  Message *response =
      connection->messagePipe->get_message(RA_DEARCHIVE_OBJECT_RESPONSE);

  if (response == NULL) {
    error_code = RA_SERVER_ERROR;
    connection->Disconnect();
    return 1;
  }

  // is the response 'success'?
  if (response->data[0] == 'n') {
    error_code = RA_DEARCHIVE_FAILED;
    delete response;
    connection->Disconnect();
    return 1;
  }

  // delete the response message and return success
  delete response;
  connection->Disconnect();
  return 0;
}
