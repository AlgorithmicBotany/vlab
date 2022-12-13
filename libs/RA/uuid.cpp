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



#include <errno.h>
#include <iostream>
#include <signal.h>
#include <sstream>
#include <stack>
#include <stdlib.h>
#include <sys/dir.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <vector>
//#include <QTemporaryFile>
//#include <QTextStream>
//#include <QFile>

#include <fstream>
#include <sstream>

#include "uuid.h"

// there are idiotic differences between BSD and Linux, and I guess the reason
// is stupid too :(
#ifdef VLAB_MACX
typedef const dirent *mydirent;
#else
typedef const dirent *mydirent;
#endif

// QDir::entryList() seems to be broken in QT 4.6.2 + MacOSX 10.6.3, in that it
// seems to crash the child process if used after fork(). Until it's fixed,
// we'll need a workaround. QT bug was created: QTBUG-9762
std::list<std::string>
QDir_entryList_dirs_nosyms_nodot(const std::string &path) {
  struct local {
    static int sel(mydirent e) {
      if (e->d_type != DT_DIR)
        return 0;
      std::string path = e->d_name;
      if (path == "." || path == "..")
        return 0;
      return 1;
    }
  };
  std::list<std::string> qlist;
  dirent **clist = 0;
  int n = scandir(path.c_str(), &clist, local::sel, 0);
  for (int i = 0; i < n; i++) {
    qlist.push_back(clist[i]->d_name);
    free(clist[i]);
  }
  free(clist);
  return qlist;
}

// file locking class, based on unix file rename (first process to successfuly
// rename a randome file to a predetermined file will win the race)
class Lock {
  // private constructor to force using the static lock() method
  Lock() {}
  Lock(const std::string &lockfname) { _lockfname = lockfname; }
  std::string _lockfname;

public:
  bool valid() { return !_lockfname.empty(); }
  // remove the lock
  void release() {
    if (!_lockfname.empty())
      if (remove(_lockfname.c_str()) != 0) {
        std::cerr << "Failed to remove lock: " << _lockfname << "!!!\n";
      }
    _lockfname = std::string();
  }
  ~Lock() { release(); }
  static Lock lock(const std::string &fname, int ntries = 15,
                   int msdelay = 1000000) {
    std::string lockFname = fname + "-LOCK";
    //	QTemporaryFile tmpfile( lockFname + "-XXXXXX" );
    std::ofstream tmpfile;
    std::string tmplockname = lockFname + "-XXXXXX";
    tmpfile.open(tmplockname.c_str());

    if (tmpfile.fail()) {
      std::cerr << "uuid: cannot create pre-lock on .uuids:\n"
                << "      " << tmplockname << "\n";
      return Lock();
    }
    // write our PID to the pre-lock, in case this process fails to remove the
    // lock
    { tmpfile << getpid() << "\n"; }
    // try to create the lock by renaming the temporary file
    bool success = false;
    for (int i = 0; i < ntries; i++) {
      // if someone is holding the lock, let's see if the process is still
      // active and if it's not, delete the lock
      std::ifstream f;

      f.open(lockFname.c_str());
      if (!f.fail()) {
        pid_t pid;
        f >> pid;
        std::cerr << "uuid: process " << pid << " is holding the lock\n";
        if (0 == kill(pid, 0) || errno == EPERM) {
          std::cerr << "uuid: and it's still running.\n";
        } else {
          std::cerr
              << "uuid: and the process is not running. Releasing lock.\n";
          remove(lockFname.c_str());
        }
      }
      // now try to move the pre-lock into place
      if (!rename(tmplockname.c_str(), lockFname.c_str())) {
        success = true;
        break;
      }
      // we failed, try again in a bit
      std::cerr << "uuid: waiting on lock ( try # " << i << "/" << ntries
                << ")\n";
      usleep(msdelay);
    }
    if (!success) {
      std::cerr << "uuid: Giving up establishing lock on " << fname << "\n";
      return Lock();
    }
    // at this point we don't want to remove the lock when we exit,
    // that's what Lock::release() will do
    // tmpfile.setAutoRemove( false );

    // return a Lock structure
    // Bug in Qt 4.5.1 - after rename, Qtemporaryfile does not update it's
    // filename so instead of :
    //     return Lock( tmpfile.fileName());
    // we have to do: :(
    return Lock(lockFname);
  }
};

