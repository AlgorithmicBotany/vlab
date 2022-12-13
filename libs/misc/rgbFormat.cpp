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

class RGBIOHandler : public QImageIOHandler {
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

  bool write(const QImage &image) {
    if (writeSGI(device(), image))
      return true;
    else
      return false;
    return false;
  }
};

class RGBIOPlugin : public QImageIOPlugin {
public:
  RGBIOPlugin(QObject *parent = 0) : QImageIOPlugin(parent) {
  }

  Capabilities capabilities(QIODevice *device, const QByteArray &format) const {
    if (format.toLower() == "rgb")
      return QImageIOPlugin::CanRead | QImageIOPlugin::CanWrite;
    char mag[2];
    device->peek(mag, 2);
    if (mag[0] == '\001' and mag[1] == '\332')
      return QImageIOPlugin::CanRead | QImageIOPlugin::CanWrite;
    return 0;
  }

  QImageIOHandler *create(QIODevice *device,
                          const QByteArray &format = QByteArray()) const {
    RGBIOHandler *h = new RGBIOHandler();
    h->setDevice(device);
    h->setFormat(format);
    return h;
  }

  QStringList keys() const { return _keys; }

  static const QStringList _keys;
};

static QStringList create_keys() {
  QStringList k;
  k << "rgb";
  return k;
}

const QStringList RGBIOPlugin::_keys = create_keys();

Q_EXPORT_PLUGIN2(RGBplugin, RGBIOPlugin);
