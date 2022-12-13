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




#ifndef __LIBVLABD_H__
#define __LIBVLABD_H__

#include "vlabd_msgid.h"

class VlabD {

public:

    static const int n_invoke_retries = 5; // number of times we try to
				           // start vlabd
    static const int invoke_pause = 1;     // number of seconds to wait between
				           // restarts
    static const int n_connect_retries = 5;// number of times we try to connect
                                           // to vlabd
    static const int connect_pause = 1;    // number of seconds to wait between
                                           // conects
    
    VlabD( void);

    // establish a socket connection to VLABD
    // - if vlabd is not running, it will be started, but only if if 'allowStart=true'
    // - returns -1 if connection could not be made
    // -          0 if connection worked
    int init( bool allowStart = true );

    // returns true if connection is valid
    bool valid() { return _valid; }

    // send a message to VLABD
    int send_message( long code, const char * data, long size);
    int send_message( long code, const char * str);
    int send_message( long code);

    int va_send_message( long code, const char * fmt_str, ...);

    // is there anything waiting to be read on the socket?
    int has_data( void);

    // receive a message from VLABD
    int get_message( long & code, char * & data, long & length);

    // get the text description of the last error
    char * get_errstr( void);

    // access to the socket
    int get_sock( void);

private:
    
    int sock;			// the socket connected to VLABD
    char err_buff[ 4096];	// description of the last error
    bool _valid;		// is the connection valid?

    // get the next size bytes from the socket
    int receive_n_bytes( char * buffer, long size);
    
};

#endif
