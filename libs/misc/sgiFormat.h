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



#ifndef _VLAB_SGI_FORMAT_H__
#define _VLAB_SGI_FORMAT_H__
#include "icon.h"
#include "xmemory.h"
#include <QImageIOHandler>
#include <QImageWriter>
#include <QTemporaryFile>
#include <iostream>
#include <qimage.h>
#include <qpainter.h>
#include <qpixmap.h>

#include <QFileInfo>
#include <QImage>

bool writeSGI(QIODevice *IODevice, QImage image);
QImage readSGI(const char *filename);
QImage readSGI(QIODevice *img);

#endif
