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




#ifndef __MESSAGE_PIPE_H__
#define __MESSAGE_PIPE_H__

#include "MessageQueue.h"

class MessagePipe
{

public:

    /* constructor */         MessagePipe ( int sock);
    /* destructor */         ~MessagePipe ( );

    int                      send_message ( const Message & message);
    Message *           get_first_message ( void);
    Message *                 get_message ( Code c);

private:

    int                              sock ;
    MessageQueue             messageQueue ;
    char unsigned *                buffer ;
    long                         buffSize ;
    long                           nBytes ;

    int                  receive_messages ( void);
};

#endif

