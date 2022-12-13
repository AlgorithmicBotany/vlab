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



#include <string.h>

#include "QTStringDialog.h"

QTStringDialog::QTStringDialog(const char *description, const char *title,
                               const char *def_str, QWidget *parent,
                               const char *name, bool modal)
    : QInputDialog(parent) {
  setObjectName(name);
  setModal(modal);
  setWindowTitle(title ? title : "Prompt");

  setInputMode(QInputDialog::TextInput);
  setLabelText(description);
  setTextValue(def_str);

  answer = NULL;
}

QTStringDialog::~QTStringDialog() { delete[] answer; }

void QTStringDialog::accept() {
  answer = new char[256];
  strncpy(answer, this->textValue().toLatin1().constData(), 256);
  answer[255] = '\0';
  QDialog::accept();
}

char *QTStringDialog::getResult() { return answer; }
