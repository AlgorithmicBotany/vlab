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




#ifndef __COMM_LIB_H__
#define __COMM_LIB_H__


#define CMAXPARAMS 20  /* max. number of symbol parameters */
#define CMAXSYMBOLLEN 4 /* max. length of a module in characters (e.g. @Gs) */

#define CMAXFIELDS 20  /* maximal number of environmental fields/processes */

/* the local structure containing a L-system module */
struct module_type
{
  char symbol[CMAXSYMBOLLEN+1];
  int num_params;
  struct param_type
  {
    float value;
    char  set;        /* which parameters are set by the environment */
  } params[CMAXPARAMS];
};
typedef struct module_type Cmodule_type;

/* the local structure describing the turtle */
struct CTURTLE
{
  float position[3];
  int positionC;  /* number of obtained parameters - for the slave */
  float heading[3];
  int headingC;
  float left[3];
  int leftC;
  float up[3];
  int upC;
  float line_width;
  int line_widthC;
  float scale_factor;
  int scale_factorC;
};
typedef struct CTURTLE CTURTLE;

#define CMIN_MASTER_INDEX 0

#ifdef WIN32
typedef unsigned short u_short;
#endif

/************** prototypes ***********************/

/**** Two process communication ****/

#ifdef __cplusplus
extern "C" {
#endif
/* master (CM) */
int  CMAddProcess(char *filename);
int  CMInitialize(void);
int  CMBeginTransmission(void);

/* 
   for sending and receiving data, use function from multiprocess communication
   with the first parameter 'index' set to 0 
*/

void CMFreeStructures(void);
int  CMEndTransmission(int current_step);
int  CMTerminate(void);

/* slave CS */
void CSInitialize(int *argc, char ***argv);
int  CSBeginTransmission(void);
void CSMainLoop(int (*AnswerQuery)(Cmodule_type *, CTURTLE *));
int  CSEndTransmission(void);
void CSTerminate(void);


/**** multiprocess communication ****/
/* common */
void CInitialize(char *program_name, char *command_string);
void CTerminate(void);
int  CShouldTerminate(void);

/* master (CM) */
int  CMGetNumberOfSlaves(void);

int  CMSendString(int index, char *item);
int  CMGetString(int index, char *str, int length);

int  CMSendBinaryData(int index, char *item, int item_size, int nitems);
/* returns number of items read */
int  CMGetBinaryData(int index, char *data, int item_size, int nitems);

int  CMSendCommSymbol(int index, unsigned long distance, 
		      Cmodule_type *two_modules, CTURTLE *turtle);
int  CMGetCommunicationModule(int index, unsigned long *module_id, 
			      Cmodule_type *comm_module);



/* slave (CS) */
int  CSGetNumberOfMasters(void);

int  CSGetData(int *master, unsigned long *distance, 
	       Cmodule_type *two_modules, CTURTLE *turtle);
void CSSendData(int master, unsigned long dist, Cmodule_type *comm_symbol);

int  CSGetString(int *master, char *str, int length);
int  CSSendString(int index, char *item);

int  CSSendBinaryData(int index, char *item, int item_size, int nitems);

#ifdef __cplusplus
}
#endif


#else
	#error File already included
#endif
