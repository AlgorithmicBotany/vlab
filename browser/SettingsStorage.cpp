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



#include "SettingsStorage.h"
#include <iostream>
#include <qsettings.h>
#include <stack>

SettingsStorage::SettingsStorage(const QString &filename)
    : _qset(filename, QSettings::IniFormat) {
 }

QSettings &SettingsStorage::qset() { return _qset; }

QStringList SettingsStorage::subkeyList(const QString&) {
  //[Pascal : Problem ...]
  QStringList groups = qset().childKeys();
  return groups;
}

void SettingsStorage::removeKey(const QString &key)
// warning - this is recursive...
{
  qset().remove(key);
}


#include <QTextStream>

QString SettingsStorage::readEntry(const QString &key, const QString &def) {
  return qset().value(key, def).toString();
}

int SettingsStorage::readNumEntry(const QString &key, int def) {
  return qset().value(key, def).toInt();
}

bool SettingsStorage::readBoolEntry(const QString &key, bool def) {
  return qset().value(key, def).toBool();
}

// convenience function to read qcolor entry
QColor SettingsStorage::readColorEntry(const QString &key, const QColor &def) {
  QString name = readEntry(key);
  if (name != QString::null)
    return QColor(name);
  else
    return def;
}

// convenience function to read qfont entry
QFont SettingsStorage::readFontEntry(const QString &key, const QFont &def) {
  QString name = readEntry(key);
  if (name != QString::null) {
    QFont f = QApplication::font();
    f.fromString(name);
    return f;
  }

  return def;
}

double SettingsStorage::readDoubleEntry(const QString &key, double def) {
  qset().value(key, def).toDouble();
  return true; // to keep compatibility with old code
}

QStringList SettingsStorage::readStringListEntry(const QString &key,
                                                 const QStringList &def) {
  return qset().value(key, QVariant(def)).toStringList();
}

bool SettingsStorage::writeEntry(const QString &key, const QString &val) {
  qset().setValue(key, val);
  return true;
}

bool SettingsStorage::writeEntry(const QString &key, const QStringList &val) {
  qset().setValue(key, val);
  return true;
}

bool SettingsStorage::writeEntry(const QString &key, bool val) {
  qset().setValue(key, val);
  return true;
}

bool SettingsStorage::writeEntry(const QString &key, int val) {
  qset().setValue(key, val);
  return true;
}

bool SettingsStorage::writeEntry(const QString &key, double val) {
  qset().setValue(key, val);
  return true;
}

// convenience function to write a QColor entry
bool SettingsStorage::writeEntry(const QString &key, const QColor &val) {
  return writeEntry(key, val.name());
}

// convenience function to write a QFont entry
bool SettingsStorage::writeEntry(const QString &key, const QFont &val) {
  return writeEntry(key, val.toString());
}
