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




#ifndef __RA_CONNECTION_H__
#define __RA_CONNECTION_H__

#include "MessagePipe.h"
#include <string>

// connection class for the Remote Access library

typedef enum { RA_LOCAL_CONNECTION,
	       RA_REMOTE_CONNECTION,
	       RA_NO_CONNECTION } RA_Connection_Type;

class RA_Connection {

public:

    // -- methods --

    /*constructor*/        RA_Connection ( void);

    int                          Connect ( const char * host_name,
					   const int    port_num,
					   const char * user_name,
					   const char * password);

    void                      Disconnect ( void);

    int                          same_as ( RA_Connection * c);
    bool isLocal() {
        return connection_type == RA_LOCAL_CONNECTION;
    }
    bool IP_has_changed(void);
    std::string get_IP();
    bool check_connection(void);
    int reconnect(void);
    int reconnect_and_remain_open(void);
    void Disconnect_remain_open_connection(void);
    void set_keep_connection_open(bool state);

    bool get_keep_connection_open(void);


    // -- variables --

    RA_Connection_Type     connection_type ;
    char *                 host_name ;
    char *                 login_name ;
    char *                 password ;
    MessagePipe *          messagePipe;

   // -- methods --
    // [Pascal] Should be private ?
    RA_Connection_Type get_connection_type ( void);
    int                    port_num;
    bool  keep_connection_open; // false close, true open

private:

    std::string local_ip_address;
    // -- variables --
    unsigned char *                 addr;
    int                    addr_length;
    int                    sock;

};

#endif
