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



/* MaterialWorld

   Implementation of Class: MW


*/

#include "mw.h"
#include <QCloseEvent>
#include <QFileDialog>
#include <QLabel>
#include <QTextStream>
#include <iostream>
#include "resources.h"
#include <QDesktopServices>
#include <QDir>
#include <QMenu>
#include <QTextBrowser>
#include <QDialogButtonBox>

using namespace Qt;
using namespace std;
// -------------------- Construction --------------------
MW::MW(int n, char **args, QWidget *parent)
    : QMainWindow(parent), filename(0) {
  setUnifiedTitleAndToolBarOnMac(true);
  modified = false;
  init();
  QFile *c = new QFile(QString(getenv("VLABCONFIGDIR")) + QString("/medit"));
  if (c->exists())
    loadconfiguration(c);
  parse(n, _savingMode, args);
  viewer->setSavingMode(_savingMode);
}

void MW::init() {
  // new viewer and editor
  editor = new MWEditor(this, "editor", Qt::Tool);
  // [PASCAL] next attribute is to fix a bug in Qt to make sure the window is always visible
  editor->setAttribute(Qt::WA_MacAlwaysShowToolWindow, true);

  viewer = new MWViewer(this, "viewer");
  setCentralWidget(viewer);

  // menu bar
  file = new QMenu("File", this);
  file->setObjectName("file");
  QAction *act = 0;

  act = file->addAction("New", this, SLOT(newfile()));
  addAction(act);
  act = file->addAction("&New Window", this, SLOT(newWindow()), CTRL + Key_N);
  addAction(act);
  act = file->addAction("&Load...", this, SLOT(load()), CTRL + Key_L);
  addAction(act);
  act = file->addAction("Load New Window...", this, SLOT(loadWindow()));
  addAction(act);

  _sv = file->addAction("&Save", this, SLOT(save()), CTRL + Key_S);
  file->insertSeparator(_sv);

  _sv->setEnabled(false);
  addAction(_sv);

  act = file->addAction("Save As...", this, SLOT(saveas()));
  addAction(act);

  _rs = file->addAction("Revert to Saved", this, SLOT(revertsaved()));

  _rs->setEnabled(false);
  file->insertSeparator(_rs);
  act = file->addAction("About", this, SLOT(about()));
  file->insertSeparator(act);
  act = file->addAction("E&xit", qApp, SLOT(closeAllWindows()), CTRL + Key_X);
  menuBar()->addMenu(file);

  edit = new QMenu("Edit\1", this);
  edit->setObjectName("edit");
  act = edit->addAction("M-Edit", editor, SLOT(edit()));
  edit->insertSeparator(act);
  edit->addAction("Cut", viewer, SLOT(cut()));
  edit->addAction("C&opy", viewer, SLOT(copy()), CTRL + Key_O);
  _ps = edit->addAction("&Paste", viewer, SLOT(paste()), CTRL + Key_P);
  _ps->setEnabled(false);
  _in = edit->addAction("Insert", viewer, SLOT(insert()));
  _in->setEnabled(false);
  edit->addAction("Interpolate", viewer, SLOT(interpolate()), CTRL + Key_I);
  act = edit->addAction("Set to Default", viewer, SLOT(defaultmat()),
                        CTRL + Key_D);
  act = edit->addAction("Select Range...", viewer, SLOT(rangeDialog()));
  edit->insertSeparator(act);
  edit->addAction("Select all in Page", viewer, SLOT(selectpage()));
  edit->addAction("Select all", viewer, SLOT(selectall()),
                  CTRL + Key_G);
  menuBar()->addMenu(edit);
 
  options = new QMenu("View", this);
  options->setObjectName("options");
  _nx = options->addAction("Next Page", viewer, SLOT(nextpage()), CTRL + Key_W);
  _pv = options->addAction("Prev Page", viewer, SLOT(prevpage()), CTRL + Key_Q);
  _fp = options->addAction("First Page", viewer, SLOT(firstpage()));
  _pv->setEnabled(false);
  _fp->setEnabled(false);
  _xs = options->addAction("xs - 1x1", viewer, SLOT(xspage()));
  options->insertSeparator(_xs);
  _sm = options->addAction("sm - 4x4", viewer, SLOT(smpage()));
  _md = options->addAction("md - 8x8", viewer, SLOT(mdpage()));
  _lg = options->addAction("lg - 16x16", viewer, SLOT(lgpage()));
  // options->addSeparator();
   //options->addAction("Image Quality...", this, SLOT(editSmooth()));
  //  _bg = options->addAction("Show Background", this, SLOT(bg()), CTRL + Key_B);
  menuBar()->addMenu(options);

  help = new QMenu("Help", this);
  help->setObjectName("help");
  // help->addAction("Quick Help Documentation", this, SLOT(onlinehelp()));
   QAction *qHelp=help->addAction("Quick help",this,SLOT(quickHelp()));
  qHelp->setEnabled(true);
  //help->addAction("Medit Help", this, SLOT(help()));
  help->addAction("Tools manual", this, SLOT(pdfHelp()));
 menuBar()->addMenu(help);

  // in the status bar
  pglabel = new QLabel("Set", statusBar());
  pglabel->setObjectName("pglabel");
  pagestepper = new QSpinBox(statusBar());
  pagestepper->setObjectName("pagestepper");
  pagestepper->setMinimum(1);
  pagestepper->setMaximum(16);
  pagestepper->setSingleStep(1);
  pagestepper->setValue(0);

  pagelabel = new QLabel(" of 16 ", statusBar());
  pagelabel->setObjectName("pagelabel");
  indexlabel = new QLabel(" Index: ", statusBar());
  indexlabel->setObjectName("indexlabel");
  indexstepper = new QSpinBox(statusBar());
  indexstepper->setObjectName("indexstepper");
  indexstepper->setMinimum(0);
  indexstepper->setMaximum(255);
  indexstepper->setSingleStep(1);
  indexstepper->setValue(0);

 
  statusBar()->addPermanentWidget(indexlabel, 0);
  statusBar()->addPermanentWidget(indexstepper, 0);
  statusBar()->addPermanentWidget(pglabel, 0);
  statusBar()->addPermanentWidget(pagestepper, 0);
  statusBar()->addPermanentWidget(pagelabel, 0);

  statusBar()->showMessage("Ready!", 3500);

  RD = new SelectRangeDialog(this, "rd", Qt::Window);
  imqual = new ImageDialog(this, "im", Qt::Window);

  // Connections
  QObject::connect(indexstepper, SIGNAL(valueChanged(int)), viewer,
                   SLOT(select(int)));
  QObject::connect(pagestepper, SIGNAL(valueChanged(int)), viewer,
                   SLOT(gotopage(int)));
  QObject::connect(viewer, SIGNAL(setselect(int, Material)), this,
                   SLOT(setselect(int, Material)));
  QObject::connect(viewer, SIGNAL(pageflip(int, int)), this,
                   SLOT(pageflip(int, int)));
  QObject::connect(this, SIGNAL(read(QFile *)), viewer, SLOT(read(QFile *)));
  QObject::connect(this, SIGNAL(write(QFile *)), viewer, SLOT(write(QFile *)));
  QObject::connect(viewer, SIGNAL(notice(const QString &)), this,
                   SLOT(notice(const QString &)));
  QObject::connect(viewer, SIGNAL(error(const QString &, int)), this,
                   SLOT(error(const QString &, int)));
  QObject::connect(viewer, SIGNAL(edit()), editor, SLOT(edit()));
  QObject::connect(viewer, SIGNAL(modify(bool)), this, SLOT(setModified(bool)));
  QObject::connect(this, SIGNAL(imageON(bool)), viewer,
                   SLOT(showBackground(bool)));

  // Connection with viewer
  QObject::connect(viewer, SIGNAL(newfile_signal()), this, SLOT(newfile()));
  QObject::connect(viewer, SIGNAL(newWindow_signal()), this, SLOT(newWindow()));
  QObject::connect(viewer, SIGNAL(load_signal()), this, SLOT(load()));
  QObject::connect(viewer, SIGNAL(loadWindow_signal()), this,
                   SLOT(loadWindow()));
  QObject::connect(viewer, SIGNAL(save_signal()), this, SLOT(save()));
  QObject::connect(viewer, SIGNAL(saveas_signal()), this, SLOT(saveas()));
  QObject::connect(viewer, SIGNAL(revertsaved_signal()), this,
                   SLOT(revertsaved()));
  QObject::connect(this, SIGNAL(modify_save_viewer(bool)), viewer,
                   SLOT(setmodified_save(bool)));
  QObject::connect(this, SIGNAL(modify_saveas_viewer(bool)), viewer,
                   SLOT(setmodified_save_as(bool)));

  QObject::connect(editor, SIGNAL(doneEvent(Material)), viewer,
                   SLOT(setMaterial(Material)));
  QObject::connect(editor, SIGNAL(applyEvent(Material)), viewer,
                   SLOT(showMaterial(Material)));
  QObject::connect(viewer, SIGNAL(setdefault(Material)), editor,
                   SLOT(setdefault(Material)));

  // THIS DISABLES SELECTION IF EDITED MATERIAL .... (comment out?)
  QObject::connect(editor, SIGNAL(isEdited(bool)), this, SLOT(selectOff(bool)));

  QObject::connect(viewer, SIGNAL(getRange(int, int)), RD,
                   SLOT(getRange(int, int)));
  QObject::connect(RD, SIGNAL(rangeEvent(int, int)), viewer,
                   SLOT(selectrange(int, int)));
  QObject::connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));

  QObject::connect(QApplication::clipboard(), SIGNAL(dataChanged()), this,
                   SLOT(enablePaste()));
  QObject::connect(QApplication::clipboard(), SIGNAL(dataChanged()), viewer,
                   SLOT(enablePaste()));

  QObject::connect(imqual, SIGNAL(setSmoothness(int)), viewer,
                   SLOT(setSmoothness(int)));
  QObject::connect(imqual, SIGNAL(adjustResize(bool)), viewer,
                   SLOT(setEnhance(bool)));
  QObject::connect(imqual, SIGNAL(adjustPageSize(bool)), viewer,
                   SLOT(setPageSmooth(bool)));
  QObject::connect(imqual, SIGNAL(saveconfig()), this, SLOT(saveconfig()));
  QObject::connect(editor, SIGNAL(triggered()), this,
                   SLOT(saveInTriggeredMode()));
}

