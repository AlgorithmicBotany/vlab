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
 * Vlab Daemon
 *
 * This program manages connections between the other programs in the
 * vlab. Each program must register with this server.
 *
 * Earle Lowe 1993-1994
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and its documentation for any purpose, provided that the
 * above copyright notice appears in all copies of this software.
 *
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <pwd.h>

/* Vlab includes */
#include "vlab.h"
#include "commutil.h"
#include "list.h"
#include "socket.h"
#include "queue.h"
#include "getObject.h"
#include "vlabd_msgid.h"
#include "delete_recursive.h"
#include "xmemory.h"
#include "xstring.h"

const int DEBUG = 0;

/*
 * The tokenInfo structure is merely a wrapper around an integer, so
 * we can through this stucture into the generic linked list.
 */
typedef struct tokenInfo {
    int token;
} tokenInfo;

/*
 * This is the information that the communications server needs about
 * each process that is registered with it. Each process must send this
 * information when they register.
 */
typedef struct processInfo {
    int id;
    int socket;
    Queue *s_queue;	/* Queue needed to save socket messages */
    int nextsize;
    char *name;
    char *extra;
    LIST *tokenList;	/* list of tokens the process wants to receive */
} processInfo;

static processInfo *currentProcess = NULL; /* pointer to the current process */
static processInfo *currentObject = NULL; /* pointer to the current object */
static int lock_fd;		// lock file descriptor
static char lock_fname[ 4096];	// locak file name

LIST *processList;

/* function prototypes */
static tokenInfo * FindToken(processInfo *process, int token);
static void RegisterProcess(processInfo *process, char *data);
static void CreateProcess(int socketid);
static void RemoveProcess(processInfo *process);
static int BounceToken(processInfo *from, int token, char *data);
static int ProcessMessage(processInfo *process, char *message);

/* global variables */
int main_socket = -1;
struct cleanup {
    int release_lock;
    int delete_lock_file;
    int delete_vlabd_file;
    int delete_cut_copy_paste_dir;
} cleanup_status = { 0, 0, 0, 0};

/* prototypes */
static void Quit( int);

void LOG( const char * fmt_str, ...)
{
    if( ! DEBUG) return;

    long pid = long( getpid());
    fprintf( stderr, "vlabd[%ld]:", pid);

    va_list ap;
    va_start( ap, fmt_str);
    vfprintf( stderr, fmt_str, ap);
    va_end( ap);

    fprintf( stderr, "\n");
}

