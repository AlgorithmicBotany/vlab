#ifdef WIN32
#include "warningset.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "quaternions.h"
#include "utility.h"

#define EPSILON 0.0000000001

/* local prototypes */
static double DDot_quat(const double q[4], const double p[4]);

/* ----------------------------------------------------------------------- */

void vscale(double *v, double div) {
  v[0] *= div;
  v[1] *= div;
  v[2] *= div;
}

void vcopy(const double *v1, double *v2) {
  int i;
  for (i = 0; i < 3; i++)
    v2[i] = v1[i];
}

void vcross(const double *v1, const double *v2, double *cross) {
  double temp[3];

  temp[0] = (v1[1] * v2[2]) - (v1[2] * v2[1]);
  temp[1] = (v1[2] * v2[0]) - (v1[0] * v2[2]);
  temp[2] = (v1[0] * v2[1]) - (v1[1] * v2[0]);
  vcopy(temp, cross);
}

void vadd(const double *src1, const double *src2, double *dst) {
  dst[0] = src1[0] + src2[0];
  dst[1] = src1[1] + src2[1];
  dst[2] = src1[2] + src2[2];
}

double vdot(const double *v1, const double *v2) {
  return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

static double DDot_quat(const double q[4], const double p[4]) {
  return q[0] * p[0] + q[1] * p[1] + q[2] * p[2] + q[3] * p[3];
}

void vec_to_quat(const double v[3], double q[4]) /* Radek */
{
  q[0] = 0;
  q[1] = v[0];
  q[2] = v[1];
  q[3] = v[2];
}

void quat_to_vec(const double q[4], double v[3]) /* Radek */
{
  v[0] = q[1];
  v[1] = q[2];
  v[2] = q[3];
}

void mult_quats(const double q1[4], const double q2[4],
                double dest[4]) /* Radek */
{
  dest[0] = q1[0] * q2[0] - (q1[1] * q2[1] + q1[2] * q2[2] + q1[3] * q2[3]);

  dest[1] = q1[0] * q2[1] + q2[0] * q1[1] + q1[2] * q2[3] - q1[3] * q2[2];
  dest[2] = q1[0] * q2[2] + q2[0] * q1[2] + q1[3] * q2[1] - q1[1] * q2[3];
  dest[3] = q1[0] * q2[3] + q2[0] * q1[3] + q1[1] * q2[2] - q1[2] * q2[1];
}

void inverse_quat(const double q[4], double dest[4]) /* Radek */
{
  double size = 1.0 / (DDot_quat(q, q));

  dest[0] = size * q[0];
  dest[1] = -size * q[1];
  dest[2] = -size * q[2];
  dest[3] = -size * q[3];
}

void rot_by_quat2(const double v[3], const double q[4],
                  double dest[3]) /* Radek */
{
  double qv[4];
  double qi[4];
  double qr[4];

  inverse_quat(q, qi);
  vec_to_quat(v, qv);

  mult_quats(qi, qv, qr);
  mult_quats(qr, q, qv);

  quat_to_vec(qv, dest);
}

void rot_by_quat(double v[3], const double q[4]) /* Radek */
{
  double qv[4];
  double qi[4];
  double qr[4];

  inverse_quat(q, qi);
  vec_to_quat(v, qv);

  mult_quats(qi, qv, qr);
  mult_quats(qr, q, qv);

  quat_to_vec(qv, v);
}

/*
 * Given two rotations, e1 and e2, expressed as quaternion rotations,
 * figure out the equivalent single rotation and stuff it into dest.
 *
 * NOTE: This routine is written so that q1 or q2 may be the same
 * as dest (or each other).
 */

void add_quats(const double q1[4], const double q2[4], double dest[4]) {
  double t1[4], t2[4], t3[4];
  double tf[4];

  vcopy(q1, t1);
  vscale(t1, q2[3]);

  vcopy(q2, t2);
  vscale(t2, q1[3]);

  vcross(q2, q1, t3);
  vadd(t1, t2, tf);
  vadd(t3, tf, tf);
  tf[3] = q1[3] * q2[3] - vdot(q1, q2);

  dest[0] = tf[0];
  dest[1] = tf[1];
  dest[2] = tf[2];
  dest[3] = tf[3];
}

/*
 * Quaternions always obey:  a^2 + b^2 + c^2 + d^2 = 1.0
 * If they don't add up to 1.0, dividing by their magnitued will
 * renormalize them.
 *
 * Note: See the following for more information on quaternions:
 *
 * - Shoemake, K., Animating rotation with quaternion curves, Computer
 *   Graphics 19, No 3 (Proc. SIGGRAPH'85), 245-254, 1985.
 * - Pletinckx, D., Quaternion calculus as a basic tool in computer
 *   graphics, The Visual Computer 5, 2-13, 1989.
 */
void normalize_quat(double q[4]) {
  int i;
  double mag;

  mag = (q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
  for (i = 0; i < 4; i++)
    q[i] /= mag;
}

/*
 * Build a rotation matrix, given a quaternion rotation.
 *
 */
void qt_to_matrix(double m[4][4], const double q[4]) {
  m[0][0] = 1.0 - 2.0 * (q[1] * q[1] + q[2] * q[2]);
  m[0][1] = 2.0 * (q[0] * q[1] - q[2] * q[3]);
  m[0][2] = 2.0 * (q[2] * q[0] + q[1] * q[3]);
  m[0][3] = 0.0;

  m[1][0] = 2.0 * (q[0] * q[1] + q[2] * q[3]);
  m[1][1] = 1.0 - 2.0 * (q[2] * q[2] + q[0] * q[0]);
  m[1][2] = 2.0 * (q[1] * q[2] - q[0] * q[3]);
  m[1][3] = 0.0;

  m[2][0] = 2.0 * (q[2] * q[0] - q[1] * q[3]);
  m[2][1] = 2.0 * (q[1] * q[2] + q[0] * q[3]);
  m[2][2] = 1.0 - 2.0 * (q[1] * q[1] + q[0] * q[0]);
  m[2][3] = 0.0;

  m[3][0] = 0.0;
  m[3][1] = 0.0;
  m[3][2] = 0.0;
  m[3][3] = 1.0;
}

void matrix_to_qt(double q[4], const double m[4][4]) {
  double s;
  double w_sqrd;
  double x_sqrd;
  double y_sqrd;

  w_sqrd = .25 * (1 + m[0][0] + m[1][1] + m[2][2]);

  if (w_sqrd > EPSILON) {
    q[3] = sqrt((double)w_sqrd);
    s = q[3] * 4.0;
    q[0] = (m[1][2] - m[2][1]) / s;
    q[1] = (m[2][0] - m[0][2]) / s;
    q[2] = (m[0][1] - m[1][0]) / s;
  } else {
    q[3] = 0.0;
    x_sqrd = -0.5 * (m[1][1] + m[2][2]);

    if (x_sqrd > EPSILON) {
      q[0] = sqrt(x_sqrd);
      s = 2.0 * q[0];
      q[1] = m[0][1] / s;
      q[2] = m[0][2] / s;
    } else {
      q[0] = 0.0;
      y_sqrd = 0.5 * (1.0 - m[2][2]);

      if (y_sqrd > EPSILON) {
        q[1] = sqrt(y_sqrd);
        q[2] = m[1][2] / (2.0 * q[1]);
      } else {
        q[1] = 0.0;
        q[2] = 1.0;
      }
    }
  }
}

/* Does a true linear interpolation, not a spherical linear interpolation...
   This is a little faster, but a little inaccurate in that it will speed up in
   the middle */

void qt_slerp_lin(double dst[4], double p[4], double q[4], double t) {
  double tmp[4];
  tmp[3] = p[3] + t * (q[3] - p[3]);
  tmp[0] = p[0] + t * (q[0] - p[0]);
  tmp[1] = p[1] + t * (q[1] - p[1]);
  tmp[2] = p[2] + t * (q[2] - p[2]);
  normalize_quat(tmp);
  dst[0] = tmp[0];
  dst[1] = tmp[1];
  dst[2] = tmp[2];
  dst[3] = tmp[3];
}

/* Does a spherical (4D) interpolation  */

void qt_slerp(double dst[4], double p[4], double q[4], double t) {
  double a, b, phi, cosphi, sinphi;

  normalize_quat(q);
  normalize_quat(p);

  if (fabs(cosphi = DDot_quat(q, p)) == 1.0) {
    /* both quaternions specify the same direction */
    /* OR the opposite ones - then no interpolation is done - THIS IS
    cpfg SPECIALITY!! - normally a quaternion perpendicular to q and p
    should be chosen and the interpolation is done through it.       */

    dst[0] = q[0];
    dst[1] = q[1];
    dst[2] = q[2];
    dst[3] = q[3];
    return;
  }

  sinphi = 1.0 / sqrt(1.0 - cosphi * cosphi);
  phi = acos(cosphi);

  a = sin(t * phi) * sinphi;
  b = sin((1.0 - t) * phi) * sinphi;

  dst[0] = q[0] * a + p[0] * b;
  dst[1] = q[1] * a + p[1] * b;
  dst[2] = q[2] * a + p[2] * b;
  dst[3] = q[3] * a + p[3] * b;
}
