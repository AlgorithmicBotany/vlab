/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#include "AdjacenciesWindow.h"

#include <QSizePolicy>
#include <QLayout>

AdjacenciesWindow::AdjacenciesWindow(Surface *surface, QWidget *parent)
    : QDialog(parent) {
  this->surface = surface;

  // Create the patch selection combo boxes
  albox = createComboBox();
  abox = createComboBox();
  arbox = createComboBox();
  lbox = createComboBox();
  cbox = createComboBox();
  rbox = createComboBox();
  blbox = createComboBox();
  bbox = createComboBox();
  brbox = createComboBox();

  // Create the adjacency selection combo boxes
  aladjbox = createCornerComboBox();
  aadjbox = createEdgeComboBox();
  aradjbox = createCornerComboBox();
  ladjbox = createEdgeComboBox();
  radjbox = createEdgeComboBox();
  bladjbox = createCornerComboBox();
  badjbox = createEdgeComboBox();
  bradjbox = createCornerComboBox();

  // Create and connect the surface viewers
  Trackball *trackball =
      new Trackball(75); // Shared trackball for all viewports
  alview = new SurfaceViewport(surface, trackball, this, Colour(1, 1, 0));
  aview = new SurfaceViewport(surface, trackball, this, Colour(1, 0, 0));
  arview = new SurfaceViewport(surface, trackball, this, Colour(1, 0, 0.5));
  lview = new SurfaceViewport(surface, trackball, this, Colour(0, 1, 0));
  cview = new SurfaceViewport(surface, trackball, this);
  rview = new SurfaceViewport(surface, trackball, this, Colour(1, 0, 1));
  blview = new SurfaceViewport(surface, trackball, this, Colour(0, 1, 1));
  bview = new SurfaceViewport(surface, trackball, this, Colour(0, 0, 1));
  brview = new SurfaceViewport(surface, trackball, this, Colour(0.5, 0, 1));
  connect(alview, SIGNAL(trackballMoved()), this, SLOT(updateAllViewports()));
  connect(aview, SIGNAL(trackballMoved()), this, SLOT(updateAllViewports()));
  connect(arview, SIGNAL(trackballMoved()), this, SLOT(updateAllViewports()));
  connect(lview, SIGNAL(trackballMoved()), this, SLOT(updateAllViewports()));
  connect(cview, SIGNAL(trackballMoved()), this, SLOT(updateAllViewports()));
  connect(rview, SIGNAL(trackballMoved()), this, SLOT(updateAllViewports()));
  connect(blview, SIGNAL(trackballMoved()), this, SLOT(updateAllViewports()));
  connect(bview, SIGNAL(trackballMoved()), this, SLOT(updateAllViewports()));
  connect(brview, SIGNAL(trackballMoved()), this, SLOT(updateAllViewports()));

  // Interface buttons
  okButton = new QPushButton("&OK", this);
  applyButton = new QPushButton("&Apply", this);
  cancelButton = new QPushButton("&Cancel", this);
  connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
  connect(applyButton, SIGNAL(clicked()), this, SIGNAL(applied()));
  connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

  // Create the layouts
  QVBoxLayout *mainLayout = new QVBoxLayout;
  QHBoxLayout *buttonLayout = new QHBoxLayout;
  QGridLayout *realLayout = new QGridLayout;
  QVBoxLayout *centerLayout = new QVBoxLayout;

  mainLayout->addLayout(realLayout);
  mainLayout->addLayout(buttonLayout);

  QFrame *cframe = new QFrame(this);
  cframe->setLayout(centerLayout);
  cframe->setFrameShape(QFrame::Box);

  realLayout->addWidget(albox, 0, 0);
  realLayout->addWidget(aladjbox, 1, 0);
  realLayout->addWidget(alview, 2, 0);
  realLayout->addWidget(abox, 0, 1);
  realLayout->addWidget(aadjbox, 1, 1);
  realLayout->addWidget(aview, 2, 1);
  realLayout->addWidget(arbox, 0, 2);
  realLayout->addWidget(aradjbox, 1, 2);
  realLayout->addWidget(arview, 2, 2);
  realLayout->addWidget(lbox, 3, 0);
  realLayout->addWidget(ladjbox, 4, 0);
  realLayout->addWidget(lview, 5, 0);
  realLayout->addWidget(cbox, 3, 1);
  realLayout->addWidget(cframe, 5, 1);
  centerLayout->addWidget(cview);
  realLayout->addWidget(rbox, 3, 2);
  realLayout->addWidget(radjbox, 4, 2);
  realLayout->addWidget(rview, 5, 2);
  realLayout->addWidget(blbox, 6, 0);
  realLayout->addWidget(bladjbox, 7, 0);
  realLayout->addWidget(blview, 8, 0);
  realLayout->addWidget(bbox, 6, 1);
  realLayout->addWidget(badjbox, 7, 1);
  realLayout->addWidget(bview, 8, 1);
  realLayout->addWidget(brbox, 6, 2);
  realLayout->addWidget(bradjbox, 7, 2);
  realLayout->addWidget(brview, 8, 2);

  connect(albox, SIGNAL(currentIndexChanged(int)), alview,
          SLOT(setCurrentPatch(int)));
  connect(abox, SIGNAL(currentIndexChanged(int)), aview,
          SLOT(setCurrentPatch(int)));
  connect(arbox, SIGNAL(currentIndexChanged(int)), arview,
          SLOT(setCurrentPatch(int)));
  connect(lbox, SIGNAL(currentIndexChanged(int)), lview,
          SLOT(setCurrentPatch(int)));
  connect(cbox, SIGNAL(currentIndexChanged(int)), cview,
          SLOT(setCurrentPatch(int)));
  connect(cbox, SIGNAL(currentIndexChanged(int)), this,
          SLOT(changeTarget(
              int))); // Change the patch whose adjacencies are being edited
  connect(rbox, SIGNAL(currentIndexChanged(int)), rview,
          SLOT(setCurrentPatch(int)));
  connect(blbox, SIGNAL(currentIndexChanged(int)), blview,
          SLOT(setCurrentPatch(int)));
  connect(bbox, SIGNAL(currentIndexChanged(int)), bview,
          SLOT(setCurrentPatch(int)));
  connect(brbox, SIGNAL(currentIndexChanged(int)), brview,
          SLOT(setCurrentPatch(int)));

  connect(albox, SIGNAL(currentIndexChanged(int)), this,
          SLOT(setALAdjacency(int)));
  connect(abox, SIGNAL(currentIndexChanged(int)), this,
          SLOT(setAAdjacency(int)));
  connect(arbox, SIGNAL(currentIndexChanged(int)), this,
          SLOT(setARAdjacency(int)));
  connect(lbox, SIGNAL(currentIndexChanged(int)), this,
          SLOT(setLAdjacency(int)));
  connect(rbox, SIGNAL(currentIndexChanged(int)), this,
          SLOT(setRAdjacency(int)));
  connect(blbox, SIGNAL(currentIndexChanged(int)), this,
          SLOT(setBLAdjacency(int)));
  connect(bbox, SIGNAL(currentIndexChanged(int)), this,
          SLOT(setBAdjacency(int)));
  connect(brbox, SIGNAL(currentIndexChanged(int)), this,
          SLOT(setBRAdjacency(int)));

  connect(aladjbox, SIGNAL(currentIndexChanged(int)), this,
          SLOT(setALAdjacency(int)));
  connect(aadjbox, SIGNAL(currentIndexChanged(int)), this,
          SLOT(setAAdjacency(int)));
  connect(aradjbox, SIGNAL(currentIndexChanged(int)), this,
          SLOT(setARAdjacency(int)));
  connect(ladjbox, SIGNAL(currentIndexChanged(int)), this,
          SLOT(setLAdjacency(int)));
  connect(radjbox, SIGNAL(currentIndexChanged(int)), this,
          SLOT(setRAdjacency(int)));
  connect(bladjbox, SIGNAL(currentIndexChanged(int)), this,
          SLOT(setBLAdjacency(int)));
  connect(badjbox, SIGNAL(currentIndexChanged(int)), this,
          SLOT(setBAdjacency(int)));
  connect(bradjbox, SIGNAL(currentIndexChanged(int)), this,
          SLOT(setBRAdjacency(int)));

  buttonLayout->addWidget(okButton);
  buttonLayout->addWidget(applyButton);
  buttonLayout->addWidget(cancelButton);

  setLayout(mainLayout);
  setWindowTitle(tr("Edit Adjacencies"));
}

