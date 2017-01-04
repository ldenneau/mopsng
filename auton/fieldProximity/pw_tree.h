/*
   File:        mtree.h
   Author(s):   Kubica
   Created:     Mon June 8 2004
   Description: A tree structure for piecewise linear approximations.
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

#ifndef PW_TREE_H
#define PW_TREE_H

#include "pw_linear.h"
#include "obs.h"

typedef struct pw_tree {
  int num_points;

  pw_linear*  anchor;
  double radius;      /* RA/DEC Spherical dist */

  dyv* pw_radius;

  struct pw_tree* left;
  struct pw_tree* right;

  ivec *trcks;
} pw_tree;


/* ===== NOTE ======================================== */
/* The pw_linear tree assumes that ALL of the linear   */
/* approximations the same number of times at the SAME */
/* times.  This makes it MUCH fast to compute some     */
/* functions.                                          */
/* =================================================== */

/* The tree contains a pw_radius.  This is the radius of the    */
/* bundle at each knot point (time step).  The radius between   */
/* time steps i and i+1 is always <= max(radius(i),radius(i+1)) */

/* --------------------------------------------------------------------- */
/* --- Tree Memory Functions ------------------------------------------- */
/* --------------------------------------------------------------------- */

pw_tree* mk_empty_pw_tree(void);

/* Split all dim indicated whether to use a full (expensive) */
/* maximum distance split or to approximate it using the     */
/* widest node.                                              */
pw_tree* mk_pw_tree(pw_linear_array* arr, double ts, double te,
                    int max_leaf_pts, bool split_all_dim);

void free_pw_tree(pw_tree* old);


/* --------------------------------------------------------------------- */
/* --- Getter/Setter Functions ----------------------------------------- */
/* --------------------------------------------------------------------- */

int safe_pw_tree_N(pw_tree* tr);
int safe_pw_tree_num_points(pw_tree* tr);

bool safe_pw_tree_is_leaf(pw_tree* tr);
ivec* safe_pw_tree_tracks(pw_tree* tr);
pw_tree* safe_pw_tree_right_child(pw_tree* tr);
pw_tree* safe_pw_tree_left_child(pw_tree* tr);

double safe_pw_tree_radius(pw_tree* tr);
double safe_pw_tree_pw_radius(pw_tree* tr, int i);

pw_linear* safe_pw_tree_anchor(pw_tree* tr);
double safe_pw_tree_anchor_predict(pw_tree* tr, int dim, double t);

/* Allow a few speedups */
#ifdef AMFAST

#define pw_tree_N(X)               (X->num_points)
#define pw_tree_num_points(X)      (X->num_points)
#define pw_tree_is_leaf(X)         (X->left == NULL)
#define pw_tree_tracks(X)          (X->trcks)
#define pw_tree_right_child(X)     (X->right)
#define pw_tree_left_child(X)      (X->left)

#define pw_tree_radius(X)          (X->radius)
#define pw_tree_pw_radius(X,i)     (dyv_ref(X->pw_radius,i))

#define pw_tree_anchor(X)             (X->anchor)
#define pw_tree_anchor_predict(X,d,t) (pw_linear_predict(X->anchor,t,d));

#else

#define pw_tree_N(X)               (safe_pw_tree_num_points(X))
#define pw_tree_num_points(X)      (safe_pw_tree_num_points(X))
#define pw_tree_is_leaf(X)         (safe_pw_tree_is_leaf(X))
#define pw_tree_tracks(X)          (safe_pw_tree_tracks(X))
#define pw_tree_right_child(X)     (safe_pw_tree_right_child(X))
#define pw_tree_left_child(X)      (safe_pw_tree_left_child(X))

#define pw_tree_radius(X)          (safe_pw_tree_radius(X))
#define pw_tree_pw_radius(X,i)     (safe_pw_tree_pw_radius(X,i))

#define pw_tree_anchor(X)             (safe_pw_tree_anchor(X))
#define pw_tree_anchor_predict(X,d,t) (safe_pw_tree_anchor_predict(X,d,t))

#endif


/* --- Simple I/O Functions ------------------------------------ */

void fprintf_pw_tree_pts(FILE* f, pw_tree* tr);


#endif
