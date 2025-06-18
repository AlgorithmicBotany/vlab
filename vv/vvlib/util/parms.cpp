// Parameter reader class
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <cctype>
#include <algorithm>
#include <iterator>
#include "parms.hpp"
#include <qdir.h>

namespace util
{
  using std::cout;
  using std::cerr;
  using std::endl;
  using std::ifstream;
  using std::istringstream;
  using std::string;
  using std::map;
  using std::ostream_iterator;
  using std::copy;

  Parms::Parms(std::string parmFile, int vl )
    : ParmFileName( parmFile )
    , CheckExist( false )
      {
      verboseLevel( vl );
      ifstream pIn(ParmFileName.c_str());
      if(!pIn) {
        if( VerboseLevel > 0 )
          cerr << "Parms::Parms:Error opening " << ParmFileName << endl;
        exit(1);
      }
      unsigned int line = 0;
      size_t pos;
      string buff("");
      while(pIn) {
        line++;
        // read in line
        getline(pIn, buff);
        // find C++ style comments
        pos = buff.find("//");
        // and remove to end of line
        if(pos != string::npos)
          buff = buff.substr(0, pos);
        // remove leading and trailing whitespace
        removeWhitespace(buff);
        // skip line if blank
        if(buff.length() == 0)
          continue;
        // Look for section
        if(buff[0] == '[' && buff[buff.length() - 1] == ']') {
          Section = buff.substr(1, buff.length() - 2);
          if(( Section.length() == 0) && ( VerboseLevel > 0 ) )
            cerr << "Parms::Parms:Error on line " << line << ", []" << endl;
          continue;
        }
        // split key and value
        pos = buff.find(":");
        // error if no : delimiter
        if(pos == string::npos) {
          if( VerboseLevel > 0 )
            cerr << "Parms::Parms:Error on line " << line << ", missing :" << endl;
          continue;
        }
        // get key and value and remove leading/trailing blanks
        string key = buff.substr(0, pos);
        string value = buff.substr(pos + 1, string::npos);
        removeWhitespace(key);
        removeWhitespace(value);
        // error if no key
        if(key.length() == 0) {
          if( VerboseLevel > 0 )
            cerr << "Parms::Parms:Error on line " << line << ", missing key" << endl;
          continue;
        }
        // error if no value
        if(value.length() == 0) {
          if( VerboseLevel > 0 )
            cerr << "Parms::Parms:Error on line "<< line << ", missing value" << endl;
          continue;
        }

        // Now we have key and value, add to map
        Parameters[Section + ":" + key].push_back( value );
      }
      };

  Parms::~Parms()
    {
    }

  void Parms::removeWhitespace(std::string &s)
    {
    size_t pos = s.find_first_not_of(" \t\r");
    if(pos == string::npos)
      s = "";
    else {
      s = s.substr(pos, string::npos);
      pos = s.find_last_not_of(" \t\r");
      if(pos != string::npos)
        s = s.substr(0, pos + 1);
    }
    }

  // Check if parm exists
  bool Parms::check(std::string &key)
    {
    return ( Parameters.find( key ) != Parameters.end() );
    }

  bool Parms::operator()( std::string section, std::string key, bool& value )
    {
    return operator()<bool>( section, key, value );
    }

  bool Parms::operator()( std::string section, std::string key, int& value )
    {
    return operator()<int>( section, key, value );
    }

  bool Parms::operator()( std::string section, std::string key, float& value )
    {
    return operator()<float>( section, key, value );
    }

  bool Parms::operator()( std::string section, std::string key, double& value )
    {
    return operator()<double>( section, key, value );
    }

  bool Parms::operator()( std::string section, std::string key, std::string& value )
    {
    return operator()<std::string>( section, key, value );
    }

  bool Parms::all( std::string section, std::string key, std::vector<bool>& value )
    {
    return all<bool>( section, key, value );
    }

  bool Parms::all( std::string section, std::string key, std::vector<int>& value )
    {
    return all<int>( section, key, value );
    }

  bool Parms::all( std::string section, std::string key, std::vector<float>& value )
    {
    return all<float>( section, key, value );
    }

  bool Parms::all( std::string section, std::string key, std::vector<double>& value )
    {
    return all<double>( section, key, value );
    }

  bool Parms::all( std::string section, std::string key, std::vector<std::string>& value )
    {
    return all<std::string>( section, key, value );
    }

  bool Parms::readValue( std::string raw_value, bool& variable )
    {
    std::string value;
    forall( char c, raw_value )
      {
      value.push_back( tolower( c ) );
      }
    if( value == "true" )
      {
      variable = true;
      return true;
      }
    else if( value == "false" )
      {
      variable = false;
      return true;
      }
    return false;
    }

  bool Parms::readValue( std::string value, std::string& variable )
    {
    variable = value;
    return true;
    }

  bool Parms::extractValues( std::string section, std::string key, std::vector<std::string>& values )
    {
    key = section + ":" + key;

    if(check(key))
      {
      values = Parameters[key];

      if( VerboseLevel > 3 )
        {
        cerr << "Parms::extractValues:Debug strings for key [" << section << "]" << key << ": -";
        copy( values.begin(), values.end(), ostream_iterator<std::string>( cerr, "-" ) );
        cerr << endl;
        }
      return true;
      }

    if( CheckExist && ( VerboseLevel > 0 ) )
      std::cerr << "Parms::operator():Error key not found [" << section << "]"
        << key << std::endl;
    return false;
    }

}
