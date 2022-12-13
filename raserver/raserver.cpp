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



// Unix includes
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/param.h>
#include <assert.h>
#include <errno.h>
#include <time.h>
#include <stdarg.h>
#include <iostream>
#include <pthread.h>

#include "quuid.h"

// VLAB includes
#include "raserver.h"
#include "getUUID.h"
#include "uuid.h"
#include "MessagePipe.h"
#include "RA.h"
#include "xstring.h"
#include "dsprintf.h"
#include "xmemory.h"
#include "dirList.h"
#include "utilities.h"
#include "confirm_login.h"
#include "edit_passwords.h"
#include "permissions.h"
#include "get_extensions.h"
#include "debug.h"
#include "rename_object.h"
#include "delete_object.h"
#include "prototype_object.h"
#include "paste_object.h"
#include "Mem.h"
#include "Archive.h"
#include "delete_recursive.h"
#include <local_search.h>

#include "fixoofs.h"

typedef unsigned long ulong;

// whether the current user has only READ-ONLY access

// my PID
pid_t my_pid;
// vlab's temporary directory
char *vlab_tmp_dir;
char curr_user_name[4096];
Permissions user_permissions;

// prototypes
static void serve_client(int sock);
static void fetch_file(const char *fname, MessagePipe &pipe);
static void get_realpath(const char *path, MessagePipe &pipe);
static void get_readlink(const char *path, MessagePipe &pipe);
static void get_dir(const char *fname, MessagePipe &pipe);
static void do_get_extensions(const char *obj_name, MessagePipe &pipe);
static void do_unlink(const char *fname, MessagePipe &pipe);
static void do_stat(const char *fname, MessagePipe &pipe);
static void do_symlink(const char *str, MessagePipe &pipe);
static void do_rename(const char *str, MessagePipe &pipe);
static void do_rename_object(const char *str, MessagePipe &pipe);
static void do_delete_object(const char *str, MessagePipe &pipe);
static void do_archive_object(const char *str, MessagePipe &pipe);
static void do_dearchive_object(const char *str, MessagePipe &pipe);
static void do_paste_object(const char *str, MessagePipe &pipe);
static void do_putfile(const char *str, const long n, MessagePipe &pipe);
static void do_copyfile(const char *str, MessagePipe &pipe);
static void do_compfile(const char *str, MessagePipe &pipe);
static void do_mkdir(const char *path, MessagePipe &pipe);
static void do_deltree(const char *path, MessagePipe &pipe);
static void do_prototype_object(const char *path, MessagePipe &pipe);
static void do_rmdir(const char *path, MessagePipe &pipe);
static void do_get_uuid(const Message &m, MessagePipe &pipe);
static void do_lookup_uuid(const Message &m, MessagePipe &pipe);
static void do_reconcile_uuids(const Message &m, MessagePipe &pipe);
static void do_fix_oofs(const Message &m, MessagePipe &pipe);
static void do_search_begin(const Message &m, MessagePipe &messagePipe);
static void do_search_continue(const Message &m, MessagePipe &messagePipe);
static void do_search_end(const Message &m, MessagePipe &messagePipe);

static void usage(const char *progname);
static void print_header(const char *str);
static void log(const char *fmt_str, ...);

char raserver_debug = 0;
static char log_on = 0;
static char *log_file = NULL;

int main(int argc, char **argv) {
  {
    const char *home = getenv("HOME");
    if (!home)
      home = "/tmp";
    std::string confdir(home);
    confdir += "/.vlab";
    setenv("VLABCONFIGDIR", confdir.c_str(), 0);
    setenv("VLABTMPDIR", "/tmp", 0);
  }
  /*-----------------------------.
  | check command line arguments |
  `-----------------------------*/
  raserver_debug = 0;
  log_on = 0;
  log_file = NULL;
  strcpy(curr_user_name, "<nobody>");
  int curr = 1;
  bool edit_passwords_flag = false;
  while (curr < argc) {
    if (xstrcmp(argv[curr], "-pe") == 0) {
      edit_passwords_flag = true;
      curr++;
      // by checking this later we can reduce the number of places we set the
      // password file name to 1 instead of setting the name in edit_passwords,
      // this also lets us use a single permissions object
    } else if (xstrcmp(argv[curr], "-d") == 0) {
      raserver_debug = 1;
      curr++;
      continue;
    } else if (xstrcmp(argv[curr], "-log") == 0 && curr + 1 < argc) {
      log_on = 1;
      curr++;
      log_file = xstrdup(argv[curr]);
      curr++;
      continue;
    } else if (xstrcmp(argv[curr], "-v") == 0) {
      fprintf(stdout, "raserver from VLAB \n");
      fprintf(stdout, "         protocol version 3.\n");
      exit(0);
    } else {
      usage(argv[0]);
      exit(-1);
    }
  }
  if (!edit_passwords_flag) {
    fprintf(stderr, "raserver: running.\n");
    if (raserver_debug)
      fprintf(stderr, "raserver: Debugging information ON.\n");
    if (log_on)
      fprintf(stderr, "raserver: Actions logged into file: %s\n", log_file);

    log("STARTED");
  }

  // get the value of 'VLABTMPDIR'
  vlab_tmp_dir = getenv("VLABTMPDIR");
  if (vlab_tmp_dir == NULL) {
    fprintf(stderr, "raserver: Please set your VLABTMPDIR.\n\n");
    log("QUIT: VLABTMPDIR not set.");
    exit(-1);
  }

  // make sure that password file exists and can be read
  std::string password_file = getenv("VLABCONFIGDIR");
  password_file.append("/rapasswords");
  if (user_permissions.loadPermissions(password_file, edit_passwords_flag)) {
    if (edit_passwords_flag) {
      edit_passwords();
      exit(0);
    } else {
      fprintf(stderr, "Password file '%s' doesn't seem to exist.\n",
              password_file.c_str());
      fprintf(stderr, "Please run raserver -pe to create one.\n");
      fprintf(stderr, "Exiting.\n");
      exit(0);
    }
  }
  if (edit_passwords_flag) {
    edit_passwords();
    exit(0);
  }

  // we don't want to have zombies
  signal(SIGCHLD, SIG_IGN);

  // create a socket on which we'll listen for clients
  int main_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (main_sock < 0) {
    perror("raserver:socket()");
    log("QUIT: socket() error: %s", strerror(errno));
    exit(-1);
  }

  // name the socket
  struct sockaddr_in server;
  bzero(&server, sizeof server);
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(RA_PORT);

  // set the 'REUSABILITY' option on the socket
  int count = 1;
  if (0 >
      setsockopt(main_sock, SOL_SOCKET, SO_REUSEADDR, &count, sizeof count)) {
    perror("raserver::setsockopt()");
    log("QUIT: setsockopt() error: %s", strerror(errno));
    exit(-2);
  }

  // bind the socket to the prepared address
  if (0 > bind(main_sock, (struct sockaddr *)&server, sizeof server)) {
    perror("raserver::bind()");
    log("QUIT: bind() error: %s", strerror(errno));
    exit(-3);
  }

  // mark the socket for listening only
  listen(main_sock, 20);

  // and now just accept new clients, forking off for each client
  while (1) {
    // accept new client
    int sock = accept(main_sock, NULL, NULL);

    // fork off a separate process that will server this client

    pid_t pid = fork();

    if (pid == 0) {
      // child will server the client
      // need to exec ...
      close(main_sock);
      serve_client(sock);
      exit(0);
    }

    close(sock);
  }
}

/******************************************************************************
 *
 * serves client connected to socket 'sock'
 *
 */

