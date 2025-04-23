/* yacc.y								   */
/*									   */
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

%{
  int yylex();
  //  int  my_yyerror(char *s, char *pat1, char *pat2);
  void yyerror(const char* s);
%}

%{
#include "rayshade.h"

#include "symtab.h"
#include "builtin.h"

#include "libsurf/atmosphere.h"
#include "libsurf/surface.h"
#include "libtext/texture.h"
#include "libimage/image.h"
#include "libobj/geom.h"
#include "liblight/light.h"
#include "options.h"
#include "stats.h"
#include "viewing.h"

#include "libobj/blob.h"
#include "libobj/box.h"
#include "libobj/cone.h"
#include "libobj/csg.h"
#include "libobj/cylinder.h"
#include "libobj/disc.h"
#include "libobj/grid.h"
#include "libobj/hf.h"
#include "libobj/instance.h"
#include "libobj/list.h"
#include "libobj/plane.h"
#include "libobj/poly.h"
#include "libobj/sphere.h"
#include "libobj/torus.h"
#include "libobj/triangle.h"

#include "liblight/point.h"
#include "liblight/infinite.h"
#include "liblight/spot.h"
#include "liblight/jittered.h"
#include "liblight/extended.h"

#include "libtext/blotch.h"
#include "libtext/bump.h"
#include "libtext/checker.h"
#include "libtext/cloud.h"
#include "libtext/fbm.h"
#include "libtext/fbmbump.h"
#include "libtext/gloss.h"
#include "libtext/imagetext.h"
#include "libtext/marble.h"
#include "libtext/mount.h"
#include "libtext/sky.h"
#include "libtext/stripe.h"
#include "libtext/windy.h"
#include "libtext/wood.h"

#include "libsurf/fog.h"
#include "libsurf/fogdeck.h"
#include "libsurf/mist.h"

#include "libcommon/rotate.h"
#include "libcommon/scale.h"
#include "libcommon/translate.h"
#include "libcommon/xform.h"
#include "libcommon/transform.h"

Geom *NewAggregate();
char yyfilename[BUFSIZ];			/* Input filename */
GeomList *Defstack;				/* Geom definition stack. */
int Npoints = 0;				/* # of points in Polypoints */
Surface *tmpsurf;				/* Working surface */
SurfList *CurSurf;
Texture *CurText;				/* Working list of textures */
ImageText *Imagetext;				/* Working image texture */
Trans *TransHead, *TransTail;			/* Linked list of current transformations */
Atmosphere *CurEffect = (Atmosphere *)NULL;	/* Current atmos. effects */ 
PointList *Polypoints;				/* List of vertices */
MetaList *Metapoints, *Metapoint;
extern FILE *yyin;				/* input file pointer */
extern int yylineno;				/* Current line # in file */
extern Atmosphere *AtmosEffects;		/* atmospheric effects */
extern Medium TopMedium;			/* "air" */
extern void	GeomAddToDefined(Geom *obj),
		LightAddToDefined(Light *light),
		SurfaceAddToDefined(Surface *surf);
extern Surface	*SurfaceGetNamed(char *name);
extern Geom 	*GeomGetNamed(char *name);
%}
%union {
	char *c;
	int i;
	Float d;
	Vector v;
	Vec2d uv;
	Color col;
	Atmosphere *atmos;
	Light *light;
	Surface *surf;
	Geom *obj;
	Texture *text;
	Mapping *map;
	Trans *trans;
	Expr *e;
	SymtabEntry *sym;
}
%token <d> tFLOAT
%token <c> tSTRING tFILENAME
%token tAPERTURE tAPPLYSURF
%token tBACKGROUND tBLOB tBLOTCH tBOX tBUMP tCONE tCYL tDIRECTIONAL tCURSURF
%token tEXTENDED tEYEP tFBM tFBMBUMP tFOCALDIST tFOG tFOGDECK tFOV tGLOSS tGRID
%token tHEIGHTFIELD tLIGHT tLIST tLOOKP tMARBLE tMAXDEPTH tMIST
%token tJITTER tNOJITTER tDEFINE
%token tOBJECT tOUTFILE  tSKY tDISC tOPENDISC tDIFFERENCE tUNION tINTERSECT
%token tPLANE tPOINT tPOLY tROTATE tSPOT tPRINT
%token tSCALE tSCREEN tSPHERE tSURFACE
%token tTHRESH tTRANSLATE tTRANSFORM tTRIANGLE tTRIANGLEUV tUP tEND
%token tTEXTURE tCHECKER tWOOD tCONTRAST tCUTOFF tCLOUD
%token tAMBIENT tDIFFUSE tREFLECT tTRANSP tSPECULAR tSPECPOW
%token tINDEX tATMOSPHERE tNOSHADOW tAREA tTRANSLU tTORUS
%token tEYESEP tSHADOWTRANSP tREPORT tVERBOSE tQUIET tWINDOW tCROP tSTRIPE
%token tMAP tUV tSPHERICAL tCYLINDRICAL tPLANAR
%token tIMAGE tSMOOTH tCOMPONENT tTEXTSURF tRANGE tTILE tSTARTTIME tFRAMELENGTH
%token tNAME tFILTER tGAUSS tBODY tSAMPLE tEXTINCT tWINDY tMOUNT
%token tSHUTTER tFRAMES
%type <c> Filename
%type <e> AnimExpr MExpr ParenExpr
%type <d> Expr Float
%type <v> Vector
%type <uv> Vec2d
%type <col> Color Intensity Lightdef
%type <text> Texturetype
%type <i> SurfCompName IExpr CombineOp
%type <atmos> EffectType
%type <light> LightType
%type <obj> PrimType Primitive TransTextObj
%type <obj> Csg Aggregate Object TransObj ObjType
%type <obj> Blob Box Cone Cylinder Disc Opendisc HeightField Plane Poly
%type <obj> Sphere Triangle Torus AggregateType List Grid AggregateCreate
%type <obj> NamedObject
%type <surf> Surface OptSurface NamedSurf
%type <surf> SurfSpec ModifyNamedSurf
%type <map> Mapping MapMethod OptMapping
%type <trans> TransformType
%type <sym> Symtabent

