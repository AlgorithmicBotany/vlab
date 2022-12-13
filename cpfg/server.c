/*  Usage: server3
 *    A server program to demonstrate sockets. It will accept many
 *    clients, and will print messages from both the socket and the keyboard.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifndef _ABI_SOURCE
#ifndef LINUX
#include <bstring.h>
#endif
#endif
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>

#include "server.h"
#include "log.h"

#define problem(msg)                                                           \
  {                                                                            \
    Message("%s\n", msg);                                                      \
    return 0;                                                                  \
  } /* error messages */
#define MAX_CLIENTS 10

struct client_record { /* This structure will track the client(s) */
  int number;
  int sock_desc;
  FILE *sock_fp;
};

static struct client_record client;

static int main_sock;

static char verbose = 0;

/*************************************************************************/
int SetServer(int port) {
  struct sockaddr_in server;
  int count;

  client.sock_desc = -1;

  /* Create a socket */
  if ((main_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    problem("Socket problem");

  /* Name the socket using wildcards */
  memset(&server, 0, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(port);

  if (verbose)
    Message("port %d\n", port);

  /* Set the options of the socket */
  if ((setsockopt(main_sock, SOL_SOCKET, SO_REUSEADDR, &count, sizeof count)) <
      0)
    problem("Setsockopt problem");

  /* Bind the socket to the address */
  if (bind(main_sock, (const struct sockaddr *)&server, sizeof server) < 0)
    problem("Bind problem.");

  /* Only one client can be added to the queue */
  if (verbose)
    Message("Listening for clients...\n");

  listen(main_sock, 5);

  return 1;
}

/*************************************************************************/
/* returns 1 or 0 */
int GetNewClient(int wait) {
  fd_set rmask; /* Mask for the socket and stdin file descriptors */
  int new_sock, width;
  struct timeval timeout = {0, 100}, *tptr; /* seconds and microseconds */

  width = getdtablesize();

  FD_ZERO(&rmask);
  FD_SET(main_sock, &rmask); /* So we can detect new clients */

  tptr = wait ? NULL : &timeout;

  /*  wait till there is data available on the file descriptors */
  if (select(width, &rmask, NULL, NULL, tptr) >= 0) {

    /* Any new clients out there? If so, connect them */
    if (FD_ISSET(main_sock, &rmask)) {
      new_sock = accept(main_sock, (struct sockaddr *)NULL, (socklen_t *)NULL);
      if (new_sock == 1)
        Message("Accept problem. Connection refused");

      /* New client. Set the structure */
      client.sock_desc = new_sock;
      client.sock_fp = fdopen(new_sock, "r");

      if (verbose)
        Message("New client connected\n");

      return 1;
    }
  }
  return 0;
}

/*************************************************************************/
FILE *GetClientsData(void) {
  fd_set rmask; /* Mask for the socket and stdin file descriptors */
  /*int       width;*/

  FD_ZERO(&rmask);
  FD_SET(client.sock_desc, &rmask);

  /*width = getdtablesize();*/

  if (verbose)
    Message("Processin client's data\n");

  return client.sock_fp;
}
