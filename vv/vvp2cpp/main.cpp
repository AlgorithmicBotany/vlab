#include <stdio.h>
#include <stdarg.h>

extern int yyparse();
extern FILE* yyin;
char *filename;
FILE *out;
size_t lineno;

struct list_node {
  struct list_node* next;
  char*  val;
};

struct list {
  struct list_node* first;
  struct list_node* last;
};

extern struct list v_types;
extern struct list v_properties;
extern struct list m_properties;
extern struct list e_properties;

void set_lineno(size_t lo)
{
  lineno = lo;
}

void print_line(size_t lo)
{
  fprintf(out, "\n#line %d \"%s\"\n", lo, filename );
}

void StartEProp() {
  fprintf(out, "struct edge {");
}

void EndEProp() {
  struct list_node* node = e_properties.first;
  fprintf(out, "bool operator==(const edge& a, const edge& b) {");
  while (node) {
    fprintf(out, "  if (a.%s != b.%s) return false;", node->val, node->val);
    node = node->next;
  }
  fprintf(out, "  return true;");
  fprintf(out, "}");
}

void WriteEDecl() {
  fprintf(out, "edge");
}

void StartVProp() {
  fprintf(out, "struct Position {");
}

void EndVProp() {
  struct list_node* t_node;
  struct list_node* p_node;

  fprintf(out, "class vertex : public algebra::AbstractVertex<Position, edge> {");
  fprintf(out, "public:");

  t_node = v_types.first;
  p_node = v_properties.first;

  if (t_node)
    fprintf(out, "  vertex() : algebra::AbstractVertex<Position, edge>() {}");

  fprintf(out, "  vertex(");

  while (t_node) {
    fprintf(out, "%s %s", t_node->val, p_node->val);
    t_node = t_node->next;
    p_node = p_node->next;

    if (t_node) fprintf(out, ", ");
  }

  fprintf(out, ") : algebra::AbstractVertex<Position, edge>() {");

  p_node = v_properties.first;
  while (p_node) {
    fprintf(out, "    this->getPosition().%s = %s;", p_node->val, p_node->val);
    p_node = p_node->next;
  }

  fprintf(out, "  }};");
}

void WriteVDecl() {
  fprintf(out, "vertex::VPtr");
}

void VNew() {
  fprintf(out, " = (new vertex())->vptr();");
}

void StartMProp() {
  char mesh_code[] = 
    "class mesh : public algebra::XMLMesh<vertex> {"
    "public:"
    "  mesh();"
    "  ~mesh();"
    "  mesh& operator=(const mesh& m) {"
    "    algebra::AbstractMesh<vertex>::operator=(m);"
    "    return *this;"
    "  }"
    "  void writeMeshAttributes(std::ostream& os);";
  fprintf(out, mesh_code);
}