%left '+' '-'
%left '*' '/' '%'
%left UMINUS
%right '^'
%%
Items		: /* empty */
		| Items Item
		;
Item		: Eyep
		| Lookp
		| Up
		| Fov
		| Screen
		| Window
		| Crop
		| Report
		| Aperture
		| Focaldist
		| Eyesep
		| Maxdepth
		| Sample
		| Filter
		| Contrast
		| Cutoff
		| Background
		| Shadowtransp
		| Light
		| SurfDef
		| CurSurf
		| Outfile
		| Instance
		| NameObject
		| GlobalEffects
		| Define
		| Frames
		| Starttime
		| Shutter
		| Framelength
    		| Print
		;
Instance	: TransTextObj
		{
			if ($1) {
				/*
				 * Add instance to current object.
				 */
				$1->next = Defstack->obj->next;
				Defstack->obj->next = $1;
			}
		}
TransTextObj	: TransObj Textures
		{
			if ($$ && CurText) {
				$$->texture = TextAppend(CurText, $$->texture);
			}
			CurText = (Texture *)NULL;
		}
		;	
TransObj	: Object Transforms
		{
			$$ = $1;
			if ($$ != (Geom *)NULL) {
				if (TransHead) {
					$$->trans = TransHead;
					$$->transtail = TransTail;
					/*
					 * We compose non-animated tranformation lists,
					 * so we're only animated if it's one long,
					 * or it's animated itself.
					 */
					if ($$->trans->assoc || $$->trans->next)
						/* geometry is animated...*/
						$$->animtrans = TRUE;
				}
			}
		}
		;
Object		: ObjType
		{
			if ($$)
				StatsAddRep($$);
		}
		| NamedObject
		;
ObjType		: Primitive
		| Aggregate
		;
Primitive	: PrimType
		{
			if ($$)
				$$->prims = 1;	/* one primitive */
		}
		;
PrimType	: Plane
		| Sphere
		| Box
		| Triangle
		| Cylinder
		| Cone
		| Poly
		| HeightField
		| Disc
		| Opendisc
		| Torus
		| Blob
		;
NameObject	: tNAME tSTRING TransTextObj
		{
			if ($3) {
				$3->name = $2;
				GeomAddToDefined($3);
			}
		};
Aggdefs		: Aggdefs Aggdef
		|
		;
Aggdef		: Instance
		| SurfDef
		| CurSurf
		| NameObject
		;
Textures	: Textures Texture
		|
		;
Texture		: tTEXTURE Texturetype Transforms
		{
			if ($2 != (Texture *)NULL) {
				/*
				 * Set transformation information.
				 */
				if (TransHead) {
					$2->trans = TransHead;
					/*
					 * We compose non-animated tranformation lists,
					 * so we're only animated if it's one long,
					 * or it's animated itself.
					 */
					if ($2->trans->assoc || $2->trans->next)
						/* texture transformation is animated...*/
						$2->animtrans = TRUE;
				}
				/*
				 * Walk to the end of list of textures and
				 * append new texture.  This is done so that
				 * textures are applied in the expected order.
				 */
				CurText = TextAppend($2, CurText);
			}
		}
		;
Texturetype	: tCHECKER Surface
		{
			$$ = TextCheckerCreate($2);
		}
		| tBLOTCH Expr Surface
		{
			$$ = TextBlotchCreate($2, $3);
		}
		| tBUMP Expr
		{
			$$ = TextBumpCreate($2);
		}
		| tMARBLE
		{
			$$ = TextMarbleCreate((char *)NULL);
		}
		| tMARBLE Filename
		{
			$$ = TextMarbleCreate($2);
		}
		| tFBM Expr Expr Expr Expr IExpr Expr
		{
			$$ = TextFBmCreate($2, $3, $4, $5, $6, $7,
						(char *)NULL);
		}
		| tFBM Expr Expr Expr Expr IExpr Expr Filename
		{
			$$ = TextFBmCreate($2, $3, $4, $5, $6, $7, $8);
		}
		| tFBMBUMP Expr Expr Expr Expr IExpr 
		{
			$$ = TextFBmBumpCreate($2, $3, $4, $5, $6);
		}
		| tWOOD
		{
			$$ = TextWoodCreate();
		}
		| tGLOSS Expr 
		{
			$$ = TextGlossCreate($2);
		}
		| tCLOUD Expr Expr Expr IExpr Expr Expr Expr
		{
			$$ = TextCloudCreate($2, $3, $4, $5, $6, $7, $8);
		}
		| tSKY Expr Expr Expr IExpr Expr Expr
		{
			$$ = TextSkyCreate($2, $3, $4, $5, $6, $7);
		}
		| ImageText
		{
			/*
			 * Image texturing has so many options
			 * that specification is keyword-based.
			 */
			if (Imagetext->image == (Image *)NULL)
				$$ = (Texture *)NULL;
			else
				$$ = TextCreate((TextRef)Imagetext, ImageTextApply);
			Imagetext = (ImageText *)NULL;
		}
		| tSTRIPE Surface Expr Expr OptMapping
		{
			$$ = TextStripeCreate($2, $3, $4, $5);
		}
		| tWINDY Expr Expr Expr Expr IExpr Expr Expr Expr
		{
			$$ = TextWindyCreate($2, $3, $4, $5, $6, $7, $8, $9);
		}
		| tMOUNT Filename Expr Expr
		{
			$$ = TextMountCreate($2, $3, $4);
		}
		;
