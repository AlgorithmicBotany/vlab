/* lex.l								   */
/* Copyright (C) 1989, 1991, Craig E. Kolb				   */
/* All rights reserved.							   */
/*									   */
/* This software may be freely copied, modified, and redistributed,	   */
/* provided that this copyright notice is preserved on all copies.	   */
/*									   */
/* You may not distribute this software, in whole or in part, as part of   */
/* any commercial product without the express consent of the authors.	   */
/* 									   */
/* There is no warranty or other guarantee of fitness of this software	   */
/* for any purpose.  It is provided solely "as is".			   */
/*									   */

%{
#include "config.h"
#include "rayshade.h"
#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif
#include "liblight/light.h"
#include "libsurf/atmosphere.h"
#include "libsurf/surface.h"
#include "libtext/texture.h"
#include "libobj/geom.h"
#include "symtab.h"
#include "y.tab.h"
extern char *strsave();
void* skipcomments();
void handlehash();
%}
alpha	[a-zA-Z]
special	[\.\_-]
digit	[0-9]
exp	[Ee][-+]?{digit}+
string	{alpha}({alpha}|{digit}|{special})*
filename "/"?"/"?(("."|".."|{string})"/")*{string}
%p 9400
%e 1500
%n 600
%%
[ \t\n]			;
^#			handlehash();
"/*"			skipcomments();
ambient			return tAMBIENT;
aperture		return tAPERTURE;
applysurf		return tAPPLYSURF;
area			return tAREA;
atmosphere		return tATMOSPHERE;
background		return tBACKGROUND;
blob			return tBLOB;
blotch			return tBLOTCH;
body			return tBODY;
box			return tBOX;
bump			return tBUMP;
checker			return tCHECKER;
cloud			return tCLOUD;
cone			return tCONE;
component		return tCOMPONENT;
contrast		return tCONTRAST;
crop			return tCROP;
cursurf			return tCURSURF;
cutoff			return tCUTOFF;
cylinder		return tCYL;
cylindrical		return tCYLINDRICAL;
define			return tDEFINE;
diffuse			return tDIFFUSE;
difference		return tDIFFERENCE;
directional		return tDIRECTIONAL;
disc			return tDISC;
opendisc		return tOPENDISC;
end			return tEND;
extended		return tEXTENDED;
extinct			return tEXTINCT;
eyep			return tEYEP;
eyesep			return tEYESEP;
filter			return tFILTER;
fbm			return tFBM;
fbmbump			return tFBMBUMP;
focaldist		return tFOCALDIST;
fog			return tFOG;
fogdeck			return tFOGDECK;
fov			return tFOV;
framelength		return tFRAMELENGTH;
frames			return tFRAMES;
gauss			return tGAUSS;
gloss			return tGLOSS;
grid			return tGRID;
heightfield		return tHEIGHTFIELD;
image			return tIMAGE;
index			return tINDEX;
intersect		return tINTERSECT;
jitter			return tJITTER;
light			return tLIGHT;
list			return tLIST;
lookp			return tLOOKP;
map			return tMAP;
marble			return tMARBLE;
maxdepth		return tMAXDEPTH;
mount			return tMOUNT;
mist			return tMIST;
name			return tNAME;
nojitter		return tNOJITTER;
noshadow		return tNOSHADOW;
object			return tOBJECT;
outfile			return tOUTFILE;
plane			return tPLANE;
planar			return tPLANAR;
point			return tPOINT;
poly			return tPOLY;
polygon			return tPOLY;
print			return tPRINT;
quiet			return tQUIET;
range			return tRANGE;
reflect			return tREFLECT;
reflective		return tREFLECT;
report			return tREPORT;
resolution		return tSCREEN; /* A synonym for screen */
rotate			return tROTATE;
sample			return tSAMPLE;
scale			return tSCALE;
screen			return tSCREEN;
shadowtransp		return tSHADOWTRANSP;
shutter			return tSHUTTER;
sky			return tSKY;
smooth			return tSMOOTH;
sphere			return tSPHERE;
spherical		return tSPHERICAL;
specular		return tSPECULAR;
specpow			return tSPECPOW;
spot			return tSPOT;
starttime		return tSTARTTIME;
stripe			return tSTRIPE;
surface			return tSURFACE;
textsurf		return tTEXTSURF;
texture			return tTEXTURE;
tile			return tTILE;
torus			return tTORUS;
transform		return tTRANSFORM;
translate		return tTRANSLATE;
translu			return tTRANSLU;
translucency		return tTRANSLU;
transp			return tTRANSP;
transparent		return tTRANSP;
triangle		return tTRIANGLE;
triangleuv		return tTRIANGLEUV;
union			return tUNION;
up			return tUP;
uv			return tUV;
verbose			return tVERBOSE;
window			return tWINDOW;
windy			return tWINDY;
wood			return tWOOD;
{digit}+ |
{digit}+"."{digit}*({exp})? |
{digit}*"."{digit}+({exp})? |
{digit}+{exp}		{yylval.d = atof(yytext); return tFLOAT;}
{string}		{yylval.c = strsave(yytext); return tSTRING;}
{filename}		{yylval.c = strsave(yytext); return tFILENAME;}
.			return yytext[0];

%%
int yywrap() {return 1;}
/*
 * Skip over comments.
 */
void* skipcomments()
{
	char c;

	while (1) {
		while (input() != '*')
			;
		if ((c = input()) == '/')
			return NULL;
		unput(c);
	}
}
/*
 * Deal with ccp-produced lines of the form:
 * # n "filename"
 * and
 * # n
 * Where filename is the name of the file being processed, and n is
 * the current line number in that file.
 */
void handlehash()
{
	char buf[BUFSIZ];
	int i;
	extern int yylineno;
	extern char yyfilename[];

	/*
	 * Read the entire line into buf.
	 */
	for (i = 0; (buf[i] = input()) != '\n'; i++)
			;
	unput(buf[i]);		/* To make sure consecutive # lines work. */
	buf[i] = (char)NULL;	/* Replace newline with NULL. */

	/*
	 * Complain if the line was not of the form #n "filename"
	 */
	if ((i = sscanf(buf,"%d \"%[^\"]s\"", &yylineno, yyfilename)) == 0 &&
	    (i = sscanf(buf," line %d \"%[^\"]s\"",&yylineno,yyfilename))==0) {
		RLerror(RL_PANIC, "Unknown '#' control (%s).\n",buf);
		exit(1);
	}
	if (i == 1 && (index(buf,'"') != (char *)NULL)) {
		/*
		 * Filename was given as ""
		 */
		(void)strcpy(yyfilename, "stdin");
	}
	yylineno--;  /* The newline we unput will increment yylineno */
}
