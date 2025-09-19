%{
#include "vvp.tab.h"

#include <string.h>
#ifdef WIN32
#  include <malloc.h>
#else
#  include <unistd.h>
#endif

  void w(const char*);
  void wc(const char);
  void set_lineno(size_t lineno);
  void print_line(size_t lineno);
  
  #include <stdlib.h>
  #include <ctype.h>

  struct stack_node {
    struct stack_node* next;
    struct stack_node* prev;
    int val;
  };

  struct stack_node* stack = 0;

  int stack_get(void) {
    return (stack) ? stack->val : 0;
  }

  void stack_push(int i) {
    struct stack_node* node = (struct stack_node*)malloc(sizeof(struct stack_node));
    node->val = i;
    node->next = 0;
    if (stack)
      node->prev = stack;
    else
      node->prev = 0;
    stack = node;
  }

  void stack_pop(int enested) {
    if (stack) {
      struct stack_node* node = stack->prev;
      free(stack);
      stack = node;
      if (stack) stack->next = 0;
    }
  }

  struct list_node {
    struct list_node* next;
    char*  val;
  };

  struct list {
    struct list_node* first;
    struct list_node* last;
  };

  struct list v_types      = {0, 0};
  struct list v_properties = {0, 0};
  struct list m_properties = {0, 0};
  struct list e_properties = {0, 0};

  void list_append(struct list* l, const char* str) {
    struct list_node* node = (struct list_node*)malloc(sizeof(struct list_node));
    node->next = 0;
    node->val = (char *)malloc(strlen(str) + 1);
    strcpy(node->val, str);

    if (!l->first) {
      l->first = node;
      l->last = node;
    }
    else {
      l->last->next = node;
      l->last = node;
    }
  }

  void destroy(struct list* l) {
    struct list_node* node;

    while (l->first) {
      node = l->first;
      l->first = l->first->next;
      free(node->val);
      free(node);
    };
  }

  unsigned int first = 1;

  unsigned int vnested = 0;
  unsigned int mnested = 0;
  unsigned int enested = 0;
  unsigned int nnested = 0;

  unsigned int edge_defined = 0;
  unsigned int vertex_defined = 0;
  unsigned int mesh_defined = 0;
  unsigned int wrappers_included = 0;

  unsigned int in_for = 0;
  unsigned int in_exec = 0;
  unsigned int in_func = 0;
  unsigned int in_remove = 0;
  unsigned int make_nb_expr = 0;
  unsigned int in_path = 0;

  void write_default_edge() {
    w("struct edge {};");
    edge_defined = 1;
  }

  void write_default_vertex() {
    w("struct Position {}; typedef algebra::AbstractVertex<Position, edge> vertex;");
    vertex_defined = 1;
  }

  void write_default_mesh() {
    w("typedef algebra::XMLMesh<vertex> mesh;");
    mesh_defined = 1;
  }

  void include_wrappers() {
    print_line(yylineno);
    w("#include <generation/vvwrappers.hpp>\n");
    wrappers_included = 1;
  }
%}

delim [ \t\n]
ws    {delim}+

%s NRML VP VRTX EP EDGE MP MESH VEXPR FOR FOR2 FOR3 MEXPR PATH I
%option noyywrap
%option yylineno

%%

%{
  if (first) {
    BEGIN NRML;
    first = 0;
    w("\n#include <algebra/abstractvertex.hpp>\n#include <algebra/xmlmesh.hpp>");
    // Added extra preprocessor defines and includes so that old vv models will run without modification
    w("\n#define FALSE 0");
    w("\n#define TRUE 1");
    w("\n#ifdef __APPLE__");
    w("\n#include <OpenGL/gl.h>");
    //w("\n#include <OpenGL/glu.h>");
    w("\n#else");
    w("\n#include <GL/gl.h>");
    //w("\n#include <GL/glu.h>");
    w("\n#endif");
    print_line(yylineno);
  }
%}

"<"{ws}*"vertex"{ws}*">" {
  w("<vertex>");
}

