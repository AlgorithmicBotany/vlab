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



#include <QFileInfo>
#include <iostream>
#include <sstream>
#include <algorithm>
#include "SaveAs.h"
#include "dirList.h"
#include <QSettings>
SaveAs::SaveAs(QWidget *parent, QString objName, QString currentPath, QString pix_format)
    : QDialog(parent) {

  nodeName = objName;
  saveName = currentPath;
  setupUi(this);
  decay = 0;
  imageTypes << "BMP";
  imageTypes << "GIF";
  imageTypes << "JPG";
  imageTypes << "PBM";
  imageTypes << "PDF";
  imageTypes << "PNG";
  imageTypes << "TIFF";
  for (int i = 0; i < imageTypes.size(); i++) {
    comboBox->addItem(imageTypes.at(i));
  }
  comboBox->setCurrentText(pix_format);
  previousPixFormat = pix_format;
  
  settingsFile = QString(QDir::homePath()) + "/.vlab/snapicon.ini";


  directory->addItem(saveName);

  loadSettings();

  directory->setEditText(saveName);
  QTextCursor cursor = lineEdit->textCursor();
  this->setLineEdit(); 

  connect(lineEdit, SIGNAL(textChanged()), SLOT(preserveExtension()));

  connect(lineEdit, SIGNAL(textChanged()), SLOT(preserveFormat()));
  connect(lineEdit, SIGNAL(cursorPositionChanged()), SLOT(preserveExtension()));
  connect(directory, SIGNAL(editTextChanged(QString)),
          SLOT(setDirectoryList()));
  connect(chooseDirectory, SIGNAL(clicked()), SLOT(browse()));
  connect(comboBox, SIGNAL(currentIndexChanged(int)),
          SLOT(setImageFormat(int)));
  this->setWindowTitle("Save As ...");
  this->setFixedSize(this->size());
}

QString SaveAs::getExtension() {
  QString extension;
  switch (comboBox->currentIndex()) {
    case 0:
      extension = "bmp";
      previousPixFormat = "BMP";
      break;
    case 1:
      previousPixFormat = "GIF";
      extension = "gif";
      break;
    case 2:
      extension = "jpg";
      previousPixFormat = "JPG";
      break;
    case 3:
      extension = "pbm";
      previousPixFormat = "PBM";
      break;
    case 4:
      extension = "pdf";
      previousPixFormat = "PDF";
      break;
    case 5:
      extension = "png";
      previousPixFormat = "PNG";
      break;
    case 6:
      extension = "tiff";
      previousPixFormat = "TIFF";
      break;
    default:
      extension = "";
      break;
    }
  return extension;
}
void SaveAs::loadSettings() {
  QSettings settings(settingsFile, QSettings::NativeFormat);
  QStringList keyList = settings.allKeys();
  for (QStringList::Iterator it = keyList.begin(); it != keyList.end(); ++it) {
    QString key = *it;
    QString sText = settings.value(key, "").toString();
    bool found = false;
    for (int i = 0; ((i < directory->count()) && !found); i++) {
      QString item = directory->itemText(i);

      if (!QString::compare(item, sText))
        found = true;
    }
    if (!found)
      directory->addItem(sText);
  }
}

void SaveAs::writeSettings() {
  QSettings settings(settingsFile, QSettings::NativeFormat);
  for (int i = 0; i < std::min(10, directory->count()); i++) {
    if ((directory->itemText(i).indexOf("/tmp/") != 0) &&
        (directory->itemText(i).indexOf("/private/tmp/") != 0)) {
      std::stringstream st;
      st << i;
      std::string key = "path" + st.str();
      settings.setValue(QString::fromStdString(key), directory->itemText(i));
    }
  }
}

void SaveAs::closeEvent(QCloseEvent *) { writeSettings(); }

SaveAs::~SaveAs() {}

void SaveAs::setLineEdit() {
  QString formatedNodeName =
      QString("<span style= color:#000000;> %1</span>").arg(nodeName);
    decay = 0;
    QString ext = QString("<span style= color:#999999;>%1</span>")
                      .arg("." + getExtension());
    lineEdit->setHtml("<span style= color:#000000;>" + formatedNodeName +
                      "</span>" + ext);
}

void SaveAs::preserveFormat() {
  QTextCursor cursor = lineEdit->textCursor();
  int cursorPosition = cursor.position();
  if (cursorPosition == 1) {
    if (changeFormat) {
      changeFormat = false;
      setNodeName();
      setLineEdit();
      cursor.setPosition(1);
      lineEdit->setTextCursor(cursor);
    }
  } else
    changeFormat = true;
}

void SaveAs::setNodeName() {
  int ext_size = 4;
  if (previousPixFormat.compare( "TIFF") == 0)
    ext_size = 5;
  nodeName = lineEdit->toPlainText();
  nodeName.chop(ext_size + decay);
}

void SaveAs::preserveExtension() {

  int ext_size = 4;
  if (previousPixFormat.compare( "TIFF") == 0)
    ext_size = 5;

  QString name = lineEdit->toPlainText();

  QTextCursor cursor = lineEdit->textCursor();
  int cursorPosition = cursor.position();
  if (cursorPosition > name.size() - ext_size - decay) {
    cursor.setPosition(name.size() - ext_size - decay);
    lineEdit->setTextCursor(cursor);
  }
}

void SaveAs::setDirectoryList() {
  
  // saveName is added if it is not in the list.
  bool found = false;
  int i = 0;

  for (i = 1; ((i < directory->count()) && !found); i++) {
    QString item = directory->itemText(i);
    if (QString::compare(item, saveName)==0){
      found = true;
    }
  }
  if (!found)
      directory->insertItem(0, saveName);

  
}

int SaveAs::getFormat() { return comboBox->currentIndex(); }

int SaveAs::getImageType() { return comboBox->currentIndex(); }

QString SaveAs::getImageBaseName() {
  int ext_size = 4;
  if (previousPixFormat.compare( "TIFF") == 0)
    ext_size = 5;

  nodeName = lineEdit->toPlainText();
  nodeName.chop(ext_size + decay);

  return nodeName;
}


void SaveAs::setImageFormat(int f) {
  this->setNodeName();
  comboBox->setCurrentIndex(f);
  previousPixFormat = f;
  getExtension();
  this->setLineEdit();
}


QString SaveAs::getPath() { return directory->currentText(); }

void SaveAs::setPaths(QStringList inPaths) {
  for (int i = 0; i < inPaths.size(); i++) {
    directory->addItem(inPaths.at(i));
  }
  // intelligently set the type based on the first item added to the drop-downs,
  // might as well make it easy for the user
}

void SaveAs::browse() {

  setDirectoryList();

  QComboBox *target;
  QString temp, dir;
  int type;
  // Export tab
  target = directory;
  type = comboBox->currentIndex();
  temp = target->currentText();
  temp = QDir::fromNativeSeparators(
      temp); // QFileDialog likes these strings to be in the "/" format, so
             // let's give it to them that way
             // we aren't looking for a directory
  // if we are importing, open a load file dialog
  // otherwise, a save file dialog
  dir = QFileDialog::getExistingDirectory(
      this, tr("Select Directory"), saveName,
      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

  // then put the file/directory name back into native format (what the user
  // will be used to) and set the edit text to read this
  if (dir.length() > 0) { // only if they actually picked something
    saveName = QDir::toNativeSeparators(dir);
    directory->setEditText(saveName);
  }
}

int SaveAs::ok() {
  // we don't need to test the result() attribute of this object since
  // closeEvent() is only called when accepted but if you like you can test: if
  return 1;
}