int main( void)
{
    LOG( "starting to run");
    for (long i = 0 ; i < 10 ; i ++) {
	pid_t uid = fork ();
	if (uid == -1) {
	    char msg [4096];
	    sprintf (msg, "attempt %ld - fork() failed: %s", i, strerror (errno));
	    LOG (msg);
	    LOG ("    - sleeping for 1 sec...");
	    sleep (1);
	    LOG ("    - waking up");
	} else if (uid == 0) {
	// this is a child - we are ok
	    LOG ("fork() successful");
	    break;
	} else {
	    // parent can exit successfuly
	    exit (0);
	}
    }

    char            fname[STRLEN];
    char          * message;
    fd_set          fdset;
    processInfo   * p;
    FILE          * fp;

    // create a unique lock for vlab - if someone else already has it,
    // return (because vlabd already runs)
    {
      struct passwd *pw = getpwuid(getuid());

      const char *homedir = pw->pw_dir;
      sprintf( lock_fname, "%s/.vlab/tmp/vlab_lock_%ld", homedir, (long) getuid());
	LOG( "creating lock file '%s'\n", lock_fname);

	// open file for writing (creating it if it doesn't exist yet)
	lock_fd = open( lock_fname, O_WRONLY | O_CREAT | O_TRUNC,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
			S_IROTH | S_IWOTH);
	if( lock_fd == -1)
	{
	    LOG( "cannot create lock file.");
	    fprintf( stderr, "vlabd: cannot create lock-file %s.\n",
		     lock_fname);
	    perror( "     : ");
	    exit( -1);
	}
	LOG( "locking the lock-file");

	// try to create a lock on the entire file
	struct flock fs;
	fs.l_type = F_WRLCK;
	fs.l_whence = 0;
	fs.l_start = 0;
	fs.l_len = 0;
	fs.l_pid = 0;
	int res = fcntl( lock_fd, F_SETLK, & fs);
	if( res != 0)
	{
	    // lock cannot be created - make sure it is because some other
	    // process has the lock (this other process better be vlabd)
	    (void) fcntl( lock_fd, F_GETLK, & fs);
	    if( fs.l_type == F_UNLCK)
	    {
		fprintf( stderr, "vlabd: cannot lock file %s\n", lock_fname);
		LOG( "cannot lock file - something wrong with the system");
		Quit( -1);
	    }
	    LOG( "other process already has the lock");
	    fprintf( stderr,
		     "vlabd: process %ld already holds a lock on file %s\n",
		     (long) fs.l_pid, lock_fname );
	    Quit( 0);
	}
    }

    // tell the cleanup procedure to delete the lock file...
    cleanup_status.delete_lock_file = 1;
    cleanup_status.release_lock = 1;

    LOG( "lock created");

    // assign a 'callback' to the 'quit' signal
    (void) signal( SIGQUIT, Quit);

    // initialize the process list
    processList = lst_init();
    LOG( "process list initialized");

    // start the server
    int pnum = 4242;
    while (main_socket < 0)
	main_socket = start_server(pnum++);
    //printf( "server started on port %ld with socked %ld", long( pnum-1), main_socket);
    LOG( "server started on port %ld with socked %ld", long( pnum-1), main_socket);

    /* 
     * Create the file /tmp/vlabd<uid>. The other components get the socket
     * id of the server from this file
     */
    struct passwd *pw = getpwuid(getuid());

    const char *homedir = pw->pw_dir;
    sprintf(fname, "%s/.vlab/tmp/vlabd%ld", homedir, (long) getuid());
    fp = fopen(fname, "w");
    if (fp == NULL)
    {
	fprintf( stderr, "Communications Server could not create file %s",
		 fname);
	Quit( 0);
    }
    fprintf(fp, "%ld,%d", (long) getpid(), pnum-1);
    fclose(fp);

    // tell the cleanup procedure to delete vlabd file when we are done
    cleanup_status.delete_vlabd_file = 1;
    // tell the cleanup procedure to delete cut copy paste dir at the end
    cleanup_status.delete_cut_copy_paste_dir = 1;

    LOG( "port written to file %s", fname);


    // 
    // display the splash screen
    //
    if( ! DEBUG) {
      //Splash is now display at browser opening
	LOG( "splash displayed");
    }

    // loop forever
    while (1)
    {
	FD_ZERO(&fdset);
	FD_SET(main_socket, &fdset);
        int width = main_socket+1;
	for (p = (processInfo *) lst_first(processList);
	     p != NULL;
	     p = (processInfo *) lst_next(p))
	{
	    FD_SET(p->socket, &fdset);
            if(p->socket >= width)
              width = p->socket+1;
	}

        LOG( "waiting for messages");
	// Block while we wait for any messages
        //LOG("width = %d", width);
	select(width, &fdset, NULL, NULL, NULL);

        LOG("There is something to read on the socked.");

	if (FD_ISSET(main_socket, &fdset))
	{
            LOG("Creating a new process");
	    // request for a new connection
	    CreateProcess(main_socket);
            LOG("Process created");
	}
        LOG("Finding out which 'process' can read");
	// find out which 'process' can read
	p = (processInfo *) lst_first( processList);
	while( p != NULL)
	{
	    if ( ! (FD_ISSET(p->socket, &fdset)) )
	    {
		p = (processInfo *) lst_next(p);
		continue;
	    }
            LOG("Found process reading this data: %ld", p->socket);

	    // read data from the socket and put a message into
	    // a message queue
	    if (ReadSocket(p->socket, p->s_queue))
	    {
		int removed = 0;
		processInfo * next_process = (processInfo *) lst_next( p);

		while( 1)
		{
		    if( p == NULL)
			break;
		    message = CheckQueue(p->s_queue);
		    if( message == NULL)
			break;
		    
		    removed = ProcessMessage(p, message);
		    xfree(message);
		    message = NULL;
		    if( removed)
			break;
		}
		p = next_process;
		continue;
	    }
	    else
	    {
		/* 
		 * We couldn't read anything. Most likely this is 
		 * because the client has shut down, so we remove the
		 * process
		 */
		processInfo * next_process = (processInfo *) lst_next( p);
		RemoveProcess(p);
		p = next_process;
		continue;
	    }
	}
    }
}

