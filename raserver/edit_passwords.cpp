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



/******************************************************************************
 *
 * This set of functions allows the user to modify the
 * password file for raserver
 *
 *   - change a password
 *   - change a user name
 *   - add a user
 *   - delete a user
 *   - list users


 * The password file has the following format:
 *
 *
 * <file>          = { <entry> }
 * <entry>         = <login_name> ':' <password> ':' 'nl'
 * <login_name>    = <string>
 * <password>      = <string>
 * <string>        = [ <letter> | <digit> ]
 * <letter>        = 'a' | 'b' | ... | 'z' | 'A' | 'B' | ... | 'Z'
 * <digit>         = '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9' | '0'

 */

#include <cstdio>
#include <cstdlib>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>
#include <readline/readline.h>
#include <readline/history.h>
#include <vector>
#include <iostream>

#include "edit_passwords.h"
#include "dsprintf.h"
#include "xmemory.h"
#include "entry.h"
#include "raserver.h"
#include "xstring.h"
#include "permissions.h"

// prototypes

static void list_users(int argc, char **argv);
static void add_user(int argc, char **argv);
static void delete_user(int argc, char **argv);
static int ask_yes_no(std::string prompt);
static void change_login(int argc, char **argv);
static void change_password(int argc, char **argv);
static void edit_permissions(int argc, char **argv);
static void test_permissions(int argc, char **argv);
static int parse_string(const char *str, char **(&argv));
static void help(int argc, char **argv);
static void edit_help(std::vector<std::string> args);
static void print_user_permissions(std::string login);
static void delete_permission(std::string login, std::vector<std::string> args);
static void add_permission(std::string login, std::vector<std::string> args);
static void change_permission(std::string login, std::vector<std::string> args);
static std::vector<std::string> list_from_string(std::string args);

// --- end of prototypes

// global variables

extern Permissions user_permissions;

// end of global variables

void edit_passwords(void) {
  if (!user_permissions.isInitialized()) {
    fprintf(stderr, "User permissions object was not initalized prior to "
                    "calling edit_passwords().  Exiting\n");
    exit(-1);
  }
  printf("\nFor a list of available commands type 'help'.\n");

  add_history("help");
  while (1) {
    char *command_line = readline("raserver>");
    if (command_line == 0) {
      printf("\n");
      exit(0);
    }
    add_history(command_line);

    char **argv;
    int argc = parse_string(command_line, argv);
    xfree(command_line);

    if (argc == 0)
      continue;

    // Quit
    if (strcasecmp(argv[0], "quit") == 0) {
      exit(0);
    }

    // Help
    else if (strcasecmp(argv[0], "help") == 0 ||
             strcasecmp(argv[0], "?") == 0) {
      help(argc, argv);
    }

    // List
    else if (strcasecmp(argv[0], "ls") == 0) {
      list_users(argc, argv);
    }

    // Add user
    else if (strcasecmp(argv[0], "add") == 0) {
      add_user(argc, argv);
    }

    // Delete user
    else if (strcasecmp(argv[0], "del") == 0) {
      delete_user(argc, argv);
    }

    // Change login
    else if (strcasecmp(argv[0], "chlog") == 0) {
      change_login(argc, argv);
    }

    // Change password
    else if (strcasecmp(argv[0], "chpass") == 0) {
      change_password(argc, argv);
    }

    // Edit permissions
    else if (strcasecmp(argv[0], "edit") == 0) {
      edit_permissions(argc, argv);
    }

    else if (strcasecmp(argv[0], "test") == 0) {
      test_permissions(argc, argv);
    }

    else {
      printf("Unknown command '%s'\n", argv[0]);
    }

    // free argv
    for (int i = 0; i < argc; i++)
      xfree(argv[i]);
    xfree(argv);
  }
}

/******************************************************************************
 *
 * parser for editing permissions for a single user
 *
 */

std::vector<std::string> list_from_string(std::string args) {
  std::vector<std::string> l;
  std::string::size_type cur_pos(0);
  std::string::size_type prev_pos(0);
  while (cur_pos != std::string::npos) {
    cur_pos = args.find(" ", cur_pos);
    if (cur_pos != std::string::npos) {
      l.push_back(args.substr(prev_pos, cur_pos - prev_pos));
      // start next search after this word
      cur_pos += 1;
      prev_pos = cur_pos;
    }
  }
  l.push_back(args.substr(prev_pos, cur_pos));
  return l;
}

