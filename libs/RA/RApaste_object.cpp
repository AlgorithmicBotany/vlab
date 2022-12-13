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
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "Mem.h"
#include "RA.h"
#include "debug.h"
#include "dsprintf.h"
#include "paste_object.h"
#include "xmemory.h"
#include "xstring.h"

/******************************************************************************
 *
 * int RA::paste_object( RA_Connection * connection,
 *                       const char * oofs_dir,
 *                       const char * dst_fname,
 *                       const char * archive_name,
 *                       const char * old_path,
 *                       char move_links )
 *
 * - paste object(s) into remote database and update .uuids appropriately
 *
 * - parameters:
 *        connection - a valid connection to an RAserver
 *        oofs_dir   - root directory of the remote database
 *        dst_fname  - location of the directory where to paste the data
 *        archive_name - name of the local file holding the archive data
 *        old_path   - the original location of the objects (needed if
 *                     move_links is set)
 *        move_links - this flag specifies how to adjust the .uuids. If
 *                     not set, the newly pasted objects are all assigned
 *                     new (and unique) IDs. If set, the new objects keep
 *                     their IDs and the original ones (in old_path) will
 *                     be assigned new IDs.
 *
 * - return:  0 = success
 *           !0 = failure:
 *                  -1 = write access denied
 *                  -2 = object already exists
 *                  -4 = cannot open archive
 *                  -5 = raserver has resources (or other) problems
 *
 * - in case of a failure, the error_code is properly set.
 *
 */

int RA::Paste_object(RA_Connection *connection, const char *oofs_dir,
                     const char *dst_fname, const char *archive_name,
                     const char *old_path, char move_links) {

  assert(connection != NULL);
  assert(connection->connection_type != RA_NO_CONNECTION);

  // LOCAL connection
  // ==================================================

  // if the connection is local, do the operation localy
  if (connection->connection_type == RA_LOCAL_CONNECTION) {
    int res =
        paste_object(oofs_dir, dst_fname, archive_name, old_path, move_links);

    if (res)
      error_code = RA_PASTE_FAILED;
    return res;
  }

  // REMOTE connection
  // ==================================================

  // open the archive file (so that we know if it can be opened
  // and also so that we can figure out its size)
  if (connection->reconnect())
    return -5;

  int fd = open(archive_name, O_RDONLY);
  if (fd == -1) {
    fprintf(stderr,
            "RA::Paste_object(): cannot open archive file '%s'.\n"
            "\t-%s\n",
            archive_name, strerror(errno));
    error_code = RA_OPEN_FAILED;
    connection->Disconnect();
    return -4;
  }

  // figure out the length of the archive
  size_t archive_size = 0;
  {
    struct stat st;
    if (fstat(fd, &st)) {
      fprintf(stderr, "RA::Paste_object(): cannot fstat(2) %s.\n\t-%s\n",
              archive_name, strerror(errno));
      error_code = RA_LOCAL_STAT_FAILED;
      close(fd);
      connection->Disconnect();
      return -4;
    }
    archive_size = st.st_size;
  }

  // convert the archive_size into a network format
  u_int32_t n_archive_size = htonl(archive_size);

  // compose a message to the server requesting a PASTE operation
  //
  //  - this message contains the database name, destination and
  //    the length (network format) of the archive which will follow,
  //    as well as the old object's name and whether to move the links
  //    with the new objects
  //
  Mem mem;
  mem.append((void *)oofs_dir, xstrlen(oofs_dir) + 1);
  mem.append((void *)dst_fname, xstrlen(dst_fname) + 1);
  mem.append((void *)old_path, xstrlen(old_path) + 1);
  mem.append(&move_links, sizeof(move_links));
  mem.append(&n_archive_size, sizeof(n_archive_size));

  Message request;
  request.code = RA_PASTE_OBJECT_REQUEST;
  request.length = mem.size;
  request.data = (char *)mem.data;

  // send the message to the server
  connection->messagePipe->send_message(request);
  request.data = NULL; // to make sure ~Message() doesn't try to free it as well

  /*-----------------------------------------.
    | now send the whole file in blocks of 16K |
    `-----------------------------------------*/

  char buff[16384];
  long remains = archive_size;
  while (remains > 0) {
    // get the next 16KB (or whatever is remaining) from the archive
    long len = read(fd, buff, sizeof(buff));
    if (len <= 0) {
      fprintf(stderr, "RA::Paste_object(): cannot read(2) %s.\n\t-%s\n",
              archive_name, strerror(errno));
      error_code = RA_READ_FAILED;
      connection->Disconnect();
      return 1;
    }

    // create a message with this data
    Message m(RA_ARCHIVE_BLOCK, buff, len);
    connection->messagePipe->send_message(m);

    // update the remaining count
    if (len > remains) {
      fprintf(stderr, "RA::Paste_object(): weird error - I was able to\n"
                      "                    read more bytes out of the\n"
                      "                    archive than the original\n"
                      "                    length of the archive...\n\n");
      break;
    }
    remains -= len;
  }

  // close the input file
  close(fd);

  // wait for the response
  Message *response =
      connection->messagePipe->get_message(RA_PASTE_OBJECT_RESPONSE);
  if (response == NULL) {
    error_code = RA_SERVER_ERROR;
    connection->Disconnect();
    return 1;
  }

  // is the response 'success'?
  if (response->data[0] == 'n') {
    error_code = RA_PASTE_FAILED;

    // extract the error code from the response
    u_int32_t n_res = *(u_int32_t *)&(response->data[1]);
    int res = ntohl(n_res) - 128;

    delete response;
    connection->Disconnect();
    return res;
  }

  // delete the response message and return success
  delete response;
  connection->Disconnect();

  return 0;
}
