/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#include "BezierEditorPreferencesDialog.h"
#include <iostream>
#include <QLayout>
#include <QColorDialog>
#include <QMessageBox>
#ifdef __APPLE__
#include <unistd.h>
#endif

BezierEditorPreferencesDialog::BezierEditorPreferencesDialog(QWidget *parent)
    : QDialog(parent) {

  bgColourSwatch = createSwatch();
  pointColourSwatch = createSwatch();
  selectedPointColourSwatch = createSwatch();
  lineColourSwatch = createSwatch();
  wireframeColourSwatch = createSwatch();
  vectorColourSwatch = createSwatch();
  patchColourSwatch = createSwatch();
  selectedPatchColourSwatch = createSwatch();

  interpolationBox = new QGroupBox("Texture Interpolation Method");
  linearButton = new QRadioButton("Linear", interpolationBox);
  nearestButton = new QRadioButton("Nearest", interpolationBox);

  displayModeBox = new QGroupBox("Surface Display Mode");
  wireframeButton = new QRadioButton("Wireframe", displayModeBox);
  surfaceButton = new QRadioButton("Textured", displayModeBox);

  projectionBox = new QGroupBox("Projection");
  parallelButton = new QRadioButton("Parallel", projectionBox);
  perspectiveButton = new QRadioButton("Perspective", projectionBox);

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
  selectedPointColourButton = new QPushButton("Selected Point Color", this);
  connect(selectedPointColourButton, SIGNAL(clicked()), this,
          SLOT(pickSelectedPointColour()));
  lineColourButton = new QPushButton("Line Color", this);
  connect(lineColourButton, SIGNAL(clicked()), this, SLOT(pickLineColour()));

  wireframeColourButton = new QPushButton("Wireframe Color", this);
  connect(wireframeColourButton, SIGNAL(clicked()), this,
          SLOT(pickWireframeColour()));
  vectorColourButton = new QPushButton("Vector Color", this);
  connect(vectorColourButton, SIGNAL(clicked()), this,
          SLOT(pickVectorColour()));
  patchColourButton = new QPushButton("Patch Color", this);
  connect(patchColourButton, SIGNAL(clicked()), this, SLOT(pickPatchColour()));
  selectedPatchColourButton = new QPushButton("Selected Patch Color", this);
  connect(selectedPatchColourButton, SIGNAL(clicked()), this,
          SLOT(pickSelectedPatchColour()));

  // Create the layouts
  QVBoxLayout *mainLayout = new QVBoxLayout;
  QHBoxLayout *prefsLayout = new QHBoxLayout;
  QHBoxLayout *buttonLayout = new QHBoxLayout;
  QGridLayout *sliderLayout = new QGridLayout;
  QGridLayout *colourLayout = new QGridLayout;
  QHBoxLayout *interpolationLayout = new QHBoxLayout;
  QHBoxLayout *displayModeLayout = new QHBoxLayout;
  QHBoxLayout *projectionLayout = new QHBoxLayout;

  mainLayout->addLayout(prefsLayout);
  mainLayout->addLayout(buttonLayout);

  prefsLayout->addLayout(colourLayout);
  QFrame *divider = new QFrame();
  divider->setFrameStyle(QFrame::VLine | QFrame::Sunken);
  prefsLayout->addWidget(divider);
  prefsLayout->addLayout(sliderLayout);

  colourLayout->addWidget(bgColourButton, 0, 0);
  colourLayout->addWidget(bgColourSwatch, 0, 1);
  colourLayout->addWidget(pointColourButton, 1, 0);
  colourLayout->addWidget(pointColourSwatch, 1, 1);
  colourLayout->addWidget(selectedPointColourButton, 2, 0);
  colourLayout->addWidget(selectedPointColourSwatch, 2, 1);
  colourLayout->addWidget(lineColourButton, 3, 0);
  colourLayout->addWidget(lineColourSwatch, 3, 1);
  colourLayout->addWidget(wireframeColourButton, 4, 0);
  colourLayout->addWidget(wireframeColourSwatch, 4, 1);
  colourLayout->addWidget(vectorColourButton, 5, 0);
  colourLayout->addWidget(vectorColourSwatch, 5, 1);
  colourLayout->addWidget(patchColourButton, 6, 0);
  colourLayout->addWidget(patchColourSwatch, 6, 1);
  colourLayout->addWidget(selectedPatchColourButton, 7, 0);
  colourLayout->addWidget(selectedPatchColourSwatch, 7, 1);

  pointSizeSlider = createSlider(1, 20);
  contactPointSizeSlider = createSlider(1, 20);
  lineWidthSlider = createSlider(1, 20);
  wireframeWidthSlider = createSlider(1, 20);
  subdivisionSamplesSlider = createSlider(1, 40);

  pointSizeNum = new QLabel();
  contactPointSizeNum = new QLabel();
  lineWidthNum = new QLabel();
  wireframeWidthNum = new QLabel();
  subdivisionSamplesNum = new QLabel();
  connect(pointSizeSlider, SIGNAL(valueChanged(int)), pointSizeNum,
          SLOT(setNum(int)));
  connect(contactPointSizeSlider, SIGNAL(valueChanged(int)),
          contactPointSizeNum, SLOT(setNum(int)));
  connect(lineWidthSlider, SIGNAL(valueChanged(int)), lineWidthNum,
          SLOT(setNum(int)));
  connect(wireframeWidthSlider, SIGNAL(valueChanged(int)), wireframeWidthNum,
          SLOT(setNum(int)));
  connect(subdivisionSamplesSlider, SIGNAL(valueChanged(int)),
          subdivisionSamplesNum, SLOT(setNum(int)));

  readConfig();
  setBgColourSwatch(bgColour);
  setPointColourSwatch(pointColour);
  setSelectedPointColourSwatch(selectedPointColour);
  setLineColourSwatch(lineColour);
  setWireframeColourSwatch(wireframeColour);
  setVectorColourSwatch(vectorColour);
  setPatchColourSwatch(patchColour);
  setSelectedPatchColourSwatch(selectedPatchColour);

  sliderLayout->addWidget(new QLabel("Point Size"), 0, 0);
  sliderLayout->addWidget(pointSizeSlider, 0, 1);
  sliderLayout->addWidget(pointSizeNum, 0, 2);
  sliderLayout->addWidget(new QLabel("Contact Point Size"), 1, 0);
  sliderLayout->addWidget(contactPointSizeSlider, 1, 1);
  sliderLayout->addWidget(contactPointSizeNum, 1, 2);
  sliderLayout->addWidget(new QLabel("Line Width"), 2, 0);
  sliderLayout->addWidget(lineWidthSlider, 2, 1);
  sliderLayout->addWidget(lineWidthNum, 2, 2);
  sliderLayout->addWidget(new QLabel("Wireframe Width"), 3, 0);
  sliderLayout->addWidget(wireframeWidthSlider, 3, 1);
  sliderLayout->addWidget(wireframeWidthNum, 3, 2);
  sliderLayout->addWidget(new QLabel("Subdivision Samples"), 4, 0);
  sliderLayout->addWidget(subdivisionSamplesSlider, 4, 1);
  sliderLayout->addWidget(subdivisionSamplesNum, 4, 2);
  sliderLayout->addWidget(interpolationBox, 5, 0, 1, 3);
  sliderLayout->addWidget(displayModeBox, 6, 0, 1, 3);
  sliderLayout->addWidget(projectionBox, 7, 0, 1, 3);

  interpolationBox->setLayout(interpolationLayout);
  interpolationLayout->addWidget(linearButton);
  interpolationLayout->addWidget(nearestButton);

  displayModeBox->setLayout(displayModeLayout);
  displayModeLayout->addWidget(wireframeButton);
  displayModeLayout->addWidget(surfaceButton);

  projectionBox->setLayout(projectionLayout);
  projectionLayout->addWidget(parallelButton);
  projectionLayout->addWidget(perspectiveButton);

  buttonLayout->addWidget(okButton);
  buttonLayout->addWidget(applyButton);
  buttonLayout->addWidget(cancelButton);
  buttonLayout->addWidget(resetButton);

  setLayout(mainLayout);
  setWindowTitle(tr("Bezier Editor Preferences"));
}

