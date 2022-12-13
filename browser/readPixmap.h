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




#ifndef __VLAB_BROWSER_READ_PIXMAP_H__
#define __VLAB_BROWSER_READ_PIXMAP_H__

class QPixmap;
class RA_Connection;
class QString;

// reads in a pixmap from a file 'fname' through RA 'connection'
// - if max_size is negative, no scaling is performed
// - if max_size is non-negative, the pixmap is scaled so that its maximum dimension
//   is at most max_size
// - tmpdir is where a remote file will be stored
QPixmap readPixmap(
    RA_Connection * con
    , const QString & fname
    , int max_size
    , const QString & tmpdir );

#endif
