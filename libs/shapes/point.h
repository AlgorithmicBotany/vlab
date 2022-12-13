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




#ifndef POINT_H
#define POINT_H

#include<assert.h>
#include <cmath>
#include <fstream>
#include <iostream>

using std::ostream;
using std::istream;
using std::cout;
using std::cin;

namespace Shapes {
template<class T, int N> class Point {
 public:
   Point(const Point& p);
   Point();
   ~Point();
   Point(T x, T y)
     {coord[0]=x;coord[1]=y;};
   Point(T x, T y, T z)
     {coord[0]=x;coord[1]=y;coord[2]=z;};

  void Set(int i, T val);
  T Get(int i) const;
  T *GetData()
    {return( (T *)coord);}
  // void SetDebug(bool nd){dbug=nd;};
  T& operator[](unsigned int i);
  Point &operator=(const Point &t);
  Point operator+(const Point &t);
  Point operator-(const Point &t);
  Point operator*(const T &c);
  Point operator/(const T &c);
  Point operator+=(const Point<T, N> &c);
  Point operator-=(const Point<T, N> &c);
  Point operator*=(const T &c);
  Point operator/=(const T &c);
  //Inner Product
  T operator*(const Point &t);
  T Length() const;
  T Distance(const Point &t)
    { return (*this-t).Length();}
  void Set(T x, T y, T z)
    {assert(N>=3);coord[0]=x;coord[1]=y,coord[2]=z;}
  void Set(T x, T y)
    {assert(N>=2);coord[0]=x;coord[1]=y;}
  Point Cross(const Point &t);

  friend Point operator*(const T &c,const Point &t) {
    Point p;
    for(int i=0;i<N;i++)
      p[i]=c*t.Get(i);
    return p;
  }

  private:
    T coord[N];
};

template<class T, int N>
Point<T, N>::Point(){
  for(int i=0;i<N;i++)
    coord[i]=0;
}

template<class T, int N>
Point<T, N>::~Point() {}

template<class T, int N>
Point<T, N> Point<T, N>::Cross(const Point<T, N> &t){
  Point<T, N> p;
  assert(N == 3);
  p[0]=t.Get(2)*Get(1)-t.Get(1)*Get(2);
  p[1]=t.Get(0)*Get(2)-t.Get(2)*Get(0);
  p[2]=t.Get(1)*Get(0)-t.Get(0)*Get(1);
  return p;
}

template<class T, int N>
void Point<T, N>::Set(int i, T val){
  assert(0<=i && i<=N);
  coord[i]=val;
}

template<class T, int N> T Point<T, N>::Get(int i) const
{
  assert(0<=i && i<=N);
  return coord[i];
}

template<class T, int N> T& Point<T, N>::operator[](unsigned int i)
{
  assert(/*0<=i &&*/ i<=N); // i is unsigned => 0 is always <= i
  return *(coord + i);
}

template<class T, int N> Point<T, N>::Point(const Point<T, N> &p)
{
  (*this)=p;
}

template<class T, int N> Point<T, N> &Point<T, N>::operator=(const Point<T, N> &t)
{
  //  Point<N> p;
  for(int i=0;i<N;i++)
    coord[i]=t.Get(i);
  return *this;
}

template<class T, int N> Point<T, N> Point<T, N>::operator+(const Point<T, N> &t)
{
  Point<T, N> p;
  for(int i=0;i<N;i++)
       p[i]=t.Get(i)+coord[i];
  return p;
}

template<class T, int N> Point<T, N> Point<T, N>::operator-(const Point<T, N> &t)
{
  Point<T, N> p;
  for(int i=0;i<N;i++)
       p[i]=coord[i]-t.Get(i);
  return p;
}

template<class T, int N> Point<T, N> Point<T, N>::operator*(const T &c)
{
  Point<T, N> p;
  for(int i=0;i<N;i++)
    p[i]=c*coord[i];
  // if(dbug==true) std::cout<<c<<" ";
  return p;
}

template<class T, int N> Point<T, N> Point<T, N>::operator/(const T &c)
{
  Point<T, N> p;
  for(int i=0;i<N;i++)
    p[i]=coord[i]/c;
  // if(dbug==true) std::cout<<c<<" ";
  return p;
}

template<class T, int N>
Point<T, N> Point<T, N>::operator+=(const Point<T, N> &c)
{
  (*this)=(*this)+c;
  return (*this);
}

template<class T, int N>
Point<T, N> Point<T, N>::operator-=(const Point<T, N> &c)
{
  (*this)=(*this)-c;
  return (*this);
}

template<class T, int N>
Point<T, N> Point<T, N>::operator*=(const T &c)
{
  (*this)=(*this)*c;
  return (*this);
}

template<class T, int N>
Point<T, N> Point<T, N>::operator/=(const T &c)
{
  (*this)=(*this)/c;
  return (*this);
}

template<class T, int N> T Point<T, N>::operator*(const Point &t)
{
  T p;
  p=0;
  for(int i=0;i<N;i++)
    p+=t.Get(i)*coord[i];
  return p;

}

template<class T, int N> T Point<T, N>::Length() const
{
  T leng=0;
  for(int i=0;i<N;i++)
    leng+=(*this).Get(i)*(*this).Get(i);
  leng=sqrt(leng);
  return leng;
}

template<class T, int N> ostream& operator<<(ostream& os, const Point<T, N>& p)
{
  for(int i=0;i<N-1;i++)
    os<<p.Get(i)<<" ";
  os<<p.Get(N-1);
  return os;
}

template<class T, int N> istream& operator>>(istream& is,Point<T, N>& p)
{
   for(int i=0;i<N;i++)
     is>>p[i];
   return is;
}

} // Namespace Shapes

#endif