/*
 * This finds the tokenInfo for the given token (integer) and 
 * process.  Returns NULL if the token is not found.
 *
 * Inputs:	a processInfo structure, and an integer representing a token
 * Returns:	NULL if not found, pointer to tokenInfo if found
 * Modifies:
 */
tokenInfo *FindToken(processInfo *process, int token)
{
    tokenInfo *t = NULL;

    if (process == NULL)
	return NULL;

    for( t = (tokenInfo *) lst_first(process->tokenList);
	 t != NULL;
	 t = (tokenInfo *) lst_next(t))
    {
	if (t->token == token)
	    return t;
    }

    return NULL;
}

/*
 * This creates a new processInfo node, initializes the elements of the
 * structure, and inserts the node into the processList.  Needs some
 * memory error checking.  It initializes the socket connection.
 *
 * Inputs:	the main_socket of the comm_server
 * Modifies:	processList
 */

void CreateProcess(int socketid)
{
    processInfo *newProcess;

    newProcess = (processInfo *) lst_newnode(sizeof(processInfo));
    LOG("created node. Now creating connection from socket %ld", socketid);
    /* create a new socket connection with this process */
    newProcess->socket = new_connection(socketid);
    LOG("created connection");
    /* initialize the socket message Queue */
    newProcess->s_queue = (Queue *) xmalloc(sizeof(Queue));
    newProcess->s_queue->head = newProcess->s_queue->tail = 0;
    newProcess->s_queue->nextsize = -1;
    newProcess->id = -1;
    newProcess->name = NULL;
    newProcess->extra = NULL;
    newProcess->tokenList = NULL;

    /* Insert the new processInfo into the list */
    lst_insertafter(processList, newProcess, LST_HEAD(processList));
    LOG("node inserted");
    currentProcess = newProcess;
}

/*
 * Fills in the values for a processInfo, based on information the process
 * sent. This procedure is called when the server receives a REGISTER
 * message. Should do some malloc error checking.
 *
 * Inputs:	a processInfo structure, REGISTER data
 * Modifies:	currentObject
 */
void RegisterProcess(processInfo *process, char *data)
{
    tokenInfo *newToken;
    char *bufcopy, *t1;
    const char *sep = ",";
    int temp;

    /* The REGISTER data: PID, NAME, ?EXTRA?, ?TOKEN1?, ?TOKEN2?, ... */

    if (data == NULL)
	return;

    bufcopy = (char*) xstrdup(data);

    sscanf(strtok(bufcopy, sep), "%d", &(process->id));	/* PID */
    process->name = (char*) strdup(strtok(NULL, sep));	/* NAME */
    if( DEBUG)
	fprintf( stderr, "vlabd: REGISTER pid=%d name=%s\n",
		 process-> id, process-> name);
    /* Intialize the token list */
    process->tokenList = lst_init();
    if ((t1 = strtok(NULL, sep)) == NULL)
    {
	/* The next field is empty, so no extra information and no tokens */
	return;
    }

    if (sscanf(t1, "%d", &temp) != 1)
    {
	/* 
	 * If the next field isn't an integer, then we have some 
	 * "extra" process information (in the form of a string)
	 */
	process->extra = (char*) strdup(t1);
    }
    else
    {
	/* 
	 * We are looking for tokens. Create a new tokenInfo structure, and 
	 * fill in the value. Then add the new structure to the list.
	 */
	newToken = (tokenInfo *) lst_newnode(sizeof(tokenInfo));
	newToken->token = temp;
	lst_insertafter(process->tokenList, newToken, 
			LST_HEAD(process->tokenList));
    }
    /* 
     * Looking for tokens.  If we can get an integer out of
     * the string, then it is a token for which we are interested in 
     * receiving messages.
     */
    while ((t1 = (strtok(NULL, sep))) != NULL)
    {
	if (sscanf(t1, "%d", &temp) == 1)
	{
	    newToken = (tokenInfo *) lst_newnode(sizeof(tokenInfo));
	    newToken->token = temp;
	    lst_insertafter(process->tokenList, newToken, 
			    LST_HEAD(process->tokenList));
	}
    }

    xfree(bufcopy);
    bufcopy = NULL;

}

/*
 * Delete the given process from the processList.
 *
 * Inputs:	processInfo pointer
 * Modifies:	currentProcess and currentObject
 */
