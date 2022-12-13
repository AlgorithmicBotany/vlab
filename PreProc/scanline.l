%option noyywrap

%{

#include <string.h>
#include <unistd.h>
#include "Files.h"
#include "Decl.h"
#include "ErrorHandler.h"
#include "HashFn.h"

char    	*errmsgs[ERRORS] =
		{
			(char*) "No files specified after #include directive",
			(char*) "Missing parameters for define variable:  ",
			(char*) "Identifier has no defined parameters:  ",
			(char*) "Identifier has too many parameters:  ",
			(char*) "Identifier has too few parameters:  ",
			(char*) "Newline found in macro definition",
			(char*) "Unable to open file ",
			(char*) "Multiply defined identifier ",
		};

typedef union {
	int		integer;
	char		*fname;
} YYSTYPE;
YYSTYPE yylval;

#define S_CHAR		sizeof(char)

#define OutputNewline	LineNo++; Output((char*)"");
#define Newline		LineNo++;
#define AddHash(id)	GlobalStab = (StabEl*)AddToHashTable(FS.Head->Stab,id)
#define DelHash(id)	printf("Deleting:  |%s|\n", id); \
			PrintHashTable(FS.Head->Stab); \
			DeleteFromHashTable(FS.Head->Stab, id); \
			PrintHashTable(FS.Head->Stab)
#undef	DelHash
#define DelHash(id)	DeleteFromHashTable(FS.Head->Stab, id)
#define LookHash(id)	(StabEl *)LookUpHashTable(FS.Head->Stab, id)

int		LineNo = 0;	/* Line number in currently scanned file. */
StabEl		*GlobalStab;	/* So we can load data into the stab. */
RepList		*ParamList;	/* So we can add valuesto #defines. */
int		ParamCount;	/* Count the number of parameters defined. */
FileStack	FS;		/*
				 * Stack for maintaining state of nested
				 * files.
				 */
%}

space		[\t ]
/* id		([:alpha:]|_)([:alphanum:]|_)* */
id		([A-Za-z]|[A-Za-z_][A-Za-z0-9_]*)
loc_filename	"\""[^\"\n]+"\""
var_filename	"<"[^>\n]+">"

%x COMMENT INCLMODE DEFNMODE PARAMODE PARAMLIST
%x VALUEMODE VALUELIST REPLKILL REPLMODE UNDEFMODE

%%

	/* "\""[^"\n]"\""	{ printf("Std_string:  %s\n", yytext); } */

				/* Handle comments while maintaining lines. */
{space}*"/*"			{ BEGIN COMMENT; }
<COMMENT>\n			{ OutputNewline; }
<COMMENT>[^*\n]*		;
<COMMENT>"*"+[^*/\n]*		;
<COMMENT>"*"+"/"		{ BEGIN INITIAL; }

^[ \t]*\n			{ OutputNewline; }
\n				{ OutputNewline; }

^#include{space}+		{ BEGIN INCLMODE; }
<INCLMODE>{loc_filename}{space}*\n	{
				  yylval.fname = (char *)GetUnFname(yytext);
				  Newline;
				  BEGIN INITIAL;
				  FileStackPush(&FS, yylval.fname);
				}
<INCLMODE>{var_filename}{space}*\n	{
				  yylval.fname = (char *)GetUnFname(yytext);
				  Newline;
				  BEGIN INITIAL;
				  FileStackPush(&FS, yylval.fname);
				}
<INCLMODE>.*			{ ErrorHandler(ERROR_INCLUDE_NOFILE, NULL); }
^#include{space}*\n		{ ErrorHandler(ERROR_INCLUDE_NOFILE, NULL); }

^#define			{ BEGIN DEFNMODE; }
<DEFNMODE>{space}*		;
<DEFNMODE>{id}"("		{
						/* Remove the '(' character. */
				  yytext[strlen(yytext)-1] = '\0';
				  AddHash(yytext);
				  BEGIN PARAMODE;
				}
<DEFNMODE>{id}			{
				  AddHash(yytext);
				  BEGIN REPLKILL;
				}
<PARAMODE>{id}			{
				  AddParameter(GlobalStab, yytext);
				  BEGIN PARAMLIST;
				}
<PARAMLIST>,{id}		{ AddParameter(GlobalStab, yytext+S_CHAR); }
<PARAMLIST>")"{space}*		{ 
				  /* PrintRepList(GlobalStab->Parameters); */
				  BEGIN REPLMODE;
				}
						/* Handle whitespace. */
<REPLKILL>{space}*		{ BEGIN REPLMODE; }

	/* Remember to handle quoted strings. */

<REPLMODE>{id}			{ AddReplacement(GlobalStab, yytext, Id); }
<REPLMODE>\\\n			{
				  AddReplacement(GlobalStab, yytext, Other);
				  OutputNewline;
				}
<REPLMODE>.			{ AddReplacement(GlobalStab, yytext, Other); }
<REPLMODE>\n			{ OutputNewline; BEGIN INITIAL;	}

^#undef				{ BEGIN UNDEFMODE; }
<UNDEFMODE>{space}*		;
<UNDEFMODE>{id}			{ DelHash(yytext); }
<UNDEFMODE>\n			{ OutputNewline; BEGIN INITIAL; }

^#line.*\n			{ Outputnnl(yytext); OutputNewline; }

^#if				
^#ifdef				
^#ifndef			
^#elif				
^#else				
^#endif				

{id}"("				{
				  StabEl *temp;

				  yytext[strlen(yytext)-1] = '\0';
				  if((temp = LookHash(yytext)) == NULL) {
				  	yytext[strlen(yytext)] = '(';
					ECHO;
				  } else {
					GlobalStab = temp;
					ParamCount = GlobalStab->Number_of_params;
					ParamList = GlobalStab->Parameters;

					BEGIN VALUEMODE;
				  }
				}
{id}				{
				  StabEl *temp;

				  if((temp = LookHash(yytext)) == NULL) {
					ECHO;
				  } else {
					if(temp->Number_of_params == 0)
						PrintRepList(temp->OrigRep);
					else
						ErrorHandler(ERROR_NO_PARAMS,
								yytext);
				  }
				}
<VALUEMODE>[^,)\n]*		{
				  if(ParamList == NULL)
					ErrorHandler(ERROR_IDENT_HASPARAMS, 0);

				  ParamList->value = (char *)strdup(yytext);
				  ParamList = ParamList->next;
				  ParamCount--;

				  BEGIN VALUELIST;
				}

