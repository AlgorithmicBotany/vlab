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



#include "FileConflict.h"
#include "ImportExport.h"
#include "ui_FileConflict.h"

FileConflict::FileConflict(QWidget *parent, TransferObject *inObject,
                           QString basePath)
    : QDialog(parent), ui(new Ui::FileConflict) {
  QString temp;
  QStringList shortDirs, shortFiles;
  to = inObject;
  basePath.append("/"); // we want to strip the leading slash once we pull
                        // basePath off, so we can just add it here so when we
                        // remove this string later it is done
  base = basePath;      // we need this later when marking the conflicted files
  for (int i = 0; i < to->remoteFiles.size(); i++) {
    temp = to->remoteFiles.at(i);
    if (temp.startsWith(basePath))
      temp.remove(0, basePath.length());
    shortFiles.push_back(temp);
  }
  for (int i = 0; i < to->remoteDirs.size(); i++) {
    temp = to->remoteDirs.at(i);
    if (temp.startsWith(basePath))
      temp.remove(0, basePath.length());
    shortDirs.push_back(temp);
  }
  ui->setupUi(this);
  enableLineEdit(false);
  ui->treeWidget->setHeaderItem(new QTreeWidgetItem(QStringList("File names")));
  if ((shortDirs.size() > 0) && (shortFiles.size() > 0)) {
    QBrush tempBrush;
    // tokenize the first item in shortDirs and make a parent chain out of it,
    // then compare all the others to this chain
    ui->treeWidget->clear();
    QTreeWidgetItem *itemPointer, *parentPointer;
    QStringList splitList = shortDirs.at(0).split("/");
    itemPointer = new QTreeWidgetItem(ui->treeWidget);
    itemPointer->setText(0, splitList.at(0));
    itemPointer->setText(1, to->remoteDirs.at(0));
    itemPointer->setText(2, "d0");
    itemPointer->setText(3, "1");
    itemPointer->setExpanded(true);
    itemPointer->setCheckState(0, Qt::Checked);
    itemPointer->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
    tempBrush = itemPointer->foreground(0);
    tempBrush.setColor(Qt::black);
    itemPointer->setForeground(0, tempBrush);
    ui->treeWidget->addTopLevelItem(itemPointer);
    parentPointer = ui->treeWidget->topLevelItem(0);
    for (int i = 1; i < splitList.size();
         i++) { // build the first chain through the tree
      itemPointer = new QTreeWidgetItem(parentPointer);
      itemPointer->setText(0, splitList.at(i));
      itemPointer->setText(1, to->remoteDirs.at(0));
      itemPointer->setText(2, "d0");
      itemPointer->setText(3, "1");
      itemPointer->setExpanded(true);
      itemPointer->setCheckState(0, Qt::Checked);
      itemPointer->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
      tempBrush = itemPointer->foreground(0);
      tempBrush.setColor(Qt::black);
      itemPointer->setForeground(0, tempBrush);
      parentPointer->addChild(itemPointer);
      parentPointer = itemPointer;
    }
    Match closest;
    for (int i = 1; i < shortDirs.size(); i++) {
       splitList = shortDirs.at(i).split("/");
      closest =
          findTreeNode(splitList, 0); // search the tree, see how close we can
                                      // get to matching the full string list
      if (closest.matchItem ==
          NULL) { // if we didn't even find a base node that matches then we
                  // need to make a new one
        parentPointer = new QTreeWidgetItem(ui->treeWidget);
        parentPointer->setText(0, splitList.at(0));
        parentPointer->setText(1, to->remoteDirs.at(i));
        parentPointer->setText(2, QString("d" + QString::number(i)));
        parentPointer->setText(3, "1");
        parentPointer->setExpanded(true);
        parentPointer->setCheckState(0, Qt::Checked);
        parentPointer->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
        tempBrush = parentPointer->foreground(0);
        tempBrush.setColor(Qt::black);
        parentPointer->setForeground(0, tempBrush);
        ui->treeWidget->addTopLevelItem(parentPointer);
        closest.matchDepth = 1;
        closest.matchItem = parentPointer;
      }
      if (closest.matchDepth < splitList.size()) {
        // fprintf(stderr,"Didn't quite find the end, need to finish making the
        // chain\n matchdepth: %i splitList.size(): %i\n",closest.matchDepth,
        // splitList.size());
        // we couldn't match the full path so we need to add some new bits to
        // make this path exist in the tree
        parentPointer = closest.matchItem;
        for (int j = closest.matchDepth; j < splitList.size(); j++) {
          itemPointer = new QTreeWidgetItem(parentPointer);
          itemPointer->setText(0, splitList.at(j));
          itemPointer->setText(1, to->remoteDirs.at(i));
          itemPointer->setText(2, QString("d" + QString::number(i)));
          itemPointer->setText(3, "1");
          itemPointer->setExpanded(true);
          itemPointer->setCheckState(0, Qt::Checked);
          itemPointer->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
          tempBrush = itemPointer->foreground(0);
          tempBrush.setColor(Qt::black);
          itemPointer->setForeground(0, tempBrush);
          parentPointer->addChild(itemPointer);
          parentPointer = itemPointer;
        }
      }
    }
    for (int i = 0; i < shortFiles.size(); i++) {
      splitList = shortFiles.at(i).split("/");
      closest =
          findTreeNode(splitList, 0); // search the tree, see how close we can
                                      // get to matching the full string list
      if (closest.matchItem != NULL) {
        if (closest.matchDepth < splitList.size()) {
           // we couldn't match the full path so we need to add some new bits to
          // make this path exist in the tree
          parentPointer = closest.matchItem;
          for (int j = closest.matchDepth; j < splitList.size(); j++) {
            itemPointer = new QTreeWidgetItem(parentPointer);
            itemPointer->setText(0, splitList.at(j));
            itemPointer->setText(1, to->remoteFiles.at(i));
            itemPointer->setText(2, QString("f" + QString::number(i)));
            itemPointer->setText(3, "1");
            itemPointer->setExpanded(true);
            itemPointer->setCheckState(0, Qt::Checked);
            if (j == splitList.size() - 1) {
              itemPointer->setIcon(0,
                                   style()->standardIcon(QStyle::SP_FileIcon));
            } else {
              itemPointer->setIcon(0,
                                   style()->standardIcon(QStyle::SP_DirIcon));
            }
            tempBrush = itemPointer->foreground(0);
            tempBrush.setColor(Qt::black);
            itemPointer->setForeground(0, tempBrush);
            parentPointer->insertChild(0, itemPointer);
            parentPointer = itemPointer;
          }
        }
      } // if we have a file that is outside of any directories we have listed
        // this far, then we have BIG problems.  No need to handle this case
    }
  }
  processConflicts();
}

