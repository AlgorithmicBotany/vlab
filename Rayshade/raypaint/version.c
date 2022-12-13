/*
 * version.c
 *
 * Copyright (C) 1989, 1991, Craig E. Kolb
 * All rights reserved.
 *
 * This software may be freely copied, modified, and redistributed
 * provided that this copyright notice is preserved on all copies.
 *
 * You may not distribute this software, in whole or in part, as part of
 * any commercial product without the express consent of the authors.
 *
 * There is no warranty or other guarantee of fitness of this software
 * for any purpose.  It is provided solely "as is".
 *
 * $Id: version.c,v 1.1.1.1 2005/01/07 20:51:06 laneb Exp $
 *
 * $Log: version.c,v $
 * Revision 1.1.1.1  2005/01/07 20:51:06  laneb
 * Initial import, straight port to Windows/Linux
 *
 * Revision 4.0  91/07/17  17:37:17  kolb
 * Initial version.
 * 
 */
#include "rayshade.h"
#include "patchlevel.h"
#include "stats.h"

void
VersionPrint()
{
	extern char rcsid[];

	fprintf(Stats.fstats,
		"raypaint: %s\nPatch level %d\n", rcsid, PATCHLEVEL);
}
