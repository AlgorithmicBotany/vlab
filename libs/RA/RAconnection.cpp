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



#include <arpa/inet.h>
#include <assert.h>
#include <ifaddrs.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "RA.h"
#include "RAconnection.h"
#include "debug.h"
#include "dsprintf.h"
#include "version.h"
#include "xmemory.h"
#include "xstring.h"

/******************************************************************************
 *
 * constructor
 *
 *
 */

RA_Connection::RA_Connection(void) {
  debug_printf("RA_Connection::RA_Connection().\n");

  password = NULL;
  login_name = NULL;
  connection_type = RA_NO_CONNECTION;
  addr = NULL;
  addr_length = 0;
  sock = -1;
  messagePipe = NULL;
  keep_connection_open = false;
}

/******************************************************************************
 *
 * int what_connection( char * name)
 *
 *      - checks if name refers to local or remote machine
 *            - returns:
 *                  RA_REMOTE_CONNECTION or RA_LOCAL_CONNECTION
 *                  or RA_NO_CONNECTION
 */

RA_Connection_Type RA_Connection::get_connection_type(void) {
  // special case: if the remote host name is 'localhost', it is
  // referring to the local machine
  if (xstrcmp(host_name, "localhost") == 0)
    return RA_LOCAL_CONNECTION;

  // get IP addresses of local host. Then compare to remote host, if none are
  // similar then connection is remote.

  // 1st we get hostent about remote host (try 3 times, with 1 sec. intervals)
  int count = 0;
  while (1) {
    struct addrinfo hints, *res, *p;
    int status;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // AF_INET or AF_INET6 to force version
    hints.ai_socktype = SOCK_STREAM;

    // Get Connection Type Try to get remote IP adress
    if ((status = getaddrinfo(host_name, NULL, &hints, &res)) != 0) {

       if (count != 3) {
        sleep(1);
        count++;
        continue;
      }

      // we tried 3 times to get the IP address of the remote host,
      // unsuccessfuly, so we assume that the name of the remote
      // host is invalid
      perror("RA_Connection::get_connection_type():gethostbyname( remote)");

      return RA_NO_CONNECTION;
    } else {

      p = res;
      // success
      assert(p->ai_family == AF_INET);
      // save the address of the local host
      addr_length = p->ai_addrlen;
      struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;

      addr = (unsigned char *)xmalloc(addr_length);
      unsigned char *addr_ready = (unsigned char *)&ipv4->sin_addr.s_addr;
      memcpy(addr, addr_ready, addr_length);
      freeaddrinfo(res);
      break;
    }
  }

  // now get IP addresses from local host amd compare with remote
  RA_Connection_Type result = RA_LOCAL_CONNECTION;

  struct ifaddrs *ifAddrStruct = NULL;
  struct ifaddrs *ifa = NULL;
  void *tmpAddrPtr = NULL;

  unsigned char *local_h_addr;
  int local_h_length;

  getifaddrs(&ifAddrStruct);

  for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
    if (!ifa->ifa_addr) {
      continue;
    }
    if (ifa->ifa_addr->sa_family == AF_INET) { // check it is IP4
      // is a valid IP4 Address
      tmpAddrPtr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
      char addressBuffer[INET_ADDRSTRLEN];
      local_h_length = INET_ADDRSTRLEN;
      local_h_addr = (unsigned char *)malloc(local_h_length);

      inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);

      char *buff = (char *)malloc(10);
      buff = strtok(addressBuffer, ".");
      int i = 0;
      while (buff != NULL) {
        local_h_addr[i] = (unsigned char)atoi(buff);
        buff = strtok(NULL, ".");
        i++;
      }
      free(buff);

      // local_h_addr[1], local_h_addr[2], local_h_addr[3]);
      // compare the address of the host with the local address
      bool sameIP = true;
      for (int j = 0; j < local_h_length; j++) {
        if (local_h_addr[j] != addr[j]) {
          sameIP = false;
        }
      }
      if (sameIP) {
        result = RA_LOCAL_CONNECTION;
        xfree(local_h_addr);
        return result;
      }
    }
  }
  result = RA_REMOTE_CONNECTION;
  xfree(local_h_addr);
  debug_printf("host '%s' is %s\n", host_name,
               result == RA_LOCAL_CONNECTION ? "local" : "remote");

  return result;
}

/******************************************************************************
 *
 * connect to the host
 *
 *   - set the type of the connection (to local, to remote, none)
 *       (by comparing the IP addresses of the local machine and the
 *        remote host machine)
 *
 *   - set the address of the host in the 'addr' & 'addr_length' fields
 *
 *   - simply try to execute an 'echo' program on the remote host with
 *     a string, and then check the response
 *
 * returns: 0 = success
 *          1 = failure - invalid host name
 *          2 = failure - invalid login
 *          3 = some other (socket) error
 *          4 = server not running error
 *          5 = version mismatch
 *
 */

