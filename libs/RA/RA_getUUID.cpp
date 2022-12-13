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



#include "Mem.h"
#include "RA.h"
#include "debug.h"
#include "getUUID.h"
#include "uuid.h"
#include <assert.h>
#include <iostream>

// returns the UUID of the object, and if it does not exist,
// it will be created and inserted into the UUID lookup table
// Returns the UUID of the object, NULL UUID if none found.
QUuid RA::getUUID(RA_Connection *connection,  // RA connection
                  const std::string oofs_dir, // root of oofs
                  const std::string path,     // path to the object
                  bool create) // whether to create UUID if it does not exist
{
  debug_printf("RA::getUUID( %s:%s)\n", connection->host_name, path.c_str());

  assert(connection != NULL);
  assert(connection->connection_type != RA_NO_CONNECTION);

  if (connection->connection_type == RA_LOCAL_CONNECTION) {
    return ::getUUID(oofs_dir, path, create);
  }

  // remotely call getUUID
  //
  if (connection->reconnect())
    return QUuid();

  // prepare the message
  Mem buff;
  buff.append_string0(oofs_dir);
  buff.append_string0(path);
  buff.append_byte((unsigned char)create);
  Message request(RA_GET_UUID_REQUEST, (char *)buff.data, buff.size);
  // send the message
  if (connection->messagePipe->send_message(request)) {
    error_code = RA_SOCKET_ERROR;
    connection->Disconnect();
    return QUuid();
  }
  // receive a reply
  Message *reply = connection->messagePipe->get_message(RA_GET_UUID_RESPONSE);
  if (reply == NULL) {
    error_code = RA_SERVER_ERROR;
    connection->Disconnect();
    return QUuid();
  }

  // decode the reply
  QUuid uuid(std::string(reply->data));
  // free the response
  delete reply;
  // return the uuid
  connection->Disconnect();

  return uuid;
}

std::string RA::lookupUUID(RA_Connection *connection,   // RA connection
                           const std::string &oofs_dir, // root of oofs
                           const QUuid &uuid            // uuid to lookup
) {
  debug_printf("RA::lookupUUID( %s:%s)\n", connection->host_name,
               uuid.toString().c_str());

  assert(connection != NULL);
  assert(connection->connection_type != RA_NO_CONNECTION);

  if (connection->connection_type == RA_LOCAL_CONNECTION) {
    return ::lookupUUID(oofs_dir, uuid);
  }

  // remotely call getUUID
  //

  // prepare the message
  if (connection->reconnect())
    return "*";
  Mem buff;
  buff.append_string0(oofs_dir);
  buff.append_string0(uuid.toString());
  Message request(RA_LOOKUP_UUID_REQUEST, (char *)buff.data, buff.size);
  // send the message
  if (connection->messagePipe->send_message(request)) {
    error_code = RA_SOCKET_ERROR;
    connection->Disconnect();
    return "*";
  }
  // receive a reply
  Message *reply =
      connection->messagePipe->get_message(RA_LOOKUP_UUID_RESPONSE);
  if (reply == NULL) {
    error_code = RA_SERVER_ERROR;
    connection->Disconnect();
    return "*";
  }

  // decode the reply
  std::string path = reply->data;
  // free the response
  delete reply;
  // return the uuid
  connection->Disconnect();

  return path;
}

// fixes up UUIDs and the table in the given oofs, for all objects
// with a matching prefix (path). If recursive is not set, the
// operation will only affect a single object.
std::string RA::uuidTableReconcile(RA_Connection *connection,
                                   const std::string &oofs,
                                   const std::string &path, bool recursive,
                                   bool tablePriority) {
  debug_printf("RA::uuidTableReconcile( %s:%s)\n", connection->host_name,
               oofs.c_str());

  assert(connection != NULL);
  assert(connection->connection_type != RA_NO_CONNECTION);

  if (connection->connection_type == RA_LOCAL_CONNECTION) {
    return ::uuidTableReconcile(oofs, path, recursive, tablePriority);
  }
  if (connection->reconnect()) {
    std::string error_list;
    error_list = "Connection to raserver was lost.";
    return error_list;
  }

  // remotely call uuidTableReconcile
  //

  // prepare the message
  Mem buff;
  buff.append_string0(oofs);
  buff.append_string0(path);
  buff.append_byte((unsigned char)recursive);
  buff.append_byte((unsigned char)tablePriority);
  //    Message request( RA_RECONCILE_UUIDS_REQUEST, buff );
  Message request(RA_RECONCILE_UUIDS_REQUEST, (char *)buff.data, buff.size);
  // send the message
  if (connection->messagePipe->send_message(request)) {
    error_code = RA_SOCKET_ERROR;
    std::string error_list;
    error_list = "Connection to raserver was lost.";
    connection->Disconnect();
    return error_list;
  }
  // receive a reply
  Message *reply =
      connection->messagePipe->get_message(RA_RECONCILE_UUIDS_RESPONSE);
  if (reply == NULL) {
    error_code = RA_SERVER_ERROR;
    std::string error_list;
    error_list = "Connection to raserver was lost.";
    connection->Disconnect();
    return error_list;
    // 	return std::list<std::string>("Connection to raserver was lost.");
  }

  // decode the reply string( reply-> data ).split( "\n" );
  std::string log;
  std::string rep = std::string(reply->data);
  size_t pos = rep.find("\n");
  std::string str = rep.substr(0, pos);
  rep = rep.substr(pos);
  log = str;
  while (pos != std::string::npos) {
    pos = rep.find("\n");
    str = rep.substr(0, pos);
    rep = rep.substr(pos);
    log += str;
  }

  // free the response
  delete reply;
  // return the log
  connection->Disconnect();

  return log;
}
