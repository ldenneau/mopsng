/*
   File:        rdt_tree.h
   Author(s):   Kubica
   Created:     Mon June 2 15:50:29 EST 2004
   Description: Tree data structure for holding points in RA/DEC/time
                space.

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

#ifndef RDT_TREE_H
#define RDT_TREE_H

#include "track.h"

#define RDT_MAX_LEAF_NODES 10

#define RDT_DIM            4
#define RDT_T              0  /* Time            */
#define RDT_R              1  /* RA              */
#define RDT_D              2  /* DEC             */
#define RDT_B              3  /* Brightness/Flux */

typedef struct rdt_tree {
  int num_points;

  struct rdt_tree* left;
  struct rdt_tree* right;

  /* Bounds... */
  double hi[RDT_DIM];
  double lo[RDT_DIM];
  double mid[RDT_DIM];
  double rad[RDT_DIM];

  double radius;     /* Radius for RA/DEC */

  ivec *pts; 
} rdt_tree;


typedef struct rdt_tree_ptr_array {
  int size;
  int max_size;

  rdt_tree** trs;
} rdt_tree_ptr_array;


/* --- Tree Memory Functions -------------------------- */

rdt_tree* mk_empty_rdt_tree(void);

/* use_inds - is the indices to use (NULL to use ALL observations). */
/* force_t  - forces us to split on time first.                     */
rdt_tree* mk_rdt_tree(simple_obs_array* obs, ivec* use_inds,
                      bool force_t, int max_leaf_pts);


/* use_inds - is the indices to use (NULL to use ALL observations).    */
/* wt       - relative weighting to time (if > 0.0).                   */
/* wid      - if > 0.0, this is the width to initially split time to   */
/*            (i.e. split time to wid before splitting anything else). */
/* If both wt and wid are >0.0, wid is used.                           */
rdt_tree* mk_rdt_tree_wt(simple_obs_array* obs, ivec* use_inds,
                         double wt, double wid, int max_leaf_pts);

void free_rdt_tree(rdt_tree* old);

/* --------------------------------------------------------------------- */
/* --- Getter/Setter Functions ----------------------------------------- */
/* --------------------------------------------------------------------- */

int safe_rdt_tree_N(rdt_tree* tr);

int safe_rdt_tree_num_points(rdt_tree* tr);

bool safe_rdt_tree_is_leaf(rdt_tree* tr);

ivec* safe_rdt_tree_pts(rdt_tree* tr);

rdt_tree* safe_rdt_tree_right_child(rdt_tree* tr);
rdt_tree* safe_rdt_tree_left_child(rdt_tree* tr);

double safe_rdt_tree_RA(rdt_tree* p);
double safe_rdt_tree_DEC(rdt_tree* p);
double safe_rdt_tree_radius(rdt_tree* p);
double safe_rdt_tree_time(rdt_tree* p);

double safe_rdt_tree_lo_time(rdt_tree* p);
double safe_rdt_tree_hi_time(rdt_tree* p);
double safe_rdt_tree_mid_time(rdt_tree* p);
double safe_rdt_tree_rad_time(rdt_tree* p);

double safe_rdt_tree_lo_RA(rdt_tree* p);
double safe_rdt_tree_hi_RA(rdt_tree* p);
double safe_rdt_tree_mid_RA(rdt_tree* p);
double safe_rdt_tree_rad_RA(rdt_tree* p);

double safe_rdt_tree_lo_DEC(rdt_tree* p);
double safe_rdt_tree_hi_DEC(rdt_tree* p);
double safe_rdt_tree_mid_DEC(rdt_tree* p);
double safe_rdt_tree_rad_DEC(rdt_tree* p);

double safe_rdt_tree_lo_bright(rdt_tree* p);
double safe_rdt_tree_hi_bright(rdt_tree* p);
double safe_rdt_tree_mid_bright(rdt_tree* p);
double safe_rdt_tree_rad_bright(rdt_tree* p);


/* Allow a few speedups */
#ifdef AMFAST