// parse the args (for now, only uses first arg as filename)
void MW::parse(int n, SavingMode &savingMode, char **args) {
  QString a, ws, hs;
  resize(375, 375);
  savingMode = OFF;
  int trynewfile = 0;
  if (n == 1)
    newfile();
  else {
    int i = 1;
    const char *fileName;
    while (i < n) {
      if ((QString)(args[i]) == "-w") {
        resize((QString(args[i + 1])).toInt(), (QString(args[i + 2])).toInt());
        i += 3;
      } else if (((QString)(args[i]) == "-rmode") ||
                 ((QString)(args[i]) == "--refreshMode")) {
        const char *opt = args[i + 1];
        if ((strcmp(opt, "expl") == 0) || (strcmp(opt, "explicit") == 0))
          savingMode = OFF;
        if ((strcmp(opt, "cont") == 0) || (strcmp(opt, "continuous") == 0))
          savingMode = CONTINUOUS;
        if ((strcmp(opt, "trig") == 0) || (strcmp(opt, "triggered") == 0))
          savingMode = TRIGGERED;
        i += 2;
      } else {
        fileName = args[i];
        i++;
        if (trynewfile == 0) {
          trynewfile = 1;
          switch (loadfile(fileName)) {
          case 0:
            load();
            break;
          case 1:
            break;
          case -1:
          case -2:
          case -3:
            newfile();
            break;
          }
        } else
          loadnewfile(fileName);
      }
    }
  }
}

