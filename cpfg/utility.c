
/*
MODULE:		utility.c
AUTHOR:		J. Hanan
STARTED:	September 1991
                        Incorporates routines from render.c
                        AUTHOR: P. Prusinkiewicz STARTED ON:	May 10, 1987
                MODIFIED: March-June 1994 BY: Radek
                ansi standard + prepared for C++
*/

#ifdef WIN32
#include "warningset.h"
#endif

#include <assert.h>
#include <errno.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef LINUX
#include <sys/types.h>
#endif

#ifndef WIN32
#include <unistd.h>
#else
#include <io.h>
#include <windows.h>
#include <rand.h>
#endif

#include "control.h"
#include "utility.h"
#include "indices.h"
#include "comlineparam.h"
#include "log.h"
#include <ctype.h> // 2012.03.19 PBdR: This is a necessary include for size_t
#define WARNING_LVL 0
#ifdef WIN32
char pathSymbol = '\\';
#endif
#ifdef LINUX
char pathSymbol = '/';
#endif
#ifdef MAC
char pathSymbol = '/';
#endif

unsigned short main_xsubi[3] = {0x330E, 0, 0};
/* current value of a seed for the
           random number generator */

#ifdef WIN32
int cpp(const char *src, const char *trg, const char *options);
#endif

FILE *PreprocessFile(const char *filename, const char *options) {
#ifdef WIN32
  static char tmpfile[2048];
  char buffer[2048];
  static FILE *fp = NULL;
  int res;

  if (NULL != fp) {
    res = fclose(fp);
    res = remove(tmpfile);
  }

  if (NULL == filename)
    return NULL;

  /* Set up temporary file name and preprocess.  */
  GetEnvironmentVariable("TEMP", tmpfile, 2048);
  strcat(tmpfile, "\\cpfg.XXXXXX");
  mktemp(tmpfile);
  GetShortPathName(tmpfile, tmpfile, 2048);

  sprintf(buffer, "%s %s > %s", clp.preprocessor, filename, tmpfile);

  cpp(filename, tmpfile, options);

  /* open preprocessed file */

  fp = fopen(tmpfile, "r");

  res = unlink(tmpfile); /* unlink the temp file immediately */
  /* UNIX will keep the file around until it is closed */

  return fp;
#else /* WIN32 */
  char tmpfile[2048];
  char buffer[2048];
  extern COMLINEPARAM clp;
#ifdef LINUX_OLD
  char tmpfile2[2048];
#endif
  FILE *fp = NULL;

  /* Set up temporary file name and preprocess.  */
  strcpy(tmpfile, "/tmp/cpfg.XXXXXX");
  int fd = mkstemp(tmpfile);
  if (fd == -1) {
    Message("Failed to create temporary file. %s\n", strerror(errno));
    return NULL;
  }
  close(fd);

#ifdef LINUX_OLD
  /* linux preprocessors don't accept all extensions */
  sprintf(tmpfile2, "/tmp/cpfg%dx%d.h", getuid(), getpid());

  sprintf(buffer, "cp %s %s", filename, tmpfile2);
  system(buffer);

  sprintf(buffer, "%s %s > %s", clp.preprocessor, tmpfile2, tmpfile);
  system(buffer);

  /* remove the temporary file with extension .h */
  sprintf(buffer, "rm %s", tmpfile2);
  system(buffer);
#else
  sprintf(buffer, "%s %s %s > %s", clp.preprocessor, options, filename,
          tmpfile);
  system(buffer);
#endif

  /* open preprocessed file */
  fp = fopen(tmpfile, "r");
  if (!fp)
    Message("Failed to open file. %s\n", strerror(errno));

  unlink(tmpfile); /* unlink the temp file immediately */
  /* UNIX will keep the file around until it is closed */

  return fp;
#endif /* WIN32 */
}

#ifdef WIN32 /* Use external GNU C-preprocessor cpp */