// reads UUID from a file given by fname
// reuturns NULL-UUID if file could not be opened or contained invalid UUID
QUuid readUUIDfromFile(const std::string &fname) {
  std::ifstream uuidFile;
  uuidFile.open(fname.c_str());

  if (uuidFile.fail())
    return QUuid();
  /*
  char str[512];
  std::string line;
  uuidFile.getline(str,512);
  line = std::string(str);
  */
  std::string line;
  // std::getline(uuidFile,line);

  uuidFile >> line;
  QUuid uuid = QUuid(line);
  return uuid;
}

// if validated flag is not set yet, this method attempts to read in
// and compare the UUID stored with in the object's directory with the
// UUID recorded in the UUID table. If not the same, valid = false,
// otherwise valid = true. In either case validaded is set to true (a
// caching mechanism).
void UUIDtableEntry::validate(const std::string &oofs) {
  if (validated)
    return;
  validated = true;

  if (uuid != readUUIDfromFile(oofs + "/" + path + "/.uuid"))
    valid = false;
  else
    valid = true;
};

// debugging output - prints the contents of the uuid table
/*
static void printList(const UUIDtable &list) {
  std::cerr << "    list:\n";
  for (size_t i = 0; i < list.size(); i++) {
    std::cerr << "     ";
    if (list[i].validated)
      std::cerr << (list[i].valid ? "Y" : "N");
    else
      std::cerr << "?";
    std::cerr << " " << list[i].uuid.toString() << " " << list[i].path << "\n";
  }
}
*/
//
// find UUID in the list and return it's position
// - only unvalidated, or valid entries are considered, invalid entries are
// skipped
//
// returns: -1 if not found
//
static int findUUID(const UUIDtable &list, const QUuid &uuid) {
  for (size_t i = 0; i < list.size(); i++) {
    if (list[i].validated && !list[i].valid)
      continue;
    if (list[i].uuid == uuid)
      return (int)i;
  }
  return -1;
}

// reads in the contents of the UUID table file
UUIDtable readUUIDtable(const std::string &fname) {
  UUIDtable list;
  std::ifstream tableFile(fname.c_str(), std::ifstream::in);
  if (tableFile.good()) {
    while (1) {
      // get the next line
      // no need to do that dynamically
      // we need to get a string only
      /*
        char str[1024];
        std::string line;
        tableFile.getline(str,1024);
        line = std::string(str);
      */
      std::string line;
      std::getline(tableFile, line);
      // break out if we are at EOF
      if (line.empty())
        break;
      // get the positions of the curly bracers, and first and last
      // slashes
      int p1 = line.find_first_of('{');
      int p2 = line.find_first_of('}', p1 + 1);
      int p3 = line.find_first_of('/', p2 + 1);
      int p4 = line.find_last_of('/');
      // skip lines with syntax errors
      if (p1 == -1 || p1 >= p2 || p2 >= p3 || p3 >= p4)
        continue;
      // construct an entry and add it to the list
      UUIDtableEntry e(QUuid(line.substr(p1, p2 - p1 + 1)),
                       line.substr(p3 + 1, p4 - p3 - 1));
      if (e.uuid.isNull())
        continue; // syntax error...
      // add the entry
      list.push_back(e);
    }

    tableFile.close();
  }
  return list;
}

// Fixes up the UUID table and occasionaly UUIDs of objects.
// However, not all objects/entries are fixed. The algorithm fixes the minimum
// number of objects/entries so that at the end all objects matching the <path>
// prefix have validated UUIDs and corresponding entries in the UUID table.
// Occasionally this means that other entries have to be fixed (in case of
// conflicts).
//
// Parameters:
//   - oofs: the directory of the oofs (oofs/.uuids contains the UUID table)
//   - path: which section of the oofs to check. If empty, entire oofs is
//   checked.
//   - recursive: if set, the entire subtree is checked, otherwise only the
//   single
//       object pointed to by 'path' is checked. This is more or less an
//       optimization parameter. If you know  you are only changing a single
//       object in the tree there is no need to do things recursively.
//   - prioritizeExisting: this flag determines how to resolve conflicts, see
//   below
//
// Conflicts are resolved if the algorithm finds an object that has a UUID, but
// the UUID already exists in the table, and the entry in the table is valid
// (i.e. it points to an object that has identical UUID). In such a case, there
// are two options to resolve the conflict. Let's call object1 the object that
// we found in the file system, i.e. it has path <path>/..../object1. Let's call
// object2 the object to which the entry in the table points to. It can have
// arbitrary path.
//
// - option 1: prioritizeExisting = true
//     We delete UUID from object1 and keep everything the same (i.e. keep the
//     entry and the UUID of the object2). This should be the default operation,
//     as it is the correct action to take in most circumstances. In other
//     words, this option puts priority on UUIDs of objects that are already in
//     the table.
// - option 2: prioritizeExisting = false
//     We delete UUID from object2 and fix the entry in the table to point to
//     object1. Example of where you want to use this type of conflict
//     resolution is when you are pasting an object or subtree with 'move links'
//     enabled. In other words, this option puts priority on UUIDs that are in
//     the tree starting at <path>.
//
// A quick note - there could be yet another conflict which option 2 does not
// probably handle properly in all cases. In particular, if there are more than
// 1 objects in the tree starting with <path> that share the same UUID, the
// object that ends up with an entry in the UUID table is kind of arbitrary...
// But I am not sure what would be the proper way of dealing with this.