AdjacenciesWindow::~AdjacenciesWindow() {}

QSize AdjacenciesWindow::sizeHint() const { return QSize(10, 10); }

// Reset the adjacency window to have the selected patch as its current patch
void AdjacenciesWindow::reset() {
  int patchIndex = surface->getSelectedPatch();

  cview->setCurrentPatch(patchIndex + 1);
  cbox->setCurrentIndex(patchIndex + 1);
}

// When the center patch is changed, the other viewports have to be updated to
// show the current adjacencies on the selected patch
void AdjacenciesWindow::changeTarget(int index) {
  alview->resetView();
  aview->resetView();
  arview->resetView();
  lview->resetView();
  cview->resetView();
  rview->resetView();
  blview->resetView();
  bview->resetView();
  brview->resetView();

  Patch currentPatch;
  if (index - 1 >= 0 && index - 1 < surface->numPatches()) {
    currentPatch = *surface->getPatch(index - 1);

    // Update all the viewports and combo boxes with the new information
    alview->setCurrentPatch((currentPatch.getAdjacency(Patch::AL)) + 1);
    albox->setCurrentIndex((currentPatch.getAdjacency(Patch::AL)) + 1);
    aladjbox->setCurrentIndex((currentPatch.getAdjacencyDirection(Patch::AL)) /
                              2);

    aview->setCurrentPatch((currentPatch.getAdjacency(Patch::A)) + 1);
    abox->setCurrentIndex((currentPatch.getAdjacency(Patch::A)) + 1);
    aadjbox->setCurrentIndex((currentPatch.getAdjacencyDirection(Patch::A)) /
                             2);

    arview->setCurrentPatch((currentPatch.getAdjacency(Patch::AR)) + 1);
    arbox->setCurrentIndex((currentPatch.getAdjacency(Patch::AR)) + 1);
    aradjbox->setCurrentIndex((currentPatch.getAdjacencyDirection(Patch::AR)) /
                              2);

    lview->setCurrentPatch((currentPatch.getAdjacency(Patch::L)) + 1);
    lbox->setCurrentIndex((currentPatch.getAdjacency(Patch::L)) + 1);
    ladjbox->setCurrentIndex((currentPatch.getAdjacencyDirection(Patch::L)) /
                             2);

    rview->setCurrentPatch((currentPatch.getAdjacency(Patch::R)) + 1);
    rbox->setCurrentIndex((currentPatch.getAdjacency(Patch::R)) + 1);
    radjbox->setCurrentIndex((currentPatch.getAdjacencyDirection(Patch::R)) /
                             2);

    blview->setCurrentPatch((currentPatch.getAdjacency(Patch::BL)) + 1);
    blbox->setCurrentIndex((currentPatch.getAdjacency(Patch::BL)) + 1);
    bladjbox->setCurrentIndex((currentPatch.getAdjacencyDirection(Patch::BL)) /
                              2);

    bview->setCurrentPatch((currentPatch.getAdjacency(Patch::B)) + 1);
    bbox->setCurrentIndex((currentPatch.getAdjacency(Patch::B)) + 1);
    badjbox->setCurrentIndex((currentPatch.getAdjacencyDirection(Patch::B)) /
                             2);

    brview->setCurrentPatch((currentPatch.getAdjacency(Patch::BR)) + 1);
    brbox->setCurrentIndex((currentPatch.getAdjacency(Patch::BR)) + 1);
    bradjbox->setCurrentIndex((currentPatch.getAdjacencyDirection(Patch::BR)) /
                              2);

  } else {
    // If no patch is selected, blank everything
    alview->setCurrentPatch(0);
    albox->setCurrentIndex(0);
    aladjbox->setCurrentIndex(0);

    aview->setCurrentPatch(0);
    abox->setCurrentIndex(0);
    aadjbox->setCurrentIndex(0);

    arview->setCurrentPatch(0);
    arbox->setCurrentIndex(0);
    aradjbox->setCurrentIndex(0);

    lview->setCurrentPatch(0);
    lbox->setCurrentIndex(0);
    ladjbox->setCurrentIndex(0);

    rview->setCurrentPatch(0);
    rbox->setCurrentIndex(0);
    radjbox->setCurrentIndex(0);

    blview->setCurrentPatch(0);
    blbox->setCurrentIndex(0);
    bladjbox->setCurrentIndex(0);

    bview->setCurrentPatch(0);
    bbox->setCurrentIndex(0);
    badjbox->setCurrentIndex(0);

    brview->setCurrentPatch(0);
    brbox->setCurrentIndex(0);
    bradjbox->setCurrentIndex(0);
  }
}

