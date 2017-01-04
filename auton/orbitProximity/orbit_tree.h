/*
   File:        orbit_tree.h
   Author(s):   Kubica
   Created:     Tues Nov 16 2004
   Description: A kd-tree of orbital parameters.
   Copyright (c) Carnegie Mellon University

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
  
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ORBIT_TREE_H
#define ORBIT_TREE_H

#include "orbit.h"

/* The index of the different parameters. */
#define ORBTREE_DIMS   6
#define ORBTREE_Q      0
#define ORBTREE_E      1
#define ORBTREE_W      2
#define ORBTREE_O      3
#define ORBTREE_I      4
#define ORBTREE_T      5

typedef struct orb_tree {
  int num_points;

  struct orb_tree* left;
  struct orb_tree* right;

  double hi[ORBTREE_DIMS];
  double lo[ORBTREE_DIMS];

  ivec *trcks;
} orb_tree;


/* Orbit Trees are KD trees designed for orbit parameters.        */
/* They contain information about a variety of factors (a,e,etc.) */
/* and the relative importance of these factors during tree       */
/* construction is controlled by a weight vector W.               */
/*    W[ORBTREE_Q] = 100.0 ->  Treat q as 100x more important     */
/*    W[ORBTREE_T] =   0.0 ->  Ignore T0                          */
dyv* mk_orb_tree_weights(double q, double e, double w, 
                         double O, double i, double t0);

/* --------------------------------------------------------------------- */
/* --- Tree Helper Functions ------------------------------------------- */
/* --------------------------------------------------------------------- */

void orb_tree_fill_bounds(orb_tree* tr, orbit_array* oarr, ivec* inds);


/* --------------------------------------------------------------------- */
/* --- Tree Memory Functions ------------------------------------------- */
/* --------------------------------------------------------------------- */

orb_tree* mk_empty_orb_tree();

orb_tree* mk_orb_tree(orbit_array* arr, dyv* W, int max_leaf_pts);

void free_orb_tree(orb_tree* old);


/* --------------------------------------------------------------------- */
/* --- Tree Access Functions ------------------------------------------- */
/* --------------------------------------------------------------------- */

int safe_orb_tree_size(orb_tree* tr);
int safe_orb_tree_num_points(orb_tree* tr);
ivec* safe_orb_tree_orbits(orb_tree* tr);

bool safe_orb_tree_leaf(orb_tree* tr);
orb_tree* safe_orb_tree_left(orb_tree* tr);
orb_tree* safe_orb_tree_right(orb_tree* tr);


double safe_orb_tree_max_q(orb_tree* tr);
double safe_orb_tree_max_e(orb_tree* tr);
double safe_orb_tree_max_i(orb_tree* tr);
double safe_orb_tree_max_w(orb_tree* tr);
double safe_orb_tree_max_O(orb_tree* tr);
double safe_orb_tree_max_t0(orb_tree* tr);
double safe_orb_tree_max(orb_tree* tr, int dim);

double safe_orb_tree_min_q(orb_tree* tr);
double safe_orb_tree_min_e(orb_tree* tr);
double safe_orb_tree_min_i(orb_tree* tr);
double safe_orb_tree_min_w(orb_tree* tr);
double safe_orb_tree_min_O(orb_tree* tr);
double safe_orb_tree_min_t0(orb_tree* tr);
double safe_orb_tree_min(orb_tree* tr, int dim);

double safe_orb_tree_mid_q(orb_tree* tr);
double safe_orb_tree_mid_e(orb_tree* tr);
double safe_orb_tree_mid_i(orb_tree* tr);
double safe_orb_tree_mid_w(orb_tree* tr);
double safe_orb_tree_mid_O(orb_tree* tr);
double safe_orb_tree_mid_t0(orb_tree* tr);
double safe_orb_tree_mid(orb_tree* tr, int dim);

double safe_orb_tree_rad_q(orb_tree* tr);
double safe_orb_tree_rad_e(orb_tree* tr);
double safe_orb_tree_rad_i(orb_tree* tr);
double safe_orb_tree_rad_w(orb_tree* tr);
double safe_orb_tree_rad_O(orb_tree* tr);
double safe_orb_tree_rad_t0(orb_tree* tr);
double safe_orb_tree_rad(orb_tree* tr, int dim);

#ifdef AMFAST 

#define orb_tree_size(X)              (X->num_points)
#define orb_tree_num_points(X)        (X->num_points)
#define orb_tree_orbits(X)            (X->trcks)

#define orb_tree_leaf(X)              (X->right == NULL)
#define orb_tree_left(X)              (X->left)
#define orb_tree_right(X)             (X->right)

#define orb_tree_max_q(X)             (X->hi[ORBTREE_Q])
#define orb_tree_max_e(X)             (X->hi[ORBTREE_E])
#define orb_tree_max_i(X)             (X->hi[ORBTREE_I])
#define orb_tree_max_O(X)             (X->hi[ORBTREE_O])
#define orb_tree_max_w(X)             (X->hi[ORBTREE_W])
#define orb_tree_max_t0(X)            (X->hi[ORBTREE_T])
#define orb_tree_max(X,dim)           (X->hi[dim])

#define orb_tree_min_q(X)             (X->lo[ORBTREE_Q])
#define orb_tree_min_e(X)             (X->lo[ORBTREE_E])
#define orb_tree_min_i(X)             (X->lo[ORBTREE_I])
#define orb_tree_min_O(X)             (X->lo[ORBTREE_O])
#define orb_tree_min_w(X)             (X->lo[ORBTREE_W])
#define orb_tree_min_t0(X)            (X->lo[ORBTREE_T])
#define orb_tree_min(X,dim)           (X->lo[dim])

