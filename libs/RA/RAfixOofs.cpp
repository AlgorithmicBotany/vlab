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



// LINUX includes
#include <cassert>
#include <string>

// VLAB includes
#include "Mem.h"
#include "Message.h"
#include "RA.h"
#include "fixoofs.h"

// fixes ups oofs...
// - removes duplicate UUIDS
// - removes unused UUIDS
// - renumbers all UUIDS (if renumber = true)
// - reports broken links
// - makes sure the UUID lookup table is in sync with the rest of the OOFS
// Returns: the log
std::string RA::fixOofs(RA_Connection *connection, const std::string &oofs,
                        bool renumber) {
  assert(connection != NULL);
  assert(connection->connection_type != RA_NO_CONNECTION);

  // call fixOofs() directly if this is a local connection
  if (connection->connection_type == RA_LOCAL_CONNECTION) {
    return ::fixOofs(oofs, renumber);
  }

  // call fixOofs() via raserver
  if (connection->reconnect())
    return "No connection to raserver. Could not get a meaningful reply from "
           "raserver.";

  Mem buff;
  buff.append_string0(oofs);
  buff.append_byte((unsigned char)renumber);
  Message request(RA_FIX_OOFS_REQUEST, buff);
  if (connection->messagePipe->send_message(request)) {
    error_code = RA_SOCKET_ERROR;
    connection->Disconnect();
    return "Could not talk to raserver.";
  }

  // receive reply
  Message *reply = connection->messagePipe->get_message(RA_FIX_OOFS_RESPONSE);
  if (reply == 0) {
    error_code = RA_SERVER_ERROR;
    connection->Disconnect();
    return "Could not get a meaningful reply from raserver.";
  }

  // decode reply
  std::string log = reply->data; //+ split( "\n" );
  delete reply;
  connection->Disconnect();

  // return the log
  return log;
}
