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



#ifndef __COMMUNICATION_H__
#define __COMMUNICATION_H__

/************************************************************************/
/* types of communication */
#define COMM_MEMORY 0
#define COMM_FILES 1
#define COMM_PIPES 2

/************************************************************************/
/* semaphores  - used for memory or files communication */
/* semaphore key is determined from the cpfg's pid: (pid<<2)+0x13 */

/* values of the flag */
#define FIRST_CHUNK 1
#define LAST_CHUNK 4 /* when you are recompiling all main.c's change it to 2*/
#define PROCESS_EXIT 8

/* shared memory  - if files not used */
/* shared memory key is always the actual semaphore key minus one */

#define TO_FIELD_LENGTH 50000
#define FROM_FIELD_LENGTH 10000

#define MAX_QUERIES_IN_FILE 1000

struct shared_memory_type {
  char to_field[TO_FIELD_LENGTH];
  char from_field[FROM_FIELD_LENGTH];
};

typedef struct shared_memory_type shared_memory_type;

/* files */

#define FILENAME_TO_FIELD ".to_field.env"
#define FILENAME_FROM_FIELD ".from_field.env"

#endif
