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



#include <cassert>
#include <cstdio>
#include <climits>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <qgl.h>

#include "colors.h"
#include "file.h"
#include <platform.h>

static GLdouble EntryTable[Labels + 1][4] = {
    {0.0, 0.0, 0.0, 0.0}, // Background
    {1.0, 1.0, 1.0, 0.0}, // Points
    {0.4, 0.4, 0.6, 0.0}, // Segments
    {1.0, 1.0, 0.0, 0.0}, // Curve
    {0.2, 0.2, 0.2, 0.0}, // Grid
    {1.0, 0.0, 0.0, 0.0}, // XAxis
    {0.0, 1.0, 0.0, 0.0}, // YAxis
    {0.5, 0.5, 1.0, 0.0}  // Labels
};

static GLdouble WidthEntryTable[FontSize - Labels][1] = {
    {1.0}, // CurveWidth
    {1.0}, // AxisWidth
    {4.0}, // PointSize
    {1.0}, // SegmentSize
    {10.0} // FontSize
};

GLdouble *GetColor(ColorEntry i) {
  if ((i <= Labels) && (i >= 0))
    return EntryTable[i];
  return EntryTable[0];
}

GLdouble GetSize(ColorEntry i) {
  if ((i > Labels) && (i <= FontSize)) {
    return WidthEntryTable[i - Labels - 1][0];
  }
  return WidthEntryTable[0][0];
}

const char *ClrLabels[] = {
    "background:", "points:",       "segments:", "curve:",      "grid:",
    "xaxis:",      "yaxis:",        "labels:",   "curveWidth:", "axisWidth:",
    "pointSize:",  "segmentWidth:", "fontSize:",
};

static void _DoRead(ReadTextFile &);

void ReadColors() {

  QString userConfigDir = "";
#ifdef __APPLE__
  userConfigDir = Vlab::getUserConfigDir(false);
#else
#endif

  char bf[PATH_MAX + 1];
  const char *cdir = userConfigDir.toStdString().c_str();
  if (NULL == cdir)
    return;
  else {
    strcpy(bf, cdir);
    strcat(bf, "/");
  }
  strcat(bf, "cuspy.cfg");
  ReadTextFile src(bf);
  if (src.Valid()) {
    _DoRead(src);
  }
}

void _DoRead(ReadTextFile &src) {
  FILE *fp = src;
  while (!(feof(fp))) {
    char bf[40];
    fscanf(fp, "%s", bf);
    int i;
    for (i = 0; i <= FontSize; i++) {
      if (!strcmp(bf, ClrLabels[i]))
        break;
    }

    if (i < Labels + 1) {
      float r, g, b;
      fscanf(fp, "%f %f %f\n", &r, &g, &b);
      EntryTable[i][0] = r;
      EntryTable[i][1] = g;
      EntryTable[i][2] = b;
    } else if (i < FontSize + 1) {
      float w;
      fscanf(fp, "%f\n", &w);
      WidthEntryTable[i - Labels - 1][0] = w;
    } else {
      fprintf(stderr, "Unknown label: %s\n", bf);
      break;
    }
  }
}