"<"{ws}*"vertex"{ws}*"," {
  w("<vertex,");
}

","{ws}*"vertex"{ws}*">" {
  w(", vertex>");
}

","{ws}*"vertex"{ws}*/(([,_a-zA-Z0-9]{ws}*)+">") {
  w(", vertex");
}

"/*"([^*]|"*"[^/])*"*/" {
  w(yytext);
}

"//".*"\n" {
  w(yytext);
}

"\""("\\\""|[^\"])*"\"" {
  w(yytext);
}

<NRML>{ws} {
  w(yytext);
}
<NRML>"read_parms"{ws}*"("[^)]*")"{ws}*"{" {
  char* start = strchr(yytext, '(')+1;
  char* end = strchr(yytext, ')');
  int len = end-start;
  char buffer[1001] = "__default_vv_empty_read_parms_parameter_name__";
  if(len > 1000)
    fprintf(stderr, "Error, parameter name for read_parms is too long\nNo parameter name kept.\n");
  else
    {
    memcpy(buffer, start, len);
    buffer[len] = 0;
    }
  if (!edge_defined) write_default_edge();
  if (!vertex_defined) write_default_vertex();
  if (!mesh_defined) write_default_mesh();
  if (mesh_defined && vertex_defined && !wrappers_included) include_wrappers();
#ifdef WIN32
  w("extern \"C\" {void __declspec(dllexport) vvp_read_parms( const std::set<std::string>&");
  w(buffer);
  w(") {");
#else
  w("extern \"C\" {void vvp_read_parms( const std::set<std::string>& ");
  w(buffer);
  w(" ) {");
#endif
  print_line(yylineno);
  ++nnested;
  in_exec = 1;
}
<NRML>"start"{ws}*"{" {
  if (!edge_defined) write_default_edge();
  if (!vertex_defined) write_default_vertex();
  if (!mesh_defined) write_default_mesh();
  if (mesh_defined && vertex_defined && !wrappers_included) include_wrappers();
#ifdef WIN32
  w("extern \"C\" {void __declspec(dllexport) vvp_start() {");
#else
  w("extern \"C\" {void vvp_start() {");
#endif
  print_line(yylineno);
  ++nnested;
  in_exec = 1;
}
<NRML>"step"{ws}*"{" {
  if (!edge_defined) write_default_edge();
  if (!vertex_defined) write_default_vertex();
  if (!mesh_defined) write_default_mesh();
  if (mesh_defined && vertex_defined && !wrappers_included) include_wrappers();
#ifdef WIN32
  w("extern \"C\" {void __declspec(dllexport) vvp_step() {");
#else
  w("extern \"C\" {void vvp_step() {");
#endif
  print_line(yylineno);
  ++nnested;
  in_exec = 1;
}
<NRML>"end"{ws}*"{" {
  if (!edge_defined) write_default_edge();
  if (!vertex_defined) write_default_vertex();
  if (!mesh_defined) write_default_mesh();
  if (mesh_defined && vertex_defined && !wrappers_included) include_wrappers();
#ifdef WIN32
  w("extern \"C\" {void __declspec(dllexport) vvp_end() {");
#else
  w("extern \"C\" {void vvp_end() {");
#endif
  print_line(yylineno);
  ++nnested;
  in_exec = 1;
}
<NRML>"close"{ws}*"{" {
  if (!edge_defined) write_default_edge();
  if (!vertex_defined) write_default_vertex();
  if (!mesh_defined) write_default_mesh();
  if (mesh_defined && vertex_defined && !wrappers_included) include_wrappers();
#ifdef WIN32
  w("extern \"C\" {void __declspec(dllexport) vvp_close() {");
#else
  w("extern \"C\" {void vvp_close() {");
#endif
  print_line(yylineno);
  ++nnested;
  in_exec = 1;
}
<NRML>"render"{ws}*"{" {
  if (!edge_defined) write_default_edge();
  if (!vertex_defined) write_default_vertex();
  if (!mesh_defined) write_default_mesh();
  if (mesh_defined && vertex_defined && !wrappers_included) include_wrappers();
#ifdef WIN32
  w("extern \"C\" {void __declspec(dllexport) vvp_render() {");
#else
  w("extern \"C\" {void vvp_render() {");
#endif
  print_line(yylineno);
  ++nnested;
  in_exec = 1;
}
<NRML>"render_screen"{ws}*"{" {
  if (!edge_defined) write_default_edge();
  if (!vertex_defined) write_default_vertex();
  if (!mesh_defined) write_default_mesh();
  if (mesh_defined && vertex_defined && !wrappers_included) include_wrappers();
#ifdef WIN32
  w("extern \"C\" {void __declspec(dllexport) vvp_render_screen() {");
#else
  w("extern \"C\" {void vvp_render_screen() {");
#endif
  print_line(yylineno);
  ++nnested;
  in_exec = 1;
}  
<NRML>"render_init"{ws}*"{" {
  if (!edge_defined) write_default_edge();
  if (!vertex_defined) write_default_vertex();
  if (!mesh_defined) write_default_mesh();
  if (mesh_defined && vertex_defined && !wrappers_included) include_wrappers();
#ifdef WIN32
  w("extern \"C\" {void __declspec(dllexport) vvp_render_init() {");
#else
  w("extern \"C\" {void vvp_render_init() {");
#endif
  print_line(yylineno);
  ++nnested;
  in_exec = 1;
}
<NRML>"{" {
  w(yytext);
  ++nnested;
  set_lineno(yylineno+1);
}
<NRML>"}" {
  w(yytext);
  if (!(--nnested) && in_exec) {
    w("}");
    in_exec = 0;
  }
}
<NRML>"vertex"{ws}*"{" {
  vertex_defined = 1;
  BEGIN VP;
  ++vnested;
  return tVPROP;
}
<NRML>"vertex"/({ws}|","|")") {
  BEGIN VRTX;
  return tVDECL;
}
<NRML>[_a-zA-Z0-9]+"vertex"[_a-zA-Z0-9]+ {
  w(yytext);
}
<NRML>[_a-zA-Z0-9]+"vertex" {
  w(yytext);
}
<NRML>"edge"{ws}*"{" {
  edge_defined = 1;
  BEGIN EP;
  ++enested;
  return tEPROP;
}
<NRML>"vertex"[_a-zA-Z0-9]+ {
  w(yytext);
}
<NRML>"mesh"{ws}*"{" {
  mesh_defined = 1;
  BEGIN MP;
  ++mnested;
  return tMPROP;
}
<NRML>[_a-zA-Z0-9]+"mesh"[a-zA-Z0-9_]+ {
  w(yytext);
}
<NRML>"mesh"[_a-zA-Z0-9]+ {
  w(yytext);
}
<NRML>[_a-zA-Z0-9]+"mesh" {
  w(yytext);
}
<NRML>"mesh" {
  return tMESH;
}
<NRML>"labelof"{ws} {
  BEGIN VEXPR;
  stack_push(enested);
  enested = 0;
  return tLABEL;
}
<NRML>"valence"{ws} {
  BEGIN VEXPR;
  stack_push(enested);
  enested = 0;
  return tVALENCE;
}
<NRML>[_A-Za-z0-9]"is"{ws} {
  w(yytext);
}
<NRML>"is"{ws} {
  BEGIN VEXPR;
  stack_push(enested);
  enested = 0;
  return tIS;
}
<NRML>"any"{ws}"in"{ws} {
  BEGIN VEXPR;
  stack_push(enested);
  enested = 0;
  return tANY;
}
<NRML>"flag"{ws} {
  BEGIN VEXPR;
  stack_push(enested);
  enested = 0;
  return tFLAG;
}
<NRML>"flagged"{ws}"in"{ws} {
  BEGIN VEXPR;
  stack_push(enested);
  enested = 0;
  return tFLAGGED;
}
<NRML>"nextto"{ws} {
  BEGIN VEXPR;
  stack_push(enested);
  enested = 0;
  return tNEXT;
}
<NRML>"prevto"{ws} {
  BEGIN VEXPR;
  stack_push(enested);
  enested = 0;
  return tPREV;
}
<NRML>"nextto"{ws}"flag"{ws}"in"{ws} {
  BEGIN VEXPR;
  stack_push(enested);
  enested = 0;
  return tNEXT_FLAGGED;
}
<NRML>"prevto"{ws}"flag"{ws}"in"{ws} {
  BEGIN VEXPR;
  stack_push(enested);
  enested = 0;
  return tPREV_FLAGGED;
}
<NRML>"next"{ws}*"("{ws}* {
  BEGIN I;
  stack_push(enested);
  enested = 0;
  return tNEXTI;
}
<NRML>"prev"{ws}*"("{ws}* {
  BEGIN I;
  stack_push(enested);
  enested = 0;
  return tPREVI;
}
<NRML>"@("{ws}* {
  BEGIN VEXPR;
  stack_push(enested);
  enested = 0;
  in_path = 1;
  return tPOPEN;
}
<NRML>"`@("{ws}* {
  BEGIN VEXPR;
  stack_push(enested);
  enested = 0;
  in_path = 1;
  return tPOPENOLD;
}
<NRML>"@&("{ws}* {
  BEGIN VEXPR;
  stack_push(enested);
  enested = 0;
  in_path = 1;
  return tPROPEN;
}
<NRML>"`@&("{ws}* {
  BEGIN VEXPR;
  stack_push(enested);
  enested = 0;
  in_path = 1;
  return tPROPENOLD;
}
<NRML>"`" {
  BEGIN VEXPR;
  stack_push(enested);
  enested = 0;
  return tTICK;
}
<NRML>"erase"{ws} {
  BEGIN VEXPR;
  stack_push(enested);
  enested = 0;
  return tERASE;
}
<NRML>"erase"{ws}"flag"{ws}"from"{ws} {
  BEGIN VEXPR;
  stack_push(enested);
  enested = 0;
  return tERASE_FLAGGED;
}
<NRML>"replace"{ws} {
  BEGIN VEXPR;
  stack_push(enested);
  enested = 0;
  return tREPLACE;
}
<NRML>"splice"{ws} {
  BEGIN VEXPR;
  stack_push(enested);
  enested = 0;
  return tSPLICE;
}
<NRML>"make"{ws}*"{"{ws}* {
  BEGIN VEXPR;
  stack_push(enested);
  enested = 0;
  make_nb_expr = 1;
  return tMAKE;
}
<NRML>"forall"{ws} {
  BEGIN FOR;
  set_lineno(yylineno+1);
  return tFOR;
}
<NRML>"synchronise"{ws} {
  BEGIN MEXPR;
  return tSYNCH;
}
<NRML>"synchronize"{ws} {
  BEGIN MEXPR;
  return tSYNCH;
}
<NRML>"add"{ws} {
  BEGIN VEXPR;
  stack_push(enested);
  enested = 0;
  return tADD;
}
<NRML>"remove"{ws} {
  BEGIN VEXPR;
  stack_push(enested);
  enested = 0;
  in_remove = 1;
  return tREMOVE;
}
<NRML>{ws}*"to"{ws} {
  BEGIN MEXPR;
  return tTO;
}
<NRML>"merge"{ws} {
  BEGIN MEXPR;
  return tMERGE;
}
<NRML>"clear"{ws} {
  BEGIN MEXPR;
  return tCLEAR;
}
<NRML>"$" {
  w("->getPosition().");
}
<NRML>. {
  w(yytext);
}

<VP>{ws} {
  w(yytext);
}
<VP>"{" {
  ++vnested;
  w(yytext);
  set_lineno(yylineno+1);
}
<VP>"}" {--vnested; w(yytext);}
<VP>"}"{ws}*";" {
  w(yytext);
  --vnested;
  if (!vnested) {
    if (edge_defined && vertex_defined && mesh_defined) include_wrappers();
    BEGIN NRML;
    set_lineno(yylineno);
    return tSEMICOLON;
  }
}
<VP>({ws}|[a-zA-Z0-9_:<>,])+{ws}*";" {
  char* type;
  char* property;
  int i0 = 0, i = 0, j = 0;
  int l;
  int done = 0;

  if (vnested == 1) {
    l = strlen(yytext);

    type = (char *)malloc(l + 1);

    while (isspace(yytext[i0])) {++i0;}

    for (i = i0; i < l; i++) {
      if (yytext[i] != '\n') type[j] = yytext[i];
      if (yytext[i] == ';') break;
      j++;
    }
    for (; j > 0; j--) {
      if (isspace(type[j])) {
	type[j] = '\0';
	break;
      }
    }
    i = j + 1;

    property = (char *)malloc(l + 1);

    j = 0;
    while (!done) {
      if (type[i] == ';' || isspace(type[i])) {
	property[j] = '\0';
	done = 1;
      }
      else property[j] = type[i];
      ++i; ++j;
    }

    list_append(&v_types, type);
    list_append(&v_properties, property);

    free(type);
    free(property);
  }

  w(yytext);
}
<VP>[a-zA-Z0-9_:\~\!\#\%\^\&\*\(\)\-\=\+\/\.<>\",?\|;\[\]'] {
  if (vnested)
    w(yytext);
}

<VRTX>{ws} {
  w(yytext);
}
<VRTX>[a-zA-Z0-9_] {
  w(yytext);
}
<VRTX>";" {
  BEGIN NRML;
  set_lineno(yylineno);
  return tSEMICOLON;
}
<VRTX>[=\(\),] {
  w(yytext);
  BEGIN NRML;
  return tVDECLEND;
}
<VRTX>"("({ws}|[a-zA-Z0-9_:\~\!\#\%\^\&\*\(\)\-\=\+\/\.<>\",?\|\[\]'\(\)$])+")"{ws}*";" {
  int i, l = strlen(yytext);
  if (in_exec || nnested) {  
    w(" = (new vertex");
    for (i = 0; i < l; i++) {
      if (yytext[i] == '$') w("->getPosition().");
      else if (yytext[i] != ';') wc(yytext[i]);
    }
    w(")->vptr();");
  }
  else {
    for (i = 0; i < l; i++) {
      if (yytext[i] == '$')
        w("->getPosition().");
      else if (yytext[i] == 'v' && i + 6 < l
            && yytext[++i] == 'e'
            && yytext[++i] == 'r'
            && yytext[++i] == 't'
            && yytext[++i] == 'e'
            && yytext[++i] == 'x'
            && (yytext[i + 1] == ' '
             || yytext[i + 1] == '\t'
             || yytext[i + 1] == '\n'
             || yytext[i + 1] == ','
             || yytext[i + 1] == ')'))
        w("vertex::VPtr");
      else wc(yytext[i]);
    }
  }
  BEGIN NRML;
  return tVDECLEND;
}

<MP>{ws} {
  w(yytext);
}
<MP>"{" {
  ++mnested;
  w(yytext);
  set_lineno(yylineno+1);
}
<MP>"}" {--mnested; w(yytext);}
<MP>";" {
  w(yytext);
  if (!mnested) {
    if (edge_defined && vertex_defined && mesh_defined) include_wrappers();
    BEGIN NRML;
    set_lineno(yylineno);
    return tSEMICOLON;
  }
}
<MP>[a-zA-Z0-9_]+{ws}*";" {
  if (mnested == 1) {
    char* property;
    int i;
    int l;

    l = strlen(yytext);
    property = (char *)malloc(l + 1);
    strcpy(property, yytext);
    for (i = 0; i < l; i++) {
      if (property[i] == ';' || isspace(property[i])) property[i] = '\0';
    }
    list_append(&m_properties, property);
    free(property);
  }
  w(yytext);
}
<MP>[a-zA-Z0-9_:\~\!\#\%\^\&\*\(\)\-\=\+\/\.<>\",?\|[\]] {
  if (mnested)
    w(yytext);
}

<EP>{ws} {
  w(yytext);
}
<EP>"{" {
  ++enested;
  w(yytext);
  set_lineno(yylineno+1);
}
<EP>"}" {--enested; w(yytext);}
<EP>";" {
  w(yytext);
  if (!enested) {
    if (edge_defined && vertex_defined && mesh_defined) include_wrappers();
    BEGIN NRML;
    set_lineno(yylineno);
    return tSEMICOLON;
  }
}
<EP>[a-zA-Z0-9_]+{ws}*";" {
  if (enested == 1) {
    char* property;
    int i;
    int l;

    l = strlen(yytext);
    property = (char *)malloc(l + 1);
    strcpy(property, yytext);
    for (i = 0; i < l; i++) {
      if (property[i] == ';' || isspace(property[i])) property[i] = '\0';
    }
    list_append(&e_properties, property);
    free(property);
  }
  w(yytext);
}
<EP>[a-zA-Z0-9_:\~\!\#\%\^\&\*\(\)\-\=\+\/\.<>\",?\|] {
  if (enested)
    w(yytext);
}

<VEXPR>"flag"{ws} {
  return tFLAG;
}
<VEXPR>"nextto"{ws} {
  stack_push(enested);
  enested = 0;
  return tNEXT;
}
<VEXPR>"prevto"{ws} {
  stack_push(enested);
  enested = 0;
  return tPREV;
}
<VEXPR>"nextto"{ws}"flag"{ws}"in"{ws} {
  BEGIN VEXPR;
  stack_push(enested);
  enested = 0;
  return tNEXT_FLAGGED;
}
<VEXPR>"prevto"{ws}"flag"{ws}"in"{ws} {
  BEGIN VEXPR;
  stack_push(enested);
  enested = 0;
  return tPREV_FLAGGED;
}
<VEXPR>"next"{ws}*"("{ws}* {
  BEGIN I;
  stack_push(enested);
  enested = 0;
  return tNEXTI;
}
<VEXPR>"prev"{ws}*"("{ws}* {
  BEGIN I;
  stack_push(enested);
  enested = 0;
  return tPREVI;
}
<VEXPR>"@("{ws}* {
  stack_push(enested);
  enested = 0;
  in_path = 1;
  return tPOPEN;
}
<VEXPR>"`@("{ws}* {
  BEGIN VEXPR;
  stack_push(enested);
  enested = 0;
  in_path = 1;
  return tPOPENOLD;
}
<VEXPR>"@&("{ws}* {
  stack_push(enested);
  enested = 0;
  in_path = 1;
  return tPROPEN;
}
<VEXPR>"`@&("{ws}* {
  BEGIN VEXPR;
  stack_push(enested);
  enested = 0;
  in_path = 1;
  return tPROPENOLD;
}
<VEXPR>"any"{ws}"in"{ws} {
  stack_push(enested);
  enested = 0;
  return tANY;
}
<VEXPR>"flagged"{ws}"in"{ws} {
  stack_push(enested);
  enested = 0;
  return tFLAGGED;
}
<VEXPR>"`" {
  stack_push(enested);
  enested = 0;
  return tTICK;
}
<VEXPR>{ws}*"in"{ws} {
  return tIN;
}
<VEXPR>{ws}"from"{ws} {
  if (in_remove) {
    BEGIN MEXPR;
    in_remove = 0;
  }
  return tFROM;
}
<VEXPR>{ws}*"with"{ws} {
  return tWITH;
}
<VEXPR>{ws}"before"{ws} {
  return tBEFORE;
}
<VEXPR>{ws}"after"{ws} {
  return tAFTER;
}
<VEXPR>{ws}*","{ws}* {
  return tCOMMA;
}
<VEXPR>{ws}*"}"{ws}*"nb_of"{ws} {
  return tNBOF;
}
<VEXPR>"(" {
  w(yytext);
  ++enested;
}
<VEXPR>")" {
  --enested;
  if (in_path) {
    BEGIN PATH;
    return tCLOSE;
  }
  w(yytext);
  if (!enested)
    return tVERTEX;
}
<VEXPR>[a-zA-Z0-9_:\~\!\#\%\^\&\*\-\=\+\/\.<>\"?\|]+ {
  w(yytext);
  if (!enested)
    return tVERTEX;
}
<VEXPR>";" {
  BEGIN NRML;
  enested = stack_get();
  stack_pop(enested);
  if (make_nb_expr)
    make_nb_expr = 0;
  else w(yytext);
}
<VEXPR>"$" {
  BEGIN NRML;
  enested = stack_get();
  stack_pop(enested);
  w("->getPosition().");
}
<VEXPR>{ws} {
  if (in_for) {
    BEGIN FOR3;
    in_for = 0;
  }
  else
    BEGIN NRML;
  enested = stack_get();
  stack_pop(enested);
  w(yytext);
}
<VEXPR>"{" {
  ++nnested;
  w("{");
  set_lineno(yylineno+1);
  BEGIN NRML;
}

<FOR>[a-zA-Z0-9_]+ {
  strncpy(yylval.ident, yytext, 255); 
  yylval.ident[255] = 0;
  set_lineno(yylineno+1);
  return tIDENT;
}
<FOR>{ws}"in"{ws} {
  BEGIN FOR2;
  set_lineno(yylineno+1);
  return tIN;
}
<FOR2>[a-zA-Z0-9_]+ {
  BEGIN FOR3;
  strncpy(yylval.ident, yytext, 255); 
  yylval.ident[255] = 0;
  set_lineno(yylineno+1);
  return tIDENT;
}
<FOR2>"any"{ws}"in"{ws} {
  in_for = 1;
  BEGIN VEXPR;
  stack_push(enested);
  enested = 0;
  set_lineno(yylineno+1);
  return tANY;
}
<FOR2>"nextto"{ws} {
  in_for = 1;
  BEGIN VEXPR;
  stack_push(enested);
  enested = 0;
  set_lineno(yylineno+1);
  return tNEXT;
}
<FOR2>"prevto"{ws} {
  in_for = 1;
  BEGIN VEXPR;
  stack_push(enested);
  enested = 0;
  set_lineno(yylineno+1);
  return tPREV;
}
<FOR2>"next"{ws}*"("{ws}* {
  BEGIN I;
  stack_push(enested);
  enested = 0;
  set_lineno(yylineno+1);
  return tNEXTI;
}
<FOR2>"prev"{ws}*"("{ws}* {
  BEGIN I;
  stack_push(enested);
  enested = 0;
  set_lineno(yylineno+1);
  return tPREVI;
}
<FOR2>"`" {
  in_for = 1;
  BEGIN VEXPR;
  stack_push(enested);
  enested = 0;
  set_lineno(yylineno+1);
  return tTICK;
}

<FOR3>{ws} { }

<FOR3>"{" {
  ++nnested;
  print_line(yylineno);
  BEGIN NRML;
}

<MEXPR>[a-zA-Z0-9_]+ {
  strncpy(yylval.ident, yytext, 255); 
  yylval.ident[255] = 0;
  return tIDENT;
}
<MEXPR>{ws}"with"{ws} {
  return tWITH;
}
<MEXPR>[{ws};] {
  w(yytext);
  BEGIN NRML;
}

<PATH>{ws}*"next"{ws}* {
  return tPNEXT;
}
<PATH>{ws}*"prev"{ws}* {
  return tPPREV;
}
<PATH>{ws}*"swap"{ws}* {
  return tPSWAP;
}
<PATH>"," {
  return tCOMMA;
}
<PATH>"@" {
  in_path = 0;
  BEGIN NRML;
  return tPCLOSE;
}

<I>[a-zA-Z0-9_:\~\!\%\^\&\*\-\=\+\/\.<>\",?\|]+ {
  w(yytext);
  return tI;  
}
<I>")"{ws}*"to"{ws} {
  BEGIN VEXPR;
}