void MW::loadconfiguration(QFile *config) {
  /*
  QTextStream fin(config);
  int s;
  QString heading;
  config->open(QIODevice::ReadOnly);
  fin >> heading;
  while (!fin.atEnd()) {
    // between 0 (min) and ?(80)
    if ((heading == "SMOOTHNESS") || (heading == "smoothness")) {
      fin >> s ; fin.flush();
      viewer->setSmoothness(s);
    }
    // 1 -> ON, 0 -> OFF
    if ((heading == "ENHANCED_RESIZE") || (heading == "enhanced_resize")) {
      fin >> s; fin.flush();
      viewer->setEnhance((bool)s);
    }
    // 1 -> ON, 0 -> OFF
    if ((heading == "PAGE_SMOOTH") || (heading == "page_smooth")) {
      fin >> s; fin.flush();
      viewer->setPageSmooth((bool)s);
    }
    fin >> heading;
  }
  config->close();
  statusBar()->showMessage("Loaded Config", 2000);
  */
}

void MW::saveconfiguration(QFile *config) {
  /*
  QTextStream fout(config);
  config->open(QIODevice::WriteOnly);
  fout << "SMOOTHNESS " << viewer->getSmoothness() << "\n";
  fout << "ENHANCED_RESIZE " << viewer->getEnhance() << "\n";
  fout << "PAGE_SMOOTH " << viewer->getPageSmooth() << "\n";
  config->close();
  statusBar()->showMessage("Saved Config", 2000);
  */
}

// --------------------- Slots --------------------
void MW::saveconfig() {
  saveconfiguration(
      new QFile(QString(getenv("VLABCONFIGDIR")) + QString("/medit")));
}

void MW::editSmooth() {
  imqual->getImageQuality(viewer->getSmoothness(), viewer->getEnhance(),
                          viewer->getPageSmooth());
}

