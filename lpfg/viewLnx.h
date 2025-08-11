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



#ifndef __VIEW_H__
#define __VIEW_H__

#include <QContextMenuEvent>
#include <QTabletEvent>
#include <QCloseEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QMenu>
#include <QMainWindow>
#include <QMenuBar>
#include <QTimer>
#include <QGridLayout>

#include "volume.h"
#include "projection.h"
#include "rect.h"
#include "menu.h"
#include "viewtask.h"

// we need to include lintrfc for MouseStatus,
// and lparams for lintrfc
#include "include/lparams.h"
#include "include/lintrfc.h"

class LPFG;
class Clipping;
class WindowParams;
class DirectoryWatcher;

#include "comlineparam.h"
#include "glwidget.h"
enum PixFormat { BMP, GIF, JPG, PBM, PDF, PNG, TIFF };

class View : public QMainWindow {
  Q_OBJECT
public:
  struct CrSt {
    CrSt(LPFG *pLpfg, int id) : _pLpfg(pLpfg), _id(id) {}
    LPFG *_pLpfg;
    int _id;
  };

  static View *Create(const std::string &, const Rect &, LPFG *, int);
  ~View();

  View(const std::string &, const Rect &, LPFG *, int);
  void createMenu();

  void ResetView();
  void Resize(int, int);
  void UserContextMenu(int, int);
  void ContextMenu(int, int);
  void Command(int);

  void ShowAxis(bool);
  void ResetRotation();
  void ResetZoom();
  void ResetPan();
  void ResetRoll();

  bool IsVisible() const;
  void Show(int);
  void Show();
  void Show(const WindowParams &);
  void ShowMaximized();

  void DrawParamsChanged();

  void makeCurrent(void) const;

  void InsertX(int, int);
  void InsertPosX(int, int);

  void FillMouseStatus(MouseStatus &);

  static bool IsClean() { return 0 == _counter; }
  void Destroy();

  void Resize(int, int, int, int);
  void GetRelSize(const Rect &, float *) const;

  void Init(const bool shadows);
  bool IsRecording() const;
  PixFormat getFormat() const { return _pixFormat; }
  int getOutputFormat() const { return _outputFormat; }

  bool isAxisOn() { return _AxisOn; }
  bool isRecordingOn() { return _RecordingOn; }

  void setFormat(PixFormat format) { _pixFormat = format; }
  void setOutputFormat(int format) { _outputFormat = format; }

  std::string getPathToSave() const { return _pathToSave; }
  void setPathToSave(const std::string path) { _pathToSave = path; }

  void setNumberingImage(int n) { _numberingImageName = n; }
  // why are there two image file names: _imageBaseName and _filename?
  // And two functions to set them: setImageName() and setFilename()?
  void setImageName(const std::string name) { _imageBaseName = name; _filename = name; }
  void setLBMouseStatus(const bool b) { _mouseStatus.lbDown = b; }

protected:
  void resizeEvent(QResizeEvent *) override;
  void closeEvent(QCloseEvent *)override{};
  void showEvent(QShowEvent *) override;

private:
  void _DrawExpired() const;
  void _SaveFrame(int) const;
  Rect  getRectangleFromView(int id);

  Projection _projection;
  MouseStatus _mouseStatus;

  PixFormat _pixFormat;
  int _outputFormat;
  std::string _pathToSave;
  std::string _imageBaseName;
  bool _alphaChannel;
  int _numberingImageName;
  QTimer _resizeTimer;
  int waitOpenFile(const char *fname);
  QOpenGLWidget::UpdateBehavior _openGlBehavior;

  bool _dontPaint;
  bool clear;

  QGridLayout *m_layout;
  QWidget *_centralWidget;
  int _glWidgetClicked; // to remember in which widget a clicked as been made

public:
  LPFG *_pLpfg;
  bool dontPaint() { return _dontPaint; }
  void setDontPaint(bool b) { _dontPaint = b; }
  LPFG *getLpfg() { return _pLpfg; }

  void setGLWidgetClicked(int id) { _glWidgetClicked = id; }
  int getGLWidgetClicked() const { return _glWidgetClicked; };

  void SaveFrame(const char *, int);

  int getId() const { return _id; }
  void setId(const int id) { _id = id; }

  int getFilenameId() const { return _fileNameid; }

  void setFileNameId(const int id) { _fileNameid = id; }

  void setFilename(const std::string &filename);
  bool isRunning();
  bool isRunningForever();
  

public:
  void StartTimer();
  void StopTimer();
  GLWidget *getGLWidget(const int i) { return m_glWidgets[i]; }
  void update();
  void SetTitle();

private:
  void timerEvent(QTimerEvent *) override;
  int _idTimer;

private:
  int _id;
  int _fileNameid;
  std::string _filename;

