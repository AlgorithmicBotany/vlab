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



#ifndef BSURFACEOBJ_H
#define BSURFACEOBJ_H

#include "b_wrapper.h"
#include "include/lintrfc.h"

template <int N, int M>
BsurfaceObjNM<N, M> BsurfaceObjAdd(const BsurfaceObjNM<N, M> &l,
                                   const BsurfaceObjNM<N, M> &r) {
  BsurfaceObjNM<N, M> res;

  b_wrapper b1 = GetBsurface(l);
  b_wrapper b2 = GetBsurface(r);

  b1 = b1 + b2;

  res = ConstructBsurfaceObj(b1, res);

  return res;
}

template <int N, int M>
BsurfaceObjNM<N, M> BsurfaceObjSet(const BsurfaceObjNM<N, M> &l, int i, int j,
                                   V3f cp) {
  BsurfaceObjNM<N, M> res;

  res = l;
  res.Set(i, j, cp);
  return res;
}

template <int N, int M>
V3f BsurfaceObjGet(const BsurfaceObjNM<N, M> &l, int i, int j) {
  // Inefficient implementation, requires a copy to get the variable
  BsurfaceObjNM<N, M> k = l;

  return k.Get(i, j);
}

template <int N, int M> b_wrapper GetBsurface(const BsurfaceObjNM<N, M> &l) {

  V3f **t;
  int n_u, n_v;
  t = GetControlPoints(l, n_u, n_v);

  b_wrapper b(t, n_u, n_v);

  for (int i = 0; i < N; i++)
    delete t[i];
  delete t;

  return b;
}

template <int N, int M>
BsurfaceObjNM<N, M> ConstructBsurfaceObj(b_wrapper b,
                                         const BsurfaceObjNM<N, M> &) {

  V3f **t;
  t = new V3f *[N];
  for (int i = 0; i < N; i++)
    t[i] = new V3f[M];
  int n = N;
  int m = M;

  b.GetControlNet(t, n, m);

  BsurfaceObjNM<N, M> answer;
  ConstructBsurfaceObj(t, n, m, answer);
  return answer;
}

template <int N, int M>
BsurfaceObjNM<N, M> BsurfaceObjInterp(const BsurfaceObjNM<N, M> &l1,
                                      const BsurfaceObjNM<N, M> &l2,
                                      float alpha, int r, int c) {
  b_wrapper b1 = GetBsurface(l1);
  b_wrapper b2 = GetBsurface(l2);

  if (r > N)
    r = N;

  if (c > M)
    c = M;

  b2 = b1.Interpolate(b1, b2, alpha, r, c);

  BsurfaceObjNM<N, M> K = ConstructBsurfaceObj(b2, l1);

  return K;
}

inline BsurfaceObjS BsurfaceObjSAdd(const BsurfaceObjS &l,
                                    const BsurfaceObjS &r) {
  return BsurfaceObjAdd(l, r);
}
inline BsurfaceObjS BsurfaceObjSInterp(const BsurfaceObjS &l1,
                                       const BsurfaceObjS &l2, float alpha,
                                       int r, int c) {
  return BsurfaceObjInterp(l1, l2, alpha, r, c);
}

inline BsurfaceObjS BsurfaceObjSSet(const BsurfaceObjS &l1, int i, int j,
                                    V3f p) {
  return BsurfaceObjSet(l1, i, j, p);
}

inline V3f BsurfaceObjSGet(const BsurfaceObjS &l1, int i, int j) {
  return BsurfaceObjGet(l1, i, j);
}

inline BsurfaceObjM BsurfaceObjMSet(const BsurfaceObjM &l1, int i, int j,
                                    V3f p) {
  return BsurfaceObjSet(l1, i, j, p);
}

inline V3f BsurfaceObjMGet(const BsurfaceObjM &l1, int i, int j) {
  return BsurfaceObjGet(l1, i, j);
}

inline BsurfaceObjM BsurfaceObjMAdd(const BsurfaceObjM &l,
                                    const BsurfaceObjM &r) {
  return BsurfaceObjAdd(l, r);
}
inline BsurfaceObjM BsurfaceObjMInterp(const BsurfaceObjM &l1,
                                       const BsurfaceObjM &l2, float alpha,
                                       int r, int c) {
  return BsurfaceObjInterp(l1, l2, alpha, r, c);
}

#endif
