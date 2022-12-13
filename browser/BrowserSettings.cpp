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



#include "BrowserSettings.h"
#include "platform.h"
#include <QDir>
#include <iostream>
#include <qapplication.h>

// load the current settings from the setting storage
// - if there are no settings, default ones are used
void BrowserSettings::loadCurrent() {
  // by default = use default settings
  setDefault();
  setName("default");
  // where does the browser settings store its settings
  SettingsStorage &ss = BrowserSettings::storage();
  // try to see if in the same storage we store a value for /current
  QString cur_name = ss.readEntry("/browser/current");
  // if the current settings name is stored, we'll try to load  it
  if (cur_name != QString::null) {
    // try to load the current settings
    setName(cur_name);
    load();
  }
  // might as well store the settings we have, just in case they were not
  // stored before
  write();
  ss.writeEntry("/browser/current", name());
}

// get the settings for a particular role
QVariant BrowserSettings::get(Role role) const {
  switch (role) {
  case BackgroundColor:
    return _backgroundColor;
    break;
  case LineColor:
    return _lineColor;
    break;
  case SelectionColor:
    return _selectionColor;
    break;
  case TextColor:
    return _textColor;
    break;
  case BoxBorderColor:
    return _boxBorderColor;
    break;
  case BoxFillColor:
    return _boxFillColor;
    break;
  case HBoxBorderColor:
    return _hboxBorderColor;
    break;
  case HBoxFillColor:
    return _hboxFillColor;
    break;
  case LinkColor:
    return _linkColor;
    break;
  case IconSize:
    return _iconSize;
    break;
  case Compacting:
    return _compacting;
    break;
  case CenterParent:
    return _centerParent;
    break;
  case TextFont:
    return _textFont;
    break;
  default:
    qWarning("BrowserSettings::get():  invalid role.\n");
    return QVariant();
    break;
  }
}

// convenience function for getting a color
QColor BrowserSettings::getColor(Role role) const {
  return get(role).value<QColor>();
}

void BrowserSettings::set(Role role, QVariant val) {
  switch (role) {
  case BackgroundColor:
    _backgroundColor = val.value<QColor>();
    break;
  case LineColor:
    _lineColor = val.value<QColor>();
    break;
  case SelectionColor:
    _selectionColor = val.value<QColor>();
    break;
  case TextColor:
    _textColor = val.value<QColor>();
    break;
  case BoxBorderColor:
    _boxBorderColor = val.value<QColor>();
    break;
  case BoxFillColor:
    _boxFillColor = val.value<QColor>();
    break;
  case HBoxBorderColor:
    _hboxBorderColor = val.value<QColor>();
    break;
  case HBoxFillColor:
    _hboxFillColor = val.value<QColor>();
    break;
  case LinkColor:
    _linkColor = val.value<QColor>();
    break;
  case IconSize:
    _iconSize = val.toInt();
    break;
  case Compacting:
    _compacting = val.toBool();
    break;
  case CenterParent:
    _centerParent = val.toBool();
    break;
  case TextFont:
    _textFont = val.value<QFont>();
    break;
  default:
    qWarning("BrowserSettings::set():  invalid role.\n");
    break;
  }
  if (role == TextFont) {
    std::cerr << "---- setting font to " << _textFont.toString().toStdString()
              << "\n";
  }
}

void BrowserSettings::setLastOofs(const QString &path,
                                  const QString &password) {
  QString simple_path, host;
  if (path.contains(':')) {
    QStringList host_path = path.split(':');
    host = host_path[0];
    host_path.pop_front();
    simple_path = host_path.join(":");
    simple_path = QDir::cleanPath(simple_path);
    simple_path.prepend(host + ':');
  } else {
    QDir simple_dir(path);
    simple_dir.makeAbsolute();
    simple_path = simple_dir.canonicalPath();
    host = QString();
  }
  QStringList _recentOofsPath = recentOofsPath();
  QStringList _recentOofsPasswords = recentOofsPasswords();

  int idx = _recentOofsPath.indexOf(simple_path);
  if (idx != -1) {
    _recentOofsPath.removeAt(idx);
    _recentOofsPasswords.removeAt(idx);
  }
  _recentOofsPath.push_front(simple_path);
  _recentOofsPasswords.push_front(password);
  storage().writeEntry("/recentOofsPath", _recentOofsPath);
  storage().writeEntry("/recentOofsPasswords", _recentOofsPasswords);
}

void BrowserSettings::setRecentOofsPath(const QStringList &lst) {
  storage().writeEntry("/recentOofsPath", lst);
}

