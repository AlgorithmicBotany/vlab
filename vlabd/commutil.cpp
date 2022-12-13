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




#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <pwd.h>

/* Vlab includes */
#include "vlab.h"
#include "commutil.h"
#include "xmemory.h"

/* some local variables */
static int commID = 0;
static int commPort = -1;

/*
 * Send the given data and type through the given socket.  The size of the
 * message is calculated, and that is the first four bytes that are sent
 * through the socket.  Next the type of message is sent, then the data 
 * of the message.  Not all message types require data.
 *
 * I think the send blocks if it can not send right away on the queue.  In
 * my testing, this has not been a problem, however, it could be in the
 * future.
 *
 * Inputs:	a socket id
 *		an integer message type
 *		a pointer (can be NULL) to a string of data
 *
 * Returns: some error code (< 0) on failure, or 1 if successful
 *
 * Modifies: Some data will be put on the socket
 *
 */
int SendData(int socketid, int type, char *data)
{
    int i, size;

    // determine the size of the data to be sent over the socket
    if (data == NULL)
	size = 4;
    else
	size = strlen(data) + 4;

    // in buff we prepare the complete data to be sent
    unsigned char * buff = (unsigned char *) xmalloc( size + 4);
    assert( buff != NULL);
    
    // make sure that the vlab daemon is still running
    if (kill(GetCommID(), 0) == -1) {
	    return NO_COMMSERVER;
    }

    // put the size of the message at the beginning of the message
    buff[ 0] = (size >> 24) & 255;
    buff[ 1] = (size >> 16) & 255;
    buff[ 2] = (size >>  8) & 255;
    buff[ 3] = (size >>  0) & 255;
    
    // now add the type
    buff[ 4] = (type >> 24) & 255;
    buff[ 5] = (type >> 16) & 255;
    buff[ 6] = (type >>  8) & 255;
    buff[ 7] = (type >>  0) & 255;

    // now the actual message
    if( data != NULL)
	memcpy( buff + 8, data, strlen( data));

    // and send the data
    i = send(socketid, buff, size + 4, 0);
    if (i < 0) {
	switch (errno) {
	    case ENOTSOCK: return INVALID_QUEUE;
	    case EMSGSIZE: return MSG_WRITE;
	    default: return MSG_WRITE;
	}
    }

    assert( i == size + 4);

    xfree( buff);
	
    return 1;
}

/*
 * Start the communication server and return the port number that it is
 * listening on.  If the server is already running then we retrieve the
 * port number from the file /tmp/vlabd<uid>.  
 * If the server is not running than it is exec'ed.
 *
 * Returns: -1 on failure, 1 otherwise
 *
 * Modifies: Potentially creates a file called /tmp/vlabd<uid>
 *
 */

int startCommServer(void)
{
    char fname[STRLEN];
    char command[ 4096];
    char * commServerBin;
    struct passwd *pw = getpwuid(getuid());
    
    const char *homedir = pw->pw_dir;
    printf("homedir: %s\n",homedir);
    sprintf(fname, "%s/.vlab/tmp/vlabd%d",homedir, (int) getuid());
    if (access(fname, 00) == 0) {
	/* read the port and pid of the server */
	GetCommInfoFromFile();
	if (kill(commID, 0) != -1) {
	    /* communications server is running */
	    return commPort;
	}
    }
    /* start the server */
    commServerBin = getenv( "VLABDAEMONBIN");
    if( commServerBin == NULL)
    {
	fprintf( stderr, "Your VLABDAEMONBIN is not set.\n");
	fprintf( stderr, "Using 'vlabd' in your path instead.\n");
	commServerBin = (char*)"vlabd";
    }
    sprintf( command, "%s &", commServerBin);
    if( system( command) != 0)
    {
	fprintf( stderr, "Could not execute '%s'.\n", commServerBin);
	return( -1);
    }
    GetCommInfoFromFile();

    return commPort;
}

/*
 * Get the pid and port information about the communications server from
 * the /tmp/vlabd<uid> file.
 *
 * Modifies: the static globals commPort and commID are initialized
 *
 */
void 
GetCommInfoFromFile(void)
{
    FILE           *fp = NULL;
    char fname[1000];
//    char temp[STRLEN];
    int i;
    struct passwd *pw = getpwuid(getuid());
    
    const char *homedir = pw->pw_dir;
    sprintf(fname, "%s/.vlab/tmp/vlabd%ld", homedir, (long) getuid());
    /* 
     * Do a stupid little timeout type of thing.
     * This is needed in case we exec'ed the server, but it hasn't written
     * the file by the time we get here. So we wait for awhile.  We don't
     * want to wait forever, in case the server never manages to create
     * the file.
     */
    
    // try 10 times (with 2 sec intervals)
    for( i = 0 ; i < 10 ; i ++)
    {
	fp = fopen( fname, "r");
	if( fp != NULL)
	    break;
	sleep( 2);
    }
    if( fp == NULL)
    {
	fprintf( stderr, "Could not open file: %s\n", fname);
	fprintf( stderr, "Exiting.\n");
	exit( -1);
    }
	


    // read the information out of the file (try 10 times, with 2 secs
    // intervals)

    for( i = 0 ; i < 10 ; i ++)
    {
	if( 2 == fscanf(fp, "%d,%d", & commID, & commPort))
	    break;

	// 2 numbers could not be read in, so reopen the file in
	// two seconds and try again
	fclose( fp);
	sleep( 2);
	fp = fopen( fname, "r");
	if( fp == NULL)
	{
	    fprintf( stderr, "Could not open file: %s\n", fname);
	    fprintf( stderr, "Exiting.\n");
	    exit( -1);
	}
    }



    fclose(fp);
    return;
}

/*
 * Checks the static global commID, and returns it value.  It commID has not
 * been set (i.e == 0), we look in the /tmp/vlabd<uid> file for it's value/
 *
 * Returns: static global commID
 *
 */
int GetCommID(void)
{
    if (commID == 0)
	GetCommInfoFromFile();
    return commID;
}

/*
 * Checks the static global commPort, and returns it value.  It commID has not
 * been set (i.e == -1), we look in the /tmp/vlabd<uid> file for it's value/
 *
 * Returns: static global commPort
 *
 */
int GetCommPort(void)
{
    if (commPort == -1)
	GetCommInfoFromFile();
    return commPort;
}

/* I don't think this is ever called */
int 
QuitComm(void)
{
    int             i = 0;
    if ((i = GetCommID()) != 0) {
	if (kill(i, SIGQUIT) == -1)
	    return INVALID_COMMSERVER;
	return 1;
    }
    return NO_COMMSERVER;
}

/*
 * little error function that checks the error code returned from the 
 * SendData function.
 *
 */
void comm_err(int err)
{
    switch (err) {
	case INVALID_QUEUE:
	    fprintf( stderr, "Something is wrong with the socket connection");
	    break;
	case MSG_WRITE:
	    fprintf( stderr, "Had trouble writing on the socket");
	    break;
	case INVALID_COMMSERVER:
	    fprintf( stderr, "Could not send message to comm server, it must have crashed.  Recommend closing everything and starting again");
	    break;
	case NO_COMMSERVER:
	    fprintf( stderr, "Communications server is not running.  Recommend closing everything and starting again.");
	    break;
	default:
	    break;
    }
}
