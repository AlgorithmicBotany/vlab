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




#ifndef NEWBROWSER_H
#define NEWBROWSER_H

#include <QDialog>
#include <QString>

#include "ui_newbrowser.h"

class QDragEnterEvent;
class QDropEvent;
class QShowEvent;

class NewBrowserDlg : public QDialog
{
  Q_OBJECT
public:
  NewBrowserDlg(QWidget *parent = 0, Qt::WindowFlags f = 0);
  NewBrowserDlg(const QString& default_oofs, const QString& default_password, QWidget *parent = 0, Qt::WindowFlags f = 0);

  QString oofs();

public slots:
  void on_chooseDirectory_clicked();
  void on_buttonBox_clicked( QAbstractButton *button );
  void on_recentOofs_activated(int idx);

protected:
  void resetContent();
  void setContent(const QString& oofs, const QString& password);
  void dragEnterEvent(QDragEnterEvent *ev);
  void dropEvent(QDropEvent *ev);
  void showEvent(QShowEvent *ev);

  Ui_NewBrowser ui;
  QString default_oofs, default_password;

  QStringList recent_oofs, recent_passwords;
  Qt::WindowFlags _f;
};

#endif // NEWBROWSER_H

