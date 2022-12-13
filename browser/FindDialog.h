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




#ifndef __QTFIND_DIALOG_H
#define __QTFIND_DIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QString>
#include <QPushButton>
#include <QCheckBox>
#include "ui_FindDialog.h"
#include <vector>
#include <iostream>

class FindDialog : public QDialog, public Ui::UI_FindDialog
{
    Q_OBJECT

public:
    FindDialog( QWidget* parent=NULL, const char* name=NULL, bool modal=false);
  ~FindDialog(){
    //std::cerr<<"kill FindDialog"<<std::endl;
  }
  void show();
									      
public slots:
    void startSearchCB();
    void previousSearchCB();
    void nextSearchCB();
    void quitSearchCB();

    void resetSearch();

private:
    enum SearchState {
        FirstSearch,
        SubsequentSearch,
    };
    void nextSearch(SearchState state);
    void previousSearch(SearchState state);

private:
    std::vector<std::string> response_list;
    std::vector<std::string>::iterator current_object;
};

#endif
