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



#include "permissions.h"
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>

Permissions::Permissions() { initialized = false; }

/*
 * TestPermissions: Verifies whether a user has read, write or no permissions on
 * a directory deny overtakes allow if match depth is equal returns: 1 if the
 * user has permissions 0 if the user does not
 */

int path_depth(std::string path) {
  int count = 0;
  std::string::size_type word_pos(0);
  while (word_pos != std::string::npos) {
    word_pos = path.find("/", word_pos);
    if (word_pos != std::string::npos) {
      ++count;

      // start next search after this word
      word_pos += 1;
    }
  }

  return count;
}

int Permissions::TestPermissions(std::string userName, std::string path,
                                 std::string type) {
  int read = 0;
  int write = 0;
  int matchDepth = 0;

  // iterator contains values of permissions with
  std::map<std::string, std::vector<std::vector<std::string>>>::iterator it =
      permissions.find("all");
  std::vector<std::vector<std::string>> perm = (*it).second;
  std::vector<std::vector<std::string>>::iterator iterator = perm.begin();
  std::vector<std::string> currItem;
  while (iterator != perm.end()) {
    currItem = (*iterator);
    if (path.find(currItem.at(2)) != 0) {
      if (path_depth(currItem.at(2)) >= matchDepth) {
        matchDepth = path_depth(currItem.at(2));
        if ((currItem.at(0).compare("y") == 0) ||
            (currItem.at(0).compare("Y") == 0))
          read = 1;
        else
          read = 0;
        if ((currItem.at(1).compare("y") == 0) ||
            (currItem.at(1).compare("Y") == 0))
          write = 1;
        else
          write = 0;
      }
    }
    iterator++;
  }
  it = permissions.find(userName);
  perm = (*it).second;
  iterator = perm.begin();
  while (iterator != perm.end()) {
    currItem = (*iterator);
    if (path.find(currItem.at(2)) == 0) {
      if (path_depth(currItem.at(2)) >= matchDepth) {
        matchDepth = path_depth(currItem.at(2));
        if ((currItem.at(0).compare("y") == 0) ||
            (currItem.at(0).compare("Y") == 0))
          read = 1;
        else
          read = 0;
        if ((currItem.at(1).compare("y") == 0) ||
            (currItem.at(1).compare("Y") == 0))
          write = 1;
        else
          write = 0;
      }
    }
    iterator++;
  }

  if (((type.compare("r") == 0) || (type.compare("R") == 0)) && (read))
    return 1;
  else if (((type.compare("w") == 0) || (type.compare("W") == 0)) && (write))
    return 1;
  else if (((type.compare("rw") == 0) || (type.compare("Rw") == 0) ||
            (type.compare("rW") == 0) || (type.compare("RW") == 0)) &&
           (read) && (write))
    return 1;
  return 0;
}

int Permissions::loadPermissions(std::string filePath = "",
                                 bool create = false) {
  // read table of users and passwords
  // also build the associative array of users to their deny and allow targets
  // the array looks like this:
  // permissions is a container, it is indexed by the user name (as a QString)
  // each QMap it references is indexed by one of 2 words "read" or "write", and
  // contains a QList of directories that are allowed for each category
  permissions.clear(); // just to be sure
  users.clear();
  if (filePath.empty())
    filePath = permissionsFile;
  else
    permissionsFile = filePath;
  if (filePath.empty())
    return -1;
  std::vector<std::vector<std::string>> userPermissions;
  std::string currentName = "all";
  std::ifstream file;
  file.open(filePath.c_str());
  std::string line;
  std::vector<std::string> split;
  bool replaced;
  if (!file.is_open()) {
    // if we can't open the file then check if we are supposed to create it, and
    // if the file is really missing
    if ((!file.good()) && (create)) {
      fprintf(stderr,
              "Password file %s does not exist, trying to create it...\n",
              filePath.c_str());
      // create the password file, open it and set the permissions on it
      file.open(filePath.c_str(), std::ifstream::in);
      if (file.good()) {
      } else {
        return -1;
      }
    } else {
      return -1;
    }
  }
  while (!file.eof()) {
    char linec[256];
    file.getline(linec, 256);
    line = std::string(linec);
    size_t pos = line.find(":");
    std::string str = line;
    split.clear();
    while (pos != std::string::npos) {
      split.push_back(str.substr(0, pos));
      str = str.substr(pos + 1);
      pos = str.find(":");
    }
    split.push_back(str);
    if (split.size() > 1) {
      if (split.at(0).empty()) {
        // this is a directory entry
        replaced = false;
        split.erase(split.begin());
        // check for duplicate entries, if so take the most recent permissions
        // only
        if (split.size() == 3) {
          std::string temp = split.at(2);
          std::string temp2 = temp;
          pos = temp2.find("//");
          while (pos != std::string::npos) {
            temp.replace(pos, 2, "/");
            temp2 = temp2.substr(pos + 1);
            pos = temp2.find("//");
          }
          split.at(2) = temp;
          for (unsigned int i = 0; i < userPermissions.size(); i++) {
            if (split.at(2).compare(userPermissions.at(i).at(2)) == 0) {
              fprintf(stderr,
                      "Permissions::loadPermissions(): Duplicate directory "
                      "specified for user %s:\n%s\n",
                      currentName.c_str(), split.at(2).c_str());
              userPermissions.at(i) = split;
              replaced = true;
            }
          }
          if (!replaced)
            userPermissions.push_back(split);
        } else {
          fprintf(stderr, "Permissions::loadPermissions(): malformed line:\n ");
          for (unsigned int i = 0; i < split.size(); i++)
            fprintf(stderr, "%s:", split.at(i).c_str());
          fprintf(stderr, "\n ");
        }
      } else {
        // this is a user entry
        // push the read and write lists into the userPermissions map under the
        // name currentName
        permissions.insert(
            std::pair<std::string, std::vector<std::vector<std::string>>>(
                currentName, userPermissions));
        userPermissions.clear();
        // then clear them, set the userName and enter their password into the
        // users map
        currentName = split.at(0);
        std::map<std::string, std::string>::iterator find_users =
            users.find(currentName);
        if (find_users != users.end()) {
          fprintf(stderr, "Error, user declared twice in password file %s\n",
                  filePath.c_str());
          return -1;
        }
        users.insert(std::pair<std::string, std::string>(
            currentName,
            split.at(1))); // insert the password into the users map
      }
    }
  }
  permissions.insert(
      std::pair<std::string, std::vector<std::vector<std::string>>>(
          currentName, userPermissions));
  initialized = true;
  return 0;
}

