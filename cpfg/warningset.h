/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#ifndef WIN32
#error File to be included only in MS Windows version
#endif

#ifndef __WARNINGSET_H__
#define __WARNINGSET_H__

#pragma warning(                                                               \
    disable : 4201) /* nonstandard extension used : nameless struct/union */
#pragma warning(disable : 4214) /* nonstandard extension used : bit field      \
                                   types other than int */
#pragma warning(disable : 4514) /* 'function' : unreferenced inline function   \
                                   has been removed */

#pragma warning(disable : 4100) /* 'name' : unreferenced formal parameter */

#ifndef YYBISON

#endif

#pragma warning(                                                               \
    error : 4002) /* too many actual parameters for macro 'identifier' */
#pragma warning(                                                               \
    error : 4003) /* not enough actual parameters for macro 'identifier' */
#pragma warning(error : 4005) /* macro redefinition */
#pragma warning(                                                               \
    error : 4013) /* 'function' undefined; assuming extern returning int */
#pragma warning(error : 4020) /* 'function' : too many actual parameters */
#pragma warning(error : 4021) /* 'function' : too few actual parameters */
#pragma warning(                                                               \
    error : 4028) /* formal parameter 'number' different from declaration */
#pragma warning(error : 4035) /* 'function' : no return value */
#pragma warning(                                                               \
    error : 4087) /* 'function' : declared with 'void' parameter list */
#pragma warning(                                                               \
    error : 4090) /* 'operation' : different 'modifier' qualifiers */
#pragma warning(error : 4101) /* 'name' : unreferenced local variable */
#pragma warning(error : 4189) /* 'name' : local variable is initialized but    \
                                 not referenced */
#pragma warning(                                                               \
    error : 4210) /* nonstandard extension used : function given file scope */
#pragma warning(                                                               \
    error : 4305) /* 'identifier' : truncation from 'type1' to 'type2' */
#pragma warning(error : 4706) /* assignment within conditional expression */
#pragma warning(error : 4716) /* 'function' must return a value */
#pragma warning(error : 4761) /* integral size mismatch in argument :          \
                                 conversion supplied */

/*#define MILD*/

#ifdef MILD
#pragma warning(disable : 4244) /* 'conversion' conversion from 'type1' to     \
                                   'type2', possible loss of data */
#pragma warning(disable : 4701) /* local variable 'name' may be used without   \
                                   having been initialized */
#endif

#else
#error File already included
#endif
