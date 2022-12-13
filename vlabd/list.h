/****************************************************************************
*
*					Copyright (C) 1991 Kendall Bennett.
*							All rights reserved.
*
* Filename:		$RCSfile: list.h,v $
* Version:		$Revision: 1.1.1.1 $
*
* Language:		ANSI C
* Environment:	any
*
* Description:	Header file for linked list routines.
*
* $Id: list.h,v 1.1.1.1 2005/07/04 21:16:11 federl Exp $
*
* Revision History:
* -----------------
*
* $Log: list.h,v $
* Revision 1.1.1.1  2005/07/04 21:16:11  federl
* Imported vlab-for-mac port so that Colin and Radek can continue working on bug-fixes
*
* Revision 1.1.1.1  2001/08/08 17:05:31  jxiao
* Imported using TkCVS
*
* Revision 1.1.1.1  2001/06/14 16:23:12  jxiao
* Imported using TkCVS
*
* Revision 1.1  2001/06/12 21:53:11  jxiao
* the vlab package
*
* Revision 1.1.1.1  1998/07/31 20:42:49  federl
* Global VLAB
*
*
* Revision 1.7  91/12/31  19:41:16  kjb
* 
* Modified include files directories.
* 
* Revision 1.6  91/09/27  03:11:04  kjb
* Added compatibility with C++.
* 
* Revision 1.5  91/09/26  10:07:42  kjb
* Took out extern references
* 
* Revision 1.4  91/09/01  17:18:24  ROOT_DOS
* Changed prototype for lst_deletenext().
* 
* Revision 1.3  91/09/01  15:15:46  ROOT_DOS
* Changed search for include files to include current directory
* 
* Added function lst_kill().
* 
* Revision 1.2  91/08/22  11:06:50  ROOT_DOS
* Header file for corresponding revision of source module
* 
* Revision 1.1  91/08/21  14:11:39  ROOT_DOS
* Initial revision
* 
****************************************************************************/

#ifndef	__LIST_H
#define	__LIST_H

#ifndef	__DEBUG_H
#endif

/*---------------------- Macros and type definitions ----------------------*/

typedef struct LST_BUCKET {
    struct LST_BUCKET	*next;
    } LST_BUCKET;

typedef struct {
    int		count;	/* Number of elements currently in list	*/
    LST_BUCKET	*head;	/* Pointer to head element of list */
    LST_BUCKET	*z;	/* Pointer to last node of list	*/
    LST_BUCKET	hz[2];	/* Space for head and z nodes */
    } LIST;

/* Return a pointer to the user space given the address of the header of
 * a node.
 */

#define	LST_USERSPACE(h)	((void*)((LST_BUCKET*)(h) + 1))

/* Return a pointer to the header of a node, given the address of the
 * user space.
 */

#define	LST_HEADER(n)		((LST_BUCKET*)(n) - 1)

/* Return a pointer to the user space of the list's head node. This user
 * space does not actually exist, but it is useful to be able to address
 * it to enable insertion at the start of the list.
 */

#define	LST_HEAD(l)			LST_USERSPACE((l)->head)

/* Determine if a list is empty
 */

#define	LST_EMPTY(l)		((l)->count == 0)

/*-------------------------- Function Prototypes --------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

void *lst_newnode(int size);
void lst_freenode(void *node);
LIST *lst_init(void);
void lst_kill(LIST *l,void (*freeNode)( void * node));
void lst_insertafter(LIST *l,void *node,void *after);
void *lst_deletenext(LIST *l,void *node);
void *lst_first(LIST *l);
void *lst_next(void *prev);
void lst_mergesort(LIST *l,int (*cmp_func)());

#ifdef __cplusplus
}
#endif

#endif
