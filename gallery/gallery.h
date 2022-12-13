
#ifndef GALLERY_H
#define GALLERY_H

#include "editor.h"
#include "set.h"
#include "item.h"
#include "config.h"

#include <QWidget>
#include <QMenu>
#include <QAction>
#include <QGridLayout>

class DirectoryWatcher;
class Item;
class GLWidget;

#define SCROLLBARSIZE 20
class Gallery : public QWidget
{
    Q_OBJECT

public:
  Gallery(QWidget* parent, int argc, char** argv);
  ~Gallery();
  void CleanUp();
  void loadFile(std::string filename);

  std::string getTmpDir(){
    return _tmpDir;
  }
  void moveItemLeft(Item* pItem);
  void moveItemRight(Item* pItem);
  

  void removeWidgetAtPosition(unsigned int i);
  GLWidget* glWidgetAtPosition(unsigned int i);
  QLabel* labelAtPosition(unsigned int i);

  int getItemWidth(){
    return _itemWidth;
  }
  int getItemHeight(){
    return _itemHeight;
  }

  void activateMenu();

  enum GALLERY_TYPE {
		     FUNC,
		     CON,
		     NONE
  };

  GALLERY_TYPE galleryType(){
    return _galleryType;
  }

  bool hasChanged(){
    return _has_changed;
  }

 
public slots:
  void reloadAll();
  void saveAll();
  void saveFuncSet(std::string);
  void saveConSet(std::string);
  void saveAllAs();


  void createItem();
  void removeItem();
  void duplicateItem();
  void addItemToLayout(Item* it);
  void activateItemMenu(Item*);

  // void duplicateItem(Item *);
  void createItemWithoutDialog();
  void loadItem();
  void quit();

  void addItem(Item* pItem);
  void setSelectedItem(Item* pItem){
    //    std::cerr<<"set Selected Item"<<std::endl;
    _selectedItem = NULL;
    _selectedItem = pItem;
  }
  void unselectItems(){
    _selectedItem = NULL;
  }

  void changeSize();
  void mousePressEvent(QMouseEvent* ev);
  void mouseReleaseEvent(QMouseEvent* ev);
  
  void valueChanged(int v);
  
  void SetContinuousMode(bool enabled);
  void SetTriggeredMode(bool enabled);
  void SetExplicitMode(bool enabled);
  void setSavingMode();
  SavingMode getSavingMode();
  void Idle();
  void RequestReload();

 protected:
  void setGalleryFileName();
  void closeEvent(QCloseEvent *);
  
private:
  void setMenu();
  void parseCommandLine(int argc, char **argv);
  unsigned int positionOfItem(Item*);
  void resizeWindow();

private:
  QMenu* _pContextMnu;
  int _itemWidth;
  int _itemHeight;
  bool _has_changed;

  DirectoryWatcher* _directoryWatcher;
  QAction* _explicitMode;
  QAction* _continuousMode;
  QAction* _triggeredMode;
  QAction* _actionDelete;
  QAction* _actionDuplicate;
  SavingMode _savingMode;

  std::string _tmpDir;

  // timer for idle function
  QTimer *_idleTimer;
  bool _new_reload_pending;

  QString _galleryFilename;

  Set*  _set;
  std::vector<Item*> _items;
  GALLERY_TYPE _galleryType;
  Item* _selectedItem;


  // to retain the position of items
 public:
   int  posx(){
     return _posx;
   }
   void  setPosx(int x){
      _posx = x;
   }
   int  posy(){
     return _posy;
   }
   void  setPosy(int x){
      _posy = x;
   }

 private:
   
   int _posx;
   int _posy;
  int _margins;

  QGridLayout * _layout;


};

#endif
