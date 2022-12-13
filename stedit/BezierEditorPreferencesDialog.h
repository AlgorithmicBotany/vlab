/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#pragma once

#include <stdlib.h>
#include <string>
#include <fstream>
#include "Globals.h"
#ifdef __APPLE__
#include "platform.h"
#endif
#include <QDialog>
#include <QPushButton>
#include <QGroupBox>
#include <QRadioButton>
#include <QLabel>
#include <QSlider>

using namespace std;

class BezierEditorPreferencesDialog : public QDialog {
  Q_OBJECT

public:
  BezierEditorPreferencesDialog(QWidget *parent = 0);
  ~BezierEditorPreferencesDialog();

  QSize sizeHint() const;

signals:
  void applied();

public slots:

private slots:
  void confirmed();
  void apply();
  void pickBgColour();
  void pickPointColour();
  void pickSelectedPointColour();
  void pickLineColour();
  void pickWireframeColour();
  void pickVectorColour();
  void pickPatchColour();
  void pickSelectedPatchColour();
  void reset();
  void readConfig();

private:
  void outputConfigFile();
  void setBgColourSwatch(QColor colour);
  void setPointColourSwatch(QColor colour);
  void setSelectedPointColourSwatch(QColor colour);
  void setLineColourSwatch(QColor colour);
  void setWireframeColourSwatch(QColor colour);
  void setVectorColourSwatch(QColor colour);
  void setPatchColourSwatch(QColor colour);
  void setSelectedPatchColourSwatch(QColor colour);
  QString generateSwatchStyleString(QColor colour);

  QLabel *createSwatch();
  QSlider *createSlider(int min, int max);

  QColor bgColour;
  QColor pointColour;
  QColor selectedPointColour;
  QColor lineColour;
  QColor wireframeColour;
  QColor vectorColour;
  QColor patchColour;
  QColor selectedPatchColour;

  // UI elements
  QSlider *pointSizeSlider;
  QSlider *contactPointSizeSlider;
  QSlider *lineWidthSlider;
  QSlider *wireframeWidthSlider;
  QSlider *subdivisionSamplesSlider;

  QLabel *pointSizeNum;
  QLabel *contactPointSizeNum;
  QLabel *lineWidthNum;
  QLabel *wireframeWidthNum;
  QLabel *subdivisionSamplesNum;

  QPushButton *bgColourButton;
  QPushButton *pointColourButton;
  QPushButton *selectedPointColourButton;
  QPushButton *lineColourButton;
  QPushButton *wireframeColourButton;
  QPushButton *vectorColourButton;
  QPushButton *patchColourButton;
  QPushButton *selectedPatchColourButton;

  QLabel *bgColourSwatch;
  QLabel *pointColourSwatch;
  QLabel *selectedPointColourSwatch;
  QLabel *lineColourSwatch;
  QLabel *wireframeColourSwatch;
  QLabel *vectorColourSwatch;
  QLabel *patchColourSwatch;
  QLabel *selectedPatchColourSwatch;

  QGroupBox *interpolationBox;
  QRadioButton *linearButton;
  QRadioButton *nearestButton;

  QGroupBox *displayModeBox;
  QRadioButton *wireframeButton;
  QRadioButton *surfaceButton;

  QGroupBox *projectionBox;
  QRadioButton *parallelButton;
  QRadioButton *perspectiveButton;

  QPushButton *okButton;
  QPushButton *applyButton;
  QPushButton *cancelButton;
  QPushButton *resetButton;
};