Match FileConflict::findTreeNode(QStringList inList, int depth,
                                 QTreeWidgetItem *base) {
  Match ret, temp;
  ret.matchItem = NULL;
  ret.matchDepth = 0;
  if (inList.size() > depth) {
    // make sure we don't over-run the inList, if we do return null and the
    // function will know to return the result from 1 less level of recursion
    if (depth == 0) {
      // are we on the base level?  If so search the toplevelitems because we
      // don't have a base yet
      for (int i = 0; i < ui->treeWidget->topLevelItemCount(); i++) {
         if (ui->treeWidget->topLevelItem(i)->text(0).compare(
                inList.at(depth)) == 0) {
          // did we find an item with a name that matches our level?
           temp = findTreeNode(inList, 1, ui->treeWidget->topLevelItem(i));
          if (temp.matchDepth > ret.matchDepth) {
            // did we find any children that are a better match?
            ret.matchDepth = temp.matchDepth;
            ret.matchItem = temp.matchItem;
          } else {
            // if not, then take this node as the best match
            ret.matchItem = ui->treeWidget->topLevelItem(i);
            ret.matchDepth = 1;
          }
        }
      }
    } else {
      if (base != NULL) { // just make sure...
        if (base->childCount() > 0) { // is this the end of the line?
          for (int i = 0; i < base->childCount(); i++) {
            if (base->child(i)->text(0).compare(inList.at(depth)) == 0) {
              temp = findTreeNode(inList, depth + 1, base->child(i));
              if (temp.matchDepth > ret.matchDepth) {
                // are any of these children a better match?
                ret.matchDepth = temp.matchDepth;
                ret.matchItem = temp.matchItem;
              } else {
                // if not, take this node as the best match
                ret.matchDepth = depth + 1;
                ret.matchItem = base->child(i);
              }
            }
          }
          if (ret.matchItem == NULL) { // if we hit the point where no children
                                       // match, take the current node
            ret.matchItem = base;
            ret.matchDepth = depth;
          }
        } else {
          ret.matchDepth = depth;
          ret.matchItem = base;
        }
      }
    }
  }
  return ret;
}

