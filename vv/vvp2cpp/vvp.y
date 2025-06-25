%{
  #include <stdio.h>
  #include <string.h>

  extern int  llex(void);
  extern void lerror(const char*, ...);
  extern void lwarning(const char*, ...);

  extern int  yylex(void);
  extern void yyerror(const char* fmt, ...);
  extern void yywarning(const char* fmt, ...);
#ifdef WIN32
  #include <malloc.h>
#endif

  extern void StartVProp();
  extern void EndVProp();
  extern void StartEProp();
  extern void EndEProp();
  extern void StartMProp();
  extern void EndMProp();

  extern void WriteVDecl();
  extern void WriteEDecl();
  extern void WriteMDecl();

  extern void VNew();

  extern void w(const char* bf);
  extern void wc(const char c);

  unsigned int vretid = 0;
%}

%union {
  char ident[256];
}

%token tIDENT
%token tEPROP tEDGE
%token tVPROP tVDECL
%token tMPROP tMESH
%token tSEMICOLON tEQUALS tCOMMA
%token tVERTEX
%token tIN tWITH tBEFORE tAFTER
%token tNEXT tPREV tANY tFROM
%token tLABEL tVALENCE tIS tNBOF
%token tERASE tREPLACE tSPLICE tMAKE
%token tERASE_FLAGGED tSPLICE_FLAGGED
%token tFOR
%token tSYNCH tADD tREMOVE tTO tMERGE tCLEAR
%token tTICK
%token tVDECLEND
%token tPOPEN tPOPENOLD tPCLOSE tCLOSE tPROPEN tPROPENOLD
%token tPNEXT tPPREV tPSWAP
%token tNEXTI tPREVI tI
%token tNEXT_FLAGGED tPREV_FLAGGED
%token tFLAGGED tFLAG

%type <ident> tIDENT
%%

Translate: Translate TranslationUnit
         |
;
TranslationUnit: EProp
               | EDecl
               | VProp
               | VDecl VTerm
               | MProp
               | MDecl
               | VExpr
               | VLabel
               | VValence
               | VIs
               | VErase
               | VEraseFlagged
               | VReplace
               | VSplice
               | VAssign
               | VFlag
               | For
               | MSynch
               | MAdd
               | MRemove
               | MMerge
               | MClear
;

EProp: tEPROP {StartEProp();} tSEMICOLON {EndEProp();}
;
EDecl: tEDGE {WriteEDecl();}
;

VProp: tVPROP {StartVProp();} tSEMICOLON {EndVProp();}
;

VDecl: tVDECL {WriteVDecl();}
;
VTerm: tSEMICOLON {VNew();}
     | tVDECLEND
;

MProp: tMPROP {StartMProp();} tSEMICOLON {EndMProp();}
;
MDecl: tMESH {WriteMDecl();}
;

VExpr: VAny
     | VFlagged
     | VNext
     | VPrev
     | VNextFlagged
     | VPrevFlagged
     | VNexti
     | VPrevi
     | VOld
     | Path
     | tVERTEX
;

VLabel: tLABEL {w("(");} VExpr {w(")->getLabel()");}
;
VValence: tVALENCE {w("(");} VExpr {w(")->getNeighbourCount()");}
;
VAny: tANY {w("___vvwrappers::Select(");} VExpr {w(")");}
;
VFlagged : tFLAGGED {w("(");} VExpr {w(")->flagged()");}
;
VIs: tIS {w("___vvwrappers::In(");} VExpr tIN {w(", ");} VExpr {w(")");}
;
VNext: tNEXT {w("___vvwrappers::Next(");} VExpr tIN {w(", ");} VExpr {w(")");} 
;
VNextFlagged: tNEXT_FLAGGED {w("(");} VExpr {w(")->next_flagged()");}
;
VPrev: tPREV {w("___vvwrappers::Prev(");} VExpr tIN {w(", ");} VExpr {w(")");}
;
VPrevFlagged: tPREV_FLAGGED {w("(");} VExpr {w(")->prev_flagged()");}
;
VNexti: tNEXTI {w("___vvwrappers::Next(");} tI {w(", ");} VExpr tIN {w(", ");} VExpr {w(")");}
;
VPrevi: tPREVI {w("___vvwrappers::Prev(");} tI {w(", ");} VExpr tIN {w(", ");} VExpr {w(")");}
;
VOld: tTICK {w("(");} VExpr {w(")->getOld()");}
;

VErase: tERASE {w("___vvwrappers::Erase(");} VExpr tFROM {w(", ");} VExpr {w(")");}
;
VEraseFlagged: tERASE_FLAGGED {w("___vvwrappers::EraseFlagged(");} VExpr {w(")");}
;
VReplace: tREPLACE {w("___vvwrappers::Replace(");} Vx tWITH VExpr tIN {w(", ");} VExpr {w(")");}
;
VSplice: tSPLICE {w("___vvwrappers::Splice(");} VExpr SOrder Vx tIN VExpr {w(")");}
;
Vx: VExpr {w(", ");}
  | tFLAG;
SOrder: tBEFORE {w(", ___vvwrappers::PREV, ");}
      | tAFTER {w(", ___vvwrappers::NEXT, ");}
;

VAssign: tMAKE {w("{___vvwrappers::nb.clear();___vvwrappers::nb.push_back(std::make_pair(");} VArgs tNBOF {w(", edge()));(");} VExpr {w(")->nbAssign(___vvwrappers::nb);}");}
;