int Permissions::savePermissions() {

  if (permissionsFile.empty()) {
    fprintf(stderr, "Permissions::savePermissions(): permissionsFile variable "
                    "is not set, cannot save permissions\n");
    return -9;
  }
  std::ofstream file;
  file.open(permissionsFile.c_str(), std::ofstream::out);

  if (file.good()) {
    std::vector<std::vector<std::string>> list = getAllUserDetails(1);
    if (list.size() != 0) {
      // if we have no entries, the next call will explode badly
      if (list.at(0).at(0).compare("All Users") == 0)
        list.erase(list.begin());
      for (unsigned int i = 0; i < list.size(); i++) {
        for (unsigned int j = 0; j < list.at(i).size() - 1; j++) {
          file << list.at(i).at(j).c_str() << ":";
        }
        file << list.at(i).at(list.at(i).size() - 1).c_str();
        file << std::endl;
      }
    }
    file.close();
  } else {
    fprintf(
        stderr,
        "Permissions::savePermissions(): unable to open file %s for output\n",
        permissionsFile.c_str());
    return -9;
  }
  return 0;
}

int Permissions::testLogin(std::string userName, std::string password) {
  int success = 0;
  if (raserver_debug)
    fprintf(stderr, "confirm_login(): login = '%s' ", userName.c_str());

  // encrypt the password, this function is kind of a dinosaur...
  std::string enc_password = crypt(password.c_str(), SALT);

  if (raserver_debug)
    fprintf(stderr, "encrypted password = %s\n", enc_password.c_str());

  std::map<std::string, std::string>::iterator find_users =
      users.find(userName);
  if (find_users != users.end())
    if ((*find_users).second.compare(enc_password) == 0)
      success = 1;

  if (raserver_debug)
    fprintf(stderr, "raserver: login '%s' was %s.\n", userName.c_str(),
            success ? "confirmed" : "denied");

  // return the result
  return !success;
}

std::vector<std::vector<std::string>>
Permissions::getAllUserDetails(int longListing) {
  std::vector<std::vector<std::string>> ret;
  std::vector<std::string> entry;
  std::string currName;
  if (initialized) {
    std::map<std::string, std::string>::iterator iterator = users.begin();

    std::map<std::string, std::vector<std::vector<std::string>>>::iterator it =
        permissions.find("all");
    std::vector<std::vector<std::string>> perm = (*it).second;
    std::vector<std::vector<std::string>>::iterator listIterator = perm.begin();

    if (perm.size() != 0) {
      // don't put the header on if there are no entries to share
      if (longListing) {
        entry.clear();
        entry.push_back("All Users");
        entry.push_back("");
        ret.push_back(entry);
        while (listIterator != perm.end()) {
          entry.clear();
          entry.push_back("");
          std::vector<std::string>::iterator listIt = (*listIterator).begin();
          while (listIt != (*listIterator).end()) {
            entry.push_back(*listIt);
            listIt++;
          }
          ret.push_back(entry);
          listIterator++;
        }
      }
    }

    while (iterator != users.end()) {
      currName = (*iterator).first;
      // push name, password onto ret through entry
      entry.clear();
      entry.push_back(currName);
      entry.push_back((*users.find(currName)).second);
      ret.push_back(entry);
      if (longListing) {
        listIterator = (*permissions.find(currName)).second.begin();
        while (listIterator != (*permissions.find(currName)).second.end()) {
          entry.clear();
          entry.push_back("");
          std::vector<std::string>::iterator listIt = (*listIterator).begin();
          while (listIt != (*listIterator).end()) {
            entry.push_back(*listIt);
            listIt++;
          }
          ret.push_back(entry);
          listIterator++;
        }
      }
      iterator++;
    }
  } else {
    ret.push_back(entry); // return a non-null empty list
  }
  return ret;
}

