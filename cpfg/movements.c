/*  Functions handling tropisms
**  Author: Radomir Mech      Feb 1994
**
**  Three types of tropisms are considered:
**      - plagiotrophy: perpendicular to a vector
**      - orthotrophy: towards a vector
**      - torgue tropism: rotation around a heading vector to adjust up vector
**                        with a vector
**
**  Specification - in view file
**   code in interpret.c - function ReadViewData (new_tropism() called)
**
**  Change in function TurtleDraw (interpret.c)
**    - when a symbols F,G, or f,g are encountered    - tropisms_adjust()
**    - when ] is reached - update_tropism
**  Change in function call_blackbox (blackbox.c)
**    - when a symbol @Ti or  @Td is reached  - update_tropisms()
**  In function InitializeParameters
**     an initialization of the number or tropisms is added
**
**  Added structures:
**    structure of tropisms vectors and types (TROPISMSPARAM)
**    in structure ENVIRINMENTPARAM - all parameters
**    array of elasticities in structure TURTLE - what has to be stored in
**                                                turtle's stack
**
**  Old stuff is for compatibility reasons still present
**  but can be removed simply by deleting line
**    #define OLDTROPISMS
**  from  tropisms.h
**
   MODIFIED: March-June 1994 BY: Radek
             ansi standard + prepared for C++
*/

#ifdef WIN32
#include "warningset.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "interpret.h"
#include "utility.h"
#include "quaternions.h"
#include "comlineparam.h"

#include "log.h"

static struct TROPISMPARAM {
  double gea, geb, gec;
  double cosgamma[MAXMOVEMENTS];
  double singamma[MAXMOVEMENTS];
  double esingamma[MAXMOVEMENTS];
} tropism_param;

static struct TWISTPARAM {
  double dir[3];
  double length;
} twist_param;

/*********** global environmental structure ***********/

extern struct ENVIRONMENTPARAM environmentparam;

#define EPr environmentparam

/* ------------------------------------------------------------------------- */

double get_double(void) {
  char *token;

  token = strtok(NULL, " ,\t:");

  return atof(token);
}

/* ------------------------------------------------------------------------- */

int read_movement(char *movement_line, int type) {
  short index;
  double a, b, c, elast, len = 0.0;
  char *token;

  index = EPr.number_of_movements;

  if (index == MAXMOVEMENTS) {
    Message("Warning! No room for new movement, movement IGNORED!!!\n");
    return 0;
  }

  EPr.movements[index].type = type;

  /* default values */
  EPr.movements[index].gamma = 0.0;
  EPr.movements[index].elasticity = 0.0;
  EPr.movements[index].elasticity_incr = 0.0;

  token = strtok(movement_line, " \t:");
  for (;;) {
    if (token == NULL)
      break;

    switch (token[0]) {
    case 'T':
      EPr.movements[index].dir[0] = get_double();
      EPr.movements[index].dir[1] = get_double();
      EPr.movements[index].dir[2] = get_double();

      len = DDotProduct(EPr.movements[index].dir, EPr.movements[index].dir);
      if (len == 0.0) {
        Message("Tropism with 0 vector!\n");
        return 0;
      }
      len = 1.0 / sqrt(len); /* inverse length */
      break;

    case 'I':
      len *= get_double();
      if (len == 0.0) {
        Message("Tropism with 0 intensity!\n");
        return 0;
      }
      if (len > 1.0)
        len = 1.0;
      if (len < -1.0)
        len = -1.0;
      break;
    case 'A':
      EPr.movements[index].gamma = get_double();
      break;

    case 'E':
      EPr.movements[index].elasticity = get_double();
      break;

    case 'S':
      EPr.movements[index].elasticity_incr = get_double();
      break;
    default:
      Message("Tropism: unknown command '%c'.\n", token[0]);
      return 0;
    }
    token = strtok(NULL, " \t:\n");
  }
  /* parsing OK */

  EPr.number_of_movements++;

  /* normalize dir */
  EPr.movements[index].dir[0] *= len;
  EPr.movements[index].dir[1] *= len;
  EPr.movements[index].dir[2] *= len;

  VERBOSE("tropism vector (with proper intensity): %lf %lf %lf\n",
          EPr.movements[index].dir[0], EPr.movements[index].dir[1],
          EPr.movements[index].dir[2]);
  VERBOSE("tropism elasticity: initial %lf, step  %lf\n",
          EPr.movements[index].elasticity,
          EPr.movements[index].elasticity_incr);

  EPr.movements[index].initial_elasticity = EPr.movements[index].elasticity;
  /* to remember for the beginning of the next step */

  a = EPr.movements[index].dir[0];
  b = EPr.movements[index].dir[1];
  c = EPr.movements[index].dir[2];
  elast = EPr.movements[index].elasticity;

  if (index == 0) {
    /* some initializations at the beginning */
    twist_param.dir[0] = twist_param.dir[1] = twist_param.dir[2] = 0.0;

    tropism_param.gea = tropism_param.geb = tropism_param.gec = 0.0;
  }

  switch (EPr.movements[index].type) {
  case mtTropism:
    tropism_param.cosgamma[index] = cos(D_TO_R(EPr.movements[index].gamma));
    tropism_param.singamma[index] = sin(D_TO_R(EPr.movements[index].gamma));
    tropism_param.esingamma[index] = elast * tropism_param.singamma[index];

    tropism_param.gea += tropism_param.cosgamma[index] * elast * a;
    tropism_param.geb += tropism_param.cosgamma[index] * elast * b;
    tropism_param.gec += tropism_param.cosgamma[index] * elast * c;
    break;

  case mtTwist:
    twist_param.dir[0] += elast * a;
    twist_param.dir[1] += elast * b;
    twist_param.dir[2] += elast * c;
    twist_param.length = sqrt(DDotProduct(twist_param.dir, twist_param.dir));
    break;
  }

  return 1;
}

