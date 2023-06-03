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


#include "about.h"
#include <QTextStream>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QCloseEvent>
#include <QApplication>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLFunctions_4_3_Core>

#include <QOpenGLBuffer>

#include <QScreen>
#include "resources.h"
#include <QDesktopServices>
#include <QDir>
#include <QTextBrowser>
#include <QTextStream>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <iostream>
#include <QPushButton>


using namespace Qt;

#include "panel.h"
#include "items.h"
#include "itemdialogs.h"
#include "paneledit.h"
#include <cmath>

Panel::Panel(int ac, char **av, SavingMode savingMode)
    : QMainWindow(), currpage(NULL), menu(NULL), pagemenu(NULL), messmenu(NULL),
      editmenu(NULL), filemenu(NULL), desktop(NULL), file(NULL),
      mainWindowSize(NULL), mainWindowLocation(NULL) {
  _savingMode = savingMode;

  mode = "EXEC";

  desktop = QApplication::desktop();
  first = true;
  menufirst = false;
  modified = false;
  adding = false;
  fontset = false;
  snapon = false;
  set = false;

  bgColour[0] = 0.0;
  bgColour[1] = 0.0;
  bgColour[2] = 0.0;
  _glWidget = new GLWidget(this, qRgb(bgColour[0], bgColour[1], bgColour[2]));

  updateColourScheme();
  gridsize = 15;

  sliderdialog = new SliderDialog(NULL, NULL);
  buttondialog = new ButtonDialog(this, NULL);
  labeldialog = new LabelDialog(this, NULL);
  groupdialog = new GroupDialog(this, NULL);
  menudialog = new MenuDialog(this, NULL);
  pagedialog = new PageDialog(this, NULL);
  editor = new PanelEdit(this);
  setCentralWidget(_glWidget);

  loadconfig();
  bool ok = parseargs(ac, av);
  _cantread = false;
  if (!ok)
    _cantread = true;
  setWindowTitle(name);
  if (mainWindowLocation != NULL)
    move(*mainWindowLocation);
}

bool Panel::parseargs(int argc, char **argv) {
  QString W;

  //int wscr = getDesktopWidth();
  //int hscr = getDesktopHeight();
  QDesktopWidget widget;
  QRect mainScreenSize = widget.availableGeometry(widget.primaryScreen());
  int wscr = mainScreenSize.width();
  int hscr = mainScreenSize.height();

  int xpos = -1;
  int ypos = -1;
  int xsize = 0;
  int ysize = 0;

  _savingMode = NONE;
  if (argc == 1) {
    xpos = 0;
    ypos = 0;
    mainWindowLocation = new QPoint(xpos, ypos);
    QString filename = QFileDialog::getSaveFileName(NULL, QString("Save Function"),"noname.pnl");
    if (filename.isEmpty())
      return false;
    bool panelok = loadpanel(filename);
    return panelok;
  }
  char *panelFileName = (char*) "noname.pnl";
  while (--argc > 0) {
    if ((*++argv)[0] == '-') {
      if (!strcmp(argv[0], "-rmode") || !strcmp(argv[0], "-refreshMode")) {
        const char *opt = *++argv;
        if (strcmp(opt, "expl") == 0 || strcmp(opt, "explicit") == 0)
          _savingMode = NONE;
        if (strcmp(opt, "cont") == 0 || strcmp(opt, "continuous") == 0)
          _savingMode = CONTINUOUS;
        if (strcmp(opt, "trig") == 0 || strcmp(opt, "triggered") == 0)
          _savingMode = TRIGGERED;
        --argc;
      } else if (!strcmp(argv[0], "-wp")) {
        /* window position */
        xpos = atoi(*++argv);
        ypos = atoi(*++argv);

        --argc;
        --argc;
      } else if (!strcmp(argv[0], "-wpr")) {
        float xr, yr;
        xr = atof(*++argv);
        yr = atof(*++argv);
        --argc;
        --argc;
        if ((xr > 1.f) && (yr > 1.f)) {
          xpos = xr * wscr / 100.f;
          ypos = yr * hscr / 100.f;
        } else {
          xpos = xr * wscr;
          ypos = yr * hscr;
        }
      } else if (!strcmp(argv[0], "-wr")) {
        float xr, yr;
        xr = atof(*++argv);
        yr = atof(*++argv);
        --argc;
        --argc;
        xsize = xr * wscr;
        ysize = yr * hscr;
      } else if (!strcmp(argv[0], "-ws")) {
        xsize = atof(*++argv);
        ysize = atof(*++argv);
        --argc;
        --argc;
      }
    } else {
      panelFileName = argv[0];
      --argc;
    }
  }
  bool panelok;
  if (!strcmp(panelFileName, "")){
    //panelFileName = "";
    panelok = loadpanel("noname.pnl");
  }
  else
    panelok = loadpanel(panelFileName);
  if (!panelok)
    return false;

  if (xsize != 0 && ysize != 0)
    mainWindowSize = new QSize(xsize, ysize);
  if (xpos != -1 && ypos != -1)
    mainWindowLocation = new QPoint(xpos, ypos);

  return true;
}

void Panel::loadconfig() {
  font.setFamily("Arial");
  font.setFixedPitch(false);
  font.setPointSize(14);
}

void Panel::saveconfig() {
  if (configfile->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    QTextStream data(configfile);
    data << "font: " << font.rawName() << "\n";
    configfile->close();
  }
}

void Panel::saveDefault() {
  QMessageBox::StandardButton resBtn = QMessageBox::question( this, "Panels",tr("Do you want to save the current configuration as a default?\n"),
    QMessageBox::Yes | QMessageBox::No,
    QMessageBox::Yes);
  if (resBtn != QMessageBox::Yes)
    {
      return;
    } 
  
  int curpageNb = 0;
  while (currpage != pages[curpageNb])
    curpageNb++;

  for (unsigned int i = 0; i < pages.size(); ++i) {
    pages[i]->saveDefaultValue();
  }

  QWidget::window()->setWindowTitle(name);
  currpage = pages[curpageNb];
  setupMenu();

  broadcastall();
  _glWidget->update();
  save();

}


