#ifdef WIN32
#include "warningset.h"
#endif

#include <math.h>
#include <stdio.h>

#include "turtle.h"
#include "utility.h"
#include "quaternions.h"

/* ------------------------------------------------------------------------- */
/* adjusts the up and left vector to keep the segments from twist
   so the angle of prev_up with plane (prev_H,new_H) should be same as the
   angle of new_U with the same plane. Basically, a rotation moving prev_H
   to new_H is computed and prev_U is rotated by it (to get new_U).
   */

void adjust_up_and_left(TURTLE *turtle) {
  double rot[3];
  double len, angle, bendsin;
  double q[4];

  DCrossProduct(turtle->heading, turtle->prev_heading, rot);

  if ((len = sqrt(rot[0] * rot[0] + rot[1] * rot[1] + rot[2] * rot[2])) >
      0.000001) {
    turtle->up[0] = turtle->prev_up[0];
    turtle->up[1] = turtle->prev_up[1];
    turtle->up[2] = turtle->prev_up[2];

    if (len > 1)
      len = 1;

    angle = asin(len) * 0.5; /* angle of rotation around vec /2 */
    if (DDotProduct(turtle->heading, turtle->prev_heading) < 0)
      angle = M_PI * 0.5 - angle; /* because angle is already half */

    rot[0] /= len;
    rot[1] /= len;
    rot[2] /= len; /* the rotation is around rot - necessary to normalize */

    q[0] = cos(angle);
    q[1] = (bendsin = sin(angle)) * rot[0];
    q[2] = bendsin * rot[1];
    q[3] = bendsin * rot[2];

    rot_by_quat(turtle->up, q);
  }

  /* from up and heading get left */
  DCrossProduct(turtle->heading, turtle->up, turtle->left);
}