int cpp(const char *src, const char *trg, const char *options) {
  static char cmndLine[(_MAX_PATH + 1) * 3];
  static char buffer[256];
  static char cppfnm[_MAX_PATH + 4] = "";
  int ix;
  DWORD code;
  BOOL res;
  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  HANDLE hStdInR, hStdInW;
  HANDLE hStdOutR, hStdOutW;

  if (0 == cppfnm[0]) {
    char *bs;
    GetModuleFileName(NULL, cppfnm, _MAX_PATH + 2);
    bs = strrchr(cppfnm, '\\');
    if (NULL == bs)
      bs = cppfnm;
    else {
      ++bs;
      *bs = 0;
      GetShortPathName(cppfnm, cppfnm, _MAX_PATH + 2);
    }
    strcat(cppfnm, "vlabcpp"); /* This should use clp.preprocessor? */
  }

  {
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    CreatePipe(&hStdInR, &hStdInW, &sa, 0);
    CreatePipe(&hStdOutR, &hStdOutW, &sa, 0);
  }

  {
    si.cb = sizeof(STARTUPINFO);
    si.lpReserved = NULL;
    si.lpDesktop = NULL;
    si.lpTitle = NULL;
    si.dwX = si.dwY = si.dwXSize = si.dwYSize = 0;
    si.dwXCountChars = si.dwYCountChars = 0;
    si.dwFillAttribute = 0;
    si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.wShowWindow = SW_HIDE;
    si.cbReserved2 = 0;
    si.lpReserved = NULL;
    si.hStdInput = hStdInR;
    si.hStdOutput = hStdOutW;
    si.hStdError = hStdOutW;
  }
  sprintf(cmndLine, "%s -lang-c89 %s %s %s", cppfnm, options, src, trg);
  if (clp.verbose)
    Message("Preprocessing:\n%s\n", cmndLine);

  res = CreateProcess(NULL,     /* Application name */
                      cmndLine, /* Command line */
                      NULL,     /* Process attributes */
                      NULL,     /* Thread attributes */
                      TRUE,     /* Inherit handles */
                      0,        /* Creation flags */
                      NULL,     /* Environment */
                      NULL,     /* Current directory */
                      &si,      /* Startup info */
                      &pi);     /* Process information */

  if (!res) {
    DWORD err = GetLastError();
    CloseHandle(hStdInR);
    CloseHandle(hStdInW);
    CloseHandle(hStdOutR);
    CloseHandle(hStdOutW);

    if (err != 0)
      return -2;
    return -1;
  }

  WaitForSingleObject(pi.hProcess, INFINITE);

  ix = 0;
  for (;;) {
    DWORD read, avail;
    PeekNamedPipe(hStdOutR, NULL, 0, NULL, &avail, NULL);
    if (avail > 0) {
      ReadFile(hStdOutR, buffer + ix, 1, &read, NULL);
      if (10 == buffer[ix]) {
        buffer[ix + 1] = 0;
        Message(buffer);
        ix = 0;
      } else if (13 != buffer[ix]) {
        ix++;
        if (ix == 255) {
          buffer[ix] = 0;
          Message(buffer);
          ix = 0;
        }
      }
    } else
      break;
  }

  res = GetExitCodeProcess(pi.hProcess, &code);
  assert(res);
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
  CloseHandle(hStdInR);
  CloseHandle(hStdInW);
  CloseHandle(hStdOutR);
  CloseHandle(hStdOutW);

  return code;
}

#endif /* WIN32 */

/*************************************************************************/
/* The following code for producing random samples from a normal
   distribution was taken from "Numerical Recipies in C" (Press, Teukolsky,
   Vetterling, and Flannery, Second Edition, Cambridge University Press,
   1992).  The original code assumed zero mean and unit variance.  It has
   been modified here to support non-zero means and non-unit variances.

   - Mark Hammel, May 4, 1995
*/
/* modified by Radomir Mech:
   - iset should be set to 0 by srand(), otherwise the sequences are
     different in subsequent simulations
   - squared root of variance used as the parameter
   */
int iset = 10;

double nrand(double mean, double sqrt_variance) {
  static double gset;
  double fac, rsq, v1, v2;

  if (iset == 0) {
    do {
      v1 = 2.0 * erand48(main_xsubi) - 1.0;
      v2 = 2.0 * erand48(main_xsubi) - 1.0;
      rsq = v1 * v1 + v2 * v2;
    } while (rsq >= 1.0 || rsq == 0.0);
    fac = sqrt(-2.0 * log(rsq) / rsq);
    gset = v1 * fac;
    iset = 1;
    return v2 * fac * sqrt_variance + mean;
  } else {
    iset = 0;
    return gset * sqrt_variance + mean;
  }
}

void Do_srand(long int a) {
  main_xsubi[0] = (unsigned short)0x330E;
  main_xsubi[1] = (unsigned short)(0xFFFF & a);
  main_xsubi[2] = (unsigned short)(0xFFFF & (a >> 16));
  iset = 0;
}

