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



#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QActionGroup>
#include <platform.h>
#include "common.h"
#include "rubberband.h"
#include "Preferences.h"

class QCloseEvent;

class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  MainWindow();
  void draw_frame();
  void setPreferences();

private slots:
  void about();
  void aboutQt();
  void help();
  void quickHelp();
  void size_actual();
  void size_double();
  void size_quadruple();
  void size_aspect();
  void size_free();
  void format_rgb();
  void format_bmp();
  void format_jpeg();
  void format_pbm();
  void format_pgm();
  void format_png();
  void format_ppm();
  void format_xbm();
  void format_xpm();
  void save();
  void saveIcon();
  bool saveRGB();
  void save_as();
  void preview();
  void snap_save_exit();
  void save_exit();
  void snap();
  void newScreenShot();
  void exit();
  void open_menu();
  void editPreferencesCB();
  bool OverwriteDialogBox(const char *sfilename);


protected:
  void resizeEvent(QResizeEvent *);

private:
  void createActions();
  void createMenus();
  void createBarMenus();
  void snapSaveExit();

  QMenu *helpMenu;
  RubberBand *rubberband;

  QAction *aboutAct;
  QAction *aboutQtAct;
  QAction *helpAct;
  QAction *quickHelpAct;

  // Context menu widgets
  QMenu *menu;
  QMenu *size_menu;
  QMenu *format_menu;
  QMenu *snap_menu;
  QMenu *option_menu;

  // Snap image
  QPixmap snap_image;

  // Context menu entry id
  QAction *saveAct, *saveAsAct, *previewAct, *optionAct, *snap_save_exitAct,
      *save_exitAct, *exitAct, *snapAct;
  QAction *sizeActActual, *sizeActDouble, *sizeActQuadruple, *sizeActAspect,
      *sizeActFree;
  QActionGroup *sizeActGroup;

  QAction *changeColorAct, *changeBorderAct, *preferencesAct;

  QAction *formatAct;
  QAction *formatAct_rgb, *formatAct_bmp, *formatAct_jpeg, *formatAct_pbm,
      *formatAct_pgm, *formatAct_png, *formatAct_ppm, *formatAct_xbm,
      *formatAct_xpm;
  QActionGroup *formatActGroup;

  ::SizeMode size_mode;                   // Snapframe size mode
  int is_win_move;                        // Control panel move flag
  int is_desktop_freeze;                  // Desktop freeze flag
  int is_panel_hide;                      // Control panel hide on snap flag
  int offsetX, offsetY;                   // Mouse offsets
  int frame_x, frame_y, frame_w, frame_h; // Snapframe dimension
  int ctrl_x, ctrl_y, ctrl_w, ctrl_h;     // Snap control panel dimension
  int in_proc_menu;                       // Context menu processing flag
  int dtW, dtH;                           // Desktop width height
  int snap_delay;                         // Snap shot delay
  int is_latest_save;                     // Latest save flag
  int is_image_modified;                  // snap and save flag
  int deviceRatio;
  QString name, basename, pix_format, currentPath;

  int rubberBandBorderSize;
  Preferences *preferences;
};

#endif // MAINWINDOW_H
