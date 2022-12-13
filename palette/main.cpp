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



/* Palette

   Main Routine

   Last Modified by: Joanne
   On Date: 14-06-01
*/

#include "palette.h"
#include <QTimer>

#ifdef __APPLE__
#include "mainwindow.h"
#endif

#define MAXW 700
#define MAXH 1200

// some functions to call from main
int parseArgumentList(int, char **, page *, QString *, SavingMode &savingMode);
int filecheck(QFile *, QString *);
void initPages(page *);

// -------------------- Main Routine
int main(int argc, char **argv) {

  Palette *P;
  page *pagelist = NULL;
  QString *mess;
  int numfiles;
  int w = 280;
  int h = 520;

  mess = new QString("");
  pagelist = new page[16];
  initPages(pagelist);

  QApplication::setColorSpec(QApplication::CustomColor);
  QApplication app(argc, argv);

  bool ok;
  if (argc > 3) {
    if (QString(argv[argc - 3]) == "-w") {
      QString ws = argv[argc - 2];
      QString hs = argv[argc - 1];
      int wi = ws.toInt(&ok, 10);
      int hi = hs.toInt(&ok, 10);
      if (!ok)
        *mess += "Usage: palette ... -w [width] [height]\n";
      if ((wi < 0) || (wi > MAXW) || (hi < 0) || (hi > MAXH)) {
        ok = false;
        *mess += "Specified dimensions not within valid range.  width: 0-";
        *mess += QString::number(MAXW);
        *mess += ", height: 0-";
        *mess += QString::number(MAXH) + "\n";
      }
      argc -= 3;
      if (ok) {
        w = wi;
        h = hi;
      }
    }
  }
  SavingMode savingMode = OFF;
  numfiles = parseArgumentList(argc, argv, pagelist, mess, savingMode);

#ifdef __APPLE__
  MainWindow *wnd = new MainWindow();
#endif

  if (numfiles == 0) {
    std::cerr << (*mess).toStdString().c_str();
    P = new Palette(mess, pagelist, savingMode);
  } else if (numfiles < 0) {
    std::cerr << (*mess).toStdString().c_str();
    P = new Palette(mess, pagelist, savingMode);
  } else
    P = new Palette(NULL, pagelist, savingMode);

#ifdef __APPLE__
  wnd->setCentralWidget(P);
  wnd->setWindowIcon(QPixmap()); // use qt3support
  wnd->setWindowTitle("Palette");
  wnd->resize(w, h);
  QObject::connect(P, SIGNAL(quit()), wnd, SLOT(close()));
  QObject::connect(wnd, SIGNAL(quit()), P, SLOT(close()));
  wnd->show();
  QTimer::singleShot(0, wnd, SLOT(raise()));
  wnd->setWindowIcon(QPixmap());
#else
  P->resize(w, h);
  P->show();
  QTimer::singleShot(0, P, SLOT(raise()));
  // On Ubuntu, the next line disables the icon
  // P->setWindowIcon(QPixmap());
#endif
  app.setWindowIcon(QPixmap(":icon.png"));
  return app.exec();
}

// ------------------ Some Functions to Call from Main
// initializes pages
void initPages(page *p) {
  for (int i = 0; i < 16; i++) {
    p[i].pg = 0;
    p[i].modified = false;
    p[i].colourfile = NULL;
    p[i].name = new QString(QString("default") + QString::number(i + 1) +
                            QString(".map"));
    p[i].colours = NULL;
  }
}

