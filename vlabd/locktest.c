#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


void usage( void);

int main( int argc, char ** argv)
{
    // get the filename
    if( argc != 2) usage();
    char * fname = argv[1];

    // try to create a file for writing
    int fd = open( fname, O_WRONLY | O_CREAT | O_TRUNC,
		   S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if( fd == -1)
    {
	perror( "open( WRONLY) failed with err. code:");
	exit( 0);
    }

    fprintf( stderr, "open(WRONLY) ok\n");

    // try to create a lock
    while( 1)
    {
	struct flock fs = {
	    F_WRLCK,
	    0, 0, 0,
	    0, 0 };
	int res = fcntl( fd, F_SETLK, & fs);
	fprintf( stderr, "fcntl -> %d\n", res);
	if( res == 0) break;
	perror( "fcntl( SETLK) failed:");
	sleep( 1);
    }

    // lock created
    fprintf( stderr, "fcntl( SETLK) ok.\n");

    // now sleep forever
    while( 1)
    {
	sleep( 1);
    }

    return 0;
}

void usage( void)
{
    fprintf( stderr, "Usage: lock fname\n");
    exit( 0);
}
