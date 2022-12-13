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




#ifndef __CREATEITEMDLG_H__
#define __CREATEITEMDLG_H__

#include <QDialog>
#include <QRadioButton>
#include <QLineEdit>
#include <QPushButton>
#include <QFileInfo>
#include <iostream>

class CreateItemDlg : public QDialog {
  Q_OBJECT

 public:
  enum SELECTION {
    FUNC,
    CON,
    NONE
  };

  CreateItemDlg(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags = 0,SELECTION sel=NONE);
 

  SELECTION getSelection() {return selection;}
  
  const std::string getFileName() {
    std::string fileName = pFileName->text().toStdString();
    
    return fileName;
  }
  const std::string getName() {
    std::string fileName = pName->text().toStdString();
    return fileName;
  }

 protected slots:
  void setSelFunc()  
  {
     selection = FUNC;
  }
  void setSelCon()   {
    selection = CON;
  }
 
 protected:
  SELECTION selection;

  QRadioButton* pFuncBtn;
  QRadioButton* pConBtn;
  QRadioButton* pImageBtn;

  QPushButton* pCreateBtn;
  QPushButton* pCancelBtn;

  QLineEdit* pName;
  QLineEdit* pFileName;
};

#endif