<VALUELIST>,[^,)\n]*		{
				  if(ParamList == NULL)
					ErrorHandler(ERROR_IDENT_EXTRAPARAMS,
							GlobalStab->Id);

				  ParamList->value = (char *)strdup(yytext+S_CHAR);
				  ParamList = ParamList->next;
				  ParamCount--;
				}
<VALUELIST>")"			{
				  if(ParamCount < 0)
					ErrorHandler(ERROR_IDENT_EXTRAPARAMS,
							GlobalStab->Id);
				  else if(ParamCount > 0)
					ErrorHandler(ERROR_IDENT_TOOFEWPARAMS,
							GlobalStab->Id);

				  WriteDefine(GlobalStab);
				  GlobalStab = NULL;
				  BEGIN INITIAL;
				}
<VALUELIST>\n			{ ErrorHandler(ERROR_MACRO_NEWLINE, NULL); }

.				ECHO;

<<EOF>>				{
				  if(FileStackSize(&FS) == 1) {
					FileStackExit(&FS);
					yyterminate();
				  } else {
					FileStackPop(&FS);
				  }
				}
%%
void ErrorHandler(int ERRORTYPE, char *string)
{
	if(string == NULL)
		fprintf(stderr, "Error:%s:%d:%s.\n",
			FS.Head->inFile, LineNo, errmsgs[ERRORTYPE]);
	else
		fprintf(stderr, "Error:%s:%d:%s%s.\n",
			FS.Head->inFile, LineNo, errmsgs[ERRORTYPE], string);

	exit(-1);
}

void Output(char *string) { puts(string); return; }
void Outputnnl(char *string) { printf("%s", string); return; }
int main(int argc, char **argv)
{
					/* Error checking for command line. */
					/* Will be replaced with getopts. */
	if(argc != 2) {
		fprintf(stderr, "USAGE:  %s <filename>\n", *argv);
		exit(-1);
	}

	FileStackInit(&FS, argv[1]);	/* Initialize the file stack. */
	yylex();			/* Parse these files. */
	return(0);
}

char *GetUnFname(char *Old)
{
	char		*Tmp = (char *)rindex(Old, '\"');
	char		*New;

	*Tmp = '\0';
	if((New = (char *)strdup(&Old[1])) == NULL) {
		perror("Strdup(GetUnFname)");
		exit(-1);
	}

	return(New);
}

void OutputFilePosLine(Stack_El *SE, FILE *out_fp)
{
	fprintf(out_fp, "# %d \"%s\" \n", SE->LineNo, SE->inFile);
}

int FileStackSize(FileStack *FS)
{
	return(FS->Number_of_files);
}

