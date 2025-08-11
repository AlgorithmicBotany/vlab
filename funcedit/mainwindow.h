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




#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAbstractButton>
#include <platform.h>

class QCloseEvent;

class MainWindow : public QMainWindow
{
  Q_OBJECT
public:
  MainWindow();

  QMenu *help_menu, *file, *view;

protected:
  void closeEvent(QCloseEvent* pEv) override;
  void showEvent(QShowEvent *) override;
  void handleScreenChanged(QScreen *);

public slots:
  void help();
  void pdfHelp();
  void quickHelp();
  void nameChanged(QString s);
  void editPreferencesCB();
  void msgBoxClosed(QAbstractButton*);

private:
  qreal m_previousDevicePixelRatio;
};

#endif // MAINWINDOW_H