void edit_permissions(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: edit login\n");
    return;
  }

  std::string login = argv[1];
  // test that the user we are editing exists
  if ((!user_permissions.userExists(login)) && (login.compare("all") != 0)) {
    printf("User %s not found.\n", login.c_str());
    return;
  }
  // print a header and the user's current permissions
  printf("\nCurrent permissions for user %s\n", login.c_str());
  print_user_permissions(login);
  // cache the command line history then clear it, only on readline version 4.2
  std::vector<std::string> oldEntries;
  if (RL_READLINE_VERSION == 0x0402) {
    for (int i = 1; i < history_get_history_state()->length; i++) {
      oldEntries.push_back(history_get(i)->line);
    }
    clear_history();
  }
  // initiate parser
  std::string prompt = login;
  prompt.append(">");
  while (1) {
    char *command_line = readline(prompt.c_str());
    if (command_line == 0) {
      printf("\n");
      exit(0);
    }

    add_history(command_line);
    std::vector<std::string> args = list_from_string(command_line); 
    delete command_line;

    if ((args.size() == 0) || (args.begin()->empty()))
      continue;
    // Quit
    if (args.begin()->compare("quit") == 0)
      break;
    // Help
    else if (args.begin()->compare("help") == 0 ||
             args.begin()->compare("?") == 0)
      edit_help(args);
    else if (args.begin()->compare("ls") == 0)
      print_user_permissions(login);
    else if (args.begin()->compare("del") == 0)
      delete_permission(login, args);
    else if (args.begin()->compare("add") == 0)
      add_permission(login, args);
    else if (args.begin()->compare("chmod") == 0)
      change_permission(login, args);
    // Unknown command
    else
      printf("Unknown command '%s'\n", args.begin()->c_str());
  }
  if (RL_READLINE_VERSION == 0x0402) {
    clear_history();
    // restore the command line history and return
    std::vector<std::string>::iterator it;
    for (it = oldEntries.begin(); it != oldEntries.end(); it++) {
      add_history(it->c_str());
    }
  }
}

void add_permission(std::string login, std::vector<std::string> args) {
  // the path may have spaces in it so the args.size() may be more than 3, but
  // never less
  if (args.size() < 3) {
    printf("Add rule: Usage error.  usage: add [r|w|rw|-|0|2|4|6] path\n");
    return;
  }
  std::vector<std::string>::iterator args_it = args.begin();
  args_it++;
  args_it++;
  std::string path = *args_it;
  args_it++; // re-construct the path including any spaces
  while (args_it != args.end()) {
    path.append(" ").append(*args_it);
    args_it++;
  }
  if (path.find("/") != 0) {
    printf("Add rule: Paths may not be relative, all paths must begin with "
           "\"/\"\n");
    return;
  }
  args_it = args.begin();
  args_it++;
  int ret = user_permissions.addRule(login, *args_it, path);
  if (ret == 0)
    printf("Success.\n");
  else if (ret == -1)
    printf("A rule already exists for this path\n");
  else if (ret == -2)
    printf("Invalid permissions specified, allowed options are "
           "[r|w|rw|-|0|2|4|6]\n");
  else if (ret == -3)
    printf("Invalid user %s\n", login.c_str());
  else if (ret == -9)
    printf("Unable to save changes to password file.\n");
}