// 2018 - we have gotten rid of non-windowsformat. We are just reading barebone
// index-mode panel information (old .l's), but we will always save as windows
// formats at the end of the day (irregardless of file type)
QString Panel::read() {
  if (!file->exists())
    return "panel " + file->fileName() + " does not exist. Create?";
  if (file->exists() && file->open(QIODevice::ReadOnly)) {
    QTextStream data(file);
    QString heading, type, nm, mess;
    int d1, d2, v1, v2, v3;
    GLfloat col1[3], col2[3], col3[3];
    QString s;

    // dummy integers that used to hold colour in index mode - we use them to
    // read data into them but their values are not being used anymore
    int c1, c2, c3;

    // reset fields
    name = "";
    font.setFamily("Arial");
    font.setFixedPitch(false);
    font.setPointSize(14);

    if (loadwindowsformat) {
      // if we are loading a windows format file the first line will read
      // "target:" instead of "panel name:" we need to save this to re-save the
      // file later so we can't just throw it away but we need to ignore it to
      // build our format of panel
      s = data.readLine();
      QStringList split =
          s.split(":", QString::SkipEmptyParts, Qt::CaseInsensitive);
      if (split.size() < 1)
        return "Expecting first line to read \"target: <target>\", not \"" + s +
               "\"";
      heading = split.at(0);
      if (heading.compare("target", Qt::CaseSensitive) != 0)
        return "Expecting first line to read \"target:\" not \"" + heading +
               "\"";
      target = s;
      name = data.readLine();
      name.remove(0, 12);
      QWidget::window()->setWindowTitle(name);
      data >> heading;

      if (heading == "background:") {
        QString rgbdecoder;
        data >> rgbdecoder;
        QStringList decoded =
            rgbdecoder.split(",", QString::SkipEmptyParts, Qt::CaseInsensitive);
        bgColour[0] = decoded.at(0).toFloat() / 255.0;
        bgColour[1] = decoded.at(1).toFloat() / 255.0;
        bgColour[2] = decoded.at(2).toFloat() / 255.0;

        updateColourScheme();
        data >> heading;
      }
      if (heading == "size:") {
        data >> d1 >> d2 ; data.flush();

        d1 = d1 < MINLONG ? MINLONG : d1;
        d2 = d2 < MINHIGH ? MINHIGH : d2;
        d1 = d1 > getDesktopWidth() ? getDesktopWidth() : d1;
        d2 = d2 > getDesktopHeight() ? getDesktopHeight() : d2;
        size = QSize(d1, d2);

        data >> heading;
      }
      if (heading == "font:") {
        s = data.readLine();
        s.remove(0, 1);
        std::string str = s.toStdString();
        font.fromString(s);
        fontset = true;
        // error check for valid font ***, (warning)
        data >> heading;
      }
    } else {
      target = "L-system";
      data >> heading >> s;
      heading += " " + s;
      if (heading != "panel name:")
        return "Expecting panel identifier \"panel name:\", not \"" + heading +
               "\"";
      name = data.readLine();
      name.remove(0, 1);
      QWidget::window()->setWindowTitle(name);
      data >> heading;
      if (heading ==
          "colourmap:") { // additional, overrides command line and config font
        data >> heading;
        data >> heading;
      }
      if (heading == "font:") { // additional, overrides config font
        s = data.readLine();
        s.remove(0, 1);
        font.fromString(s);
        fontset = true;
        // error check for valid font ***, (warning)
        data >> heading;
      }
      if (heading == "background:") {
        // read in to bgcolour to support this format, but it's not being used
        // (index-mode colouring removed as of 2018) we are using GLfloat[3]
        // bgColour for background colour now
        data >> bgcolour;
        updateColourScheme();
        data >> heading;
      }
      if (heading != "size:")
        return "Expecting panel identifier \"size:\", not \"" + heading + "\"";
      data >> d1 >> d2 ; data.flush();
      if (d1 > getDesktopWidth()) {
        d1 = getDesktopWidth();
      }
      if (d2 > getDesktopHeight()) {
        d2 = getDesktopHeight();
      }
      if (d1 < MINLONG) {
        d1 = MINLONG;
      }
      if (d2 < MINHIGH) {
        d2 = MINHIGH;
      }
      size = QSize(d1, d2);

      data >> heading;
    }

    if (heading != "type:") {
      return "Expecting identifier \"type:\", not \"" + heading + "\"";
    }
    data >> type ; data.flush();
    if (type != "PAGE") {
      pages.push_back(new Page(font, this));
      currpage = pages.back();
    }
    while (!data.atEnd()) {
      if (type == "PAGE") {
        data >> heading;
        if (heading != "name:")
          return "Expecting page identifier \"name:\", not \"" + heading + "\"";
        nm = data.readLine();
        nm.remove(0, 1);
        data >> heading;
        if (heading == "color:") {
          if (loadwindowsformat) {
            QString rgbdecoder;
            data >> rgbdecoder;
            QStringList decoded = rgbdecoder.split(",", QString::SkipEmptyParts,
                                                   Qt::CaseInsensitive);
            col1[0] = decoded.at(0).toFloat() / 255.0;
            col1[1] = decoded.at(1).toFloat() / 255.0;
            col1[2] = decoded.at(2).toFloat() / 255.0;
          } else {
            data >> c1;
            col1[0] = col1[1] = col1[2] = 1.0;
          }
          data >> heading;
        }
        if (heading != "origin:")
          return "Expecting page identifier \"origin:\", not \"" + heading +
                 "\"";
        data >> d1 >> d2;
        // bounds checking ***
        data >> heading;
        if (heading != "message:")
          return "Expecting page identifier \"message:\", not \"" + heading +
                 "\"";
        mess = data.readLine();
        mess.remove(0, 1);
        data.flush();

        // add new page
        pages.push_back(new Page(font, this, NULL, nm, col1, d1, d2, mess));
        currpage = pages.back();
      } else if (type == "SLIDER") {
        data >> heading;
        if (heading != "name:")
          return "Expecting slider identifier \"name:\", not \"" + heading +
                 "\"";
        nm = data.readLine();
        nm.remove(0, 1);
        data >> heading;
        if (heading == "colors:") {
          if (loadwindowsformat) {
            QString rgbdecoder;
            data >> rgbdecoder;
            QStringList decoded = rgbdecoder.split(",", QString::SkipEmptyParts,
                                                   Qt::CaseInsensitive);
            col1[0] = decoded.at(0).toFloat() / 255.0;
            col1[1] = decoded.at(1).toFloat() / 255.0;
            col1[2] = decoded.at(2).toFloat() / 255.0;
            data >> rgbdecoder;
            decoded = rgbdecoder.split(",", QString::SkipEmptyParts,
                                       Qt::CaseInsensitive);
            col2[0] = decoded.at(0).toFloat() / 255.0;
            col2[1] = decoded.at(1).toFloat() / 255.0;
            col2[2] = decoded.at(2).toFloat() / 255.0;
          } else {

            // read dummy data, we won't use c1 nor c2
            data >> c1 >> c2;
            // col1 and col2 will be null and we will use default colours as we
            // are reading a deprecated index-mode panel
            col1[0] = col1[1] = col1[2] = 1.0;
            col2[0] = 1.0;
            col2[1] = col2[2] = 0;
          }
          data >> heading;
        }
        if (heading != "origin:")
          return "Expecting slider identifier \"origin:\", not \"" + heading +
                 "\"";
        data >> d1 >> d2;
        if (!loadwindowsformat) {
          d2 = size.height() - d2 - SLIDERHEIGHT * 3;
        }
        data >> heading;
        if (heading != "min/max:")
          return "Expecting slider identifier \"min/max:\", not \"" + heading +
                 "\"";
        data >> v1 >> v2;
        data >> heading;
        if (heading != "value:")
          return "Expecting slider identifier \"value:\", not \"" + heading +
                 "\"";
        data >> v3;
        data >> heading;
        if (heading != "message:")
          return "Expecting slider identifier \"message:\", not \"" + heading +
                 "\"";
        mess = data.readLine();
        mess.remove(0, 1);
        data.flush();

        // add new slider
        lastslider = new Slider(currpage, this, _savingMode, nm, col1, col2, d1,
                                d2, v1, v2, v3, mess);
        currpage->addItem(lastslider);
      } else if (type == "BUTTON") {
        bool tricolor;
        data >> heading;
        if (heading != "name:")
          return "Expecting button identifier \"name:\", not \"" + heading +
                 "\"";
        nm = data.readLine();
        nm.remove(0, 1);
        data >> heading;
        if (heading == "colors:") {
          tricolor = false;

          if (loadwindowsformat) {
            QString rgbdecoder;
            data >> rgbdecoder;
            QStringList decoded = rgbdecoder.split(",", QString::SkipEmptyParts,
                                                   Qt::CaseInsensitive);
            col1[0] = decoded.at(0).toFloat() / 255.0;
            col1[1] = decoded.at(1).toFloat() / 255.0;
            col1[2] = decoded.at(2).toFloat() / 255.0;
            data >> rgbdecoder;
            decoded = rgbdecoder.split(",", QString::SkipEmptyParts,
                                       Qt::CaseInsensitive);
            col2[0] = decoded.at(0).toFloat() / 255.0;
            col2[1] = decoded.at(1).toFloat() / 255.0;
            col2[2] = decoded.at(2).toFloat() / 255.0;
            col3[0] = 0.0;
            col3[1] = 0.0;
            col3[2] = 0.0;
          } else {
            // read dummy data, we won't use c1 nor c2
            data >> c1 >> c2;
            col1[0] = col1[1] = col1[2] = 1.0;
            col2[0] = col2[1] = 0;
            col2[2] = 1.0;
            col3[0] = col3[1] = col3[2] = 0;
          }
          data >> heading;
        } else if (heading == "tricolor:") {
          tricolor = true;

          if (loadwindowsformat) {
            QString rgbdecoder;
            data >> rgbdecoder;
            QStringList decoded = rgbdecoder.split(",", QString::SkipEmptyParts,
                                                   Qt::CaseInsensitive);
            col1[0] = decoded.at(0).toFloat() / 255.0;
            col1[1] = decoded.at(1).toFloat() / 255.0;
            col1[2] = decoded.at(2).toFloat() / 255.0;
            data >> rgbdecoder;
            decoded = rgbdecoder.split(",", QString::SkipEmptyParts,
                                       Qt::CaseInsensitive);
            col2[0] = decoded.at(0).toFloat() / 255.0;
            col2[1] = decoded.at(1).toFloat() / 255.0;
            col2[2] = decoded.at(2).toFloat() / 255.0;
            data >> rgbdecoder;
            decoded = rgbdecoder.split(",", QString::SkipEmptyParts,
                                       Qt::CaseInsensitive);
            col3[0] = decoded.at(0).toFloat() / 255.0;
            col3[1] = decoded.at(1).toFloat() / 255.0;
            col3[2] = decoded.at(2).toFloat() / 255.0;
          } else {
            col1[0] = col1[1] = col1[2] = 1.0;
            col2[0] = col2[1] = 0;
            col2[2] = 1.0;
            col3[0] = col3[1] = col3[2] = 1.0;
            // read dummy data, we won't use c1, c2, nor c3
            data >> c1 >> c2 >> c3;
          }
          data >> heading;
        }
        if (heading != "origin:")
          return "Expecting button identifier \"origin:\", not \"" + heading +
                 "\"";
        data >> d1 >> d2;
        if (!loadwindowsformat) {
          d2 = size.height() - d2 - BUTTONHEIGHT;
        }
        data >> heading;
        if (heading != "value:")
          return "Expecting button identifier \"value:\", not \"" + heading +
                 "\"";
        data >> v1;
        data >> heading;
        if (heading != "message:")
          return "Expecting button identifier \"message:\", not \"" + heading +
                 "\"";
        mess = data.readLine();
        mess.remove(0, 1);
        data.flush();
        lastbutton = new Button(currpage, this, nm, col1, col2, d1, d2, v1,
                                mess, col3, tricolor);
        currpage->addItem(lastbutton);
      } else if (type == "GROUP") {
        data.skipWhiteSpace();
        heading = data.readLine();
        if (heading.indexOf("color: ") == 0) {
          if (loadwindowsformat) {
            QString rgbdecoder =
                heading.split(" ", QString::SkipEmptyParts, Qt::CaseInsensitive)
                    .at(1);
            QStringList decoded = rgbdecoder.split(",", QString::SkipEmptyParts,
                                                   Qt::CaseInsensitive);
            col1[0] = decoded.at(0).toFloat() / 255.0;
            col1[1] = decoded.at(1).toFloat() / 255.0;
            col1[2] = decoded.at(2).toFloat() / 255.0;
          } else {
            heading.remove(0, 6);
            // wont be used
            c1 = heading.toInt();
            col1[0] = col1[1] = col1[2] = 1.0;
          }
          nm = data.readLine();
	  data.skipWhiteSpace();
        } else {
          nm = heading;
        }
        // add new group
        Group *G;
        G = new Group(currpage, this, col1);
        do {
          G->addButtonName(nm);
          nm = data.readLine();
	  data.skipWhiteSpace();
        } while (nm != "ENDGROUP");
        data.flush();
        lastgroup = G;
        currpage->addItem(G);
      } else if (type == "LABEL") {
        data >> heading;
        if (heading != "name:")
          return "Expecting label identifier \"name:\", not \"" + heading +
                 "\"";
        nm = data.readLine();
        nm.remove(0, 1);
        data >> heading;
        if (heading == "color:") {
          if (loadwindowsformat) {
            QString rgbdecoder;
            data >> rgbdecoder;
            QStringList decoded = rgbdecoder.split(",", QString::SkipEmptyParts,
                                                   Qt::CaseInsensitive);
            col1[0] = decoded.at(0).toFloat() / 255.0;
            col1[1] = decoded.at(1).toFloat() / 255.0;
            col1[2] = decoded.at(2).toFloat() / 255.0;
          } else {
            // read dummy data, we won't use c1
            data >> c1;
            col1[0] = col1[1] = col1[2] = 1.0;
          }
          data >> heading;
        }
        if (heading != "origin:")
          return "Expecting label identifier \"origin:\", not \"" + heading +
                 "\"";
        data >> d1 >> d2;
        if (!loadwindowsformat) {
          d2 = size.height() - d2 - LABELHEIGHT;
        }
        data.flush();
        lastlabel = new Label(currpage, this, nm, col1, d1, d2);
        currpage->addItem(lastlabel);
      } else if (type == "MENU") {
        data >> heading;
        if (heading != "name:")
          return "Expecting menu identifier \"name:\", not \"" + heading + "\"";
        nm = data.readLine();
        nm.remove(0, 1);
        data >> heading;
        if (heading != "message:")
          return "Expecting menu identifier \"message:\", not \"" + heading +
                 "\"";
        mess = data.readLine();
        mess.remove(0, 1);
        data.flush();
        menuitems.push_back(new Menu(this, NULL, nm, mess));
      } else {
        // parse error
        if (heading == "type:")
          return "Undefined type " + type;
        else
          return "Expecting identifier \"type:\", not \"" + heading + "\"";
      }

      data >> heading;
      data >> type ; data.flush();
    }
  } else
    return "Error opening file " + file->fileName();
  file->close();

  // set window size and location
  QWidget::window()->setMinimumSize(QSize(0, 0));
  QWidget::window()->setMaximumSize(desktop->size());
  // size comes form command line prompts
  if (mainWindowSize != NULL) {
    QWidget::window()->resize(*mainWindowSize);
  }
  // size comes from config file
  else if (size.isValid()) {
    QWidget::window()->resize(size);
  }
  // compute size to fit all items
  else {
    int maxX = 0;
    int maxY = 0;
    for (int i = 1; i < currpage->numItems(); i++) {
      Item *item = currpage->getItem(i);

      maxX = fmax(maxX, item->getContour()->right());
      maxY = fmax(maxY, item->getContour()->bottom());
    }
    size = QSize(maxX + 15, maxY + 15);
  }
  QWidget::window()->setFixedSize(QWidget::window()->size());

  // assign buttons to groups
  bool found;
  for (unsigned int m = 0; m < pages.size(); m++) {
    Page *P = pages[m];
    for (int i = 0; i < P->numItems(); i++) {
      if (P->getItem(i)->getType() == "GROUP") {
        Group *G = (Group *)P->getItem(i);
        for (int j = 0; j < G->getNumNames(); j++) {
          found = false;
          for (int k = 0; k < P->numItems(); k++) {
            if (P->getItem(k)->getType() == "BUTTON" &&
                G->getNameAt(j) == P->getItem(k)->getName()) {
              G->addButton((Button *)P->getItem(k));
              found = true;
            }
          }
          if (!found) {
            return "No such button " + G->getNameAt(j) +
                   ", cannot add to group";
          }
        }
      }
    }
  }

  return "OK";
}

