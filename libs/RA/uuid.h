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

// UNIX includes
#include <string>
#include <vector>
#include <list>

// QT includes
#include "quuid.h"

// fixes up the UUID table
// entries processed are object starting at path
// if recursive is true, the object and all extensions are processed
// to apply this algorithm to the entire oofs, pass '.' as the path
//
// returns a log of actions taken/errors
//
std::string uuidTableReconcile(
    const std::string & oofs,
    const std::string & path,
    bool recursive,
    bool tablePriority = true );

std::string uuidTableReconcile_string(
    const std::string & oofs,
    const std::string & path,
    bool recursive,
    bool tablePriority = true );



// reads UUID from a file given by fname
// reuturns NULL-UUID if file could not be opened or contained invalid UUID
QUuid readUUIDfromFile( const std::string & fname );

// looks up uuid in the table
std::string lookupUUID( const std::string & oofs, const QUuid & uuid );

// looks up uuid in the table
std::string lookupUUID_withoutQt( const std::string & oofs, const char* );

// removes UUID files for a subtree (recursively if set)
// Warning: the UUID table is not updated. If you want the update, use
// reconcile after this operation.
bool deleteUUIDfiles( const std::string & path, bool recursive );

// generates a UUID, using /dev/urandom and (s)random() techniques,
// with some sprinkle of current time (in microseconds), pid(), ppid(), and some
// other techniques, making the generated UUID random whether we call it before/after
// forking, etc... Basically QUuid::createUuid() is a little dumb because it uses a dumb
// unix random() function so we have to write our own.
QUuid QcreateUuid();

// UUIDtableEntry stores information about a single entry in the UUID table
struct UUIDtableEntry {
    QUuid uuid; // uuid of the object
    std::string path; // relative path to the object
    // these are internals used by the reconcile algorithm:
    bool validated; // whether the entry was validated (default is false)
    bool valid; // whether the entry is valid (only meaningful if validated is set)
    // constructors:
    UUIDtableEntry() {
	validated = false;
    }
  UUIDtableEntry( const QUuid & uuid, const std::string & path, bool validated, bool valid ) {
	this-> uuid = uuid;
	this-> path = path;
	this-> validated = validated;
	this-> valid = valid;
    }
  UUIDtableEntry( const QUuid & uuid, const std::string & path ) {
	this-> uuid = uuid;
	this-> path = path;
	this-> validated = false;
    }
    // if validated flag is not set yet, this method attempts to read
    // in and compare the UUID stored with in the object's directory with the UUID recorded
    // in the UUID table. If not the same, valid = false, otherwise valid = true. In either
    // case validaded is set to true (a caching mechanism).
  void validate( const std::string & oofs );
};

// reads in the contents of the UUID table file
typedef std::vector< UUIDtableEntry > UUIDtable;
UUIDtable readUUIDtable( const std::string & fname );

// QDir::entryList() seems to be broken in QT 4.6.2 + MacOSX 10.6.3, in that it seems
// to crash the child process if used after fork(). Until it's fixed, we'll need a
// workaround. QT bug was created: QTBUG-9762
// Update: apparently QT does not consider this a bug. They just don't support such
// functionality, whatever that means. So I guess this function is here to stay.
std::list<std::string> QDir_entryList_dirs_nosyms_nodot( const std::string & path );