void change_permission(std::string login, std::vector<std::string> args) {
  // begin the sanity checking
  if (args.size() != 3) {
    printf("Change rule: Usage error. Usage: chmod [r|w|rw|-|0|2|4|6] #\n");
    return;
  }
  std::vector<std::string>::iterator args_it = args.begin();
  args_it++;
  if ((args[1].empty()) || ((++args_it)->empty())) {
    printf("Change rule: Usage error. Usage: chmod [r|w|rw|-|0|2|4|6] #\n");
    return;
  }
  if (!isdigit(args[2][0])) {
    printf("Change rule: Usage error.  No non-numberic characters allowed in "
           "rule number\n");
    return;
  }
  std::vector<std::vector<std::string>> permissions =
      user_permissions.getUserPermissions(login);
  int ruleNum = atoi(args[2].c_str());
  if ((ruleNum >= int(permissions.size())) || (ruleNum < 0)) {
    printf("Change rule: Usage error.  Invalid rule number specified: %i\n",
           ruleNum);
    return;
  }
  if (permissions.at(ruleNum).at(0).compare("i") == 0) {
    printf("Change rule: Unable to edit global rule.  Quit then use \"edit "
           "all\" to change these rules\n");
    return;
  }
  // okay, we really are deleting one of our own rules, let's find out which one
  // by subtracting the number of global rules from the index number
  int i = 0;
  while (permissions.at(i).at(0).compare("i") == 0) {
    i++;
  }
  ruleNum -= i;
  int ret = user_permissions.changeRule(login, args[1], ruleNum);
  if (ret == 0)
    printf("Success.\n");
  else if (ret == -1)
    printf("Failed.\n"); // really it should be impossible to get here, but just
                         // incase
  else if (ret == -2)
    printf("Invalid permissions specified, allowed options are "
           "[r|w|rw|-|0|2|4|6]\n");
  else if (ret == -9)
    printf("Unable to save changes to password file.\n");
}

void delete_permission(std::string login, std::vector<std::string> args) {
  // begin the sanity checking....
  if (args.size() != 2) {
    printf("Delete rule: Usage error. Usage: del #\n");
    return;
  }
  if (args.at(1).empty()) {
    printf("Delete rule: Usage error.  usage: del #\n");
    return;
  }
  if (!isdigit(args[1][0])) {
    printf("Delete rule: Usage error.  No non-numberic characters allowed in "
           "rule number\n");
    return;
  }
  std::vector<std::vector<std::string>> permissions =
      user_permissions.getUserPermissions(login);
  int ruleNum = atoi(args.at(1).c_str());
  if ((ruleNum >= int(permissions.size())) || (ruleNum < 0)) {
    printf("Delete rule: Usage error.  Invalid rule number specified: %i\n",
           ruleNum);
    return;
  }
  if (permissions.at(ruleNum).at(0).compare("i") == 0) {
    printf("Delete rule: Unable to delete global rule.  Quit then use \"edit "
           "all\" to change these rules\n");
    return;
  }
  // okay, we really are deleting one of our own rules, let's find out which one
  // by subtracting the number of global rules from the index number
  int i = 0;
  while (permissions.at(i).at(0).compare("i") == 0) {
    i++;
  }
  ruleNum -= i;
  int ret = user_permissions.deleteRule(login, ruleNum);
  if (ret == 0)
    printf("Success.\n");
  else if (ret == -1)
    printf("Failed.\n"); // really it should be impossible to get here, but just
                         // incase
  else if (ret == -9)
    printf("Unable to save changes to password file.\n");
}

void print_user_permissions(std::string login) {
  printf("##  I  R/W   Path\n");
  printf("---------------------------------------------------------------------"
         "---------\n");
  std::vector<std::vector<std::string>> permissions =
      user_permissions.getUserPermissions(login);
  for (unsigned int i = 0; i < permissions.size(); i++) {
    printf("%-4i", i);
    if (permissions.at(i).at(0).compare("i") == 0)
      printf("I  ");
    else
      printf("   ");
    if (permissions.at(i).at(1).compare("y") == 0)
      printf("R/");
    else
      printf("-/");
    if (permissions.at(i).at(2).compare("y") == 0)
      printf("W");
    else
      printf("-");
    printf("   %s\n", permissions.at(i).at(3).c_str());
  }
}

/******************************************************************************
 *
 * will list the users in the password file
 *
 */

void list_users(int argc, char **argv) {
  int long_listing = 0;
  if (argc == 2)
    if (strcasecmp("-l", argv[1]) == 0)
      long_listing = 1;

  std::vector<std::vector<std::string>> usersAndPermissions =
      user_permissions.getAllUserDetails(long_listing);

  // display all entries
  printf("\n");
  if (long_listing)
    printf("User Name          Encrypted Password   R/W  Path\n"
           "-------------------------------------------------------------------"
           "-----------\n");
  else
    printf("User Name          Encrypted Password\n"
           "----------------------------------------\n");
  std::vector<std::string> line;
  int size = usersAndPermissions.size();
  for (int i = 0; i < size; i++) {
    line = usersAndPermissions[i];
    if (line.at(0).empty()) {
      printf("                                        ");
      if (line.at(1).compare("y") == 0)
        printf("R/");
      else
        printf("-/");
      if (line.at(2).compare("y") == 0)
        printf("W");
      else
        printf("-");
      printf(" %s\n", line.at(3).c_str());
    } else
      printf("%-18s %-20s\n", line.at(0).c_str(), line.at(1).c_str());
  }
  printf("\n");
}

