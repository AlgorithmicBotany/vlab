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



#include "TextureEditorPreferencesDialog.h"
#include <iostream>
#include <QColorDialog>
#include <QMessageBox>
#ifdef __APPLE__
#include <unistd.h>
#endif

TextureEditorPreferencesDialog::TextureEditorPreferencesDialog(QWidget *parent)
    : QDialog(parent) {

  bgColourSwatch = new QLabel();
  bgColourSwatch->setMaximumSize(15, 15);
  bgColourSwatch->setMinimumSize(15, 15);
  pointColourSwatch = new QLabel();
  pointColourSwatch->setMaximumSize(15, 15);
  pointColourSwatch->setMinimumSize(15, 15);
  lineColourSwatch = new QLabel();
  lineColourSwatch->setMaximumSize(15, 15);
  lineColourSwatch->setMinimumSize(15, 15);

  interpolationBox = new QGroupBox("Interpolation Method");
  linearButton = new QRadioButton("Linear", interpolationBox);
  nearestButton = new QRadioButton("Nearest", interpolationBox);

  okButton = new QPushButton("&OK", this);
  applyButton = new QPushButton("&Apply", this);
  cancelButton = new QPushButton("&Cancel", this);
  resetButton = new QPushButton("&Reset", this);
  connect(okButton, SIGNAL(clicked()), this, SLOT(confirmed()));
  connect(applyButton, SIGNAL(clicked()), this, SLOT(apply()));
  connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
  connect(resetButton, SIGNAL(clicked()), this, SLOT(reset()));

  bgColourButton = new QPushButton("Background Color", this);
  connect(bgColourButton, SIGNAL(clicked()), this, SLOT(pickBgColour()));
  pointColourButton = new QPushButton("Point Color", this);
  connect(pointColourButton, SIGNAL(clicked()), this, SLOT(pickPointColour()));
  lineColourButton = new QPushButton("Line Color", this);
  connect(lineColourButton, SIGNAL(clicked()), this, SLOT(pickLineColour()));

  // Create the layouts
  QVBoxLayout *mainLayout = new QVBoxLayout;
  QHBoxLayout *prefsLayout = new QHBoxLayout;
  QHBoxLayout *buttonLayout = new QHBoxLayout;
  QGridLayout *sliderLayout = new QGridLayout;
  QGridLayout *colourLayout = new QGridLayout;
  QHBoxLayout *interpolationLayout = new QHBoxLayout;

  mainLayout->addLayout(prefsLayout);
  mainLayout->addLayout(buttonLayout);

  prefsLayout->addLayout(colourLayout);
  QFrame *divider = new QFrame();
  divider->setFrameStyle(QFrame::VLine | QFrame::Sunken);
  prefsLayout->addWidget(divider);
  prefsLayout->addLayout(sliderLayout);

  // colourLayout->addWidget(new QLabel("Click here to destroy the universe"));
  colourLayout->addWidget(bgColourButton, 0, 0);
  colourLayout->addWidget(bgColourSwatch, 0, 1);
  colourLayout->addWidget(pointColourButton, 1, 0);
  colourLayout->addWidget(pointColourSwatch, 1, 1);
  colourLayout->addWidget(lineColourButton, 2, 0);
  colourLayout->addWidget(lineColourSwatch, 2, 1);

  pointSizeSlider = new QSlider();
  pointSizeSlider->setRange(1, 20);
  pointSizeSlider->setOrientation(Qt::Horizontal);
  pointSizeSlider->setMinimumSize(100, 15);

  lineWidthSlider = new QSlider();
  lineWidthSlider->setRange(1, 20);
  lineWidthSlider->setOrientation(Qt::Horizontal);
  lineWidthSlider->setMinimumSize(100, 15);

  pointSizeNum = new QLabel();
  lineWidthNum = new QLabel();
  connect(pointSizeSlider, SIGNAL(valueChanged(int)), pointSizeNum,
          SLOT(setNum(int)));
  connect(lineWidthSlider, SIGNAL(valueChanged(int)), lineWidthNum,
          SLOT(setNum(int)));

  readConfig();
  setBgColourSwatch(bgColour);
  setPointColourSwatch(pointColour);
  setLineColourSwatch(lineColour);

  sliderLayout->addWidget(new QLabel("Point Size"), 0, 0);
  sliderLayout->addWidget(pointSizeSlider, 0, 1);
  sliderLayout->addWidget(pointSizeNum, 0, 2);
  sliderLayout->addWidget(new QLabel("Line Width"), 1, 0);
  sliderLayout->addWidget(lineWidthSlider, 1, 1);
  sliderLayout->addWidget(lineWidthNum, 1, 2);
  sliderLayout->addWidget(interpolationBox, 2, 0, 1, 3);
  interpolationBox->setLayout(interpolationLayout);
  interpolationLayout->addWidget(linearButton);
  interpolationLayout->addWidget(nearestButton);

  buttonLayout->addWidget(okButton);
  buttonLayout->addWidget(applyButton);
  buttonLayout->addWidget(cancelButton);
  buttonLayout->addWidget(resetButton);

  setLayout(mainLayout);
  setWindowTitle(tr("Texture Editor Preferences"));
}

TextureEditorPreferencesDialog::~TextureEditorPreferencesDialog() {}

QSize TextureEditorPreferencesDialog::sizeHint() const { return QSize(10, 10); }