ImageText	: ImageTextType ImageTextOptions
		;
ImageTextType	: tIMAGE Filename
		{
			Imagetext = ImageTextCreate($2);
		}
		;
ImageTextOptions: ImageTextOptions ImageTextOption
		| /* EMPTY */
		;
ImageTextOption: tCOMPONENT SurfCompName
		{
			/* set texture to modify given component */	
			ImageTextSetComponent(Imagetext, $2);
		}
		| tTILE Expr Expr
		{
			Imagetext->tileu = $2;
			Imagetext->tilev = $3;
		}
		| tTEXTSURF Surface
		{
			Imagetext->surf = $2;
		}
		| tRANGE Expr Expr
		{
			Imagetext->hi = $2;
			Imagetext->lo = $3;
		}
		| tSMOOTH
		{
			Imagetext->smooth = TRUE;
		}
		| Mapping
		{
			Imagetext->mapping = $1;
		};
NamedObject	: tOBJECT Surface tSTRING
		{
			Geom *otmp;
			/*
			 * Create an instance of the named object.
			 */
			otmp = GeomGetNamed($3);
			if (otmp == (Geom *)NULL)
				RLerror(RL_PANIC,
				  "There is no object named \"%s\".", $3);
			$$ = GeomInstanceCreate(otmp);
			$$->surf = $2;
			$$->prims = otmp->prims;
		}
		| tOBJECT tSTRING
		{
			Geom *otmp;

			otmp = GeomGetNamed($2);
			if (otmp == (Geom *)NULL)
				RLerror(RL_PANIC,
				  "There is no object named \"%s\".", $2);
			$$ = GeomInstanceCreate(otmp);
			$$->surf = CurSurf->surf;
			$$->prims = otmp->prims;
		};
Transforms	: Transforms PostTransform
		| /* empty */
		{
			TransHead = TransTail = (Trans *)NULL;
		};
PostTransform	: TransformType
		{
			if (TransHead == (Trans *)NULL) {
				/* we're the list, head and tail */
				TransHead = TransTail = $1;
			} else {
				if ($1->animated || TransTail->animated) {
					/* new tail */
					$1->prev = TransTail;
					TransTail->next = $1;
					TransTail = $1;
				} else {
					/* collapse with tail */
					TransCompose(TransTail, $1, TransTail);
					TransFree($1);
				}
			}
		}
		;
TransformType	: tSCALE AnimExpr AnimExpr AnimExpr
		{
			$$ = TransScaleCreate();
			TransScaleSetX($$, $2);
			TransScaleSetY($$, $3);
			TransScaleSetZ($$, $4);
			if (!$$->animated)
				TransPropagate($$);
				
		}
		| tTRANSLATE AnimExpr AnimExpr AnimExpr
		{
			$$ = TransTranslateCreate();
			TransTranslateSetX($$, $2);
			TransTranslateSetY($$, $3);
			TransTranslateSetZ($$, $4);
			if (!$$->animated)
				TransPropagate($$);
		}
		| tROTATE AnimExpr AnimExpr AnimExpr AnimExpr
		{
			$$ = TransRotateCreate();
			TransRotateSetX($$, $2);
			TransRotateSetY($$, $3);
			TransRotateSetZ($$, $4);
			TransRotateSetTheta($$, $5);
			if (!$$->animated)
				TransPropagate($$);
		}
		| tTRANSFORM	AnimExpr AnimExpr AnimExpr
				AnimExpr AnimExpr AnimExpr
				AnimExpr AnimExpr AnimExpr
		{
			$$ = TransXformCreate();
			TransXformSetX0($$, $2);
			TransXformSetY0($$, $3);
			TransXformSetZ0($$, $4);
			TransXformSetX1($$, $5);
			TransXformSetY1($$, $6);
			TransXformSetZ1($$, $7);
			TransXformSetX2($$, $8);
			TransXformSetY2($$, $9);
			TransXformSetZ2($$, $10);
			if (!$$->animated)
				TransPropagate($$);
		}
		| tTRANSFORM	AnimExpr AnimExpr AnimExpr
				AnimExpr AnimExpr AnimExpr
				AnimExpr AnimExpr AnimExpr
				AnimExpr AnimExpr AnimExpr
		{
			$$ = TransXformCreate();
			TransXformSetX0($$, $2);
			TransXformSetY0($$, $3);
			TransXformSetZ0($$, $4);
			TransXformSetX1($$, $5);
			TransXformSetY1($$, $6);
			TransXformSetZ1($$, $7);
			TransXformSetX2($$, $8);
			TransXformSetY2($$, $9);
			TransXformSetZ2($$, $10);
			TransXformSetXt($$, $11);
			TransXformSetYt($$, $12);
			TransXformSetZt($$, $13);
			if (!$$->animated)
				TransPropagate($$);
		};