std::vector<std::vector<std::string>>
Permissions::getUserPermissions(std::string userName) {
  std::vector<std::vector<std::string>> ret;
  std::vector<std::string> line;
  // since there is no default (or blank) constructor for a QListIterator, we
  // can either have a mess of if statements at the top here or we can
  // initialize 2 different iterators depending on the situation and waste a
  // couple of bytes of memory....
  if (permissions.find("all") != permissions.end()) {
    std::map<std::string, std::vector<std::vector<std::string>>>::iterator it =
        permissions.find("all");
    std::vector<std::vector<std::string>> perm = (*it).second;
    std::vector<std::vector<std::string>>::iterator allIterator = perm.begin();
    while (allIterator != perm.end()) {
      line.clear();
      if (userName.compare("all") != 0) {
        line.push_back("I");
        std::vector<std::string>::iterator listIt = (*allIterator).begin();
        while (listIt != (*allIterator).begin()) {
          line.push_back(*listIt);
          listIt++;
        }
      } else {
        line.push_back("");
        std::vector<std::string>::iterator listIt = (*allIterator).begin();
        while (listIt != (*allIterator).begin()) {
          line.push_back(*listIt);
          listIt++;
        }
      }
      ret.push_back(line);
      allIterator++;
    }
  }
  std::map<std::string, std::vector<std::vector<std::string>>>::iterator it =
      permissions.find(userName);
  if ((it != permissions.end()) && (userName.compare("all") != 0)) {
    std::vector<std::vector<std::string>> perm = (*it).second;
    std::vector<std::vector<std::string>>::iterator nameIterator = perm.begin();
    while (nameIterator != perm.end()) {
      line.clear();
      line.push_back("");
      std::vector<std::string>::iterator listIt = (*nameIterator).begin();
      while (listIt != (*nameIterator).end()) {
        line.push_back(*listIt);
        listIt++;
      }
      ret.push_back(line);
      nameIterator++;
    }
  }
  return ret;
}

int Permissions::addUser(std::string userName, std::string password,
                         std::string copyUser) {
  // return status:
  // -1: user already exists
  // -2: copyUser does not exist
  // -3: error adding user to map
  // -9: error writing password file
  if (users.find(userName) != users.end())
    return -1;
  if (userName.compare("all") == 0)
    return -1;
  if (!copyUser.empty()) {
    if (users.find(copyUser) == users.end())
      return -2;
  }
  std::string enc_password = crypt(password.c_str(), SALT);
  users.insert(std::pair<std::string, std::string>(userName, enc_password));
  std::vector<std::vector<std::string>> list;
  permissions.insert(
      std::pair<std::string, std::vector<std::vector<std::string>>>(userName,
                                                                    list));
  if (!copyUser.empty()) {
    std::vector<std::vector<std::string>> list =
        (*permissions.find(copyUser)).second;
    fprintf(stderr, "copying %i permissions from %s to %s\n", int(list.size()),
            copyUser.c_str(), userName.c_str());
    permissions.insert(
        std::pair<std::string, std::vector<std::vector<std::string>>>(userName,
                                                                      list));
  }
  return savePermissions();
}

int Permissions::delUser(std::string userName) {
  // return status:
  // -1: user was not in user list
  // -2: tried to remove user "all"
  // -9: error writing new password file
  if (userName.compare("all") == 0)
    return -2;
  if (users.erase(userName) != 1)
    return -1;
  permissions.erase(
      userName); // we don't care about the return value here, it is not crucial
                 // that the user had any specific permissions
  return savePermissions();
}

int Permissions::changeLogin(std::string oldName, std::string newName) {
  // return status:
  // -1: oldName was not in user list
  // -2: newName is already in user list
  // -9: error writing new password file
  // first, some sanity checks
  if (users.find(oldName) == users.end())
    return -1;
  if (users.find(newName) != users.end())
    return -2;
  // then the re-assignment
  users.insert(std::pair<std::string, std::string>(
      newName, (*users.find(oldName)).second));
  users.erase(oldName);
  if (permissions.find(oldName) != permissions.end()) {
    std::map<std::string, std::vector<std::vector<std::string>>>::iterator it =
        permissions.find(oldName);
    std::vector<std::vector<std::string>> perm = (*it).second;

    permissions.insert(
        std::pair<std::string, std::vector<std::vector<std::string>>>(newName,
                                                                      perm));
    permissions.erase(oldName);
  }
  return savePermissions();
}

