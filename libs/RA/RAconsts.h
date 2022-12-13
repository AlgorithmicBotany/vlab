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




#ifndef __RACONSTANTS_H__
#define __RACONSTANTS_H__


// listening port of raserver
const unsigned short int RA_PORT = 12847;


enum RA_Error
{
	RA_FOPEN_FAILED,
	RA_FETCH_FAILED,
	RA_OPERATION_NOT_SUPPORTED,
	RA_MKTEMP_FAILED,
	RA_FCLOSE_FAILED,
	RA_UNLINK_FAILED,
	RA_INVALID_HOST_NAME,
	RA_INVALID_LOGIN,
	RA_SOCKET_ERROR,
	RA_LOCAL_STAT_FAILED,
	RA_SERVER_ERROR,
	RA_DELTREE_FAILED,
	RA_OPEN_FAILED,
	RA_WRITE_FAILED,
	RA_READ_FAILED,
	RA_REALPATH_FAILED,
	RA_READLINK_FAILED,
	RA_RENAME_FAILED,
	RA_DELETE_FAILED,
	RA_DBASE_INIT_FAILED,
	RA_ARCHIVE_FAILED,
	RA_DEARCHIVE_FAILED,
	RA_TYPE_MISMATCH,
	RA_PROTOTYPE_OBJECT_FAILED,
	RA_PASTE_FAILED
};

enum RA_File_Type
{
	RA_REG_TYPE,
	RA_DIR_TYPE,
	RA_OTHER_TYPE,
	RA_NOEXIST_TYPE
};

struct RA_Stat_Struc 
{
	RA_File_Type type;
	unsigned char readable;
	unsigned char writeable;
	unsigned char executable;
	unsigned char is_link;
};

#define raq(NAME) RA_ ## NAME ## _REQUEST, RA_ ## NAME ## _RESPONSE
enum RA_Message_Type
{
    // pre vlab 4.3 protocol, if you change the format of these messages, or even their
    // order, pre vlab 4.3 clients won't be able to talk to this raserver !!!
    raq( LOGIN ),            raq( FETCH_FILE ),     raq( LOGOUT ),
    raq( GETDIR ),           raq( UNLINK ),         raq( STAT ),
    raq( SYMLINK ),          raq( PUTFILE ),        raq( COPYFILE ),
    raq( COMPFILE ),         raq( RMDIR ),          raq( MKDIR ),
    raq( REALPATH ),         raq( RENAME ),         raq( DELTREE ),
    raq( READLINK ),         raq( GET_EXTENSIONS),  raq( RENAME_OBJECT ),
    raq( DELETE_OBJECT ),    raq( ARCHIVE_OBJECT ), raq( DEARCHIVE_OBJECT ),
    raq( PROTOTYPE_OBJECT ), RA_ARCHIVE_BLOCK,      raq( PASTE_OBJECT ),
    // vlab 4.3 extended protocol
    raq( VERSION ),
    raq( GET_UUID ),         raq( LOOKUP_UUID ),    raq( RECONCILE_UUIDS ),
    raq( SEARCH_BEGIN ),     raq( SEARCH_CONTINUE ),raq( SEARCH_END ),
    raq( FIX_OOFS )
};
#undef raq



#endif 

