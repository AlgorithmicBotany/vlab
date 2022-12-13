/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>
#include "BezierWindow.h"
#include "Globals.h"
#ifdef __APPLE__
#include <QtPlugin>
#endif

#ifdef __APPLE__

#include <OpenGL/gl.h>
#include <OpenGL/glu.h>

#else

#ifdef WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>

#endif

void printValidArgs() {
  qDebug()<<"Usage:";
  qDebug()<<"\tstedit -bezier surfacefile";
  qDebug()<<"\tstedit: -bezier surfacefile texturefile";
  qDebug()<<"\tstedit: -warp texturefile";
  qDebug()<<"\tstedit: -both surfacefile texturefile  ";
}

int parseCommandLine(int argc, char **argv, SavingMode &savingMode, int & bezierMode, std::string &bezierName, std::string &textureName, bool &extendedFormatFlag){
  int i = 1;
  savingMode = OFF;
  bezierMode = 0; // bezier only = 0, texture only = 1, both = 2
  bezierName = "";
  textureName = "";
  extendedFormatFlag = false;
  while (i < argc) {
    if (string(argv[i]) == "-ef") {
      extendedFormatFlag = true;
      ++i;
      continue;
    }
 
    if (strcmp(argv[i], "-rmode") == 0) {
      ++i;
      if ((strcmp(argv[i], "expl") == 0) || (strcmp(argv[i], "explicit") == 0))
        savingMode = OFF;
      else if ((strcmp(argv[i], "cont") == 0) ||
               (strcmp(argv[i], "continuous") == 0))
        savingMode = CONTINUOUS;
      else if ((strcmp(argv[i], "trig") == 0) ||
               (strcmp(argv[i], "triggered") == 0))
        savingMode = TRIGGERED;
      else {
	printValidArgs();
        return -1;
      }
      ++i;
      continue;
    }
    if (strcmp(argv[i], "-bezier") == 0){
      ++i;
      bezierMode = 0;
       if (i >= argc){
	printValidArgs();
	return -1;
      }	
    
      bezierName = std::string(argv[i]);
      ++i;
      
      if ((i < argc) && (argv[i][0] != '-')){
	textureName = std::string(argv[i]);
	++i;
      }	
      continue;
    }
    if (strcmp(argv[i], "-warp") == 0){
      ++i;
      bezierMode = 1;
      if (i >= argc){
	printValidArgs();
	return -1;
      }	
      textureName = std::string(argv[i]);
      ++i;
      continue;
    }
    if (strcmp(argv[i], "-both") == 0){
      ++i;
      bezierMode = 2;
      if (i >= argc){
	printValidArgs();
	return -1;
      }	
      bezierName = std::string(argv[i]);
      ++i;
      textureName = std::string(argv[i]);
	++i;
      continue;
    }
    printValidArgs();
    return -1;
  }
  return 0;
}





int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  a.setWindowIcon(QIcon(":/images/icon.ico"));
  QWidget *w;
  SavingMode savingMode; int bezierMode; std::string bezierName;
  std::string textureName; bool extendedFormatFlag;
  int res = parseCommandLine(argc,argv,savingMode,bezierMode,bezierName, textureName, extendedFormatFlag);
  if (res !=0)
    return -1;
  if (bezierMode == 0){
    if (textureName.compare("") == 0)
      w = new BezierWindow(extendedFormatFlag,savingMode,bezierName);
    else
      w = new BezierWindow(extendedFormatFlag,savingMode,bezierName,textureName);
    w->resize(800, 600);
  }
  if (bezierMode == 1){
    w = new TextureWindow(textureName,savingMode);
    w->resize(600, 600);
  }
  if (bezierMode == 2){
    w = new BezierWindow(extendedFormatFlag,savingMode,bezierName,textureName,true);
    w->resize(800, 600);
  }
#ifdef __APPLE__
  w->setWindowIcon(QIcon());
#endif
  w->show();
  return a.exec();
}