void BrowserSettings::setRecentOofsPasswords(const QStringList &lst) {
  storage().writeEntry("/recentOofsPasswords", lst);
}

QStringList BrowserSettings::recentOofsPath() {
  return storage().readStringListEntry("/recentOofsPath");
}

QStringList BrowserSettings::recentOofsPasswords() {
  return storage().readStringListEntry("/recentOofsPasswords");
}

void BrowserSettings::setRecentExportPath(const QString &path) {
  QStringList recentPaths = recentExportPath();
  int index = recentPaths.indexOf(path);
  if (index != -1)
    recentPaths.removeAt(index);
  recentPaths.push_front(path);
  if (recentPaths.size() > 5) // let's not store more than 5 previous choices,
                              // otherwise it is ugly when it pops up
    for (int i = 0; i < (recentPaths.size() - 5); i++)
      recentPaths.removeLast();
  storage().writeEntry("/recentExportPath", recentPaths);
}

void BrowserSettings::setRecentExportFormat(int format) {
  storage().writeEntry("/recentExportFormat", format);
}

void BrowserSettings::setRecentExportType(int type) {
  storage().writeEntry("/recentExportType", type);
}

void BrowserSettings::setRecentExportRecursive(bool recursive) {
  storage().writeEntry("/recentExportRecursive", recursive);
}

void BrowserSettings::setRecentExportSymLinks(bool symLinks) {
  storage().writeEntry("/recentExportSymLinks", symLinks);
}

QStringList BrowserSettings::recentExportPath() {
  return storage().readStringListEntry("/recentExportPath");
}

int BrowserSettings::recentExportFormat() {
  return storage().readNumEntry("/recentExportFormat");
}

int BrowserSettings::recentExportType() {
  return storage().readNumEntry("/recentExportType");
}

bool BrowserSettings::recentExportRecursive() {
  return storage().readBoolEntry("/recentExportRecursive", true);
}

bool BrowserSettings::recentExportSymLinks() {
  return storage().readBoolEntry("/recentExportSymLinks", true);
}

QString BrowserSettings::shellCommandLine() const {
  SettingsStorage &ss = storage();
  QString e = ss.readEntry("/shellCommandLine");
  if (e.isEmpty()) {
#ifdef __APPLE__
    e = "echo $PATH | pbcopy && osascript -e \"tell application "
        "\\\"Terminal\\\" to do script \\\"cd `pwd`; export "
        "PATH=\\`pbpaste\\`\\\"\"";
#else
    //e = "xterm"; // not always available
    e = "x-terminal-emulator"; // works for Debian-based distros...
#endif
  }
  return e;
}

void BrowserSettings::setShellCommandLine(QString cmd) {
  storage().writeEntry("/shellCommandLine", cmd);
}

QString BrowserSettings::editorCommandLine() const {
  SettingsStorage &ss = storage();
  QString e = ss.readEntry("/editorCommandLine");
  if (e.isEmpty()) {
#ifdef __APPLE__
    e = "open \"%1\"";
#else
    e = "emacs \"%1\"&";
#endif
  }
  return e;
}

void BrowserSettings::setEditorCommandLine(QString cmd) {
  storage().writeEntry("/editorCommandLine", cmd);
}

void BrowserSettings::setDefault() {
  _backgroundColor = QColor(17, 17, 17);
  _lineColor = QColor(255, 247, 59);
  _selectionColor = QColor(158, 97, 178);
  _textColor = QColor(255, 248, 251);
  _boxBorderColor = QColor(0, 164, 0);
  _boxFillColor = QColor(0, 63, 0);
  _hboxBorderColor = QColor(240, 240, 0);
  _hboxFillColor = QColor(40, 0, 250);
  _linkColor = QColor(0, 255, 255);
  _compacting = true;
  _centerParent = true;
  _iconSize = 41;
  _textFont = QApplication::font();
#ifdef __APPLE__
  _textFont = QFont("Lucida Grande", 14, QFont::Normal);
#endif
}

