/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#ifndef __USERMENU_H__
#define __USERMENU_H__

#include <map>
using std::multimap;
#include <string>
using std::string;

struct UserMenu {
  typedef multimap<unsigned int, string> Map;
  Map entries;

  UserMenu(void) : entries() {}
  void add(const string &name, unsigned int refnum) {
    entries.insert(Map::value_type(refnum, name));
  }
  void clear(void) { entries.clear(); }
  bool empty(void) const { return entries.empty(); }
};

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
