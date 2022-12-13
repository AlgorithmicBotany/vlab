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
 * queue.c
 * FIFO queue functions.  This is a fairly braindead implementation.
 */ 

/* includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Vlab includes */
#include "vlab.h"
#include "queue.h"
#include "xmemory.h"

/*
 * insert() inserts the given data into the end of the queue.  Only 
 * number of bytes are inserted.
 *
 * Inputs:	queue into which to insert the bytes
 * 		some data to insert
 * 		the number of bytes to insert into the queue
 *
 * Modifies:	some bytes are read onto the queue
 *
 */
void insert(Queue *queue, char *data, int number)
{
    int i;

    if ((queue == NULL) || (data == NULL) || (number <= 0))
	return;
    for (i = 0; i < number; i++) {
	if (queue->tail > STRLEN)
	    queue->tail = 0;
	/* probably should have dynamic queues */
	if (queue->tail == queue->head - 1) {
	    fprintf(stderr, "MAJOR ERROR, QUEUE IS FULL\n");
	    exit(-1);
	}
	queue->buf[queue->tail++] = data[i];
    }
}

/*
 * push() pushes the given data back onto the front of the queue.  This is
 * called when we read some data of the queue, but it does not form 
 * a complete message, so we place the data back on the queue.
 *
 * !!!! I don't think the sizeof() stuff works !!!!
 *
 * Inputs:	Queue to push data onto
 * 		data to push
 *
 * Modifies:	the queue of course
 *
 */
void push(Queue *queue, char *data)
{
    char *temp = NULL;

    if ((queue == NULL) || (data == NULL))
	return;
    temp = pop(queue, size(queue));
    insert(queue, data, sizeof(&data));
    if (temp != NULL)
	insert(queue, temp, sizeof(&temp));
    xfree(temp);
}

/*
 * pop() removes the given number of bytes of the queue. pop() returns a
 * char * to a newly allocated piece of memory holding the data.
 *
 * Inputs:	Queue from which to remove data
 * 		The number of bytes to remove
 *
 * Returns:	a pointer to some memory holding the popped off data
 *
 * Modifies:	some bytes are removed from the queue
 *
 */
char *pop(Queue *queue, int number) 
{
    char *buf = NULL;
    int i;

    if ((number <= 0) || (queue == NULL))
	return NULL;
    buf = (char *) xmalloc(number);
    if (buf == NULL)
	return NULL;
    for (i = 0; i < number; i++) {
	if (queue->head == STRLEN) 
	    queue->head = 0;
	if (queue->head == queue->tail)
	    return buf;
	buf[i] = queue->buf[queue->head++];
    }
    return buf;
}

/*
 * size() returns the size of the given queue.  This is probably pretty
 * ineffiecent as a function.
 *
 * Inputs:	Queue to calculate the size for.
 *
 * Returns:	the size of the queue.
 *
 */
int size(Queue *queue)
{
    if (queue->tail < queue->head)
	return (STRLEN - queue->head + queue->tail + 1);
    else
	return (queue->tail - queue->head);
}
