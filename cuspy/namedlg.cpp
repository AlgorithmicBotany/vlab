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

#include "namedlg.h"
#include "geometry.h"
#include "model.h"

NameDlg::NameDlg(QWidget *parent, const char *name, Qt::WindowFlags f)
    : QDialog(parent, f), pModel(0) {
  setWindowTitle("Name");
  setObjectName(name);
  setModal(true);

  QVBoxLayout *pLV = new QVBoxLayout(this);
  QHBoxLayout *pLT = new QHBoxLayout();
  QHBoxLayout *pLB = new QHBoxLayout();
  pLV->addLayout(pLT);
  pLV->addLayout(pLB);

  pLT->addWidget(new QLabel("Name:  ", this));
  pName = new QLineEdit("noname", this);
  pLT->addWidget(pName);

  pOK = new QPushButton("OK", this);
  pCancel = new QPushButton("Cancel", this);

  pLB->addWidget(pOK);
  pLB->addWidget(pCancel);

  connect(pOK, SIGNAL(clicked()), this, SLOT(nameChanged()));
  connect(pCancel, SIGNAL(clicked()), this, SLOT(close()));
}

void NameDlg::setModel( Model *pModel) {
  this->pModel = pModel;

  if (!pModel)
    return;

  QString s = (pModel->getName()).c_str();
  pName->setText(s);
}

void NameDlg::nameChanged() {
  QString s = pName->text();
  emit updateName(s);

  if (pModel){
    pModel->setName(s.toStdString());
    emit modified();
  }
  close();
}
