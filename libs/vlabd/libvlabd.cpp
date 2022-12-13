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



#include "libvlabd.h"
#include "Mem.h"
#include "connect_to.h"
#include "vlabd_msgid.h"
#include "xmemory.h"
#include <assert.h>
#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

static const int DEBUG = 0;

// Constructor:
//    - initializes sock to '-1', indicating that a connection with vlabd has
//      not yet been established
VlabD::VlabD(void) {
  sock = -1;
  _valid = false;
}

// Initializes a connection to vlabd. If vlabd is not running, it will be
// automatically started (unless allowStart = false)
//
// Returns  0 = success
//         -1 = error (verbose error can be retrieved by get_errstr)
//
int VlabD::init(bool allowStart) {
  _valid = false;
  std::string last_err_msg;

  long vlabd_port;

  volatile int n_invoke_tries = 0;

  // GRRRRRRR GOTO, can someone rewrite this so that there is no goto please?
try_to_connect:
  if (n_invoke_tries >= n_invoke_retries) {
    // we have exceeded the number of tries... time to quit
    sprintf(err_buff,
            "Failed to establish connection to vlabd.\n"
            "Last error message was: %s\n",
            last_err_msg.c_str());
    return -1;
  }
  if (n_invoke_tries > 0) {
    // another try - sleep first
    sleep(invoke_pause);
  }
  // increment the counter
  n_invoke_tries++;

  // check if vlabd is already running
  // ---------------------------------
  bool vlabd_running = false;
  char fname[4096];
  struct passwd *pw = getpwuid(getuid());

  const char *homedir = pw->pw_dir;

  sprintf(fname, "%s/.vlab/tmp/vlabd%ld", homedir, long(getuid()));
  if (DEBUG)
    fprintf(stderr, "VlabD::init(): opening file '%s'\n", fname);
  // open the file
  FILE *fp = fopen(fname, "r");
  if (fp != NULL) {
    if (DEBUG)
      fprintf(stderr, "VlabD::init(): trying to read id/port info.\n");
    // try to read the PID and PORT of vlabd
    pid_t vlabd_pid;
    if (2 == fscanf(fp, "%d,%ld", &vlabd_pid, &vlabd_port)) {
      if (DEBUG)
        fprintf(stderr, "VlabD::init(): id=%ld port=%ld\n", long(vlabd_pid),
                vlabd_port);
      // see if vlabd with that PID is still running
      if (kill(vlabd_pid, 0) == 0) {
        vlabd_running = true;
        if (DEBUG)
          fprintf(stderr, "VlabD::init(): vlabd[%ld] running.\n",
                  long(vlabd_pid));
      } else {
        if (DEBUG)
          fprintf(stderr, "VlabD::init(): vlabd[%ld] not running.\n",
                  long(vlabd_pid));
      }
    }
    fclose(fp);
  } else {
    if (DEBUG)
      fprintf(stderr, "VlabD::init(): could not open\n");
  }

  // if vlabd is not running and we're not allow to start it up, we're done
  if ((!vlabd_running) && (!allowStart)) {
    sprintf(err_buff, "vlabd is not running.");
    return -1;
  }

  // if vlabd is not running, start it up
  // ------------------------------------
  if (!vlabd_running) {
    if (DEBUG)
      fprintf(stderr, "VlabD::init(): trying to start vlabd.\n");
    // get the name of the executable
    const char *vlabd_bin_str = getenv("VLABDAEMONBIN");
    if (DEBUG)
      fprintf(stderr, "VlabD::init(): program launched: '%s'\n", vlabd_bin_str);
    if (vlabd_bin_str == NULL) {
      fprintf(stderr, "Your VLABDAEMONBIN is not set.\n");
      fprintf(stderr, "Using 'vlabd' in your path instead.\n");
      vlabd_bin_str = "vlabd";
    }
    pid_t pid = vfork();
    if (pid < 0) {
      last_err_msg = std::string("Cannot vfork(): ") + strerror(errno);
      goto try_to_connect;
    }
    if (pid == 0) {
      execlp(vlabd_bin_str, vlabd_bin_str, (char *)NULL);
      last_err_msg = std::string("execlp() failed: ") + strerror(errno);
      _exit(0);
    }
    // parent will continue only once the child has called _exit() or exec()
    goto try_to_connect;
  }

  // ----------------
  // connect to VLABD
  // ----------------

  int reconnect_try = 0;
reconnect:

  //    sock = connect_to( "localhost", vlabd_port);
  sock = connect_to("127.0.0.1", vlabd_port);
  if (sock == -1) {
    reconnect_try++;
    if (reconnect_try <= n_connect_retries) {
      sleep(connect_pause);
      goto reconnect;
    }
    sprintf(err_buff, "Could not connect to VLABD on %d tries.",
            n_connect_retries);
    if (DEBUG)
      fprintf(stderr, "VlabD::init(): %s\n", err_buff);
    return -1;
  } else if (sock == -2) {
    sprintf(err_buff, "Could not get host address.");
    if (DEBUG)
      fprintf(stderr, "VlabD::init(): Could not get host address.\n");
    return -1;
  }
  if (DEBUG)
    fprintf(stderr, "VlabD::init(): connected to vlabd with socket %d.\n",
            sock);

  // --------------------------------------------
  // send VLABD an ACK request and wait for reply
  // --------------------------------------------
  if (send_message(ACK_REQUEST, "ack request", 11)) {
    sprintf(err_buff, "Cannot send ACK request.");
    if (DEBUG)
      fprintf(stderr, "VlabD::init(): %s\n", err_buff);
    return -1;
  }
  if (DEBUG)
    fprintf(stderr, "VlabD::init(): sent ACK request.\n");
  // wait until there is something to be read from VLABD
  while (!has_data()) {
    sleep(1);
    if (DEBUG)
      fprintf(stderr, "VlabD::init(): waiting for ACK.\n");
  }
  if (DEBUG)
    fprintf(stderr, "VlabD::init(): there is data waiting.\n");
  // read a message from VLABD
  long code;
  char *msg;
  long length;
  if (get_message(code, msg, length)) {
    sprintf(err_buff, "Cannot receive ACK request.");
    if (DEBUG)
      fprintf(stderr, "VlabD::init(): %s\n", err_buff);
    return -1;
  }
  if (DEBUG)
    fprintf(stderr, "VlabD::init(): received ACK request.\n");
  // if the received message is ACK_RESPONSE, parse it
  if (code == ACK_RESPONSE) {
    Mem str;
    str.append(msg, length);
    str.append("\0", 1);

    if (DEBUG)
      fprintf(stderr,
              "VlabD::init(): VLABD responded with msg[len=%ld]:\n"
              "               %s\n",
              length, str.data);

    long version_major, version_minor;
    if (2 !=
        sscanf((char *)str.data, "%ld %ld", &version_major, &version_minor)) {
      sprintf(err_buff, "Cannot synchronize with vlabd - bad ACK response.");
      if (DEBUG)
        fprintf(stderr, "VlabD::init(): %s\n", err_buff);
      return -1;
    }
    if (version_major < 4 || (version_major == 4 && version_minor < 0)) {
      sprintf(err_buff, "Old version of vlabd (%ld.%ld). Need >= 4.0.",
              version_major, version_minor);
      if (DEBUG)
        fprintf(stderr, "VlabD::init(): %s\n", err_buff);
      return -1;
    }
  }
  // the received message is not ACK - report error (version mismatch?)
  else {
    sprintf(err_buff, "Cannot synchronize with vlabd - maybe bad version.");
    if (DEBUG)
      fprintf(stderr, "VlabD::init(): %s\n", err_buff);
    return -1;
  }

  _valid = true;
  return 0;
}