BezierEditorPreferencesDialog::~BezierEditorPreferencesDialog() {}

QSize BezierEditorPreferencesDialog::sizeHint() const { return QSize(10, 10); }

void BezierEditorPreferencesDialog::outputConfigFile() {
  ofstream outFile;
#ifdef __APPLE__
  QString userConfigDir = Vlab::getUserConfigDir();
  userConfigDir.append("/stedit-bezier.cfg");
  outFile.open(userConfigDir.toStdString().c_str());
#else
  outFile.open("stedit-bezier.cfg");
#endif

  outFile << bgColour.red() << "\t" << bgColour.green() << "\t"
          << bgColour.blue() << endl;
  outFile << pointColour.red() << "\t" << pointColour.green() << "\t"
          << pointColour.blue() << endl;
  outFile << selectedPointColour.red() << "\t" << selectedPointColour.green()
          << "\t" << selectedPointColour.blue() << endl;
  outFile << lineColour.red() << "\t" << lineColour.green() << "\t"
          << lineColour.blue() << endl;
  outFile << wireframeColour.red() << "\t" << wireframeColour.green() << "\t"
          << wireframeColour.blue() << endl;
  outFile << vectorColour.red() << "\t" << vectorColour.green() << "\t"
          << vectorColour.blue() << endl;
  outFile << patchColour.red() << "\t" << patchColour.green() << "\t"
          << patchColour.blue() << endl;
  outFile << selectedPatchColour.red() << "\t" << selectedPatchColour.green()
          << "\t" << selectedPatchColour.blue() << endl;
  outFile << pointSizeSlider->value() << endl;
  outFile << contactPointSizeSlider->value() << endl;
  outFile << lineWidthSlider->value() << endl;
  outFile << wireframeWidthSlider->value() << endl;
  outFile << subdivisionSamplesSlider->value() << endl;
  outFile << linearButton->isChecked() << endl;
  outFile << wireframeButton->isChecked() << endl;
  outFile << parallelButton->isChecked() << endl;

  outFile.close();
}

