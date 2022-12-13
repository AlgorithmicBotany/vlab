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




#ifndef __VLAB_BROWSER__COLOR_BUTTON_H__
#define __VLAB_BROWSER__COLOR_BUTTON_H__
#include <QWidget>
#include "ui_ColorButton.h"

class ColorButton :  public QWidget, public Ui::UI_ColorButton
{
    Q_OBJECT

public:
    ColorButton( QWidget * parent = NULL, const char * name = NULL, Qt::WindowFlags fl = 0 );

    QColor getColor();

public slots:
    void setName( const QString & name );
    void setColor( const QColor & col );
    void colorChanging(QColor);

signals:
    void colorChanged();
    void rejected();

private slots:
    void button_cb();
    void rejected_cb();

private:
    QColor _color;
    QString _name ;
};

# endif
