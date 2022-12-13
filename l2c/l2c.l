%{

#include <assert.h>
#include <string.h>


#define YY_NEVER_INTERACTIVE 1

#include <lparams.h>

#include "module.h"
#include "contextdata.h"
#include "l2c_yacc.h"
#include "maxpath.h"

extern ModuleTable moduleTable;

extern bool bModulesOnly; // Mik 04/2013 - check if parser is in ModulesOnly state

// Pascal New version of Flex
#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
extern int isatty( int );
#ifdef __cplusplus
}
#endif /* __cplusplus */



extern "C" int l2cwrap()
{ return 1; }

int nested;
int lineno = 1;
bool first_time = true;
int BeforeString;
char FileName[MaxPath+3];


bool verbose = false;
void DumpOut(const char*);
void SetCurLine(const char* bf)
{
	sscanf(bf, "%d %s", &lineno, FileName);
}


%}

/* Mik 04/2013 - add SKIP state to skip lines before "#pragma L2C start" */
/*               (below) add rules for skipping header before the L2C directive */
%s SKIP NRMLSRC MODULE PRODPROTO PRODUCTION CONIGN STRING CHARACTER EXPRESSION GROUPSTMT INCONTEXT

%%

%{
	if (first_time)
	{
                // Mik 04/2013 - if only parsing the module file, there is nothing to skip 
                if (bModulesOnly)
                  BEGIN NRMLSRC;
                else
		  BEGIN SKIP;
		nested = 0;
		first_time = false;
	}
%}

<SKIP>^"#pragma L2C start" { BEGIN NRMLSRC; }
<SKIP>\n { lineno++; DumpOut(l2ctext); }
<SKIP>.+ { DumpOut(l2ctext); }

<NRMLSRC>"derivation length:" { BEGIN EXPRESSION; return tDERIVLENGTH; }
<NRMLSRC>"ring L-system:" { BEGIN EXPRESSION; return tRINGLSYSTEM; }
<NRMLSRC>"maximum depth:" { BEGIN EXPRESSION; return tMAXDEPTH; }
<NRMLSRC>"module" { BEGIN MODULE; return tMODULE; }
<NRMLSRC>"consider:" { BEGIN CONIGN; return tCONSIDER; }
<NRMLSRC>"ignore:" { BEGIN CONIGN; return tIGNORE; }
<NRMLSRC>"VerifyString:" { BEGIN CONIGN; return tVERIFYSTRING; }
<NRMLSRC>"interpretation:" { return tINTERPRETATION; }
<NRMLSRC>"decomposition:" { return tDECOMPOSITION; }
<NRMLSRC>"production:" { return tPRODUCTION; }
<NRMLSRC>"produce" {
	BEGIN PRODUCTION;
	nested = 0;
	return tPRODUCE;
	}
<NRMLSRC>"nproduce" {
	BEGIN PRODUCTION;
	nested = 0;
	return tNPRODUCE;
	}
<NRMLSRC>"propensity" { return tPROPENSITY; }
<NRMLSRC>"Axiom:" { BEGIN PRODUCTION; nested = 0; return tAXIOM; }
<NRMLSRC>"axiom:" { BEGIN PRODUCTION; nested = 0; return tAXIOM; }
<NRMLSRC>"Start:" { return tSTART; }
<NRMLSRC>"StartEach:" { return tSTARTEACH; }
<NRMLSRC>"EndEach:" { return tENDEACH; }
<NRMLSRC>"End:" { return tEND; }
<NRMLSRC>"group" { BEGIN GROUPSTMT; return tGROUP; }
<NRMLSRC>"ggroup" { BEGIN GROUPSTMT; return tGGROUP; }
<NRMLSRC>"endgroup" { return tENDGROUP; }
<NRMLSRC>"window" { BEGIN GROUPSTMT; return tVGROUP; }
<NRMLSRC>"vgroup" { BEGIN GROUPSTMT; return tVGROUP; }
<NRMLSRC>"endwindow" { return tENDVGROUP; }
<NRMLSRC>"endvgroup" { return tENDVGROUP; }
<NRMLSRC>"InRightContext" { BEGIN INCONTEXT; nested = 0; return tINRIGHTCONTEXT; }
<NRMLSRC>"InLeftContext" { BEGIN INCONTEXT; nested = 0; return tINLEFTCONTEXT; }
<NRMLSRC>"InNewRightContext" { BEGIN INCONTEXT; nested = 0; return tINNEWRIGHTCONTEXT; }
<NRMLSRC>"InNewLeftContext" { BEGIN INCONTEXT; nested = 0; return tINNEWLEFTCONTEXT; }
<NRMLSRC>[a-zA-Z_][0-9a-zA-Z_]* {
	if (moduleTable.IsModuleIdent(l2ctext))
	{
	  if (verbose)
	    printf("module recognized: %s\n", l2ctext);
	  strncpy(l2clval.Ident, l2ctext, __lc_eMaxIdentifierLength);
	  l2clval.Ident[__lc_eMaxIdentifierLength] = 0;
	  BEGIN PRODPROTO;
	  return tMODULEIDENT;
	}
	else
	  DumpOut(l2ctext);
	}

