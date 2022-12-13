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



#include "MessageBox.h"
#include <QAbstractButton>
#include <QDialogButtonBox>
#include <QEvent>
#include <QTextEdit>
#include <QLabel>
#include <iostream>

MessageBox::MessageBox(QWidget *parent) : QMessageBox(parent) {
  setTextFormat(Qt::AutoText);
  setMouseTracking(true);
  setSizeGripEnabled(true);
  if (QWidget *textEdit = findChild<QTextEdit *>()) {
    textEdit->setMinimumHeight(200);
  }
}


bool MessageBox::event(QEvent *e) {
  bool res = QMessageBox::event(e);
  switch (e->type()) {
  case QEvent::MouseMove:
  case QEvent::MouseButtonPress:
    setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    if (QWidget *textEdit = findChild<QTextEdit *>()) {
      textEdit->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
      textEdit->setMaximumHeight(QWIDGETSIZE_MAX);
    }
    break;
  default:
    break;
  }
  return res;
}



