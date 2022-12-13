#ifndef __DOS2UNIX_H
#define __DOS2UNIX_H

/*
 *  Copyright (C) 2009 Erwin Waterlander
 *  Copyright (C) 1994, 1995 Benjamin Lin.
 *  All rights reserved.
 *
 *  This file is distributed under the same license as the dos2unix package.
 */

#ifdef __GNUC__
#ifndef strcmpi
#define strcmpi(s1, s2) strcasecmp(s1, s2)
#endif
#endif

#define VER_REVISION "5.1"
#define VER_DATE "2010-04-03"

int dos2unix(int argc, char *argv[]);
int unix2dos(int argc, char *argv[]);

#ifdef ENABLE_NLS

#include <libintl.h>
#define _(String) gettext(String)
#define gettext_noop(String) String
#define N_(String) gettext_noop(String)

#else

#define _(String) (String)
#define N_(String) String
#define textdomain(Domain)
#define bindtextdomain(Package, Directory)

#endif

#endif
