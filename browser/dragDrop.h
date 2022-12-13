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




#ifndef __DRAGDROP_H__
#define __DRAGDROP_H__

#include <QEvent>
#include "tree.h"

typedef struct dragdata_struct DRAGDATA;

struct dragdata_struct {
  char transType[128];
  char data[8096];
};

void startDrag( NODE * node);
void DragDropFinish();
QString ConvertProc(NODE* node);
bool HandleDropFileName(QString data);
void TransferProc(char* type, char* data);
void copyNameIntoNode( char * i_name, NODE * node);
void copy_object_into_node( char * name, NODE * node);
void copy_mobject_into_node( char * name, NODE * node);
void ParseDragData(QString data);
void ParseDragSubData(int idx, QString data);
void scrollWindowRight(bool flag);
void scrollWindowLeft(bool flag);
void scrollWindowDown(bool flag);
void scrollWindowUp(bool flag);

#endif
