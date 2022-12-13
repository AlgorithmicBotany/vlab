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




#ifndef __VLAB_REPORT_H__
#define __VLAB_REPORT_H__

#include <string>
#include <vector>

namespace Vlab {

class Report {
public:
    bool empty() const {
        return errors.empty() && warnings.empty() && log.empty();
    }
    bool fatal_errors() const {
        return ! errors.empty();
    }
    const std::vector<std::string> & get_errors() const { return errors; }
    const std::vector<std::string> & get_warnings() const { return warnings; }
    const std::string & get_log() const { return log; }
    void add_error( const std::string & s ) {
        errors.push_back( s );
        log += "Error: " + s + "\n";
    }
    void add_warning( const std::string & s ) {
        warnings.push_back( s );
        log += "Warning: " + s + "\n";
    }
    void add_to_log( const std::string & s ) {
        log += s;
    }
private:
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
    std::string log;
}; // class Report

}; // namespace Vlab

#endif
