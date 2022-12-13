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




#ifndef __NODE_INFO_H__
#define __NODE_INFO_H__

#include "quuid.h"
#include <string>
#include "RA.h"

class NodeInfo
{

public:
    // empty constructor
    NodeInfo();
    // constructor that reads in the file and parses it
    NodeInfo( RA_Connection * c, const std::string & fname);
    // constructor that just populates the structure, not reading anything
    NodeInfo( RA_Connection * c, const std::string & fname,
	      const QUuid & uuid, const std::string & name ) {
	_raConn = c; _fname = fname, _name = name; _uuid = uuid;
    }
    // returns uuid
    const QUuid & uuid() const { return _uuid; }
    // sets the uuid of the node
    void uuid( const QUuid & uuid ) { _uuid = uuid; }
    // returns the name of the node
    const std::string & name() const { return _name; };
    // sets the name of the node
    void name( const std::string & name ) { _name = name; }
    ~NodeInfo(); // destructor
    bool write( void);

private:
    std::string _fname; // filename of the node, saved for writing
    QUuid _uuid;        // extracted from the node file
    std::string _name;  // extracted from the node file, empty if not set
    RA_Connection * _raConn; // saved RA connection
};

#endif