void test_permissions(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: test path\n");
    return;
  }
  std::vector<std::vector<std::string>> users =
      user_permissions.getAllUserDetails(0);
  std::string path = argv[1];
  printf("\nTesting user permissions on path:\n");
  printf("%s\n", path.c_str());
  printf("Username            R/W\n");
  printf("---------------------------------------------------------------------"
         "---------\n");
  printf("All Users           ");
  if (user_permissions.TestPermissions("all", path, "r"))
    printf("R/");
  else
    printf("-/");
  if (user_permissions.TestPermissions("all", path, "w"))
    printf("W\n");
  else
    printf("-\n");
  for (unsigned int i = 0; i < users.size(); i++) {
    printf("%-18s  ", users.at(i).at(0).c_str());
    if (user_permissions.TestPermissions(users.at(i).at(0), path, "r"))
      printf("R/");
    else
      printf("-/");
    if (user_permissions.TestPermissions(users.at(i).at(0), path, "w"))
      printf("W\n");
    else
      printf("-\n");
  }
  printf("\n");
}

/******************************************************************************
 *
 * add a user to the password file
 *
 */

void add_user(int argc, char **argv) {
  // make sure we have both a login name and a password
  if ((argc != 3) && (argc != 4)) {
    printf("Usage: add login password [user]\n"
           "Where [user] is the user to copy permissions from\n");
    return;
  }

  std::string login = argv[1];
  std::string password = argv[2];
  std::string copyUser;
  if (argc == 4)
    copyUser = argv[3];

  // ask the user for confirmation
  std::string prompt = std::string("Please confirm:\n  add user " + login +
                                   " with password " + password);
  if (argc == 4)
    prompt.append(" as copy of user " + copyUser);
  prompt.append("?");
  if (!ask_yes_no(prompt))
    return;

  int ret = user_permissions.addUser(login, password, copyUser);

  // Handle the possible return values
  if (ret == 0)
    printf("Success.\n");
  else if (ret == -1)
    printf("Unable to add user %s, user already exists.\n", login.c_str());
  else if (ret == -2)
    printf("Unable to find user %s to copy permissions from.\n",
           copyUser.c_str());
  else if (ret == -3)
    printf("Unable to add user to table.\n");
  else if (ret == -9)
    printf("Unable to save changes to password file.\n");
}

/******************************************************************************
 *
 * deletes a user from the password file
 *
 */

void delete_user(int argc, char **argv) {
  // first make sure we have proper arguments
  if (argc != 2) {
    printf("Delete user: Usage error. Usage: del login\n");
    return;
  }

  std::string login = argv[1];

  // ask for confirmation
  if (!ask_yes_no(
          std::string("Are you sure you want to delete user " + login + "?"))) {
    return;
  }

  int ret = user_permissions.delUser(login);
  if (ret == 0)
    printf("Success.\n");
  else if (ret == -1)
    printf("User %s did not exist in password file.\n", login.c_str());
  else if (ret == -2)
    printf("User \"all\" cannot be deleted.\n");
  else if (ret == -9)
    printf("Unable to save changes to password file.\n");
}

/******************************************************************************
 *
 * allows a user to change a login name in the password file
 *
 */

void change_login(int argc, char **argv) {
  // make sure we have enough parameters
  if (argc != 3) {
    fprintf(stderr, "chlog: usage error. Usage: chlog "
                    "old_login new_login\n");
    return;
  }

  std::string old_login = argv[1];
  std::string new_login = argv[2];

  // ask for confirmation
  if (!ask_yes_no(std::string("Are you sure you want to rename user " +
                              old_login + " to " + new_login + "?"))) {
    return;
  }

  int ret = user_permissions.changeLogin(old_login, new_login);
  if (ret == 0)
    printf("Success.\n");
  else if (ret == -1)
    printf("User %s not found in password file.\n", old_login.c_str());
  else if (ret == -2)
    printf("User %s already exists in password file.\n", new_login.c_str());
  else if (ret == -9)
    printf("Unable to save changes to password file.\n");
}

