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




/*
 * Some various socket functions
 * Taken from some code by Andrew Brownbill
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>

#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <netdb.h>

/* Vlab includes */
#include "vlab.h"
#include "queue.h"
#include "socket.h"

int start_server(int port)
{
    int sock,opt;
    struct sockaddr_in address;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 1) {
        return sock;
    }

    opt=1;

    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,(char *) &opt, 
        sizeof(opt));

    address.sin_port = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_family = AF_INET;

    if (bind(sock, (struct sockaddr *) &address, sizeof(address)) < 0) {
        return -1;
    }
   
    /* It should be noted here that the magic number 5 is taken from SUN's
    man pages. Do a man listen if you want to see why :-) */

    if (listen(sock, 5) < 0) {
        perror("listen");
        exit(1);
    }

    return sock;

}

/*
attempt to accept a connection on "fd"
*/

int new_connection(int fd)
{
    int newfd;

    newfd = accept(fd, NULL,NULL);


    if (newfd < 0) {
        printf("errno = %d\n", errno);
        perror("accept");
        return 0;
    }

    return newfd;

}

void close_connection(int fd)
{
    shutdown(fd, 2);
    close(fd);

}

/*
 * Read some information from a socket, and place it into a queue. Zero (0)
 * is returned if nothing is read.
 *
 * Inputs:	a socket to read from
 *		a pointer to a queue
 *
 * Modifies:	the queue
 *
 * Returns:	0 if no bytes are read, 1 otherwise
 *
 */
int ReadSocket(int socketid, Queue *queue)
{
    unsigned char buf[1024];
    int read_size = 0;

    read_size = recv(socketid, buf, 1024, 0);

    if (read_size == 0)
    {
	return 0;
    }

    insert(queue, (char *) buf, read_size);

    return 1;
}

/*
 * Check to see if there is a complete message on the given queue.  The first
 * four bytes represent the size of the message.  Four bytes are popped off
 * the queue and the size of the message is put into nextsize.  If there
 * aren't four bytes on the queue, and there is no message pending 
 * (nextsize == -1), we return immediately.
 *
 * If there isn't a complete message in the queue, then we push the four
 * bytes read earlier, and NULL is returned.  If a complete message is
 * in the queue, a pointer to the data is returned
 *
 * Inputs:	a pointer to a queue
 *		a pointer to nextsize
 *
 * Returns:	a pointer to the message or NULL
 *
 * Modifies:	the queue, and nextsize
 *
 */

char *CheckQueue(Queue *queue)
{
    unsigned char *tempsize;

    if ((size(queue) >= 4) && (queue->nextsize == -1))
    {
	tempsize = (unsigned char *) pop(queue, 4);
	queue->nextsize =
	    tempsize[ 0] * 256 * 256 * 256 +
	    tempsize[ 1] * 256 * 256 +
	    tempsize[ 2] * 256 +
	    tempsize[ 3] ;

	free(tempsize);
    }
    else if (size(queue) < 4)
	return NULL;

    if (size(queue)  >= queue->nextsize)
    {
	return (pop(queue, queue->nextsize));
    } 
    else 
    {
	return NULL;
    }
}

