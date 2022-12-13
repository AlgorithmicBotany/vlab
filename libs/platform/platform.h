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




#ifndef __VLAB_PLATFORM_H__
#define __VLAB_PLATFORM_H__

#include <string>
#include <vector>
#include <QString>
#include "Report.h"

namespace Vlab {

    Report validate_environment( int argc, char ** argv );
    
    void display_report_cerr( const Report & r );
    
/*
// The following functions make sure that the environment is set up properly.
// If not, a warning will be displayed with the error messages.
//
// - a gui will be displayed - the parent will be used
bool validate_environment_gui( QWidget * parent );
// - a gui will be displayed - a new QApplication will be created
bool validate_environment_gui( const std::string & app_name );
// - the error messages will be sent to the ostream
bool validate_environment_cerr();
*/
    
    // returns the root directory of the vlab installation
    // - if VLABROOT environment is set, this is returned
    // - otherwise it is derived from the path of the executable used to invoke vlab
    const QString & getVlabRoot();
    // returns where the user keeps his personal config files
    // - the place is $(HOME)/.vlab
    // - if 'create' is true, then files in this directory are populated with defaults
    //   if they don't exist
    const QString & getUserConfigDir( bool create = true );
    // returns vlab tmp directory
    const QString & getTmpDir();
    // updates the binary directory of vlab
    typedef std::vector< std::string> UpdateBinLog;
    UpdateBinLog updateBin();
    // sets up vlab environment (mostly environment variables)
    void setupVlabEnvironment();

}; // namespace Vlab

#endif
