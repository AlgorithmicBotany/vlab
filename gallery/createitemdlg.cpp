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



#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QDialogButtonBox>
#include <QVBoxLayout>

#include "createitemdlg.h"

CreateItemDlg::CreateItemDlg(QWidget *parent, const char *, bool modal,
                             Qt::WindowFlags f, SELECTION sel)
    : QDialog(parent, f) {
  setModal(modal);
  if (sel != NONE)
    selection = sel;
  else
    selection = FUNC;

  QVBoxLayout *pLayout = new QVBoxLayout(this);

  QGroupBox *pSelGrp = new QGroupBox("Item type", this);
  pLayout->addWidget(pSelGrp);


  QVBoxLayout *pSelLayout = new QVBoxLayout(pSelGrp);
  pFuncBtn = new QRadioButton("Function (.func)", pSelGrp);
  pConBtn = new QRadioButton("Contour (.con)", pSelGrp);

  if (sel == FUNC) {
    pFuncBtn->setChecked(true);
    pConBtn->setCheckable(false);
    pConBtn->setEnabled(false);
  }

  if (sel == CON) {
    pConBtn->setChecked(true);
    pFuncBtn->setCheckable(false);
    pFuncBtn->setEnabled(false);
  }


  pSelLayout->addWidget(pFuncBtn);
  pSelLayout->addWidget(pConBtn);

  pLayout->addWidget(new QLabel("Item name:", this));
  pName = new QLineEdit("noname", this);
  pLayout->addWidget(pName);

 
  QDialogButtonBox *buttonBox =
      new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

  pLayout->addWidget(buttonBox);

  connect(pFuncBtn, SIGNAL(clicked()), SLOT(setSelFunc()));
  connect(pConBtn, SIGNAL(clicked()), SLOT(setSelCon()));
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}