#define rdt_tree_N(X)               (X->num_points)
#define rdt_tree_num_points(X)      (X->num_points)
#define rdt_tree_is_leaf(X)         (X->left == NULL)
#define rdt_tree_pts(X)             (X->pts)
#define rdt_tree_right_child(X)     (X->right)
#define rdt_tree_left_child(X)      (X->left)

#define rdt_tree_RA(X)              (X->mid[RDT_R])
#define rdt_tree_DEC(X)             (X->mid[RDT_D])
#define rdt_tree_radius(X)          (X->radius)
#define rdt_tree_time(X)            (X->mid[RDT_T])

#define rdt_tree_lo_time(X)         (X->lo[RDT_T])
#define rdt_tree_hi_time(X)         (X->hi[RDT_T])
#define rdt_tree_mid_time(X)        (X->mid[RDT_T])
#define rdt_tree_rad_time(X)        (X->rad[RDT_T])

#define rdt_tree_lo_RA(X)           (X->lo[RDT_R])
#define rdt_tree_hi_RA(X)           (X->hi[RDT_R])
#define rdt_tree_mid_RA(X)          (X->mid[RDT_R])
#define rdt_tree_rad_RA(X)          (X->rad[RDT_R])

#define rdt_tree_lo_DEC(X)          (X->lo[RDT_D])
#define rdt_tree_hi_DEC(X)          (X->hi[RDT_D])
#define rdt_tree_mid_DEC(X)         (X->mid[RDT_D])
#define rdt_tree_rad_DEC(X)         (X->rad[RDT_D])

#define rdt_tree_lo_bright(X)       (X->lo[RDT_B])
#define rdt_tree_hi_bright(X)       (X->hi[RDT_B])
#define rdt_tree_mid_bright(X)      (X->mid[RDT_B])
#define rdt_tree_rad_bright(X)      (X->rad[RDT_B])


#else

#define rdt_tree_N(X)               (safe_rdt_tree_num_points(X))
#define rdt_tree_num_points(X)      (safe_rdt_tree_num_points(X))
#define rdt_tree_is_leaf(X)         (safe_rdt_tree_is_leaf(X))
#define rdt_tree_pts(X)             (safe_rdt_tree_pts(X))
#define rdt_tree_right_child(X)     (safe_rdt_tree_right_child(X))
#define rdt_tree_left_child(X)      (safe_rdt_tree_left_child(X))

#define rdt_tree_RA(X)              (safe_rdt_tree_RA(X))
#define rdt_tree_DEC(X)             (safe_rdt_tree_DEC(X))
#define rdt_tree_radius(X)          (safe_rdt_tree_radius(X))
#define rdt_tree_time(X)            (safe_rdt_tree_time(X))

#define rdt_tree_lo_time(X)         (safe_rdt_tree_lo_time(X))
#define rdt_tree_hi_time(X)         (safe_rdt_tree_hi_time(X))
#define rdt_tree_mid_time(X)        (safe_rdt_tree_mid_time(X))
#define rdt_tree_rad_time(X)        (safe_rdt_tree_rad_time(X))

#define rdt_tree_lo_RA(X)           (safe_rdt_tree_lo_RA(X))
#define rdt_tree_hi_RA(X)           (safe_rdt_tree_hi_RA(X))
#define rdt_tree_mid_RA(X)          (safe_rdt_tree_mid_RA(X))
#define rdt_tree_rad_RA(X)          (safe_rdt_tree_rad_RA(X))

#define rdt_tree_lo_DEC(X)          (safe_rdt_tree_lo_DEC(X))
#define rdt_tree_hi_DEC(X)          (safe_rdt_tree_hi_DEC(X))
#define rdt_tree_mid_DEC(X)         (safe_rdt_tree_mid_DEC(X))
#define rdt_tree_rad_DEC(X)         (safe_rdt_tree_rad_DEC(X))

