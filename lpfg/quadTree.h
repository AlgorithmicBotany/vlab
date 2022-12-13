/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

/*
Header file for implementation of a quad tree for use with
a Level of Detail algorithm for drawing terrain.

By: Steven Longay
June 13th 2008

Algorithm used is based off of that explained in the paper
Level of Detail for Terrain Geometry Images, 2007
Duncan Andrew Keith Mc Roberts and Alexandre Hardy
*/

#include <iostream>

#ifndef _QUAD_TREE_H
#define _QUAD_TREE_H

enum Quadrant { NE_QUAD, NW_QUAD, SE_QUAD, SW_QUAD, NONE };

template <class T> class QuadTree {
public:
  QuadTree(QuadTree *_parent, int startLevel, Quadrant _isFrom);
  ~QuadTree();

  // Setters and getters
  void setNW(QuadTree *);
  QuadTree *getNW();
  void setNE(QuadTree *);
  QuadTree *getNE();
  void setSW(QuadTree *);
  QuadTree *getSW();
  void setSE(QuadTree *);
  QuadTree *getSE();

  void spawnChildren();
  void setGlobalPointers();

  T *data;
  int level;
  QuadTree *parent;
  Quadrant isFrom;
  QuadTree *N, *E, *S,
      *W; // global positioning if quadtree is used for layout of a grid
  QuadTree *NW, *NE, *SW, *SE;
};

template <class T>
QuadTree<T>::QuadTree(QuadTree *_parent, int startLevel, Quadrant _isFrom) {
  NW = NE = SE = SW = NULL;
  N = E = S = W = NULL;
  level = startLevel;
  parent = _parent;
  isFrom = _isFrom;
}

template <class T> QuadTree<T>::~QuadTree() {
  if (NW != NULL)
    delete NW;
  if (NE != NULL)
    delete NE;
  if (SW != NULL)
    delete SW;
  if (SE != NULL)
    delete SE;
}

template <class T> void QuadTree<T>::spawnChildren() {
  // Clear out any existing children if there are any
  if (NW != NULL)
    delete NW;
  if (NE != NULL)
    delete NE;
  if (SW != NULL)
    delete SW;
  if (SE != NULL)
    delete SE;

  NW = new QuadTree<T>(this, level + 1, NW_QUAD);
  NE = new QuadTree<T>(this, level + 1, NE_QUAD);
  SW = new QuadTree<T>(this, level + 1, SW_QUAD);
  SE = new QuadTree<T>(this, level + 1, SE_QUAD);
}

template <class T> void QuadTree<T>::setGlobalPointers() {
  // NW
  if (N != NULL)
    NW->N = N->SW;

  NW->E = NE;
  NW->S = SW;

  if (W != NULL)
    NW->W = W->NE;

  // NE
  if (N != NULL)
    NE->N = N->SE;

  if (E != NULL)
    NE->E = E->NW;

  NE->S = SE;
  NE->W = NW;

  // SW
  SW->N = NW;
  SW->E = SE;

  if (S != NULL)
    SW->S = S->NW;

  if (W != NULL)
    SW->W = W->SE;

  // SE
  SE->N = NE;

  if (E != NULL)
    SE->E = E->SW;

  if (S != NULL)
    SE->S = S->NE;

  SE->W = SW;
}

// Setters and getters
template <class T> void QuadTree<T>::setNW(QuadTree *qt) {
  if (NW != NULL)
    delete NW;
  NW = qt;
}

template <class T> QuadTree<T> *QuadTree<T>::getNW() { return NW; }

template <class T> void QuadTree<T>::setNE(QuadTree<T> *qt) {
  if (NE != NULL)
    delete NE;
  NE = qt;
}

template <class T> QuadTree<T> *QuadTree<T>::getNE() { return NE; }

template <class T> void QuadTree<T>::setSW(QuadTree<T> *qt) {
  if (SW != NULL)
    delete SW;
  SW = qt;
}

template <class T> QuadTree<T> *QuadTree<T>::getSW() { return SW; }

template <class T> void QuadTree<T>::setSE(QuadTree<T> *qt) {
  if (SE != NULL)
    delete SE;
  SE = qt;
}

template <class T> QuadTree<T> *QuadTree<T>::getSE() { return SE; }

#endif