void serve_client(int sock) {
  // get my own pid
  my_pid = getpid();

  MessagePipe messagePipe(sock);

  // the first request must be a version request
  Message *version_request = messagePipe.get_first_message();
  if (version_request->code != RA_VERSION_REQUEST) {
    fprintf(stderr, "raserver:: Did not receive a version request.\n");
    fprintf(stderr,
            "raserver:: I think I am talking to an old client (pre 4.3)\n");
  } else {
    std::string version = "protocol 4.3";
    messagePipe.send_message(
        Message(RA_VERSION_RESPONSE, version.c_str(), version.length() + 1));
  }

  // the next request must be a login request
  Message *login_request;
  if (version_request->code != RA_VERSION_REQUEST) {
    // assuming we are talking to an old client, so the first reqeust should be
    // login request
    login_request = version_request;
  } else {
    // otherwise we are talking to a new client, let's get the login request
    login_request = messagePipe.get_first_message();
  }
  // make sure this is a login request, otherwise we have a protocol issue
  if (login_request->code != RA_LOGIN_REQUEST) {
    // the first message was not a login request, so we don't care
    // about this request - close down the connection
    if (raserver_debug)
      fprintf(stderr, "raserver:: Did not receive a LOGIN request.\n");
    close(sock);
    exit(0);
  }

  if (raserver_debug)
    fprintf(stderr, "Received a login request.\n");

  // the message is a login request: extract the user name and password
  long pos = 0;
  char *data = (char *)(login_request->data);
  while (data[pos] != ':')
    pos++;
  char *user_name = (char *)xmalloc(pos + 1);
  memcpy(user_name, data, pos);
  user_name[pos] = '\0';
  char *password = (char *)xmalloc(login_request->length - pos);
  memcpy(password, data + pos + 1, login_request->length - pos - 1);
  password[login_request->length - pos - 1] = '\0';

  log("Login '%s' requested.", user_name);

  if (raserver_debug)
    fprintf(stderr, "user_name = '%s' password = '%s'\n", user_name, password);

  // confirm login, and in case of a failure, return rejection
  // and cancel connection
  if (user_permissions.testLogin(user_name, password)) {
    Message response(RA_LOGIN_RESPONSE, "login denied",
                     strlen("login denied") + 1);
    log("Login '%s' denied.", user_name);
    messagePipe.send_message(response);
    close(sock);
    exit(0);
  }

  // save the user name for logging purposes
  strcpy(curr_user_name, user_name);

  log("LOGIN.");
  if (raserver_debug)
    fprintf(stderr, "raserver: user '%s'' logged in.\n", user_name);

  xfree(user_name);
  xfree(password);

  // login confirmed, send appropriate message
  Message response(RA_LOGIN_RESPONSE, "login confirmed",
                   strlen("login confirmed") + 1);
  messagePipe.send_message(response);

  // go into a loop accepting messages
  while (1) {
    if (raserver_debug)
      fprintf(stderr, "Waiting for message...\n");
    Message *m = messagePipe.get_first_message();
    if (m == NULL)
      break;
    if (raserver_debug)
      fprintf(stderr, "Processing message...\n");

    switch (m->code) {
    case RA_LOGIN_REQUEST:
      if (raserver_debug)
        fprintf(stderr, "raserver: WARNING: another LOGIN_REQUEST?!?\n"
                        "raserver:          ...ignoring\n");
      break;
    case RA_FETCH_FILE_REQUEST:
      if (raserver_debug)
        print_header("FETCH_FILE_REQUEST");
      fetch_file(m->data, messagePipe);
      break;
    case RA_REALPATH_REQUEST:
      if (raserver_debug)
        print_header("REALPATH_REQUEST");
      get_realpath(m->data, messagePipe);
      break;
    case RA_READLINK_REQUEST:
      if (raserver_debug)
        print_header("READLINK_REQUEST");
      get_readlink(m->data, messagePipe);
      break;
    case RA_GETDIR_REQUEST:
      if (raserver_debug)
        print_header("GETDIR_REQUEST");
      get_dir(m->data, messagePipe);
      break;
    case RA_GET_EXTENSIONS_REQUEST:
      if (raserver_debug)
        print_header("GET_EXTENSIONS_REQUEST");
      do_get_extensions(m->data, messagePipe);
      break;
    case RA_UNLINK_REQUEST:
      if (raserver_debug)
        print_header("UNLINK_REQUEST");
      do_unlink(m->data, messagePipe);
      break;
    case RA_DELTREE_REQUEST:
      if (raserver_debug)
        print_header("DELTREE_REQUEST");
      do_deltree(m->data, messagePipe);
      break;
    case RA_PROTOTYPE_OBJECT_REQUEST:
      if (raserver_debug)
        print_header("PROTOTYPE_REQUEST");
      do_prototype_object(m->data, messagePipe);
      break;
    case RA_STAT_REQUEST:
      if (raserver_debug)
        print_header("STAT_REQUEST");
      do_stat(m->data, messagePipe);
      break;
    case RA_SYMLINK_REQUEST:
      if (raserver_debug)
        print_header("SYMLINK_REQUEST");
      do_symlink(m->data, messagePipe);
      break;
    case RA_RENAME_REQUEST:
      if (raserver_debug)
        print_header("RENAME_REQUEST");
      do_rename(m->data, messagePipe);
      break;
    case RA_RENAME_OBJECT_REQUEST:
      if (raserver_debug)
        print_header("RENAME_OBJECT_REQUEST");
      do_rename_object(m->data, messagePipe);
      break;
    case RA_DELETE_OBJECT_REQUEST:
      if (raserver_debug)
        print_header("DELETE_OBJECT_REQUEST");
      do_delete_object(m->data, messagePipe);
      break;
    case RA_ARCHIVE_OBJECT_REQUEST:
      if (raserver_debug)
        print_header("ARCHIVE_OBJECT_REQUEST");
      do_archive_object(m->data, messagePipe);
      break;
    case RA_DEARCHIVE_OBJECT_REQUEST:
      if (raserver_debug)
        print_header("DEARCHIVE_OBJECT_REQUEST");
      do_dearchive_object(m->data, messagePipe);
      break;
    case RA_PASTE_OBJECT_REQUEST:
      if (raserver_debug)
        print_header("PASTE_OBJECT_REQUEST");
      do_paste_object(m->data, messagePipe);
      break;
    case RA_COMPFILE_REQUEST:
      if (raserver_debug)
        print_header("COMPFILE_REQUEST");
      do_compfile(m->data, messagePipe);
      break;
    case RA_COPYFILE_REQUEST:
      if (raserver_debug)
        print_header("COPYFILE_REQUEST");
      do_copyfile(m->data, messagePipe);
      break;
    case RA_PUTFILE_REQUEST:
      if (raserver_debug)
        print_header("PUTFILE_REQUEST");
      do_putfile(m->data, m->length, messagePipe);
      break;
    case RA_MKDIR_REQUEST:
      if (raserver_debug)
        print_header("MKDIR_REQUEST");
      do_mkdir(m->data, messagePipe);
      break;
    case RA_RMDIR_REQUEST:
      print_header("RMDIR_REQUEST");
      do_rmdir(m->data, messagePipe);
      break;
    case RA_LOGOUT_REQUEST:
      print_header("LOGOUT_REQUEST");
      messagePipe.send_message(Message(RA_LOGOUT_RESPONSE, "y", 2));
      exit(0);
      break;
    case RA_GET_UUID_REQUEST:
      print_header("GET_UUID_REQUEST");
      do_get_uuid(*m, messagePipe);
      break;
    case RA_LOOKUP_UUID_REQUEST:
      print_header("RA_LOOKUP_UUID_REQUEST");
      do_lookup_uuid(*m, messagePipe);
      break;
    case RA_RECONCILE_UUIDS_REQUEST:
      print_header("RA_RECONCILE_UUIDS_REQUEST");
      do_reconcile_uuids(*m, messagePipe);
      break;
    case RA_FIX_OOFS_REQUEST:
      print_header("RA_FIX_OOFS_REQUEST");
      do_fix_oofs(*m, messagePipe);
      break;
    case RA_SEARCH_BEGIN_REQUEST:
      print_header("RA_SEARCH_BEGIN_REQUEST");
      do_search_begin(*m, messagePipe);
      break;
    case RA_SEARCH_CONTINUE_REQUEST:
      print_header("RA_SEARCH_CONTINUE_REQUEST");
      do_search_continue(*m, messagePipe);
      break;
    case RA_SEARCH_END_REQUEST:
      print_header("RA_SEARCH_END_REQUEST");
      do_search_end(*m, messagePipe);
      break;
    default:
      std::cerr << "raserver: received a message with unknown request.\n"
                << "  Maybe you are using incompatible client?\n";
    }

    delete m;
  }

  close(sock);
  exit(0);
}