// Slots

// Auxilliary functions

// Sets the given QStringList as the patch list, with "None" prepended to use as
// the list of combo box options
void AdjacenciesWindow::setPatchList(QStringList patchNames) {
  patchList = patchNames;
  patchList.prepend("None");
  updateComboBoxes();
}

// Creates a combo box with the list of patch names in it
QComboBox *AdjacenciesWindow::createComboBox() {
  QComboBox *cb = new QComboBox();
  for (int i = 0; i < patchList.size(); i++) {
    cb->addItem(patchList.at(i));
  }
  return cb;
}

// Creates a combo box containing the colour codes for edge adjacencies
QComboBox *AdjacenciesWindow::createEdgeComboBox() {
  QComboBox *cb = new QComboBox();
  cb->addItem("Red");
  cb->addItem("Green");
  cb->addItem("Violet");
  cb->addItem("Blue");
  return cb;
}

// Creates a combo box containing the colour codes for corner adjacencies
QComboBox *AdjacenciesWindow::createCornerComboBox() {
  QComboBox *cb = new QComboBox();
  cb->addItem("Yellow");
  cb->addItem("Magenta");
  cb->addItem("Cyan");
  cb->addItem("Purple");
  return cb;
}

// Updates all combo boxes with new information in case the patch list was
// changed
void AdjacenciesWindow::updateComboBoxes() {
  albox->clear();
  abox->clear();
  arbox->clear();
  lbox->clear();
  cbox->clear();
  rbox->clear();
  blbox->clear();
  bbox->clear();
  brbox->clear();

  for (int i = 0; i < patchList.size(); i++) {
    albox->addItem(patchList.at(i));
    abox->addItem(patchList.at(i));
    arbox->addItem(patchList.at(i));
    lbox->addItem(patchList.at(i));
    cbox->addItem(patchList.at(i));
    rbox->addItem(patchList.at(i));
    blbox->addItem(patchList.at(i));
    bbox->addItem(patchList.at(i));
    brbox->addItem(patchList.at(i));
  }
}