// We always write as windowsformat (RGB mode) as of 2018
QString Panel::write() {
  // store the current page
  Page *pageCopy = currpage;

  if (file->open(QIODevice::WriteOnly)) {

    QTextStream data(file);
    // write the panel data
    data << "target: Lsystem" << "\n";
    data << "panel name: " << name << "\n";
    data << "background: " << int(bgColour[0] * 255.0) << ","
         << int(bgColour[1] * 255.0) << "," << int(bgColour[2] * 255.0) << "\n";
    data << "size: " << size.width() << " " << size.height() << "\n";
    data << "font: " << font.toString() << "\n";
    // write all pages
    unsigned int i = 0;
    do {

      if (pages.size() > 1 || !pages[i]->getName().isEmpty()) {
        // if we're using multi pages OR if the page has been given a name,
        // print it
        currpage = pages[i];
        data << "\n";
        data << "type: " << currpage->getType() << "\n";
        data << "name: " << currpage->getName() << "\n";

        Item *pageLabel = currpage->getItem(0);

        data << "color: " << int(pageLabel->getColour1()[0] * 255.0) << ","
             << int(pageLabel->getColour1()[1] * 255.0) << ","
             << int(pageLabel->getColour1()[2] * 255.0) << "\n";

        QPoint origin = pageLabel->getOrigin();

        data << "origin: " << origin.x() << " " << origin.y() << "\n";
        data << "message: " << currpage->getMessage() << "\n";
      }
      // write all items
      for (int j = 1; j < currpage->numItems();
           j++) { // we already wrote items[0] above

        Item *item = currpage->getItem(j);
        data << "\n";
        data << "type: " << item->getType() << "\n";

        if (item->getType() == "SLIDER") {
          data << "name: " << item->getName() << "\n";

          data << "colors: " << int(item->getColour1()[0] * 255.0) << ","
               << int(item->getColour1()[1] * 255.0) << ","
               << int(item->getColour1()[2] * 255.0) << " "
               << int(item->getColour2()[0] * 255.0) << ","
               << int(item->getColour2()[1] * 255.0) << ","
               << int(item->getColour2()[2] * 255.0) << "\n";

          QPoint origin = item->getOrigin();
          data << "origin: " << origin.x() << " " << origin.y() << "\n";
          data << "min/max: " << ((Slider *)item)->getMinValue() << " "
               << ((Slider *)item)->getMaxValue() << "\n";
          data << "value: " << ((Slider *)item)->getDefaultValue() << "\n";
          data << "message: " << item->getMessage() << "\n";
        }
        if (item->getType() == "BUTTON") {
          data << "name: " << item->getName() << "\n";
          if (!((Button *)item)->isTransparent()) {

            data << "tricolor: " << int(item->getColour1()[0] * 255.0) << ","
                 << int(item->getColour1()[1] * 255.0) << ","
                 << int(item->getColour1()[2] * 255.0) << " "
                 << int(item->getColour2()[0] * 255.0) << ","
                 << int(item->getColour2()[1] * 255.0) << ","
                 << int(item->getColour2()[2] * 255.0) << " "
                 << int(((Button *)item)->getColour3()[0] * 255.0) << ","
                 << int(((Button *)item)->getColour3()[1] * 255.0) << ","
                 << int(((Button *)item)->getColour3()[2] * 255.0) << "\n";

          } else {

            data << "colors: " << int(item->getColour1()[0] * 255.0) << ","
                 << int(item->getColour1()[1] * 255.0) << ","
                 << int(item->getColour1()[2] * 255.0) << " "
                 << int(item->getColour2()[0] * 255.0) << ","
                 << int(item->getColour2()[1] * 255.0) << ","
                 << int(item->getColour2()[2] * 255.0) << "\n";
          }
          QPoint origin = item->getOrigin();

          data << "origin: " << origin.x() << " " << origin.y() << "\n";
          data << "value: " << item->getValue() << "\n";
          data << "message: " << item->getMessage() << "\n";
        }
        if (item->getType() == "LABEL") {
          data << "name: " << item->getName() << "\n";

          data << "color: " << int(item->getColour1()[0] * 255.0) << ","
               << int(item->getColour1()[1] * 255.0) << ","
               << int(item->getColour1()[2] * 255.0) << "\n";

          QPoint origin = item->getOrigin();

          data << "origin: " << origin.x() << " " << origin.y() << "\n";
        }
        if (item->getType() == "GROUP") {
          data << "color: " << int(item->getColour1()[0] * 255.0) << ","
               << int(item->getColour1()[1] * 255.0) << ","
               << int(item->getColour1()[2] * 255.0) << "\n";

          for (int k = 0; k < ((Group *)item)->getNumButtons(); k++) {
            data << ((Group *)item)->getButtonAt(k)->getName() << "\n";
          }
          data << "ENDGROUP" << "\n";
        }
      }
      i++;
    } while (i < pages.size());
    if (pages.size()) {
      if (pageCopy)
        currpage = pageCopy;
      else
        currpage = pages[0];
    }

    // write all menu items
    for (unsigned int i = 0; i < menuitems.size(); i++) {
      data << "\n";
      data << "type: " << menuitems[i]->getType() << "\n";
      data << "name: " << menuitems[i]->getName() << "\n";
      data << "message: " << menuitems[i]->getMessage() << "\n";
    }
    file->close();

    return "OK";
  }
  return ("Error opening file " + file->fileName());
}

void Panel::nothing() {
  QMessageBox::warning(
      this, "Oops...",
      "This feature is not implemented yet.\nPlease be patient.",
      "OK, but HURRY!");
}

GLfloat *Panel::getBGColour() { return bgColour; }

void Panel::removeFromGroup(Button *B) {
  B->groupIn(NULL);
  ((Group *)(currpage->getItemWithActiveDialogue()))->deleteButton(B);
}

void Panel::deletePage() {
  for (unsigned int i = 0; i < pages.size(); i++) {
    if (pages[i] == currpage) {
      pages.erase(pages.begin() + i);
      break;
    }
  }
  if (pages.size())
    currpage = pages[0];
  else {
    pages.push_back(new Page(font, this));
    currpage = pages.back();
  }
  closeOpenDialogs();
  setupMenu();

  setEdited(true);
  _glWidget->update();
}

void Panel::deleteMenuItem() {
  for (unsigned int i = 0; i < menuitems.size(); i++) {
    if (menuitems[i] == selectmenu) {
      menuitems.erase(menuitems.begin() + i);
      break;
    }
  }
  closeOpenDialogs();
  setupMenu();
  setEdited(true);
}

void Panel::deleteSelect() {
  int numSelectedItems = currpage->getSelectedItems().size();
  for (int i = 0; i < numSelectedItems; i++) {
    Item *item = currpage->getSelectedItems().back();
    currpage->getSelectedItems().pop_back();

    // take care of button groups ... they're special!
    if (item->getType() == "BUTTON") {
      Button *b = (Button *)item;
      if (b->getGroup()) {
        b->getGroup()->deleteButton(b);
        if (b->getGroup()->getNumButtons() == 0) {
          currpage->deleteItem(b->getGroup());
        }
      }
    }
    if (item->getType() == "GROUP") {
      Group *g = (Group *)item;
      for (int i = 0; i < g->getNumButtons(); i++) {
        g->getButtonAt(i)->groupIn(NULL);
      }
    }
    // label #0 is also special, don't forget ...
    if (item == currpage->getItem(0)) {
      QMessageBox::warning(this, "Page Label", "Page Label cannot be deleted.",
                           "OK");
      return;
    }

    currpage->deleteItem(item);
  }
  currpage->clearSelectedItems();
  
  _copy->setEnabled(false);
  _delete->setEnabled(false);
  _clone->setEnabled(false);
  _edititem->setEnabled(false);
  closeOpenDialogs();

  setEdited(true);
  _glWidget->update();
}

void Panel::copySelect() {
  for (unsigned int i = 0; i < copiedItems.size(); i++) {
    delete copiedItems[i];
  }
  copiedItems.clear();
  // clone all other items
  for (unsigned int i = 0; i < currpage->getSelectedItems().size(); i++) {
    Item *item = currpage->getSelectedItems()[i];
    if (item->getType() == "SLIDER") {
      Slider *s = new Slider((Slider *)item);
 
      copiedItems.push_back(s);

    } else if (item->getType() == "BUTTON") {
      Button *button = (Button *)item;

      if (!button->getGroup()->isSelected() || button->getGroup() == NULL) {
        Button *b = new Button(button);
        copiedItems.push_back(b);
      }
    } else if (item->getType() == "LABEL") {
      Label *l = new Label((Label *)item);
      copiedItems.push_back(l);

    } else if (item->getType() == "GROUP") {
      std::vector<Button *> buttonsToClone;
      for (int i = 0; i < ((Group *)item)->getNumButtons(); i++) {
        Button *bToClone = (Button *)(((Group *)item)->getButtonAt(i));
	Button *b = new Button(bToClone);
	//	b->setName(bToClone->getName().remove(bToClone->getExtension()));
	b->setName(bToClone->getName());
        b->moveBy(25, 25);
        //currpage->addItem(b);
        setEdited(true);
        b->setMode(mode);
        buttonsToClone.push_back(b);
      }

      Group *g = new Group(currpage, this);
      g->setButtons(buttonsToClone);
      copiedItems.push_back(g);
    }
  }
   _paste->setEnabled(true);
  _glWidget->update();
}

void Panel::pasteCopy() {

  std::vector<Item *> newSelectedItems;

  // clone all other items
  for (unsigned int i = 0; i < copiedItems.size(); i++) {
    Item *item = copiedItems[i];
    if (item->getType() == "SLIDER") {
      Slider *s = new Slider((Slider *)item);
      setEdited(true);
      s->moveBy(25, 25);
      s->setMode(mode);
      currpage->addItem(s);
      newSelectedItems.push_back(s);
    } else if (item->getType() == "BUTTON") {
      Button *button = (Button *)item;

      if (!button->getGroup()->isSelected() || button->getGroup() == NULL) {
        Button *b = new Button(button);
        // This will avoid a duplicated name
	// b->setName(button->getName().remove(button->getExtension()));
        b->setName(button->getName());
        b->moveBy(25, 25);
        currpage->addItem(b);
        setEdited(true);
        b->setMode(mode);
        newSelectedItems.push_back(b);
      }
    } else if (item->getType() == "LABEL") {
      Label *l = new Label((Label *)item);
      l->moveBy(25, 25);
      currpage->addItem(l);
      setEdited(true);
      l->setMode(mode);
      newSelectedItems.push_back(l);

    } else if (item->getType() == "GROUP") {
      std::vector<Button *> buttonsToClone;
      for (int i = 0; i < ((Group *)item)->getNumButtons(); i++) {
        Button *bToClone = (Button *)(((Group *)item)->getButtonAt(i));
        Button *b = new Button(bToClone);
        //b->setName(bToClone->getName().remove(bToClone->getExtension()));
        b->setName(bToClone->getName());
        b->moveBy(25, 25);
        currpage->addItem(b);
        setEdited(true);
        b->setMode(mode);
        buttonsToClone.push_back(b);
      }

      Group *g = new Group(currpage, this);
      currpage->addItem(g);
      setEdited(true);
      g->setMode(mode);
      // we removed the dialog box to set the group
      //groupdialog->setGroup(g);
      newSelectedItems.push_back(g);
      g->setButtons(buttonsToClone);
    }
  }
  currpage->clearSelectedItems();
  for (unsigned int i = 0; i < newSelectedItems.size(); i++) {
    currpage->addItemToSelectedItems(newSelectedItems[i]);
  }
  _copy->setEnabled(true);
  _delete->setEnabled(true);
  _clone->setEnabled(true);
  _edititem->setEnabled(true);
  closeOpenDialogs();

  _paste->setEnabled(true);
  _glWidget->update();
}

