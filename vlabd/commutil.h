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




#ifndef _COMMUTIL_
#define _COMMUTIL_

/* possible error during communication */
#define INVALID_QUEUE -1
#define INVALID_COMMSERVER -2
#define MSG_WRITE -3
#define NO_COMMSERVER -4

/* function prototypes */
int SendData(int socketid, int type, char *data);
int startCommServer(void);
void GetCommInfoFromFile(void);
int GetCommID(void);
int GetCommPort(void);
int QuitComm(void);
void comm_err(int err);

#endif
