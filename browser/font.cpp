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



#include "QTGLbrowser.h"
#include "main.h"
#include <QFont>

int strWidth(const std::string &s) {
  return QFontMetrics(sysInfo.mainForm->browserSettings()
                          .get(BrowserSettings::TextFont)
                          .value<QFont>())
      .width(QString(s.c_str()));
}

void getFontMaxSize(int *width, int *height) {
  QFontMetrics fm(sysInfo.mainForm->browserSettings()
                      .get(BrowserSettings::TextFont)
                      .value<QFont>());
  *width = fm.width("W");
  *height = fm.height();
}

bool fontname_valid(char *) { return true; }

void change_font(char *) {}
