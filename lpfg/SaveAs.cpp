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
SaveAs::SaveAs(QWidget *parent, QString objName, QString currentPath,
               QString labTablePath, int number, int id, int outputFormat,
               int pix_format, bool enableAlphaChannel)
    : QDialog(parent) {
  // check if the OS theme is light or dark (Qt 6.5 solution will be different, but may still work)
  const QPalette defaultPalette;
  const auto text = defaultPalette.color(QPalette::WindowText);
  const auto window = defaultPalette.color(QPalette::Window);
  _isLightTheme = text.lightness() < window.lightness();
  
  _enableAlphaChannel = enableAlphaChannel;
  changeFormat = false;
  nodeName = objName;
  saveName = currentPath;
  setupUi(this);
  // we want explicit control of the order of the items in the combobox so that
  // we know from the code what each index is
  formats << "Image";
  formats << "Postscript";
  formats << "Obj";
  formats << "POV-Ray";
  formats << "Rayshade";
  formats << "View";
  for (int i = 0; i < formats.size(); i++) {
    comboBox->addItem(formats.at(i));
  }
  comboBox->setCurrentIndex(outputFormat);
  previousOutputFormat = outputFormat;
  setAlphaChannelBox(outputFormat);
 
  imageTypes << "BMP";
  imageTypes << "GIF";
  imageTypes << "JPG";
  imageTypes << "PBM";
  imageTypes << "PDF";
  imageTypes << "PNG";
  imageTypes << "TIFF";
  for (int i = 0; i < imageTypes.size(); i++) {
    comboBox_2->addItem(imageTypes.at(i));
  }
  comboBox_2->setCurrentIndex(pix_format);
  previousPixFormat = pix_format;
  settingsFile = QString(QDir::homePath()) + "/.vlab/lpfgsettings.ini";
  directory_2->addItem(saveName);
  if (saveName != labTablePath)
    directory_2->addItem(labTablePath);
  loadSettings();

  directory_2->setEditText(saveName);
  QTextCursor cursor = lineEdit->textCursor();
  this->setLineEdit();
  setFileNumber(id);
  setNumbering(number);
  startNumber->setValue(id);

  setFormat(outputFormat);

 

  connect(lineEdit, SIGNAL(textChanged()), SLOT(preserveExtension()));
  connect(startNumber, SIGNAL(valueChanged(int)), SLOT(setFileNumber(int)));
  connect(startNumber, SIGNAL(valueChanged(int)), this, SLOT(setLineEdit()));
  connect(startNumber, SIGNAL(valueChanged(int)), this, SLOT(preserveFormat()));

  connect(lineEdit, SIGNAL(textChanged()), SLOT(preserveFormat()));
  connect(lineEdit, SIGNAL(cursorPositionChanged()), SLOT(preserveExtension()));
  connect(directory_2, SIGNAL(editTextChanged(QString)),
          SLOT(setDirectoryList()));
  connect(chooseDirectory_5, SIGNAL(clicked()), SLOT(browse()));
  connect(comboBox_2, SIGNAL(currentIndexChanged(int)),
          SLOT(setAlphaChannelBox(int)));
  connect(comboBox_2, SIGNAL(currentIndexChanged(int)),
          SLOT(setImageFormat(int)));
  connect(checkBox, SIGNAL(stateChanged(int)), SLOT(setNumbering(int)));
  connect(comboBox, SIGNAL(currentIndexChanged(int)), SLOT(setFormat(int)));
  this->setWindowTitle("Save As ...");
}

QString SaveAs::getExtension() {
  QString extension;
  switch (comboBox->currentIndex()) {
  case 0:
    switch (comboBox_2->currentIndex()) {
    case 0:
      extension = "bmp";
      break;
    case 1:
      extension = "gif";
      break;
    case 2:
      extension = "jpg";
      break;
    case 3:
      extension = "pbm";
      break;
    case 4:
      extension = "pdf";
      break;
    case 5:
      extension = "png";
      break;
    case 6:
      extension = "tiff";
      break;
    default:
      extension = "";
      break;
    }
    break;
  case 1:
    extension = "ps";
    break;
  case 2:
    extension = "obj";
    break;
  case 3:
    extension = "pov";
    break;
  case 4:
    extension = "ray";
    break;
  case 5:
    extension = "view";
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
    for (int i = 0; ((i < directory_2->count()) && !found); i++) {
      QString item = directory_2->itemText(i);

      if (!QString::compare(item, sText))
        found = true;
    }
    if (!found)
      directory_2->addItem(sText);
  }
}

void SaveAs::writeSettings() {
  QSettings settings(settingsFile, QSettings::NativeFormat);
  for (int i = 0; i < std::min(10, directory_2->count()); i++) {
    if ((directory_2->itemText(i).indexOf("/tmp/") != 0) &&
        (directory_2->itemText(i).indexOf("/private/tmp/") != 0)) {
      std::stringstream st;
      st << i;
      std::string key = "path" + st.str();
      ;
      settings.setValue(QString::fromStdString(key), directory_2->itemText(i));
    }
  }
}

