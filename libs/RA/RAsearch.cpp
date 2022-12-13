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
#include "local_search.h"
#include <cassert>
#include <iostream>
#include <stdio.h>
#include <unistd.h>

// searching (starting, continuing and ending)
void RA::searchBegin(RA_Connection *connection, const std::string &oofs,
                     const std::string &start_path, const std::string &pattern,
                     bool caseSensitive, bool exactMatch) {
  assert(connection);
  assert(connection->connection_type != RA_NO_CONNECTION);
  if (connection->connection_type == RA_LOCAL_CONNECTION) {
    return ::searchBegin(oofs, start_path, pattern, caseSensitive, exactMatch);
  }

  // remotely call searchBegin()
  // --------------------------------------------------
  if (connection->reconnect())
    return;

  // prepare the message
  Mem buff;
  buff.append_string0(oofs);
  buff.append_string0(start_path);
  buff.append_string0(pattern);
  buff.append_byte((unsigned char)caseSensitive);
  buff.append_byte((unsigned char)exactMatch);
  //    Message request( RA_SEARCH_BEGIN_REQUEST, buff );
  Message request(RA_SEARCH_BEGIN_REQUEST, (char *)buff.data, buff.size);
  // send the message
  if (connection->messagePipe->send_message(request)) {
    error_code = RA_SOCKET_ERROR;
    return;
  }
  // receive a reply
  Message *reply =
      connection->messagePipe->get_message(RA_SEARCH_BEGIN_RESPONSE);
  if (reply == NULL) {
    error_code = RA_SERVER_ERROR;
    std::cerr << "searchBegin has received an error message" << std::endl;
    return;
  }
  // free the response (it contains nothing)
  delete reply;

  return;
}

// returns path to the next object that matches the pattern
// returns "*" if search ended
// returns "**" if no match found so far, but there may be more (only
//    applicable if 'blocking=false')
std::string RA::searchContinue(RA_Connection *connection, bool blocking) {
  assert(connection);
  assert(connection->connection_type != RA_NO_CONNECTION);

  if (connection->connection_type == RA_LOCAL_CONNECTION) {
    return ::searchContinue(blocking);
  }

  // remotely call searchContinue()
  // --------------------------------------------------

  // prepare the message
  Mem buff;
  buff.append_string0("void");
  Message request(RA_SEARCH_CONTINUE_REQUEST, (char *)buff.data, buff.size);
  // send the message
  if (connection->messagePipe->send_message(request)) {
    error_code = RA_SOCKET_ERROR;
    return "*";
  }
  // receive a reply
  Message *reply =
      connection->messagePipe->get_message(RA_SEARCH_CONTINUE_RESPONSE);

  if (reply == NULL) {
    error_code = RA_SERVER_ERROR;
    return "*";
  }
  // decode the reply
  std::string path = reply->data;
  // free the response
  delete reply;
  // done

  return path;
}

// terminates the search (killing qprocess if still running)
void RA::searchEnd(RA_Connection *connection) {
  if (connection->reconnect())
    return;
  assert(connection);
  assert(connection->connection_type != RA_NO_CONNECTION);
  if (connection->connection_type == RA_LOCAL_CONNECTION) {
    return ::searchEnd();
  }
  // remotely call searchEnd()
  // --------------------------------------------------
  //    connection-> reconnect();

  // prepare the message
  Mem buff;
  buff.append_string0("void");
  Message request(RA_SEARCH_END_REQUEST, (char *)buff.data, buff.size);
  // send the message
  if (connection->messagePipe->send_message(request)) {
    error_code = RA_SOCKET_ERROR;
    return;
  }
  // receive a reply
  Message *reply = connection->messagePipe->get_message(RA_SEARCH_END_RESPONSE);

  if (reply == NULL) {
    error_code = RA_SERVER_ERROR;
    return;
  }
  // free the response (it contains nothing)
  delete reply;
  // done
  connection->Disconnect();

  return;
}