int RA_Connection::Connect(const char *new_host_name, const int new_port_num,
                           const char *new_login_name,
                           const char *new_password) {
  //
  // first determine if this is a local/remote connection
  //

  if (new_host_name == NULL)
    new_host_name = "localhost";

  host_name = xstrdup(new_host_name);
  port_num = new_port_num;

  // check if the host name refers to the local host
  connection_type = get_connection_type();

  // if the connection is invalid, return with failure
  if (connection_type == RA_NO_CONNECTION) {
    debug_printf("Return No connection \n");
    return 4;
  }

  // save the login name and password
  login_name = xstrdup(new_login_name);
  password = xstrdup(new_password);

  // if the host is a local machine, just return success
  if (connection_type == RA_LOCAL_CONNECTION)
    return 0;

  // the connection is REMOTE - so establish a connection to the
  // remote RAserver by opening a socket

  // Create socket
  sock = socket(AF_INET, SOCK_STREAM, 0);

  if (sock == -1) {
    perror("RAconnection:Connect():socket()");
    return 3;
  }

  // Get server's host info, and fill in the sockaddr_in structure
  struct hostent *host_ptr = gethostbyname(host_name);
  if (host_ptr == NULL) {
    perror("RAconnection:Connect():gethostbyname()");
    return 1;
  }

  struct sockaddr_in server_info;
  bzero((char *)&server_info, sizeof server_info);
  server_info.sin_family = AF_INET;
  server_info.sin_port = htons(port_num);

  bcopy(host_ptr->h_addr, (char *)&server_info.sin_addr, host_ptr->h_length);

  // Now connect socket to the server
  if ((connect(sock, (struct sockaddr *)&server_info, sizeof server_info)) !=
      0) {
    perror("RAconnection:Connect():connect()");
    return 4;
  }

  // establish a message pipe
  if (messagePipe)
    delete messagePipe;
  messagePipe = new MessagePipe(sock);

  // request vlab version info and match it to ours
  messagePipe->send_message(Message(RA_VERSION_REQUEST, "?", 2));
  Message *version_reply = messagePipe->get_first_message();
  if (version_reply == 0)
    return 5;
  std::string ra_server_version = version_reply->data;
  delete version_reply;
  std::string our_version = "protocol 4.3";
  if (ra_server_version != our_version) {
    fprintf(stderr, "raserver is talking in an old protocol.\n");
    return 5;
  }
  // versions match...
  // send a LOGIN request to the server
  Message request;
  request.code = RA_LOGIN_REQUEST;
  request.data = (char *)dsprintf("%s:%s", login_name, password);
  request.length = xstrlen((char *)request.data);
  if (messagePipe->send_message(request)) {
    debug_printf("RAconnection:Connect():send_message() problem\n");
    return 3;
  }

  // retrieve the response
  Message *response = messagePipe->get_message(RA_LOGIN_RESPONSE);
  if (response == NULL) {
    debug_printf("RAconnection:Connect(): login not confirmed.\n");
    return 2;
  }
  if (xstrcmp("login confirmed", (char *)response->data)) {
    debug_printf("RAconnection:Connect(): login not confirmed.\n");
    return 2;
  }
  delete response;

  debug_printf("RAconnection:Connect(): Connection established.\n");
  //   keep_connection_open = true;
  // return success
  return 0;
}

/******************************************************************************
 *
 * int RA_Connection::same_as( RA_Connection * c)
 *
 * - returns:  0 - if the connection is the same as connection 'c'
 *                 (i.e. same host & same user name)
 *
 *             1 - otherwise
 */

int RA_Connection::same_as(RA_Connection *c) {
  // preconditions
  assert(c != NULL);
  assert(connection_type != RA_NO_CONNECTION);
  assert(c->connection_type != RA_NO_CONNECTION);

  // compare first the usernames
  if (xstrcmp(login_name, c->login_name) != 0)
    return 1;

  // now compare the IP addresses
  if (addr_length != c->addr_length)
    return 1;

  // return 0 if the addresses are the same, otherwise return 1
  return (memcmp(addr, c->addr, addr_length) == 0) ? 0 : 1;
}

/******************************************************************************
 *
 * disconnects from the server
 *
 */

void RA_Connection::Disconnect(void) {
  if (!keep_connection_open) {

    assert(connection_type != RA_NO_CONNECTION);

    if (connection_type == RA_REMOTE_CONNECTION) {
      Message m(RA_LOGOUT_REQUEST, "y", 2);
      messagePipe->send_message(m);
      messagePipe->get_message(RA_LOGOUT_RESPONSE);
    }
    debug_printf("Disconnecting\n");

    keep_connection_open = false;
    if (sock != -1)
      close(sock);
  }
}