std::string uuidTableReconcile_string(const std::string &oofs,
                                      const std::string &path, bool recursive,
                                      bool tablePriority) {

  return uuidTableReconcile(oofs, path, recursive, tablePriority);
}

std::string uuidTableReconcile(const std::string &oofs, const std::string &path,
                               bool recursive, bool tablePriority) {
  std::string qlog = "";


  std::string qoofs = oofs;
  std::string qpath = path;

  // sanity check: path must start with oofs
  // example: oofs=/usr/u/oofs and path=/usr/u/oofs/ext/lychnis is valid
  //          oofs=/usr/u/oofs and path=/usr/u/oofs2/ext/lychnis is NOT valid

  if (((qpath + "/").find(qoofs + "/")) != 0) {
    qlog +=
        "INTERNAL ERROR! arguments to uuidTableReconcile() are invalid:<br>";
    qlog += " oofs = " + qoofs + "<br>";
    qlog += "  path = " + qpath + "<br>";
    qlog += "  Parameter path path must start with oofs. <br>";
    return qlog;
  }
  // STEP 1 : obtain a relative path of the object wrt. oofs
  // --------------------------------------------------------------------------------
  std::string qrpath = "";
  if (qpath != qoofs)
    qrpath = qpath.substr(qoofs.length() + 1);
  // STEP 2 : obtain a lock on the .uuids file
  // --------------------------------------------------------------------------------
  Lock uuidlock = Lock::lock(qoofs + "/.uuids");
  if (!uuidlock.valid()) {
    qlog += "Could not establish lock on uuid table in file:<br>";
    qlog += "    " + qoofs + "/.uuids<br>";
    qlog += "The process holding the lock must be still running. Is it a "
            "zombie?<br>";
    qlog += "In any case, no changes have been made.<br>";
    std::cerr
        << "uuidTableReconcile:: Could not establish lock on uuid table<br>"
        << "  in file " << oofs << "/.uuids<br>";
    return qlog;
  }
  //    qlog << "Locked the table file.";
  // STEP 3 : read .uuids into list
  // --------------------------------------------------------------------------------
  UUIDtable list = readUUIDtable(qoofs + "/.uuids");
  std::stringstream out;
  out << list.size();
  qlog += "Read in UUID table with " + out.str() + " entries.<br>";
  // printList( list );
  // STEP 4 : validate all entries in list that match the prefix <path>
  // --------------------------------------------------------------------------------
  //    qlog << "Validating entries matching prefix.";
  std::string prefix = (qrpath == "") ? "" : (qrpath + "/");
  for (size_t i = 0; i < list.size(); i++) {
    UUIDtableEntry &e = list[i];
    // only process entries matching path prefix, but honor the 'recursive' flag
    if ((e.path + "/").find(prefix) != 0)
      continue;
    if ((!recursive) && (qrpath != e.path))
      continue;
    // determine if the entry is valid, and flag invalid entries
    e.validate(qoofs);
    if (e.validated && !e.valid)
      qlog += "Removed invalid entry: " + e.path + "<br>";
  }
  // printList( list );
  // STEP 5 : process all objects starting at path that have a UUID and append
  //          them to the list if they are not already there
  //    qlog << "Finding and processing all objects in oofs with specified
  //    prefix.";
  // std::cerr<<"STEP 5"<<std::endl;
  std::stack<std::string> st;
  st.push(qrpath);
  while (!st.empty()) {
    // get the top object off the stack
    std::string path = st.top();
    st.pop();
    std::string fullPath = qoofs + "/" + path;
    // see if the object has a valid UUID
    QUuid uuid = readUUIDfromFile(fullPath + "/.uuid");
    // depending on whether UUID is already in the list, insert it into the
    // list, do nothing, or delete the UUID
    if (!uuid.isNull()) {
      // this object has a uuid
      int ind = findUUID(list, uuid);
      if (ind == -1) {
        // this UUID is not in the list yet, so insert it
        list.push_back(UUIDtableEntry(uuid, path, true, true));
        qlog += "Added new object: " + path + "<br>";
      } else {
        // this UUID is already in the list, see if it's valid
        // [PASCAL] why qoofs ?????
        list[ind].validate(qoofs);
        if (!list[ind].valid) {
          // the UUID in the list was not valid, that means there is
          // no conflict, we just add this object
          list.push_back(UUIDtableEntry(uuid, path, true, true));
          qlog += "Added new object: " + path + "<br>";
        } else {
          // the UUID in the list was valid, is this a conflict?
          if (!(list[ind].path == path)) {
            // yes, this is a conflict.
            if (tablePriority) {
              // delete the .uuid of this object
              std::string file_to_be_removed = fullPath + "/.uuid";
              remove(file_to_be_removed.c_str());
              qlog += "Removed duplicate UUID from object: " + path + "<br>";
            } else {
              // we delete the uuid that the entry points to
              std::string file_to_be_removed =
                  qoofs + "/" + list[ind].path + "/.uuid";
              remove(file_to_be_removed.c_str());
              qlog += "Removed conflicting UUID from object: " + qoofs + "/" +
                      list[ind].path + "<br>";
              // and rewrite the path of the entry
              list[ind].path = path;
            }
          } // if( ! list[ind].path == path )
        }   // else ( ! list[ind].valid )
      }     // else (ind == -1 )
    }       // if( ! uuid.isNull())
    // now we need to recursively find all extensions and process those too
    if (!recursive)
      break;
    // this is broken in QT 4.6.2 & under MAC OS X 10.6.3 after fork
    //	std::stringList lst = QDir( fullPath + "/ext/" ).entryList(
    //	    QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot );
    // ... we need a workaround, trusty old Unix syscalls come to the rescue
    std::list<std::string> lst =
        QDir_entryList_dirs_nosyms_nodot(fullPath + "/ext/");
    std::list<std::string>::iterator it;

    for (it = lst.begin(); it != lst.end(); it++)
      if (path == "") // special case if this object is the top object
        st.push("ext/" + *it);
      else
        st.push(path + "/ext/" + *it);
  }
  std::cerr.flush();
  // std::cerr<<"STEP 6"<<std::endl;
  // printList( list );
  // STEP 6 : write result to .uuids file
  // --------------------------------------------------------------------------------
  // only write entries that have either:
  // - not been validated
  // - or are valid
  int validCount = 0;
  std::ofstream tableFile;
  std::string tableFilename = qoofs + "/.uuids";
  tableFile.open(tableFilename.c_str(), std::fstream::trunc);
  if (tableFile.good()) {
    for (size_t i = 0; i < list.size(); i++) {
      // skip invalid entries
      if (list[i].validated && !list[i].valid)
        continue;
      validCount++;
      // write entries that are valid, or entries that were not validated
      tableFile << list[i].uuid.toString() << " "
                << "/" << list[i].path << "/\n";
    }
    tableFile.close();
    std::stringstream argValidCount;
    argValidCount << validCount;

    qlog += "Wrote new UUID table with" + argValidCount.str() + " entries.<br>";
  } else {
    qlog += "Could not write the resulting uuid table to file:<br>";
    qlog += "  " + tableFilename;
    std::cerr << "uuidTableReconcile: cannot write uuid table to file:<br>"
              << tableFilename << "<br>";
  }
  // STEP 7 : release the lock on .uuids file
  // --------------------------------------------------------------------------------
  uuidlock.release();
  // std::cerr<<"STEP 7"<<std::endl;
  // return the log of actions
  return qlog;
}

