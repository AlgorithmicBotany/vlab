#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QScrollArea>
#include <platform.h>
#include "Preferences.h"
#include "timeline.h"

class Timeline;
class QMenu;
class QAction;
class DirectoryWatcher;


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
  Q_OBJECT


public:
  explicit MainWindow(int argc, char *argv[], QWidget *parent = 0);
  ~MainWindow();

  void setPreferences();
  void setSelectedIndex(int index){
    _selectedIndex = index;
  }

protected:
  void resizeEvent(QResizeEvent *event);
  void closeEvent(QCloseEvent *event);
  void mousePressEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void wheelEvent(QWheelEvent *event);

  void mousePressEventInExecuteMode(QMouseEvent *event);
  void mousePressEventInEditMode(QMouseEvent *event);

private slots:
  void newFile();
  void open();
  void save();
  void saveAs();
  void addEvent();
  void deleteSelected();
  void deleteEvent();
  void editEvent();
  void deselectAll();
  void deselectTimelines();
  void moveEventUp();
  void moveEventDown();
  void help();
  void about();
  void pdfHelp();
  void quickHelp();
  void reloadConfig();
  void selectAllLeftOfCursor();
  void openPopupMenu(QMouseEvent*);
  void openFunction();
  void editPreferencesCB();
  void ContinuousSavingMode();
  void TriggeredSavingMode();
  void ModeOff();
  void leftClick(int index);

  void Idle();
  void RequestSave();

  void setToEditMode();
  void setToExecuteMode();
  void change(){
    _change = true;
  }

private:
  void createMenus();
  void createEditMenu();
  void createExecuteMenu();
  void createBarMenu();
  void createActions();

  // Tracks mouse status
  bool mouseLeftPressed;
  bool hasMoved;
  bool _selected;
  bool _leftClick;
  int _selectedIndex;
  float prevMouseX;
  float prevMouseY;
  QPoint prevMouse;

  // Top Menus
  QMenu *fileMenu;
  QMenu *popupMenu;
  QMenu *editMenu;
  QMenu *helpMenu;
  QMenu *menu;
  QMenu *barFileMenu; // new bar menu
  QMenu *executeMenu; // new execute menu
  QMenu *newEditMenu; // new edit menu
  QMenu *modeMenu; // new edit menu

  // Actions if file menu
  QAction *newAct;
  QAction *openAct;
  QAction *saveAct;
  QAction *saveAsAct; // new save as action
  QAction *editAct; // new refresh action
  QAction *executeAct; // new refresh action

  // Actions in edit menu
  QAction *addEventAct;
  QAction *deleteSelectedAct;
  QAction *deleteEventAct;
  QAction *editEventAct;
  QAction *deselectAllAct;
  QAction *moveEventUpAct;
  QAction *moveEventDownAct;

  // Actions in help menu
  QAction *helpAct;
  QAction *reloadConfigAct;

  // Actions without a menu item
  QAction *selectAllRightOfCursorAct;

  QAction *openFunctionAct;

  QAction *exitAct;

  Ui::MainWindow *ui;
  Timeline *timeline;

  SavingMode _savingMode;
  QAction *_savingContinu_act;
  QAction *_savingTriggered_act;
  QAction *_savingMenu_act;
  Preferences *window;
  DirectoryWatcher* _directoryWatcher;
 // timer for idle function
  QTimer *_idleTimer;
  bool _new_save_pending;

  QScrollArea* scrollArea;

  int _mode; // 0 = execute mode; 1 = edit mode
  bool _change;
  bool _selectShift;
  int _selectedNb;
 
};

#endif // MAINWINDOW_H
