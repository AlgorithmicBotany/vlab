/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#pragma once

#include "Point.h"
#include "Vector3.h"
#include <QDialog>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QCheckBox>
class ScaleWindow;

class ScaleWindow : public QDialog {
  Q_OBJECT

public:
  ScaleWindow(QWidget *parent);
  ~ScaleWindow();

  QSize sizeHint() const;

  void reset();

signals:
  void update(Vector3 scale);

public slots:

private slots:
  void confirm();
  void cancel();
  void setPreview(bool value);
  void setUniform(bool value);
  void editedX();
  void editedY();
  void editedZ();

private:
  bool preview; // Indicates whether or not to preview the changes
  bool uniform; // Indicates whether or not to scale uniformly on all axes

  QDoubleSpinBox *createSpinBox();

  // UI elements
  QDoubleSpinBox *xbox;
  QDoubleSpinBox *ybox;
  QDoubleSpinBox *zbox;

  QPushButton *okButton;
  QPushButton *cancelButton;

  QCheckBox *previewBox;
  QCheckBox *uniformBox;
};
