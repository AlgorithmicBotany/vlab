%{

#define YYDEBUG 1

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <lparams.h>

#include "module.h"
#include "contextdata.h"
#include "production.h"

extern int l2clex(void);
extern void l2cerror(const char*, ...);
extern void l2cwarning(const char*, ...);
extern void StartGenerateProduce(const char*);
extern void ParameterCast();
extern void EndParameterCast();
extern void EndGenerateProduce();
extern void StartAxiom();
extern void EndAxiom();
extern void StartProduce();
extern void EndProduce();
extern void StartNProduce();
extern void EndNProduce();
extern void ExpandStart();
extern void ExpandStartEach();
extern void ExpandEndEach();
extern void ExpandEnd();
extern void GeneratePred(FormalModuleDtList*, FormalModuleDtList*, FormalModuleDtList*);
extern void StartConsider();
extern void StartIgnore();
extern void EndConsider();
extern void EndIgnore();
extern void StartVerify();
extern void EndVerify();
extern void AppendConIgnModule(const char*);
extern void StartDerivLength();
extern void EndDerivLength();
extern void StartMaxDepth();
extern void EndMaxDepth();
extern void SwitchToInterpretation();
extern void SwitchToDecomposition();
extern void SwitchToProduction();
extern ProductionType ProductionMode();
extern void StartRingLsystem();
extern void EndRingLsystem();
extern void StartGroup(int);
extern void StartGGroup(int);
extern void EndGroup();
extern void StartVGroup(int);
extern void EndVGroup();
extern void StartGProduce();
extern void EndGProduce();
extern void StartPropensity();
extern void GenerateInRightContext(const FormalModuleDtList* pContextModules);
extern void GenerateInLeftContext(FormalModuleDtList* pContextModules);
extern void GenerateInNewRightContext(const FormalModuleDtList* pContextModules);
extern void GenerateInNewLeftContext(FormalModuleDtList* pContextModules);

extern ModuleTable moduleTable;
extern ProductionTable productionTable;
extern HomomorphismTable interpretationTable;
extern HomomorphismTable decompositionTable;

static int counter = __lc_eFirstModuleId;

int ModuleCounter()
{ return counter; }

%}

%union
{
  char Ident[__lc_eMaxIdentifierLength+1];
  int ModuleId;
  int Integer;
  ParametersList ParamsList;
  FormalModuleDt* pFormalModuleData;
  FormalModuleDtList* pFormalModuleDataList;
  ContextData context;
};

%token tAXIOM tMODULE tPRODUCE tNPRODUCE tSTART tSTARTEACH tENDEACH tEND tMAXDEPTH
%token tPROPENSITY tGGROUP
%token tCONSIDER tIGNORE tDERIVLENGTH tRINGLSYSTEM
%token tINTERPRETATION tDECOMPOSITION tPRODUCTION
%token tGROUP tENDGROUP tVGROUP tENDVGROUP tCOLON
%token tLPAREN tRPAREN tSEMICOLON tCOMMA
%token tLESSTHAN tLEFTSHIFT tGREATERTHAN tRIGHTSHIFT tENDPRODPROTO
%token tEQUALS
%token <Integer> tINTEGER
%token <Ident> tIDENT tMODULEIDENT
%token tERROR tVERIFYSTRING
%token tINRIGHTCONTEXT tINLEFTCONTEXT tINNEWRIGHTCONTEXT tINNEWLEFTCONTEXT

%type <ParamsList> Parameters
%type <pFormalModuleData> FormalModule
%type <pFormalModuleDataList> StrictPred FormalModules
%type <context> LeftContext RightContext


%%


Translate: Translate TranslationUnit
	|
	;