void RemoveProcess(processInfo *process)
{
    processInfo *p;

    /* 
     * We want to remove process from the list, so we have to free up all
     * memory in use by the structure. This includes the token list.
     */

    close_connection(process->socket);	/* close the socket connection */
    process-> socket = -1;
    free(process->s_queue);	/* free the message queue */
    process-> s_queue = NULL;
    free(process->name);	/* free the process name */
    process-> name = NULL;
    if (process->extra)
      free(process->extra);	/* free process extra, if exists */
    process-> extra = NULL;

    if (process->tokenList)	/* free the token list, if exists */
	lst_kill(process->tokenList, lst_freenode);
    process-> tokenList = NULL;

    if (process == lst_first(processList))
    {
	/* removing the first element of the list */
	currentProcess = (processInfo* ) lst_next(process);
	lst_freenode(lst_deletenext(processList, LST_HEAD(processList)));
    } 
    else
    {
	/* 
	 * This is a singly linked list, so we need to find the element
	 * before the one we want to delete, in order to use lst_deletenext.
	 */
	for( p=(processInfo* ) lst_first(processList);
	     lst_next(p) != process;
	     p=(processInfo *) lst_next(p))
	{
	    ; // NOOP
	}

	lst_freenode(lst_deletenext(processList, p));
	/* reset the currentProcess pointer */
	if (currentProcess == process)
	    currentProcess = p;
	if (currentObject == process)
	    currentObject = p;
    }

    /* If there aren't any processes in the list, the server Quits. */
    if (LST_EMPTY(processList))
    {
	LOG( "process list empty - quitting");
	Quit( 0);
	return;
    }

    process = NULL;
}

/*
 * Take the given tokenInfo pointer and bounce the given data to the 
 * components which specified the token when they registered.
 *
 * Inputs:	from process, message token, message data
 * Modifies:	
 * Returns:	the number of messages bounced
 */
int BounceToken(processInfo *, int token, char *data)
{
    processInfo *p;
    int found = 0;

    if (token < 0)
    {
	return found;
    }

    for( p = (processInfo *) lst_first(processList);
	 p != NULL;
	 p = (processInfo *) lst_next(p))
    {
	if (FindToken(p, token))
	{
	    /*
	     * Check to make sure the process is really running, if the
	     * process doesn't really exist, it is removed from the 
	     * process list.
	     */
	    if (kill(p->id, 0) < 0)
	    {
		switch (errno)
		{
		case ESRCH:
		    RemoveProcess(p);
		    break;
		default:
		    break;
		}
	    }
	    else
	    {
		comm_err(SendData(p->socket, token, data));
		found++;
	    }
	}
    }
    return found;
}

/*
 * Process any messages the server receives. The server defaults to 
 * bouncing the message to the approperiate location using the data
 * the sending process has registered
 *
 * Inputs:	the process that sent the message
 *		the message sent
 * Returns:     0 - if the process still exists
 *              1 - if the process exits
 * Modifies:	the variable nextsize for the process is reset to -1
 *
 */
