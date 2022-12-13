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



#ifdef __APPLE__
#include <QFont>
#include <cstdio>
#include <errno.h>
#include <sys/sysctl.h>

#endif

void setDefaultFont() {
#ifdef __APPLE__

  char str[256];
  size_t size = sizeof(str);
  sysctlbyname("kern.osrelease", str, &size, NULL, 0);
  int version, x1, x2;
  sscanf(str, "%d.%d.%d", &version, &x1, &x2);
  if (version > 12) {
    // fix Mac OS X 10.9 (mavericks) font issue
    // https://bugreports.qt-project.org/browse/QTBUG-32789
    QFont::insertSubstitution(".Helvetica Neue DeskInterface", "Lucida Grande");
    QFont::insertSubstitution(".Lucida Grande UI", "Lucida Grande");
  }
#endif
}
