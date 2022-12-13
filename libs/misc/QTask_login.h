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



#ifndef __QTASK_LOGIN_H
#define __QTASK_LOGIN_H

#include "ui_UI_RALogin.h"

class QTask_login : public QDialog {
  Q_OBJECT

public:
  QTask_login(const QString &username = QString::null,
              const QString &password = QString::null, QWidget *parent = NULL,
              const char *name = NULL, bool modal = true);
  ~QTask_login();
  // returns the contents of username & password (should be called only if exec
  // returned
  //          QDialog::Accepted
  void getResponse(QString &username, QString &password);

private:
  Ui::UI_RALogin ui;
};


#endif