void FileStackInit(FileStack *FS, char *Filename)
{
	LineNo = 1;			/* Init the global line no for lex. */
	FS->Number_of_files = 1;	/* Set this to be the first file. */
					/* Create a head element. */
	if((FS->Head = (Stack_El *)calloc(1, sizeof(Stack_El))) == NULL) {
		perror("Calloc(FileStackInit)");
		exit(-1);
	}

					/* Open the imput file. */
	if((FS->Head->in_fp = (FILE *)fopen(Filename, "r")) == NULL)
		ErrorHandler(ERROR_OPENINGFILE, Filename);

	FS->Head->LineNo = 1;		/* Current line number is 1. */
					/* Save the file name for #line. */
	(void)strncpy(FS->Head->inFile, Filename, MAXPATHLEN);
	FS->Head->Stab = (HashTable **)CreateHashTable();

	yyin = (FILE *)FS->Head->in_fp;	/* Set lexer input to current file. */

					/* Create new flex buffer. */
	FS->Head->yy_buf_state = yy_create_buffer(yyin, YY_BUF_SIZE);
					/* Switch to the new flex buffer. */
	yy_switch_to_buffer(FS->Head->yy_buf_state);

					/* Output #line for new file. */
	OutputFilePosLine(FS->Head, stdout);

	return;
}

void FileStackExit(FileStack *FS)
{
	fclose(FS->Head->in_fp);
	DeleteHashTable(FS->Head->Stab);
	free(FS->Head);

	return;
}

void FileStackPush(FileStack *FS, char *Filename)
{
	Stack_El		*temp = (Stack_El *)FS->Head;

	temp->in_fp = (FILE *)yyin;	/* Save the current state of file. */
	temp->LineNo = LineNo;		/* Save the current line number. */

	FS->Number_of_files++;		/* Increment the stack level. */
	if((FS->Head = (Stack_El *)calloc(1, sizeof(Stack_El))) == NULL) {
		perror("Calloc(FileStackInit)");
		exit(-1);
	}

	FS->Head->Next = temp;		/* Save the pointer to previous file */

	if((FS->Head->in_fp = (FILE *)fopen(Filename, "r")) == NULL)
		ErrorHandler(ERROR_OPENINGFILE, Filename);

	LineNo = 1;			/* Global line number is 1. */
	FS->Head->LineNo = 1;		/* Current line number is 1. */
					/* Save the file name for #line. */
	(void)strncpy(FS->Head->inFile, Filename, MAXPATHLEN);
	FS->Head->Stab = (HashTable **)CreateHashTable();

	yyin = (FILE *)FS->Head->in_fp;	/* Set lexer input to current file. */
					/* Output #line for new file. */
	OutputFilePosLine(FS->Head, stdout);

					/* Create new flex buffer. */
	FS->Head->yy_buf_state = yy_create_buffer(yyin, YY_BUF_SIZE);
					/* Switch to the new flex buffer. */
	yy_switch_to_buffer(FS->Head->yy_buf_state);

	return;
}
	
void FileStackPop(FileStack *FS)
{
	Stack_El		*temp = (Stack_El *)FS->Head;

					/* Delete the current flex buffer. */
	yy_delete_buffer(YY_CURRENT_BUFFER);
	FS->Head = FS->Head->Next;	/* Remove the top of stack. */
	fclose(temp->in_fp);		/* Close the input file. */
	free(temp);			/* Free the Stack_El. */

	yyin = (FILE *)FS->Head->in_fp;	/* Restore the previous lex input. */
	LineNo = FS->Head->LineNo;	/* Restore the new file's line no. */
					/* Switch to the new flex buffer. */
	yy_switch_to_buffer(FS->Head->yy_buf_state);

					/* Output #line for end of file. */
	OutputFilePosLine(FS->Head, stdout);

	FS->Number_of_files--;		/* Decrement the stack level. */
	return;
}

char *SearchParamList(RepList *Params, char *data)
{
	RepList		*temp = Params;
	StabEl		*stabtemp;


					/*
					 * Parameters have higher scope than
					 * previous #defines.
					 */
	while(temp != NULL) {
		if(!strcmp(data, temp->data))
			return(temp->value);
		temp = temp->next;
	}

	if((stabtemp = LookHash(data)) == NULL) {
		return(data);
	} else {
		if(stabtemp->Number_of_params == 0) {
			PrintRepList(stabtemp->OrigRep);
			return((char*)"");
		} else {
					/* Is this error fatal? */
			Output((char*)"Error:  Has params in XXX(x,y,z).");
			return NULL;
		}
	}
}

void WriteDefine(StabEl *stab)
{
	int		i;
	RepList		*rep = stab->OrigRep;

#ifdef DEBUG
	printf("WriteDefine:  %d\n", stab->Number_of_repsegs);
	PrintRepList(rep);
#endif

	for(i = stab->Number_of_repsegs; i > 0; i--) {
		if(rep->type == Other)
			printf("%s", rep->data);
		else
			printf("%s", 
			       SearchParamList(stab->Parameters, rep->data));

		rep = rep->next;
	}
}


