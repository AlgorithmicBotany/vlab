/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#pragma once

#include <QDialog>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QCheckBox>
#include "Point.h"
#include "Vector3.h"

class TurtleParametersWindow;

class TurtleParametersWindow : public QDialog {
  Q_OBJECT

public:
  TurtleParametersWindow(QWidget *parent);
  ~TurtleParametersWindow();

  QSize sizeHint() const;

signals:
  void buttonPressed();
  void updateContactPoint(Point point);
  void updateEndPoint(Point point);
  void updateHeading(Vector3 vector);
  void updateUp(Vector3 vector);
  void updateSize(double value);

public slots:
  void setContactPoint(Point point);
  void setEndPoint(Point point);
  void setHeading(Vector3 vector);
  void setUp(Vector3 vector);
  void setSize(double value);

private slots:
  void confirm();
  void cancel();
  void reset();
  void setPreview(bool value);
  void setNormalize(bool value);
  void setOrthagonalizeHeading(bool value);
  void setOrthagonalizeUp(bool value);
  void contactPointEdited();
  void endPointEdited();
  void headingEdited();
  void upEdited();
  void sizeEdited();

private:
  Point previousContactPoint; // Value of the contact point prior to editing in
                              // case the user decides to cancel or not preview
  Point previousEndPoint;     // Value of the end point prior to editing
  Vector3 previousHeading;    // Value of the heading vector prior to editing
  Vector3 previousUp;         // Value of the up vector prior to editing
  double previousSize;        // Value of size prior to editing
  bool preview;               // Indicates whether or not to preview the changes
  bool normalize; // Indicates whether or not to normalize the vectors when
                  // finished
  bool orthagonalizeHeading; // Indicates whether or not to orthagonalize the
                             // heading vector when finished
  bool orthagonalizeUp;      // Indicates whether or not to orthagonalize the up
                             // vector when finished

  QDoubleSpinBox *createSpinBox();

  // UI elements
  QDoubleSpinBox *contactPointXbox;
  QDoubleSpinBox *contactPointYbox;
  QDoubleSpinBox *contactPointZbox;
  QDoubleSpinBox *endPointXbox;
  QDoubleSpinBox *endPointYbox;
  QDoubleSpinBox *endPointZbox;
  QDoubleSpinBox *headingXbox;
  QDoubleSpinBox *headingYbox;
  QDoubleSpinBox *headingZbox;
  QDoubleSpinBox *upXbox;
  QDoubleSpinBox *upYbox;
  QDoubleSpinBox *upZbox;
  QDoubleSpinBox *sizeBox;

  QPushButton *okButton;
  QPushButton *cancelButton;
  QPushButton *resetButton;

  QCheckBox *previewBox;
  QCheckBox *normalizeBox;

  QCheckBox *orthagonalizeHeadingBox;
  QCheckBox *orthagonalizeUpBox;
};
