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



#include "warningset.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <lparams.h>

#include "production.h"
#include "module.h"
#include "file.h"

extern int l2cparse();
extern FILE *l2cin;
FILE *fOut = NULL;

extern int l2cdebug;
extern int l2c_flex_debug;
extern bool first_time;
extern bool verbose;

void InitDefault();
void BuildCallers(ProductionType);
void BuildProdProtos(ProductionType);
int BuildProdProtos(int, ProductionType);
void BuildModuleSize();
void GenerateFixed();

const char *ModulesOnly = "-ModulesOnly";
const char *ProfileProductions = "-ProfileProductions";
bool bModulesOnly = false;
bool bProfileProductions = false;

char bf[2048];

bool ContainsParameter(int argc, char **argv, const char *flag) {
  for (int iParameter = 3; iParameter < argc; ++iParameter) {
    if (0 == strcmp(argv[iParameter], flag))
      return true;
  }
  return false;
}

int main(int argc, char **argv) {
  if (argc < 3)
    return -1;
  verbose = false;
  l2cdebug = 0;

  bModulesOnly = ContainsParameter(argc, argv, ModulesOnly);
  bProfileProductions = ContainsParameter(argc, argv, ProfileProductions);

  try {
    ReadTextFile src(argv[1]);
    WriteTextFile trg(argv[2]);
    l2cin = src.Fp();
    fOut = trg.Fp();
    first_time = true;
    InitDefault();
    l2cparse();

    if (bModulesOnly) {
    } else {
      BuildCallers(eProduction);
      BuildCallers(eDecomposition);
      BuildCallers(eInterpretation);

      BuildProdProtos(eProduction);
      BuildProdProtos(eDecomposition);
      BuildProdProtos(eInterpretation);

      BuildModuleSize();
      GenerateFixed();
    }

    fOut = NULL;
  } catch (const char *msg) {
    fprintf(stderr,"%s\n", msg);
    return 1;
  }
  return 0;
}

extern int lineno;
extern char *l2ctext;
extern char FileName[];

void l2cerror(const char *fmt, ...) {
  static char aux[1025];
  static char msg[1025];
  va_list args;
  va_start(args, fmt);
  vsprintf(aux, fmt, args);
  va_end(args);
  sprintf(msg, "Error: %s in %s line %d. Current token: %s\n", aux, FileName,
          lineno, l2ctext);
  throw msg;
}

void l2cwarning(const char *fmt, ...) {
  static char msg[1025];
  va_list args;
  va_start(args, fmt);
  vsprintf(msg, fmt, args);
  va_end(args);
  fprintf(stderr, "Warning/Info: %s in %s line %d. Current token: %s\n", msg,
          FileName, lineno, l2ctext);
}

void DumpOut(const char *bf) {
  assert(NULL != fOut);
  fputs(bf, fOut);
}
