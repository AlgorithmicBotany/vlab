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




#ifndef MESSAGE_BOX_H
#define MESSAGE_BOX_H

#include <QMessageBox>

/* QMessageBox is not expandable, which makes reading the detail
   text very difficult. This class subclasses QMessageBox and
   adjusts its internal widgets to resize properly.

The initial implementation was based on suggestions here:
http://www.qtcentre.org/threads/24888-Resizing-a-QMessageBox
*/
class MessageBox : public QMessageBox
{
Q_OBJECT
public:
    MessageBox(QWidget *parent = 0);
 
protected:
    bool event(QEvent *event);


};

#endif
