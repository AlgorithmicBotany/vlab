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



#include <cstdio>
#include <cstring>
#include <sstream>
#include <algorithm>
#include "funcs.h"
#include "utils.h"
#include "file.h"

Functions functions;

Functions::Functions() {}

Functions::~Functions() {}



bool Functions::Load(const char *fsetfile) {
  //std::ifstream src(fsetfile, std::ios::in);
  ReadTextFile src(fsetfile,true);
  if (!src.good())
    return false;
  char line[128];
  src.ReadLine(line, 128);
  int vmaj, vmin;
  int res = sscanf(line, "funcgalleryver %d %d\n", &vmaj, &vmin);
  bool success = false;
  if (2 != res) {
    // check if it's a ttset
    res = sscanf(line, "timeEdit %d %d\n", &vmaj, &vmin);
    if (2 != res){
	success =  _Read0103(fsetfile);
	return success;
    }
  }
  int ver = 100 * vmaj + vmin;
  switch (ver) {
  case 101:
    success = _Read0101(src);
    break;
  case 102: // corresponds to timeEdit
    success = _Read0102(src);
    break;
  default:
    return false;
  }
  return success;
}

bool Functions::LoadIndividualFunctions(
    const std::vector<std::string> functionsFileVector) {
  bool success = true;
  for (size_t i = 0; i < functionsFileVector.size(); ++i) {
    std::string fileName = functionsFileVector[i];
    ReadTextFile src(fileName.c_str(), true);
    if (!src.good())
      return false;
    Function New(src);
    if (New.isDefaultLoaded())
      success = false;
    push_back(New);
  }
  return success;
}

bool Functions::_Read0101(ReadTextFile &src) {
  char line[128];
  src.ReadLine(line, 128);
  int items;
  bool success = true;
  int res = sscanf(line, "items: %d", &items);
  if (res != 1) {
    Utils::Message("Error reading function set file\n");
    return false;
  }
  for (int i = 0; i < items; ++i) {
    Function New(src);
    if (New.isDefaultLoaded()){
      success = false;
      return false;
    }
    push_back(New);
  }
  return success;
}

bool Functions::_Read0102(ReadTextFile &src) {
  char line[128];
  src.ReadLine(line, 128);
  int items;
  bool success = true;
  int res = sscanf(line, "items: %d", &items);
  if (res != 1) {
    Utils::Message("Error reading function set file\n");
    return false;
  }
  for (int i = 0; i < items; ++i) {
    float lowLimit, upLimit;
    src.ReadLine(line, 128);
    res = sscanf(line, "start: %f", &lowLimit);
    if (res != 1) {
      Utils::Message("Error reading function set file, can't read start\n");
      return false;
    }
    src.ReadLine(line, 128);
    res = sscanf(line, "end: %f", &upLimit);
    if (res != 1) {
      Utils::Message("Error reading function set file, can't read end\n");
      return false;
    }
    // we ignore the following for now
    src.ReadLine(line, 128);
    if (0 != strncmp(line, "name: ", 6)) {
      Utils::Message("Error reading function set file, can't read name\n");
      return false;
    }
    src.ReadLine(line, 128);
    if (0 == strncmp(line, "color: ", 7)) {
      //ignore color
      src.ReadLine(line, 128);    
    }
    if (0 != strncmp(line, "startLabel: ", 12)) {
      Utils::Message(
          "Error reading function set file, can't read startLabel\n");
      return false;
    }
    src.ReadLine(line, 128);
    if (0 != strncmp(line, "endLabel: ", 10)) {
      Utils::Message("Error reading function set file, can't read endLabel\n");
      return false;
    }
    src.ReadLine(line, 128);
    if (0 != strncmp(line, "timelineFunceditFileStart:", 26)) {
      Utils::Message("Error reading function set file, can't read "
                     "timelineFunceditFileStart\n");
      return false;
    }
    Function New(src, lowLimit, upLimit);
    if (New.isDefaultLoaded())
      success = false;
    push_back(New);
    src.ReadLine(line, 128);

    if (0 != strncmp(line, "timelineFunceditFileEnd:", 24)) {
      Utils::Message("Error reading function set file, can't read "
                     "timelineFunceditFileEnd\n");
      return false;
    }
  }
  return success;
}


// Function so #defines in lpfg can be modified with recompiling a model
bool Functions::_Read0103(const char* fsetfile) {
  std::ifstream src(fsetfile, std::ios::in);
  src.seekg (0, std::ios::beg);
  std::string line;
  bool success = true;
  while (std::getline(src,line)){
    // 1- replace any tab by a space
    replace(line.begin(),line.end(),'\t',' ');
    // 2- tokenize in space
    // Vector of string to save tokens 
    std::vector <std::string> tokens;       
    // stringstream class check1 
    std::stringstream check1(line); 
    std::string intermediate; 
    // Tokenizing w.r.t. space ' ' 
    while(getline(check1, intermediate, ' ')) 
    {
      if (!intermediate.empty()){
	tokens.push_back(intermediate);
      }
    }
    if (tokens.size() == 0)
      continue;
    if (tokens[0].compare("#define") != 0) {
      Utils::Message("Warning reading function set file, can't read #define\n");
      return false;
    }
    std::string value_name = tokens[1];
    float value = std::stof(tokens[2]);
    
    Function New(value_name, value);
    if (New.isDefaultLoaded()){
      return false;
    }
    push_back(New);
  }
  return success;
}
