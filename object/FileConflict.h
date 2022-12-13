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




#ifndef FILECONFLICT_H
#define FILECONFLICT_H

#include <QDialog>
#include <QTreeWidgetItem>
#include "TransferObject.h"

struct Match {
    QTreeWidgetItem* matchItem;
    int matchDepth;
};

namespace Ui {
    class FileConflict;
}

class FileConflict : public QDialog {
    Q_OBJECT
public:
    FileConflict(QWidget *parent, TransferObject* inObject, QString basePath);
    ~FileConflict();

public slots:
    void lineEditChanged(QString);
    void treeSelectionChanged();
    void lineEditLostFocus();
    void refreshTree(QTreeWidgetItem* clicked);
    void accept();

protected:
    void changeEvent(QEvent *e);
    Match findTreeNode(QStringList inList, int depth, QTreeWidgetItem* base = NULL);
    void processConflicts();
    void enableLineEdit(bool inValue);
    QString getFullName(QTreeWidgetItem* child);
    void updateLists(QTreeWidgetItem* base);
    void updateTreeCheckboxes(QTreeWidgetItem* clicked, int value);
    void colourSubTree(QTreeWidgetItem* clicked);

private:
    Ui::FileConflict *ui;
    TransferObject* to;
    QString base;
};

#endif // FILECONFLICT_H
