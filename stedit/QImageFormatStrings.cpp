/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#include "QImageFormatStrings.h"

QString QImageFormatStrings::getString(QString format) {
  format = format.toLower();
  if (format == "bmp")
    return "Windows Bitmap";
  if (format == "gif")
    return "GIF Image";
  if (format == "ico")
    return "Icon File";
  if (format == "jpg")
    return "JPG Image";
  if (format == "jpeg")
    return "JPEG Image";
  if (format == "mng")
    return "MNG Image";
  if (format == "png")
    return "PNG Image";
  if (format == "pbm")
    return "Portable Bitmap";
  if (format == "pgm")
    return "Portable Graymap";
  if (format == "ppm")
    return "Portable Pixmap";
  if (format == "rgb")
    return "SGI RGB Image";
  if (format == "sgi")
    return "SGI RGB Image";
  if (format == "svg")
    return "SVG Image";
  if (format == "tif")
    return "TIF Image";
  if (format == "tiff")
    return "TIFF Image";
  if (format == "xbm")
    return "X11 Bitmap";
  if (format == "xpm")
    return "X11 Pixmap";
  else
    return "?";
}

bool QImageFormatStrings::isSupportedFormat(QString filename) {
  QList<QByteArray> loadFormatsList = QImageReader::supportedImageFormats();
  QByteArray ext = filename.section('.', 1, 1).toLatin1();
  if (loadFormatsList.contains(ext))
    return true;
  return false;
}