void EndMProp() {
  struct list_node* node;
  char code0[] =
    "class Parser : public algebra::AbstractXMLMeshParser<vertex> {"
    "public:"
    "  Parser(mesh* m) : algebra::AbstractXMLMeshParser<vertex>(m) {}"
    "  ~Parser() {}"
    "protected:"
    "  virtual void processMeshAttribute(algebra::XMLMesh<vertex>* pM, std::string name, std::istringstream& is) {";
  char code1[] =
    "  }"
    "  virtual void processVertexAttribute(algebra::AbstractMesh<vertex>::VPtr& v, std::string name, std::istringstream& is) {";
  char code2[] =
    "  }"
    "  virtual void processEdgeAttribute(vertex::edge_type& e, std::string name, std::istringstream& is) {";
  char code3[] =
    "  }"
    "};"
    "class Writer : public algebra::AbstractXMLMeshWriter<vertex> {"
    "public:"
    "  void operator()(algebra::AbstractMesh<vertex>::VPtr v) {"
    "    *os << \"  <v\";";
  char code4[] =
    "    *os <<  \" nb=\\\"\";"
    "    v->forEachNeighbour(write_neighbourhood);"
    "    *os << \"\\\"/>\" << std::endl;"
    "  }"
    "};"
    "class WriteEdges : public algebra::AbstractXMLWriteEdges<vertex> {"
    "  void operator()(vertex::VPtr v, vertex::VPtr n) {"
    "    if ((v^n) == (n^v)) {"
    "      if (v < n) {"
    "        *os << \"  <e first=\\\"\" << v->getLabel()"
    "            << \"\\\" second=\\\"\" << n->getLabel()"
    "            << \"\\\" symmetric=\\\"1\\\" \";";
  char code5[] =
    "        *os << \" />\" << std::endl;"
    "      }"
    "    }"
    "    else {"
    "      *os << \"  <e first=\\\"\" << v->getLabel()"
    "          << \"\\\" second=\\\"\" << n->getLabel()"
    "          << \"\\\" symmetric=\\\"0\\\" \";";
  char code6[] =
    "      *os << \" />\" << std::endl;"
    "    }"
    "  }"
    "};"
    "class EWriter : public algebra::AbstractXMLMeshEdgeWriter<vertex> {"
    "public:"
    "  EWriter() {"
    "    write_edges = new WriteEdges();"
    "  }"
    "  ~EWriter() {"
    "     delete write_edges;"
    "  }"
    "};"
    "\n#include <set>\n"
    "std::set<mesh*> ___meshes;"
    "mesh::mesh() {"
    "  pParser = new Parser(this);"
    "  pWriter = new Writer();"
    "  pEWriter = new EWriter();"
    "  ___meshes.insert(this);"
    "}"
    "mesh::~mesh() {"
    "  delete pParser;"
    "  delete pWriter;"
    "  delete pEWriter;"
    "  ___meshes.erase(this);"
    "}"
    "void mesh::writeMeshAttributes(std::ostream& os) {";
  char code7[] =
    "}"
    "mesh* ___m;"
    "extern \"C\" {"
    "  void vvp_error() {"
    "    std::set<mesh*>::iterator i;"
    "    for (i = ___meshes.begin(); i != ___meshes.end(); ++i)"
    "      ___m = *i;"
    "      ___m->printXMLFile();"
    "  }"
    "}";

  fprintf(out, code0);
  node = m_properties.first;
  while (node) {
    fprintf(out, "    if (name == \"%s\") is >> dynamic_cast<mesh*>(pM)->%s;", node->val, node->val);
    node = node->next;
  }
  fprintf(out, code1);
  node = v_properties.first;
  while (node) {
    fprintf(out, "    if (name == \"%s\") is >> v->getPosition().%s;", node->val, node->val);
    node = node->next;
  }
  fprintf(out, code2);
  node = e_properties.first;
  while (node) {
    fprintf(out, "    if (name == \"%s\") is >> e.%s;", node->val, node->val);
    node = node->next;
  }
  fprintf(out, code3);
  node = v_properties.first;
  while (node) {
    fprintf(out, "    *os << \" %s=\\\"\" << v->getPosition().%s << \"\\\"\";", node->val, node->val);
    node = node->next;
  }
  fprintf(out, code4);
  node = e_properties.first;
  while (node) {
    fprintf(out, "  *os << \" %s=\\\"\" << (v|n).%s << \"\\\"\";", node->val, node->val);
    node = node->next;
  }
  fprintf(out, code5);
  node = e_properties.first;
  while (node) {
    fprintf(out, "  *os << \" %s=\\\"\" << (v^n).%s << \"\\\"\";", node->val, node->val);
    node = node->next;
  }
  fprintf(out, code6);
  node = m_properties.first;
  while (node) {
    fprintf(out, "  os << \" %s=\\\"\" << this->%s << \"\\\"\";", node->val, node->val);
    node = node->next;
  }
  fprintf(out, code7);
  print_line( lineno );
}

void WriteMDecl() {
  fprintf(out, "mesh");
}

int main(int argc, char** argv) {
#ifdef WIN32
  char dll[] =
    "#include <windows.h>\n"
    "BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {"
    "  switch (ul_reason_for_call) {"
    "  case DLL_PROCESS_ATTACH:"
    "  case DLL_THREAD_ATTACH:"
    "  case DLL_THREAD_DETACH:"
    "  case DLL_PROCESS_DETACH:"
    " 	 break;"
    "  }"
    "  return TRUE;"
    "}";
char proxy[] =
    "\n#include <generation/proxy.hpp>\n"
    "#include <algebra/opqueue.hpp>\n"
    "extern \"C\" {"
    "  ___vvproxy::Proxy __declspec(dllexport) proxy;"
    "}";
#else
char proxy[] =
    "\n#include <generation/proxy.hpp>\n"
    "#include <algebra/opqueue.hpp>\n"
    "___vvproxy::Proxy proxy;\n"
    "extern \"C\" {\n"
    "  ___vvproxy::Proxy& getProxy() {return proxy;}\n"
    "}";
#endif

  if (argc != 3) {
    fprintf(stderr, "Usage: vvp2cpp <input vvp file> <output cpp file>");
    return -1;
  }

  yyin = fopen(argv[1], "r");
  if (!yyin) {
    fprintf(stderr, "Error: Could not open the input file.");
    return -2;
  }

  filename = argv[1];

  out = fopen(argv[2], "w");
  if (!out) {
    fprintf(stderr, "Error: Could not open the output file.");
    return -2;
  }

  fprintf(out, "%s", proxy);
  print_line( 1 );

  yyparse();

  fprintf(out, "\n");

  return 0;
}

// everything that follows is blatantly copied from
//  Radek's l2c/main.cpp

extern int yylineno;
extern char *yytext;
char FileName[] = "";

void yyerror(const char* fmt, ...) {
  static char aux[1025];
  va_list args;
  va_start(args, fmt);
  vsprintf(aux, fmt, args);
  va_end(args);
  fprintf(stderr, "Error: %s in %s line %d. Current token: %s\n", aux, FileName, yylineno, yytext);
}

void yywarning(const char* fmt, ...) {
  static char msg[1025];
  va_list args;
  va_start(args, fmt);
  vsprintf(msg, fmt, args);
  va_end(args);
  fprintf(stderr, "Warning/Info: %s in %s line %d. Current token: %s\n", msg, FileName, yylineno, yytext);
}

void w(const char* bf) {
  fputs(bf, out);
}

void wc(const char c) {
  fputc(c, out);
}