TranslationUnit: ModuleDeclaration
	| ConsiderStatement
	| IgnoreStatement
	| VerifyStatement
	| AxiomStatement
	| ProductionPredecessor
	| ProduceStatement
	| NProduceStatement
	| GProduceStatement
	| DerivLength
	| RingLsystem
	| tSTART
	{ ExpandStart(); }
	| tSTARTEACH
	{ ExpandStartEach(); }
	| tENDEACH
	{ ExpandEndEach(); }
	| tEND
	{ ExpandEnd(); }
	| tINTERPRETATION
	{ SwitchToInterpretation(); }
	| tDECOMPOSITION
	{ SwitchToDecomposition(); }
	| tPRODUCTION
	{ SwitchToProduction(); }
	| MaxDepthStatement
	| GroupStart
	| tENDGROUP
	{ EndGroup(); }
	| VGroupStart
	| tENDVGROUP
	{ EndVGroup(); }
	| InRightContext
	| InLeftContext
	| InNewRightContext
	| InNewLeftContext
	;

ModuleDeclaration: tMODULE tIDENT tLPAREN Parameters tRPAREN tSEMICOLON
	{ 
		ModuleDeclaration mdecl($2, &($4), counter++);
		mdecl.GenerateModId();
		moduleTable.Add(mdecl);
	}
	| tMODULE tIDENT tLPAREN Parameters tRPAREN tEQUALS tINTEGER tSEMICOLON
	{ 
		counter = $7;
		ModuleDeclaration mdecl($2, &($4), counter++);
		mdecl.GenerateModId();
		moduleTable.Add(mdecl);
	}
	| tMODULE tIDENT tSEMICOLON
	{ 
		ModuleDeclaration mdecl($2, NULL, counter++);
		mdecl.GenerateModId();
		moduleTable.Add(mdecl);
	}
	| tMODULE tIDENT tEQUALS tINTEGER tSEMICOLON
	{ 
		counter = $4;
		ModuleDeclaration mdecl($2, NULL, counter++);
		mdecl.GenerateModId();
		moduleTable.Add(mdecl);
	}
	;

InRightContext: tINRIGHTCONTEXT tLPAREN FormalModules tRPAREN
	{
		GenerateInRightContext($3);
	}
	;

InNewRightContext: tINNEWRIGHTCONTEXT tLPAREN FormalModules tRPAREN
	{
		GenerateInNewRightContext($3);
	}
	;

InLeftContext: tINLEFTCONTEXT tLPAREN FormalModules tRPAREN
	{
		GenerateInLeftContext($3);
	}
	;

InNewLeftContext: tINNEWLEFTCONTEXT tLPAREN FormalModules tRPAREN
	{
		GenerateInNewLeftContext($3);
	}
	;

ConsiderStatement: tCONSIDER { StartConsider(); } ModuleList tSEMICOLON
	{ EndConsider(); }
	;

IgnoreStatement: tIGNORE { StartIgnore(); } ModuleList tSEMICOLON
	{ EndIgnore(); }
	| tIGNORE tSEMICOLON { StartIgnore(); EndIgnore(); }
	;

VerifyStatement: tVERIFYSTRING { StartVerify(); } ModuleList tSEMICOLON
	{ EndVerify(); }
	;

ModuleList: tMODULEIDENT
	{ AppendConIgnModule($1); }
	| ModuleList tMODULEIDENT
	{ AppendConIgnModule($2); }
	;

DerivLength: tDERIVLENGTH { StartDerivLength(); } tSEMICOLON
	{ EndDerivLength(); }
	;

RingLsystem: tRINGLSYSTEM { StartRingLsystem(); } tSEMICOLON
	{ EndRingLsystem(); }
	;

MaxDepthStatement: tMAXDEPTH { StartMaxDepth(); } tSEMICOLON
	{ EndMaxDepth(); }
	;