<CONIGN>[a-zA-Z_][0-9a-zA-Z_]* {
	if (moduleTable.IsModuleIdent(l2ctext))
	{
		if (verbose)
		  printf("module recognized: %s\n", l2ctext);
		strncpy(l2clval.Ident, l2ctext, __lc_eMaxIdentifierLength);
		l2clval.Ident[__lc_eMaxIdentifierLength] = 0;
		return tMODULEIDENT;
	}
	else
		return tERROR;
	}
<CONIGN>";" { BEGIN NRMLSRC; return tSEMICOLON; }


<MODULE>"(" { return tLPAREN; }
<MODULE>")" { return tRPAREN; }
<MODULE>"," { return tCOMMA; }
<MODULE>"=" { return tEQUALS; }
<MODULE,GROUPSTMT>[0-9]+ { l2clval.Integer = atoi(l2ctext); return tINTEGER; }
<MODULE>";" { BEGIN NRMLSRC; return tSEMICOLON; }
<MODULE>[a-zA-Z_][0-9a-zA-Z_]* {
	strncpy(l2clval.Ident, l2ctext, __lc_eMaxIdentifierLength);
	l2clval.Ident[__lc_eMaxIdentifierLength] = 0;
	return tIDENT; }

<PRODPROTO>"(" { return tLPAREN; }
<PRODPROTO>")" { return tRPAREN; }
<PRODPROTO>"," { return tCOMMA; }
<PRODPROTO>":" { BEGIN NRMLSRC; return tENDPRODPROTO; }
<PRODPROTO>"<" { return tLESSTHAN; }
<PRODPROTO>">" { return tGREATERTHAN; }
<PRODPROTO>"<<" { return tLEFTSHIFT; }
<PRODPROTO>">>" { return tRIGHTSHIFT; }
<PRODPROTO>[a-zA-Z_][0-9a-zA-Z_]* {
	strncpy(l2clval.Ident, l2ctext, __lc_eMaxIdentifierLength);
	l2clval.Ident[__lc_eMaxIdentifierLength] = 0;
	if (moduleTable.IsModuleIdent(l2ctext))
	{
		if (verbose)
		  printf("module recognized: %s\n", l2ctext);
		return tMODULEIDENT;
	}
	else
		return tIDENT;
	}

<PRODUCTION>"(" { nested++; if (1==nested) return tLPAREN; else DumpOut(l2ctext); }
<PRODUCTION>")" { nested--; if (0==nested) return tRPAREN; else DumpOut(l2ctext); }
<PRODUCTION>"," { if (1==nested) return tCOMMA; else DumpOut(l2ctext); }
<PRODUCTION>[a-zA-Z_][0-9a-zA-Z_]* {
	if (moduleTable.IsModuleIdent(l2ctext))
	{
	  if (verbose)
	    printf("module recognized: %s\n", l2ctext);
	  strncpy(l2clval.Ident, l2ctext, __lc_eMaxIdentifierLength);
	  l2clval.Ident[__lc_eMaxIdentifierLength] = 0;
	  return tMODULEIDENT;
	}
	else
	{
	  if (0==nested)
	    printf("module expected. %s found at %d in %s\n", l2ctext, lineno, FileName);
	  DumpOut(l2ctext);
	}
	}
<PRODUCTION>";" { BEGIN NRMLSRC; return tSEMICOLON; }

<GROUPSTMT>":" { BEGIN NRMLSRC; return tCOLON; }

<EXPRESSION>";" { BEGIN NRMLSRC; DumpOut(l2ctext); return tSEMICOLON; }

<INCONTEXT>"(" { 
		++nested; 
		return tLPAREN;
	}

<INCONTEXT>")" {
		--nested;
		if (0==nested)
		{
			BEGIN NRMLSRC;
		}
		return tRPAREN;
	}

<INCONTEXT>[a-zA-Z_][0-9a-zA-Z_]* {
		strncpy(l2clval.Ident, l2ctext, __lc_eMaxIdentifierLength);
		if (moduleTable.IsModuleIdent(l2ctext))
		{
			return tMODULEIDENT;
		}
		else
		{
			return tIDENT;
		}
	}

<INCONTEXT>"," { return tCOMMA; }

<STRING>\' { DumpOut(l2ctext); }
<STRING>\\\" { DumpOut(l2ctext); }
<STRING>\" { BEGIN BeforeString; DumpOut(l2ctext); }

<CHARACTER>\\\' { DumpOut(l2ctext); }
<CHARACTER>\' { BEGIN BeforeString; DumpOut(l2ctext); }

^"#line "[0-9]+.+\n	{
	SetCurLine(l2ctext+6);
	DumpOut(l2ctext);
	lineno++;
	}
^"# "[0-9]+.+\n	{
	SetCurLine(l2ctext+2);
	DumpOut(l2ctext);
	lineno++;
	}

\" { BeforeString = YY_START; BEGIN STRING; DumpOut(l2ctext); }
\' { BeforeString = YY_START; BEGIN CHARACTER; DumpOut(l2ctext); }

"\n" { lineno++; DumpOut(l2ctext); }

[ \t] { DumpOut(l2ctext); }

[0-9]+\.[0-9]?[fFlL] { DumpOut(l2ctext); }
\.[0-9]+[fFlL] { DumpOut(l2ctext); }


. { DumpOut(l2ctext); }

%%

/*
*/