/******************************************************************************
 *
 * will send the requested file (fname) through the pipe
 *
 * - the first byte of the data in the message is 'y' if the file
 *   is sent
 * - if the file cannot be sent (i.e. cannot be open), the first
 *   character of the message is not 'y'
 *
 */

void fetch_file(const char *fname, MessagePipe &pipe) {
  if (raserver_debug)
    fprintf(stderr, "raserver:fetch_file( %s)\n", fname);

  Message m(RA_FETCH_FILE_RESPONSE, "n", 1);

  if (!user_permissions.TestPermissions(curr_user_name, fname, "r")) {
    if (raserver_debug)
      fprintf(stderr,
              "User %s trying to read file %s without read permissions!\n",
              curr_user_name, fname);
    // simulate a file open failure
    pipe.send_message(m);
    return;
  }

  // open the file for reading
  int fd = open(fname, O_RDONLY);
  if (fd == -1) {
    // open failed - send message with a failure
    pipe.send_message(m);
    return;
  }

  long alloc = 16384;
  char *buf = (char *)xmalloc(alloc);
  buf[0] = 'y';
  long size = 1;

  // read the entire file into memory
  while (1) {

    int l = read(fd, buf + size, alloc - size);
    if (l <= 0)
      break;
    size += l;
    if (alloc == size) {
      alloc += 16384;
      buf = (char *)xrealloc(buf, alloc);
    }
  }
  // close the file
  close(fd);

  if (raserver_debug)
    fprintf(stderr, "raserver:fetch_file() sending %ld bytes.\n", size);

  // send the file
  xfree(m.data);
  m.data = buf;
  m.length = size;
  pipe.send_message(m);
  m.data = NULL;

  // free the buffer
  xfree(buf);
}

/******************************************************************************
 *
 * will send a directory list to the client
 *
 * - the first byte of the response is 'y' if the list is valide
 *
 * - otherwise the list is invalid (i.e. directory could not be read)
 *
 * - the sent data has the form:
 *
 *      'y'"entry1"'\0'"entry2"'\0'"entry3"'\0'...."entry4"'\0''\0'
 *
 */

void get_dir(const char *dir_name, MessagePipe &pipe) {
  Message m(RA_GETDIR_RESPONSE, "n", 2);

  if (raserver_debug)
    fprintf(stderr, "raserver:get_dir( '%s')\n", dir_name);

  if (!user_permissions.TestPermissions(curr_user_name, dir_name, "r")) {
    if (raserver_debug)
      fprintf(stderr,
              "User %s trying to list directory %s without read permissions!\n",
              curr_user_name, dir_name);
    // If the user does not have permissions return an empty directory listing
    // to them
    Message empty(RA_GETDIR_RESPONSE, "y\0", 2);
    pipe.send_message(empty);
    return;
  }

  // retrieve the directory list
  char **list;
  int n = dirList(dir_name, &list);
  if (n < 0) {
    // report error
    pipe.send_message(m);
    return;
  }

  // compute the length of the data response
  int length = 2; // initial 'y' and end '\0'
  int i;
  for (i = 0; i < n; i++)
    length += xstrlen(list[i]) + 1;

  // prepare the data
  xfree(m.data);
  m.data = (char *)xmalloc(length);
  m.length = length;

  m.data[0] = 'y'; // the response is valid
  char *ptr = m.data + 1;
  for (i = 0; i < n; i++) // put each entry into data
  {
    int l = strlen(list[i]) + 1;
    memcpy(ptr, list[i], l);
    ptr += l;
  }
  m.data[length - 1] = '\0'; // append extra '\0' at the end

  // send the response
  pipe.send_message(m);
}

/******************************************************************************
 *
 * will send a list of extensions
 *
 * - the first byte of the response is 'y' if the list is valide
 *
 * - otherwise the list is invalid (i.e. directory could not be read)
 *
 * - the sent data has the form:
 *
 *      'y'"entry1""entry2""entry3"...."entry4"'\0'
 *
 *   where entry has the format:
 *
 *      name + real_path + 1 byte of flatg ( 01 = DIR, 02 = LINK)
 *
 */

void do_get_extensions(const char *obj_name, MessagePipe &pipe) {
  Message m(RA_GET_EXTENSIONS_RESPONSE, "n", 2);

  debug_printf("raserver:get_extensions( '%s')\n", obj_name);

  if (!user_permissions.TestPermissions(curr_user_name, obj_name, "r")) {
    if (raserver_debug)
      fprintf(stderr,
              "User %s trying to get extensions on object %s without read "
              "permissions!\n",
              curr_user_name, obj_name);
    Message empty(RA_GET_EXTENSIONS_RESPONSE, "y\0", 2);
    pipe.send_message(empty);
    return;
  }

  // retrieve the list of extensions
  char **list;
  int n = get_extensions(obj_name, list);
  if (n < 0) {
    // report error
    pipe.send_message(m);
    return;
  }

  // compute the length of the data response
  int length = 2; // initial 'y' + end '\0'
  int i;
  for (i = 0; i < n; i++) {
    int len1 = xstrlen(list[i]) + 1;
    int len2 = xstrlen(list[i] + len1) + 1;
    length += len1 + len2 + 1;
  }

  // prepare the data
  xfree(m.data);
  m.data = (char *)xmalloc(length);
  m.length = length;

  m.data[0] = 'y'; // the response is valid
  char *ptr = m.data + 1;
  for (i = 0; i < n; i++) // put each entry into data
  {
    int len1 = xstrlen(list[i]) + 1;
    int len2 = xstrlen(list[i] + len1) + 1;
    int len = len1 + len2 + 1;
    memcpy(ptr, list[i], len);
    ptr += len;
  }
  m.data[length - 1] = '\0'; // append an extra '\0' at the end

  // send the response
  pipe.send_message(m);
}

/******************************************************************************
 *
 * unlink a file and send the result to the client
 *
 */

void do_unlink(const char *fname, MessagePipe &pipe) {
  if (!user_permissions.TestPermissions(curr_user_name, fname, "rw")) {
    if (raserver_debug)
      fprintf(stderr,
              "User %s trying to unlink file %s without read + write "
              "permissions!\n",
              curr_user_name, fname);
    Message m(RA_UNLINK_RESPONSE, "n", 2);
    pipe.send_message(m);
    return;
  }


  // delete the file
  if (unlink(fname)) {
    if (raserver_debug)
      perror("raserver:do_unlink():unlink()");

    // send a FAILURE message to the client
    Message m(RA_UNLINK_RESPONSE, "n", 2);
    pipe.send_message(m);
  } else {
    // send a message to the client - SUCCESS
    Message m(RA_UNLINK_RESPONSE, "y", 2);
    pipe.send_message(m);
  }
}