/******************************************************************************
 *
 * allows a user to change a password in the password file
 *
 */

void change_password(int argc, char **argv) {
  // make sure we have enough parameters
  if (argc != 3) {
    fprintf(stderr, "chpass: usage error. Usage: chpass "
                    "login new_password\n");
    return;
  }

  std::string login = argv[1];
  std::string new_password = argv[2];

  // ask for confirmation
  if (!ask_yes_no(
          std::string("Are you sure you want to change the password for user " +
                      login + " to " + new_password + "?"))) {
    return;
  }

  int ret = user_permissions.changePassword(login, new_password);
  if (ret == 0)
    printf("Success.\n");
  else if (ret == -1)
    printf("User %s not found in database.\n", login.c_str());
  else if (ret == -9)
    printf("Unable to save changes to password file.\n");
}

/******************************************************************************
 *
 * gives the user a prompt (supplied to the function) and asks to answer
 * with yes or no
 *
 * returns: 1 - if the answer is yes
 *          0 - if the answer is no
 *
 */

int ask_yes_no(std::string prompt) {
  while (1) {
    printf("%s [y/n] ", prompt.c_str());

    char buf[4];
    if (NULL == fgets(buf, sizeof(buf), stdin))
      return 0;

    // get rid of the \n
    size_t l = strlen(buf);
    if (l > 0)
      if (buf[l - 1] == '\n')
        buf[l - 1] = 0;

    if (xstrcmp("y", buf) == 0)
      return 1;
    if (xstrcmp("yes", buf) == 0)
      return 1;
    if (xstrcmp("n", buf) == 0)
      return 0;
    if (xstrcmp("no", buf) == 0)
      return 0;
  }
}

/******************************************************************************
 *
 * gives the user prompt (supplied to the function) and asks to respond
 * with a string
 *
 */

/*
char * ask_string( const char * prompt)
{
    char buf[ 4096];

    printf( "%s ", prompt);

    if( NULL == fgets( buf, sizeof( buf), stdin))
        return NULL;

    return xstrdup( buf);
}
*/

/******************************************************************************
 *
 * will separate the input string into a list of strings
 *
 * Returs:
 *
 *    - number of strings in the string (separated by blanks)
 *    - the strings are stored in argv
 */

int parse_string(const char *str, char **(&argv)) {
  char *ptr = (char *)str;
  int argc = 0;
  argv = (char **)xmalloc(sizeof(char *));

  // process the entire string
  while (*ptr != '\0') {
    // skip all blanks
    if (isspace(*ptr)) {
      ptr++;
      continue;
    }

    // no blank - read the string in (into tmp)
    char *tmp = (char *)xmalloc(1);
    int n = 0;
    while ((!isspace(*ptr)) && (*ptr != '\0')) {
      tmp[n] = *ptr;
      n++;
      tmp = (char *)xrealloc(tmp, n + 1);
      ptr++;
    }
    tmp[n] = '\0'; // end the string with '\0'

    // add the string to argv and update argc
    argv[argc] = tmp;
    argc++;
    argv = (char **)xrealloc(argv, sizeof(char *) * (argc + 1));
  }
  argv[argc] = NULL;

  // return number of arguments parsed
  return argc;
}

/******************************************************************************
 *
 * give help to the user
 *
 */