VFlag: tFLAG {w("___vvwrappers::Flag(");} VExpr {w(", ");} tIN VExpr {w(")");}
;

VArgs: VExpr tCOMMA {w(", edge()));___vvwrappers::nb.push_back(std::make_pair(");} VArgs
     | VExpr
;

For: tFOR tIDENT tIN tIDENT {
       char loop[256];
       sprintf(loop, "for (%s.loopStart(); %s.loopNotDone(); %s.loopNext()) {vertex::VPtr %s = %s.getCurrent();", $4, $4, $4, $2, $4);
       w(loop);
     }
   | tFOR tIDENT tIN {char vret[256]; sprintf(vret, "vertex::VPtr ___vret%d = ", ++vretid); w(vret);} VOld {
       char loop[256];
       sprintf(loop, ";for (___vret%d->getOld().loopStart(); ___vret%d->getOld().loopNotDone(); ___vret%d.loopNext()) {vertex::VPtr %s = ___vret%d.getCurrent();", vretid, vretid, vretid, $2, vretid);
       w(loop);
     }
   | tFOR tIDENT tIN {char vret[256]; sprintf(vret, "vertex::VPtr ___vret%d = ", ++vretid); w(vret);} VAny {
       char loop[256];
       sprintf(loop, ";for (___vret%d->getOld().loopStart(); ___vret%d->getOld().loopNotDone(); ___vret%d.loopNext()) {vertex::VPtr %s = ___vret%d.getCurrent();", vretid, vretid, vretid, $2, vretid);
       w(loop);
     }
   | tFOR tIDENT tIN {char vret[256]; sprintf(vret, "vertex::VPtr ___vret%d = ", ++vretid); w(vret);} VNext {
       char loop[256];
       sprintf(loop, ";for (___vret%d->getOld().loopStart(); ___vret%d->getOld().loopNotDone(); ___vret%d.loopNext()) {vertex::VPtr %s = ___vret%d.getCurrent();", vretid, vretid, vretid, $2, vretid);
       w(loop);
     }
   | tFOR tIDENT tIN {char vret[256]; sprintf(vret, "vertex::VPtr ___vret%d = ", ++vretid); w(vret);} VPrev {
       char loop[256];
       sprintf(loop, ";for (___vret%d->getOld().loopStart(); ___vret%d->getOld().loopNotDone(); ___vret%d.loopNext()) {vertex::VPtr %s = ___vret%d.getCurrent();", vretid, vretid, vretid, $2, vretid);
       w(loop);
     }
   | tFOR tIDENT tIN {char vret[256]; sprintf(vret, "vertex::VPtr ___vret%d = ", ++vretid); w(vret);} VNexti {
       char loop[256];
       sprintf(loop, ";for (___vret%d->getOld().loopStart(); ___vret%d->getOld().loopNotDone(); ___vret%d.loopNext()) {vertex::VPtr %s = ___vret%d.getCurrent();", vretid, vretid, vretid, $2, vretid);
       w(loop);
     }
   | tFOR tIDENT tIN {char vret[256]; sprintf(vret, "vertex::VPtr ___vret%d = ", ++vretid); w(vret);} VPrevi {
       char loop[256];
       sprintf(loop, ";for (___vret%d->getOld().loopStart(); ___vret%d->getOld().loopNotDone(); ___vret%d.loopNext()) {vertex::VPtr %s = ___vret%d.getCurrent();", vretid, vretid, vretid, $2, vretid);
       w(loop);
     }
;

MSynch: tSYNCH tIDENT {
          char statement[256];
          sprintf(statement, "(%s).synchronise()", $2);
          w(statement);
      }
;
MAdd: tADD {w("___vvwrappers::Add(");} VExpr {w(",");} tTO tIDENT {
        char statement[256];
        sprintf(statement, "%s)", $6);
        w(statement);
      }
;
MRemove: tREMOVE {w("___vvwrappers::Remove(");} VExpr {w(",");} tFROM tIDENT {
           char statement[256];
           sprintf(statement, " %s)", $6);
           w(statement);
         }
;
MMerge: tMERGE tIDENT tWITH tIDENT {
          char statement[256];
          sprintf(statement, "(%s).merge(%s)", $2, $4);
          w(statement);
        }
;
MClear: tCLEAR tIDENT {
          char statement[256];
          sprintf(statement, "(%s).clear()", $2);
          w(statement);
        }
;

Path: tPOPEN     {w("algebra::path<vertex>(false, ");} VExpr tCOMMA {w(", ");} VExpr tCLOSE {w(", ");} PArgs tPCLOSE {w("algebra::opStop)");}
    | tPOPENOLD  {w("algebra::path<vertex>(true, ");}  VExpr tCOMMA {w(", ");} VExpr tCLOSE {w(", ");} PArgs tPCLOSE {w("algebra::opStop)");}
    | tPROPEN    {w("algebra::pathref<vertex>(false, ");} VExpr tCOMMA {w(", ");} VExpr tCLOSE {w(", ");} PArgs tPCLOSE {w("algebra::opStop)");}
    | tPROPENOLD {w("algebra::pathref<vertex>(true, ");}  VExpr tCOMMA {w(", ");} VExpr tCLOSE {w(", ");} PArgs tPCLOSE {w("algebra::opStop)");}
;
PArgs: POp PArgs
     | POp
;
POp: tPNEXT {w("algebra::opNext, ");}
   | tPPREV {w("algebra::opPrev, ");}
   | tPSWAP {w("algebra::opSwap, ");}
;
