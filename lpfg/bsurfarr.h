/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#ifndef __BSURFARR_H__
#define __BSURFARR_H__

#include "b_wrapper.h"

#include <vector>

class BSurfaceArray {
public:
  void AddSurface(const char *);
  bool ValidId(size_t id) const { return id < _arr.size(); }
  b_wrapper &Get(size_t id) {
    ASSERT(ValidId(id));
    return _arr[id];
  }
  void Reread();
  size_t Count() const { return _arr.size(); }

private:
  std::vector<b_wrapper> _arr;
};

extern BSurfaceArray bsurfaces;

#endif
