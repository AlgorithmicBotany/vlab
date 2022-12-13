#include "mainwindow.h"
#include <QScrollArea>
#include <QApplication>

int main(int argc, char *argv[]) {
  // Q_INIT_RESOURCE(basicdrawing);
  QApplication a(argc, argv);
  a.setWindowIcon(QIcon(":/icon.png"));
  MainWindow *w = new MainWindow(argc, argv);
  w->show();
  return a.exec();
}
