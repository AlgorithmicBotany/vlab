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
#include <algorithm>
#include <iostream>
#include <sstream>

Preferences::Preferences(QWidget *parent, QString fileName)
    : QDialog(parent), _fileName(fileName) {
  setupUi(this);

  if (!_fileName.isEmpty()) {
    QFile file(_fileName);
    if (file.exists()) {
      if (file.open(QFile::ReadOnly | QFile::Text))
        plainTextEdit->setPlainText(file.readAll());
    }

    else {
      // load default file
      QString defaultPreferences("background: 0.0 0.0 0.0\n"
                                 "points: 1.0 1.0 1.0\n"
                                 "segments: 0.4 0.4 0.6\n"
                                 "curve: 1.0 1.0 0.0\n"
                                 "grid: 0.2 0.2 0.2\n"
                                 "xaxis: 1.0 0.0 0.0\n"
                                 "yaxis: 0.0 1.0 0.0 \n"
                                 "labels: 0.5 0.5 1.0\n"
                                 "curveWidth: 1.0  \n"
                                 "pointSize: 4.0\n"
                                 "axisWidth: 1.0\n"
                                 "segmentWidth: 1.0\n ");
      plainTextEdit->setPlainText(defaultPreferences);
    }
  }

  connect(buttonBox, SIGNAL(clicked(QAbstractButton *)), SLOT(Save(QAbstractButton *)));
  QPoint parentPos = parent->pos();
  this->move(parentPos.x() + parent->width(), parentPos.y());

  this->setModal(false);
  this->setWindowTitle("Preferences");
}

Preferences::~Preferences() {}

void Preferences::Save(QAbstractButton *btn) {
  switch (buttonBox->buttonRole(btn)) {
  case QDialogButtonBox::RejectRole:
    close();
    break;
  default:
    if (!_fileName.isEmpty()) {
      QString preferences = plainTextEdit->toPlainText();
      QFile file(_fileName);
      if (file.open(QFile::WriteOnly | QFile::Text)) {
	QTextStream out(&file);
	out << preferences;
	file.close();
      }
      emit preferenceChanged();
    }
    break;
  }
  if (buttonBox->buttonRole(btn) == QDialogButtonBox::AcceptRole)
    close();
}

void Preferences::copy() {}
void Preferences::paste() {}
