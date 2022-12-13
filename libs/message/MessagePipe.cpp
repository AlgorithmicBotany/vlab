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



#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "MessagePipe.h"
#include "debug.h"
#include "xmemory.h"
#include "xstring.h"

/******************************************************************************
 *
 * Constructor for MessagePipe
 *
 */

MessagePipe::MessagePipe(int new_sock) {

  // set up the incoming buffer
  buffSize = 4096; // initial size
  buffer = (unsigned char *)xmalloc(buffSize);
  nBytes = 0; // no data in the buffer yet

  sock = new_sock;
}

/******************************************************************************
 *
 * Destructor for MessagePipe
 *
 */

MessagePipe::~MessagePipe() {
  if (buffer != NULL)
    xfree(buffer);
}

/******************************************************************************
 *
 * int MessagePipe::send_message( Message & msg)
 *
 * - sends a message through the socket
 *
 * - returns:  0 - success
 *            !0 - error
 *
 */

int MessagePipe::send_message(const Message &msg) {
  // prepare the total length of the information to be transmitted
  long datasize = 4 /*sizeof code*/ + 4 /*sizeof length*/ + msg.length;

  // allocate room for the data in buffer
  unsigned char *buf = (unsigned char *)xmalloc(datasize);

  // put the code of the message into the buffer (assuming code has at
  // most 4 bytes)
  buf[0] = (msg.code / (1)) % 256;
  buf[1] = (msg.code / (256)) % 256;
  buf[2] = (msg.code / (256 * 256)) % 256;
  buf[3] = (msg.code / (256 * 256 * 256)) % 256;

  // put the code of the length into the buffer (assuming the length
  // has at most 4 bytes)
  buf[4] = (msg.length / (1)) % 256;
  buf[5] = (msg.length / (256)) % 256;
  buf[6] = (msg.length / (256 * 256)) % 256;
  buf[7] = (msg.length / (256 * 256 * 256)) % 256;

  // copy the actual data into the buffer
  if (msg.length > 0)
    memcpy(buf + 8, msg.data, msg.length);

  // now output the code onto the socket
  long nWritten = 0;
  unsigned char *ptr = buf;
  while (nWritten < datasize) {
    int n = write(sock, ptr, datasize - nWritten);


    if (n == -1) {
      // problem with writing the data to the socket
      fprintf(stderr, "MessageQueue:send_message():write() error\n");
      xfree(buf);
      return 1;
    }
    nWritten += n;
    ptr = ptr + n;
  }

  xfree(buf);
  return 0;
}

/******************************************************************************
 *
 * Message * MessagePipe::get_message( Code code)
 *
 * - will receive a message with code 'code' and return it
 * - if the incoming message has a diferent code, it is stored in a queue
 *   and then we wait for a next message (which could also be stored in
 *   the queue)
 * - it first tries to retrieve the message from the queue, and if it is not
 *   in the queue, it tries to refill the queue with the incoming messages
 *
 * - returns:      - pointer to the message
 *            NULL - error
 *
 */

Message *MessagePipe::get_message(Code code) {
  Message *message;

  while (1) {
    // try to find the message in the queue
    message = messageQueue.findAndRemove(code);
    if (message != NULL)
      break;
    // it is not in the queue, so try to refill the queue
    if (receive_messages()) {
      return NULL;
    }
  }

  return message;
}

/******************************************************************************
 *
 * will return the first message on the queue
 *
 * - returns:   pointer to the message
 *              or NULL in case of an error
 *
 */

Message *MessagePipe::get_first_message(void) {
  while (1) {
    Message *m = messageQueue.getFirstMessage();
    if (m != NULL)
      return m;
    // there are no messages on the queue, so we better receieve
    // some messages
    if (receive_messages()) {
      return 0;
    }
  }
}

/******************************************************************************
 *
 * int MessagePipe::receive_messages( void)
 *
 * - reads a message from the socket and puts it in the message queue
 *
 * - returns:   0 = success
 *             !0 = error
 *
 */

int MessagePipe::receive_messages(void) {
  int done = 0;
  Code c = 0;
  long l = 0;

  while (!done) {
    // read incoming data from the socket
    // unsigned char newData[8192];
    unsigned char newData[4096];
    // we need to check if the socket is stil alive
    int count = read(sock, newData, sizeof(newData));

    // check for errors
    if (count <= 0) {
      fprintf(stderr, "MessagePipe:receive_messages():read(): \n"
                      "              - the other side is not responding\n");
      close(sock);
      sock = -1;
      return 1;
    }

    // now add the data into the data that we already have stored
    if (count + nBytes > buffSize) {
      // we need larger buffer
      buffSize = count + nBytes;
      buffer = (unsigned char *)xrealloc(buffer, buffSize);
    }

    // append the new data at the end of the buffer
    memcpy(buffer + nBytes, newData, count);
    nBytes += count;

    // extract all messages from the raw data
    while (nBytes >= 8) {

      // extract the code of the message from the buffer
      c = buffer[0] + buffer[1] * 256 + buffer[2] * 256 * 256 +
          buffer[3] * 256 * 256 * 256;

      //	  debug_printf("c : %d\n",c);

      // extract the length of data
      l = buffer[4] + buffer[5] * 256 + buffer[6] * 256 * 256 +
          buffer[7] * 256 * 256 * 256;

      // now we have the length of the message. Do we have
      // that much data in the buffer?
      if (nBytes - 8 < l) {
        break;
      }

      // create and add a message to the message queue
      messageQueue.addMessage(new Message(c, (char *)buffer + 8, l));

      // shift the buffer to delete the current message
      memmove(buffer, buffer + 8 + l, nBytes - 8 - l);
      nBytes -= 8 + l;

      // and set a flag that we have at least one message
      done = 1;
    } // while( nbytes >= sizeof( size_t) + sizeof( uchar_t))
  }   // while( ! done)

  // return success
  return 0;
}