std::string lookupUUID_withoutQt(const std::string &oofs, const char *ptr) {
  QUuid uuid = QUuid(ptr);
  // call lookupUUID
  std::string path = lookupUUID(oofs, uuid);
  return path;
}
// looks up uuid in the table
// returns '*' if not found
std::string lookupUUID(const std::string &oofs, const QUuid &uuid) {
  /*
    std::cerr << "lookupUUID running\n"
              << "   oofs = " << oofs << "\n"
              << "   uuid = " << uuid.toString() << "\n";
  */
  std::string qoofs = oofs;
  // establish a lock
  Lock lock = Lock::lock(qoofs + "/.uuids", 10, 1);
  // read the uuids
  UUIDtable list = readUUIDtable(qoofs + "/.uuids");
  // we don't need the lock anymore
  lock.release();
  // find the entry, validate it
  int ind = findUUID(list, uuid);
  // if not found, return
  // std::cerr<<"list[ind].path = "<<list[ind].path<<std::endl;
  if (ind == -1)
    return "*";
  // otherwise validate it to make sure it's ok
  list[ind].validate(qoofs);
  if (list[ind].valid) {
    std::string s = list[ind].path;
    if (s == "")
      return oofs;
    else
      return oofs + "/" + s;
  } else {
    return "*";
  }
}

