/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#pragma once

#include <QImageReader>
#include <QList>
#include <QString>

class QImageFormatStrings {
public:
  static QString getString(QString format);
  static bool isSupportedFormat(QString filename);
};
