/*
 * error.h
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
 *
 */
#ifndef ERROR_H
#define ERROR_H
/*
 * Error severity codes, passed to user-provided RLerror()
 * function which optionally prints and optionally exits.
 *
 * RL_ADVISE	Message may safely be safely suppressed, though
 *		the resulting image may not be exactly what you expect.
 * RL_WARN	Message should probably be printed; image will most
 *		likely be affected.
 * RL_ABORT	Message should be printed -- couldn't perform a request.
 *		The resulting image will be affected.
 * RL_PANIC	Fatal error -- call to RLerror() must not return.
 */
#define RL_ADVISE 0 /* Advisory */
#define RL_WARN 1   /* Warning */
#define RL_ABORT 2  /* Aborted */
#define RL_PANIC 3  /* Panic */

extern void RLerror(); /* application-supplied reporting routine */
#endif                 /* ERROR_H */
