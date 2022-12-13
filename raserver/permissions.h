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



#ifndef PERMISSIONS_H
#define PERMISSIONS_H
#include <string>
#include <vector>
#include <map>
#include "raserver.h"

class Permissions {
public:
  Permissions();
  int TestPermissions(std::string userName, std::string path, std::string type);
  int loadPermissions(std::string filePath, bool create);
  int testLogin(std::string userName, std::string password);
  inline bool isInitialized() { return initialized; };
  std::vector<std::vector<std::string>> getAllUserDetails(int longListing);
  int addUser(std::string userName, std::string password,
              std::string copyUser = "");
  int delUser(std::string userName);
  int changeLogin(std::string oldName, std::string newName);
  int changePassword(std::string userName, std::string newPassword);
  bool userExists(std::string userName);
  std::vector<std::vector<std::string>>
  getUserPermissions(std::string userName);
  int deleteRule(std::string userName, int ruleNumber);
  int addRule(std::string userName, std::string readWrite, std::string path);
  int changeRule(std::string userName, std::string readWrite, int ruleNumber);

private:
  int savePermissions();
  int parseReadWrite(std::string readWrite);
  std::map<std::string, std::vector<std::vector<std::string>>> permissions;
  std::map<std::string, std::string> users;
  bool initialized;
  std::string permissionsFile;
};

#endif // PERMISSIONS_H
