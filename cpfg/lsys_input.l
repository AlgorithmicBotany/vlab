%{

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


/* lsys_input.l								    */
/* Jim Hanan 10/91							    */
/*											    */

/* MSH March 27, 1995
	- add recognition for ring label
	  is this too much or should the label just be "ring"? */

#ifdef WIN32
#include "warningset.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#ifndef WIN32
#include <unistd.h>
#endif

#ifdef WIN32
#include <io.h>
#endif
#include <string.h>

#include "control.h"
#include "generate.h"
#include "lsys_input_yacc.h"
extern YYSTYPE lsys_inputlval;
#ifdef __cplusplus
extern "C" {
#endif
    int lsys_inputwrap(void);
#ifdef __cplusplus
};
#endif

#include "test_malloc.h"

#ifdef WIN32
#define read _read
#define fileno _fileno
#endif


extern int lineNumber;
extern char *inputFile;
char inputFileBuffer[80];
int nesting;
int c_expression;
int CheckReserved(char *name);
char *ProcessString(char *str);
%}
%e 800
%p 8000
%a 35000
%o 3900
%k 240
%n 6000
digit	[0-9]
exp	[Ee][-+]?{digit}+
integer {digit}+
dreal	{digit}+"."{digit}*|{digit}*"."{digit}+
ereal	{dreal}{exp}
real	{dreal}|{ereal}
alpha	[a-zA-Z]
symbol	[^<>:* \n\t]
qstring	\"[^\"\n]*[\"\n]
name	{alpha}({alpha}|{digit}|\_)*
%start COMMENT SCOMMENT ECOMMENT SYMBOL EXPRESSION
%%
^"#line "{digit}+.+\n	{
	sscanf(yytext, "#line %d %s", &lineNumber, inputFileBuffer);
	inputFile = inputFileBuffer;
	}
^"# "{digit}+.+\n	{
				sscanf(yytext, "# %d %s", &lineNumber, inputFileBuffer);
				inputFile = inputFileBuffer;
				}
" "			;
\t			;
\n\t			{
				lineNumber++;
				}
\n				{
				lineNumber++;
				if(c_expression == 0){ 
				  BEGIN SYMBOL; 
				  return(tEOL);
				}
				}
(Lsystem|lsystem)":" {
				BEGIN SYMBOL;
				return(tLSTART);
				}
(seed|Seed)":"	{
				BEGIN EXPRESSION;
				return(tSEED);
				}
(D|d)erivation" "?(L|l)ength":" {
				BEGIN EXPRESSION;
				return(tDLENGTH);
				}
(M|m)aximum" "(D|d)epth":" {
				BEGIN EXPRESSION;
				return(tMDEPTH);
				}
(consider|Consider)":" {
				return(tCONSIDER);
				}
(ignore|Ignore)":" {
				return(tIGNORE);
				}
(axiom|Axiom)":" {
				return(tAXIOM);
				}
(define|Define)":" {
				BEGIN EXPRESSION;
				return(tDEFINE);
				}
(R|r)ing" "(L|l)system {
				return(tRING);
				}
(start|Start)":" {
				BEGIN EXPRESSION;
				return(tSTARTBLOCK);
				}
(end|End)":" 	{
				BEGIN EXPRESSION;
				return(tENDBLOCK);
				}
(start|Start)" "?(each|Each)":" {
				BEGIN EXPRESSION;
				return(tSTARTEACH);
				}
(end|End)" "?(each|Each)":" 	{
				BEGIN EXPRESSION;
				return(tENDEACH);
				}
(H|h)omomorphism {
				return(tSTARTHOMO);
				}
(H|h)omomorphism":" {
				return(tSTARTHOMO);
				}
(D|d)ecomposition {
				return(tSTARTDECOMP);
				}
(D|d)ecomposition":" {
				return(tSTARTDECOMP);
				}
"no warnings" {
				return(tNOWARNING);
				}
"warnings" {
				return(tWARNING);
				}
(E|e)nd(L|l)system {
				return(tEND);
				}
<SYMBOL>"/*"	{
				BEGIN SCOMMENT;
				}
<SCOMMENT>[^/\n]*\n	{
				lineNumber++;
				}
<SCOMMENT>[^/\n]+"/"	{
				if(yytext[yyleng-2] == '*')
				BEGIN SYMBOL;
				}
<SYMBOL>"*"		{
				return(tNULL);
				}
<SYMBOL>":"		{
				nesting = 1;
				BEGIN EXPRESSION;
				return(tCOLON);
				}
<SYMBOL>"-->"	{
				return(tYIELDS);
				}
<SYMBOL>"-o>"	{
				return(tOYIELDS);
				}