Eyep		: tEYEP Vector Transforms
		{
			Camera.pos = $2;
			/*
			 * Eye can be transformed...
			if (CurMatrix) {
				PointTransform(&Camera.pos, CurMatrix);
				free((voidstar)CurMatrix);
				CurMatrix = (Matrix*)NULL;
			}
			 */
		}
		;
Lookp		: tLOOKP Vector
		{
			Camera.lookp = $2;
		}
		;
Up		: tUP Vector
		{
			Camera.up = $2;
		}
		;
Fov		: tFOV Expr Expr
		{
			Camera.hfov = $2;
			Camera.vfov = $3;
		}
		| tFOV Expr
		{
			Camera.hfov = $2;
			Camera.vfov = UNSET;
		}
		;
Sample		: tSAMPLE IExpr tJITTER
		{
			if (!Options.samples_set)
				Options.samples = $2;
			if (!Options.jitter_set)
				Options.jitter = TRUE;
		}
		| tSAMPLE IExpr tNOJITTER
		{
			if (!Options.samples_set)
				Options.samples = $2;
			if (!Options.jitter_set)
				Options.jitter = FALSE;
		}
		| tSAMPLE IExpr
		{
			if (!Options.samples_set)
				Options.samples = $2;
		}
		;
Filter		: tFILTER tBOX Expr
		{
			Options.gaussian = FALSE;
			Options.filterwidth = $3;
		}
		| tFILTER tBOX
		{
			Options.gaussian = FALSE;
		}	
		| tFILTER tGAUSS Expr
		{
			Options.gaussian = TRUE;
			Options.filterwidth = $3;
		}
		| tFILTER tGAUSS
		{
			Options.gaussian = TRUE;
		};
Starttime	: tSTARTTIME Expr
		{
			Options.starttime = $2;
		};
Frames		: tFRAMES IExpr
		{
			if (!Options.totalframes_set)
				Options.totalframes = $2;
		};
Framelength	: tFRAMELENGTH Expr
		{
			Options.framelength = $2;
		};
Shutter		: tSHUTTER Expr
		{
			Options.shutterspeed = $2;
		};
Contrast	: tCONTRAST Expr Expr Expr
		{
			if (!Options.contrast_set) {
				Options.contrast.r = $2;
				Options.contrast.g = $3;
				Options.contrast.b = $4;
			}
		}
		;
Cutoff		: tCUTOFF Intensity
		{
			if (!Options.cutoff_set)
				Options.cutoff = $2;
		}
		;
Screen		: tSCREEN IExpr IExpr 
		{
			if (!Options.resolution_set) {
				Screen.xres = $2;
				Screen.yres = $3;
			}
		}
		;
Window		: tWINDOW IExpr IExpr IExpr IExpr
		{
			if (!Options.window_set) {
				Options.window[LOW][X] = $2;
				Options.window[HIGH][X] = $3;
				Options.window[LOW][Y] = $4;
				Options.window[HIGH][Y] = $5;
				/*
				 * We must let ViewingSetup know
				 * that a window has been defined.
				 */
				Options.window_set = TRUE;
			}
		}
		;
Crop		: tCROP Expr Expr Expr Expr
		{
			if (!Options.crop_set) {
				Options.crop[LOW][X] = $2;
				Options.crop[HIGH][X] = $3;
				Options.crop[LOW][Y] = $4;
				Options.crop[HIGH][Y] = $5;
			}
		}
		;
Report		: tREPORT Verbose Quiet IExpr Filename
		{
			if (!Options.freq_set)
				Options.report_freq = $4;
			if (Options.statsname == (char *)NULL)
				Options.statsname = strsave($5);
		}
		| tREPORT Verbose Quiet IExpr
		{
			if (!Options.freq_set)
				Options.report_freq = $4;
		}
		| tREPORT Verbose Quiet Filename
		{
			if (Options.statsname == (char *)NULL)
				Options.statsname = strsave($4);
		}
		| tREPORT Verbose Quiet
		;
Verbose		: tVERBOSE
		{ Options.verbose = TRUE; }
		|
		;
Quiet		: tQUIET
		{ Options.quiet = TRUE; }
		|
		;
Aperture	: tAPERTURE Expr
		{
			Camera.aperture = $2;
		}
		;
Focaldist	: tFOCALDIST Expr
		{
			Camera.focaldist = $2;
		}
		;
Eyesep		: tEYESEP Expr
		{
			if (!Options.eyesep_set)
				Options.eyesep = $2;
		}
		;
Maxdepth	: tMAXDEPTH IExpr
		{
			if (!Options.maxdepth_set)
				Options.maxdepth = $2;
		}
		;
Background	: tBACKGROUND Color
		{
			Screen.background = $2;
		}
		;
Shadowtransp	: tSHADOWTRANSP
		{
			Options.shadowtransp = !Options.shadowtransp;
		}
		;
