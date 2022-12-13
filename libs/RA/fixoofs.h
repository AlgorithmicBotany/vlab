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




#pragma once

#include <string>
#include <vector>

// will 'fix' oofs database
// - it will renumber all objects by giving them new UUIDs and adjust links
// - it will remove all unused (unreferenced) UUIDs
// - there will be no duplicate UUIDs (conflict resolution in case of duplicate UUIDs will be
//   affected by the UUID lookup table, i.e. if one object is in the table and the other(s) are not,
//   the object that is in the table will get to keep the UUID, othe other will lose it)
// - all UUIDs will be in the UUID table
// - it will report actions taken
// - it will report invalid hyperlinks

std::string fixOofs( const std::string & oofs_path, bool renumber );

std::string fixOofs_string( const std::string & oofs_path, bool renumber );
	 