<SYMBOL>"<"		{
				return(tLSEP);
				}
<SYMBOL>">"		{
				return(tRSEP);
				}
<SYMBOL>"("		{
				nesting = 1;
				BEGIN EXPRESSION;
				return(tLPAREN);
				}
<SYMBOL>")"		{
				return(tNULL);
				}
<SYMBOL>{symbol} {
				lsys_inputlval.symbol = yytext[0];
				return(tSYMBOL);
				}
<EXPRESSION>"/*" {
				BEGIN ECOMMENT;
				}
<ECOMMENT>[^/\n]*\n	{
				lineNumber++;
				}
<ECOMMENT>[^/\n]+"/"	{
				if(yytext[yyleng-2] == '*')
				BEGIN EXPRESSION;
				}
<EXPRESSION>"-->"	{
				BEGIN SYMBOL;
				return(tYIELDS);
				}
<EXPRESSION>"-o>"	{
				BEGIN SYMBOL;
				return(tOYIELDS);
				}
<EXPRESSION>"="	{
				lsys_inputlval.operator_ = (char*) "=";
				return(tASSIGN);
				}
<EXPRESSION>"&"	{
				lsys_inputlval.operator_ = (char*) "&";
				return(tADDRESS);
				}
<EXPRESSION>"+"	{
				lsys_inputlval.operator_ = (char*) "+";
				return(tPLUS);
				}
<EXPRESSION>"-"	{
				lsys_inputlval.operator_ = (char*) "-";
				return(tMINUS);
				}
<EXPRESSION>"*"	{
				lsys_inputlval.operator_ = (char*) "*";
				return(tTIMES);
				}
<EXPRESSION>"/"	{
				lsys_inputlval.operator_ = (char*) "/";
				return(tDIVIDE);
				}
<EXPRESSION>"%"	{
				lsys_inputlval.operator_ = (char*) "%";
				return(tREM);
				}
<EXPRESSION>"^"	{
				lsys_inputlval.operator_ = (char*) "^";
				return(tPOW);
				}
<EXPRESSION>";"	{
				lsys_inputlval.operator_ = (char*) ";";
				return(tSEMI);
				}
<EXPRESSION>","	{
				lsys_inputlval.operator_ = (char*) ",";
				return(tCOMMA);
				}
<EXPRESSION>"?"	{
				lsys_inputlval.operator_ = (char*) ",";
				return(tQUESTION);
				}
<EXPRESSION>":"	{
				lsys_inputlval.operator_ = (char*) ",";
				return(tCOLON);
				}
<EXPRESSION>"("	{
				lsys_inputlval.operator_ = (char*) "(";
				nesting++;
				return(tLPAREN);
				}
<EXPRESSION>")"	{
				lsys_inputlval.operator_ = (char*) ")";
				nesting--;
				if (nesting == 0) BEGIN SYMBOL;
				return(tRPAREN);
				}
<EXPRESSION>"["	{
				lsys_inputlval.operator_ = (char*) "[";
				return(tLBRACKET);
				}
<EXPRESSION>"]"	{
				lsys_inputlval.operator_ = (char*) "]";
				return(tRBRACKET);
				}
<EXPRESSION>"<"	{
				lsys_inputlval.operator_ = (char*) "<";
				return(tLT);
				}
<EXPRESSION>">"	{
				lsys_inputlval.operator_ = (char*) ">";
				return(tGT);
				}
<EXPRESSION>"<>"|"!="	{
				lsys_inputlval.operator_ = (char*) "!=";
				return(tNOTEQUAL);
				}
<EXPRESSION>"==" {
				lsys_inputlval.operator_ = (char*) "==";
				return(tEQUAL);
				}
<EXPRESSION>"<=" {
				lsys_inputlval.operator_ = (char*) "<=";
				return(tLE);
				}
<EXPRESSION>">=" {
				lsys_inputlval.operator_ = (char*) ">=";
				return(tGE);
				}
<EXPRESSION>"!"	{
				lsys_inputlval.operator_ = (char*) "!";
				return(tNOT);
				}
<EXPRESSION>"&&" {
				lsys_inputlval.operator_ = (char*) "&&";
				return(tAND);
				}
<EXPRESSION>"||" {
				lsys_inputlval.operator_ = (char*) "||";
				return(tOR);
				}
<EXPRESSION>"{"	{
				lsys_inputlval.operator_ = (char*) "{";
				nesting++;
				return(tLBRACE);
				}
<EXPRESSION>"}"	{
				lsys_inputlval.operator_ = (char*) "}";
				nesting--;
				return(tRBRACE);
				}