Light		: LightType
		{
			LightAddToDefined($1);
		}
		| LightType tNOSHADOW
		{
			$1->shadow = FALSE;
			LightAddToDefined($1);
		}
		| tLIGHT Intensity tAMBIENT
		{
			Options.ambient = $2;
		}
		| Lightdef tAREA Vector Vector IExpr Vector IExpr
		{
			extern void AreaLightCreate(
				Color *color, Vector *corner, Vector *u, int usamp, Vector *v, int vsamp, int shadow);
			/* Area light is strange in that the
			 * Creation routine does the installation.
			 */
			AreaLightCreate(&$1, &$3, &$4, $5, &$6, $7, TRUE);
		}
		| Lightdef tAREA Vector Vector IExpr Vector IExpr tNOSHADOW
		{
			extern void AreaLightCreate(
				Color *color, Vector *corner, Vector *u, int usamp, Vector *v, int vsamp, int shadow);
			/* Area light is strange in that the
			 * Creation routine does the installation.
			 */
			AreaLightCreate(&$1, &$3, &$4, $5, &$6, $7, FALSE);
		};
LightType	: Lightdef tPOINT Vector
		{
			$$ = LightPointCreate(&$1, &$3);
		}
		| Lightdef tDIRECTIONAL Vector
		{
			$$ = LightInfiniteCreate(&$1, &$3);
		}
		| Lightdef tEXTENDED Expr Vector
		{
			$$ = LightExtendedCreate(&$1, $3, &$4);
		}
		| Lightdef tSPOT Vector Vector Expr
		{
			$$ = LightSpotCreate(&$1, &$3, &$4, $5, 0., 0.);
		}
		| Lightdef tSPOT Vector Vector Expr Expr Expr
		{
			/* light <intens> spot from <to> coef inner_rad
					outer_rad */
			$$ = LightSpotCreate(&$1, &$3, &$4, $5, $6, $7);
		};
Lightdef	: tLIGHT Intensity
		{
			$$ = $2;
		}
		;
CurSurf		: tAPPLYSURF Surface
		{
			CurSurf->surf = $2;
		}
		;
OptSurface	: Surface
		| /* EMPTY */
		{
			$$ = CurSurf->surf;
		}
		;
Surface		: NamedSurf
		| ModifyNamedSurf
		| SurfSpec
		;
NamedSurf	: tSTRING
		{
			$$ = SurfaceGetNamed($1);
			/*
			 * Free up memory allocated for surf name.
			 * We bother doing this because for large models
			 * converted from 3.0, surfnames this can account
			 * for lots o' bytes.
			 */
			free((voidstar)$1);
		}
		| tCURSURF
		{
			extern Surface DefaultSurface;

			if (CurSurf->surf)
				$$ = CurSurf->surf;
			else
				$$ = &DefaultSurface;
		}
		;
ModifyNamedSurf : CopyNamedSurf SurfComponent SurfComponents
		{
			$$ = tmpsurf;
			tmpsurf = (Surface *)NULL;
		}
		| CopyCurSurf SurfComponent SurfComponents
		{
			$$ = tmpsurf;
			tmpsurf = (Surface *)NULL;
		}
		;
CopyNamedSurf	: tSTRING
		{
			tmpsurf = SurfaceCopy(SurfaceGetNamed($1));
		}
		;
CopyCurSurf	: tCURSURF
		{
			extern Surface DefaultSurface;
			if (CurSurf->surf)
				tmpsurf = SurfaceCopy(CurSurf->surf);
			else
				tmpsurf = SurfaceCopy(&DefaultSurface);
		}
		;
SurfSpec	: SurfComponent SurfComponents
		{
			$$ = tmpsurf;
			tmpsurf = (Surface *)NULL;
		} 
		;
SurfDef		: tSURFACE tSTRING Surface
		{
			tmpsurf = SurfaceCopy($3);
			tmpsurf->name = strsave($2);
			SurfaceAddToDefined(tmpsurf);
			tmpsurf = (Surface *)NULL;
		}
		| tSURFACE tSTRING
		{
			/* black surface */
			tmpsurf = SurfaceCreate();
			tmpsurf->name = strsave($2);
			SurfaceAddToDefined(tmpsurf);
			tmpsurf = (Surface *)NULL;
		}
		;
SurfComponents	: SurfComponents SurfComponent
		| /* EMPTY */
		;
SurfComponent	: Ambient
		| Diffuse
		| Specular
		| Specpow
		| Body
		| Reflect
		| Transp
		| Extinct
		| Index
		| Translu
		| Noshadow
		;
Ambient		: tAMBIENT Color
		{
			if (tmpsurf == (Surface *)NULL)
				tmpsurf = SurfaceCreate();
			tmpsurf->amb = $2;
		}
		;
Diffuse		: tDIFFUSE Color
		{
			if (tmpsurf == (Surface *)NULL)
				tmpsurf = SurfaceCreate();
			tmpsurf->diff = $2;
		}
		;
Specular	: tSPECULAR Color
		{
			if (tmpsurf == (Surface *)NULL)
				tmpsurf = SurfaceCreate();
			tmpsurf->spec = $2;
		}
		;
Body		: tBODY Color
		{
			if (tmpsurf == (Surface *)NULL)
				tmpsurf = SurfaceCreate();
			tmpsurf->body = $2;
		};
Extinct		: tEXTINCT Expr
		{
			if (tmpsurf == (Surface *)NULL)
				tmpsurf = SurfaceCreate();
			tmpsurf->statten = $2;
		};
