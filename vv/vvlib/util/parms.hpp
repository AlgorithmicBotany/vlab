#ifndef __PARMS_H__
#define __PARMS_H__

#include <string>
#include <map>
#include <sstream>
#include <iostream>
#include <vector>
#include <util/forall.hpp>

namespace util
{

  /**
   * \class Parms parms.hpp <util/parms.hpp>
   * \brief A utility class to parse L-Studio like parameter files
   *
   * <h2>Format of the parameter file</h2>
   *
   * The basic information in the parameter file is a key associated to 
   * a value, possibly with a C++-like comment:
   *
   * \code
   * key: value // Comment
   * \endcode
   *
   * Keys can be organized in sections:
   *
   * \code
   * [Section1]
   * key1: value1
   * key2: value2
   * ...
   *
   * [Section2]
   * key1: value3
   * key2: value4
   * ...
   * \endcode
   *
   * Empty line or comment-only line are ignored, any other line will raise an 
   * error. The default section is named with an empty string "".
   *
   * <h2> Usage example of util::Parm </h2>
   *
   * Here is an example:
   *
   * \code
   * int a,b;
   * std::string s1, s2;
   * std::vector<double> all_d;
   * util::Parms parms( "view.v", 2 );
   *
   * // First read simple parameters
   * parms( "", "a", a );
   * parms( "Main", "b", b, 0 );
   * parms( "Main", "string1", s1 );
   * parms( "Main", "string2", s2 );
   *
   * // Then read all the keys "double" in section "Values"
   * parms.all( "Values, "double", all_d);
   * \endcode
   *
   * <h2>Reading typed parameters</h2>
   *
   * To read user-defined typed as parameter, it is enough to overload the 
   * function
   * \code std::istream& operator>>( std::istream&, const T& ). \endcode
   * \p T being the type of the parameter to read.
   *
   * <h2>Key duplicates</h2>
   *
   * If the same key is used may times in the same section, either all the 
   * values can be retrieved using the Parms::all() method, or only the last 
   * can be retrieved using the normal Parms::operator()().
   *
   * <h2> \anchor ParmVerbosity Verbosity</h2>
   *
   * The class accepts 5 verbosity levels:
   * - 0: No output
   * - 1: Errors only
   * - 2: Errors and warning
   * - 3: User information
   * - 4: Debug information
   *
   * Debug information output the raw string before evaluation to set up 
   * a parameter.
   *
   */
  class Parms {
  public:
    /**
     * Constructor of the parameter file.
     */
    Parms(std::string parmFile, int verboseLevel = 1);
    ~Parms();

    /**
     * Change the verbosity level.
     *
     * \see \ref ParmVerbosity "Verbosity"
     */
    void verboseLevel( int vl ) { VerboseLevel = ( vl < 0 ) ? 0 : vl; }

    /**
     * This operator retrieve a single parameter.
     * \param section Section in which the parameter is looked for
     * \param key Key to look for
     * \param value Variable to set up if possible
     *
     * If the [section]key exists, \c value is set up to the parameter value. 
     * Any key placed before the first section is considered in the first 
     * secion. If the [section]key is multiply defined, only the last one is 
     * taken and a warning is issued (see \ref ParmVerbosity "Verbosity"). If 
     * the [section]key does not exist, or its content cannot be interpreted as 
     * the requested type, then ar error is issued.
     *
     * \returns True if there was no error while converting the different 
     * parameters from their string representation and the [section]key 
     * existes.
     */
    template <typename T>
      bool operator()( std::string section, std::string key, T& value );

    /**
     * Retrieve a single parameter.
     *
     * Boolean value must read "true" or "false", case being meaningless.
     *
     * \see operator()( std::string section, std::string key, T& value )
     */
    bool operator()( std::string section, std::string key, bool& value );
    /**
     * Retrieve a single parameter.
     *
     * \see operator()( std::string section, std::string key, T& value )
     */
    bool operator()( std::string section, std::string key, int& value );
    /**
     * Retrieve a single parameter.
     *
     * \see operator()( std::string section, std::string key, T& value )
     */
    bool operator()( std::string section, std::string key, float& value );
    /**
     * Retrieve a single parameter.
     *
     * \see operator()( std::string section, std::string key, T& value )
     */
    bool operator()( std::string section, std::string key, double& value );
    /**
     * Retrieve a single parameter.
     *
     * For string, the value is the whole line with whitespaces and comment 
     * stripped before and after the parameter.
     *
     * \see operator()( std::string section, std::string key, T& value )
     */
    bool operator()( std::string section, std::string key, std::string& value );

    /**
     * Variation on the previous, but if the [section]key is not found, an 
     * information message is issued (instead of an error) and \c value is set 
     * up to \c def.
     */
    template <typename T>
      bool operator()( std::string section, std::string key, T& value, const T& def );