/* ------------------------------------------------------------------------- */
/* delta_elast = new_elasticity - old_elasticity
** Updates the global tropism vector for the particular type of movements
** Called from blackbox.c and when ] is encountered (elasticity is changed)
*/

void update_movement(short index, double delta_elast) {
  double a, b, c;

  a = EPr.movements[index].dir[0];
  b = EPr.movements[index].dir[1];
  c = EPr.movements[index].dir[2];

  EPr.movements[index].elasticity += delta_elast;

  switch (EPr.movements[index].type) {
  case mtTropism:
    if (tropism_param.cosgamma[index] != 0.0) {
      tropism_param.gea += tropism_param.cosgamma[index] * delta_elast * a;
      tropism_param.geb += tropism_param.cosgamma[index] * delta_elast * b;
      tropism_param.gec += tropism_param.cosgamma[index] * delta_elast * c;
    }

    if (tropism_param.singamma[index] != 0.0) {
      tropism_param.esingamma[index] +=
          delta_elast * tropism_param.singamma[index];
    }
    break;
  case mtTwist:
    twist_param.dir[0] += delta_elast * a;
    twist_param.dir[1] += delta_elast * b;
    twist_param.dir[2] += delta_elast * c;
    twist_param.length = sqrt(DDotProduct(twist_param.dir, twist_param.dir));
  }
}

/* ------------------------------------------------------------------------- */
/* Rotates a vector towards the given vector by the angle specified by the
** size of the torque vector (heading x given_vector)
** Returns 1 if rotated. Used only for torque tropism!
*/

static int rotate_to_vector(double adj_vec[3], const double des_dir[3]) {
  double vec[3], angle;
  double len;
  double bendsin;
  double q[4]; /* quaternion */

  DCrossProduct(adj_vec, des_dir, vec);
  if ((len = sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2])) <
      0.0000001)
    return 0;

  if (len > 1.0)
    len = 1.0;

  angle = len * 0.5; /* angle of rotation around vec /2 */

  vec[0] /= len;
  vec[1] /= len;
  vec[2] /= len; /* the rotation is around vec - necessary to normalize */

  q[0] = cos(angle);
  q[1] = (bendsin = sin(angle)) * vec[0];
  q[2] = bendsin * vec[1];
  q[3] = bendsin * vec[2];

  rot_by_quat(adj_vec, q);

  return 1;
}

/* ------------------------------------------------------------------------- */
/* input is the rotation vector with the size specifying the intensity */

#define sign(x) (x >= 0 ? 1.0 : -1.0)

