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



#include "connect_to.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>

// Get a host address
static int get_host_address(const char *name, struct in_addr *addr) {
  if (*name == '\0') {
    return 0;
  } else if ((*name >= '0') && (*name <= '9')) { /* IP address */
    addr->s_addr = inet_addr(name);
    if (addr->s_addr == (uint32_t)-1)
      return 0;
  } else { /* Host name */
    struct hostent *blob;

    if (!(blob = gethostbyname(name)))
      return 0;
    bcopy(blob->h_addr, (char *)addr, sizeof(struct in_addr));
  }

  return 1; /* Success */
}

// try to connect to "port" on "host"
// return file descriptor, -1 if failed
int connect_to(const char *host, int port) {
  struct in_addr host_address;
  struct sockaddr_in socket_address;
  int fd;

  if (!get_host_address(host, &host_address)) {
    printf("ERROR in connect_to, socket is -2\n");
    return -2;
  }
  socket_address.sin_addr.s_addr = host_address.s_addr;
  socket_address.sin_family = AF_INET;
  socket_address.sin_port = htons(port);

  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    return -1;
  }

  if (connect(fd, (struct sockaddr *)&socket_address,
              sizeof(struct sockaddr_in)) < 0) {
    perror("connect");
    printf("ERROR in connect_to, socket is -1\n");
    return -1;
  }

  return fd;
}
