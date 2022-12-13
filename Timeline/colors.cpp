#include <cassert>
#include <cstdio>
#include <climits>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <qgl.h>

#include "colors.h"
#include "file.h"

static GLdouble EntryTable[BackgroundEdit + 1][4] = {
    {0.0, 0.0, 0.0, 0.0}, // Background
    {1.0, 1.0, 1.0, 0.0}, // Points
    {0.4, 0.4, 0.6, 0.0}, // Segments
    {1.0, 1.0, 0.0, 0.0}, // Curve
    {0.2, 0.2, 0.2, 0.0}, // Grid
    {1.0, 0.0, 0.0, 0.0}, // XAxis
    {0.0, 1.0, 0.0, 0.0}, // YAxis
    {1.0, 0.5, 0.5, 0.0}, // SelectedPoints
    {0.5, 1.0, 0.5, 0.0}, // ButtonTitles
    {0.5, 1.0, 0.5, 0.0}, // ButtonLabels
    {0.5, 0.5, 1.0, 0.0},  // GridLabels
    {0.5, 0.5, 0.5, 0.0}   // BackgroundEdit
};

GLdouble *GetColor(ColorEntry i) { return EntryTable[i]; }

QColor GetQColor(ColorEntry i) {
  const GLdouble *color = GetColor(i);
  QColor qcolor = QColor(color[0] * 255, color[1] * 255, color[2] * 255,
                         255 - color[3] * 255);
  return qcolor;
}

const char *ClrLabels[] = {"background color",  "points color",
                           "segments color",    "curve color",
                           "grid color",        "xaxis color",
                           "yaxis color",       "selected points color",
                           "event title color", "event label color",
                           "grid label color",  "background edit color"};
const char *GetLabel(int i) { return ClrLabels[i]; }
GLdouble GetColor(int i, int j) {
  if (j < 0)
    return 0.0;
  if (j <= 2)
    return EntryTable[i][j];
  return 0.0;
}

static void _DoRead(ReadTextFile &);

void ReadColors(QString fileName) {
  const char *bf = fileName.toLatin1().data();
  ReadTextFile src(bf);
  if (src.Valid()) {
    fprintf(stderr, "Reading colors from: %s\n", bf);
    _DoRead(src);
  }
}

void _DoRead(ReadTextFile &src) {
  FILE *fp = src;
  while (!(feof(fp))) {
    char bf[40];
    fscanf(fp, "%[^:]", bf);

    int i;
    for (i = 0; i <= BackgroundEdit; i++) {
      if (!strcmp(bf, ClrLabels[i]))
        break;
    }
    if (i == BackgroundEdit + 1) {
      break;
    }
    float r, g, b;
    fscanf(fp, "%f %f %f\n", &r, &g, &b);
    EntryTable[i][0] = r;
    EntryTable[i][1] = g;
    EntryTable[i][2] = b;
  }
}

void ReadColors(std::string line) {
  //
  float r = 0.0, g = 0.0, b = 0.0;
  char bf[40];
  char buffer[40];
  int i;
  sscanf(line.c_str(), "%[^':']:%[^'\n']\n", bf, buffer);
  for (i = 0; i <= BackgroundEdit; i++) {
    if (!strcmp(bf, ClrLabels[i]))
      break;
  }

  if (i == BackgroundEdit + 1) {
    return;
  }
  sscanf(buffer, "%f %f %f\n", &r, &g, &b);

  EntryTable[i][0] = r;
  EntryTable[i][1] = g;
  EntryTable[i][2] = b;
}

void setColor(ColorEntry i, QColor color) {
  //
  EntryTable[i][0] = 1.0 * color.red() / 255;
  EntryTable[i][1] = 1.0 * color.green() / 255;
  EntryTable[i][2] = 1.0 * color.blue() / 255;
}
