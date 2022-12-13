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




#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <string>

class Config {
 private:
  Config() {}
  Config(const Config&) {}

 public:
  static void readConfigFile(std::string fileName);
  static void readConfigFile();

  static int getVSize()     {return vsize;}
  static int getHSize()     {return hsize;}
  static int getDragRed()   {return drag_red;}
  static int getDragGreen() {return drag_green;}
  static int getDragBlue()  {return drag_blue;}
  static int getItemWidth() {return item_width;}
  static int getMargins() {return margins;}

  class ConfigErrorExc {
  public:
    ConfigErrorExc();
    ConfigErrorExc(std::string msg) : _msg(msg) {}
    const std::string& message() {return _msg;}
  private:
    std::string _msg;
  };

 protected:
  // sizes
  static int vsize;
  static int hsize;
  static int drag_red;
  static int drag_green;
  static int drag_blue;
  static int item_width;
  static int margins;
};

#endif