//____________________________________________________________________________

char *VlabD::get_errstr(void)
/*
- returns a textual description of the last error
*/
{
  return err_buff;
}

//____________________________________________________________________________

int VlabD::has_data(void)
/*
- returns whether there is any information waiting from vlabd

- returns: 1 = yes, there is something
0 = no, there is nothing
*/
{
  if (sock == -1)
    return 0;
  fd_set rmask;
  struct timeval timeout;

  timeout.tv_sec = 0;   // Timeout duration of 0 seconds
  timeout.tv_usec = 10; // and 1 microsecond

  struct rlimit rl;
  (void)getrlimit(RLIMIT_NOFILE, &rl);
  long max_fd = rl.rlim_max;

  FD_ZERO(&rmask);
  FD_SET(sock, &rmask);

  select((int)max_fd, &rmask, NULL, NULL, &timeout);

  return (FD_ISSET(sock, &rmask) ? 1 : 0);
}

//____________________________________________________________________________

int VlabD::send_message(long type, const char *data, long size)
/*
send_message()

- sends VLABD a message

- returns: 0 = success
!0 = failure (use get_errstr() to find out more)

*/
{
  // prepare the memory where the raw message data will be stored
  Mem buff(size + 8);

  // encode the size of the message (not including the size info, but
  // including the type)
  unsigned long l = size + 4;
  unsigned char size_buff[4];
  size_buff[0] = (l >> 24) & 255;
  size_buff[1] = (l >> 16) & 255;
  size_buff[2] = (l >> 8) & 255;
  size_buff[3] = (l >> 0) & 255;

  // encode the type of the message
  unsigned char type_buff[4];
  type_buff[0] = (type >> 24) & 255;
  type_buff[1] = (type >> 16) & 255;
  type_buff[2] = (type >> 8) & 255;
  type_buff[3] = (type >> 0) & 255;

  // put everything into the buffer
  buff.append(size_buff, 4);
  buff.append(type_buff, 4);
  buff.append(data, size);

  if (DEBUG) {
    fprintf(stderr, "VlabD::send_message(%ld,...,%ld)\n", type, size);
    long i;
    fprintf(stderr, "       ");
    for (i = 0; i < buff.size; i++) {
      int d = *(unsigned char *)&buff[i];
      fprintf(stderr, "%x%x", d / 16, d % 16);
    }
    fprintf(stderr, "\n");
    fprintf(stderr, "       ");
    for (i = 0; i < buff.size; i++) {
      int d = *(unsigned char *)&buff[i];
      if (d >= 32 && d < 128)
        fprintf(stderr, " %c", char(d));
      else
        fprintf(stderr, " .");
    }
    fprintf(stderr, "\n");
  }

  //
  // send the contents of the buffer through the socket
  //
  long remains = buff.size;
  long curr = 0;
  while (remains > 0) {
    long sent = write(sock, buff.data + curr, remains);
    if (sent < 0) {
      sprintf(err_buff, "write() on socket to vlabd failed: %s\n",
              strerror(errno));
      return -1;
    }

    remains -= sent;
    curr += sent;
  }

  return 0;
}