void MW::pdfHelp() {
  QDir helpDir(getHelpDirectory());
  QDesktopServices::openUrl(
      QUrl::fromLocalFile(helpDir.filePath("VLABToolsManual.pdf")));
}

void MW::quickHelp(){
  QDir helpDir(getHelpDirectory());
#ifndef __APPLE__
  helpDir.cd("Quick_Help");
#endif
  QString path = helpDir.filePath("MeditQuickHelp.html");
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
  msgBox->setWindowTitle("Medit: Quick Help");
  msgBox->setWindowFlags(Qt::Dialog);
  //msgBox->setWindowModality(Qt::WindowModal);
  msgBox->setModal(false);
  QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok);
  QPushButton* okBtn = bb->button(QDialogButtonBox::Ok);
  connect(okBtn, SIGNAL(clicked()),msgBox,SLOT(close()));
  QVBoxLayout *layout = new QVBoxLayout;
  layout->addWidget(tb);
  layout->addWidget(bb);
  msgBox->setLayout(layout);
  msgBox->resize(400,300);


  /*
  QMessageBox *msgBox = new QMessageBox;
  msgBox->setTextFormat(Qt::RichText);
  msgBox->setText(message);
  msgBox->setModal(false);
  */
  
  msgBox->show();
}


void MW::onlinehelp() {
  QString mess;
  mess =
      "<h2>"
      "Quick Help for VLAB Material Editor</h2>"
      ""
      "<h2>"
      "<b><i>1 Medit Execution</i></b></h2>"
      "Execute the VLAB Material Editor using the following arguments:"
      "<p><b>>&nbsp; medit</b>"
      "<br>Open an empty gallery for up to 256 materials."
      "<p><b>>&nbsp; medit [file.mat] ...</b>"
      "<br>Opens the specified material file(s)."
      "<br>If the file does not exist, creates new file."
      "<br>If a list of material files is specified, each is open in a separate"
      "application window."
      "<p><b>>&nbsp; medit ... -w [width] [height]</b>"
      "<br>Opens medit with specified dimensions"
      "<br><i></i>&nbsp;"
      "<h2>"
      "<i>2 View and Selection</i></h2>"
      ""
      "<ul>"
      "<li>"
      "When the program starts, 16 of the 256 spheres are displayed, beginning"
      "on the first page. Index 0 is in the upper left corner, with consecutive"
      "entries vertically, top to bottom.&nbsp; To display more or less spheres"
      "per page, select <b><i>View</i></b> from the <b>menu bar </b>at the"
      "top of the window, then select the desired <b>page size</b> (1, 4x4, "
      "8x8,"
      "16x16).</li>"
      ""
      "<li>"
      "A <b>material index may be selected </b>by clicking the <b>left "
      "mouse</b>"
      "button on the entry, or by entering the index in the first spinbox in "
      "the"
      "lower right corner.</li>"
      ""
      "<li>"
      "A <b>range of indices</b> can be selected by clicking the left mouse on"
      "the lesser index, and dragging to the greater index.&nbsp; A range can"
      "also be selected by selecting <b><i>Edit</i></b> from the menu bar, and"
      "then selecting <b><i>Select Range...</i></b>, which opens a dialog box"
      "to input the start index and end index or number selected.</li>"
      ""
      "<li>"
      "All materials on the current page, or all materials in the gallery, can"
      "be selected by selecting <b><i>Edit </i></b>from the menu bar, then "
      "<b><i>Select"
      "All in Page</i></b> or <b><i>Select All in Material</i></b>.</li>"
      ""
      "<li>"
      "Selected indices are <b>outlined</b> in red.</li>"
      ""
      "<li>"
      "A <b>page may be jumped to </b>by entering the page number in the second"
      "spinbox in the lower right corner, or by selecting <b><i>View</i></b>"
      "from the menu bar and then selecting <b><i>Next Page, Previous "
      "Page</i></b>"
      "or <b><i>First Page</i></b>.&nbsp; The number of pages, as well as the"
      "page a particular material falls on, depends on the page size.&nbsp; The"
      "indices materials that are displayed on the current page are shown in "
      "the"
      "upper right corner of the window.&nbsp; When a page is changed, the "
      "current"
      "selection is not affected.</li>"
      ""
      "<li>"
      "The background is black by default.&nbsp; To show transparency, it is "
      "useful"
      "to <b>change the background to a checkerboard pattern</b>.&nbsp; To do"
      "this, select <b><i>View </i></b>from the menu bar, then select "
      "<b><i>Show"
      "Background</i></b>.&nbsp; To return to a solid black background, select"
      "this option again.</li>"
      ""
      "<li>"
      "The spheres are rendered as a polygon mesh between vertices.&nbsp; The"
      "rendering speed varies inversely to the smoothness of the spheres.&nbsp;"
      "To <b>adjust the smoothness</b>, and to set the auto-adjustment of the"
      "smoothness, select <b><i>View</i></b> from the menu bar, then select"
      "<b><i>Image Quality...</i></b> to edit the smoothenss parameters.&nbsp;"
      "Once the display is satisfactory, the parameters may be written to the"
      "configuration file by clicking the button marked "
      "<b><i>Save</i></b>.</li>"
      "</ul>"
      "<b><u></u></b>"
      "<p><br><b><u>2.1 Actions on Selection</u></b>"
      "<p>The <b>Popup Menu</b> is activated by clicking the <b>right mouse</b>"
      "button anywhere on the material gallery.&nbsp; The popup menu contains"
      "actions to be performed on the selection."
      "<br>The same options are available by selecting <b><i>Edit</i></b> from"
      "the menu bar."
      "<br>&nbsp;"
      "<p><b>Cut, Copy, Paste, Insert</b>"
      "Cut, Copy, Paste, and Insert can be invoked by <b>activating the popup"
      "menu</b> or by selecting <b><i>Edit </i></b>from the menu bar, and then"
      "selecting the desired option."
      ""
      "<ul>"
      "<li>"
      "If a selection is <b>cut</b>, all following entries are shuffled up, and"
      "the gallery is padded with default materials.</li>"
      ""
      "<li>"
      "If a selection is <b>inserted</b>, all following entries (<i>including"
      "the current selection</i>) are shuffled down, and extra entries are "
      "pushed"
      "out.</li>"
      ""
      "<li>"
      "If the selection that is <b>inserted or pasted</b> is larger than the "
      "space"
      "remaing in the gallery, the extra entries are ignored.</li>"
      "</ul>"
      "<b></b>"
      "<p><br><b>Interpolation</b>"
      "<br>All material properties can be&nbsp; interpolated over&nbsp; a "
      "selected"
      "a range of indices by <b>activating the popup menu</b> or by selecting"
      "<b><i>Edit </i></b>from the menu bar, and then selecting "
      "<b><i>Interpolate.</i></b>"
      "<br><b><i></i></b>&nbsp;<b></b>"
      "<p><b>Set to Default</b>"
      "<br>A selected index or range of selected indices can be set to the "
      "OpenGL"
      "default&nbsp; material by <b>activating the popup menu</b> or by "
      "selecting"
      "<b><i>Edit </i></b>from the menu bar, and then selecting <b><i>Set to "
      "Default.</i></b>"
      "<br>This can also be done through the <b>M-Edit Dialog</b> (<i>described"
      "below</i>) by clicking the button marked <b><i>Default.</i></b>"
      "<br>&nbsp;<b></b>"
      "<p><b>M-Edit</b>"
      "<br>A material editing dialog (<i>see below</i>) is opened for the "
      "selected"
      "material (or a range of materials) by <b>activating the popup menu</b>"
      "by right clicking on the gallery, or by selecting <b><i>Edit "
      "</i></b>from"
      "the menu bar, and then selecting <b><i>M-Edit.</i></b>"
      "<br>&nbsp;<b><u></u></b>"
      "<p><b><u>2.2 Material Edit Dialog</u></b>"
      "<p>A preview of the selected sphere is displayed on the left of the "
      "dialog."
      "<br>The material properties ambient, diffuse, specular, emissive, "
      "shininess,"
      "and transparency can be edited using the sliders or by entering a value"
      "in the edit boxes to the right of the sliders (the <b><i>return key "
      "</i></b>must"
      "be pressed after editing in the boxes)."
      "<br>&nbsp;<b></b>"
      "<p><b>Material Properties</b>"
      "<ul>"
      "<li>"
      "<b>Ambient</b>, <b>diffuse</b>,&nbsp; <b>specular </b>and "
      "<b>emissive</b>"
      "are values between 0 (none) and 1 (full).</li>"
      ""
      "<li>"
      "<b>Shininess</b> is a value between 0(non-shiny) and "
      "128(max-shiny).&nbsp;"
      "If a material has a specular property, for best effect modify shininess"
      "as well.</li>"
      ""
      "<li>"
      "<b>Transparency</b> is a value between 0(opaque) and "
      "1(transparent).</li>"
      "</ul>"
      "<b></b>"
      "<p><br><b>Colour Swatches</b>"
      "<ul>"
      "<li>"
      "The colour swatch <b>popup menu</b> is activated by pressing the "
      "<b>right"
      "mouse</b> button.</li>"
      ""
      "<li>"
      "The colour of ambient, diffuse,&nbsp; specular and emissive can be "
      "<b>edited</b>"
      "by pressing the left mouse button over the colour swatch to the left of"
      "the property labels, or by activating the popup menu and&nbsp; selecting"
      "the option <b><i>Edit</i></b> from the popup menu.&nbsp; <b>The HSV "
      "Colour"
      "Picker</b> is opened to edit the colour.</li>"
      ""
      "<li>"
      "The left side of the colour swatch displays the actual colour at the "
      "current"
      "value of the material property.&nbsp; The smaller section on the right"
      "displays the full intensity of the colour.</li>"
      ""
      "<li>"
      "The full intensity of the colour can be <b>copied and pasted</b> to "
      "another"
      "colour swatch on the dialog.</li>"
      ""
      "<li>"
      "If the material property has a value of 0, the full intensity is white,"
      "and the actual colour is black.</li>"
      "</ul>"
      "<b></b>"
      "<p><br><b>Modifications and Action Buttons</b>"
      "<ul>"
      "<li>"
      "If the selected material has been modified, the user cannot select "
      "another"
      "material index in the gallery until the modification has been applied or"
      "undone by clicking the button marked <b><i>Apply</i></b> or the one "
      "marked"
      "<b><i>Undo.</i></b></li>"
      ""
      "<li>"
      "A label displays <b><i>Modified</i></b> below the preview if the dialog"
      "material has been modified.</li>"
      ""
      "<li>"
      "If the button marked <b><i>Default</i></b> is clicked, the material is"
      "set to the OpenGL default material.</li>"
      ""
      "<li>"
      "If the dialog is closed by clicking the button marked "
      "<b><i>Close</i></b>,"
      "the material remains as it appears in the gallery.</li>"
      "</ul>"
      "<b><u></u></b>"
      "<p><br><b><u>2.3 HSV Colour Picker</u></b>"
      "<ul>"
      "<li>"
      "The <b>HSV Colour Picker</b> is invoked by pressing the <b>left "
      "mouse</b>"
      "over a colour swatch, or by <b>activating the popup menu</b> and&nbsp;"
      "selecting the option <b><i>Edit</i></b> from the popup menu.</li>"
      ""
      "<li>"
      "The colour being selected is the <b>actual colour</b> (rather than the"
      "full intensity).&nbsp; The intensity of the material property can be "
      "adjusted"
      "by moving the slider labelled <b><i>V </i></b>on the left, or by moving"
      "the <b>colour value slider</b> on the right of the colour wheel, or by"
      "moving the slider associated with the material property on the <b>M-Edit"
      "Dialog</b>.</li>"
      ""
      "<li>"
      "The colours <b>HSV or RGB</b> values can be adjusted using the "
      "<b>sliders</b>"
      "on the left side of the dialog, or by entering the value in the edit "
      "boxes"
      "to the right of the sliders (the <b><i>return key </i></b>must be "
      "pressed"
      "after editing in the boxes).</li>"
      ""
      "<li>"
      "The <b>hue and saturation</b> of the colour can be selected on the "
      "rainbow"
      "colour wheel with the mouse by pressing the <b>left mouse</b> button to"
      "select a colour, and <b>moving the mouse</b> to adjust the "
      "selection.&nbsp;"
      "The hue line and saturation circle are drawn to aid in the "
      "selection.</li>"
      ""
      "<li>"
      "The <b>large box</b> below the colour wheel shows the <b>current "
      "selection</b>.&nbsp;"
      "The selection is automatically applied.</li>"
      ""
      "<li>"
      "The <b>smaller box</b> below the colour value slider shows the "
      "<b>original"
      "colour</b>.&nbsp; To <b>revert</b> to this colour, <b>left click</b> "
      "inside"
      "this box.</li>"
      ""
      "<li>"
      "If the colour picker is closed by clicking the button marked "
      "<b><i>Close</i></b>,"
      "the material property colour remains as it appears in the colour swatch"
      "on the <b>M-Edit Dialog</b>.</li>"
      "</ul>"
      ""
      "<h2>"
      "<i></i></h2>"
      ""
      "<h2>"
      "<i>3 File Options</i></h2>"
      "<b>New </b>Opens a new material gallery in the current VLAB Material "
      "Editor."
      "<br><b>New Window </b>Opens a new material gallery in a newVLAB Material"
      "Editor application window."
      "<br><b>Load...</b> Opens a file dialog in which the user can select an"
      "existing file to load in the current VLAB Material Editor."
      "<br><b>Load Window... </b>Opens a file dialog in which the user can "
      "select"
      "an existing file to load in a newVLAB Material Editor application "
      "window."
      "<br><b>Save </b>Saves the current material file.&nbsp; If no file has "
      "been"
      "specified, a file dialog is opened."
      "<br><b>Save as... </b>Opens a file dialog in which the user can specify"
      "a new file to save the current material file as."
      "<br><b>Revert to Saved </b>Rereads the material file as it was last "
      "saved"
      "and restores the materials in the gallery.";

  QScrollArea *helpscroll = new QScrollArea();
  QLabel *label = new QLabel("", helpscroll->viewport());
  label->setTextFormat(RichText);
  label->setText(mess);
  helpscroll->setWidget(label);
  helpscroll->show();
}

