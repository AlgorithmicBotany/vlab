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
#include <QSlider>
#include <QGroupBox>
#include <QRadioButton>
#include <QIcon>
#include <QLayout>
#include <QDialog>
#include <QLabel>
#include <QPushButton>
using namespace std;

class TextureEditorPreferencesDialog;

class TextureEditorPreferencesDialog : public QDialog {
  Q_OBJECT

public:
  TextureEditorPreferencesDialog(QWidget *parent = 0);
  ~TextureEditorPreferencesDialog();

  QSize sizeHint() const;

signals:
  void applied();

public slots:

private slots:
  void confirmed();
  void apply();
  void pickBgColour();
  void pickPointColour();
  void pickLineColour();
  void reset();
  void readConfig();

private:
  void outputConfigFile();
  void setBgColourSwatch(QColor colour);
  void setPointColourSwatch(QColor colour);
  void setLineColourSwatch(QColor colour);
  QString generateSwatchStyleString(QColor colour);

  QColor bgColour;
  QColor pointColour;
  QColor lineColour;

  // UI elements
  QSlider *pointSizeSlider;
  QSlider *lineWidthSlider;

  QLabel *pointSizeNum;
  QLabel *lineWidthNum;

  QPushButton *bgColourButton;
  QPushButton *pointColourButton;
  QPushButton *lineColourButton;

  QLabel *bgColourSwatch;
  QLabel *pointColourSwatch;
  QLabel *lineColourSwatch;

  QGroupBox *interpolationBox;
  QRadioButton *linearButton;
  QRadioButton *nearestButton;

  QPushButton *okButton;
  QPushButton *applyButton;
  QPushButton *cancelButton;
  QPushButton *resetButton;
};
