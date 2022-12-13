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
  // void closeEvent(QCloseEvent *event);

public:
  Preferences(QWidget *parent = 0, QString objName = NULL);
  ~Preferences();
  void WriteColors(std::string line);
  void loadConfig();

signals:
  void configChanged();
  void colorChanged(QColor);
  void backgroundColorChanged(QColor);
  void borderWidthChanged(int);

public slots:
  void Save(QAbstractButton *);
  void Save(int);
  void cancel();
  void paste();
  void keyPressEvent(QKeyEvent *e) {
    if (e->key() != Qt::Key_Escape)
      QDialog::keyPressEvent(e);
    else { /* minimize */
    }
  } // this is to avoid escape key to close the dialog
    //  int ok();

  void pickColor();
  void pickBackgroundColor();

  void setColor(const QColor &color);
  void setBackgroundColor(const QColor &color);

  void resetColor();
  void resetBackgroundColor();

  void set_BorderWidth(int value);

  QColor getBackgroundColor() { return _backgroundColor; }

  QColor getColor() { return _color; }
  int getBorderWidth() { return _borderWidth; }

private:
  QString _fileName;
  QString _fileNametmp;

  QColor _backgroundColor;
  QColor _previousBackgroundColor;

  QColor _color;
  QColor _previousColor;

  int _borderWidth;
  int _previousBorderWidth;

  void setButtonColor(QPushButton *button, const QColor &color);
};

#endif // PREFERENCES_H
