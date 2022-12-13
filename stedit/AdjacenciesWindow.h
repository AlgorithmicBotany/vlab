/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#pragma once

#include "SurfaceViewport.h"
#include <QComboBox>
#include <QPushButton>
#include <QDialog>

class AdjacenciesWindow;

class AdjacenciesWindow : public QDialog {
  Q_OBJECT

public:
  AdjacenciesWindow(Surface *surface, QWidget *parent);
  ~AdjacenciesWindow();

  QSize sizeHint() const;

  void reset();
  void setPatchList(QStringList patchNames);

signals:
  void buttonPressed();
  void adjacenciesClosed();
  void update();
  void cancelChanges();
  void applied();

public slots:
  void updateAllViewports();

private slots:
  void changeTarget(int index);
  void setALAdjacency(int index);
  void setAAdjacency(int index);
  void setARAdjacency(int index);
  void setLAdjacency(int index);
  void setRAdjacency(int index);
  void setBLAdjacency(int index);
  void setBAdjacency(int index);
  void setBRAdjacency(int index);
  void setAdjacency(int index, Patch::Adjacency adj);

private:
  QStringList patchList;
  Surface *surface;

  // UI elements
  // Combo boxes for selecting the patch for each viewer
  QComboBox *albox;
  QComboBox *abox;
  QComboBox *arbox;
  QComboBox *lbox;
  QComboBox *cbox;
  QComboBox *rbox;
  QComboBox *blbox;
  QComboBox *bbox;
  QComboBox *brbox;

  // Combo boxes for selecting which edge of the patch will be used in the
  // adjacency
  QComboBox *aladjbox;
  QComboBox *aadjbox;
  QComboBox *aradjbox;
  QComboBox *ladjbox;
  QComboBox *radjbox;
  QComboBox *bladjbox;
  QComboBox *badjbox;
  QComboBox *bradjbox;

  // The viewports for each of the adjacent surfaces
  SurfaceViewport *alview;
  SurfaceViewport *aview;
  SurfaceViewport *arview;
  SurfaceViewport *lview;
  SurfaceViewport *cview;
  SurfaceViewport *rview;
  SurfaceViewport *blview;
  SurfaceViewport *bview;
  SurfaceViewport *brview;

  QPushButton *okButton;
  QPushButton *applyButton;
  QPushButton *cancelButton;

  QComboBox *createComboBox();
  QComboBox *createEdgeComboBox();
  QComboBox *createCornerComboBox();
  void updateComboBoxes();
};