// signal recieved from viewer
void MW::setselect(int i, Material m) {
  indexstepper->setValue(i);
  editor->selectM(i, m);
}

// disable selection when editing
void MW::selectOff(bool s) {
  menuBar()->setDisabled(s);
  indexstepper->setDisabled(s);
  viewer->selectOff(s);
}

void MW::about() {
  QMessageBox box;
  box.setWindowTitle("About");
  box.setText(
      "VLAB Material Editor\nJoanne Penner\nUniversity of Calgary, 2001");
  box.setStandardButtons(QMessageBox::Ok);
  box.setDefaultButton(QMessageBox::Ok);
  box.setIcon(QMessageBox::Information);
  box.exec();
 }

void MW::setModified(bool m) {
  modified = m;
  _sv->setEnabled(m);
  if (m) {
    if (materialfile) {
      _rs->setEnabled(true);
    }

    else {
      _rs->setEnabled(false);
    }
  } else {
    _rs->setEnabled(false);
  }
}

bool MW::askSave(QString message) {
  if (message.isNull())
    message = "Save before creating new material ?";
  if ((modified) && (_savingMode == OFF)) {
    QString fn;
    if (filename)
      fn = *filename;
    else
      fn = "noname.mat";
    QString m = QString(
			"File %1 has been modified.\n").arg(fn) + message;
    QMessageBox msgBox(
        QMessageBox::Warning, QString("Save File %1?").arg(fn),m,
        QMessageBox::Yes | QMessageBox::No, this, Qt::Dialog);
    msgBox.setDefaultButton(QMessageBox::No);

     int answer = msgBox.exec();
    switch (answer) {
    case QMessageBox::Yes:
      save();
      break;
    default:
      break;
    }
  }
  return true;
}