double Do_ran(double v) { return erand48(main_xsubi) * v; }

double Do_nrand(double m, double t) { return nrand(m, t); }

double Do_bran(double a, double b) { return brand(a, b); }

double Do_biran(int n, double p) { return binrand(n, p); }

/*************************************************************************/

#ifndef min
double min(double a, double b) {
  if (a < b)
    return (a);

  return (b);
}
#endif

/*************************************************************************/
/* code provided by Paul Kruszewski <kruz@muff.cs.mcgill.ca> */
double brand(double beta_a, double beta_b) {
  double s, u, lambda;
  double U1, U2, V, Ym;

  /* set up */
  s = beta_a + beta_b;

  if (min(beta_a, beta_b) <= 1)
    lambda = min(beta_a, beta_b);
  else
    lambda = sqrt((2 * beta_a * beta_b - s) / (s - 2));

  u = beta_a + lambda;

  /* generator */
  do {
    do {
    } while ((U1 = erand48(main_xsubi)) == 0);
    do {
    } while ((U2 = erand48(main_xsubi)) == 0);
    V = (1 / lambda) * log(U1 / (1 - U1));
    Ym = beta_a * exp(V);
  } while (s * log(s / (beta_b + Ym)) + u * V - log(4) < log(U1 * U1 * U2));

  return Ym / (beta_b + Ym);
}

/*************************************************************************/
/* code provided by Paul Kruszewski <kruz@muff.cs.mcgill.ca> */
/* binomial(n,p) distribution */
/* simple and obvious O(n) algorithm */

double binrand(int n, double p) {
  /*
  Generation of samples from BINOMIAL distribution.
  based on the obvious coin flipping algorithm
          */
  int i, j;

  j = 0;

  for (i = 1; i <= n; i++) {
    j = j + (erand48(main_xsubi) <= p);
  }

  return (j);
}

/*************************************************************************/
float DotProduct(const float vec1[3], const float vec2[3]) {
  return vec1[0] * vec2[0] + vec1[1] * vec2[1] + vec1[2] * vec2[2];
}

/*************************************************************************/
double DDotProduct(const double vec1[3], const double vec2[3]) {
  return vec1[0] * vec2[0] + vec1[1] * vec2[1] + vec1[2] * vec2[2];
}

/*************************************************************************/
double DFDotProduct(const double vec1[3], const float vec2[3]) {
  int i;
  double value;

  for (i = 0, value = 0.0; i <= 2; i++)
    value += vec1[i] * vec2[i];

  return (value);
}

/*************************************************************************/
double DDistance(const double vec1[3], const double vec2[3]) {
  int i;
  double value;

  for (i = 0, value = 0.0; i <= 2; i++)
    value += (vec1[i] - vec2[i]) * (vec1[i] - vec2[i]);

  return (value);
}

/*************************************************************************/
/*---------------------------------------------------------------------------
 * Loads the cross product of v1 with v2 into the vector res.
 *---------------------------------------------------------------------------
 */
void CrossProduct(const float v1[3], const float v2[3], float res[3]) {
  res[eX] = v1[eY] * v2[eZ] - v1[eZ] * v2[eY];
  res[eY] = v1[eZ] * v2[eX] - v1[eX] * v2[eZ];
  res[eZ] = v1[eX] * v2[eY] - v1[eY] * v2[eX];
}

/*************************************************************************/
/*---------------------------------------------------------------------------
 * Loads the cross product of v1 with v2 into the vector res.
 *---------------------------------------------------------------------------
 */
void DCrossProduct(const double v1[3], const double v2[3], double res[3]) {
  res[eX] = v1[eY] * v2[eZ] - v1[eZ] * v2[eY];
  res[eY] = v1[eZ] * v2[eX] - v1[eX] * v2[eZ];
  res[eZ] = v1[eX] * v2[eY] - v1[eY] * v2[eX];
}

/*---------------------------------------------------------------------------
 * Normalize the vector v1.
 *---------------------------------------------------------------------------
 */
void Normalize(float v1[3]) {
  int i;
  float n;

  if ((n = sqrt(DotProduct(v1, v1))) != 0) {
    for (i = eX; i <= eZ; i++) {
      v1[i] *= (1.0 / n);
    }
  }
}

/*---------------------------------------------------------------------------
 * Normalize the vector v1.
 *---------------------------------------------------------------------------
 */