std::string RA_Connection::get_IP() {
  // special case: if the remote host name is 'localhost', it is
  // referring to the local machine
  debug_printf("RA_Connection:: get_IP() \n");

  if (xstrcmp(host_name, "localhost") == 0)
    return std::string("127.0.0.1");

  // determine the name of the local host
  char local_host_name[4096];
  if (gethostname(local_host_name, sizeof(local_host_name))) {
    perror("RA_Connection::get_connection_type():gethostname()");
    assert(1 == 2);
  }

  // get hostent about local host (try 3 times, with 1 sec. intervals)
  unsigned char *local_h_addr;
  int local_h_length;
  //  struct hostent *h;
  int count = 0;
  std::string new_local_ip_address;
  while (1) {

    struct addrinfo hints, *res, *p;
    int status;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // AF_INET or AF_INET6 to force version
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(local_host_name, NULL, &hints, &res)) != 0) {
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
      if (count != 3) {
        sleep(1);
        count++;
        continue;
      }

      // we have tried 3 times and still cannot get the IP address
      // of the local machine, so we set the IP address to the
      // localhost (127.0.0.1)
      perror("RA_Connection::get_IP():gethostbyname( local)");
      local_h_length = 4;
      local_h_addr = (unsigned char *)xmalloc(local_h_length);
      assert(local_h_addr != NULL);
      local_h_addr[0] = 127;
      local_h_addr[1] = 0;
      local_h_addr[2] = 0;
      local_h_addr[3] = 1;
      new_local_ip_address = std::string("127.0.0.1");
      break;
    }

    else // getaddrinfo successful
    {

      p = res;
      // success
      assert(p->ai_family == AF_INET);
      // save the address of the local host
      local_h_length = p->ai_addrlen;
      struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;

      local_h_addr = (unsigned char *)xmalloc(local_h_length);
      assert(local_h_addr != NULL);
      unsigned char *addr_ready = (unsigned char *)&ipv4->sin_addr.s_addr;
      memcpy(local_h_addr, addr_ready, local_h_length);

      new_local_ip_address = inet_ntoa((struct in_addr)ipv4->sin_addr);

      debug_printf("My IP adress : %s\n",
                   inet_ntoa((struct in_addr)ipv4->sin_addr));
      freeaddrinfo(res);
      break;
    }
  }

  xfree(local_h_addr);
  return new_local_ip_address;
}

bool RA_Connection::IP_has_changed(void) {

  std::string new_local_ip_address = get_IP();
  debug_printf("My current IP adress : %s\n", new_local_ip_address.c_str());
  debug_printf("My old IP adress : %s\n", local_ip_address.c_str());

  return new_local_ip_address.compare(local_ip_address) != 0;
}

bool RA_Connection::check_connection(void) {
  if (!keep_connection_open) {
    if (reconnect())
      return false;
    else {
      Disconnect();
      return true;
    }
  }
  return true;
}

/******************************************************************************
 *
 * reconnect to the host
 *
 *   - Trying to reconnect to remote server
 *   - simply try to execute again Connect() function
 *
 * returns: 0 = success
 *          1 = failure - invalid host name
 *          2 = failure - invalid login
 *          3 = some other (socket) error
 *          4 = server not running error
 *          5 = version mismatch
 *
 */

int RA_Connection::reconnect(void) {
  // reconnect only if host is remote
  // otherwise do nothing
  // check if the host name refers to the local host

  if (!keep_connection_open) {
    connection_type = get_connection_type();
    if (connection_type == RA_NO_CONNECTION) {
      perror("Communication error - could not reconnect "
             "to RAserver\n");
      return 4;
    } else if (connection_type == RA_REMOTE_CONNECTION) {
      int c = Connect(host_name, port_num, login_name, password);
      if (c) {
        perror("Communication error - could not reconnect "
               "to RAserver\n");
      }
      return c;
    } else
      return 0;
  }
  return 0;
}

int RA_Connection::reconnect_and_remain_open(void) {

  if (keep_connection_open)
    std::cerr << "Warning : Trying to reopen an already opened connection"
              << std::endl;
  int res = reconnect();
  if (!res)
    keep_connection_open = true;
  return res;
}

void RA_Connection::Disconnect_remain_open_connection(void) {
  if (!keep_connection_open)
    std::cerr << "Warning : Trying to close an already closed connection"
              << std::endl;
  keep_connection_open = false;
  Disconnect();
}

void RA_Connection::set_keep_connection_open(bool state) {
  keep_connection_open = state;
}

bool RA_Connection::get_keep_connection_open(void) {
  return keep_connection_open;
}
