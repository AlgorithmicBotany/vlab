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
 public:
  static void readConfig();

  inline static const float* getGridColour()             {return gridColour;}
  inline static const float* getXAxisColour()            {return xAxisColour;}
  inline static const float* getYAxisColour()            {return yAxisColour;}
  inline static const float* getZAxisColour()            {return zAxisColour;}
  inline static const float* getUpVectorColour()         {return upVectorColour;}
  inline static const float* getHeadingColour()          {return headingColour;}
  inline static const float* getConnectionPointColour()  {return connectionPointColour;}
  inline static const float* getEndPointColour()         {return endPointColour;}
  inline static const float* getControlPointsColour()    {return controlPointsColour;}
  inline static const float* getWireColour()             {return wireColour;}
  inline static const float* getSolidColour()            {return solidColour;}
  inline static const float* getSelectedColour()         {return selectedColour;}
  inline static const float* getStickyColour()           {return stickyColour;}
  inline static const float* getSplitColour()            {return splitColour;}
  inline static const float* getSelectedForMergeColour() {return selectedForMergeColour;}
  inline static const float* getControlPolyColour()      {return controlPolyColour;}
  inline static const float* getControlPolyInColour()    {return controlPolyInColour;}
  inline static const float* getControlPolyHLColour()    {return controlPolyHLColour;}
  inline static const float* getWireHLColour()           {return wireHLColour;}
  inline static const float* getSolidHLColour()          {return solidHLColour;}
  inline static const float* getAmbientColour()          {return ambientColour;}
  inline static float        getPointSize()              {return pointSize;}
  inline static float        getSelectionSize()          {return selectionSize;}
  inline static int          getUDivs()                  {return uDivs;}
  inline static int          getVDivs()                  {return vDivs;}

  class ConfigErrorExc {
  public:
    ConfigErrorExc();
    ConfigErrorExc(std::string msg) : _msg(msg) {}
    const std::string& message() {return _msg;}
  private:
    std::string _msg;
  };

 protected:
  // colours
  static float gridColour[4];
  static float xAxisColour[4];
  static float yAxisColour[4];
  static float zAxisColour[4];
  static float upVectorColour[4];
  static float headingColour[4];
  static float connectionPointColour[4];
  static float endPointColour[4];
  static float controlPointsColour[4];
  static float wireColour[4];
  static float solidColour[4];
  static float selectedColour[4];
  static float stickyColour[4];
  static float splitColour[4];
  static float selectedForMergeColour[4];
  static float controlPolyColour[4];
  static float controlPolyInColour[4];
  static float controlPolyHLColour[4];
  static float wireHLColour[4];
  static float solidHLColour[4];
  static float ambientColour[4];
 
  // sizes
  static float pointSize;
  static float selectionSize;
  static int   uDivs;
  static int   vDivs;
};

#endif