Specpow		: tSPECPOW Expr
		{
			if (tmpsurf == (Surface *)NULL)
				tmpsurf = SurfaceCreate();
			tmpsurf->srexp = $2;
		}
		;
Reflect		: tREFLECT Expr
		{
			if (tmpsurf == (Surface *)NULL)
				tmpsurf = SurfaceCreate();
			tmpsurf->reflect = $2;
		}
		;
Transp		: tTRANSP Expr
		{
			if (tmpsurf == (Surface *)NULL)
				tmpsurf = SurfaceCreate();
			tmpsurf->transp = $2;
		}
		;
Index		: tINDEX Expr
		{
			if (tmpsurf == (Surface *)NULL)
				tmpsurf = SurfaceCreate();
			tmpsurf->index = $2;
		}
		;
Translu		: tTRANSLU Expr Color Expr
		{
			if (tmpsurf == (Surface *)NULL)
				tmpsurf = SurfaceCreate();
			tmpsurf->translucency = $2;
			tmpsurf->translu = $3;
			tmpsurf->stexp = $4;
		}
		;
Noshadow	: tNOSHADOW
		{
			if (tmpsurf == (Surface *)NULL)
				tmpsurf = SurfaceCreate();
			tmpsurf->noshadow = TRUE;
		}
		;
HeightField	: tHEIGHTFIELD Surface Filename
		{
			$$ = GeomHfCreate($3);
			if ($$)
				$$->surf = $2;
		}
		| tHEIGHTFIELD Filename
		{
			$$ = GeomHfCreate($2);
		}
		;
Poly		: tPOLY OptSurface Polypoints
		{
			$$ = GeomPolygonCreate(Polypoints, Npoints,
				Options.flipnorm);
			if ($$)
				$$->surf = $2;
			Polypoints = (PointList *)NULL;
			Npoints = 0;
		}
		;
Polypoints	: /* empty */
		| Polypoints Polypoint
		;
Polypoint	: Vector
		{
			PointList *ptmp;

			ptmp = (PointList *)Malloc(sizeof(PointList));
			ptmp->vec = $1;
			ptmp->next = Polypoints;
			Polypoints = ptmp;
			Npoints++;
		}
		;
Aggregate	: AggregateDef
		{
			if (Defstack->obj) {
				/*
				 * Set object texture to current texture.
				 */
				Defstack->obj->texture = CurText;
			}
			CurText = (Texture *)NULL;
			/*
			 * Pop topmost object on stack.
			 */
			$$ = Defstack->obj;
			Defstack = GeomStackPop(Defstack);
			/* Pop current surface */
			CurSurf = SurfPop(CurSurf);
			/* Make current default surf aggregate's default */
			$$->surf = CurSurf->surf;
		}
		;
AggregateDef	: AggregateCreate Aggdefs tEND
		{
			/* Convert aggregate, pop stacks, etc. */
			if ($1) {
				if (Defstack->obj->next == (Geom *)NULL) {
					RLerror(RL_WARN,
						"Null object defined.\n");
					Defstack->obj = (Geom *)NULL;
				} else {
					/*
					 * Convert the linked list of objects
					 * associated with the topmost object
					 * to the appropriate aggregate type.
					 */
					Defstack->obj->prims=AggregateConvert(
						Defstack->obj,
						Defstack->obj->next);
					/*
					 * Make sure conversion worked OK.
					 */
					if (Defstack->obj->prims <= 0)
						Defstack->obj = (Geom *)NULL;
				}
			}
		}
		;
AggregateCreate	: AggregateType
		{
			if ($1) {
				Defstack = GeomStackPush($1, Defstack);
				CurSurf = SurfPush((Surface *)NULL, CurSurf);
			}
		};
AggregateType	: List
		| Grid
		| Csg
		;
List		: tLIST
		{
			$$ = GeomListCreate();
		}
		;
Grid		: tGRID IExpr IExpr IExpr
		{
			$$ = GeomGridCreate($2, $3, $4);
		}
		;
Csg		: CombineOp
		{
			$$ = GeomCsgCreate($1);
			Options.csg = TRUE;
		}
		;
CombineOp	: tUNION
		{
		    $$ = CSG_UNION;
		}
		| tINTERSECT
		{
		    $$ = CSG_INTERSECT;
		}
		| tDIFFERENCE
		{
		    $$ = CSG_DIFFERENCE;
		}
    		;
Cone		: tCONE OptSurface Expr Vector Expr Vector
		{
			if (equal($3, $5)) {
				/* It's really a cylinder */
				$$ = GeomCylinderCreate($3, &$4, &$6);
			} else
				$$ = GeomConeCreate($3, &$4, $5, &$6);
			if ($$)
				$$->surf = $2;
		}
		;
Cylinder	: tCYL OptSurface Expr Vector Vector
		{
			$$ = GeomCylinderCreate($3, &$4, &$5);
			if ($$)
				$$->surf = $2;
		}
		;
Sphere		: tSPHERE OptSurface Expr Vector
		{
			$$ = GeomSphereCreate($3, &($4));
			if ($$)
				$$->surf = $2;
		}
		;
Disc		: tDISC OptSurface Expr Vector Vector
		{
			$$ = GeomDiscCreate($3, 0.0, &($4), &($5));
			if ($$)
				$$->surf = $2;
		}
		;