FileConflict::~FileConflict() { delete ui; }

void FileConflict::changeEvent(QEvent *e) {
  QDialog::changeEvent(e);
  switch (e->type()) {
  case QEvent::LanguageChange:
    ui->retranslateUi(this);
    break;
  default:
    break;
  }
}

void FileConflict::updateLists(QTreeWidgetItem *base) {
  if (base != NULL) {
    QBrush tempBrush = base->foreground(0);
    QString flags;
    if (base->text(2).contains("f"))
      flags = to->fileFlags.at(QString(base->text(2).remove(0, 1)).toInt());
    else
      flags = to->dirFlags.at(QString(base->text(2).remove(0, 1)).toInt());
    if (base->checkState(0) == Qt::Unchecked)
      tempBrush.setColor(Qt::darkGreen);
    else {
      if (flags.contains("O")) {
        tempBrush.setColor(Qt::blue);
      } else {
        tempBrush.setColor(Qt::black);
      }
    }
    base->setForeground(0, tempBrush);
    QString number = base->text(2).right(base->text(2).length() - 1);
    if (base->text(2).contains("f")) {
      to->remoteFiles.replace(number.toInt(), getFullName(base));
    } else {
      to->remoteDirs.replace(number.toInt(), getFullName(base));
    }
    for (int i = 0; i < base->childCount(); i++) {
      updateLists(base->child(i));
    }
  }
}

QString FileConflict::getFullName(QTreeWidgetItem *child) {
  QString ret = child->text(0);
  if (child->parent() != NULL)
    ret.prepend("/").prepend(getFullName(child->parent()));
  else
    ret.prepend(base);
  return ret;
}

void FileConflict::refreshTree(QTreeWidgetItem *clicked) {
  if ((clicked->checkState(0) == Qt::Checked) &&
      (clicked->text(3).compare("1") != 0)) {
    // if the checkbox is currently checked and wasn't before this action, then
    // send the update
    updateTreeCheckboxes(clicked, 1);
    clicked->setCheckState(0, Qt::Checked);
    clicked->setText(3, "1");
  } else if ((clicked->checkState(0) == Qt::Unchecked) &&
             (clicked->text(3).compare("0") != 0)) {
    updateTreeCheckboxes(clicked, 0);
    clicked->setCheckState(0, Qt::Unchecked);
    clicked->setText(3, "0");
  } else if ((clicked->checkState(0) == Qt::PartiallyChecked) &&
             (clicked->text(3).compare("2") != 0)) {
    updateTreeCheckboxes(clicked, 2);
    clicked->setCheckState(0, Qt::PartiallyChecked);
    clicked->setText(3, "2");
  }
  // since this function can be called when you click an enabled item's disabled
  // child we need to check the parent state before we enable the check box on
  // the item which called the event, or else you can disable a parent, then
  // click 1 level at a time through the tree re-enabling items which are
  // disabled at a higher level since QT lets you call events on an enabled
  // item's disabled children
  if (clicked->parent() != NULL) {
    if ((clicked->parent()->checkState(0) == Qt::Checked) ||
        (clicked->parent()->checkState(0) == Qt::PartiallyChecked))
      clicked->setFlags(clicked->flags() | Qt::ItemIsEnabled);
  } else
    clicked->setFlags(clicked->flags() | Qt::ItemIsEnabled);
  for (int i = 0; i < ui->treeWidget->topLevelItemCount(); i++) {
    colourSubTree(ui->treeWidget->topLevelItem(i));
  }
  // colourSubTree(clicked);
  // ^^^^
  // can't just do this, since if you change a name then click down inside the
  // item you don't know at what level you changed the name and as a result some
  // parts of the tree go un-coloured
}

