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



#include "config.h"
#include <fstream>
#include <iostream>

#include <QColor>
#include <QSettings>
#include <QString>
#include <QVariant>

using namespace std;

float Config::gridColour[4] = {0.5f, 0.5f, 0.5f, 0.5f};
float Config::xAxisColour[4] = {0.0f, 0.0f, 1.0f, 0.8f};
float Config::yAxisColour[4] = {1.0f, 0.0f, 0.0f, 0.8f};
float Config::zAxisColour[4] = {1.0f, 1.0f, 0.0f, 0.8f};
float Config::upVectorColour[4] = {0.8f, 0.8f, 0.8f, 0.8f};
float Config::headingColour[4] = {0.4f, 0.4f, 0.4f, 0.8f};
float Config::connectionPointColour[4] = {0.7f, 0.2f, 0.8f, 0.9f};
float Config::endPointColour[4] = {0.6f, 0.1f, 0.7f, 0.9f};
float Config::controlPointsColour[4] = {0.9f, 0.7f, 0.4f, 1.0f};
float Config::wireColour[4] = {0.5f, 0.8f, 0.3f, 1.0f};
float Config::solidColour[4] = {0.8f, 0.8f, 0.5f, 1.0f};
float Config::selectedColour[4] = {1.0f, 0.6f, 0.6f, 1.0f};
float Config::stickyColour[4] = {0.0f, 1.0f, 0.0f, 1.0f};
float Config::splitColour[4] = {0.0f, 1.0f, 1.0f, 1.0f};
float Config::selectedForMergeColour[4] = {1.0f, 0.2f, 0.2f, 1.0f};
float Config::wireHLColour[4] = {0.5f, 0.8f, 0.8f, 1.0f};
float Config::solidHLColour[4] = {0.8f, 0.8f, 1.0f, 1.0f};
float Config::controlPolyColour[4] = {0.0f, 0.8f, 1.0f, 0.9f};
float Config::controlPolyInColour[4] = {0.0f, 0.4f, 0.5f, 0.8f};
float Config::controlPolyHLColour[4] = {0.5f, 1.0f, 1.0f, 0.9f};
float Config::ambientColour[4] = {0.5f, 0.5f, 0.5f, 1.0f};
float Config::pointSize = 5.0f;
float Config::selectionSize = 10.0f;
int Config::uDivs = 20;
int Config::vDivs = 20;

static void readColour(float colour[], QString name, QSettings &settings) {
  QColor qcol = QColor::fromRgbF(colour[0], colour[1], colour[2], colour[3]);
  qcol = settings.value(name, qcol).value<QColor>();
  colour[0] = qcol.redF();
  colour[1] = qcol.greenF();
  colour[2] = qcol.blueF();
  colour[3] = qcol.alphaF();
}

void Config::readConfig() {
  QSettings settings;

  readColour(gridColour, "GridColour", settings);
  readColour(xAxisColour, "XAxisColour", settings);
  readColour(yAxisColour, "YAxisColour", settings);
  readColour(zAxisColour, "ZAxisColour", settings);
  readColour(upVectorColour, "UpVectorColour", settings);
  readColour(headingColour, "HeadingColour", settings);
  readColour(connectionPointColour, "ConnectionPointColour", settings);
  readColour(endPointColour, "EndPointColour", settings);
  readColour(controlPointsColour, "ControlPointsColour", settings);
  readColour(wireColour, "WireColour", settings);
  readColour(solidColour, "SolidColour", settings);
  readColour(selectedColour, "SelectedColour", settings);
  readColour(stickyColour, "StickyColour", settings);
  readColour(splitColour, "SplitColour", settings);
  readColour(selectedForMergeColour, "SelectedForMergeColour", settings);
  readColour(wireHLColour, "WireHLColour", settings);
  readColour(solidHLColour, "SolidHLColour", settings);
  readColour(controlPolyColour, "ControlPolyColour", settings);
  readColour(controlPolyInColour, "ControlPolyInColour", settings);
  readColour(controlPolyHLColour, "ControlPolyHLColour", settings);
  readColour(ambientColour, "AmbientColour", settings);

  bool ok;
  float size = settings.value("PointSize", pointSize).toDouble(&ok);
  if (!ok)
    throw ConfigErrorExc("Invalid point size");
  if (size < 1.0)
    throw ConfigErrorExc("Invalid point size (must be larger than 1.0)");
  pointSize = size;

  size = settings.value("SelectionSize", selectionSize).toDouble(&ok);
  if (!ok)
    throw ConfigErrorExc("Invalid selection size");
  if (size < 1.0)
    throw ConfigErrorExc("Invalid selection size (must be larger than 1.0)");
  selectionSize = size;

  int divs = settings.value("UDivs", uDivs).toInt(&ok);
  if (!ok)
    throw ConfigErrorExc("Invalid u divs");
  if (size < 4)
    throw ConfigErrorExc("Invalid u divs (must be at least 4)");
  uDivs = divs;

  divs = settings.value("VDivs", vDivs).toInt(&ok);
  if (!ok)
    throw ConfigErrorExc("Invalid v divs");
  if (size < 4)
    throw ConfigErrorExc("Invalid v divs (must be at least 4)");
  vDivs = divs;
}
