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




#ifndef FIXOOFSDIALOG_H
#define FIXOOFSDIALOG_H

#include <QDialog>
#include <QUrl>

namespace Ui {
    class FixOofsDialog;
}

class FixOofsDialog : public QDialog {
    Q_OBJECT
public:
    FixOofsDialog(QWidget *parent = 0);
    ~FixOofsDialog();

protected:
    void changeEvent(QEvent *e);
protected slots:
    void linkClickedCB( const QUrl & url );
    void goCB();
    void saveCB();

private:
    Ui::FixOofsDialog *ui;
    QString _html;
};

#endif // FIXOOFSDIALOG_H