// open new file in this window
void MW::newfile() {
  if (!askSave())
    return;
  filename = new QString("noname.mat");
  setWindowTitle(*filename);
  materialfile = NULL;
  emit read(materialfile);
}

void MW::newWindow() { system("medit &"); }

void MW::loadWindow() {
  QString fn = QFileDialog::getOpenFileName(this, "Material Files (*.mat)", ".",
                                            tr("Material (*.mat)"));
  if (!fn.isEmpty()) {
    loadnewfile(fn);
  }
}

void MW::loadnewfile(QString fn) {
  QString arg = "medit ";
  arg += fn;
  arg += " &";
  system((const char *)arg.toStdString().c_str());
}

// load new file in this window
void MW::load() {
  if (!askSave())
    return;
  QString fn = QFileDialog::getOpenFileName(this, "Material Files (*.mat)", ".",
                                            tr("Material (*.mat)"));
  if (!fn.isEmpty()) {
    switch (loadfile(fn)) {
    case 0:
      load();
      return;
      //      break;
    case 1:
      break;
    case -1:
    case -2:
    case -3:
      return;
      //      break;
    }
  } else
    statusBar()->showMessage("Abort Load", 2000);
}

// returns 1 if all is well
// returns 0 if user wishes to try again
// returns - if user decides to cancel
//
// automatically creates new file if it doesn't exist
int MW::loadfile(QString fn) {

  if ((fn.length() < 4) || (fn.lastIndexOf(".mat") != (fn.length() - 4)))
    fn += ".mat";

  QString fd = fn;
  int dir = fd.lastIndexOf("/");
  if (dir != -1)
    fd.remove(0, dir + 1);

  QFile *f = new QFile(fn);

  // if the file is too big, try again?
  if (f->exists() && (f->size() > 3840)) {
    switch (QMessageBox::warning(this, "File Too Big",
                                 fd + " is not a material file.\nMaterial "
                                      "files can be at most 3840 bytes.",
                                 "Try Again", "Cancel Load")) {
    case 0:
      return 0;
      break;
    case 1:
    default:
      return -2;
      break;
    }
  }
  // do we really want it to ask this?
  // if the file does not exist, create?
  if (!f->exists()) {
    switch (QMessageBox::warning(
        this, "Cannot find file " + fd,
        "The file you specified does not exist.\n\nCreate new material named " +
            fd + "?",
        "Create", "Cancel")) {
    case 0:
      setWindowTitle(fd);
      filename = new QString(fd);
      materialfile = new QFile(fd);
      ;
      emit read(materialfile);
      _sv->setEnabled(true);
      setModified(true);
      emit modify_save_viewer(true);

      return 1;
      break;
    default:
      return -3;
      break;
    }
  }
  materialfile = new QFile(fn);
  setWindowTitle(fd);
  filename = new QString(fd);
  emit read(materialfile);
  return 1;
}

