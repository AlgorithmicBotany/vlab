/* ******************************************************************** *
   Copyright (C) 1990-2022 University of Calgary
  
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
  
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * ******************************************************************** */




#include <iostream>
#include <QApplication>
#include <QTimer>
#include <QSplashScreen>
#include <QPainter>
//Added by qt3to4:
#include <QPixmap>
#include <sstream>
#include "version.h"

#include <iostream>

using namespace Qt;

const unsigned long splashDuration = 3500; // 1/1000 seconds

class QMouseEvent;

class MySplashScreen : public QSplashScreen
{
public:
  MySplashScreen(const QPixmap& pix, Qt::WindowFlags f)
    : QSplashScreen(pix, f)
  {
   setAttribute(Qt::WA_DeleteOnClose);
  }

protected:
  void mousePressEvent(QMouseEvent *event)
  {
    QSplashScreen::mousePressEvent(event);
    close();
  }

};

int displaySplash( int & argc, char ** & argv )
{
   QApplication app( argc, argv );
   //std::cerr<<"Display Splash"<<std::endl;
   //  QSplashScreen * splash = new QSplashScreen( QPixmap::fromImage( QImage("logo.png" )),
   //					       Qt::WStyle_StaysOnTop ); //P.F. Qt4

//     QSplashScreen * splash = new QSplashScreen( QPixmap::fromMimeSource( "logo.png" ),
//                                                Qt::WStyle_StaysOnTop ); // Qt3 version
   QSplashScreen * splash = new MySplashScreen( QPixmap( ":/logo.png" ),
                                                Qt::WindowStaysOnTopHint );

   std::ostringstream msg;
   msg << "\t\t\t\t\t\t\t\tversion " << vlab::version_string() << "\n\t\t\t\t\t\t\t\t"
       << vlab::build_info();
   //std::cerr<<msg.str()<<std::endl;
   // Qt4
   std::string str_msg = msg.str();
   //splash-> message( QString(str_msg.c_str()), Qt::AlignHCenter | Qt::AlignBottom, Qt::white ); //Qt4
   splash-> showMessage( QString(str_msg.c_str()), Qt::AlignLeft | Qt::AlignBottom, Qt::white );
   //   splash-> message( msg.str(), Qt::AlignHCenter | Qt::AlignBottom, Qt::white ); // Qt3
   splash-> show();

   QTimer::singleShot( splashDuration, & app, SLOT(quit()) );
   QObject::connect(splash, SIGNAL(destroyed()), &app, SLOT(quit()));
   return app.exec();
}

#ifdef DONT_COMPILE

extern char * vlab_logo_xpm[];
extern int vers_x, vers_y;

const unsigned long splashDuration = 3500; // 1/1000 seconds

int displaySplash( int & argc, char ** & argv )
{
   QApplication app( argc, argv );
   QPixmap pixmap( (const char **) vlab_logo_xpm);
   QPainter paint( & pixmap );
   paint.setPen( Qt::black );
   paint.setFont( QFont( "Helvetica", 14 ) );
   paint.drawText( vers_x, vers_y, ("version " + vlab::version_string()).c_str() );
   paint.setPen( Qt::white );
   paint.setFont( QFont( "Helvetica", 10 ) );
   paint.drawText( vers_x, vers_y + 20, vlab::build_info().c_str() );
   QSplashScreen * splash = new QSplashScreen( pixmap, Qt::WStyle_StaysOnTop);
   splash-> show();

   QTimer::singleShot( splashDuration, & app, SLOT(quit()) );
   return app.exec();
}

#endif

int main( int argc, char **argv )
{
  // Change by Pascal Ferraro on Mac OS 10.5 you cannot for without exec

  //   pid_t id = fork ();
  //if (id == -1 || id == 0)
  //{
      displaySplash (argc, argv);
      exit (0);
      //}
      //else
      // {
      // this is the parent - so return the shell
      // exit (0);
      // }
}