// returns number of files specified in command line
// negative if warning found
int parseArgumentList(int argc, char **argv, page *pagelist, QString *mess,
                      SavingMode &savingMode) {
  QFile *f;
  savingMode = OFF;

  // >palette
  if (argc == 1) {
    *mess += "Usage: palette -rmode [expl|trig|cont] -m[page#] file.map ...\n";
    return 0;
  }
  // >palette filename.map
  if (argc == 2) {
    f = new QFile(argv[1]);
    switch (filecheck(f, mess)) {
      // okay
    case 0:
      pagelist[0].colourfile = f;
      pagelist[0].pg = 0;
      pagelist[0].name = new QString(argv[1]);
      return 1;
      break;

      // invalid file size
    case 1:
      return 0;
      break;

      // no such file
    case 2:
      pagelist[0].name = new QString(argv[1]);
      return -1;
      break;
    }
  }

  // >palette -m[page#] filename1.map ... -m[page#] filenameX.map ...

  if (argc > 2) {
    bool err = false;
    bool ok = false;
    bool multerr = false;
    bool usage = true;
    int numfiles = 0;
    int pageno;
    QString pageflag;
    int done[16];
    for (int i = 0; i < 16; i++)
      done[i] = -1;
    int step = 2;
    for (int i = 1; i < argc; i += step) {
      pageflag = argv[i];
      if (pageflag.indexOf('-') != 0) {
        // this is a file name
        step = 1;
        f = new QFile(argv[i]);
        switch (filecheck(f, mess)) {
          // okay
        case 0:
          pagelist[0].colourfile = f;
          pagelist[0].pg = 0;
          pagelist[0].name = new QString(argv[i]);
          break;

          // invalid file size
        case 1:
          // return 0;
          break;

          // no such file
        case 2:
          pagelist[0].name = new QString(argv[i]);
          // return -1;
          break;
        }
        continue;
      } else {

        step = 2;
        if ((pageflag == "-rmode") || (pageflag == "--refreshMode")) {
          const char *opt = argv[i + 1];
          if ((strcmp(opt, "expl") == 0) || (strcmp(opt, "explicit") == 0))
            savingMode = OFF;
          if ((strcmp(opt, "cont") == 0) || (strcmp(opt, "continuous") == 0))
            savingMode = CONTINUOUS;
          if ((strcmp(opt, "trig") == 0) || (strcmp(opt, "triggered") == 0))
            savingMode = TRIGGERED;
          continue;
        } else {
          if (pageflag == "-m")
            pageno = 1;
          else {
            if (pageflag.indexOf("-m") != 0) {
              usage = false;
              continue;
            }
            pageflag.remove(0, 2);
            pageno = pageflag.toInt(&ok, 10);
            if (!ok) {
              usage = false;
              continue;
            }
            if ((pageno > 16) || (pageno < 1)) {
              usage = false;
              continue;
            }
          }

          multerr = false;
          for (int j = 0; done[j] != -1; j++) {
            if (pageno == done[j]) {
              *mess += "Multiple colour maps specified for page ";
              *mess += QString::number(pageno);
              *mess += ". All ignored except ";
              *mess += *pagelist[done[j] - 1].name;
              *mess += ".\n";
              err = true;
              multerr = true;
              break;
            }
            if (QString(argv[i + 1]) == *pagelist[done[j] - 1].name) {
              *mess += "Colour map ";
              *mess += *pagelist[done[j] - 1].name;
              *mess +=
                  " specified for multiple pages. All ignored except page ";
              *mess += QString::number(done[j]);
              *mess += ".\n";
              err = true;
              multerr = true;
              break;
            }
          }
          if (multerr)
            continue;

          f = new QFile(argv[i + 1]);

          switch (filecheck(f, mess)) {
            // ok
          case 0:
            pagelist[pageno - 1].colourfile = f;
            pagelist[pageno - 1].pg = pageno - 1;
            pagelist[pageno - 1].name = new QString(argv[i + 1]);
            done[numfiles] = pageno;
            numfiles++;
            break;
            // invalid file size
          case 1:
            err = true;
            break;
            // no such file
          case 2:
            pagelist[pageno - 1].name = new QString(argv[i + 1]);
            done[numfiles] = pageno;
            numfiles++;
            err = true;
            break;
          }
        }
      }
    }
    if (!usage)
      *mess += "Usage: palette -m[page#] file.map ...\n";
    if ((err || !usage) && numfiles)
      numfiles *= -1;
    return numfiles;
  }
  // default
  return 0;
}

// returns error code
// 0: okay
// 1: invalid size
// 2: no such file
int filecheck(QFile *f, QString *mess) {
  if (f->exists()) {
    if (f->size() == 768) {
      return 0;
    } else {
      *mess += "File ";
      *mess += f->fileName();
      *mess += " has invalid size of ";
      *mess += QString::number(f->size());
      *mess += " bytes (Colour Map must be 768 bytes).\n";
      return 1;
    }
  } else {
    *mess += "File ";
    *mess += f->fileName();
    *mess += " not found.\n";
    return 2;
  }
}

// EOF: main.cc
