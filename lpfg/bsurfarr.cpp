/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#include "bsurfarr.h"

BSurfaceArray bsurfaces;

void BSurfaceArray::AddSurface(const char *line) {
  b_wrapper news(line);
  _arr.push_back(news);
  _arr[_arr.size() - 1] = news;
}

void BSurfaceArray::Reread() {
  for (std::vector<b_wrapper>::iterator it = _arr.begin(); it != _arr.end();
       ++it)
    it->Reread();
}
