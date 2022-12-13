/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#pragma once

#include <QDialog>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include "Point.h"
#include "Vector3.h"

class TranslateWindow;

class TranslateWindow : public QDialog {
  Q_OBJECT

public:
  TranslateWindow(QWidget *parent);
  ~TranslateWindow();

  QSize sizeHint() const;

  void reset();

signals:
  void update(Vector3 translation);

public slots:

private slots:
  void confirm();
  void cancel();
  void setPreview(bool value);
  void edited();

private:
  bool preview; // Indicates whether or not to preview the changes

  QDoubleSpinBox *createSpinBox();

  // UI elements
  QDoubleSpinBox *xbox;
  QDoubleSpinBox *ybox;
  QDoubleSpinBox *zbox;

  QPushButton *okButton;
  QPushButton *cancelButton;

  QCheckBox *previewBox;
};
