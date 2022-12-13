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




#ifndef __REMOTE_FILE_ACCESS_H__
#define __REMOTE_FILE_ACCESS_H__

// UNIX includes
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <list>

// QT includes
#include "quuid.h"

// VLAB includes
#include "RAconnection.h"
#include "RAfile.h"
#include "ProgressReporter.h"

#include "RAconsts.h"

// REMOTE ACCESS class

class RA
{
	
public:
	
	// -- methods --
	
	static RA_File *    Fopen ( RA_Connection * connection,
				    const char * fname,
				    const char * type);
	
	static FILE *
	Fopen_read ( RA_Connection * connection,
		     const char * fname);
	
	static RA_Connection *
	new_connection ( const char * host_name,
			 const char * login_name,
			 const char * password);
	
	static void
	close_connection ( RA_Connection * connection);
	
	static int  Compare_files ( RA_Connection * connection1,
				    const char * fname1,
				    RA_Connection * connection2,
				    const char * fname2);
	
	static int      Copy_file ( RA_Connection * src_connection,
				    const char * src_fname,
				    RA_Connection * dst_connection,
				    const char * dst_fname);
	
	static int         Unlink ( RA_Connection * connection,
				    const char * fname);
	
	static int        Deltree ( RA_Connection * connection,
				    const char * dirname);
	
	static int        Symlink ( RA_Connection * connection,
				    const char * src_fname,
				    const char * dst_fname);
	
	static int         Rename ( RA_Connection * connection,
				    const char * src_fname,
				    const char * dst_fname);
	
	static int           Stat ( RA_Connection * connection,
				    const char * fname,
				    RA_Stat_Struc * stat_struc );
	
	static int        Get_dir ( RA_Connection * connection,
				    const char * dir_name,
				    char *** list);
	
	static int       Readlink ( RA_Connection * connection,
				    const char * fname,
				    char * (& result));
	
	static int        Is_link ( RA_Connection * connection,
				    const char * fname);
	
	static int     Write_file ( RA_Connection * connection,
				    const char * fname,
				    const char * buffer,
				    const long size);
	
	static int      Read_file ( RA_Connection * connection,
				    const char * fname,
				    char * ( & buffer),
				    long & size);
	
	static RA_File_Type Get_file_type ( RA_Connection * connection,
				    const char * fname,
				    RA_File_Type & type);
	
	static int         Fclose ( RA_File * fp);
	
	static void         Error ( const char * err_msg);
	
	static int     Fetch_file ( RA_Connection * connection,
				    const char * remote_fname,
				    const char * local_fname);
	
	static int     Fetch_file ( RA_Connection * connection,
				    const char * remote_fname,
				    FILE * fp);
	
	static int       Put_file ( const char * local_fname,
				    RA_Connection * connection,
				    const char * remote_fname);
	
	static int          Mkdir ( RA_Connection * connection,
				    const char * path,
				    mode_t mode=0);
	
	static int          Rmdir ( RA_Connection * connection,
				    const char * path);
	
	static int         Access ( RA_Connection * connection,
				    const char * fname,
				    const int amode);
	
	static int       Realpath ( RA_Connection * connection,
				    const char * path,
				    char * (& result));
	
	static int          Fgetc ( RA_File * fp);
	
	static int           Feof ( RA_File * fp);
	
	static const char *  err_to_str ( void);
	static const char *  err_to_str ( int err_code);
	
	// vlab extensions
	
	static int Get_extensions ( RA_Connection * connection,
				    const char * path,
				    char ** (& list));
	
	static int  Rename_object ( RA_Connection * connection,
				    const char * oofs_dir,
				    const char * src_fname,
				    const char * new_name);
	
	static int  Delete_object ( RA_Connection * connection,
				    const char * oofs_dir,
				    const char * fname);
	
	static int Archive_object ( RA_Connection * connection,
				    const char * oofs_dir,
				    const char * object_name,
				    const char * local_file,
				    int recursive
				    );
	
	static int Dearchive_object (
		RA_Connection * connection,
		const char * local_file,
		const char * oofs_dir,
		const char * destination);
	
	static int Paste_object (
		RA_Connection * connection,
		const char * oofs_dir,
		const char * destination,
		const char * local_file,
		const char * old_path,
		char move_links
		);
	
	static int Prototype_object (
		RA_Connection * connection,
		const char * object_path);

	// returns the UUID of the object, and if it does not exist,
	// it will be created and inserted into the UUID lookup table
	static QUuid getUUID (
	    RA_Connection * connection,
	    const std::string oofs_dir,
	    const std::string path,
	    bool create = true );

	// lookup UUID via UUID table, if not found '*' is returned (invalid filename)
	static std::string lookupUUID(
	    RA_Connection * connection,
	    const std::string & oofs,
	    const QUuid & uuid );

	// fixes up UUIDs and the table in the given oofs, for all objects with a
	// matching prefix (path). If recursive is not set, the operation will only
	// affect a single object (path)
	static std::string uuidTableReconcile(
	    RA_Connection * connection,
	    const std::string & oofs,
	    const std::string & path,
	    bool recursive,
	    bool tablePriority );

	// fixes ups oofs...
	// - removes duplicate UUIDS
	// - removes unused UUIDS
	// - renumbers all UUIDS (if renumber = true)
	// - reports broken links
	// - makes sure the UUID lookup table is in sync with the rest of the OOFS
	// Returns: the log
	static std::string fixOofs(
	    RA_Connection * connection,
	    const std::string & oofs,
	    bool renumber );
	
	// searching (starting, continuing and ending)
	static void searchBegin(
	    RA_Connection * connection,
	    const std::string & oofs,
	    const std::string & start_path,
	    const std::string & pattern,
	    bool caseSensitive,
	    bool exactMatch );
	// returns path to the next object that matches the pattern
	// returns "*" if search ended
	// returns "**" if no match found so far, but there may be more (only
	//    applicable if 'blocking=false')
	static std::string searchContinue(
	    RA_Connection * connection,
	    bool blocking = true );
	static void searchEnd(
	    RA_Connection * connection );
	
	// -- variables
	static const char *       tmp_dir;
	static RA_Error           error_code;
public:
	static void appendErrorLog (const std::string & entry);
	static std::string getErrorLog ();
	static void setProgressReporter (ProgressReporter * rep);

private:
	static std::string        errorLog_;
	static void setProgress (double val);
	static ProgressReporter * progressReporter_;
};

#endif