<EXPRESSION>{name}	{
				lsys_inputlval.function = CheckReserved(yytext);
				if (lsys_inputlval.function) {
					return lsys_inputlval.function;
				}
				lsys_inputlval.name = Strdup(yytext);
				return(tNAME);
				}
<EXPRESSION>{qstring}	{
				lsys_inputlval.name = ProcessString(yytext);
				return(tSTRING);
				}
<EXPRESSION>{integer}	{
				sscanf(yytext, "%lf", &lsys_inputlval.value);
				return(tVALUE);
				}
<EXPRESSION>{real} {
				sscanf(yytext, "%lf", &lsys_inputlval.value);
				return(tVALUE);
				}
"/*"			{
				BEGIN COMMENT;
				}
<COMMENT>[^/\n]*\n	{
				lineNumber++;
				}
<COMMENT>[^/\n]+"/"	{
				if(yytext[yyleng-2] == '*')
				BEGIN 0;
				}
%%

#ifdef WIN32

int yywrap(void) { return 1; }

#else


int lsys_inputwrap( void ) {
    return 1;
}

#endif

/* there was
*/

/********************************************************************/
/*  CheckReserved - determines if a name string is a valid function name */
/*             and returns the appropriate token                    */
/********************************************************************/
int CheckReserved(char *name)
{
	static struct function {
		char* name;
		int tokenValue;
	} functions[] = 
	{
		{(char*)"if", tIF},
		{(char*)"while", tWHILE},
		{(char*)"do", tDO},
		{(char*)"else", tELSE},
		{(char*)"array", tARRAY},
#ifdef JIM
		{(char*)"external", tEXTERNAL},
#endif
		{(char*)"tan", tTAN},
		{(char*)"sin", tSIN},
		{(char*)"atan2", tATAN2},
		{(char*)"cos", tCOS},
		{(char*)"atan", tATAN}, 
		{(char*)"asin", tASIN},
		{(char*)"acos", tACOS},
		{(char*)"ran", tRAN},
		{(char*)"nran", tNRAN},
		{(char*)"bran", tBRAN},
		{(char*)"biran", tBIRAN},
		{(char*)"srand", tSRAND},
		{(char*)"exp", tEXP},
		{(char*)"log", tLOG},
		{(char*)"floor", tFLOOR}, 
		{(char*)"ceil", tCEIL},
                {(char*)"sign", tSIGN},
                {(char*)"sqrt", tSQRT},    /* Radek */
		{(char*)"trunc", tTRUNC},
		{(char*)"fabs", tFABS},
		{(char*)"inblob", tINBLOB},
                {(char*)"stop", tSTOP},
		{(char*)"printf", tPRINT},
		{(char*)"fprintf", tFPRINTF},
		{(char*)"fopen", tFOPEN},
		{(char*)"fclose", tFCLOSE},
		{(char*)"fflush", tFFLUSH},
		{(char*)"fscanf", tFSCANF},
#if CPFG_VERSION>=4000
		{(char*)"func", tFUNC},
		{(char*)"play", tPLAY},
#endif
#if CPFG_VERSION>=6400
		{(char*)"curveX", tCURVEX},
		{(char*)"curveY", tCURVEY},
		{(char*)"curveZ", tCURVEZ},
		{(char*)"curveGAL", tCURVEGAL},
#endif
#if CPFG_VERSION>=6500
		{(char*)"SetDerivationLength", tSETDERIVLENGTH},
		{(char*)"GetDerivationLength", tGETDERIVLENGTH},
#endif
#if CPFG_VERSION>=6600
		{(char*)"vvXmin", tVVXMIN},
		{(char*)"vvXmax", tVVXMAX},
		{(char*)"vvYmin", tVVYMIN},
		{(char*)"vvYmax", tVVYMAX},
		{(char*)"vvZmin", tVVZMIN},
		{(char*)"vvZmax", tVVZMAX},
		{(char*)"vvScale", tVVSCALE},
		{(char*)"Display", tDISPLAY},
#endif
		{(char *) NULL, 0}
	};
	struct function *ptr = functions;

	while (ptr->name != NULL && strcmp(ptr->name, name) != 0) {
		ptr++;
	}

	return ptr->tokenValue;

}

char *ProcessString(char *str)
{
	static char buf[1024];
	char *next = buf;
	
	str++;		/* skip opening double quote */
	while(*str){
		if(*str=='\\'){
			str++;
			switch(*str){
			case 'n': *next++ = '\n';
				str++;
				break;
			case 't': *next++ = '\t';
				str++;
				break;
			case 'r': *next++ = '\r';
				str++;
				break;
			default: *next++ = *str++;
			}
		}
		else
			*next++ = *str++;
	}
	*(--next) = 0;	/* skip closing double quote */

        return buf;
}