void MW::revertsaved() {
  if (materialfile) {
    emit read(materialfile);
  } else
    statusBar()->showMessage("Never Been Saved.", 3500);
}

// save the current material file
void MW::save() {
  if (!materialfile) {
    if (modified) {
      saveas();
    } else
      statusBar()->showMessage("No Changes", 3500);
  } else {
    if (modified) {
      emit write(materialfile);
    } else
      statusBar()->showMessage("No Changes", 3500);
  }
}

// save the current material file under new name
void MW::saveas() {
  QString chosenFilterString = "Material Files (*.mat)";

  QString fn = QFileDialog::getSaveFileName(this, QString("Save Material"),
                                            *filename, QString(""));

  if (!fn.isEmpty()) {
    if (fn.lastIndexOf(".mat") != (fn.length() - 4)) {
      switch (QMessageBox::information(
          this, "File Extension",
          "File " + fn +
              " doesn't have .mat extension.\nChange file extension to .mat ?",
          "Yes", "No", "Cancel")) {
      case 0:
        fn += ".mat";
        break;
      case 1:
        break;
      case 2:
      default:
        return;
      }
    }
    QString fd = fn;
    int dir = fd.lastIndexOf("/");
    if (dir != -1)
      fd.remove(0, dir + 1);

    QFile *f = new QFile(fn);

    if (f->exists()) {
      switch (QMessageBox::information(this, "Overwrite File",
                                       "File " + fd + " exists.\nOverwrite?",
                                       "Overwrite", "No", "Cancel")) {
      case 0:
        break;
      case 1:
        saveas();
        return;
      case 2:
      default:
        return;
      }
    }

    materialfile = new QFile(fn);
    setWindowTitle(fd);
    filename = new QString(fd);
    emit write(materialfile);
  } else
    statusBar()->showMessage("Abort Save", 2000);
}

