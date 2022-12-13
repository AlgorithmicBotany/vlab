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



/*
 * Ped:
 *
 * Uses 'ed' to edit the given file based on formatted messages.
 *
 *  % ped file
 *
 * Normally the message come from a control panel:
 *
 *  % panel panelfile | ped file
 *
 * An 'ed' process is spawned and messages received from the panel
 * are converted to commands to 'ed'.
 *
 * Lynn Mercer
 * Computer Graphics Group
 * University of Regina
 *
 * June, 1989
 */

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

int pfd[2];
FILE *pfp; /* output fp to 'ed' process */

/*
 * InitEd() - creates the pipe and forks the 'ed' process
 */

void InitEd(char *filename) {
  if (pipe(pfd) < 0) {
    fprintf(stderr, "Ped: Pipe error\n");
    exit(-1);
  }
  switch (fork()) {
  case -1:
    fprintf(stderr, "Ped: Fork error\n");
    exit(-1);

  case 0:
    close(pfd[1]);
    close(0);
    dup(pfd[0]);
    close(1);
    open("/dev/null", O_WRONLY);
    execl("/bin/ed", "ed", filename, (char *)NULL);
    fprintf(stderr, "Ped: Cannot execute: ed\n");
    exit(-1);

  default:
    close(pfd[0]);
    pfp = fdopen(pfd[1], "w");
    break;
  }
}

/*
 * EdNumb()
 *      - sends an 'ed' command for a numeric update
 *  - the message contains the line number, field number (max. 3),
 *    scaling factor (power of 10), and integer value
 *      - the first field follows a ':', subsequent fields are
 *        separated by commas
 *      - if the scale is non-zero, the field is assumed to be
 *        floating point and will have 3 decimal places
 */

void EdNumb(char *msg) {
  int line, field, scale, value;
  double fscale, fvalue;

  sscanf(msg, "%d %d %d %d", &line, &field, &scale, &value);

  fprintf(pfp, "%ds", line);
  switch (field) {
  case 1:
    fprintf(pfp, "/: *[-\\.0-9]*/: ");
    break;

  case 2:
    fprintf(pfp, "/, *[-\\.0-9]*/,");
    break;

  case 3:
    fprintf(pfp, "/\\(, *[-\\.0-9]*, *\\)[-\\.0-9]*/\\1");
    break;

  default:
    fprintf(stderr, "Ped: Illegal field: %d\n", field);
    exit(-1);
  }
  if (scale == 0)
    fprintf(pfp, "%d/\n", value);
  else {
    fscale = (double)scale;
    fscale = pow((double)10.0, fscale);
    fvalue = value * fscale;
    fprintf(pfp, "%.4f/\n", fvalue);
  }
  fprintf(pfp, "w\n");
  fflush(pfp);
}

/*
 * EdOff()
 *  - sends 'ed' command for a switch (on/off) update
 *  - message contains line number and integer value (0 or 1)
 *  - the value is converted to a string: 0 = off, 1 = on
 */

void EdOff(char *msg) {
  int line, value;

  sscanf(msg, "%d %d", &line, &value);
  fprintf(pfp, "%ds", line);
  fprintf(pfp, "/: *[a-z]*/: ");
  if (value == 0)
    fprintf(pfp, "off/\n");
  else
    fprintf(pfp, "on/\n");
  fprintf(pfp, "w\n");
  fflush(pfp);
}

/*
 * ProcessMsg() - checks for the two types of messages and calls the
 *      appropriate routine
 */

void ProcessMsg(char *msg) {
  switch (msg[0]) {
  case 'n':
    EdNumb(&msg[1]);
    break;

  case 'o':
    EdOff(&msg[1]);
    break;

  default:
    break;
  }
}

int main(int argc, char **argv) {
  char msg[100];

  if (argc < 2) {
    fprintf(stderr, "Usage: ped file\n");
    exit(0);
  }
  InitEd(argv[1]);
  while (fgets(msg, 100, stdin) != 0)
    ProcessMsg(msg);
  fprintf(pfp, "q\nq\n");
  fflush(pfp);

  return 0;
}
