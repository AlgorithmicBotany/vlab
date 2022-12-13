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



#include <QImageIOHandler>
#include <QImageIOPlugin>

#include "sgiFormat.h"
#include <QtPlugin>
using namespace Qt;
#undef Q_EXPORT_PLUGIN2
#define Q_EXPORT_PLUGIN2(PLUGIN, PLUGINCLASS)                                  \
  Q_DECL_EXPORT QT_PREPEND_NAMESPACE(QObject) *                                \
      qt_plugin_instance_##PLUGIN() Q_PLUGIN_INSTANCE(PLUGINCLASS)

class SGIIOHandler : public QImageIOHandler {
public:
  bool canRead() const {
    char mag[2];
    device()->peek(mag, 2);
    if (mag[0] == '\001' and mag[1] == '\332')
      return true;
    return false;
  }

  bool read(QImage *image) {
    *image = readSGI(device());

    if (image->isNull())
      return false;
    return true;
  }
};

class SGIIOPlugin : public QImageIOPlugin {
public:
  SGIIOPlugin(QObject *parent = 0) : QImageIOPlugin(parent) {
  }

  Capabilities capabilities(QIODevice *device, const QByteArray &format) const {
    if (format.toLower() == "sgi")
      return QImageIOPlugin::CanRead;
    char mag[2];
    device->peek(mag, 2);
    if (mag[0] == '\001' and mag[1] == '\332')
      return QImageIOPlugin::CanRead;
    return 0;
  }

  QImageIOHandler *create(QIODevice *device,
                          const QByteArray &format = QByteArray()) const {
    SGIIOHandler *h = new SGIIOHandler();
    h->setDevice(device);
    h->setFormat(format);
    return h;
  }

  QStringList keys() const { return _keys; }

  static const QStringList _keys;
};

static QStringList create_keys() {
  QStringList k;
  k << "sgi";
  return k;
}

const QStringList SGIIOPlugin::_keys = create_keys();

Q_EXPORT_PLUGIN2(SGIplugin, SGIIOPlugin);

bool writeSGI(QIODevice *IODevice, QImage image) {

  //
  // convert the QImage into a raw RGB format
  // --------------------------------------------------
  char *res;
  if (image.hasAlphaChannel())
    res = new char[image.width() * image.height() * 4];
  else
    res = new char[image.width() * image.height() * 3];
  //std::cerr<<image.width()<<" - "<<image.height()<<std::endl;
  if (res == NULL) {
    fprintf(stderr, "Could not malloc() memory for RGB image.\n");
    return false;
  }
  char *ptr = res;
  int x, y;
  for (long i = 0; i < long(image.width() * image.height()); i++) {
    y = i / image.width();
    x = i - y * image.width();
    *ptr++ = (char)qRed(image.pixel(x, y));
    *ptr++ = (char)qGreen(image.pixel(x, y));
    *ptr++ = (char)qBlue(image.pixel(x, y));
    if (image.hasAlphaChannel())
      *ptr++ = (char)qAlpha(image.pixel(x, y));
  }

  // now save the image into a temporary file
  QTemporaryFile file;
  // the file must be opened to generate the unique filename so we will just do
  // that now then close it
  if (file.open())
    file.close();
  else
    return false;
  FILE *fp = fopen(file.fileName().toLatin1().data(), "w");
  if (fp == NULL) {
    fprintf(stderr, "Could not open temporary file %s\n",
            file.fileName().toLatin1().data());
    return false;
  }
  int colorType = (image.hasAlphaChannel()) ? F_RGBA : F_FULLCOLOR;
  if (WriteIRIS(fp, (byte *)res, PIC24, image.width(), image.height(), NULL,
                NULL, NULL, colorType)) {
    fprintf(stderr, "Could not write to file %s\n",
            file.fileName().toLatin1().data());
    return false;
  }
  fclose(fp);
  delete[] res;
  // then read that tempFile into the IODevice
  file.open();
  QByteArray tempBA = file.readAll();
  if (tempBA.size() == 0)
    return false;
  int ret = IODevice->write(tempBA);
  if (ret == -1)
    return false;
  file.close();
  return true;
}

QImage readSGI(const char *filename) {
  // by default we fail:
 
  // load the icon
  QImage res;
  PICINFO icon;
  int loaded = LoadIRIS(filename, &icon);
  // if there was an error loading, return error
  if (loaded != 1) {
    return res;
  }
  if (icon.type != PIC24) {
    qWarning("sgiFormat.cpp: readSGI() - cannot handle indexed images.");
    xfree(icon.pic);
    return res;
  }
  // otherwise convert the image into QImage
  if (icon.colType == F_RGBA)
    res = QImage(icon.w, icon.h, QImage::Format_ARGB32_Premultiplied);
  else
    res = QImage(icon.w, icon.h, QImage::Format_RGB32);
  if (res.isNull())
    return res; // cannot allocate image
  unsigned char *psrc = icon.pic;
  for (int y = 0; y < icon.h; y++) {
    QRgb *p = (QRgb *)res.scanLine(y);
    for (int x = 0; x < icon.w; x++) {
      if (res.hasAlphaChannel()) {
        *p++ = qRgba(psrc[1], psrc[2], psrc[3], psrc[0]);
        psrc += 4;
      } else {
        *p++ = qRgb(psrc[0], psrc[1], psrc[2]);
        psrc += 3;
      }
    }
  }

  xfree(icon.pic);

  return res;
}

QImage readSGI(QIODevice *img) {
  // instead of reading directly from the supplied device, let's
  // put the contents into a temporary file and use the old function to read in
  // the icon from a file
  QTemporaryFile file;
  if (file.open()) {
    file.write(img->readAll());
    file.flush();
    file.seek(0);
    return readSGI(file.fileName().toLatin1().data());
  }
  return QImage();
}
