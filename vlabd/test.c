#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdarg.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>

void LOG( const char * fmt_str, ...)
{
    if( ! DEBUG) return;

    long pid = long( getpid());
    fprintf( stderr, "test[%ld]:", pid);

    va_list ap;
    va_start( ap, fmt_str);
    vfprintf( stderr, fmt_str, ap);
    va_end( ap);

    fprintf( stderr, "\n");
}

void Quit( int r)
{
    LOG( "quitting with code %d\n", r);
    exit( r);
}

void Pause( int amount)
{
    LOG( "Pausing for %d seconds...", amount);
    for( int cnt = amount ; cnt > 0 ; cnt --)
    {
	LOG( "     %d remaining", cnt);
	sleep( 1);
    }
    LOG( "Continuing...");
}


int main( int, char **)
{
    char lock_fname[ 4096];
    struct passwd *pw = getpwuid(getuid());
    
    const char *homedir = pw->pw_dir;

    sprintf( lock_fname, "%s/.vlab/tmp/vlab_lock_%ld", homedir, (long) getuid());
    LOG( "creating lock file '%s'", lock_fname);

    // create a file-lock (fail if it already exists)
    int lock_fd = open( lock_fname, O_WRONLY | O_CREAT | O_TRUNC, // | O_EXCL,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
			S_IROTH | S_IWOTH);
    if( lock_fd == -1)
    {
	LOG( "cannot create lock file: %s", strerror( errno));

	LOG( "trying to open lock file");
	// try to open the existing file
	lock_fd = open( lock_fname, O_WRONLY);
	if( lock_fd == -1)
	{
	    LOG( "cannot open lock file: %s", strerror( errno));
	    Quit( -1);
	}
    }
    
    Pause( 5);

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
	    LOG( "cannot lock file - something wrong with the system");
	    Quit( -1);
	}
	LOG( "process %ld already holds a lock on file %s\n",
	     (long) fs.l_pid, lock_fname );
	Quit( -1);
    }
    LOG( "lock created - quitting");

    Pause( 10);

    return 0;
}