void AdjacenciesWindow::updateAllViewports() {
  alview->update();
  aview->update();
  arview->update();
  lview->update();
  cview->update();
  rview->update();
  blview->update();
  bview->update();
  brview->update();
}

void AdjacenciesWindow::setALAdjacency(int index) {
  setAdjacency(index, Patch::AL);
}
void AdjacenciesWindow::setAAdjacency(int index) {
  setAdjacency(index, Patch::A);
}
void AdjacenciesWindow::setARAdjacency(int index) {
  setAdjacency(index, Patch::AR);
}
void AdjacenciesWindow::setLAdjacency(int index) {
  setAdjacency(index, Patch::L);
}
void AdjacenciesWindow::setRAdjacency(int index) {
  setAdjacency(index, Patch::R);
}
void AdjacenciesWindow::setBLAdjacency(int index) {
  setAdjacency(index, Patch::BL);
}
void AdjacenciesWindow::setBAdjacency(int index) {
  setAdjacency(index, Patch::B);
}
void AdjacenciesWindow::setBRAdjacency(int index) {
  setAdjacency(index, Patch::BR);
}

void AdjacenciesWindow::setAdjacency(int index, Patch::Adjacency adj) {
  if (cbox->currentIndex() > 0 && index >= 0) {
    Patch *currentPatch = surface->getPatch(cbox->currentIndex() - 1);
    if (adj == Patch::AL) {
      currentPatch->setAdjacency(albox->currentIndex() - 1, adj);
      currentPatch->setAdjacencyDirection(aladjbox->currentIndex() * 2.5, adj);
    } else if (adj == Patch::A) {
      currentPatch->setAdjacency(abox->currentIndex() - 1, adj);
      currentPatch->setAdjacencyDirection((aadjbox->currentIndex() + 1) * 1.5,
                                          adj);
    } else if (adj == Patch::AR) {
      currentPatch->setAdjacency(arbox->currentIndex() - 1, adj);
      currentPatch->setAdjacencyDirection(aradjbox->currentIndex() * 2.5, adj);
    } else if (adj == Patch::L) {
      currentPatch->setAdjacency(lbox->currentIndex() - 1, adj);
      currentPatch->setAdjacencyDirection((ladjbox->currentIndex() + 1) * 1.5,
                                          adj);
    } else if (adj == Patch::R) {
      currentPatch->setAdjacency(rbox->currentIndex() - 1, adj);
      currentPatch->setAdjacencyDirection((radjbox->currentIndex() + 1) * 1.5,
                                          adj);
    } else if (adj == Patch::BL) {
      currentPatch->setAdjacency(blbox->currentIndex() - 1, adj);
      currentPatch->setAdjacencyDirection(bladjbox->currentIndex() * 2.5, adj);
    } else if (adj == Patch::B) {
      currentPatch->setAdjacency(bbox->currentIndex() - 1, adj);
      currentPatch->setAdjacencyDirection((badjbox->currentIndex() + 1) * 1.5,
                                          adj);
    } else {
      currentPatch->setAdjacency(brbox->currentIndex() - 1, adj);
      currentPatch->setAdjacencyDirection(bradjbox->currentIndex() * 2.5, adj);
    }
  }
}
