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



#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "MessageQueue.h"
#include "xmemory.h"

//////////////////////////////////////////////////////////////////////////////
//
// constructor
//

MessageQueue::MessageQueue() {
  nMessages = 0;
  messageTableSize = 10; // room for 10 messages by default
  message = (Message **)xmalloc(sizeof(Message *) * messageTableSize);
  assert(message != NULL);
}

//////////////////////////////////////////////////////////////////////////////
//
// destructor
//

MessageQueue::~MessageQueue() {
  // first free all of the messages in the queue (table)
  for (size_t i = 0; i < nMessages; i++)
    delete message[i];

  // now delete the actual message table
  if (message != NULL)
    xfree(message);
}

//////////////////////////////////////////////////////////////////////////////
//
// add a message into the queue
//

void MessageQueue::addMessage(Message *m) {
  if (nMessages == messageTableSize) {
    messageTableSize += 10;
    message =
        (Message **)xrealloc(message, sizeof(Message *) * messageTableSize);
    assert(message != NULL);
  }

  message[nMessages] = m;
  nMessages += 1;
}

//////////////////////////////////////////////////////////////////////////////
//
// finds out whether there is a message with the given code
// in the message queue
//
// on success, a pointer to this message is returned, otherwise
// NULL is returned
//

Message *MessageQueue::findAndRemove(Code c) {
  for (size_t i = 0; i < nMessages; i++) {
    if (message[i]->code == c) {
      // found it
      Message *result = message[i];
      // now delete it by shiftin all the other messages up
      for (size_t j = i + 1; j < nMessages; j++)
        message[j - 1] = message[j];
      // update the number of messages in the queue
      nMessages -= 1;
      // return the result
      return result;
    }
  }

  // not found
  return NULL;
}

//////////////////////////////////////////////////////////////////////////////
//
// returns number of messages on the message queue
//

long MessageQueue::getNumOfMessages(void) { return nMessages; }

//////////////////////////////////////////////////////////////////////////////
//
// returns the first message on the queue
//

Message *MessageQueue::getFirstMessage(void) {
  if (nMessages == 0)
    return NULL;

  Message *result;

  // return the first message on the queue
  result = message[0];
  // shift the rest of the queue upwards
  for (size_t i = 1; i < nMessages; i++)
    message[i - 1] = message[i];
  // adjust the number of messages on the queue
  nMessages -= 1;

  return result;
}
