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



#include "ColorButton.h"
#include <iostream>
#include <qcolordialog.h>
#include <qlabel.h>
#include <qpushbutton.h>

ColorButton::ColorButton(QWidget *parent, const char *, Qt::WindowFlags )
    : QWidget(parent) {
  setupUi(this);
  setColor(QColor(50, 100, 200));
  connect(button, SIGNAL(clicked()), SLOT(button_cb()));
}

QColor ColorButton::getColor() { return _color; }

void ColorButton::setColor(const QColor &col) {

  _color = col;
  QPalette pal;
  pal.setColor(QPalette::Background, _color);

  QString style = "QPushButton{background-color: rgb(%1, %2, %3);}";
  button->setStyleSheet(
      style.arg(_color.red()).arg(_color.green()).arg(_color.blue()));

  float brightness =
      (299 * _color.red() + 587 * _color.green() + 114 * _color.blue()) / 1000.;

  if (brightness < 125) {
    style = "QPushButton {background-color: rgb(%1, %2, %3);color: "
            "rgb(255,255,255);}";
    button->setStyleSheet(
        style.arg(_color.red()).arg(_color.green()).arg(_color.blue()));
  }

}

void ColorButton::button_cb() {
 
  QPoint buttonPos = this->pos();

  QColorDialog *qcd = new QColorDialog(_color, this);
  qcd->move(buttonPos.x() + 100, buttonPos.y() + 100);
  qcd->setOption(QColorDialog::DontUseNativeDialog);
  qcd->setWindowTitle("Select color : " + _name);
  connect(qcd, SIGNAL(rejected()), SLOT(rejected_cb()));

  connect(qcd, SIGNAL(currentColorChanged(QColor)), this,
          SLOT(colorChanging(QColor)));
  qcd->show();
  QColor col = qcd->selectedColor();
  if (col.isValid()) {
    setColor(col);
    emit colorChanged();
  }
}

void ColorButton::rejected_cb() { emit rejected(); }

void ColorButton::colorChanging(QColor col) {
  if (col.isValid()) {
    setColor(col);
    emit colorChanged();
  }
}

void ColorButton::setName(const QString &name) {
  button->setText(name);
  _name = name;
}