void SaveAs::closeEvent(QCloseEvent *) { writeSettings(); }

void SaveAs::changeEvent(QEvent *event) {
  // check if the OS theme has changed (tested on macOS)
  if (event->type() == QEvent::PaletteChange) {
    _isLightTheme = !_isLightTheme;
    this->setLineEdit();
  } else {
    QWidget::changeEvent(event);
  }
}

SaveAs::~SaveAs() {}

void SaveAs::setLineEdit() {
  QString textColor = _isLightTheme ? "#000000" : "#FFFFFF";
  QString formatedNodeName =
      QString("<span style='color:" + textColor + ";'>" + nodeName + "</span>");
  QString ext;
  if (numbering == 2) {
    decay = 4;
    ext = QString("<span style='color:#999999;'>%1</span>").arg(fileNameNumber + "." + getExtension());
  } else {
    decay = 0;
    ext = QString("<span style='color:#999999;'>%1</span>").arg("." + getExtension());
  }
  lineEdit->setHtml(formatedNodeName + ext);
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
  if (previousOutputFormat == 1)
    ext_size = 3;
  else if (previousOutputFormat == 6)
    ext_size = 5;
  else if ((previousOutputFormat == 0) && (previousPixFormat == 6))
    ext_size = 5;

  nodeName = lineEdit->toPlainText();
  nodeName.chop(ext_size + decay);
}

void SaveAs::preserveExtension() {

  int ext_size = 4;
  if (previousOutputFormat == 1)
    ext_size = 3;
  else if (previousOutputFormat == 6)
    ext_size = 5;
  else if ((previousOutputFormat == 0) && (previousPixFormat == 6))
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
  for (int i = 0; ((i < directory_2->count()) && !found); i++) {
    QString item = directory_2->itemText(i);
    if (!QString::compare(item, saveName))
      found = true;
  }
  if (!found)
    directory_2->insertItem(1, saveName);
}

int SaveAs::getFormat() { return comboBox->currentIndex(); }

int SaveAs::getImageType() { return comboBox_2->currentIndex(); }

QString SaveAs::getImageBaseName() {
  int ext_size = 4;
  if (previousOutputFormat == 1)
    ext_size = 3;
  else if (previousOutputFormat == 6)
    ext_size = 5;
  else if ((previousOutputFormat == 0) && (previousPixFormat == 6))
    ext_size = 5;

  nodeName = lineEdit->toPlainText();
  nodeName.chop(ext_size + decay);

  return nodeName;
}

int SaveAs::getNumbering() { return numbering; }

void SaveAs::setNumbering(int b) {
  this->setNodeName();
  numbering = b;
  if (numbering == 2) {
    startNumber->setEnabled(true);

    decay = 4;
    checkBox->setCheckState(Qt::Checked);
  } else {
    startNumber->setEnabled(false);

    decay = 0;
    checkBox->setCheckState(Qt::Unchecked);
  }
  this->setLineEdit();
}

bool SaveAs::getAlphaChannel() { return checkBox_2->isChecked(); }

void SaveAs::setAlphaChannel(bool b) {
  if (b) {
    checkBox_2->setCheckState(Qt::Checked);
  } else {
    checkBox_2->setCheckState(Qt::Unchecked);
  }
}

void SaveAs::setImageFormat(int f) {
  this->setNodeName();
  comboBox_2->setCurrentIndex(f);
  previousPixFormat = f;

  this->setLineEdit();
}

void SaveAs::setFormat(int inValue) {
  this->setNodeName();
  comboBox->setCurrentIndex(inValue);
  if (comboBox->currentIndex() != 0) {
    comboBox_2->setEnabled(false);
    checkBox_2->setEnabled(false);
  } else {
    comboBox_2->setEnabled(true);
    if (comboBox_2->currentIndex() >= 4)
      checkBox_2->setEnabled(true&_enableAlphaChannel);
    else
      checkBox_2->setEnabled(false);
  }
  previousOutputFormat = inValue;

  this->setLineEdit();
}

void SaveAs::setAlphaChannelBox(int inValue) {
  comboBox_2->setCurrentIndex(inValue);
  if (comboBox_2->currentIndex() >= 4)
    checkBox_2->setEnabled(true&_enableAlphaChannel);
  else
    checkBox_2->setEnabled(false);
}

QString SaveAs::getPath() { return directory_2->currentText(); }

void SaveAs::setPaths(QStringList inPaths) {
  for (int i = 0; i < inPaths.size(); i++) {
    directory_2->addItem(inPaths.at(i));
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
  target = directory_2;
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
    directory_2->setEditText(saveName);
  }
}

int SaveAs::ok() {
  // we don't need to test the result() attribute of this object since
  // closeEvent() is only called when accepted but if you like you can test: if
  return 1;
}
