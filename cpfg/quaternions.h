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



#ifndef __QUATERNIONS_H__
#define __QUATERNIONS_H__

/*
 * $RCSfile: quaternions.h,v $
 * $Revision: 1.1.1.1 $
 * $Date: 2005/07/04 21:15:25 $
 * $Author: federl $
 * $Locker:  $
 *
 * $Log: quaternions.h,v $
 * Revision 1.1.1.1  2005/07/04 21:15:25  federl
 * Imported vlab-for-mac port so that Colin and Radek can continue working on
 * bug-fixes
 *
 * Revision 1.3  2004/12/03 22:54:44  radekk
 * Sync with the windows version
 *
 * Revision 1.2  2001/08/31 23:21:20  federl
 * More changes (bugfixes) performed by Samantha Filkas.
 *
 * Revision 1.3  2001/07/09 19:59:15  filkas
 * Hoping it's right now.
 *
 * Revision 1.1.1.1  2001/06/12 20:14:02  filkas
 * Imported using TkCVS
 *
 * Revision 1.1  1993/12/16  19:20:05  jamesm
 * Experimental Quaternions code
 *
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void add_quats(const double q1[4], const double q2[4], double dest[4]);
void matrix_to_qt(double q[4], const double m[4][4]);
void qt_to_matrix(double m[4][4], const double q[4]);
void normalize_quat(double[4]);
void vec_to_quat(const double v[3], double q[4]);
void quat_to_vec(const double q[4], double v[3]);
void mult_quats(const double q1[4], const double q2[4], double dest[4]);
void rot_by_quat2(const double v[3], const double q[4], double dest[3]);
void rot_by_quat(double v[3], const double q[4]);
void inverse_quat(const double q[4], double dest[4]);

#ifdef __cplusplus
}
#endif

#endif
