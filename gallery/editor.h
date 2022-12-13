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




#ifndef __EDITOR_H__
#define __EDITOR_H__

#include <string>
#include <QProcess>

enum SavingMode {CONTINUOUS,TRIGGERED, OFF};

class Editor {
 public:
  void launch(std::string filename,SavingMode savingMode,int posx, int posy);
  QProcess* getProcess() { return process;}
 protected:
  std::string command;
  QProcess *process;
  
};

class FuncEditor : public Editor {
 public:
  FuncEditor();
};

class ConEditor : public Editor {
 public:
  ConEditor();
};


#endif