#define rdt_tree_lo_bright(X)       (safe_rdt_tree_lo_bright(X))
#define rdt_tree_hi_bright(X)       (safe_rdt_tree_hi_bright(X))
#define rdt_tree_mid_bright(X)      (safe_rdt_tree_mid_bright(X))
#define rdt_tree_rad_bright(X)      (safe_rdt_tree_rad_bright(X))

#endif


/* --- Simple I/O Functions ------------------------------------ */

void fprintf_rdt_tree_pts(FILE* f, rdt_tree* tr);

void fprintf_rdt_tree(FILE* f, rdt_tree* tr);

void fprintf_rdt_tree_node(FILE* f, char* pre, rdt_tree* tr, char* post);

void fprintf_rdt_tree_pts_list(FILE* f, rdt_tree* tr);

/* --------------------------------------------------------------------- */
/* --- Query Functions ------------------------------------------------- */
/* --------------------------------------------------------------------- */

int rdt_tree_range_count(rdt_tree* tr, simple_obs_array* arr,
                         simple_obs* X, double ts, double te, double thresh);


/* Finds the nearest neighbor to X within the time range [ts, te] */
/* such that the neighbor is within distance thresh.  If no such  */
/* point exists, returns -1.  To do a pure nearest neighbor (no   */
/* threshold), use thresh <= -1.0.                                */
int rdt_tree_NN(rdt_tree* tr, simple_obs_array* arr,
                simple_obs* X, double ts, double te,
                double thresh);

ivec* mk_rdt_tree_range_search(rdt_tree* tr, simple_obs_array* arr,
                               simple_obs* X, double ts, double te,
                               double thresh);

/* Find all observations occurring between times ts and te */
/* such that if X was allowed to move upto maxv then it    */
/* could endup within distance of thresh of the point.     */
/* Both maxv and thresh are given in radians.              */
ivec* mk_rdt_tree_moving_pt_query(rdt_tree* tr, simple_obs_array* arr,
                                  simple_obs* X, double ts, double te,
                                  double minv, double maxv, double thresh);



/* ------- Line Segment Based Queries ---------------------------------- */

/* Find all observations that come within threshold 'thresh' */
/* The line segments are defined by a matrix with 3 columns  */
/* and each row is [time_i ra_i dec_i] for knot point i      */
/* Thresh is the threshold in RADIANS.                       */
ivec* mk_rdt_tree_near_line_segs(rdt_tree* tr, simple_obs_array* arr,
                                 dym* segs, double thresh);


/* -------------------------------------------------------------------- */
/* --- RDT Tree Pointer Array ----------------------------------------- */
/* -------------------------------------------------------------------- */

rdt_tree_ptr_array* mk_sized_empty_rdt_tree_ptr_array(int size);

rdt_tree_ptr_array* mk_empty_rdt_tree_ptr_array(void);

void free_rdt_tree_ptr_array(rdt_tree_ptr_array* old);

/* --- Access Functions ------------------------------------------- */

rdt_tree* safe_rdt_tree_ptr_array_ref(rdt_tree_ptr_array* X, int index);

void rdt_tree_ptr_array_set(rdt_tree_ptr_array* X, int index, rdt_tree* A);

void rdt_tree_ptr_array_add(rdt_tree_ptr_array* X, rdt_tree* A);

int safe_rdt_tree_ptr_array_size(rdt_tree_ptr_array* X);

int safe_rdt_tree_ptr_array_max_size(rdt_tree_ptr_array* X);

/* Allow a few speedups */
#ifdef AMFAST

#define rdt_tree_ptr_array_size(X)     (X->size)
#define rdt_tree_ptr_array_max_size(X) (X->max_size)
#define rdt_tree_ptr_array_ref(X,i)    (X->trs[i])

#else

#define rdt_tree_ptr_array_size(X)     (safe_rdt_tree_ptr_array_size(X))
#define rdt_tree_ptr_array_max_size(X) (safe_rdt_tree_ptr_array_max_size(X))
#define rdt_tree_ptr_array_ref(X,i)    (safe_rdt_tree_ptr_array_ref(X,i))

#endif


#endif