/******************************************************************************
 *
 * remove (recursively) a directory and send the status to the
 * client ( "y" = success, "n" = failure)
 *
 */

void do_deltree(const char *path, MessagePipe &pipe) {
 
  if (!user_permissions.TestPermissions(curr_user_name, path, "rw")) {
    if (raserver_debug)
      fprintf(stderr,
              "User %s trying to recursively delete %s without read + write "
              "permissions!\n",
              curr_user_name, path);
    Message m(RA_DELTREE_RESPONSE, "n", 2);
    pipe.send_message(m);
    return;
  }
  // attempt the deletion
  int result = delete_recursive(path);

  // check the result
  if (result != 0) {
    if (raserver_debug)
      perror("raserver:do_deltree():unlink()");

    // send a FAILURE message to the client
    Message m(RA_DELTREE_RESPONSE, "n", 2);
    pipe.send_message(m);
  } else {
    // send a message to the client - SUCCESS
    Message m(RA_DELTREE_RESPONSE, "y", 2);
    pipe.send_message(m);
  }
}

/******************************************************************************
 *
 * prototype a vlab object
 *
 * returns "y" = success, "n" = failure
 *
 */

void do_prototype_object(const char *path, MessagePipe &pipe) {
  if (raserver_debug)
    fprintf(stderr, "do_prototype_object( %s)\n", path);

  if (!user_permissions.TestPermissions(curr_user_name, path, "rw")) {
    if (raserver_debug)
      fprintf(stderr,
              "User %s trying to prototype object %s without read + write "
              "permissions!\n",
              curr_user_name, path);
    Message m(RA_DELTREE_RESPONSE, "n", 2);
    pipe.send_message(m);
    return;
  }

  // call prototype_object
  if (prototype_object(path)) {
    if (raserver_debug)
      fprintf(stderr, "prototype failed.");

    // send a FAILURE message to the client
    Message m(RA_PROTOTYPE_OBJECT_RESPONSE, "n", 2);
    pipe.send_message(m);
  } else {
    // send a message to the client - SUCCESS
    Message m(RA_PROTOTYPE_OBJECT_RESPONSE, "y", 2);
    pipe.send_message(m);
  }
}

/******************************************************************************
 *
 * find out statistics about a file and send it to the client
 *
 * - needs to find out:
 *
 *    is the file DIRECTORY or REGULAR FILE
 *    is the file a LINK or not
 *    is the file readable to the user
 *    is the file writeable to the user
 *    is the file executable to the user
 *
 */

void do_stat(const char *fname, MessagePipe &pipe) {
  if (raserver_debug)
    fprintf(stderr, "do_stat( '%s')\n", fname);

  Message m(RA_STAT_RESPONSE, "n----", xstrlen("n----"));
  if (!user_permissions.TestPermissions(curr_user_name, fname, "r")) {
    if (raserver_debug)
      fprintf(stderr,
              "User %s trying to stat file %s without read permissions!\n",
              curr_user_name, fname);
    pipe.send_message(m);
    return;
  }

  // does the file 'fname' exist?
  struct stat buff;
  if (stat(fname, &buff) != 0) {
    pipe.send_message(m);
    return;
  }

  // is 'fname' directory ?
  if (S_ISDIR(buff.st_mode))
    m.data[0] = 'd';
  else if (S_ISREG(buff.st_mode))
    m.data[0] = 'f';
  else
    m.data[0] = 'o';

  // is 'fname' readable ?
  if (!access(fname, R_OK))
    m.data[2] = 'r';

  // is 'fname' writable ? (only for users with write access)
  if (!access(fname, W_OK) &&
      user_permissions.TestPermissions(curr_user_name, fname, "w"))
    m.data[3] = 'w';

  // is 'fname' executable ?
  if (!access(fname, X_OK))
    m.data[4] = 'x';

  // is 'fname' a link ?
  if (lstat(fname, &buff) == 0) {
    if (S_ISLNK(buff.st_mode))
      m.data[1] = 'l';
  }

  pipe.send_message(m);
}

/******************************************************************************
 *
 * perform a symlink(2) operation on the files in 'str'
 *
 * - send a message to the client with the result
 *   'y' = success
 *   'n' = failure
 *
 */

void do_symlink(const char *str, MessagePipe &pipe) {
  const char *src = str;
  const char *dst = str + xstrlen(src) + 1;
  if ((!user_permissions.TestPermissions(curr_user_name, src, "r")) ||
      (!user_permissions.TestPermissions(curr_user_name, dst, "w"))) {
    if (raserver_debug)
      fprintf(
          stderr,
          "User %s trying to symlink %s to %s without proper permissions!\n",
          curr_user_name, src, dst);
    Message m(RA_SYMLINK_RESPONSE, "n", 2);
    pipe.send_message(m);
    return;
  }

  if (raserver_debug)
    fprintf(stderr, "raserver:do_symlink( %s, %s)\n", src, dst);

  if (symlink(src, dst)) {
    if (raserver_debug)
      perror("raserver:do_symlink():symlink()");

    // report failure
    Message m(RA_SYMLINK_RESPONSE, "n", 2);
    pipe.send_message(m);
  } else {
    // report success
    Message m(RA_SYMLINK_RESPONSE, "y", 2);
    pipe.send_message(m);
  }
}

/******************************************************************************
 *
 * perform a rename(2) operation on the files in 'str'
 *
 * - send a message to the client with the result
 *   'y' = success
 *   'n' = failure
 *
 */

void do_rename(const char *str, MessagePipe &pipe) {
 
  const char *src = str;
  const char *dst = str + xstrlen(src) + 1;
  if ((!user_permissions.TestPermissions(curr_user_name, src, "rw")) ||
      (!user_permissions.TestPermissions(curr_user_name, dst, "w"))) {
    if (raserver_debug)
      fprintf(stderr,
              "User %s trying to rename %s to %s without proper permissions!\n",
              curr_user_name, src, dst);
    Message m(RA_RENAME_RESPONSE, "n", 2);
    pipe.send_message(m);
    return;
  }
  if (raserver_debug)
    fprintf(stderr, "raserver:do_rename( %s, %s)\n", src, dst);

  if (rename(src, dst)) {
    if (raserver_debug)
      perror("raserver:do_rename():rename()");

    // report failure
    Message m(RA_RENAME_RESPONSE, "n", 2);
    pipe.send_message(m);
  } else {
    // report success
    Message m(RA_RENAME_RESPONSE, "y", 2);
    pipe.send_message(m);
  }
}
/******************************************************************************
 *
 * rename an object
 *
 * - send a message to the client with the result
 *   'y' = success
 *   'n' = failure
 *
 */

void do_rename_object(const char *str, MessagePipe &pipe) {
  char *ptr = (char *)str;
  char *oofs_dir = ptr;
  ptr += xstrlen(ptr) + 1;
  char *src_fname = ptr;
  ptr += xstrlen(ptr) + 1;
  char *dst_fname = ptr;
  ptr += xstrlen(ptr) + 1;

  if ((!user_permissions.TestPermissions(curr_user_name, src_fname, "rw")) ||
      (!user_permissions.TestPermissions(curr_user_name, dst_fname, "w"))) {
    if (raserver_debug)
      fprintf(stderr,
              "User %s trying to rename %s to %s without proper permissions!\n",
              curr_user_name, src_fname, dst_fname);
    Message m(RA_RENAME_OBJECT_RESPONSE, "n", 2);
    pipe.send_message(m);
    return;
  }

  if (raserver_debug)
    fprintf(stderr, "raserver:do_rename_object( %s, %s, %s)\n", oofs_dir,
            src_fname, dst_fname);

  int res = rename_object(oofs_dir, src_fname, dst_fname);
  if (res) {
    if (raserver_debug) {
      switch (res) {
      case 1:
        debug_printf("raserver: invalid parameters passed "
                     "to rename_object()\n");
        break;
      case 2:
        debug_printf("raserver: rename() failed: ");
        perror("");
        break;
      }
    }

    // report failure
    Message m(RA_RENAME_OBJECT_RESPONSE, "n", 2);
    pipe.send_message(m);
    return;
  }

  // report success
  Message m(RA_RENAME_OBJECT_RESPONSE, "y", 2);
  pipe.send_message(m);
}