#define orb_tree_mid_q(X)             ((X->hi[ORBTREE_Q]+X->lo[ORBTREE_Q])/2.0)
#define orb_tree_mid_e(X)             ((X->hi[ORBTREE_E]+X->lo[ORBTREE_E])/2.0)
#define orb_tree_mid_w(X)             ((X->hi[ORBTREE_W]+X->lo[ORBTREE_W])/2.0)
#define orb_tree_mid_O(X)             ((X->hi[ORBTREE_O]+X->lo[ORBTREE_O])/2.0)
#define orb_tree_mid_i(X)             ((X->hi[ORBTREE_I]+X->lo[ORBTREE_I])/2.0)
#define orb_tree_mid_t0(X)            ((X->hi[ORBTREE_T]+X->lo[ORBTREE_T])/2.0)
#define orb_tree_mid(X,dim)           ((X->hi[dim]+X->lo[dim])/2.0)

#define orb_tree_rad_q(X)             ((X->hi[ORBTREE_Q]-X->lo[ORBTREE_Q])/2.0)
#define orb_tree_rad_e(X)             ((X->hi[ORBTREE_E]-X->lo[ORBTREE_E])/2.0)
#define orb_tree_rad_w(X)             ((X->hi[ORBTREE_W]-X->lo[ORBTREE_W])/2.0)
#define orb_tree_rad_O(X)             ((X->hi[ORBTREE_O]-X->lo[ORBTREE_O])/2.0)
#define orb_tree_rad_i(X)             ((X->hi[ORBTREE_I]-X->lo[ORBTREE_I])/2.0)
#define orb_tree_rad_t0(X)            ((X->hi[ORBTREE_T]-X->lo[ORBTREE_T])/2.0)
#define orb_tree_rad(X,dim)           ((X->hi[dim]-X->lo[dim])/2.0)

#else

#define orb_tree_size(X)              (safe_orb_tree_size(X))
#define orb_tree_num_points(X)        (safe_orb_tree_num_points(X))
#define orb_tree_orbits(X)            (safe_orb_tree_orbits(X))

#define orb_tree_leaf(X)              (safe_orb_tree_leaf(X))
#define orb_tree_left(X)              (safe_orb_tree_left(X))
#define orb_tree_right(X)             (safe_orb_tree_right(X))

#define orb_tree_max_q(X)             (safe_orb_tree_max_q(X))
#define orb_tree_max_e(X)             (safe_orb_tree_max_e(X))
#define orb_tree_max_w(X)             (safe_orb_tree_max_w(X))
#define orb_tree_max_O(X)             (safe_orb_tree_max_O(X))
#define orb_tree_max_i(X)             (safe_orb_tree_max_i(X))
#define orb_tree_max_t0(X)            (safe_orb_tree_max_t0(X))
#define orb_tree_max(X,d)             (safe_orb_tree_max(X,d))

#define orb_tree_min_q(X)             (safe_orb_tree_min_q(X))
#define orb_tree_min_e(X)             (safe_orb_tree_min_e(X))
#define orb_tree_min_w(X)             (safe_orb_tree_min_w(X))
#define orb_tree_min_O(X)             (safe_orb_tree_min_O(X))
#define orb_tree_min_i(X)             (safe_orb_tree_min_i(X))
#define orb_tree_min_t0(X)            (safe_orb_tree_min_t0(X))
#define orb_tree_min(X,d)             (safe_orb_tree_min(X,d))

#define orb_tree_mid_q(X)             (safe_orb_tree_mid_q(X))
#define orb_tree_mid_e(X)             (safe_orb_tree_mid_e(X))
#define orb_tree_mid_w(X)             (safe_orb_tree_mid_w(X))
#define orb_tree_mid_O(X)             (safe_orb_tree_mid_O(X))
#define orb_tree_mid_i(X)             (safe_orb_tree_mid_i(X))
#define orb_tree_mid_t0(X)            (safe_orb_tree_mid_t0(X))
#define orb_tree_mid(X,d)             (safe_orb_tree_mid(X,d))

#define orb_tree_rad_q(X)             (safe_orb_tree_rad_q(X))
#define orb_tree_rad_e(X)             (safe_orb_tree_rad_e(X))
#define orb_tree_rad_w(X)             (safe_orb_tree_rad_w(X))
#define orb_tree_rad_O(X)             (safe_orb_tree_rad_O(X))
#define orb_tree_rad_i(X)             (safe_orb_tree_rad_i(X))
#define orb_tree_rad_t0(X)            (safe_orb_tree_rad_t0(X))
#define orb_tree_rad(X,d)             (safe_orb_tree_rad(X,d))

#endif


/* --------------------------------------------------------------------- */
/* --- Tree Display Functions ------------------------------------------ */
/* --------------------------------------------------------------------- */

void fprintf_orb_tree(FILE* f, orb_tree* tr);


/* --------------------------------------------------------------------- */
/* --- Tree Search Functions ------------------------------------------- */
/* --------------------------------------------------------------------- */

/* Use thresh = -1.0 for ignore. */
dyv* mk_orb_tree_search_thresh(double qthresh, double ethresh, 
                               double ithresh, double Othresh,
                               double wthresh, double t0thresh);

ivec* mk_orb_tree_range_search(orbit* q, orb_tree* tr, 
                               orbit_array* orbs, dyv* thresh);

ivec* mk_orb_tree_range_search_exh(orbit* q, orbit_array* orbs, 
                                   dyv* thresh, bool verb);

#endif
