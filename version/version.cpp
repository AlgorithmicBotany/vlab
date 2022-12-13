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




#include <iostream>
#include <string> 
#include "version.h"

using namespace std;

/* Used by:
   - build scripts (for version string, e.g. 4.2.9)
   - lstudio-splash (for version string, build #, build date)
   - nothing else?
*/

/* Command line arguments:
   No arguments - version number alone (e.g. 4.2.9)
   With arguments - outputs requested values, separated by a single space:
     -v argument - version number alone (e.g. 4.2.9)
     -b argument - build number alone (e.g. 229)
     -d argument - build date along (e.g. July 10, 2006)
     -a argument - build for ATE version
   So version -v -b -d outputs:
     4.2.9 229 July 10, 2006
*/

int main(int argc, char ** argv)
{
  bool started = false;
  if(argc == 1) // No arguments - version number alone
    cout << vlab::version_string();
  else
    for(int i = 1 ; i < argc ; i++)
      if (string(argv[i]) == string("-v"))
      {
	if(started) cout << " "; else started = true;
	cout << vlab::version_string();
      }
      else if(string(argv[i]) == string("-b"))
      {
	if(started) cout << " "; else started = true;
	cout << vlab::build_number();
      }
      else if(string(argv[i]) == string("-d"))
      {
	if(started) cout << " "; else started = true;
	cout << vlab::build_date_string();
      }
	else if(string(argv[i]) == string("-a"))
      {
	if(started) cout << " "; else started = true;
	cout << " ATE ";
      }

  cout << "\n";
  return 0;
}