static void adjust_by_orthotropism(struct TURTLE *tu, double vec[3]) {
  double angle;
  double len;
  double bendsin;
  double q[4]; /* quaternion */

  if ((len = sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2])) <
      0.000001)
    return; /* no rotation */
  if (len > 1.0)
    len = 1.0;

  angle = -len * 0.5;
  /* angle of rotation around vec /2 */

  len = 1.0 / len;
  vec[0] *= len;
  vec[1] *= len;
  vec[2] *= len; /* the rotation is around vec - necessary to normalize */

  q[0] = cos(angle);
  q[1] = (bendsin = sin(angle)) * vec[0];
  q[2] = bendsin * vec[1];
  q[3] = bendsin * vec[2];

  rot_by_quat(tu->heading, q);

  adjust_up_and_left(tu);
}

/* ------------------------------------------------------------------------- */
/*
 **/

void movement_adjust(struct TURTLE *tu) {
  double rot_vec[3], des_dir[3];
  double vec[3];
  double vec2[3];
  int i;
  double len;

  if (EPr.number_of_movements == 0)
    return;

  /* tropisms */

  des_dir[0] = tropism_param.gea;
  des_dir[1] = tropism_param.geb;
  des_dir[2] = tropism_param.gec;
  /* des_dir (V) stores the sum of all cos(gamma)*T_i*elast_i */

  /* des_dir (V) has to be adjusted for all tropisms with gamma!=0 */

  for (i = 0; i <= EPr.number_of_movements; i++) {
    if (EPr.movements[i].type == mtTropism) {
      if (tropism_param.singamma[i] != 0.0) {
        /* H x T_i */
        DCrossProduct(tu->heading, EPr.movements[i].dir, vec);
        len = sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);

        if (len < 0.0000001) {
          /* H = T_i. Use left vector to rotate around
          so R = tu->left instead of R = (H x T_i) / |H x T_i|
          and H x (T_i x R) = - R.|T_i| = -R.(H.T_i)
          (because R is perpendicular to H=T_i)
                  */
          vec[0] = tu->left[0];
          vec[1] = tu->left[1];
          vec[2] = tu->left[2];
          len = 1.0; /* length of tu->left is 1.0 */
        }

        /* T x (H x T_i) */
        DCrossProduct(EPr.movements[i].dir, vec, vec2);

        len = (tropism_param.esingamma[i] / len);
        /* sin(gamma)* elast_i / |H x T_i| */

        des_dir[0] += len * vec2[0];
        des_dir[1] += len * vec2[1];
        des_dir[2] += len * vec2[2];
      }
    }
  }

  DCrossProduct(tu->heading, des_dir, rot_vec);

  /* all tropisms together */
  adjust_by_orthotropism(tu, rot_vec);

  /* twist movements */
  if ((twist_param.dir[0] != 0.0) || (twist_param.dir[1] != 0.0) ||
      (twist_param.dir[2] != 0.0)) {
    /* We have to use a normalized twist_dir to find the rotation axis */
    vec2[0] = twist_param.dir[0];
    vec2[1] = twist_param.dir[1];
    vec2[2] = twist_param.dir[2];
    DNormalize(vec2);

    DCrossProduct(tu->heading, vec2, vec);

    if (DDotProduct(vec, vec) > 0.00000001) {
      /* only if H and twist_dir are not aligned */

      DCrossProduct(vec, tu->heading, des_dir);
      /* des_dir is now perpendicular to the heading vector and in the same
      plane as the heading vector and tropism vector - just projection to
      the plane perpendicular to the heading vector */

      des_dir[0] *= twist_param.length;
      des_dir[1] *= twist_param.length;
      des_dir[2] *= twist_param.length;

      if (rotate_to_vector(tu->up, des_dir)) {
        /* heading stays the same */
        DCrossProduct(tu->heading, tu->up, tu->left);
      }

      /* previous twist was desirable. set prev-up to current up*/
      tu->prev_heading[0] = tu->heading[0];
      tu->prev_heading[1] = tu->heading[1];
      tu->prev_heading[2] = tu->heading[2];

      tu->prev_up[0] = tu->up[0];
      tu->prev_up[1] = tu->up[1];
      tu->prev_up[2] = tu->up[2];
    }
  }
}
