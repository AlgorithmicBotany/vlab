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



#ifndef __xutils_h__
#define __xutils_h__

#include "QTStringDialog.h"
#include <qmessagebox.h>
#include <string>

namespace vlabxutils {
#ifdef _DUMMY_IFDEF____
} // stop emacs from indenting this namespace
#endif

void infoBox(QWidget *parent, const std::string &message,
             const std::string &title = "Message");

bool askYesNo(QWidget *parent, const std::string &question,
              const std::string &title = "Please answer");

void popupInfoBox(QWidget *parent, const char *title, const char *message_fmt,
                  ...);

void setProgress(double val);

void tempBoxPopUp(int x, int y, QWidget *parent, const char *message,
                  const char *title = "Wait");

void tempBoxPopDown(QWidget *parent);
}

#endif /* #ifndef __xutils_h__ */