/******************************************************************************
 *
 * delete an object
 *
 * - send a message to the client with the result
 *   'y' = success
 *   'n' = failure
 *
 */

void do_delete_object(const char *str, MessagePipe &pipe) {
 
  char *ptr = (char *)str;
  char *oofs_dir = ptr;
  ptr += xstrlen(ptr) + 1;
  char *fname = ptr;
  ptr += xstrlen(ptr) + 1;
  if (!user_permissions.TestPermissions(curr_user_name, fname, "w")) {
    if (raserver_debug)
      fprintf(stderr,
              "User %s trying to delete %s without proper permissions!\n",
              curr_user_name, fname);
    Message m(RA_DELETE_OBJECT_RESPONSE, "n", 2);
    pipe.send_message(m);
    return;
  }

  if (raserver_debug)
    fprintf(stderr, "raserver:do_delete_object( %s, %s)\n", oofs_dir, fname);

  int res = delete_object(oofs_dir, fname);
  if (res) {
    if (raserver_debug) {
      switch (res) {
      case 1:
        debug_printf("raserver: invalid parameters passed "
                     "to delete_object()\n");
        break;
      case 2:
        debug_printf("raserver: delete_recursive() failed: ");
        perror("");
        break;
      case 3:
        debug_printf("raserver: .dbase could not be initialized.\n");
        break;
      case 4:
        debug_printf("raserver: .dbase could not be updated.\n");
        break;
      }
    }

    // report failure
    Message m(RA_DELETE_OBJECT_RESPONSE, "n", 2);
    pipe.send_message(m);
    return;
  }

  // report success
  Message m(RA_DELETE_OBJECT_RESPONSE, "y", 2);
  pipe.send_message(m);
}

/******************************************************************************
 *
 * archive an object
 *
 * - send a message to the client with the result
 *   'y' = success
 *   'n' = failure
 *   + the archive appended if success
 *
 */

void do_archive_object(const char *str, MessagePipe &pipe) {
  char *ptr = (char *)str;
  char *oofs_dir = ptr;
  ptr += xstrlen(ptr) + 1;
  char *fname = ptr;
  ptr += xstrlen(ptr) + 1;
  int recursive = (*ptr == '1');

  if (raserver_debug)
    fprintf(stderr, "raserver:do_archive_object( %s, %s, %s)\n", oofs_dir,
            fname, recursive ? "recursive" : "single");

  if (!user_permissions.TestPermissions(curr_user_name, fname, "r")) {
    if (raserver_debug)
      fprintf(stderr,
              "User %s trying to archive %s without proper permissions!\n",
              curr_user_name, fname);
    Message m(RA_ARCHIVE_OBJECT_RESPONSE, "n", 2);
    pipe.send_message(m);
    return;
  }

  // create a temporary file
  char tmp_file_name[4096];
  sprintf(tmp_file_name, "%s/ratmp%ld-XXXXXX", vlab_tmp_dir, (long)my_pid);
  int res = mkstemp(tmp_file_name);
  if (DEBUG)
    fprintf(stderr, "tmp_file_name = '%s'\n", tmp_file_name);

  // archive the requested object
  Archive ar;
  ar.set_destination(Archive::AR_FILE, tmp_file_name);

  res = ar.pack(fname, recursive);
  if (res) {
    fprintf(stderr, "raserver: archive failed because:\n%s\n", ar.get_errors());

    // report failure - format:
    // "n" + the error message(s)
    Mem mem;
    mem.append("n", 2);
    mem.append("FROM RASERVER\n", xstrlen("FROM RASERVER\n"));
    mem.append(ar.get_errors(), xstrlen(ar.get_errors()) + 1);
    Message m(RA_ARCHIVE_OBJECT_RESPONSE, (char *)mem.data, mem.size);
    pipe.send_message(m);
    return;
  }

  // get the size of the archive
  long archive_size = ar.get_archive_size();
  if (DEBUG)
    fprintf(stderr, "archive_size = %ld\n", archive_size);
  assert(archive_size >= 0);
  ulong n_archive_size = htonl(archive_size);

  // report success - message format: 'y' + archive_size (in network format)
  char message[4096];
  message[0] = 'y';
  memcpy(message + 1, &n_archive_size, sizeof(n_archive_size));
  Message m(RA_ARCHIVE_OBJECT_RESPONSE, message, 1 + sizeof(n_archive_size));
  pipe.send_message(m);

  // now send the local archive over the socket to the client, in chunks
  // of up to 16Kb
  int fd = open(tmp_file_name, O_RDONLY);
  if (fd == -1) {
    fprintf(stderr,
            "raserver: do_archive_object(): cannot read %s\n"
            "          because %s\n",
            tmp_file_name, strerror(errno));
    fprintf(stderr, "          Sending empty file (ZEROs only)\n");

    // send blocks of ZEROs to the client
    long remain = archive_size;
    while (remain > 0) {
      char buff[16384];
      memset(buff, 0, sizeof(buff));
      long len = sizeof(buff);
      if (remain < len)
        len = remain;
      Message m(RA_ARCHIVE_BLOCK, buff, sizeof(buff));
      pipe.send_message(m);
      remain -= len;
    }
  }

  // read a block and send it, until the entire archive is sent
  long n_read = 0;
  while (1) {
    // read a block from the file
    char buff[16384];
    long n = read(fd, buff, sizeof(buff));

    // check the terminating condition
    if (n <= 0)
      break;
    n_read += n;

    // send a message with this block
    Message m(RA_ARCHIVE_BLOCK, buff, n);
    pipe.send_message(m);
  }

  // close the temporary file
  close(fd);

  // remove the temporary file
  unlink(tmp_file_name);

  // make sure that the correct number of bytes was sent
  if (n_read != archive_size) {
    fprintf(stderr,
            "raserver: do_archive_object(): archive size is %ld but\n"
            "          %ld bytes were sent.\n",
            archive_size, n_read);
  }
}