  qreal m_previousDevicePixelRatio;

  QList<QDockWidget *> docks;
  std::string getExtension();
  bool OverwriteDialogBox(const char *sfilename);
  void updateFilename();
  void InitializeDocks();

  QString _title;
  QVector<GLWidget *> m_glWidgets;
  QVector<QWidget *> layoutWidgets;

  int new_model_pending;
  DirectoryWatcher *directoryWatcher;

  SavingMode _savingMode;
  QMenu *_pMenu;
  QMenu *_pSecondaryMenu;
  QMenu *_pOutputFormat;
  QMenu *helpMenu;

  QAction *_saveAction;
  QAction *_clearAction;

  QAction *explicitMode;
  QAction *continuousMode;

  QAction *helpAct;
  QAction *pdfHelpAct;

  bool _mPopmenu;
  bool _sPopmenu;
  // PixFormat _pix_format;
  QAction *_recordingId;
  QAction *_format_id_bmp, *_format_id_jpg, *_format_id_gif, *_format_id_pbm,
      *_format_id_tiff, *_format_id_png;

  bool _actionTriggered;
  bool _isRunning;
  bool _isRunningForEver;

  QPoint _mouse_init_position;

  int _proportion;

  bool _AxisOn;
  bool _RecordingOn;

  unsigned int _flist;
  unsigned int _oflist;
  void *_pQ;

  class CurrentContext {
  public:
    CurrentContext(const View *pView) { Q_UNUSED(pView); }
    ~CurrentContext() {}
  };
  friend class View::CurrentContext;

public:
  class GLRestore {
  public:
    GLRestore(GLenum cap) : _cap(cap), _enable(glIsEnabled(_cap)) {}
    ~GLRestore() {
      if (_enable)
        glEnable(_cap);
      else
        glDisable(_cap);
    }

  private:
    const GLenum _cap;
    const GLboolean _enable;
  };

public:
  void ResetFrustum();
  void ShowAxis();
  void OutputBmp();
  void OutputRGB() const;
  void _SaveAsBmp(const char *) const;
  void _DoInsertX(int, int);
  void _DoInsertPosX(int, int);
  void addNew();
  void setlayout();
  void RunSimulation() { Run(); };

signals:
  void keyPress(QKeyEvent *);
  void keyRelease(QKeyEvent *);

public slots:
  void save();
  void resizeDone();
  void addDockWindows(const std::string &title, const Rect &r, int i);
  void removeDockWindows(int id);
  void placeDockWindows(int id, const Rect &r);
  void resetDockWindowPosition();
								     

private slots:
  void addNewWidget() { addNew(); }
  void Step();
  void Run();
  void Forever();
  void Stop();

  void Pause();

  void Rewind();
  void NewModel();
  void NewLsystem();
  void Rerun();
  void NewView();
  void RereadDrawParams();
  void RereadAnimParams();

  void Clear();
  void Recording();

  void RereadColors();
  void RereadSurfaces();
  void RereadContours();
  void RereadFunctions();

  void StringLoad();
  void StringOutput();
  void SetExplicitMode(bool);
  void SetContinuousMode(bool);

  void RestoreState();
  void ActionTriggered();
  bool getAlphaChannel() { return _alphaChannel; }

  void Idle();
  void RequestNewModel(QString);
  int ActionToBePerformedInContinuousMode(QString f);
  void ExecuteAction(int code);

  void saveAs();
  void Animate();

  void aboutCB();
  void helpCB() const;
  void pdfHelpCB() const;
  void quickHelp();
  void mousePressed(const QPoint &);
  void mouseMoved(const QPoint &);
  void mouseReleased(const QPoint &);
  void resetWindowTitle();

  void handleScreenChanged(QScreen *screen);

protected:
  void contextMenuEvent(QContextMenuEvent *) override;
  void keyPressEvent(QKeyEvent *) override;
  void keyReleaseEvent(QKeyEvent *) override;

 public:
  void setProportion(int p) { _proportion = p; }
  void RecordingForPovray();
  void OutputPovray(int frameNumber) const; // Used when recording
  void OutputPovray(const char *FolderName) const;
  void OutputPovray() const;
  void PovrayCameraParallel(std::ostream &) const;
  void PovrayCameraPerspective(std::ostream &) const;

  void OutputObj();
  void OutputRayshade();
  void OutputPostscript();
  void OutputView();

private:
  static int _counter;
};

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
