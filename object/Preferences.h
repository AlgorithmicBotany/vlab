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




#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QDialog>
#include <QDir>
#include <QFileDialog>
#include <QMenu>
#include "ui_Preferences.h"


class Preferences : public QDialog, public Ui_Preferences {
    Q_OBJECT

      protected:

public:
  Preferences(QWidget *parent = 0,QString objName = NULL, QString configName = NULL);
    ~Preferences();


public slots:
    void Save();
    void copy();
    void paste();

private:
    QString _fileName;
    QString _configName;
    QMenu* _editMenu;
};


#endif // PREFERENCES_H
