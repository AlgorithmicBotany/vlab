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




#ifndef __VLAB_BROWSER_FINDER_WIDGET_H__
#define __VLAB_BROWSER_FINDER_WIDGET_H__

#include <iostream>
#include <QLineEdit>
#include <QKeyEvent>

class FinderWidget
    : public QLineEdit
{
    Q_OBJECT
public:
    FinderWidget( QWidget * parent, const char * name = 0 );
    void activate();
protected:
    virtual void focusOutEvent( QFocusEvent * e);
    virtual void keyPressEvent( QKeyEvent * e );
    void doSearching();
    bool in_progress;
private slots:
    void textChanged_cb();

};

#endif