int Permissions::changePassword(std::string userName, std::string newPassword) {
  // return status:
  // -1: userName is not in user list
  // -9: error writing new password file
  if (users.find(userName) == users.end())
    return -1;
  std::string enc_password = crypt(newPassword.c_str(), SALT);
  (*users.find(userName)).second = enc_password;
   return savePermissions();
}

bool Permissions::userExists(std::string userName) {
  return users.find(userName) != users.end();
}

int Permissions::deleteRule(std::string userName, int ruleNumber) {
  // return status:
  // -1: ruleNumber is out of range for permissions list for userName, this also
  // covers "user does not exist" -9: error writing new password file
  std::vector<std::vector<std::string>> list =
      (*permissions.find(userName)).second;
  // if userName is not in permissions we will get back a 0-length list, so the
  // ruleNumber will always be out of range
  if (list.size() == 0)
    return -1;
  if (int(list.size()) <= ruleNumber)
    return -1;
  std::vector<std::vector<std::string>>::iterator listIterator = list.begin();
  for (int i = 0; i < ruleNumber; i++)
    listIterator++;
  list.erase(listIterator);
  (*permissions.find(userName)).second = list;
  return savePermissions();
}

int Permissions::addRule(std::string userName, std::string readWrite,
                         std::string path) {
  // return status:
  // -1: rule already exists for this path
  // -2: invalid permissions specified
  // -3: user does not exist
  // -9: error writing new password file
  if ((!userExists(userName)) && (userName.compare("all") != 0))
    return -3;
  std::vector<std::vector<std::string>> list =
      (*permissions.find(userName)).second;
  std::string temp2 = path;
  std::size_t pos = temp2.find("//");
  while (pos != std::string::npos) {
    path.replace(pos, 2, "/");
    temp2 = temp2.substr(pos + 1);
    pos = temp2.find("//");
  }

  for (unsigned int i = 0; i < list.size(); i++) {
    if (path.compare(list.at(i).at(2)) == 0)
      return -1;
  }
  int rw = parseReadWrite(readWrite);
  if (rw < 0)
    return -2;
  std::vector<std::string> entry;
  if ((rw == 4) || (rw == 6))
    entry.push_back("y");
  else
    entry.push_back("");
  if ((rw == 2) || (rw == 6))
    entry.push_back("y");
  else
    entry.push_back("");
  entry.push_back(path);
  list.push_back(entry);
  (*permissions.find(userName)).second = list;
  return savePermissions();
}

int Permissions::changeRule(std::string userName, std::string readWrite,
                            int ruleNumber) {
  // return status:
  // -1: rule number out of range
  // -2: invalid permissions specified
  // -9: error writing new password file
  std::vector<std::vector<std::string>> list =
      (*permissions.find(userName)).second;
  // if userName is not in permissions we will get back a 0-length list, so the
  // ruleNumber will always be out of range
  if (list.size() == 0)
    return -1;
  if (int(list.size()) <= ruleNumber)
    return -1;
  std::vector<std::string> rule = list.at(ruleNumber);
  int rw = parseReadWrite(readWrite);
  if (rw < 0)
    return -2;
  if ((rw == 4) || (rw == 6))
    rule.at(0) = "y";
  else {
    rule.at(0) = "";
  }
  if ((rw == 2) || (rw == 6))
    rule.at(1) = "y";
  else
    rule.at(1) = "";
  list.at(ruleNumber) = rule;
  (*permissions.find(userName)).second = list;
   return savePermissions();
}

int Permissions::parseReadWrite(std::string readWrite) {
  int ret = -1;
  if ((readWrite.compare("r") == 0) || (readWrite.compare("R") == 0)) {
    ret = 4;
  } else if (readWrite.compare("4") == 0) {
    ret = 4;
  } else if ((readWrite.compare("rw") == 0) || (readWrite.compare("Rw") == 0) ||
             (readWrite.compare("rW") == 0) || (readWrite.compare("RW") == 0)) {
    ret = 6;
  } else if (readWrite.compare("6") == 0) {
    ret = 6;
  } else if ((readWrite.compare("w") == 0) || (readWrite.compare("W") == 0)) {
    ret = 2;
  } else if (readWrite.compare("2") == 0) {
    ret = 2;
  } else if (readWrite.compare("0") == 0)
    ret = 0;
  else if (readWrite.compare("-") == 0)
    ret = 0;
  return ret;
}