void help(int argc, char **argv) {
  if (argc == 1) {
    // just a general help
    printf("\n"
           "Commands:\n"
           "\n"
           "quit    - quit the program\n"
           "help    - gives you this help\n"
           "ls      - gives you a list of users in the password file\n"
           "add     - add a user\n"
           "del     - delete a user\n"
           "chlog   - change login name\n"
           "chpass  - change password for a user\n"
           "edit    - edit directory permissions for a user\n"
           "\n");
    return;
  }

  for (int i = 1; i < argc; i++) {
    printf("\n");
    if (strcasecmp(argv[i], "help") == 0) {
      printf("NAME\n"
             "     help - show help for editing password file\n"
             "\n"
             "SYNOPSIS\n"
             "     help [command ...]\n");
    } else if (strcasecmp(argv[i], "quit") == 0) {
      printf("NAME\n"
             "     quit - quit editing passoword file\n"
             "\n"
             "SYNOPSIS\n"
             "     quit\n");
    } else if (strcasecmp(argv[i], "ls") == 0) {
      printf("NAME\n"
             "     ls - list the users in the password file\n"
             "\n"
             "SYNOPSIS\n"
             "     ls [-l]\n");
    } else if (strcasecmp(argv[i], "add") == 0) {
      printf("NAME\n"
             "     add - add a user to the password file\n"
             "\n"
             "SYNOPSIS\n"
             "     add login_name password write_access\n"
             "     where write_access is either 'y' or 'n'\n");
    } else if (strcasecmp(argv[i], "del") == 0) {
      printf("NAME\n"
             "     del - delete a user from the password file\n"
             "\n"
             "SYNOPSIS\n"
             "     del login_name\n");
    } else if (strcasecmp(argv[i], "chlog") == 0) {
      printf("NAME\n"
             "     chlog - change a login for a user\n"
             "\n"
             "SYNOPSIS\n"
             "     chlog login_name new_login\n");
    } else if (strcasecmp(argv[i], "chpass") == 0) {
      printf("NAME\n"
             "     chpass - change the password for a user\n"
             "\n"
             "SYNOPSIS\n"
             "     chpass login_name new_password\n");
    }

    else if (strcasecmp(argv[i], "edit") == 0) {
      printf("NAME\n"
             "     edit - edit directory permissions for a user\n"
             "\n"
             "SYNOPSIS\n"
             "     edit login_name\n"
             "     if login_name is \"all\" then it will allow you to edit the "
             "global rules list\n");
    }

    else {
      printf("Unknown keyword '%s'\n", argv[i]);
    }
    printf("\n");
  }
}
void edit_help(std::vector<std::string> args) {
  if (args.size() == 1) {
    // just a general help
    printf("\n"
           "Commands:\n"
           "\n"
           "quit    - quit the permissions editor\n"
           "help    - gives you this help\n"
           "ls      - gives you a list of rules that apply to this user\n"
           "add     - add a rule\n"
           "del     - delete a rule\n"
           "chmod   - change permissions for a rule\n"
           "\n");
    return;
  }

  for (unsigned int i = 1; i < args.size(); i++) {
    printf("\n");
    if (args.at(i).compare("help") == 0) {
      printf("NAME\n"
             "     help - show help for editing password file\n"
             "\n"
             "SYNOPSIS\n"
             "     help [command ...]\n");
    } else if (args.at(i).compare("quit") == 0) {
      printf("NAME\n"
             "     quit - quit the permissions editor\n"
             "\n"
             "SYNOPSIS\n"
             "     quit\n");
    } else if (args.at(i).compare("ls") == 0) {
      printf(
          "NAME\n"
          "     ls - gives you a list of rules that apply to this user\n"
          "\n"
          "SYNOPSIS\n"
          "     ls\n"
          "\n"
          "OUTPUT\n"
          "     ##  I  R/W   Path\n"
          "------------------------------------------------\n"
          " ##   : Rule number\n"
          " I    : Rule is (I)nherited from \"All Users\"\n"
          " R/W  : User has (R)ead or (W)rite permissions\n"
          " Path : Path on which permissions are granted\n"
          "        If the path does not end with a / then the rule will apply "
          "to actions such as deleting or renaming the object itself\n");
    } else if (args.at(i).compare("add") == 0) {
      printf("NAME\n"
             "     add - allows you to add a rule for this user\n"
             "\n"
             "SYNOPSIS\n"
             "     add [r|w|rw|-|0|2|4|6] path\n");
    } else if (args.at(i).compare("del") == 0) {
      printf("NAME\n"
             "     del - allows you to delete a rule for this user\n"
             "\n"
             "SYNOPSIS\n"
             "     del # - where # is the rule number displayed in the list\n");
    } else if (args.at(i).compare("chmod") == 0) {
      printf("NAME\n"
             "     chmod - allows you to modify the permissions of an existing "
             "rule\n"
             "\n"
             "SYNOPSIS\n"
             "     chmod [r|w|rw|-|0|2|4|6] # - where # is the rule number "
             "displayed on the list\n");
    } else {
      printf("Unknown keyword '%s'\n", args.at(i).c_str());
    }
    printf("\n");
  }
}
