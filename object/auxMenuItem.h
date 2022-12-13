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




#include <qlabel.h>
#include <qstring.h>
#include <qfont.h>
#include <qpainter.h>

#ifndef __AUXMENUITEM_H
#define __AUXMENUITEM_H

class auxMenuItem : public QLabel
{
 public:
  auxMenuItem(const QString& s, const QFont& f, QWidget* parent, 
	      const char* name=0);
  ~auxMenuItem();

 protected:
  void drawContents(QPainter* p);

 private:
  QString string;
  QFont font;
};

#endif