// write settings
void BrowserSettings::write() {
  SettingsStorage &ss = BrowserSettings::storage();
  QString p = QString("/browser/set/%1").arg(name());
  ss.writeEntry(p + "/backgroundColor", _backgroundColor);
  ss.writeEntry(p + "/lineColor", _lineColor);
  ss.writeEntry(p + "/selectionColor", _selectionColor);
  ss.writeEntry(p + "/textColor", _textColor);
  ss.writeEntry(p + "/boxBorderColor", _boxBorderColor);
  ss.writeEntry(p + "/boxFillColor", _boxFillColor);
  ss.writeEntry(p + "/hboxBorderColor", _hboxBorderColor);
  ss.writeEntry(p + "/hboxFillColor", _hboxFillColor);
  ss.writeEntry(p + "/linkColor", _linkColor);
  ss.writeEntry(p + "/compacting", _compacting);
  ss.writeEntry(p + "/centerParent", _centerParent);
  ss.writeEntry(p + "/textFont", _textFont);
  ss.writeEntry(p + "/iconSize", _iconSize);
}

bool BrowserSettings::load() {
  BrowserSettings dset;
  dset.setDefault();

  SettingsStorage &ss = BrowserSettings::storage();
  QString p = QString("/browser/set/%1").arg(name());
  _backgroundColor = ss.readColorEntry(
      p + "/backgroundColor", dset.get(BackgroundColor).value<QColor>());
  _lineColor =
      ss.readColorEntry(p + "/lineColor", dset.get(LineColor).value<QColor>());
  _selectionColor = ss.readColorEntry(p + "/selectionColor",
                                      dset.get(SelectionColor).value<QColor>());
  _textColor =
      ss.readColorEntry(p + "/textColor", dset.get(TextColor).value<QColor>());
  _boxBorderColor = ss.readColorEntry(p + "/boxBorderColor",
                                      dset.get(BoxBorderColor).value<QColor>());
  _boxFillColor = ss.readColorEntry(p + "/boxFillColor",
                                    dset.get(BoxFillColor).value<QColor>());
  _hboxBorderColor = ss.readColorEntry(
      p + "/hboxBorderColor", dset.get(HBoxBorderColor).value<QColor>());
  _hboxFillColor = ss.readColorEntry(p + "/hboxFillColor",
                                     dset.get(HBoxFillColor).value<QColor>());
  _linkColor =
      ss.readColorEntry(p + "/linkColor", dset.get(LinkColor).value<QColor>());
  _compacting =
      ss.readBoolEntry(p + "/compacting", dset.get(Compacting).toBool());
  _centerParent =
      ss.readBoolEntry(p + "/centerParent", dset.get(CenterParent).toBool());
  _textFont =
      ss.readFontEntry(p + "/textFont", dset.get(TextFont).value<QFont>());
  _iconSize = ss.readNumEntry(p + "/iconSize", dset.get(IconSize).toInt());

  return true;
}

bool BrowserSettings::reloadColors() {
  bool compacting = _compacting;
  bool centerParent = _centerParent;
  int iconSize = _iconSize;
  QFont textFont = _textFont;

  BrowserSettings dset;
  dset.setDefault();

  SettingsStorage &ss = BrowserSettings::storage();
  QString p = QString("/browser/set/%1").arg(name());
  _backgroundColor = ss.readColorEntry(
      p + "/backgroundColor", dset.get(BackgroundColor).value<QColor>());
  _lineColor =
      ss.readColorEntry(p + "/lineColor", dset.get(LineColor).value<QColor>());
  _selectionColor = ss.readColorEntry(p + "/selectionColor",
                                      dset.get(SelectionColor).value<QColor>());
  _textColor =
      ss.readColorEntry(p + "/textColor", dset.get(TextColor).value<QColor>());
  _boxBorderColor = ss.readColorEntry(p + "/boxBorderColor",
                                      dset.get(BoxBorderColor).value<QColor>());
  _boxFillColor = ss.readColorEntry(p + "/boxFillColor",
                                    dset.get(BoxFillColor).value<QColor>());
  _hboxBorderColor = ss.readColorEntry(
      p + "/hboxBorderColor", dset.get(HBoxBorderColor).value<QColor>());
  _hboxFillColor = ss.readColorEntry(p + "/hboxFillColor",
                                     dset.get(HBoxFillColor).value<QColor>());
  _linkColor =
      ss.readColorEntry(p + "/linkColor", dset.get(LinkColor).value<QColor>());

  _compacting = compacting;
  _centerParent = centerParent;
  _iconSize = iconSize;
  _textFont = textFont;

  return true;
}

SettingsStorage &BrowserSettings::storage() {
  // all variables here are static... so all of this is only ever going to
  // execute once
  static QString userConfigDir = Vlab::getUserConfigDir();
  static QString iniFile = QString("%1/browser.ini").arg(userConfigDir);
  static SettingsStorage _storage(iniFile);

  return _storage;
};