// Slots
void BezierEditorPreferencesDialog::confirmed() {
  outputConfigFile();
  accept();
}

void BezierEditorPreferencesDialog::apply() {
  outputConfigFile();
  emit(applied());
}

void BezierEditorPreferencesDialog::pickBgColour() {
  QColor colour =
      QColorDialog::getColor(bgColour, this, "Pick Background Color",
                             QColorDialog::DontUseNativeDialog);
  if (colour.isValid()) {
    bgColour = colour;
    setBgColourSwatch(bgColour);
  }
}

void BezierEditorPreferencesDialog::pickPointColour() {
  QColor colour = QColorDialog::getColor(pointColour, this, "Pick Point Color",
                                         QColorDialog::DontUseNativeDialog);
  if (colour.isValid()) {
    pointColour = colour;
    setPointColourSwatch(pointColour);
  }
}

void BezierEditorPreferencesDialog::pickSelectedPointColour() {
  QColor colour = QColorDialog::getColor(selectedPointColour, this,
                                         "Pick Selected Point Color",
                                         QColorDialog::DontUseNativeDialog);
  if (colour.isValid()) {
    selectedPointColour = colour;
    setSelectedPointColourSwatch(selectedPointColour);
  }
}

void BezierEditorPreferencesDialog::pickLineColour() {
  QColor colour = QColorDialog::getColor(lineColour, this, "Pick Line Color",
                                         QColorDialog::DontUseNativeDialog);
  if (colour.isValid()) {
    lineColour = colour;
    setLineColourSwatch(lineColour);
  }
}

void BezierEditorPreferencesDialog::pickWireframeColour() {
  QColor colour =
      QColorDialog::getColor(wireframeColour, this, "Pick Wireframe Color",
                             QColorDialog::DontUseNativeDialog);
  if (colour.isValid()) {
    wireframeColour = colour;
    setWireframeColourSwatch(wireframeColour);
  }
}

void BezierEditorPreferencesDialog::pickVectorColour() {
  QColor colour =
      QColorDialog::getColor(vectorColour, this, "Pick Vector Color",
                             QColorDialog::DontUseNativeDialog);
  if (colour.isValid()) {
    vectorColour = colour;
    setVectorColourSwatch(vectorColour);
  }
}