//____________________________________________________________________________

int VlabD::send_message(long type, const char *str)
/*
- sends a null-terminated str
*/
{
  return send_message(type, str, strlen(str) + 1);
}

//____________________________________________________________________________

int VlabD::send_message(long type)
/*
- sends just the type, with an empty body
*/
{
  return send_message(type, "", 1);
}

//____________________________________________________________________________

int VlabD::va_send_message(long type, const char *fmt_str, ...)
/*
- accepts formatting string just like printf)
*/
{

  if (DEBUG)
    printf("Sending Message : %s \n", fmt_str);
  // prepare the message into 'buff'
  char buff[4096];
  va_list ap;
  va_start(ap, fmt_str);
  vsprintf(buff, fmt_str, ap);
  va_end(ap);

  // send the message
  int res = send_message(type, buff);

  // return the result
  return res;
}

//____________________________________________________________________________

int VlabD::receive_n_bytes(char *buffer, long size)
/*
- will attempt to receive size bytes into buffer
*/
{
  long i;
  long read_size;

  for (i = 0; i < size; i++) {
    read_size = (long)read(sock, (void *)(&(buffer[i])), 1);
    if (read_size < 1) {
      sprintf(err_buff, "read() on socket to vlabd: %s", strerror(errno));
      return -1;
    }
  }
  return 0;
}

//____________________________________________________________________________

int VlabD::get_message(long &code, char *&data, long &length)
/*
- reads a message from VLABD

- returns:   0 = success
!0 = error (more info through get_errstr())
*/
{
  unsigned char tmp[8]; // temporary buffer for 4-byte numbers

  // get the first 8 bytes into tmp
  if (receive_n_bytes((char *)tmp, 8))
    return -1;

  // extract the size of the message
  long size =
      tmp[0] * 256 * 256 * 256 + tmp[1] * 256 * 256 + tmp[2] * 256 + tmp[3];

  if (DEBUG)
    fprintf(stderr, "VlabD::get_message(): Waiting for %ld bytes.\n", size);

  // extract the code
  code = tmp[4] * 256 * 256 * 256 + tmp[5] * 256 * 256 + tmp[6] * 256 + tmp[7];

  // receive the rest of the message
  if (size - 4 > 0) {
    data = (char *)xmalloc(size - 4);
    if (receive_n_bytes(data, size - 4))
      return -1;
  } else {
    data = NULL;
  }
  length = size - 4;

  return 0;
}

//____________________________________________________________________________

int VlabD::get_sock(void)
/*
- returns the socket of the connection
*/
{

  return sock;
}