void do_dearchive_object(const char *str, MessagePipe &pipe)
/*-----------------------------------------------.
| dearchive an archive into a destination        |
|                                                |
| - send a message to the client with the result |
|                                                |
|   'y' = success                                |
|                                                |
|   'n' = failure                                |
`-----------------------------------------------*/
{
  // extract the oofs, destination and archive length from the message
  char *ptr = (char *)str;
  char *oofs_dir = ptr;
  ptr += xstrlen(ptr) + 1;
  char *destination = ptr;
  ptr += xstrlen(ptr) + 1;
  ulong n_archive_size;
  if (!user_permissions.TestPermissions(curr_user_name, destination, "w")) {
    if (raserver_debug)
      fprintf(stderr,
              "User %s trying to dearchive into path %s without proper "
              "permissions!\n",
              curr_user_name, destination);
    Message m(RA_DEARCHIVE_OBJECT_RESPONSE, "n", 2);
    pipe.send_message(m);
    return;
  }

  memcpy(&n_archive_size, ptr, sizeof(n_archive_size));
  ulong archive_size = ntohl(n_archive_size);

  if (raserver_debug)
    fprintf(stderr, "raserver:do_dearchive_object( %s, %s, %lu)\n", oofs_dir,
            destination, archive_size);

  // create a temporary file where the archive will be stored
  char tmp_file_name[4096];
  sprintf(tmp_file_name, "%s/ratmp%ld-XXXXXX", vlab_tmp_dir, (long)my_pid);
  int res = mkstemp(tmp_file_name);
  if (DEBUG)
    fprintf(stderr, "tmp_file_name = '%s' mkstemp()=%d\n", tmp_file_name, res);

  // open the temporary file
  int fd = open(tmp_file_name, O_WRONLY | O_CREAT | O_TRUNC,
                S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
  if (fd == -1) {
    fprintf(stderr,
            "raserver:do_dearchive_object(): could not create "
            "a local copy of the\n"
            "                                archive,"
            " because open(2) failed.\n"
            "         reason: %s\n",
            strerror(errno));
    Message m(RA_DEARCHIVE_OBJECT_RESPONSE, "n", 2);
    pipe.send_message(m);
    return;
  }

  // received the archive over the network in chunks
  unsigned long n_read = 0;
  while (n_read < archive_size) {
    // receive the next ARCHIVE_BLOCK message
    Message *bl = pipe.get_message(RA_ARCHIVE_BLOCK);
    if (bl == NULL) {
      fprintf(stderr, "raserver: do_dearchive_object(): "
                      "get_message() failed.\n");
      Message m(RA_DEARCHIVE_OBJECT_RESPONSE, "n", 2);
      pipe.send_message(m);
      return;
    }

    // write the block to the file
    if (ulong(bl->length) > 0 &&
        ulong(bl->length) <= ulong(archive_size - n_read)) {
      if (bl->length != write(fd, bl->data, bl->length)) {
        fprintf(stderr,
                "raserver:do_dearchive_object(): could not create "
                "a local copy of the\n"
                "                                archive,"
                " because write(2) failed.\n"
                "         reason: %s\n",
                strerror(errno));
        Message m(RA_DEARCHIVE_OBJECT_RESPONSE, "n", 2);
        pipe.send_message(m);
        return;
      }

      n_read += bl->length;
    } else {
      fprintf(stderr, "raserver:do_dearchive_object(): invalid "
                      "message size.\n");
      Message m(RA_DEARCHIVE_OBJECT_RESPONSE, "n", 2);
      pipe.send_message(m);
      return;
    }
  }

  // close the temporary file
  close(fd);

  // unpack the archive
  Archive ar;
  if (ar.unpack(tmp_file_name, destination)) {
    // unpack failed - report error
    fprintf(stderr,
            "raserver:do_dearchive_object(): dearchive failed:\n"
            "%s\n",
            ar.get_errors());
    Message m(RA_DEARCHIVE_OBJECT_RESPONSE, "n", 2);
    pipe.send_message(m);

    // remote the local copy of the archive
    unlink(tmp_file_name);
    return;
  }

  // remove the temporary file
  unlink(tmp_file_name);

  // send a SUCCESS response back
  Message m(RA_DEARCHIVE_OBJECT_RESPONSE, "y", 2);
  pipe.send_message(m);
}

/******************************************************************************
 *
 * copy file1 to file2 (file1 and file2 are stored in 'str')
 *
 * - send result of this operation to client
 *    'y' = success
 *    'n' = failure
 *
 */

void do_copyfile(const char *str, MessagePipe &pipe) {
  const char *src = str;
  const char *dst = str + xstrlen(src) + 1;

  if (raserver_debug)
    fprintf(stderr, "raserver:do_copyfile( %s, %s)\n", src, dst);
  if ((!user_permissions.TestPermissions(curr_user_name, src, "r")) ||
      (!user_permissions.TestPermissions(curr_user_name, dst, "w"))) {
    if (raserver_debug)
      fprintf(stderr,
              "User %s trying to copy file from %s to %s without proper "
              "permissions!\n",
              curr_user_name, src, dst);
    Message m(RA_COPYFILE_RESPONSE, "n", 2);
    pipe.send_message(m);
    return;
  }
  if (cpfile(src, dst)) {
    if (raserver_debug)
      perror("raserver:do_copyfile():copyfile()");

    // report failure
    Message m(RA_COPYFILE_RESPONSE, "n", 2);
    pipe.send_message(m);
  } else {
    // report success
    Message m(RA_COPYFILE_RESPONSE, "y", 2);
    pipe.send_message(m);
  }
}

/******************************************************************************
 *
 * compare file1 with file2 (file1 and file2 are stored in 'str')
 *
 * - send result of this operation to client
 *    'y' = success
 *    'n' = failure
 *
 */

void do_compfile(const char *str, MessagePipe &pipe) {
  const char *src = str;
  const char *dst = str + xstrlen(src) + 1;

  if (raserver_debug)
    fprintf(stderr, "raserver:do_compfile( %s, %s)\n", src, dst);
  if ((!user_permissions.TestPermissions(curr_user_name, src, "r")) ||
      (!user_permissions.TestPermissions(curr_user_name, dst, "r"))) {
    if (raserver_debug)
      fprintf(stderr,
              "User %s trying to compare file %s to %s without proper "
              "permissions!\n",
              curr_user_name, src, dst);
    Message m(RA_COMPFILE_RESPONSE, "n", 2);
    pipe.send_message(m);
    return;
  }
  int result = cmpfiles(src, dst);
  if (raserver_debug)
    fprintf(stderr, "result=%d\n", result);
  if (result < 0) {
    if (raserver_debug)
      perror("raserver:do_compfile():compfile()");

    // report error
    Message m(RA_COMPFILE_RESPONSE, "e", 2);
    pipe.send_message(m);
  } else if (result == 0) {
    // report success
    Message m(RA_COMPFILE_RESPONSE, "y", 2);
    pipe.send_message(m);
  } else {
    // report failure - files not the same
    Message m(RA_COMPFILE_RESPONSE, "n", 2);
    pipe.send_message(m);
  }
}

/******************************************************************************
 *
 * put the content of the incoming message into a file 'fname'
 * 'fname' is the part of the message, ended by a '\0'
 *
 * - send the result of the operation to the client
 *   'y' = success
 *   'n' = failure
 *
 */

void do_putfile(const char *str, const long length, MessagePipe &pipe) {
  Message m(RA_PUTFILE_RESPONSE, "n", 2);

  // extract the filename
  const char *fname = str;

  if (!user_permissions.TestPermissions(curr_user_name, fname, "w")) {
    if (raserver_debug)
      fprintf(stderr,
              "User %s trying to put file %s without proper permissions!\n",
              curr_user_name, fname);
    pipe.send_message(m);
    return;
  }

  // extract the data
  const char *data = str + xstrlen(fname) + 1;
  long n = length - xstrlen(fname) - 1;

  // in case the output file is a link, unlink it, otherwise
  // the open() would destroy the file this links points to
  unlink(fname);

  // open file for writing - delete existing file
  int fd = open(fname, O_WRONLY | O_CREAT | O_TRUNC,
                S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
  if (fd == -1) {
    // could not open the destination file
    pipe.send_message(m);
    perror("raserver:do_putfile():open()");
    return;
  }

  // write the data to the destination file
  if (n != write(fd, data, n)) {
    // could not write to the file (at least not the entire message
    pipe.send_message(m);
    perror("raserver:do_putfile():write()");
    return;
  }

  // close the file
  close(fd);

  // send a 'success' message to the client
  Message m2(RA_PUTFILE_RESPONSE, "y", 2);
  pipe.send_message(m2);
}

/******************************************************************************
 *
 * will attemp create a directory 'path'
 *
 * - result of this attempt will be sent to the client 'y' = success
 *   'n' = failure
 *
 */

void do_mkdir(const char *path, MessagePipe &pipe) {
  if (!user_permissions.TestPermissions(curr_user_name, path, "w")) {
    if (raserver_debug)
      fprintf(
          stderr,
          "User %s trying to create directory %s without proper permissions!\n",
          curr_user_name, path);
    Message m(RA_MKDIR_RESPONSE, "n", 2);
    pipe.send_message(m);
    return;
  }

  if (!mkdir(path, 0755)) {
    Message success(RA_MKDIR_RESPONSE, "y", 2);
    pipe.send_message(success);
  } else {
    Message failure(RA_MKDIR_RESPONSE, "n", 2);
    pipe.send_message(failure);
  }
}

/******************************************************************************
 *
 * will attemp remove a directory 'path'
 *
 * - result of this attempt will be sent to the client 'y' = success
 *   'n' = failure
 *
 */

void do_rmdir(const char *path, MessagePipe &pipe) {
  if (!user_permissions.TestPermissions(curr_user_name, path, "w")) {
    if (raserver_debug)
      fprintf(
          stderr,
          "User %s trying to remove directory %s without proper permissions!\n",
          curr_user_name, path);
    Message m(RA_RMDIR_RESPONSE, "n", 2);
    pipe.send_message(m);
    return;
  }

  if (!rmdir(path)) {
    Message success(RA_RMDIR_RESPONSE, "y", 2);
    pipe.send_message(success);
  } else {
    Message failure(RA_RMDIR_RESPONSE, "n", 2);
    pipe.send_message(failure);
  }
}

/******************************************************************************
 *
 * will get realpath() of a 'path'
 *
 * - client will receive either:
 *
 *       'y' + realpath (in case of success)
 *
 *   or
 *
 *       "n"
 *
 */


// [Pascal] I am removing the use of Qt which is unsafe in Mac Os X
static void get_realpath(const char *path, MessagePipe &pipe) {
  char tmp[4096];
  tmp[0] = 'y';
  if (NULL == realpath(path, tmp)) {
    std::cerr << "realPath failure" << std::endl;
    Message failure(RA_REALPATH_RESPONSE, "n", 2);
    pipe.send_message(failure);
  } else {
    std::string tmp_path;
    tmp_path = "y" + std::string(tmp);
    Message success(RA_REALPATH_RESPONSE, tmp_path.c_str(),
                    xstrlen(tmp_path.c_str()) + 1);
    pipe.send_message(success);
  }
}


/******************************************************************************
 *
 * will get readlink() of a 'path'
 *
 * - client will receive either:
 *
 *       'y' + readlink (in case of success)
 *
 *   or
 *
 *       "n"
 *
 */

void get_readlink(const char *path, MessagePipe &pipe) {
  char tmp[4096];
  tmp[0] = 'y';

  int length = readlink(path, tmp + 1, sizeof(tmp) - 1);
  if (length <= 0) {
    Message failure(RA_READLINK_RESPONSE, "n", 2);
    pipe.send_message(failure);
  } else {
    tmp[length + 1] = '\0';
    Message success(RA_READLINK_RESPONSE, tmp, xstrlen(tmp) + 1);
    pipe.send_message(success);
  }
}

/******************************************************************************
 *
 * show the usage
 *
 */

void usage(const char *) {
  fprintf(stderr, "Usage: raserver -pe \n"
                  "       raserver [-d] [-log logfile]\n"
                  "\n"
                  "       -pe              invokes the password file editor\n"
                  "       -d               extra debugging output on\n"
                  "       -log log_file    logs user's action into a log file\n"
                  "       -v               print vlab version\n"
                  "\n");
}

void print_header(const char *str)
/*---------------------------------------------------------------.
| prints a header of a received message to standard error output |
`---------------------------------------------------------------*/
{
  if (!raserver_debug)
    return;
  // calculate the lenght of the string
  size_t str_len = strlen(str);

  // print dashes on the left and right so that the total
  // line length is always the same (LINE_LENGTH)

  const size_t LINE_LEN = 78;

  size_t i;
  // print the left dashes
  // size_t left_side = (LINE_LEN - str_len - 2) / 2 ;
  size_t left_side = 5;
  for (i = 0; i < left_side; i++)
    fprintf(stderr, "-");

  // print the center
  fprintf(stderr, " %s ", str);

  // print the right side
  size_t right_side = (LINE_LEN - str_len - 2 - left_side);
  for (i = 0; i < right_side; i++)
    fprintf(stderr, "-");
  fprintf(stderr, "\n");
}

void do_paste_object(const char *str, MessagePipe &pipe)
/*-----------------------------------------------.
| paste an archive into a destination            |
|                                                |
| - send a message to the client with the result |
|                                                |
|   'y' = success                                |
|   'n' = failure                                |
|                                                |
`-----------------------------------------------*/
{

  // extract the oofs_dir, dst_fname, old_path, move_links and n_archive_size
  char *ptr = (char *)str;
  char *oofs_dir = ptr;
  ptr += xstrlen(ptr) + 1;
  char *dst_fname = ptr;
  ptr += xstrlen(ptr) + 1;
  char *old_path = ptr;
  ptr += xstrlen(ptr) + 1;
  char move_links = *ptr;
  ptr += sizeof(char);
  if (!user_permissions.TestPermissions(curr_user_name, dst_fname, "w")) {
    if (raserver_debug)
      fprintf(stderr,
              "User %s trying to paste object into directory %s without proper "
              "permissions!\n",
              curr_user_name, dst_fname);
    // send a response indicating an RAserver problem
    Mem mem;
    mem.append("n", 1);
    // -1 = write access denied
    int res = -1;
    u_long n_res = htonl(u_long(res + 128));
    mem.append(&n_res, sizeof(n_res));
    Message m(RA_PASTE_OBJECT_RESPONSE, (char *)mem.data, mem.size);
    pipe.send_message(m);
    return;
  }

  ulong n_archive_size;
  memcpy(&n_archive_size, ptr, sizeof(n_archive_size));

  ulong archive_size = ntohl(n_archive_size);

  if (raserver_debug)
    fprintf(stderr, "raserver:do_paste_object(%s,%s,%s,%s,%lu)\n", oofs_dir,
            dst_fname, old_path, move_links ? "move_links" : "keep_links",
            archive_size);

  // create a temporary file where the archive will be stored
  char archive_name[4096];
  sprintf(archive_name, "%s/ratmp%ld-XXXXXX", vlab_tmp_dir, (long)my_pid);
  int res = mkstemp(archive_name);
  if (DEBUG)
    fprintf(stderr, "archive_name = '%s'\n", archive_name);

  // open the temporary file
  int fd = open(archive_name, O_WRONLY | O_CREAT | O_TRUNC,
                S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
  if (fd == -1) {
    fprintf(stderr,
            "raserver:do_paste_object(): could not create a local copy \n"
            "                            of the archive, because \n"
            "                            open(2) failed. Reason:\n"
            "\n"
            "           %s\n",
            strerror(errno));

    // send a response indicating an RAserver problem
    Mem mem;
    mem.append("n", 1);
    int res = -5;
    u_long n_res = htonl(u_long(res + 128));
    mem.append(&n_res, sizeof(n_res));
    Message m(RA_PASTE_OBJECT_RESPONSE, (char *)mem.data, mem.size);
    pipe.send_message(m);
    return;
  }

  // received the archive over the network in chunks
  unsigned long n_read = 0;
  while (n_read < archive_size) {

    // receive the next ARCHIVE_BLOCK message
    Message *bl = pipe.get_message(RA_ARCHIVE_BLOCK);
    if (bl == NULL) {
      fprintf(stderr, "raserver: do_paste_object(): "
                      "get_message() failed.\n");

      // send a response indicating an RAserver problem
      Mem mem;
      mem.append("n", 1);
      int res = -5;
      u_long n_res = htonl(u_long(res + 128));
      mem.append(&n_res, sizeof(n_res));
      Message m(RA_PASTE_OBJECT_RESPONSE, (char *)mem.data, mem.size);
      pipe.send_message(m);
      return;
    }

    // write the block to the file
    if (ulong(bl->length) > 0 &&
        ulong(bl->length) <= ulong(archive_size - n_read)) {
      if (bl->length != write(fd, bl->data, bl->length)) {
        fprintf(stderr,
                "raserver:do_paste_object(): could not create "
                "a local copy of the\n"
                "                                archive,"
                " because write(2) failed.\n"
                "         reason: %s\n",
                strerror(errno));

        // send a response indicating an RAserver problem
        Mem mem;
        mem.append("n", 1);
        int res = -5;
        u_long n_res = htonl(u_long(res + 128));
        mem.append(&n_res, sizeof(n_res));
        Message m(RA_PASTE_OBJECT_RESPONSE, (char *)mem.data, mem.size);
        pipe.send_message(m);
        return;
      }

      n_read += bl->length;
    } else {
      fprintf(stderr, "raserver:do_paste_object(): invalid "
                      "message size.\n");

      // send a response indicating an RAserver problem
      Mem mem;
      mem.append("n", 1);
      int res = -5;
      u_long n_res = htonl(u_long(res + 128));
      mem.append(&n_res, sizeof(n_res));
      Message m(RA_PASTE_OBJECT_RESPONSE, (char *)mem.data, mem.size);
      pipe.send_message(m);
      return;
    }
  }

  // close the temporary file
  close(fd);

  // perform the paste operation
  res = paste_object(oofs_dir, dst_fname, archive_name, old_path, move_links);

  // remove the local copy of the archive
  unlink(archive_name);

  // send a response to the other side
  if (res) {
    Mem mem;
    mem.append("n", 1);
    u_long n_res = htonl(u_long(res + 128));
    mem.append(&n_res, sizeof(n_res));
    Message m(RA_PASTE_OBJECT_RESPONSE, (char *)mem.data, mem.size);
    pipe.send_message(m);
  } else {
    Message m(RA_PASTE_OBJECT_RESPONSE, "y", 2);
    pipe.send_message(m);
  }

  // we are done
  return;
}

static void do_get_uuid(const Message &m, MessagePipe &pipe) {
  // extract the parameters from message
  char *ptr = (char *)m.data;
  std::string oofs_dir = ptr;
  ptr += oofs_dir.size() + 1;
  std::string path = ptr;
  ptr += path.size() + 1;
  bool create = (bool)(*ptr);
  // call getUUID
  QUuid uuid = getUUID(oofs_dir, path, create);
  // encode the result
  Mem buff;
  buff.append_string0(uuid.toString());
  Message response(RA_GET_UUID_RESPONSE, (char *)buff.data, buff.size);
  // send the result
  pipe.send_message(response);
}

void do_lookup_uuid(const Message &m, MessagePipe &pipe) {
  // extract the parameters from message
  char *ptr = (char *)m.data;
  std::string oofs_dir = ptr;
  ptr += oofs_dir.size() + 1;
  QUuid uuid = QUuid(ptr);
  // call lookupUUID
  std::string path = lookupUUID(oofs_dir, uuid);
  // return the result
  Mem buff;
  buff.append_string0(path.c_str());
  Message response(RA_LOOKUP_UUID_RESPONSE, (char *)buff.data, buff.size);
  pipe.send_message(response);
}

void do_reconcile_uuids(const Message &m, MessagePipe &pipe) {
  // extract the parameters from message
  char *ptr = (char *)m.data;
  std::string oofs_dir = ptr;
  ptr += oofs_dir.size() + 1;
  std::string path = ptr;
  ptr += path.size() + 1;
  bool recursive = (bool)(*ptr);
  ptr += 1;
  bool tablePriority = (bool)(*ptr);
  // call reconcile

  std::string res =
      ::uuidTableReconcile(oofs_dir, path, recursive, tablePriority);
  // encode the result
  Mem buff;
  std::string res_message = res + "\n";
  buff.append_string0(res_message.c_str());
  Message response(RA_RECONCILE_UUIDS_RESPONSE, buff);
  pipe.send_message(response);
}

void do_fix_oofs(const Message &m, MessagePipe &pipe) {
  // extract the parameters from message
  char *ptr = (char *)m.data;
  std::string oofs_dir = ptr;
  ptr += oofs_dir.size() + 1;
  bool renumber = (bool)(*ptr);
  // call fix oofs
  std::string res = ::fixOofs(oofs_dir, renumber);
  // encode the result
  Mem buff;
  std::string res_message = res + "\n";
  buff.append_string0(res_message.c_str());
  Message response(RA_FIX_OOFS_RESPONSE, buff);
  pipe.send_message(response);
}

void do_search_begin(const Message &m, MessagePipe &messagePipe) {
  // extract the parameters from message
  char *ptr = (char *)m.data;
  std::string oofs = ptr;
  ptr += oofs.size() + 1;
  std::string start_path = ptr;
  ptr += start_path.size() + 1;
  std::string pattern = ptr;
  ptr += pattern.size() + 1;
  bool caseSensitive = (bool)(*ptr);
  ptr += 1;
  bool exactMatch = (bool)(*ptr);
  // call searchBegin
  searchBegin(oofs, start_path, pattern, caseSensitive, exactMatch);
  // send result (which is really nothing other than the code)
  Mem buff;
  buff.append_string0("void");
  Message response(RA_SEARCH_BEGIN_RESPONSE, (char *)buff.data, buff.size);
  messagePipe.send_message(response);
}

void do_search_continue(const Message &m, MessagePipe &messagePipe) {
  // extract the parameters from message
  char *ptr = (char *)m.data;
  bool blocking = (bool)(*ptr);
  // call searchContinue
  std::string res = searchContinue(blocking);
  // send result
  Mem buff;
  buff.append_string0(res);
  Message response(RA_SEARCH_CONTINUE_RESPONSE, (char *)buff.data, buff.size);
  messagePipe.send_message(response);
}
void do_search_end(const Message &, MessagePipe &messagePipe) {
  // call searchEnd() and acknowledge
  searchEnd();
  // send result
  Mem buff;
  buff.append_string0("void");
  Message response(RA_SEARCH_END_RESPONSE, (char *)buff.data, buff.size);
  messagePipe.send_message(response);
}

static void log(const char *fmt_str, ...)
/*-----------------------------------.
| logs messages produced by raserver |
`-----------------------------------*/
{
  // if logging is not on, return
  if (!log_on)
    return;

  // open the log_file for appending
  FILE *fp = NULL;
  if (xstrcmp(log_file, "-") == 0)
    fp = stderr;
  else
    fp = fopen(log_file, "a");
  if (fp == NULL) {
    static char error_reported = 0;
    if (!error_reported) {
      fprintf(stderr, "raserver: cannot log messages to file %s.\n", log_file);
      perror("          fopen() problem:");
      error_reported = 1;
    }
  }

  // format the time string
  time_t t = time(NULL);
  struct tm *time_struct = localtime(&t);
  char time_string[4096];
  strftime(time_string, sizeof(time_string), "%b%d %H:%M:%S", time_struct);

  // format the rest of the string
  char buff[4096];
  va_list ap;
  va_start(ap, fmt_str);
  vsprintf(buff, fmt_str, ap);
  va_end(ap);

  // print the message
  fprintf(fp, "%10s[%6ld] %s: %s\n", curr_user_name, (long)my_pid, time_string,
          buff);

  if (fp != stderr)
    fclose(fp);
}