void Panel::cloneSelect() {

  std::vector<Item *> newSelectedItems;

  // clone all other items
  for (unsigned int i = 0; i < currpage->getSelectedItems().size(); i++) {
    Item *item = currpage->getSelectedItems()[i];
    if (item->getType() == "SLIDER") {
      Slider *s = new Slider((Slider *)item);
      s->moveBy(25, 25);
      currpage->addItem(s);

      setEdited(true);
      s->setMode(mode);
      newSelectedItems.push_back(s);

    } else if (item->getType() == "BUTTON") {
      Button *button = (Button *)item;

      if (!button->getGroup()->isSelected() || button->getGroup() == NULL) {
        Button *b = new Button(button);
        // This will avoid a duplicated name
        //b->setName(button->getName().remove(button->getExtension()));
        b->setName(button->getName());
        b->moveBy(25, 25);
        currpage->addItem(b);
        setEdited(true);
        b->setMode(mode);
        newSelectedItems.push_back(b);
      }
    } else if (item->getType() == "LABEL") {
      Label *l = new Label((Label *)item);
      l->moveBy(25, 25);
      currpage->addItem(l);
      setEdited(true);
      l->setMode(mode);
      newSelectedItems.push_back(l);
    } else if (item->getType() == "GROUP") {
      std::vector<Button *> buttonsToClone;
      for (int i = 0; i < ((Group *)item)->getNumButtons(); i++) {
        Button *bToClone = (Button *)(((Group *)item)->getButtonAt(i));
        Button *b = new Button(bToClone);
        //b->setName(bToClone->getName().remove(bToClone->getExtension()));
        b->setName(bToClone->getName());
        b->moveBy(25, 25);
        currpage->addItem(b);
        setEdited(true);
        b->setMode(mode);
        newSelectedItems.push_back(b);
        buttonsToClone.push_back(b);
      }

      Group *g = new Group(currpage, this);
      currpage->addItem(g);
      setEdited(true);
      g->setMode(mode);
      // we removed the dialog box to set the group
      //groupdialog->setGroup(g);
      newSelectedItems.push_back(g);
      g->setButtons(buttonsToClone);
    }
  }

  currpage->clearSelectedItems();
  for (unsigned int i = 0; i < newSelectedItems.size(); i++) {
    currpage->addItemToSelectedItems(newSelectedItems[i]);
  }
  _copy->setEnabled(true);

  _delete->setEnabled(true);
  _clone->setEnabled(true);
  _edititem->setEnabled(true);
  _glWidget->update();
}

void Panel::editPage() {

  setEdited(true);
  closeOpenDialogs();

  pagedialog->setPage((Page *)currpage);
}

void Panel::toggleSnap() {
  if (snapon) {
    _snap->setChecked(false);
    snapon = false;
  } else {
    _snap->setChecked(true);
    snapon = true;
  }
}

void Panel::preferences() {}

void Panel::itemDefaults() {}

void Panel::setPanelName(QString s) {
  name = s;
  QWidget::window()->setWindowTitle(name);

  setEdited(true);
}

void Panel::setBGColour(GLfloat *col) {
  if (col) {
    bgColour[0] = col[0];
    bgColour[1] = col[1];
    bgColour[2] = col[2];

    updateColourScheme();
    updateOpenDialogs();

    setEdited(true);
  }
}

void Panel::updateColourScheme() {

  // grid colour
  QColor C = QColor((int)(bgColour[0] * 255.0), (int)(bgColour[1] * 255.0),
                    (int)(bgColour[2] * 255.0));
  // set background colour for mainWindow so that it matches the QGLWidget's
  // (the panel) background colour
  QString qss = QString("QMainWindow {background:rgb(%1,%2,%3);};")
                    .arg(C.red())
                    .arg(C.green())
                    .arg(C.blue());
  window()->setStyleSheet(qss);

  GLfloat h, s, v;

  h = (C.hue() + 180) % 360;
  s = (C.saturation() + 128) % 255;
  v = (C.value() + 128) % 255;

  C.setHsv(h, s, v);
  gridvec[0] = C.red() / 255.0;
  gridvec[1] = C.green() / 255.0;
  gridvec[2] = C.blue() / 255.0;

  // select highlight colour
  C = QColor(bgColour[0] * 255.0, bgColour[1] * 255.0, bgColour[2] * 255.0);

  if (C.hue()) {
    if (C.hue() < 180.00) {
      h = 180.00 + C.hue();
    } else {
      h = C.hue() - 180.00;
    }
    C.setHsv(h, 1.00, 1.00);
    highvec[0] = C.red();
    highvec[1] = C.green();
    highvec[2] = C.blue();
  } else {
    C.setRed(255);
    C.setGreen(216);
    C.setBlue(0);
    C.setHsv(C.hue(), 1.00, 1.00);
    highvec[0] = C.red();
    highvec[1] = C.green();
    highvec[2] = C.blue();
  }
}

void Panel::setPanelWidth(int i) {
  resize(i, size.height());
  window()->resize(size);
  updateOpenDialogs();

  setEdited(true);
}

void Panel::setPanelHeight(int i) {
  resize(size.width(), i);
  window()->resize(size);
  updateOpenDialogs();

  setEdited(true);
}

void Panel::newItem(int i) {

  closeOpenDialogs();
  switch (i) {
  case 2: // label
  {
    Label *I = new Label(currpage, this);
    I->moveBy(width() / 2 - I->getContour()->width() / 2,
              height() / 2 + I->getContour()->height() / 2);

    lastlabel = I;
    currpage->addItem(I);
    setEdited(true);
    I->setMode(mode);
    labeldialog->setLabel(I);

    currpage->clearSelectedItems();
    currpage->addItemToSelectedItems((Item *)I);
    currpage->setItemWithActiveDialogue((Item *)I);

    _copy->setEnabled(true);
    _delete->setEnabled(true);
    _clone->setEnabled(true);
    _edititem->setEnabled(true);
    break;
  }
  case 0: // slider
  {
    Slider *I = new Slider(currpage, this, _savingMode, "Slider", NULL, NULL,
                           width() / 2 - SLIDERLENGTH / 2,
                           height() / 2 - SLIDERHEIGHT / 2);
 
    lastslider = I;
    currpage->addItem(I);
    setEdited(true);
    I->setMode(mode);
    sliderdialog->setSlider(I);

    currpage->clearSelectedItems();
    currpage->addItemToSelectedItems((Item *)I);
    currpage->setItemWithActiveDialogue((Item *)I);

    _copy->setEnabled(true);
    _delete->setEnabled(true);
    _clone->setEnabled(true);
    _edititem->setEnabled(true);
    break;
  }
  case 1: // button
  {
    Button *I = new Button(currpage, this, "Button", NULL, NULL,
                           width() / 2 - BUTTONLENGTH / 2,
                           height() / 2 - BUTTONHEIGHT / 2);
 
    lastbutton = I;
    currpage->addItem(I);
    setEdited(true);
    I->setMode(mode);
    buttondialog->setButton(I);

    currpage->clearSelectedItems();
    currpage->addItemToSelectedItems((Item *)I);
    currpage->setItemWithActiveDialogue((Item *)I);

    _copy->setEnabled(true);
    _delete->setEnabled(true);
    _clone->setEnabled(true);
    _edititem->setEnabled(true);
    break;
  }
  case 3: // button group
  {
    std::vector<Button *> buttonsToAdd;
    for (unsigned int i = 0; i < currpage->getSelectedItems().size(); i++) {
      if (currpage->getSelectedItems()[i]->getType() == "BUTTON") {
        buttonsToAdd.push_back((Button *)currpage->getSelectedItems()[i]);
      }
    }
    Group *I = new Group(currpage, this);
    
    lastgroup = I;
    currpage->addItem(I);
    setEdited(true);
    I->setMode(mode);
    currpage->addItemToSelectedItems((Item *)I);
    currpage->setItemWithActiveDialogue((Item *)I);
    I->setButtons(buttonsToAdd);
    groupdialog->setGroup(I);

    break;
  }
  case 4: // menu item
  {
    Menu *I = new Menu(this);
    setEdited(true);
    I->setMode(mode);
    menudialog->setMenu(I);
    menuitems.push_back(I);
    selectmenu = I;
    currpage->setItemWithActiveDialogue((Item *)I);
    setupMenu();
    break;
  }
  case 5: // page
  {
    Page *I = new Page(font, this);
    pages.push_back(I);
    currpage = I;
    I->setName("Page " + QString::number(pages.size()));
    setEdited(true);
    I->setMode(mode);
	
    pagedialog->setPage(I);
    setupMenu();
    break;
  }
  }
}

void Panel::HalignSelection(int alignment) {
  if (currpage) {
    currpage->alignSelected(alignment, "Horizontal");
  }
}

void Panel::horizontalDistributionSelected() {
  if (currpage) {
    currpage->distributeHorizontally();
  }
}

void Panel::ValignSelection(int alignment) {
  if (currpage) {
    currpage->alignSelected(alignment, "Vertical");
  }
}

void Panel::verticalDistributionSelected() {
  if (currpage) {
    currpage->distributeVertically();
  }
}

void Panel::flipItemsVerticallySelected() {
  if (currpage) {
    currpage->flipItemsVertically();
    updateOpenDialogs();
    _glWidget->update();
  }
}

void Panel::closeOpenDialogs() {
  if (sliderdialog->isVisible())
    sliderdialog->close();
  if (buttondialog->isVisible())
    buttondialog->close();
  if (labeldialog->isVisible())
    labeldialog->close();
  if (groupdialog->isVisible())
    groupdialog->close();
  if (menudialog->isVisible())
    menudialog->close();
  if (pagedialog->isVisible())
    pagedialog->close();
}

void Panel::updateOpenDialogs() {
  if (sliderdialog->isVisible())
    sliderdialog->update();
  if (buttondialog->isVisible())
    buttondialog->update();
  if (labeldialog->isVisible())
    labeldialog->update();
  if (groupdialog->isVisible())
    groupdialog->update();
  if (menudialog->isVisible())
    menudialog->update();
  if (pagedialog->isVisible())
    pagedialog->update();
}

