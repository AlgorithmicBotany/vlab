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



#include "main.h"
#include "tree.h"
#include <QPixmap>

NODE::NODE() {}

QPoint NODE::getTopLeft() const
// return the top left corner of the node
// - this will work after build_tree() was called
{
  return QPoint(x, y);
}

QSize NODE::getNodeSize() const
// rerurns the size of the node itself (not including children)
// - this will work after build_tree() was called
{
  return QSize(nodeWidth, nodeHeight);
}

QRect NODE::getNodeRect() const
// returns the bounding rectangle of the node
// - this will work after build_tree() was called
{
  return QRect(getTopLeft(), getNodeSize());
}

const QRect &NODE::getTreeRect() const
// returns the bounding rectangle of the tree starting at the node
// - this will work after build_tree() was called
{
  return _treeRect;
}

const QPixmap &NODE::icon()
// returns the icon of the node
// - if icon is loaded, return the loaded icon
// - if icon loaded unsuccessfuly, return default icon
// - if icon is in the process of being loaded, return the temporary icon
{
  int iconSize = sysInfo.mainForm->browserSettings()
                     .get(BrowserSettings::IconSize)
                     .toInt();
  if (_isIconLoading)
    return QTbrowser::hourglassIcon(iconSize);
  if (_icon.isNull())
    return QTbrowser::defaultIcon(iconSize);
  else
    return _icon;
}

bool NODE::isIconLoading()
// returns whether the icon is loading or not
// - icon is loading when its loading flag and showing flag is set
{
  return _isIconLoading && iconShow;
}
