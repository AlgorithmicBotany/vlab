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



#include "nodeinfo.h"
#include "RA.h"
#include "xmemory.h"
#include "xstring.h"
#include <assert.h>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>

NodeInfo::NodeInfo() { _raConn = 0; }

/******************************************************************************
 *
 * Constructor for NodeInfo.
 *
 * Parses the file 'fname' that it retrieves through connection 'c'. If
 * no parse errors occured, 'error' is set to 0, otherwise it is set to 1.
 *
 */

NodeInfo::NodeInfo(RA_Connection *c, const std::string &fname) {
  // set the default values
  _fname = fname;
  _raConn = c;

  // read the file into memory 'data'
  char *data = NULL;
  long size = 0;
  if (RA::Read_file(c, _fname.c_str(), data, size))
    return;

  // extract the ID and the name of the object
  // QByteArray qbuff = QByteArray::fromRawData( data, size );
  std::stringstream in(data);

  std::string line;
  std::getline(in, line);
  _uuid = QUuid(line);
  std::getline(in, line);

  if (line.empty())
    line = "";
  _name = line;

  // free up memory
  xfree(data);
}

/******************************************************************************
 *
 * destructor
 *
 */

NodeInfo::~NodeInfo() {}

/******************************************************************************
 *
 * Method 'write'
 *
 * Will write the contents of the class into a file 'fname'.
 *
 * Returns:  true on success,
 *           false on failure
 *
 */

bool NodeInfo::write(void) {
  // print the entire contents of the file into memory
  std::string val;
  val = _uuid.toString() + "\n" + _name + "\n";

  // write the 'node' file
  int write_success =
      RA::Write_file(_raConn, _fname.c_str(), val.c_str(), val.length());
 
  return write_success == 0;
}