bool Panel::thereAreOpenDialogs() {
  return ((sliderdialog->isVisible()) || (buttondialog->isVisible()) ||
          (labeldialog->isVisible()) || (groupdialog->isVisible()) ||
          (menudialog->isVisible()) || (pagedialog->isVisible()));
}

void Panel::resizeItemDialogs(QSize size) {
  sliderdialog->resize(size);
  buttondialog->resize(size);
  labeldialog->resize(size);
  groupdialog->resize(size);
  pagedialog->resize(size);
  menudialog->resize(size);
}

void Panel::moveItemDialogs(QPoint pos) {
  sliderdialog->move(pos);
  buttondialog->move(pos);
  labeldialog->move(pos);
  groupdialog->move(pos);
  pagedialog->move(pos);
  menudialog->move(pos);
}

void Panel::setDialog() {
  closeOpenDialogs();

  if (currpage->getItemWithActiveDialogue() != NULL) {

    if (currpage->getItemWithActiveDialogue()->getType() == "SLIDER") {
      sliderdialog->setSlider(
          (Slider *)(currpage->getItemWithActiveDialogue()));
      
      
    } else if (currpage->getItemWithActiveDialogue()->getType() == "BUTTON") {
      buttondialog->setButton(
          (Button *)(currpage->getItemWithActiveDialogue()));
    } else if (currpage->getItemWithActiveDialogue()->getType() == "LABEL") {
      if (currpage->getItemWithActiveDialogue() == currpage->getItem(0)) {
	
        pagedialog->setPage((Page *)currpage);
      } else {
        labeldialog->setLabel((Label *)(currpage->getItemWithActiveDialogue()));
      }
    } else if (currpage->getItemWithActiveDialogue()->getType() == "GROUP") {
      groupdialog->setGroup((Group *)(currpage->getItemWithActiveDialogue()));
    }
  } else { // "PAGE"
    pagedialog->setPage((Page *)currpage);
  }
}

void Panel::editPanel() {
  mode = "EDIT";
  modified = false;
  for (unsigned int i = 0; i < pages.size(); i++) {
    pages[i]->setMode(mode);
  }
  for (unsigned int i = 0; i < menuitems.size(); i++) {
    menuitems[i]->setMode(mode);
  }
  setupMenu();
  setMinimumSize(QSize(10, 10));
  setMaximumSize(desktop->size());
  QWidget::window()->setMinimumSize(QSize(10, 10));
  QWidget::window()->setMaximumSize(desktop->size());
  editor->editPanel();
  _glWidget->update();
}

void Panel::execPanel() {
  editor->closeEditPanel();
 
  if (currpage) {
    currpage->draw(_glWidget);
  }
}

void Panel::setDoneEditing() {
  mode = "EXEC";
  closeOpenDialogs();
  setMinimumSize(size);
  setMaximumSize(size);
  QWidget::window()->setFixedSize(QWidget::window()->size());

  for (unsigned int i = 0; i < pages.size(); i++) {
    pages[i]->setMode(mode);
  }
  for (unsigned int i = 0; i < menuitems.size(); i++) {
    menuitems[i]->setMode(mode);
  }
  setupMenu();
}

void Panel::pagemenuHandler(QAction *active) {
  for (unsigned int i = 0; i < pages.size(); i++) {
    if (pages[i]->getID() == active) {
      currpage = pages[i];
      pages[i]->getID()->setChecked(true);
      if (mode == "EXEC") {
        currpage->activate();
      } else {
        closeOpenDialogs();
      }

    } else {
      pages[i]->getID()->setChecked(false);
    }
  }
  _glWidget->update();
}

void Panel::messmenuHandler(QAction *active) {
  for (unsigned int i = 0; i < menuitems.size(); i++) {
    if (menuitems[i]->getID() == active) {
      if (mode == "EXEC") {
        menuitems[i]->activate();
      } else {
        selectmenu = menuitems[i];
        currpage->clearSelectedItems();
        closeOpenDialogs();
        menudialog->setMenu(menuitems[i]);
      }
      break;
    }
  }
}
void Panel::setfont() {

  previousFont = font;

  QFontDialog *dialog = new QFontDialog(font);
  connect(dialog, SIGNAL(currentFontChanged(QFont)), this,
          SLOT(fontSelectedFromDialog(QFont)));
  connect(dialog, SIGNAL(rejected()), this, SLOT(fontRejectedFromDialog()));

  dialog->exec();
}

void Panel::fontSelectedFromDialog(QFont f) {
  font = f;

  for (unsigned int i = 0; i < pages.size(); i++) {
    pages[i]->setFont(f);
  }
  if (mode == "EDIT") {
    editor->updatePanel();
    setEdited(true);
    fontset = true;
  }
}

void Panel::fontRejectedFromDialog() {
  font = previousFont;

  for (unsigned int i = 0; i < pages.size(); i++) {
    pages[i]->setFont(previousFont);
  }
  if (mode == "EDIT") {
    editor->updatePanel();
    setEdited(true);
    fontset = true;
  }
}

void Panel::restore() {
  int curpageNb = 0;
  while (currpage != pages[curpageNb])
    curpageNb++;

  for (unsigned int i = 0; i < pages.size(); ++i) {
    pages[i]->resetToDefaultValue();
  }

  QWidget::window()->setWindowTitle(name);
  currpage = pages[curpageNb];
  setupMenu();

  broadcastall();
  _glWidget->update();
}

void Panel::reread() {
  int curpageNb = 0;
  while (currpage != pages[curpageNb])
    curpageNb++;

  QString tempmode = mode;
  if (getModified()) {
    /*
    QMessageBox msgBox;
    msgBox.setText("The panel has unsaved changes");
    //msgBox.setInformativeText("Do you want to save your changes?");
    msgBox.setStandardButtons(QMessageBox::Ok |
                              QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Ok);
    int ret = msgBox.exec();
    */
    QMessageBox::StandardButton ret;
    ret= QMessageBox::question(this, "Test", "Do you want to save your changes?",
                                QMessageBox::Yes|QMessageBox::No |
                              QMessageBox::Cancel);

    switch (ret) {
    case QMessageBox::Ok:
      // Save was clicked
      //editor->saveEditPanel();
      break;
    case QMessageBox::Discard:
      // Don't Save was clicked
      return;
      break;
    case QMessageBox::Cancel:
      // Cancel was clicked
      return;
    default:
      // should never be reached
      break;
    }
  }

  
  if (!scrap())
    return;
  loadconfig();
  loadpanel(file->fileName());
  if ((int)(curpageNb) >= pages.size())
    curpageNb = 0;
  currpage = pages[curpageNb];

  QWidget::window()->setWindowTitle(name);
  if (tempmode == "EDIT")
    editPanel();
  _glWidget->update();
  setupMenu();
}

void Panel::reopen() {}

void Panel::broadcast() {
  for (int i = 0; i < currpage->numItems(); i++) {
    Item *item = currpage->getItem(i);
    // ------------> DO I BROADCAST MENU ITEMS ???
    if (item->getType() == "BUTTON") {
      Button *button = (Button *)item;

      if (button->getGroup() == NULL || button->getValue() == ON)
        button->processMessage();
    } else if (item->getType() == "SLIDER")
      item->processMessage();
  }
}

void Panel::broadcastall() {
  for (unsigned int j = 0; j < pages.size(); ++j) {
    for (int i = 0; i < pages[j]->numItems(); i++) {
      Item *item = pages[j]->getItem(i);
      // ------------> DO I BROADCAST MENU ITEMS ???
      if (item->getType() == "BUTTON") {
        Button *button = (Button *)item;

        if (button->getGroup() == NULL || button->getValue() == ON)
          button->processMessage();
      } else if (item->getType() == "SLIDER")
        item->processMessage();
    }
  }
}

// delete stuff, re-init stuff, reset default stuff
int Panel::scrap() {
  while (pages.size()) {
    pages.pop_back();
  }
  while (menuitems.size()) {
    menuitems.pop_back();
  }

  // if (mode == "EDIT")
  //  editor->close();

  lastslider = NULL;
  lastbutton = NULL;
  lastgroup = NULL;
  lastlabel = NULL;

  if (menu) {
    if (editmenu)
      editmenu->close();
    if (pagemenu)
      pagemenu->close();
    if (messmenu)
      messmenu->close();
    if (filemenu)
      filemenu->close();
    menu->close();
  }

  menu = NULL;
  editmenu = NULL;
  pagemenu = NULL;
  messmenu = NULL;
  filemenu = NULL;

  setEdited(false);
  bgColour[0] = bgColour[1] = bgColour[2] = 0.0;
  updateColourScheme();

  return 1;
}

void Panel::load() {
  QString fn = QFileDialog::getOpenFileName(
      this, QString::null, "Panel Files (panel.*);;All Files (*.*)");
  if (!fn.isEmpty()) {
    QString tempmode = mode;
    if (!scrap())
      return;
    loadpanel(fn);
    setWindowTitle(name);

    if (tempmode == "EDIT") {
      editPanel();
    }
  }
}

void Panel::saveslot(QString inName) {
  if (!file) {
    if (inName.isEmpty()) // just one last error check, the save dialog should
                          // be catching this one
      return;
    file = new QFile(inName);
    write();
    setEdited(false);
    // make the file for output and make everything happen
  } else {
    if (file->isOpen()) // make sure the file is closed or this has _VERY_ bad
                        // results
      file->close();
    file->setFileName(inName);
    write();
    setEdited(false);
  }
}

void Panel::save() {

  if (!loadwindowsformat) {
    QMessageBox msgBox;
    msgBox.setText("This panel was loaded using an outdated format, and will "
                   "be overwritten with the newer format");
    msgBox.setInformativeText("Do you want to save your changes?");
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard |
                              QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Save);
    int ret = msgBox.exec();

    switch (ret) {
    case QMessageBox::Save:
      // Save was clicked
      break;
    case QMessageBox::Cancel:
      // Cancel was clicked
      return;
    default:
      // should never be reached
      break;
    }
  }
  if (!file) {
    saveas();
    if (file) {
      if (mode == "EDIT")
        _saveas->setEnabled(true);
      _reread->setEnabled(true);
    }
  } else {

    write();

    setEdited(false);
    loadwindowsformat = true;
  }
}

void Panel::saveas() {
  QString s;
  if (file)
    s = file->fileName();
  else
    s = "";
  QString fn =
      QFileDialog::getSaveFileName(this, "Choose a filename and format", s,
                                   "Mac/Linux Panel Files (panel.*);;Windows "
                                   "Panel Files (*.pnl);;All Files (*.*)");
  if (fn.isEmpty())
    return;
  file = new QFile(fn);
  write();
  setEdited(false);
}

void Panel::setEdited(bool E) {
  modified = E;
  if (mode == "EDIT") {
    _save->setEnabled(E);
  }
}

void Panel::closeEvent(QCloseEvent *ce) {
  if (mode == "EDIT") {
    editor->close();
    return;
  }

  if (getModified()) {
    /*
    QMessageBox msgBox;
    msgBox.setText("The panel has unsaved changes");
    msgBox.setInformativeText("Do you want to save your changes?");
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard |
                              QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Save);
    int ret = msgBox.exec();
    */
    QMessageBox::StandardButton ret;
    ret= QMessageBox::question(this, "Test", "Do you want to save your changes?",
                                QMessageBox::Yes|QMessageBox::No |
                              QMessageBox::Cancel);

    switch (ret) {
    case QMessageBox::Save:
      // Save was clicked
      save();
      break;
    case QMessageBox::Discard:
      // Don't Save was clicked
      break;
    case QMessageBox::Cancel:
      // Cancel was clicked
      ce->ignore();
      return;
    default:
      // should never be reached
      break;
    }
  }
}