void BezierEditorPreferencesDialog::pickPatchColour() {
  QColor colour = QColorDialog::getColor(patchColour, this, "Pick Patch Color",
                                         QColorDialog::DontUseNativeDialog);
  if (colour.isValid()) {
    patchColour = colour;
    setPatchColourSwatch(patchColour);
  }
}

void BezierEditorPreferencesDialog::pickSelectedPatchColour() {
  QColor colour = QColorDialog::getColor(selectedPatchColour, this,
                                         "Pick Selected Patch Color",
                                         QColorDialog::DontUseNativeDialog);
  if (colour.isValid()) {
    selectedPatchColour = colour;
    setSelectedPatchColourSwatch(selectedPatchColour);
  }
}

void BezierEditorPreferencesDialog::reset() {
  int ret = QMessageBox::question(
      this, "Reset Defaults",
      "Are you sure you want to reset to default preferences?\n Your existing "
      "preferences will be lost.",
      QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);

  if (ret == QMessageBox::Ok) {
#ifdef __APPLE__
    QString userConfigDir = Vlab::getUserConfigDir();
    userConfigDir.append("/stedit-bezier.cfg");
    unlink(userConfigDir.toStdString().c_str());
#else
    remove("stedit-bezier.cfg");
#endif
    bgColour = QColor(0, 0, 0);
    pointColour = QColor(255, 0, 0);
    lineColour = QColor(255, 0, 0);
    pointSizeSlider->setValue(DEFAULT_POINT_SIZE);
    contactPointSizeSlider->setValue(DEFAULT_CONTACT_POINT_SIZE);
    lineWidthSlider->setValue(DEFAULT_LINE_WIDTH);
    wireframeWidthSlider->setValue(DEFAULT_WIREFRAME_WIDTH);
    subdivisionSamplesSlider->setValue(DEFAULT_SUBDIVISION_SAMPLES);
    linearButton->setChecked(true);
    nearestButton->setChecked(false);
    wireframeButton->setChecked(false);
    surfaceButton->setChecked(true);
    parallelButton->setChecked(true);
    perspectiveButton->setChecked(false);

    bgColour = QColor(0, 0, 0);
    pointColour = QColor(255, 0, 0);
    selectedPointColour = QColor(0, 0, 255);
    lineColour = QColor(255, 0, 0);
    wireframeColour = QColor(0, 255, 0);
    vectorColour = QColor(153, 0, 204);
    patchColour = QColor(255, 255, 255);
    selectedPatchColour = QColor(255, 0, 0);

    setBgColourSwatch(bgColour);
    setPointColourSwatch(pointColour);
    setSelectedPointColourSwatch(selectedPointColour);
    setLineColourSwatch(lineColour);
    setWireframeColourSwatch(wireframeColour);
    setVectorColourSwatch(vectorColour);
    setPatchColourSwatch(patchColour);
    setSelectedPatchColourSwatch(selectedPatchColour);
  }
}

