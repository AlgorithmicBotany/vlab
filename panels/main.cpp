#include "panel.h"
#include <QTimer>
#include <QScreen>
#include <QScrollArea>
#include <QStatusBar>
#include <QMenu>
#include <QMenuBar>

// -------------------- Main Routine
int main(int argc, char **argv) {
  QApplication::setColorSpec(QApplication::CustomColor);
  QApplication app(argc, argv);
  app.setWindowIcon(QIcon(":/images/icon.png"));
  Panel *wnd = new Panel(argc, argv);
  if (wnd->cantreadfile())
    return 0;
  wnd->setStyleSheet("QMainWindow {background: 'black';}");

  QMenuBar *menu = wnd->menuBar();
  QMenu *help = menu->addMenu("Help");
  QMenu *about = menu->addMenu("About");
  QAction *qHelp=help->addAction("Quick help",wnd,SLOT(quickHelp()));
  qHelp->setEnabled(true);
  //help->addAction("Medit Help", this, SLOT(help()));
  help->addAction("Tools manual", wnd, SLOT(pdfHelp()));

  //help->addAction("Help PDF", wnd, SLOT(help()));
  about->addAction("About", wnd, SLOT(about()));
  wnd->show();
  wnd->setEdited(false);

  return app.exec();
}
