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



#include "Preferences.h"
#include <QFileInfo>
#include <QSettings>
#include <QTextStream>
#include <QTabWidget>
#include <algorithm>
#include <iostream>
#include <sstream>

#include "object.h"

Preferences::Preferences(QWidget *parent, QString fileName, QString configName)
    : QDialog(parent), _fileName(fileName), _configName(configName) {
  setupUi(this);
  if (!_fileName.isEmpty()) {
    QFile file(_fileName);
    if (file.open(QFile::ReadOnly | QFile::Text))
      specPlainTextEdit->setPlainText(file.readAll());
  }
  if (!_configName.isEmpty()) {
    QFile file(_configName);
    if (file.open(QFile::ReadOnly | QFile::Text))
      configPlainTextEdit->setPlainText(file.readAll());
  }
  /*
  _editMenu = new QMenu(("&Edit"), this);
  _editMenu->addAction("Copy", this, SLOT(copy()));
  _editMenu->addAction("Paste", this, SLOT(paste()));
  */

  connect(buttonBox, SIGNAL(accepted()), SLOT(Save()));

  this->setWindowTitle("Preferences");

  this->move(obj.obj_posx, obj.obj_posy);
}

Preferences::~Preferences() {}

void Preferences::Save() {
  if (!_fileName.isEmpty()) {
    QString preferences = specPlainTextEdit->toPlainText();
    QFile file(_fileName);
    if (file.open(QFile::WriteOnly | QFile::Text)) {
      QTextStream out(&file);
      out << preferences;
      file.close();
    }
  }

  if (!_configName.isEmpty()) {
    QString preferences = configPlainTextEdit->toPlainText();
    QFile file(_configName);
    if (file.open(QFile::WriteOnly | QFile::Text)) {
      QTextStream out(&file);
      out << preferences;
      file.close();
    }
  }
}

void Preferences::copy() {}
void Preferences::paste() {}
