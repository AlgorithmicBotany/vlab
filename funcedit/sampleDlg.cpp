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



#include <QLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "sampleDlg.h"
#include "geometry.h"
#include "model.h"

SampleDlg::SampleDlg(QWidget *parent, const int , Qt::WindowFlags f)
    : QDialog(parent, f), pModel(0) {
  setWindowTitle("Sample");
  setObjectName("Sample");
  setModal(true);

  QVBoxLayout *pLV = new QVBoxLayout(this);
  QHBoxLayout *pLT = new QHBoxLayout();
  QHBoxLayout *pLB = new QHBoxLayout();
  pLV->addLayout(pLT);
  pLV->addLayout(pLB);
  
  pLT->addWidget(new QLabel("Sample number: ", this));
  pSample = new QLineEdit(this);
  pSample->setText("");
  pLT->addWidget(pSample);

  pApply = new QPushButton("Apply", this);
  pOK = new QPushButton("OK", this);
  pCancel = new QPushButton("Cancel", this);

  pLB->addWidget(pApply);
  pLB->addWidget(pOK);
  pLB->addWidget(pCancel);

  connect(pApply, SIGNAL(clicked()), this, SLOT(sampleChangedDontClose()));
  connect(pOK, SIGNAL(clicked()), this, SLOT(sampleChanged()));
  connect(pCancel, SIGNAL(clicked()), this, SLOT(close()));
}

void SampleDlg::setModel(Model *pModel) {
  this->pModel = pModel;

  if (!pModel)
    return;

  int s = pModel->getSamples();
  pSample->setText(QString(s));
}

void SampleDlg::sampleChanged() {
  
  QString s = pSample->text();
  emit updateSample(s);
  unsigned int n = s.toUInt();
  if (pModel){
    pModel->setSamples(n);
    emit modified();
  }
  close();
}

void SampleDlg::sampleChangedDontClose() {
  
  QString s = pSample->text();
  emit updateSample(s);
  unsigned int n = s.toUInt();
  if (pModel){
    pModel->setSamples(n);
    emit modified();
  }
}
