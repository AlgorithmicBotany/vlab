
#include "gallery.h"

#include <QApplication>
#include <QSurfaceFormat>
#include <QScrollArea>
#include <QScrollBar>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    auto format = QSurfaceFormat::defaultFormat();
    format.setSwapInterval(0);
    QSurfaceFormat::setDefaultFormat(format);
    QApplication app(argc, argv);

     MainWindow * mainwindow = new MainWindow();
    //mainwindow->setGeometry( 100, 100, 260, 260);

    Gallery *gallery = new Gallery(mainwindow, argc,argv);
    

    
    QScrollArea *scrollArea = new QScrollArea( mainwindow );
    scrollArea->setWidgetResizable( true );
    scrollArea->setGeometry( 0, 0, gallery->width(), gallery->height());
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->verticalScrollBar()->setEnabled(false);
    scrollArea->setWidget(gallery);
    
    mainwindow->setCentralWidget(scrollArea);
    mainwindow->setGallery(gallery);
    //mainwindow->adjustSize();
    app.setWindowIcon(QIcon(":/images/icon.png"));
    gallery->show();
    mainwindow->show();
    return app.exec();
}