int ProcessMessage(processInfo *process, char *message)
{
    if( DEBUG)
    {
	fprintf( stderr, "vlabd: process message:\n");
	fprintf( stderr, "       from: %s[%d]\n",
		 process-> name, process-> id);
	long i;
	fprintf( stderr, "       ");
	for( i = 0 ; i < process-> s_queue-> nextsize ; i ++)
	{
	    int d = * (unsigned char *) & message[ i];
	    fprintf( stderr, "%x%x", d / 16, d % 16);
	}
	fprintf( stderr, "\n");
	fprintf( stderr, "       ");
	for( i = 0 ; i < process-> s_queue-> nextsize ; i ++)
	{
	    int d = * (unsigned char *) & message[ i];
	    if( d >= 32 && d < 128)
		fprintf( stderr, " %c", char( d));
	    else
		fprintf( stderr, " .");
	}
	fprintf( stderr, "\n");
    }

    char *bufcopy, buf[STRLEN];
    int message_type;

    // One is added to nextsize to get the end-of-string '\0' char
    bufcopy = (char *)xmalloc(process->s_queue->nextsize + 1);
    if (bufcopy == NULL) return 0;
    memcpy(bufcopy, message, process->s_queue->nextsize);
    // Don't forget '\0' character at the end
    bufcopy[process->s_queue->nextsize] = '\0';

    // Get the message type, please forgive the hard numbers
    message_type = (16777216 * (int) (unsigned char) bufcopy[0]) + 
	(65536 * (int) (unsigned char) bufcopy[1]) + 
	(256 * (int) (unsigned char) bufcopy[2]) + 
	(int) (unsigned char) bufcopy[3];

    switch (message_type)
    {

    case REGISTER:
	RegisterProcess(process, bufcopy+4);
	// reset the size of the next message in the queue
	process->s_queue->nextsize = -1;
	break;
    case REMOVE:
	RemoveProcess(process);
	return 1;
	// break;
	
    case PANELSTART:
	if (currentObject)
	    sprintf(buf, "%d,%s", currentObject->id,currentObject->extra);
	else
	    sprintf(buf, "0,None");
	SendData(process->socket, OBJECTINFO, buf);
	// reset the size of the next message in the queue
	process->s_queue->nextsize = -1;
	break;
	
    case OBJCUR:
	currentObject = process;
	// reset the size of the next message in the queue
	process->s_queue->nextsize = -1;
	break;

    case GETOBJECT:
	getObject( (char *) (bufcopy + 4));
	// reset the size of the next message in the queue
	process->s_queue->nextsize = -1;
	break;

    case ACK_REQUEST:
	SendData( process-> socket, ACK_RESPONSE, (char*)"4 0");
	process-> s_queue-> nextsize = -1;
	break;
	
    default:
	/*
	 * Bounce the token to the any registered processes that
	 * want the message. Send NULL with the token if there isn't
	 * any data available with the token.
	 */
	
	if (process->s_queue->nextsize > 4)
	{
	    BounceToken(process, message_type, bufcopy+4);
	}
	else
	{
	    BounceToken(process, message_type, NULL);
	}
	// reset the size of the next message in the queue
	process->s_queue->nextsize = -1;
	break;
    }
    return 0;
}

void Quit( int quit_code)
                                                                            /* 
  Quit the vlab daemon and clean up:
 
    - if this vlabd created /tmp/vlabd<uid> then remove it and the cut/copy/
      paste directory
    - if this vlabd locked the lock file, unlock it
    - if this vlabd created the lock file, remove it
 
									    */
{
    char fname[ 4096];

    if( cleanup_status.delete_vlabd_file)
    {
	// remove the /tmp/vlabd<uid> file
      struct passwd *pw = getpwuid(getuid());

      const char *homedir = pw->pw_dir;
      sprintf(fname, "%s/.vlab/tmp/vlabd%ld", homedir, (long) getuid());
	LOG( "removing port file %s", fname);
	if( access( fname, F_OK) == 0)
	    if(unlink(fname) == -1)
		fprintf(stderr, "something went wrong with the unlink\n");
    }

    if( cleanup_status.delete_cut_copy_paste_dir)
    {
	// remove the cutCopyPaste<uid> directory
      struct passwd *pw = getpwuid(getuid());

      const char *homedir = pw->pw_dir;
      sprintf( fname, "%s/.vlab/tmp/cutCopyPaste%ld", homedir, (long) getuid());
	LOG( "removing cut/copy/paste file %s", fname);
	(void) delete_recursive( fname);
	if( access( fname, F_OK) == 0)
	{
	    fprintf( stderr, "vlabd: Could not remove '%s'.\n", fname);
	}
    }

    if( cleanup_status.release_lock)
    {
	// remove the lock and the lock file
	struct flock fs;
	fs.l_type = F_UNLCK;
	fs.l_whence = 0;
	fs.l_start = 0;
	fs.l_len = 0;
	fs.l_pid = 0;
	(void) fcntl( lock_fd, F_SETLK, & fs);
	(void) close( lock_fd);
    }

    if( cleanup_status.delete_lock_file)
    {
	(void) unlink( lock_fname);
	if( access( lock_fname, F_OK) == 0)
	{
	    LOG( "lock file %s not removed", lock_fname);
	    fprintf( stderr, "vlabd: lock file %s not removed: ", lock_fname);
	    perror( "");
	}
	else
	{
	    LOG( "lock file removed");
	}
    }
    
    if( main_socket != -1)
	close_connection(main_socket);	
    LOG( "main socket shut down");

    LOG( "exit( %d)", quit_code);
    exit( quit_code);
}