void FileConflict::updateTreeCheckboxes(QTreeWidgetItem *clicked, int value) {
  if (value == 1) {
    clicked->setCheckState(0, Qt::Checked);
    clicked->setText(3, "1");
    clicked->setFlags(clicked->flags() | Qt::ItemIsEnabled);
  } else if (value == 0) {
    clicked->setCheckState(0, Qt::Unchecked);
    clicked->setText(3, "0");
    clicked->setFlags(clicked->flags() & ~Qt::ItemIsEnabled);
  } else {
    clicked->setCheckState(0, Qt::PartiallyChecked);
    clicked->setText(3, "2");
    clicked->setFlags(clicked->flags() | Qt::ItemIsEnabled);
  }
  for (int i = 0; i < clicked->childCount(); i++)
    updateTreeCheckboxes(clicked->child(i), value);
}

void FileConflict::colourSubTree(QTreeWidgetItem *clicked) {
  // identify type of node
  // check flags for that index of that type
  // color appropriately
  // recurse on children
  if (clicked != NULL) { // just a sanity check
    QBrush tempBrush;
    tempBrush = clicked->foreground(0);
    if (clicked->checkState(0) == Qt::Unchecked) {
      tempBrush.setColor(Qt::darkGreen);
    } else {
      QString flags;
      if (clicked->text(2).contains("f"))
        flags =
            to->fileFlags.at(QString(clicked->text(2).remove(0, 1)).toInt());
      else
        flags = to->dirFlags.at(QString(clicked->text(2).remove(0, 1)).toInt());
      if (flags.contains("O"))
        tempBrush.setColor(Qt::blue);
      else if (flags.contains("C"))
        tempBrush.setColor(Qt::red);
      else
        tempBrush.setColor(Qt::black);
    }
    clicked->setForeground(0, tempBrush);
    for (int i = 0; i < clicked->childCount(); i++) {
      colourSubTree(clicked->child(i));
    }
  }
}

void FileConflict::processConflicts() {
  // build new lists for dirs and files to pass back to the ImportExport object
  for (int i = 0; i < ui->treeWidget->topLevelItemCount(); i++) {
    updateLists(ui->treeWidget->topLevelItem(i));
  }
  // update the colour of the tree now
  for (int i = 0; i < ui->treeWidget->topLevelItemCount(); i++) {
    colourSubTree(ui->treeWidget->topLevelItem(i));
  }
}

void FileConflict::lineEditChanged(QString inText) {
  ui->treeWidget->currentItem()->setText(0, inText);
  if (ui->treeWidget->currentItem()->text(2).contains("f"))
    to->fileFlags.replace(
        QString(ui->treeWidget->currentItem()->text(2).remove(0, 1)).toInt(),
        "R");
  else
    to->dirFlags.replace(
        QString(ui->treeWidget->currentItem()->text(2).remove(0, 1)).toInt(),
        "R");
}

void FileConflict::enableLineEdit(bool inValue) {
  if (inValue) {
    ui->lineEdit->setEnabled(true);
    QPalette temp = ui->label->palette();
    temp.setColor(QPalette::WindowText, Qt::black);
    ui->label->setPalette(temp);
    temp = ui->lineEdit->palette();
    temp.setColor(QPalette::WindowText, Qt::black);
    temp.setColor(QPalette::Text, Qt::black);
    ui->lineEdit->setPalette(temp);
  } else {
    ui->lineEdit->setEnabled(false);
    QPalette temp = ui->label->palette();
    temp.setColor(QPalette::WindowText, Qt::gray);
    ui->label->setPalette(temp);
    temp = ui->lineEdit->palette();
    temp.setColor(QPalette::WindowText, Qt::gray);
    temp.setColor(QPalette::Text, Qt::gray);
    ui->lineEdit->setPalette(temp);
  }
}

void FileConflict::lineEditLostFocus() { processConflicts(); }

void FileConflict::treeSelectionChanged() {
  ui->lineEdit->setText(ui->treeWidget->currentItem()->text(0));
  enableLineEdit(true);
 }

void FileConflict::accept() {
  ((ImportExport *)this->parent())->receiveTransferObject(to);
  QDialog::accept(); 
}