bool Panel::loadpanel(QString fn, int, int) {
  bool error = false;
  //std::cerr<<fn.toStdString()<<"\n";
  if (!fn.isEmpty()) {
    file = new QFile(fn);

    loadwindowsformat = isWindowsLoadFormat();

    // we don't want to read, we need to reload the default values only
    QString errmess = read();

    if (errmess != "OK") {
      if (file)
        file->close();
      error = true;
      // message box depends on if a new file should be created: yes or no
      // OR if there is a parse error: OK only
      if (errmess.contains("Create?", Qt::CaseInsensitive)) {
        if (QMessageBox::question(this, "Create new panel?", errmess) == QMessageBox::No)
          return false;
      } else {
        if (QMessageBox::warning(this, "Parse Error", errmess) == QMessageBox::Ok)
          return false;
      }
    }
    if (!error) {
      currpage = pages[0];
    } else {
      scrap();
      pages.push_back(new Page(font, this));
      currpage = pages.back();
      name = "New Panel";
      size = QSize(275, 450);
      bgColour[0] = bgColour[1] = bgColour[2] = 0.0;
      updateColourScheme();
    }
  } else {
    pages.push_back(new Page(font, this));
    currpage = pages.back();
    name = "New Panel";
    size = QSize(275, 450);
    bgColour[0] = bgColour[1] = bgColour[2] = 0.0;
    updateColourScheme();
  }

  setMinimumSize(size);
  setMaximumSize(size);

  if (mode == "EDIT") {
    for (unsigned int i = 0; i < pages.size(); i++) {
      pages[i]->setMode(mode);
    }
    for (unsigned int i = 0; i < menuitems.size(); i++) {
      menuitems[i]->setMode(mode);
    }
  }
  setupMenu();
  return true;
}

bool Panel::isWindowsLoadFormat() {
  if (file->exists() && file->open(QIODevice::ReadOnly)) {
    QTextStream data(file);

    QString s = data.readLine();
    QStringList split =
        s.split(":", QString::SkipEmptyParts, Qt::CaseInsensitive);

    if (split.size() < 1) {
      file->close();
      return false;
    }
    QString heading = split.at(0);
    if (heading.compare("target", Qt::CaseSensitive) != 0) {
      file->close();
      return false;
    }
  }
  file->close();
  return true;
}

void Panel::setupMenu() {
  // out with the old ...
  if (menu) {
    menu->close();
    menu = NULL;
    if (editmenu) {
      editmenu->close();
      editmenu = NULL;
    }
    if (filemenu) {
      filemenu->close();
      filemenu = NULL;
    }
    if (messmenu) {
      messmenu->close();
      messmenu = NULL;
    }
    if (pagemenu) {
      pagemenu->close();
      pagemenu = NULL;
    }
  }

  QAction *id;
  QString nm;
  // page menu
  if (pages.size() > 1) {
    pagemenu = new QMenu("Pages", this);
    for (unsigned int i = 0; i < pages.size(); i++) {
      if (pages[i]->getName().isEmpty()) {
        nm = "Page " + QString::number(i + 1);
      }

      else
        nm = pages[i]->getName();
      id = pagemenu->addAction(nm);

      pagemenu->actions().back()->setCheckable(true);
      if (currpage == pages[i]) {
        pagemenu->actions().back()->setChecked(true);
      }

      pages[i]->setID(id);
    }
  }

  // mess menu
  if (menuitems.size()) {
    messmenu = new QMenu("Custom messages", this);
    for (unsigned int i = 0; i < menuitems.size(); i++) {
      if (menuitems[i]->getName().isEmpty()) {
        nm = "Message " + QString::number(i + 1);
      } else
        nm = menuitems[i]->getName();
      id = messmenu->addAction(nm);
      menuitems[i]->setID(id);
    }
  }

  // menu menu menu
  menu = new QMenu("Messages", this);
  if (pagemenu) {
    menu->addMenu(pagemenu);
    if (!messmenu)
      menu->addSeparator();
  }
  if (messmenu) {
    menu->addMenu(messmenu);
    menu->addSeparator();
  }

  if (mode == "EXEC") {
    _restore = menu->addAction("Reset", this, SLOT(restore()));
    //    _reread = menu->addAction("Reread", this, SLOT(reread()));
    _broadall = menu->addAction("Broadcast", this, SLOT(broadcastall()));
    // _broad = menu->addAction("Broadcast current page", this, SLOT(broadcast()));
    //_load = menu->addAction("Load", this, SLOT(load()));
    menu->addSeparator();
    _edit = menu->addAction("Edit\1", this, SLOT(editPanel()));
    _saveDefault = menu->addAction("Save as default", this, SLOT(saveDefault()));
    //if (currpage->numItems() == 1)
    //  _broad->setEnabled(false);
    if (currpage->numItems() == 1)
      _broadall->setEnabled(false);
    if (!file) {
      _reread->setEnabled(false);
      _restore->setEnabled(false);
    }
  } else {
    // edit menu
    editmenu = new QMenu("Edit options", this);
    _edititem = editmenu->addAction("Edit item", this, SLOT(editSelect()));
    _clone = editmenu->addAction("Clone item", this, SLOT(cloneSelect()));
    _copy = editmenu->addAction("Copy item", this, SLOT(copySelect()));
    _paste = editmenu->addAction("Paste item", this, SLOT(pasteCopy()));
    _delete = editmenu->addAction("Delete item", this, SLOT(deleteSelect()));
    editmenu->addSeparator();
    _editpage = editmenu->addAction("Edit page", this, SLOT(editPage()));
    editmenu->addAction("Delete page", this, SLOT(deletePage()));
    editmenu->addSeparator();
    _snap = editmenu->addAction("Snap to grid", this, SLOT(toggleSnap()));
    _snap->setCheckable(true);
    _snap->setChecked(snapon);
    _copy->setEnabled(false);
    _delete->setEnabled(false);
    _clone->setEnabled(false);
    _edititem->setEnabled(false);
    _paste->setEnabled(false);

    _options = menu->addMenu(editmenu);
    _exec = menu->addAction("Execute", this, SLOT(execPanel()));
    menu->addSeparator();

    filemenu = new QMenu("File", this);
    _load = filemenu->addAction("Load...", this, SLOT(load()));
    _reread = filemenu->addAction("Reread", this, SLOT(reread()));
    _save = filemenu->addAction("Save", this, SLOT(save()));
    _saveas = filemenu->addAction("Save as...", this, SLOT(saveas()));
    _save->setEnabled(modified);
    if (!file) {
      _saveas->setEnabled(false);
      _reread->setEnabled(false);
    }
    _filestuff = menu->addMenu(filemenu);
  }

  QMenu *modeMenu = menu->addMenu("Refresh mode");
  _savingTriggered_act =
      modeMenu->addAction("Triggered", this, SLOT(TriggeredSavingMode()));
  _savingTriggered_act->setCheckable(true);

  _savingContinu_act =
      modeMenu->addAction("Continuous", this, SLOT(ContinuousSavingMode()));
  _savingContinu_act->setCheckable(true);
  if (_savingMode == CONTINUOUS) {
    _savingContinu_act->setChecked(true);
    ContinuousSavingMode();
  }
  if ((_savingMode == TRIGGERED) || (_savingMode == NONE)) {
    _savingTriggered_act->setChecked(true);
    TriggeredSavingMode();
  }

  menu->addSeparator();

  menu->addAction("E&xit", qApp, SLOT(closeAllWindows()));


  if (pagemenu)
    connect(pagemenu, SIGNAL(triggered(QAction *)), this,
            SLOT(pagemenuHandler(QAction *)));
  if (messmenu)
    connect(messmenu, SIGNAL(triggered(QAction *)), this,
            SLOT(messmenuHandler(QAction *)));
}

void Panel::about() {
  vlab::about(this,"Panels");
  return;
  QMessageBox::information(
      this, "About",
      "VLAB Panel Manager\n\nJoanne Penner\n Lynn Mercer\n Alejandro Garcia",
      "Close");
}

void Panel::quickHelp(){
  QDir helpDir(getHelpDirectory());
#ifndef __APPLE__
  helpDir.cd("Quick_Help");
#endif
  QString path = helpDir.filePath("PanelQuickHelp.html");
  QFile f(path);
  if (!f.open(QFile::ReadOnly | QFile::Text)) {
    std::cerr<<"Path: "<<path.toStdString()<<"doesn't exist"<<std::endl;
    return;
  }
  QTextStream in(&f);
  QString message = in.readAll();
  QTextBrowser *tb = new QTextBrowser(this);
  tb->setOpenExternalLinks(true);
  tb->setHtml(message);

  QDialog *msgBox = new QDialog;
  msgBox->setWindowTitle("Panel: Quick Help");
  msgBox->setWindowFlags(Qt::Dialog);
  msgBox->setModal(false);
  QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok);
  QPushButton* okBtn = bb->button(QDialogButtonBox::Ok);
  connect(okBtn, SIGNAL(clicked()),msgBox,SLOT(close()));
  QVBoxLayout *layout = new QVBoxLayout;
  layout->addWidget(tb);
  layout->addWidget(bb);
  msgBox->setLayout(layout);
  msgBox->resize(400,300);
 
  msgBox->show();

}

void Panel::pdfHelp(){
  QDir helpDir(getHelpDirectory());
  QDesktopServices::openUrl(
    QUrl::fromLocalFile(helpDir.filePath("VLABToolsManual.pdf")));
}

