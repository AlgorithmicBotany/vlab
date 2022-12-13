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
#include "Preferences.h"
#include "common.h"
#include <QSettings>
#include <QTextStream>
#include <QColorDialog>
#include <QFontDialog>
#include <regex>
#include <fstream>

Preferences::Preferences(QWidget *parent, QString fileName)
    : QDialog(parent), _fileName(fileName) {
  QString fName = "./snapicon.cfg";
  QFile file(fName);
  if (file.open(QFile::ReadOnly | QFile::Text))
    _fileName = fName;


  setupUi(this);
  if (!_fileName.isEmpty()) {
    _fileNametmp = _fileName + QString("~");
    QFile::remove(_fileNametmp);

    QFile file(_fileName);
    if (file.open(QFile::ReadOnly | QFile::Text))
      plainTextEdit->setPlainText(file.readAll());

    // copy file to a temporary file so we can reload in case we cancel changes
    QFile::copy(_fileName, _fileNametmp);
  }
  loadConfig();

  connect(buttonBox, SIGNAL(rejected()), SLOT(cancel()));

  connect(tabWidget, SIGNAL(tabBarClicked(int)), SLOT(Save(int)));
  connect(buttonBox, SIGNAL(clicked(QAbstractButton *)),
          SLOT(Save(QAbstractButton *)));
  connect(border_color_button, SIGNAL(clicked()), SLOT(pickColor()));
  connect(background_color_button, SIGNAL(clicked()),
          SLOT(pickBackgroundColor()));

  // set spin boxes
  connect(borderWidth_spinBox, SIGNAL(valueChanged(int)),
          SLOT(set_BorderWidth(int)));
  gridLayout->setColumnStretch(0, 132);
  setAttribute(Qt::WA_DeleteOnClose);

  this->setWindowTitle("Preferences");
}

Preferences::~Preferences() {
  // delete temporary file ?
}

void Preferences::Save(QAbstractButton *button) {
  if ((QPushButton *)button == buttonBox->button(QDialogButtonBox::Cancel))
    return;
  if (!_fileName.isEmpty()) {
    QString preferences = plainTextEdit->toPlainText();
    QFile file(_fileName);
    if (file.open(QFile::WriteOnly | QFile::Text)) {
      QTextStream out(&file);
      out << preferences;
      file.close();
    }
    QFile::remove(_fileNametmp);
    QFile::copy(_fileName, _fileNametmp);

    loadConfig();
    emit configChanged();
    emit borderWidthChanged(_borderWidth);
    emit colorChanged(_color);
    emit backgroundColorChanged(_backgroundColor);
  }
}
void Preferences::Save(int index) {
  if (index == 1)
    return;
  if (!_fileName.isEmpty()) {
    QString preferences = plainTextEdit->toPlainText();
    QFile file(_fileName);
    if (file.open(QFile::WriteOnly | QFile::Text)) {
      QTextStream out(&file);
      out << preferences;
      file.close();
    }
    loadConfig();
    emit configChanged();
    emit borderWidthChanged(_borderWidth);
    emit colorChanged(_color);
    emit backgroundColorChanged(_backgroundColor);
  }
}

void Preferences::cancel() {
  QFile::remove(_fileName);
  QFile::copy(_fileNametmp, _fileName);
  loadConfig();
  emit borderWidthChanged(_borderWidth);
  emit colorChanged(_color);
  emit backgroundColorChanged(_backgroundColor);
  emit configChanged();
}

void Preferences::paste() {}

void Preferences::set_BorderWidth(int value) {
  _borderWidth = value;
  emit borderWidthChanged(_borderWidth);
  WriteColors(_fileName.toStdString());
}

void Preferences::pickColor() {
  QColorDialog *colorDialog = new QColorDialog(_color, this);
  _previousColor = _color;
  colorDialog->setOptions(QColorDialog::DontUseNativeDialog |
                          QColorDialog::ShowAlphaChannel);
  colorDialog->setCurrentColor(_color);

  connect(colorDialog, SIGNAL(currentColorChanged(QColor)),
          SLOT(setColor(QColor)));
  connect(colorDialog, SIGNAL(rejected()), SLOT(resetColor()));

  colorDialog->show();
  colorDialog->setCurrentColor(_color);
}

void Preferences::setColor(const QColor &color) {
  _color = color;
  setButtonColor(border_color_button, _color);
  emit colorChanged(_color);
  WriteColors(_fileName.toStdString());
}

void Preferences::resetColor() { setColor(_previousColor); }

