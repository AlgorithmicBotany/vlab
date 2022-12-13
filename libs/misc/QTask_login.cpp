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



#include <qlabel.h>
#include <qpushbutton.h>

#include "QTask_login.h"
#include "xmemory.h"
#include "xstring.h"
#include "xutils.h"

QTask_login::QTask_login(const QString &username, const QString &password,
                         QWidget *parent, const char *name, bool modal)
    : QDialog(parent) {
  ui.setupUi(this);
  setObjectName(name);
  setModal(modal);
  setWindowTitle("Vlab Raserver Login");
  ui.username_text->setText(username);
  ui.password_text->setText(password);

}

QTask_login::~QTask_login() {}

void QTask_login::getResponse(QString &username, QString &password) {
  username = ui.username_text->text();
  password = ui.password_text->text();
}