void DNormalize(double v1[3]) {
  int i;
  double n;

  if ((n = sqrt(DDotProduct(v1, v1))) != 0) {
    for (i = eX; i <= eZ; i++) {
      v1[i] *= (1.0 / n);
    }
  }
}

/* Matrix operations */

/* Print 4x4 matrix mat1 to mat2 */
void Matprt4x4(float mat1[4][4]) {
  int i, j;

  for (i = 0; i <= 3; i++) {
    for (j = 0; j <= 3; j++)
#ifdef GL
      Message("%f ", mat1[i][j]);
#else
      Message("%f ", mat1[j][i]);
#endif
    Message("\n");
  }
}

/* Print 3 element array */
void DPrint3(const double array[3]) {
  int j;

  for (j = 0; j < 3; j++)
    Message("%f ", array[j]);
  Message("\n");
}

/* Multiply 4x4 matrices mat1 and mat2 putting result in mat3 */
void MatMult4x4(const float mat1[4][4], const float mat2[4][4],
                float mat3[4][4]) {
  int i, j, k;
  float acc;

  for (i = 0; i <= 3; i++) {
    for (j = 0; j <= 3; j++) {
      acc = 0.0;
      for (k = 0; k <= 3; k++)
        acc += mat1[i][k] * mat2[k][j];
      /* not necessary to transpone for openGL */
      /* used only from patch.c */
      mat3[i][j] = acc;
    }
  }
}

/* Multiply 4x4 matrices mat1 and mat2 putting result in mat3 */
void MatMult4x4D(const double mat1[4][4], const double mat2[4][4],
                 double mat3[4][4]) {
  int i, j, k;
  double acc;

  for (i = 0; i <= 3; i++) {
    for (j = 0; j <= 3; j++) {
      acc = 0.0;
      for (k = 0; k <= 3; k++)
        acc += mat1[i][k] * mat2[k][j];
      /* not necessary to transpone for openGL */
      /* used only from patch.c */
      mat3[i][j] = acc;
    }
  }
}

/* Multiply a 3 element vector and a 4x4 matrix - the 4th row and column
   of the matrix are ignored. */
void VecMatMult(const float vec1[3], const float mat1[4][4], float vec2[3]) {
  int i, j;
  float acc;

  for (i = 0; i < 3; i++) {
    acc = 0.0;
    for (j = 0; j < 3; j++) {
      acc = acc + vec1[j] * mat1[i][j];
    }
    vec2[i] = acc;
  }
}

/* Multiply a 4 element vector and a 4x4 matrix - matrix in normal format
   i is row j is column (not as OpenGl matrices) */
void Vec4Mat4Mult(const float vec1[4], const float mat1[4][4], float vec2[4]) {
  int i, j;
  float acc;

  for (i = 0; i < 4; i++) {
    acc = 0.0;
    for (j = 0; j < 4; j++) {
      acc = acc + vec1[j] * mat1[j][i];
    }
    vec2[i] = acc;
  }
}

/* Multiply a 4x4 matrix and a 4 element vector - matrix in normal format
   i is row j is column. (premultiplication of matrix as in OpenGL matrices) */
void Mat4Vec4Mult(float mat1[4][4], float vec1[4], float vec2[4]) {
  int i, j;
  float acc;

  for (i = 0; i < 4; i++) {
    acc = 0.0;
    for (j = 0; j < 4; j++) {
      acc = acc + mat1[i][j] * vec1[j];
    }
    vec2[i] = acc;
  }
}

/* Compute the transpose of a 3x3 matrix.  */
void Transpose(float mat[3][3]) {
  int i, j;
  float tmp;

  for (i = 0; i < 3; i++)
    for (j = i + 1; j < 3; j++) {
      tmp = mat[i][j];
      mat[i][j] = mat[j][i];
      mat[j][i] = tmp;
    }
}

static float det2x2(float a, float b, float c, float d) {
  return a * d - b * c;
}

/* Compute the determinant of a 3x3 matrix. */

float Determinant(float mat[3][3]) {
  float a1 = mat[0][0], b1 = mat[0][1], c1 = mat[0][2];
  float a2 = mat[1][0], b2 = mat[1][1], c2 = mat[1][2];
  float a3 = mat[2][0], b3 = mat[2][1], c3 = mat[2][2];

  return a1 * det2x2(b2, b3, c2, c3) - b1 * det2x2(a2, a3, c2, c3) +
         c1 * det2x2(a2, a3, b2, b3);
}

