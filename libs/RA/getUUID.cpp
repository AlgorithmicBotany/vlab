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



#include "getUUID.h"
#include "uuid.h"
#include <fstream>
#include <iostream>


QUuid getUUID(const std::string oofs_dir, // root of oofs
              const std::string path,     // path to the object
              bool create) // whether to create UUID if it does not exist
{
   // returns a UUID of the object
  // if it does not exist, and create is set, a new one is created
  // otherwise NULL UUID is returned
  std::string qpath = path.c_str();
  std::string uuidFname = qpath + "/.uuid";

  // reconcile table/oofs for this path, non-recursively
  // when done, the object either has a valid UUID, and it's in the table,
  // or it does not have UUID at all
  std::string log1 = uuidTableReconcile(oofs_dir, path, false, true);
  // if the object has .uuid now, it's valid
  QUuid uuid = readUUIDfromFile(uuidFname);
  if (!uuid.isNull())
    return uuid;
  // if create was not set, return the null UUID
  if (!create)
    return uuid;
  // create a brand new UUID
  //    uuid = QUuid::createUuid();
  uuid = QcreateUuid();
  std::ofstream f;
  f.open(uuidFname.c_str(), std::fstream::trunc);
  if (f.good()) {
    std::string uuid_string = uuid.toString();
    f << uuid_string << "\n";
  } else {
    std::cerr << "getUUID() failed to write the new UUID to file\n";
  }
  f.close();
  // resolve the table again to make sure the new uuid is written there
  std::string log2 = uuidTableReconcile(oofs_dir, path, false, true);
  // and return the UUID (rereading it just in case..., paranoia)
  uuid = readUUIDfromFile(uuidFname);
  if (uuid.isNull()) {
    std::cerr << "getUUID(): failed to make a UUID. It better be because you\n"
              << "  don't have write permissions, and not because UUID was not "
                 "unique.\n"
              << "  Below is the reconcile log in case it contains something "
                 "useful.\n"
              << "........log1......................................\n";

    std::cerr << log1;
    std::cerr << "\n........log2......................................\n";
    std::cerr << log2;
    std::cerr << "\n--------log2--------------------------------------\n";
  }
  return uuid;
}

std::string
getUUID_string(const std::string oofs_dir, // root of oofs
               const std::string path,     // path to the object
               bool create) // whether to create UUID if it does not exist
{
  return getUUID(oofs_dir, path, create);
}