void Preferences::pickBackgroundColor() {
  QColorDialog *colorDialog = new QColorDialog(_backgroundColor, this);
  _previousBackgroundColor = _backgroundColor;

  colorDialog->setModal(false);
  colorDialog->setOptions(QColorDialog::DontUseNativeDialog |
                          QColorDialog::ShowAlphaChannel);
  colorDialog->setCurrentColor(_backgroundColor);

  connect(colorDialog, SIGNAL(currentColorChanged(QColor)),
          SLOT(setBackgroundColor(QColor)));
  connect(colorDialog, SIGNAL(rejected()), SLOT(resetBackgroundColor()));

  colorDialog->show();
}

void Preferences::setBackgroundColor(const QColor &color) {
  _backgroundColor = color;
  if (_backgroundColor.alpha() < 20)
    _backgroundColor.setAlpha(20);

  setButtonColor(background_color_button, _backgroundColor);
  emit backgroundColorChanged(_backgroundColor);

  WriteColors(_fileName.toStdString());
}

void Preferences::resetBackgroundColor() {
  setBackgroundColor(_previousBackgroundColor);
}

void Preferences::setButtonColor(QPushButton *button, const QColor &color) {

  // Style sheet for the event button and labels
  button->setFixedSize(QSize(20, 20));
  std::string colorString = color.name().toStdString();
  std::string style =
      "background-color : " + colorString + ";color: " + colorString + ";";
  button->setStyleSheet(QString::fromStdString(style));
  button->setAutoFillBackground(true);
}

void Preferences::WriteColors(std::string fileName) {
  std::ofstream out;
  out.open(fileName.c_str());
  // 1st write colors
  out << "border color: " << _color.red() << " " << _color.green() << " "
      << _color.blue() << " " << _color.alpha() << "\n";
  out << "background color: " << _backgroundColor.red() << " "
      << _backgroundColor.green() << " " << _backgroundColor.blue() << " "
      << _backgroundColor.alpha() << "\n";
  // 2nd write other options

  out << "border width: " << _borderWidth << "\n";
  out.close();

  QFile file(fileName.c_str());
  if (file.open(QFile::ReadOnly | QFile::Text))
    plainTextEdit->setPlainText(file.readAll());

  emit configChanged();
}

void Preferences::loadConfig() {
  // 1s read colors
  // ReadColors(_fileName);

  // 2nd read other options
  std::string line;

  const char *bf = _fileName.toStdString().c_str();

  std::ifstream myfile(bf);
  // Attempt to open and parse the config file

  if (myfile.is_open()) {
    // Set up regexes to parse lines of the config file
    std::regex borderWidthRegex("border width: [+-]?([[:digit:]]+)");
    std::regex background_colorRegex(
        "background color: [+-]?([[:digit:]]+) [+-]?([[:digit:]]+) "
        "[+-]?([[:digit:]]+) [+-]?([[:digit:]]+)");
    std::regex colorRegex(
        "border color: [+-]?([[:digit:]]+) [+-]?([[:digit:]]+) "
        "[+-]?([[:digit:]]+) [+-]?([[:digit:]]+)");
    // When matches are found, set the config values
    while (getline(myfile, line)) {
      std::smatch matches;
      if (std::regex_match(line, matches, borderWidthRegex)) {
        _borderWidth = std::stoi(matches[1]);
      } else if (std::regex_match(line, matches, colorRegex)) {
        int r = 0, g = 0, b = 0, alpha = 0;
        char bf[40];
        char buffer[40];
        sscanf(line.c_str(), "%[^':']:%[^'\n']\n", bf, buffer);
        sscanf(buffer, "%d %d %d %d\n", &r, &g, &b, &alpha);
        _color = QColor(r, g, b, alpha);
      } else if (std::regex_match(line, matches, background_colorRegex)) {
        int r = 0, g = 0, b = 0, alpha = 0;
        char bf[40];
        char buffer[40];
        sscanf(line.c_str(), "%[^':']:%[^'\n']\n", bf, buffer);
        sscanf(buffer, "%d %d %d %d\n", &r, &g, &b, &alpha);
        _backgroundColor = QColor(r, g, b, alpha);
      }
    }
    myfile.close();
  } else {
    std::cerr << "Unable to open config file, using default configuration"
              << std::endl;
    _color = QColor(200, 0, 0);
    _borderWidth = FRAME_BORDER;
    _backgroundColor = QColor(128, 128, 128, 50);
  }

  // set Buttons
  setButtonColor(border_color_button, _color);
  setButtonColor(background_color_button, _backgroundColor);

  // set spinBoxes
  borderWidth_spinBox->setMaximum(250);
  borderWidth_spinBox->setValue(_borderWidth);
}