/* Compute the adjoint of a 3x3 matrix.  The adjoint is returned in adj. */

void Adjoint(float mat[3][3], float adj[3][3]) {
  float a1 = mat[0][0], b1 = mat[0][1], c1 = mat[0][2];
  float a2 = mat[1][0], b2 = mat[1][1], c2 = mat[1][2];
  float a3 = mat[2][0], b3 = mat[2][1], c3 = mat[2][2];

  adj[0][0] = det2x2(b2, b3, c2, c3);
  adj[1][0] = -det2x2(a2, a3, c2, c3);
  adj[2][0] = det2x2(a2, a3, b2, b3);

  adj[0][1] = -det2x2(b1, b3, c1, c3);
  adj[1][1] = det2x2(a1, a3, c1, c3);
  adj[2][1] = -det2x2(a1, a3, b1, b3);

  adj[0][2] = det2x2(b1, b2, c1, c2);
  adj[1][2] = -det2x2(a1, a2, c1, c2);
  adj[2][2] = det2x2(a1, a2, b1, b2);
}

/* Compute the inverse of a 3x3 matrix.  The inverse is returned in inv. */

void Inverse(float mat[3][3], float inv[3][3]) {
  int i, j;
  float d = Determinant(mat);
  Adjoint(mat, inv);

  if (fabs(d) < 0.00001) {
    Warning("Warning: [utility.c] Singular matrix detected. Inverse not calculated.\n",WARNING_LVL);
    return;
  }

  for (i = 0; i < 3; i++)
    for (j = 0; j < 3; j++)
      inv[i][j] /= d;
}

/* Compute the transpose of a 4x4 matrix.  */
void TransposeMat4(float mat[4][4]) {
  int i, j;
  float tmp;

  for (i = 0; i < 4; i++)
    for (j = i + 1; j < 4; j++) {
      tmp = mat[i][j];
      mat[i][j] = mat[j][i];
      mat[j][i] = tmp;
    }
}

void IdentityMat4(float mat[4][4]) {
  int i, j;
  for (i = 0; i < 4; i++)
    for (j = 0; j < 4; j++)
      mat[i][j] = (i == j ? 1 : 0);
}

/* Convert polar cooordinates to Euclidean 3d coordinates */
void PolarTo3d(float theta, float phi, float radius, float p[3]) {
  p[0] = radius * cos(phi) * sin(theta);
  p[1] = radius * sin(phi) * sin(theta);
  p[2] = radius * cos(theta);
}

/********************************************************************/
static char buffer[1024]; /* buffer for constructing label */

char *MakeLabel(const char *label, int parameters, const float *values) {
  float p[10];
  int i;

  if (parameters > 10) {
    Message("Too many parameters in label\n");
    return "";
  }

  for (i = 0; i < parameters; i++)
    p[i] = values[i];

  sprintf(buffer, label, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8],
          p[9]);

  return buffer;
}

/* Change the extension on filename "name" to ".ext" (eg. ".rib").
If ".ext" is null, then any existing extension is removed.
*/
void changeExtension(char *name, char *ext) {
  char *symPointer;

  symPointer = strrchr(name, '.');
  if (symPointer == NULL)
    strcat(name, ext);
  else
    strcpy(symPointer, ext);
}

/* Strip the directory path from filename "name" */
void stripDirectory(char *name) {
  char *symPointer;
  char temp[1024];

  if ((symPointer = strrchr(name, pathSymbol)) == NULL)
    symPointer = name;
  else
    symPointer++;

  strcpy(temp, symPointer);
  strcpy(name, temp);
}

const char *ReadQuotedString(const char *line, char *bf, int bfsz) {
  int quoted = 0;
  const char *p = line;
  while (isspace(*p) && *p != 0)
    ++p;
  if ('\"' == *p)
    quoted = 1;

  if (quoted) {
    int pos = 0;
    ++p;
    while (*p != 0 && *p != '\"' && pos < bfsz - 1) {
      bf[pos] = *p;
      ++p;
      ++pos;
    }
    bf[pos] = 0;
    if (*p == '\"')
      ++p;
    return p;
  } else {
    int pos = 0;
    while (*p != 0 && (!isspace(*p)) && pos < bfsz - 1) {
      bf[pos] = *p;
      ++p;
      ++pos;
    }
    bf[pos] = 0;
    return p;
  }
}