Opendisc	: tOPENDISC OptSurface Expr Expr Vector Vector
		{
			$$ = GeomDiscCreate($3, $4, &($5), &($6));
			if ($$)
				$$->surf = $2;
		}
		;
Box		: tBOX OptSurface Vector Vector
		{
			$$ = GeomBoxCreate(&$3, &$4);
			if ($$)
				$$->surf = $2;
		}
		;
Triangle	: tTRIANGLE OptSurface Vector Vector Vector
		{
			$$ = GeomTriangleCreate(FLATTRI, &($3), &($4), &($5),
				(Vector *)NULL, (Vector *)NULL, (Vector *)NULL,
				(Vec2d *)NULL, (Vec2d *)NULL, (Vec2d *)NULL,
				Options.flipnorm);
			if ($$)
				$$->surf = $2;
		}
		| tTRIANGLE OptSurface  Vector Vector
					Vector Vector
					Vector Vector
		{
			$$ = GeomTriangleCreate(PHONGTRI, &($3), &($5),
				&($7), &($4), &($6), &($8),
				(Vec2d *)NULL, (Vec2d *)NULL, (Vec2d *)NULL,
				Options.flipnorm);
			if ($$)
				$$->surf = $2;
		}
		| tTRIANGLEUV OptSurface Vector Vector Vec2d
					 Vector Vector Vec2d
					 Vector Vector Vec2d
		{
			$$ = GeomTriangleCreate(PHONGTRI, &($3), &($6), &($9),
						&($4), &($7), &($10),
						&($5), &($8), &($11),
						Options.flipnorm);
			if ($$)
				$$->surf = $2;
		}
		;
Plane		: tPLANE OptSurface Vector Vector
		{
			$$ = GeomPlaneCreate(&($3), &($4));
			if ($$)
				$$->surf = $2;
		}
		;
Torus		: tTORUS OptSurface Expr Expr Vector Vector
		{
			$$ = GeomTorusCreate($3, $4, &($5), &($6));
			if ($$)
				$$->surf = $2;
		}
		;
Blob		: tBLOB OptSurface Expr MetaPoints
		{
			$$ = GeomBlobCreate($3, Metapoints, Npoints);
			if ($$)
				$$->surf = $2;
			Metapoints = (MetaList *)NULL;
			Npoints = 0;
		}
		;
MetaPoints	: /* empty */
		| MetaPoints MetaPoint
		;
MetaPoint	: Expr Expr Expr Expr Expr
		{
			Metapoint = (MetaList *)Malloc(sizeof(MetaList));
			Metapoint->mvec.c0 = $1;
			Metapoint->mvec.rs = $2;
			Metapoint->mvec.x = $3;
			Metapoint->mvec.y = $4;
			Metapoint->mvec.z = $5;
			Metapoint->next = Metapoints;
			Metapoints = Metapoint;
			Npoints++;
		}
		;
Outfile		: tOUTFILE Filename
		{
			if (Options.imgname != (char *)NULL)
				/* Already set on command line. */
				RLerror(RL_WARN,
					"Ignoring output file name \"%s\".\n",
					$2);
			else
				Options.imgname = strsave($2);
		}
		;
GlobalEffects	: tATMOSPHERE Effects
		{
			AtmosEffects = CurEffect;
			CurEffect = (Atmosphere *)NULL;
		}
		| tATMOSPHERE IExpr Effects
		{
			if ($2 <= 0.)
				RLerror(RL_PANIC,
				"Index of refraction must be positive.\n");
			TopMedium.index = $2;
			AtmosEffects = CurEffect;
			CurEffect = (Atmosphere *)NULL;
		}
		;
Effects		: Effects Effect
		|
		;
Effect		: EffectType
		{
			$1->next = CurEffect;
			CurEffect = $1;
		}
		;
EffectType	: tMIST Color Color Expr Expr
		{
			$$ = AtmosMistCreate(&($2), &($3), $4, $5);
		}
		| tFOG Color Color
		{
			$$ = AtmosFogCreate(&($2), &($3));
		}
		| tFOGDECK Expr Expr Vector Expr IExpr Color Color
		{
			$$ = AtmosFogdeckCreate($2, $3, &$4, $5, $6, &$7, &$8);
		}
		;
Color		: Expr Expr Expr
		{
			$$.r = $1;
			$$.g = $2;
			$$.b = $3;
		}
		;
Vector		: Expr Expr Expr
		{
			$$.x = $1;
			$$.y = $2;
			$$.z = $3;
		}
		;
Vec2d		: Expr Expr 
		{
			$$.u = $1;
			$$.v = $2;
		}
		;
OptMapping	: Mapping
		| /* EMPTY */
		{
			$$ = UVMappingCreate();
		}
		;
Mapping		: tMAP MapMethod
		{
			$$ = $2;
		}
		;
MapMethod	: tUV
		{
			$$ = UVMappingCreate();
		}
		| tSPHERICAL
		{
			$$ = SphereMappingCreate((Vector *)NULL,
				(Vector *)NULL, (Vector *)NULL);
		}
		| tSPHERICAL Vector Vector Vector
		{
			/* origin up uaxis */
			$$ = SphereMappingCreate(&$2, &$3, &$4);
		}
		| tCYLINDRICAL
		{
			$$ = CylMappingCreate((Vector *)NULL,
				(Vector *)NULL, (Vector *)NULL);
		}
		| tCYLINDRICAL Vector Vector Vector
		{
			/* origin up uaxis */
			$$ = CylMappingCreate(&$2, &$3, &$4);
		}
		| tPLANAR
		{
			$$ = LinearMappingCreate((Vector *)NULL,
				(Vector *)NULL, (Vector *)NULL);
		}
		| tPLANAR Vector Vector Vector
		{
			/* origin up uaxis */
			$$ = LinearMappingCreate(&$2, &$3, &$4);
		}
		;