void Panel::help() {
  QString mess;
  mess =
      "<h2>"
      "Quick Help for VLAB Panel Manager</h2>"
      ""
      "<h2>"
      "<b><i>1 Panel Execution</i></b></h2>"
      "<b><u>1.1 Command Line</u></b>"
      "<br>Execute Panel Manager using the following arguments:"
      "<p><b>>&nbsp; panel [optional file name]</b>"
      "<br>Opens a new panel, with optional file name."
      "<p><b>>&nbsp; panel [panel definition file]</b>"
      "<br>Opens existing panel."
      "<p><b><u>1.2 Activating Panel Items</u></b>"
      "<p><b>Sliders</b>"
      "<ul>"
      "<li>"
      "<b>Select</b> the outlined slider&nbsp; between the title and value "
      "labels with the <b>left mouse</b> to grab focus.</li>"
      ""
      "<li>"
      "<b>Move</b> the mouse <b>left to decrease</b> the slider value, and "
      "<b>right to increase</b> the slider value.</li>"
      ""
      "<li>"
      "<b>Release</b> the mouse button to send the message.</li>"
      "</ul>"
      "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Or:"
      "<ul>"
      "<li>"
      "<b>Click</b> the <b>left mouse</b> on the small arrows on either side "
      "of the slider.&nbsp; Click the <b>left arrow to decrease</b> the slider "
      "value, and the <b>right arrow to increase</b> the slider value.</li>"
      "</ul>"
      "<b>Buttons</b>"
      "<ul>"
      "<li>"
      "<b>Select</b> the button with the <b>left mouse</b> to grab focus.</li>"
      ""
      "<li>"
      "<b>Release</b> the mouse button to send the message.</li>"
      ""
      "<li>"
      "<b>Monostable</b> buttons will change colour while pressed, and return "
      "to the original colour after release.</li>"
      ""
      "<li>"
      "<b>Toggle</b> buttons will switch between ON and OFF states (and "
      "colours) each time they are activated.</li>"
      "</ul>"
      "<b>Button Groups</b>"
      "<ul>"
      "<li>"
      "Button groups are sets of <b>mutually exclusive</b> toggle buttons.</li>"
      ""
      "<li>"
      "If any button is activated, it becomes ON (even if it was ON "
      "previously) and the others are turned OFF.</li>"
      ""
      "<li>"
      "It makes little sense to use monostable buttons in groups.</li>"
      "</ul>"
      "<b>Menu Items</b>"
      "<ul>"
      "<li>"
      "<b>Press</b> the <b>right mouse</b> anywhere on the panel to activate "
      "the popup menu.</li>"
      ""
      "<li>"
      "<b>Select</b> the menu item from the popup menu to send the "
      "message.</li>"
      "</ul>"
      "<b>Pages</b>"
      "<ul>"
      "<li>"
      "<b>Press</b> the <b>right mouse</b> anywhere on the panel to activate "
      "the popup menu.</li>"
      ""
      "<li>"
      "<b>Select</b> the page title from the popup menu to go to the page.</li>"
      ""
      "<li>"
      "A message is sent to signal the page change.</li>"
      "</ul>"
      "<b><u>1.3 Popup Menu</u></b>"
      "<ul>"
      "<li>"
      "<b>Restore</b> rereads the panel definition file to restore controls to "
      "their initial values, and sends all messages associated with the "
      "control to stdout.</li>"
      ""
      "<li>"
      "<b>Reread</b> rereads the panel definition file and redraws the "
      "controls in the panel window.</li>"
      ""
      "<li>"
      "<b>Broadcast</b> sends all messages associated with the controls to "
      "stdout.</li>"
      ""
      "<li>"
      "<b>Edit</b> opens the panel editor window, and switches to edit mode. A "
      "different set of menu options are then available.</li>"
      ""
      "<li>"
      "<b>Quit</b> outputs the null character (end-of-file), which terminates "
      "the parameter editor , and closes the panel.</li>"
      "</ul>"
      ""
      "<p><br><b><u>1.4 Interfacing with an Application</u></b>"
      "<ul>"
      "<li>"
      "The panel manager can be interfaced to <b>any application that has the "
      "ability to reread its data files</b>.</li>"
      ""
      "<li>"
      "Panel items print messages to standard output when activated with the "
      "click of the left mouse button.</li>"
      ""
      "<li>"
      "A <b>parameter editor</b> is used to interpret the messages and modify "
      "the corresponding file. Two parameter editors are commonly used in the "
      "vlab system, <i>ped</i> and <i>awkped</i>.</li>"
      ""
      "<li>"
      "The following example shows how the command line can be used to "
      "interface a panel with an application:</li>"
      "<p><b>> panel </b>[<i>panel definition file</i>] <b>| awkped "
      "</b>[<i>program file</i>]</ul>"
      ""
      "<br>"
      "<h2>"
      "<b><i>2 Panel Editor</i></b></h2>"
      "In <b>execute mode</b> (default), activate the popup menu by pressing "
      "the"
      "<b>right mouse</b> button anywhere on the panel and select<b> <i>Edit "
      "</i></b>to"
      "invoke the Panel Editor."
      "<br>The <b>panel editor</b> is a separate window which interacts with "
      "the"
      "<b>panel</b>.&nbsp;"
      "A 10x10 pixel grid is drawn over the panel, to aid with design and "
      "alignment."
      "<p><b><u>2.1 Editing the Panel</u></b>"
      "<p>The panel editor is divided into <b>three main sections</b>: the "
      "upper section contains buttons to <b>add new items</b> to the panel, "
      "the middle section contains <b>horizontal and vertical alignment</b> "
      "buttons for aligning items on the panel, and the lower section contains "
      "the <b>editable fields of the panel</b>."
      "<br>The following panel fields may be edited:"
      "<ul>"
      "<li>"
      "<b><i>Name: </i></b>The panels name can be typed in the edit box.&nbsp; "
      "The name is displayed in the panel window caption.&nbsp; It is not the "
      "same as the panel file name.</li>"
      ""
      "<li>"
      "<b><i>Font:</i></b> A font for panel text can be specified by left "
      "clicking the button marked <b><i>Set</i></b>.</li>"
      ""
      "<li>"
      "<b><i>Width:</i></b> / <b><i>Height: </i></b>The width and height for "
      "the panel can be entered into the edit boxes, or the panel ca be freely "
      "resized.&nbsp;"
      "When resizing smaller, be cautious that all items on all pages remain "
      "within the panel bounds.</li>"
      "</ul>"
      ""
      "<p><br><b><u>2.2 Panel Items</u></b>"
      "<p><b>Labels</b>, <b>sliders</b>, <b>buttons</b> and <b>button "
      "groups</b> appear on the panel.&nbsp; Their contour is defined by a "
      "rectangular border around them and their <b>origin</b> is defined as "
      "the <b>bottom left</b>"
      "corner.&nbsp; <b>Pages</b> and <b>menu items</b> appear in the panel's "
      "popup menu. The <b>panel origin (0,0)</b> is defined as the <b>bottom "
      "left</b>"
      "corner. "
      "<p><b>Adding New Items to Panel</b>"
      "<ul>"
      "<li>"
      "Select an item from the <b>group of buttons at the top of panel "
      "editor</b>.</li>"
      ""
      "<li>"
      "The new item automatically appears at <b>the middle of the panel</b> "
      "and is selected.</li>"
      ""
      "<li>"
      "A popup dialog to edit&nbsp; the new item is opened.</li>"
      "</ul>"
      "<b>Selecting Items</b>"
      "<ul>"
      "<li>"
      "<b>Press</b> the <b>left mouse</b> button over an item to select. You "
      "can also drag-select, and select multiple items by shift-clicking. A "
      "dotted contour will encompass all the selected item.</li>"
      ""
      "<li>"
      "<b>Double click</b> the <b>left mouse</b> over an item to bring up the"
      "<b>popup dialog</b>. Only one popup dialog can be open, for a single "
      "item. The item with the active dialog has a solid contour surrounding "
      "it, as opposed to a dotted one.</li>"
      ""
      "<li>"
      "The dotted and solid contours' colour contrast the panel "
      "background.</li>"
      ""
      "<li>"
      "The current <b>page</b> is selectable by pressing the <b>left mouse</b> "
      "on the panel background.&nbsp; To go to another page, <b>activate the "
      "popup menu</b> by pressing the <b>right mouse</b> anywhere on the panel "
      "and <b>select</b>"
      "the desired page.</li>"
      ""
      "<li>"
      "To select a <b>menu item</b>, <b>activate the popup menu </b>and "
      "<b>select </b>the desired menu item.&nbsp; The popup dialog for the "
      "menu item is"
      "opened.</li>"
      ""
      "<li>"
      "Once items are selected, they may be moved, deleted, cloned or "
      "edited.</li>"
      "</ul>"
      "<b>Moving Items</b>"
      "<ul>"
      "<li>"
      "Once items are selected, they may be <b>dragged by holding the left "
      "mouse</b>"
      "button while moving the mouse.</li>"
      ""
      "<li>"
      "When the item's popup dialog is open, the <b>origin may be entered</b> "
      "into the edit boxes marked <b><i>Origin:</i></b>.</li>"
      ""
      "<li>"
      "If a button group is moved, all buttons within it are moved.</li>"
      "</ul>"
      "<b>Snap to Grid</b>"
      "<ul>"
      "<li>"
      "To enable <b>Snap to Grid</b> option, <b>activate the popup menu</b> and"
      "<b>select"
      "<i>Snap to Grid</i> </b>from <b><i>Edit Options.</i></b></li>"
      ""
      "<li>"
      "When an item is selected or moved by with the <b>left mouse</b>, it "
      "will snap to grid upon <b>movement</b>.&nbsp; <b>The bottom left corner "
      "will align with the nearest grid intersection</b><i>.</i></li>"
      ""
      "<li>"
      "Buttons in groups cannot be snapped to grid, however the group can "
      "be.</li>"
      "</ul>"
      "<b>Deleting Items</b>"
      "<ul>"
      "<li>"
      "Once items are selected, they may be deleted by <b>activating the popup "
      "menu</b> by pressing the <b>right mouse</b> anywhere on the panel and "
      "<b>selecting"
      "<i>Delete Item</i></b> from <b><i>Edit Options.</i></b></li>"
      ""
      "<li>"
      "A <b>menu item</b> may be deleted by clicking the button marked "
      "<b><i>Delete</i></b> on the menu item popup dialog.</li>"
      ""
      "<li>"
      "A <b>page</b> may be deleted by <b>activating the popup menu</b> and "
      "<b>selecting <i>Delete Page</i></b> from <b><i>Edit "
      "Options.</i></b></li>"
      "</ul>"
      "<b>Cloning Items</b>"
      "<ul>"
      "<li>"
      "Once items are selected, they may be cloned by <b>activating the popup "
      "menu</b> and <b>selecting"
      "<i>Clone Item</i></b> from <b><i>Edit Options.</i></b></li>"
      ""
      "<li>"
      "A clone is <i>exactly</i> like the original item, and automatically "
      "appears slightly <b>up and right</b> and is selected.</li>"
      "</ul> <b>Aligning Items on Panel</b>"
      "<ul>"
      "<li>"
      "Once items are selected, they may be aligned<b> vertically</b> or "
      "<b>horizontally</b> on the panel.&nbsp; This is useful for panels "
      "designed as rows or columns.</li>"
      ""
      "<li>"
      "Select an alignment from the <b>groups of buttons in the middle section "
      "of the panel editor</b>.</li>"
      "</ul>"
      "<b><u>2.3 Editing Panel Items</u></b>"
      "<p>An items popup dialog can be invoked by:"
      "<ul>"
      "<li>"
      "<b>Double click</b> the <b>left mouse</b> over the item.</li>"
      ""
      "<li>"
      "<b>Activate the popup menu</b> by pressing the <b>right mouse</b> "
      "anywhere on the panel and select <b><i>Edit Item</i></b> from "
      "<b><i>Edit Options.</i></b></li>"
      ""
      "<li>"
      "<b>Add</b> a new item to the panel.</li>"
      "</ul>"
      "Each type of item has its own dialog.&nbsp; The item type is "
      "specified&nbsp; at the top of the dialog.&nbsp; The close button is "
      "located at the bottom."
      "<br>An items editable fields may include the following (not all items "
      "include all fields):"
      "<ul>"
      "<li>"
      "<b><i>Name: </i></b>The items name can be typed in the edit "
      "box<b><i>.</i></b></li>"
      ""
      "<li>"
      "<b><i>Colours: </i></b>Click on the coloured button to change the "
      "colour."
      "<b><i>Origin: </i></b>The items (x,y) origin is displayed in spin "
      "boxes.&nbsp; These values may be edited.</li>"
      ""
      "<li>"
      "<b><i>Min/Max:</i></b> The range of sliders can be defined.&nbsp; <b>Min"
      "value MUST be less than Max value.</b></li>"
      ""
      "<li>"
      "<b><i>Value: </i></b>The initial state can be defined for buttons and "
      "sliders.</li>"
      ""
      "<li>"
      "<b><i>Message:</i></b> The message that is sent upon activation in "
      "execute"
      "mode must be specified.&nbsp; As a reminder, the default message is: "
      "<b>d"
      "&lt;name> %d &lt;scale></b></li>"
      "</ul>"
      "<b>Label Dialog</b>"
      "<br>A labels editable fields include: <b>name</b>, <b>colour</b>, "
      "<b>origin</b>"
      "<p><b>Slider Dialog</b>"
      "<br>A sliders editable fields include: <b>name</b>, <b>colours "
      "</b>(2)<b>,"
      "origin, min/max, value (</b>between min and max)<b>, message</b>"
      "<p><b>Button Dialog</b>"
      "<br>A buttons editable fields include: <b>name</b>, <b>colours "
      "</b>(3)<b>,"
      "origin, value </b>(on,off,monostable)<b>, message</b>"
      "<p><b>Button Group Dialog</b>"
      "<br>A button groups editable fields include:&nbsp; <b>colour, origin, "
      "buttons"
      "</b>(list"
      "of names)"
      "<br>Buttons can be added to a button group by selecting items, and then "
      "clicking the add group button. A group will be selected out of all "
      "selected buttons, even if some were either in a group"
      "<br><b>Buttons MUST have unique names, or they cannot be added to a "
      "group. This is done automatically by adding an identifier that is not "
      "rendered to the name of the button.</b>"
      "<p><b>Menu Item Dialog</b>"
      "<br>A menu items editable fields include: <b>name, message</b>"
      "<p><b>Page Dialog</b>"
      "<br>A pages editable fields include: <b>name, page label colour, page "
      "label"
      "origin, message</b>"
      "<br>A pages name appears in its label.&nbsp; All pages have labels, and"
      "page labels cannot be deleted, although they can be empty."
      "<p><b><u>2.4 Executing the Panel</u></b>"
      "<p>The panel can return to <b>execute mode</b> in the following two "
      "ways:"
      "<ul>"
      "<li>"
      "<b>Activate the popup menu</b> by pressing the <b>right mouse</b> "
      "anywhere"
      "on the panel and select <b><i>Execute</i></b>.</li>"
      ""
      "<li>"
      "<b>Close the panel editor</b> by clicking the large button marked "
      "<b><i>Close</i></b>,"
      "located on the bottom of the panel editor.</li>"
      "</ul>"
      ""
      "<p><br><b><u>2.4 File Options</u></b>"
      "<ul>"
      "<li>"
      "<b>Load</b> opens a file dialog box to browse the users directories for"
      "the panel to load.</li>"
      ""
      "<li>"
      "<b>Reread&nbsp;</b> rereads the last saved panel definition file.</li>"
      ""
      "<li>"
      "<b>Save</b> saves the panel.&nbsp; If the panel file as not been "
      "specified,"
      "a file dialog will be opened to input the new panel file.</li>"
      ""
      "<li>"
      "<b>Save As</b> opens a file dialog to input another file to save the "
      "panel"
      "as.</li>"
      "</ul>"
      ""
      "<h2>"
      "<i>3 File Format</i></h2>"
      "The Panel Manager requires a specific file format, panels can be opened "
      "using old formats, but will be saved with the new format.&nbsp; Panel "
      "definition"
      "files may be edited manually, or may be generated using the panel "
      "editor.&nbsp;"
      "Each panel item requires certain specifications in sequence.&nbsp; Each"
      "line in the file must begin with a <i>heading</i>, followed by a "
      "<i>colon</i>"
      "and a <i>space</i> separation.&nbsp; The only <b>optional</b> "
      "specifications"
      "The following shows an example of a panel definition file, including all"
      "item types. NOTE: Only the target header field is necessary, but the "
      "order should be the same for all given title fields "
      "(target->name->background->size->font)."
      "<br>&nbsp;"
      "<table BORDER NOSAVE >"
      "<tr NOSAVE>"
      "<td NOSAVE>target: Lsystem"
      "<br>panel name: Model"
      "<br>background: 50, 250, 128"
      "<br>size: 500 500"
      "<br>font: "
      "-urw-avantgarde-demibold-r-normal--17-120-100-100-p-0-iso8859-1"
      "<p>type: PAGE"
      "<br>name: Anabaena"
      "<br>color: 7"
      "<br>origin: 200 580"
      "<br>message: p 1"
      "<p>type: LABEL"
      "<br>name: Initial Cell"
      "<br>color: 7"
      "<br>origin: 133 555"
      "<p>type: SLIDER"
      "<br>name: Initial Conc (1/10)"
      "<br>colors: 7 1"
      "<br>origin: 20 340"
      "<br>min/max: 0 10000"
      "<br>value: 9225"
      "<br>message: n 4 1 -1 %d"
      "<p>type: BUTTON"
      "<br>name: Image"
      "<br>colors: 7 1"
      "<br>origin: 10 520"
      "<br>value: 1"
      "<br>message: s 1 image"
      "<p>type: BUTTON"
      "<br>name: 1st Split"
      "<br>colors: 7 1"
      "<br>origin: 10 460"
      "<br>value: 0"
      "<br>message: s 1 split"
      "<p>type: GROUP"
      "<br>color: 7"
      "<br>Image"
      "<br>L Justify"
      "<br>1st Split"
      "<br>ENDGROUP"
      "<p>type: MENU"
      "<br>name: Show Grid "
      "<br>message: d grid 1</td>"
      "</tr>"
      "</table>";

  QLabel *label = new QLabel("");
  label->setTextFormat(RichText);
  label->setText(mess);

  QScrollArea *helpscroll = new QScrollArea;
  helpscroll->setBackgroundRole(QPalette::Dark);
  helpscroll->setWidget(label);

  helpscroll->show();
}

