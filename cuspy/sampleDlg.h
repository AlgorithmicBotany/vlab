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



#ifndef __SAMPLEDLG_H__
#define __SAMPLEDLG_H__

#include <QDialog>
#include <QLineEdit>
#include <QString>
#include <iostream>

class Model;

class SampleDlg : public QDialog {
  Q_OBJECT

public:
  SampleDlg(QWidget *parent = 0, const int sample = 0, Qt::WindowFlags f = 0);

  void setModel(Model *pModel);

  int getSample() { return pSample->text().toInt(); }
  void setSamples(const unsigned int n){
    pSample->setText(QString::number(n));
  }

			       
protected slots:
  void sampleChanged();

signals:
  void updateSample(QString);
  void modified();

protected:
  QLineEdit *pSample;
  Model *pModel;

  QPushButton *pOK;
  QPushButton *pCancel;
};

#endif
