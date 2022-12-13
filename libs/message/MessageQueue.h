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




#ifndef __MESSAGE_QUEUE_H__
#define __MESSAGE_QUEUE_H__

#include "Message.h"

class MessageQueue
{

public:

    /* constructor */           MessageQueue ();
    /* destructor */           ~MessageQueue ();
    
    // adds a message to the queue
    void                          addMessage ( Message * m);

    // finds a message in the queue according to the code
    Message *                  findAndRemove ( Code c);

    // returns the number of messages on the queue
    long                    getNumOfMessages ( void);

    // return the oldest (first) message on the queue
    Message *                getFirstMessage ( void);

private:

    Message ** message;		// array of pointers to the stored messages
    size_t     nMessages;	// number of messages in the queue
    size_t     messageTableSize; // number of places for messages allocated
};

#endif