// removes UUID files for a subtree (recursively if set)
// Warning: the UUID table is not updated. If you want the update, use
// reconcile after this operation.
bool deleteUUIDfiles(const std::string &path, bool recursive) {
  bool success = true;
  // std::cerr << "deleteUUIDfiles( " << path << "," << recursive << ")\n";
  std::string qpath = path;
  std::stack<std::string> stack;
  stack.push(qpath);
  while (!stack.empty()) {
    // pop the top
    std::string qp = stack.top();
    stack.pop();
    // remove this object's UUID file
    std::string uuid_file_name = qp + "/.uuid";
    if (remove(uuid_file_name.c_str()))
      std::cerr << "  - " << qp << "\n";
    // if UUID still exists, we failed
    FILE *file_test;
    file_test = fopen(uuid_file_name.c_str(), "r");
    if (file_test != NULL) {
      fclose(file_test);
      success = false;
      std::cerr << "deleteUUIDfiles() failed on " << qp << "/.uuid\n";
    }
    // if recursive not set, we're done
    if (!recursive)
      return success;
    // add children on the stack
    // this is broken in QT 4.6.2 & under MAC OS X 10.6.3 after fork
    //	std::list<std::string> lst = QDir( qp + "/ext/" ).entryList(
    //	    QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot );
    // ... we need a workaround, trusty old Unix syscalls come to the rescue
    std::list<std::string> lst = QDir_entryList_dirs_nosyms_nodot(qp + "/ext/");
    std::list<std::string>::iterator it;
    for (it = lst.begin(); it != lst.end(); it++)
      stack.push(qp + "/ext/" + *it);
  }
  return success;
}

// generates a UUID, using /dev/urandom and (s)random() techniques,
// with some sprinkle of current time (in microseconds), pid(), ppid(), and some
// other techniques
// Basically QUuid::createUuid() is a little dumb, because it uses the built in
// random number generator, which is very dumb.
QUuid QcreateUuid() {
  static struct timeval tv; // yes I know it'll be used unitialized,
  // that's not a bad thing when generating random numbers :)
  static unsigned char buff[16];
  //    static FILE* f;
  // f = fopen( "/dev/urandom","r" );
  static std::ifstream f("/dev/urandom");
  if (!f.is_open())
    f.open("/dev/urandom");
  if (f.is_open()) {
    // if /dev/urandom can be opened, get the intial data from there
    f.get((char *)buff, sizeof(buff));
  } else {
    // if /dev/urandom cannot be opened, do the slow sleep procedure :(
    static bool warned = false;
    if (!warned) {
      std::cerr
          << "QcreateUuid(): /dev/urandom not available. Using heuristic.\n"
          << "  The heuristic is slow and probably not too random :(\n";
      warned = true;
    }

    // if we were not able to read the random device, let's do
    // something different... Let's try to nanosleep 1000 times
    // for 1 nanosecond each, and measure how long it took. No
    // computer today can do that precisely and consistently...
    for (uint i = 0; i < sizeof(buff); i++) {
      int start = tv.tv_usec;
      for (int j = 0; j < 1000; j++) {
        struct timespec req;
        req.tv_sec = 0;
        req.tv_nsec = 1;
        nanosleep(&req, 0);
      }
      gettimeofday(&tv, 0);
      int end = tv.tv_usec;
      buff[i] = char(start - end);
    }
  }

  // now let's use the built in random number generator to xor the buffer
  // a little more
  gettimeofday(&tv, 0);
  srandom(random() ^ (getpid() << 16) ^ getuid() ^ tv.tv_sec ^ tv.tv_usec ^
          (getppid() << 8));
  gettimeofday(&tv, 0);
  for (int i = 0; i < ((tv.tv_sec ^ tv.tv_usec) & 31); i++)
    (void)random();
  for (uint i = 0; i < sizeof(buff); i++)
    buff[i] ^= char(random());

  // create the UUID from the data in <buff>
  unsigned char *p = buff;
  uint l = p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3];
  p += 4;
  ushort w1 = p[0] << 8 | p[1];
  p += 2;
  ushort w2 = p[0] << 8 | p[1];
  p += 2;
  QUuid uuid(l, w1, w2, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);

  return uuid;
}