    /**
     * This operator retrieve a all parameters with same [section]key.
     * \param section Section in which the parameter is looked for
     * \param key Key to look for
     * \param value Variable to set up
     *
     * \c value is filled with the different values found having same 
     * [section]key. If none, \c value is simply empty. The only error that can 
     * arise is a reading error, if one parameter has invalid value. This 
     * parameter will simply be ignored, all other parameters being read.
     *
     * \returns True if there was no error while converting the different 
     * parameters from their string representation.
     */
    template <typename T>
      bool all( std::string section, std::string key, std::vector<T>& value );

    /**
     * Retrieve a all parameters with same [section]key.
     *
     * \see all( std::string section, std::string key, std::vector<T>& value )
     */
    bool all( std::string section, std::string key, std::vector<bool>& value );
    /**
     * Retrieve a all parameters with same [section]key.
     *
     * \see all( std::string section, std::string key, std::vector<T>& value )
     */
    bool all( std::string section, std::string key, std::vector<int>& value );
    /**
     * Retrieve a all parameters with same [section]key.
     *
     * \see all( std::string section, std::string key, std::vector<T>& value )
     */
    bool all( std::string section, std::string key, std::vector<float>& value );
    /**
     * Retrieve a all parameters with same [section]key.
     *
     * \see all( std::string section, std::string key, std::vector<T>& value )
     */
    bool all( std::string section, std::string key, std::vector<double>& value );
    /**
     * Retrieve a all parameters with same [section]key.
     *
     * \see all( std::string section, std::string key, std::vector<T>& value )
     */
    bool all( std::string section, std::string key, std::vector<std::string>& value );


  private:
    /**
     * Strip white spaces at the beginning and the end of the string.
     */
    void removeWhitespace(std::string &s);

    /**
     * Extract a value as a string given \c section and \c key.
     *
     * \returns True if [section]key exist.
     */
    bool extractValues( std::string section, std::string key, std::vector<std::string>& values );

    /**
     * Read a value from a string.
     *
     * If the value to read is a boolean, the input string has to be equal to 
     * "true" or "false" (case insensitive).
     *
     * \returns True if the reading succeeded
     */
    bool readValue( std::string value, bool& variable );
    /**
     * Read a value from a string.
     *
     * If the value to read is a string, the input one is simply copied.
     *
     * \returns True if the reading succeeded
     */
    bool readValue( std::string value, std::string& variable );
    /**
     * Read a value from a string.
     *
     * Default is to use operator>> on a \c istringstream defined on the input 
     * string. Print an error is the flux operator fails.
     *
     * \returns True if the reading succeeded
     */
    template <typename T>
      bool readValue( std::string value, T& variable );

    /**
     * Name of the parameter file to read
     */
    std::string ParmFileName;

    /**
     * Check the existence of \c key.
     *
     * \param key [Section]key written as "section:key"
     */
    bool check(std::string &key);

    /**
     * Map of all the parameters in string form as read from the file. The 
     * strings are already stripped from whitespaces and comments.
     */
    std::map<std::string,std::vector<std::string> > Parameters;

    /**
     * Current section while reading the parameter file
     */
    std::string Section;

    /**
     * Current verbosity level
     */
    int VerboseLevel;

    /**
     * Set to True if the existence of a key output an error. Used with the 
     * default value to avoid printing an error in that case.
     */
    bool CheckExist;
  };

  template <typename T>
    bool Parms::readValue( std::string value, T& variable )
      {
      std::istringstream iss(value);
      std::boolalpha( iss );
      return bool( iss >> variable );
      }

  template <typename T>
    bool Parms::operator()( std::string section, std::string key, T& value )
      {
      std::vector<std::string> values;
      if( !extractValues( section, key, values ) )
        return false;

      if( ( values.size() > 1 ) && ( VerboseLevel > 1 ) )
        {
        std::cerr << "Parms::operator():Warning multiple value for key [" << section << "]"
          << key << ", last one used." << std::endl;
        }

      if( !readValue( values.back(), value ) )
        {
        if( VerboseLevel > 0 )
          {
          std::cerr << "Parms::operator():Error getting key [" << section << "]" <<  key
            << " value " << values.back() << std::endl;
          }
        return false;
        }
      return true;
      }

  template <typename T>
    bool Parms::operator()( std::string section, std::string key, T& value, const T& def )
      {
      CheckExist = false;
      if( !( *this )( section, key, value ) )
        {
        if( VerboseLevel > 2 )
          {
          std::cerr << "Parms::operator()::Info key [" << section << "]"
            << key << " not found, using default value" << std::endl;
          }
        value = def;
        }
      CheckExist = true;
      return true;
      }

  template <typename T>
    bool Parms::all( std::string section, std::string key, std::vector<T>& value )
      {
      bool valid = true;
      std::vector<std::string> values;
      if( !extractValues( section, key, values ) )
        return false;
      value.clear();
      forall( std::string val, values )
        {
        T single_value;
        if( readValue( val, single_value ) )
          {
          value.push_back( single_value );
          }
        else
          {
          if( VerboseLevel > 0 )
            {
            std::cerr << "Parms::all:Error reading key [" << section << "]" << key
              << " with value " << val << std::endl;
            }
          valid = false;
          }
        }
      return valid;
      }

  std::string absoluteDir( std::string filename );
}

#endif