void TextureEditorPreferencesDialog::outputConfigFile() {
  ofstream outFile;
#ifdef __APPLE__
  QString userConfigDir = Vlab::getUserConfigDir();
  userConfigDir.append("/stedit-warp.cfg");
  outFile.open(userConfigDir.toStdString().c_str());
#else
  outFile.open("stedit-warp.cfg");
#endif
  outFile << bgColour.red() << "\t" << bgColour.green() << "\t"
          << bgColour.blue() << endl;
  outFile << pointColour.red() << "\t" << pointColour.green() << "\t"
          << pointColour.blue() << endl;
  outFile << lineColour.red() << "\t" << lineColour.green() << "\t"
          << lineColour.blue() << endl;
  outFile << pointSizeSlider->value() << endl;
  outFile << lineWidthSlider->value() << endl;
  outFile << linearButton->isChecked() << endl;

  outFile.close();
}

// Slots
void TextureEditorPreferencesDialog::confirmed() {
  outputConfigFile();
  accept();
}

void TextureEditorPreferencesDialog::apply() {
  outputConfigFile();
  emit(applied());
}

void TextureEditorPreferencesDialog::pickBgColour() {
  QColor colour =
      QColorDialog::getColor(bgColour, this, "Pick Background Color",
                             QColorDialog::DontUseNativeDialog);
  if (colour.isValid()) {
    bgColour = colour;
    setBgColourSwatch(bgColour);
  }
}

void TextureEditorPreferencesDialog::pickPointColour() {
  QColor colour = QColorDialog::getColor(pointColour, this, "Pick Point Color",
                                         QColorDialog::DontUseNativeDialog);
  if (colour.isValid()) {
    pointColour = colour;
    setPointColourSwatch(pointColour);
  }
}

void TextureEditorPreferencesDialog::pickLineColour() {
  QColor colour = QColorDialog::getColor(lineColour, this, "Pick Line Color",
                                         QColorDialog::DontUseNativeDialog);
  if (colour.isValid()) {
    lineColour = colour;
    setLineColourSwatch(lineColour);
  }
}

void TextureEditorPreferencesDialog::reset() {
  int ret = QMessageBox::question(
      this, "Reset Defaults",
      "Are you sure you want to reset to default preferences?\n Your existing "
      "preferences will be lost.",
      QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);

  if (ret == QMessageBox::Ok) {
#ifdef __APPLE__
    QString userConfigDir = Vlab::getUserConfigDir();
    userConfigDir.append("/stedit-warp.cfg");
    unlink(userConfigDir.toStdString().c_str());
#else
    remove("stedit-warp.cfg");
#endif
    bgColour = QColor(0, 0, 0);
    pointColour = QColor(255, 0, 0);
    lineColour = QColor(255, 0, 0);
    pointSizeSlider->setValue(DEFAULT_POINT_SIZE);
    lineWidthSlider->setValue(DEFAULT_LINE_WIDTH);
    linearButton->setChecked(true);
    nearestButton->setChecked(false);

    bgColour = QColor(0, 0, 0);
    pointColour = QColor(255, 0, 0);
    lineColour = QColor(255, 0, 0);

    setBgColourSwatch(bgColour);
    setPointColourSwatch(pointColour);
    setLineColourSwatch(lineColour);
  }
}

QString
TextureEditorPreferencesDialog::generateSwatchStyleString(QColor colour) {
  QString styleStr = QString("QLabel { background-color : rgb(");
  QString r = QString();
  r = r.setNum(colour.red());
  QString g = QString();
  g = g.setNum(colour.green());
  QString b = QString();
  b = b.setNum(colour.blue());
  styleStr.append(r).append(", ").append(g).append(", ").append(b).append(
      "); }");
  return styleStr;
}

void TextureEditorPreferencesDialog::setBgColourSwatch(QColor colour) {
  bgColourSwatch->setStyleSheet(generateSwatchStyleString(colour));
}

void TextureEditorPreferencesDialog::setPointColourSwatch(QColor colour) {
  pointColourSwatch->setStyleSheet(generateSwatchStyleString(colour));
}

void TextureEditorPreferencesDialog::setLineColourSwatch(QColor colour) {
  lineColourSwatch->setStyleSheet(generateSwatchStyleString(colour));
}

void TextureEditorPreferencesDialog::readConfig() {
  ifstream inFile;
#ifdef __APPLE__
  QString userConfigDir = Vlab::getUserConfigDir();
  userConfigDir.append("/stedit-warp.cfg");
  inFile.open(userConfigDir.toStdString().c_str());
#else
  inFile.open("stedit-warp.cfg");
#endif
  if (inFile.is_open()) {
    int val;
    int r, g, b;

    inFile >> r >> g >> b;
    bgColour = QColor(r, g, b);
    inFile >> r >> g >> b;
    pointColour = QColor(r, g, b);
    inFile >> r >> g >> b;
    lineColour = QColor(r, g, b);
    inFile >> val;
    pointSizeSlider->setValue(val);
    pointSizeNum->setNum(val);
    inFile >> val;
    lineWidthSlider->setValue(val);
    lineWidthNum->setNum(val);
    inFile >> val;
    linearButton->setChecked(val);
    nearestButton->setChecked(!val);

    inFile.close();
  } else {
    bgColour = QColor(0, 0, 0);
    pointColour = QColor(255, 0, 0);
    lineColour = QColor(255, 0, 0);
    pointSizeSlider->setValue(DEFAULT_POINT_SIZE);
    lineWidthSlider->setValue(DEFAULT_LINE_WIDTH);
    pointSizeNum->setNum(DEFAULT_POINT_SIZE);
    lineWidthNum->setNum(DEFAULT_LINE_WIDTH);
    linearButton->setChecked(true);
    nearestButton->setChecked(false);
  }
}