SurfCompName	: tAMBIENT
		{
			$$ = AMBIENT;
		}
		| tDIFFUSE
		{
			$$ = DIFFUSE;
		}
		| tBODY
		{
			$$ = BODY;
		}
		| tSPECULAR
		{
			$$ = SPECULAR;
		}
		| tREFLECT
		{
			$$ = REFLECT;
		}
		| tTRANSP
		{
			$$ = TRANSP;
		}
		| tSPECPOW
		{
			$$ = SPECPOW;
		}
		| tBUMP
		{
			$$ = BUMP;
		}
		| tINDEX
		{
			$$ = INDEX;
		}
		;
Intensity	: Expr
		{ $$.r = $$.g = $$.b = $1; }
		| Color
		;
Print		: tPRINT Expr
		{
			fprintf(stderr,"%f\n",$2);
		}
Define		: tDEFINE tSTRING AnimExpr
		{
			SymtabAddEntry($2, $3->type, $3, NULL, $3->timevary, 0);
		};
IExpr		: Expr
		{ $$ = (int)$1; }
		;
Expr		: Float
		| ParenExpr
		{
			if (!$1->timevary) {
				$$ = ExprEval($1);
			} else {
				RLerror(RL_PANIC, "Illegal expression use.\n");
			}
		}
		;
AnimExpr	: Float
		{
			$$ = ExprReuseFloatCreate($1);
		}
		| ParenExpr
		;
ParenExpr	: '(' MExpr ')'
		{
			$$ = $2;
		};
MExpr		: tFLOAT
		{
			$$ = ExprFloatCreate($1, FALSE);
		}
		| tSTRING
		{
			$$ = ExprFloatSymtabFind($1);
		}
		| Symtabent '(' MExpr ')'
		{
			$$ = ExprResolve1($3, $1->value.fp, $1->timevary);
		}
		| Symtabent '(' MExpr ',' MExpr ')'
		{
			$$ = ExprResolve2($3, $5,
					$1->value.fp,
					$1->timevary);
		}
		| Symtabent '(' MExpr ',' MExpr ',' MExpr ')'
		{
			$$ = ExprResolve3($3, $5, $7, 
					$1->value.fp,
					$1->timevary);
		}
		| Symtabent '(' MExpr ',' MExpr ',' MExpr ',' MExpr ')'
		{
			$$ = ExprResolve4($3, $5, $7, $9, 
					$1->value.fp,
					$1->timevary);
		}
		| Symtabent
			'(' MExpr ',' MExpr ',' MExpr ',' MExpr ',' MExpr ')'
		{
			$$ = ExprResolve5($3, $5, $7, $9, $11,
					$1->value.fp,
					$1->timevary);
		}
		| '(' MExpr ')'
		{
			$$ = $2;
		}
		| MExpr '+' MExpr
		{
			$$ = ExprResolve2($1, $3, SumExpr, FALSE);
		}
		| MExpr '-' MExpr
		{
			$$ = ExprResolve2($1, $3, DiffExpr, FALSE);
		}
		| MExpr '*' MExpr
		{
			$$ = ExprResolve2($1, $3, MultExpr, FALSE);
		}
		| MExpr '/' MExpr
		{
			$$ = ExprResolve2($1, $3, DivideExpr, FALSE);
		}
		| MExpr '%' MExpr
		{
			$$ = ExprResolve2($1, $3, ModExpr, FALSE);
		}
		| '-' MExpr %prec UMINUS
		{
			$$ = ExprResolve1($2, NegateExpr, FALSE);
		}
		| '+' MExpr %prec UMINUS
		{
			$$ = $2;
		}
		| MExpr '^' MExpr
		{
			$$ = ExprResolve2($1, $3, PowExpr, FALSE);
		} ;
Float		: tFLOAT
		| '-' tFLOAT
		{ $$ = -$2; }
		| '+' tFLOAT
		{ $$ = $2; };
Filename	: tSTRING
		| tFILENAME
		;
Symtabent	: tSTRING
		{
			$$ = SymtabBuiltinFind($1);
		};
%%
/*
 * Issue error message containing filename and line number, and exit.
 */
/*VARARGS1*/
		  //void yyerror(s, pat1, pat2)
		  //char *s, *pat1, *pat2;
void yyerror(const char* s)
{
	fprintf(stderr,"%s: Error: %s: line %d: ", Options.progname,
			yyfilename, yylineno);
	//	fprintf(stderr, s, pat1, pat2);
	fprintf(stderr,"%s",s);
	if (*s && s[strlen(s) -1] != '\n')
		/* YACC doesn't put newlines on error messages. */
		fprintf(stderr,"\n");	
	fflush(stderr);
	exit(1);
}

Geom * NewAggregate(Geom *obj)
{
	obj->name = Defstack->obj->name;
	obj->next = Defstack->obj->next;
	return obj;
}