void Panel::ContinuousSavingMode() {
  _savingMode = CONTINUOUS;
  _savingContinu_act->setChecked(true);
  _savingTriggered_act->setChecked(false);

  // set continuous Mode in items
  for (int j = 0; j < (int)pages.size(); ++j) {
    for (int i = 0; i < pages[j]->numItems(); i++) {
      pages[j]->getItem(i)->setSavingMode(_savingMode);
    }
  }
}

void Panel::TriggeredSavingMode() {
  _savingMode = TRIGGERED;
  _savingContinu_act->setChecked(false);
  _savingTriggered_act->setChecked(true);

  // set triggered Mode in items
  for (int j = 0; j < (int)pages.size(); ++j) {
    for (int i = 0; i < pages[j]->numItems(); i++) {
      pages[j]->getItem(i)->setSavingMode(_savingMode);
    }
  }
}

void Panel::ModeOff() {
  _savingMode = NONE;
  _savingContinu_act->setChecked(false);
  _savingTriggered_act->setChecked(false);
}

void Panel::update() {
  _glWidget->update();
  QMainWindow::update();
}

void Panel::keyPressed(QKeyEvent *me) {
  if (mode == "EDIT") {
    if (Key_Left == me->key()) {
      moveSelectedItemsRight(-1);
    }
    if (Key_Right == me->key())
      moveSelectedItemsRight(1);
    if (Key_Up == me->key())
      moveSelectedItemsUp(1);
    if (Key_Down == me->key())
      moveSelectedItemsUp(-1);
    _glWidget->update();
  }
}

void Panel::mouseClicked(QMouseEvent *me) {
  if (LeftButton == me->button()) {
    if (me->modifiers() & Qt::ShiftModifier) {
      currpage->shiftMousePress(QPoint(me->x(), me->y()));
    } else {
      currpage->mousePress(QPoint(me->x(), me->y()));
    }

    if (mode == "EDIT") {
      if (!currpage->getSelectedItems().empty()) {
	_copy->setEnabled(true);

        _delete->setEnabled(true);
        _clone->setEnabled(true);
        _edititem->setEnabled(true);
      } else {
	_copy->setEnabled(false);
        _delete->setEnabled(false);
        _clone->setEnabled(false);
        _edititem->setEnabled(false);
      }

      currpage->updateAlignmentButtonsEnabled();
    }
    _glWidget->update();
  }

  if (RightButton == me->button()) {
    menu->exec(QCursor::pos());
  }
}

void Panel::mouseDoubleClicked(QMouseEvent *me) {
  mousePressEvent(me);
  me->ignore();
  if (mode == "EDIT") {
    currpage->mouseDoublePress(QPoint(me->x(), me->y()));
    if (!currpage->getSelectedItems().empty()) {
      setDialog();
    } else {
      closeOpenDialogs();
    }
  }
}

void Panel::mouseMoved(QMouseEvent *me) {
  if (currpage) {
    currpage->mouseMove(QPoint(me->x(), me->y()));

    updateOpenDialogs();
    _glWidget->update();
  }
}

void Panel::mouseReleased(QMouseEvent *me) {
  if (currpage) {
    currpage->mouseRelease(QPoint(me->x(), me->y()));

    if (mode == "EDIT") {
      if (!currpage->getSelectedItems().empty()) {
        _copy->setEnabled(true);
        _delete->setEnabled(true);
        _clone->setEnabled(true);
        _edititem->setEnabled(true);
      } else {
	_copy->setEnabled(false);
        _delete->setEnabled(false);
        _clone->setEnabled(false);
        _edititem->setEnabled(false);
      }

      currpage->updateAlignmentButtonsEnabled();
    }

    _glWidget->update();
  }
}

void Panel::editSelect() {

  if (mode == "EDIT") {
    if (!currpage->getSelectedItems().empty()) {
      std::vector<Item*> v = currpage->getSelectedItems();
      setDialog();
    } else {
      closeOpenDialogs();
    }
  }
}

void Panel::showEvent(QShowEvent *) {
}

bool Panel::eventFilter(QObject *obj, QEvent *event) {
  // check to see whether we can resize or not
  if (obj == QWidget::window() && event->type() == QEvent::Resize) {
    if (mode != "EDIT")
      QWidget::window()->setFixedSize(QWidget::window()->size());
    else {
      QWidget::window()->setMinimumSize(QSize(0, 0));
      QWidget::window()->setMaximumSize(desktop->size());
    }
  }
  if (event->type() == QEvent::KeyPress) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
    if (keyEvent->key() == Qt::Key_Right) {
      moveSelectedItemsRight(5);
    } else if (keyEvent->key() == Qt::Key_Left) {
      moveSelectedItemsRight(-5);
    } else if (keyEvent->key() == Qt::Key_Up) {
      moveSelectedItemsUp(5);
    } else if (keyEvent->key() == Qt::Key_Down) {
      moveSelectedItemsUp(-5);
    } else if (keyEvent->key() == Qt::Key_Backspace ||
               keyEvent->key() == Qt::Key_Delete) {
      deleteSelect();
    }
  }

  return 1;
}

void Panel::moveSelectedItemsRight(int dx) {
  currpage->moveSelectedItemsRight(dx);
}
void Panel::moveSelectedItemsUp(int dy) { currpage->moveSelectedItemsUp(dy); }

// EOF: panel.cc
