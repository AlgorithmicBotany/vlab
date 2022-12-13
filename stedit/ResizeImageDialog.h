/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#pragma once

#include <QSpinBox>
#include <QPushButton>
#include <QDialog>
class ResizeImageDialog;

class ResizeImageDialog : public QDialog {
  Q_OBJECT

public:
  ResizeImageDialog(int width, int height, QWidget *parent = 0);
  ~ResizeImageDialog();

  QSize sizeHint() const;

signals:
  void result(int width, int height);

public slots:
  void confirmed();

private slots:

private:
  QSpinBox *createSpinBox();

  // UI elements
  QSpinBox *wbox;
  QSpinBox *hbox;

  QPushButton *okButton;
  QPushButton *cancelButton;
};
