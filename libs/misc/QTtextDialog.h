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



#ifndef __QTTEXTDIALOG_H
#define __QTTEXTDIALOG_H

#include <qdialog.h>

class QTtextDialog : public QDialog {
public:
  QTtextDialog(int x, int y, char *title, char *message, QWidget *parent = NULL,
               const char *name = NULL, bool modal = false);
  ~QTtextDialog();
};

#endif
