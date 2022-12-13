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



#include "auxMenuItem.h"
#include <QLabel>

auxMenuItem::auxMenuItem(const QString &s, const QFont &f, QWidget *parent,
                         const char *name)
    : QLabel(s, parent), string(s), font(f)
//------------------------------------------------------------------------
// Setup string and font style for the menu item
//------------------------------------------------------------------------
{
  setObjectName(name);
}

auxMenuItem::~auxMenuItem()
//------------------------------------------------------------------------
// Destructor
//------------------------------------------------------------------------
{}

void auxMenuItem::drawContents(QPainter *p)
//------------------------------------------------------------------------
// Draw text in the menu entry area, dimension is provide by caller
//------------------------------------------------------------------------
{
  p->setFont(font);
}