// coming soon...
void MW::pref() {}

void MW::list() {}

// display a notice to the user
void MW::notice(const QString &message) {
  statusBar()->showMessage(message, 3500);
}

// in case an error occurs (eg. parsing input file)
void MW::error(const QString &, int) {}

// toggle background display
void MW::bg() {
  if (_bg->isChecked()) {
    _bg->setChecked(false);
    emit imageON(false);
  } else {
    _bg->setChecked(true);
    emit imageON(true);
  }
}

void MW::enablePaste() {
  _ps->setEnabled(true);
  _in->setEnabled(true);
}

// flip to another page and update stuff
void MW::pageflip(int start, int pgsz) {
  int pgno = (start / pgsz) + 1;
  pagelabel->setText(" of " + QString::number(256 / pgsz) + " ");
   pagestepper->setMaximum(256 / pgsz);
  pagestepper->setValue(pgno);

  if (pgno == 1) {
    _pv->setEnabled(false);
    _fp->setEnabled(false);
  } else {
    _pv->setEnabled(true);
    _fp->setEnabled(true);
  }
  if (((pgno + 1) * pgsz) > 256)
    _nx->setEnabled(false);
  else
    _nx->setEnabled(true);

  switch (pgsz) {
  case 1:
    _xs->setChecked(true);
    _sm->setChecked(false);
    _md->setChecked(false);
    _lg->setChecked(false);
    break;
  case 16:
    _xs->setChecked(false);
    _sm->setChecked(true);
    _md->setChecked(false);
    _lg->setChecked(false);
    break;
  case 64:
    _xs->setChecked(false);
    _sm->setChecked(false);
    _md->setChecked(true);
    _lg->setChecked(false);
    break;
  case 256:
    _xs->setChecked(false);
    _sm->setChecked(false);
    _md->setChecked(false);
    _lg->setChecked(true);
    break;
  default:
    _xs->setChecked(false);
    _sm->setChecked(false);
    _md->setChecked(false);
    _lg->setChecked(false);
    break;
  }
}

// --------------------- Event Handling --------------------
// check for modifications, then exit
void MW::closeEvent(QCloseEvent *ce) {
  if (!askSave(QString("Save before exiting")))
    return;
  ce->accept();
  emit quit();
}

// eof: mw.h