QString
BezierEditorPreferencesDialog::generateSwatchStyleString(QColor colour) {
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

void BezierEditorPreferencesDialog::setBgColourSwatch(QColor colour) {
  bgColourSwatch->setStyleSheet(generateSwatchStyleString(colour));
}

void BezierEditorPreferencesDialog::setPointColourSwatch(QColor colour) {
  pointColourSwatch->setStyleSheet(generateSwatchStyleString(colour));
}

void BezierEditorPreferencesDialog::setSelectedPointColourSwatch(
    QColor colour) {
  selectedPointColourSwatch->setStyleSheet(generateSwatchStyleString(colour));
}

void BezierEditorPreferencesDialog::setLineColourSwatch(QColor colour) {
  lineColourSwatch->setStyleSheet(generateSwatchStyleString(colour));
}

void BezierEditorPreferencesDialog::setWireframeColourSwatch(QColor colour) {
  wireframeColourSwatch->setStyleSheet(generateSwatchStyleString(colour));
}

void BezierEditorPreferencesDialog::setVectorColourSwatch(QColor colour) {
  vectorColourSwatch->setStyleSheet(generateSwatchStyleString(colour));
}

void BezierEditorPreferencesDialog::setPatchColourSwatch(QColor colour) {
  patchColourSwatch->setStyleSheet(generateSwatchStyleString(colour));
}

void BezierEditorPreferencesDialog::setSelectedPatchColourSwatch(
    QColor colour) {
  selectedPatchColourSwatch->setStyleSheet(generateSwatchStyleString(colour));
}

void BezierEditorPreferencesDialog::readConfig() {
  ifstream inFile;
#ifdef __APPLE__
  QString userConfigDir = Vlab::getUserConfigDir();
  userConfigDir.append("/stedit-bezier.cfg");
  inFile.open(userConfigDir.toStdString().c_str());
#else
  inFile.open("stedit-bezier.cfg");
#endif
  if (inFile.is_open()) {
    int val;
    int r, g, b;

    inFile >> r >> g >> b;
    bgColour = QColor(r, g, b);
    inFile >> r >> g >> b;
    pointColour = QColor(r, g, b);
    inFile >> r >> g >> b;
    selectedPointColour = QColor(r, g, b);
    inFile >> r >> g >> b;
    lineColour = QColor(r, g, b);
    inFile >> r >> g >> b;
    wireframeColour = QColor(r, g, b);
    inFile >> r >> g >> b;
    vectorColour = QColor(r, g, b);
    inFile >> r >> g >> b;
    patchColour = QColor(r, g, b);
    inFile >> r >> g >> b;
    selectedPatchColour = QColor(r, g, b);
    inFile >> val;
    pointSizeSlider->setValue(val);
    pointSizeNum->setNum(val);
    inFile >> val;
    contactPointSizeSlider->setValue(val);
    contactPointSizeNum->setNum(val);
    inFile >> val;
    lineWidthSlider->setValue(val);
    lineWidthNum->setNum(val);
    inFile >> val;
    wireframeWidthSlider->setValue(val);
    wireframeWidthNum->setNum(val);
    inFile >> val;
    subdivisionSamplesSlider->setValue(val);
    subdivisionSamplesNum->setNum(val);
    inFile >> val;
    linearButton->setChecked(val);
    nearestButton->setChecked(!val);
    inFile >> val;
    wireframeButton->setChecked(val);
    surfaceButton->setChecked(!val);
    inFile >> val;
    parallelButton->setChecked(val);
    perspectiveButton->setChecked(!val);

    inFile.close();
  } else {
    bgColour = QColor(0, 0, 0);
    pointColour = QColor(255, 0, 0);
    selectedPointColour = QColor(0, 0, 255);
    lineColour = QColor(255, 0, 0);
    wireframeColour = QColor(0, 255, 0);
    vectorColour = QColor(153, 0, 204);
    patchColour = QColor(255, 255, 255);
    selectedPatchColour = QColor(255, 0, 0);
    pointSizeSlider->setValue(DEFAULT_POINT_SIZE);
    contactPointSizeSlider->setValue(DEFAULT_CONTACT_POINT_SIZE);
    lineWidthSlider->setValue(DEFAULT_LINE_WIDTH);
    wireframeWidthSlider->setValue(DEFAULT_WIREFRAME_WIDTH);
    subdivisionSamplesSlider->setValue(DEFAULT_SUBDIVISION_SAMPLES);
    pointSizeNum->setNum(DEFAULT_POINT_SIZE);
    contactPointSizeNum->setNum(DEFAULT_CONTACT_POINT_SIZE);
    lineWidthNum->setNum(DEFAULT_LINE_WIDTH);
    wireframeWidthNum->setNum(DEFAULT_WIREFRAME_WIDTH);
    subdivisionSamplesNum->setNum(DEFAULT_SUBDIVISION_SAMPLES);
    linearButton->setChecked(true);
    nearestButton->setChecked(false);
    wireframeButton->setChecked(true);
    surfaceButton->setChecked(false);
    parallelButton->setChecked(true);
    perspectiveButton->setChecked(false);
  }
}

QLabel *BezierEditorPreferencesDialog::createSwatch() {
  QLabel *swatch = new QLabel();
  swatch->setMaximumSize(15, 15);
  swatch->setMinimumSize(15, 15);
  return swatch;
}

QSlider *BezierEditorPreferencesDialog::createSlider(int min, int max) {
  QSlider *slider = new QSlider();
  slider->setRange(min, max);
  slider->setOrientation(Qt::Horizontal);
  slider->setMinimumSize(100, 15);
  return slider;
}