ProductionPredecessor: LeftContext StrictPred RightContext tENDPRODPROTO
{ 
	if ($1.HasNewContext() && $3.HasNewContext())
		l2cerror("Production cannot have both left new context "
				"and right new context");

	ProductionProto* pNew = new ProductionProto(&($1), $2, &($3));
	pNew->Generate();
	switch (ProductionMode())
	{ 
		case eInterpretation :
			interpretationTable.Add(pNew);
			break;
		case eDecomposition :
			decompositionTable.Add(pNew);
			break;
		case eProduction :
			productionTable.Add(pNew);
			break;
	}		
}
	| StrictPred RightContext tENDPRODPROTO
{ 
	ProductionProto* pNew = new ProductionProto(NULL, $1, &($2));
	pNew->Generate();
	switch (ProductionMode())
	{ 
		case eInterpretation :
			interpretationTable.Add(pNew);
			break;
		case eDecomposition :
			decompositionTable.Add(pNew);
			break;
		case eProduction :
			productionTable.Add(pNew);
			break;
	}
}
	| LeftContext StrictPred tENDPRODPROTO
{ 
	ProductionProto* pNew = new ProductionProto(&($1), $2, NULL);
	pNew->Generate();
	switch (ProductionMode())
	{ 
		case eInterpretation :
			interpretationTable.Add(pNew);
			break;
		case eDecomposition :
			decompositionTable.Add(pNew);
			break;
		case eProduction :
			productionTable.Add(pNew);
			break;
	}
}
	| StrictPred tENDPRODPROTO
	{ 
		ProductionProto* pNew = new ProductionProto(NULL, $1, NULL);
		pNew->Generate();
		switch (ProductionMode())
		{ 
		case eInterpretation :
			interpretationTable.Add(pNew);
			break;
		case eDecomposition :
			decompositionTable.Add(pNew);
			break;
		case eProduction :
			productionTable.Add(pNew);
			break;
		}
	}
	;

LeftContext : FormalModules tLEFTSHIFT
	{ 
		$$.SetContext(NULL);
		$$.SetNewContext($1);
	}
	| FormalModules tLESSTHAN
	{ 
		$$.SetContext($1);
		$$.SetNewContext(NULL);
	}
	;

RightContext : tGREATERTHAN FormalModules
	{ 
		$$.SetContext($2);
		$$.SetNewContext(NULL);
	}
	| tRIGHTSHIFT FormalModules
	{ 
		$$.SetContext(NULL);
		$$.SetNewContext($2);
	}
	;

StrictPred: FormalModules 
	{ 
	$$ = $1;
	}
	;

FormalModules: FormalModules FormalModule
	{ 
		$$ = $1; $$->Add($2); 
	}
	| FormalModule
	{ 
		$$ = new FormalModuleDtList($1); 
	}
	;

FormalModule: tMODULEIDENT tLPAREN Parameters tRPAREN
	{ 
	$$ = new FormalModuleDt($1, &($3)); 
	}
	| tMODULEIDENT
	{ 
	$$ = new FormalModuleDt($1,NULL);
 	}

AxiomStatement: tAXIOM { StartAxiom(); } ParametricWord tSEMICOLON
	{ EndAxiom(); }
	;

GProduceStatement: tPROPENSITY { StartPropensity(); } tPRODUCE { StartGProduce(); } ParametricWord tSEMICOLON { EndGProduce(); }
	;

ProduceStatement: tPRODUCE { StartProduce(); } ParametricWord tSEMICOLON
	{ EndProduce(); }
	;

NProduceStatement: tNPRODUCE { StartNProduce(); } ParametricWord tSEMICOLON
	{ EndNProduce(); }
	;


ParametricWord: ParametricWord ParametricLetter { }
	|
	;


ParametricLetter: tMODULEIDENT { StartGenerateProduce($1); } tLPAREN { ParameterCast(); } CallParameters { EndParameterCast(); } tRPAREN
	{ EndGenerateProduce(); }
	| tMODULEIDENT
	{ StartGenerateProduce($1); EndGenerateProduce(); }
	;

CallParameters: CallParameters { EndParameterCast(); } tCOMMA { ParameterCast(); }
	|
	;

Parameters: Parameters tCOMMA tIDENT
	{ 
	  if ($1.count == __lc_eMaxParams)
	    l2cerror("Too many parameters");
	  strcpy($1.Params[$1.count], $3);
	  $1.count++;
	  $$ = $1;
	}
	| tIDENT
	{ 
    strcpy($$.Params[0], $1);
	  $$.count = 1;
	}
	|
	{ 
	$$.count = 0; 
}
	;

GroupStart: tGROUP tINTEGER tCOLON
	{ StartGroup($2); }
	| tGGROUP tINTEGER tCOLON
	{ StartGGroup($2); }
	;

VGroupStart: tVGROUP tINTEGER tCOLON
	{ StartVGroup($2); }
	;


%%
