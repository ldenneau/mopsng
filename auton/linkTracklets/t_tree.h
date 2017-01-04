/*
   File:        t_tree.h
   Author(s):   Kubica
   Created:     Mon June 8 2004
   Description: Data struture for a tree structure to hold RA/DEC points 
                with velocities.
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

#ifndef T_TREE_H
#define T_TREE_H

#include "track.h"

#define T_TIME  0    /* Track time         */
#define T_R     1    /* Track RA           */
#define T_D     2    /* Track DEC          */
#define T_VR    3    /* Track RA Velocity  */
#define T_VD    4    /* Track DEC Velocity */
#define T_BR    5    /* Track Brightness   */

#define T_NUM_DIMS 6

typedef struct t_tree {
  int num_points;
  int split_dim;
  double split_val;
  bool from_tracks;

  struct t_tree* left;
  struct t_tree* right;

  double hi[T_NUM_DIMS];
  double lo[T_NUM_DIMS];

  ivec *trcks;
} t_tree;


/* A structure for astronomical bounds */

#define TB_R    0  /* Track RA   */
#define TB_D    1  /* Track DEC  */

#define TB_NUM_DIMS 2

typedef struct t_pbnds {
  double t0;

  double a_hi[TB_NUM_DIMS];
  double v_hi[TB_NUM_DIMS];
  double x_hi[TB_NUM_DIMS];

  double a_lo[TB_NUM_DIMS];
  double v_lo[TB_NUM_DIMS];
  double x_lo[TB_NUM_DIMS];
} t_pbnds;


/* 
   Track trees are KD trees designed for astronomical tracks.  They
   contain information about a variety of factors (RA,DEC,T,I,etc.)
   and the relative importance of these factors during tree
   construction is controlled by a weight vector W.

   W[T_TIME]   = 100.0 ->  Treat time as 100x more important
   W[T_BRIGHT] = 0.0   ->  Ignore brightness

   The weights for velocities are automatically set to 0.0 when
   building the tree from simple_obs.
*/

/* --------------------------------------------------------------------- */
/* --- Tree Memory Functions ------------------------------------------- */
/* --------------------------------------------------------------------- */

dyv* mk_t_tree_weights(double time, double ra, double dec,
                       double vra, double vdec, double brightness);

t_tree* mk_empty_t_tree();

t_tree* mk_t_tree(track_array* arr, simple_obs_array* obs,
                  dyv* W, int max_leaf_pts);

t_tree* mk_t_tree_from_simple_obs(simple_obs_array* obs,
                                  dyv* W, int max_leaf_pts);

/* Build a tree by first splitting on time, then split based */
/* on a type of midpoint projection.                         */
t_tree* mk_t_tree_projections(track_array* arr, simple_obs_array* obs, int leaf_pts);

void free_t_tree(t_tree* old);

/* --------------------------------------------------------------------- */
/* --- Getter/Setter Functions ----------------------------------------- */
/* --------------------------------------------------------------------- */

int safe_t_num_points(t_tree* tr);

int safe_t_split_dim(t_tree* tr);

double safe_t_split_val(t_tree* tr);

bool safe_t_is_leaf(t_tree* tr);

ivec* safe_t_tracks(t_tree* tr);

t_tree* safe_t_right_child(t_tree* tr);

t_tree* safe_t_left_child(t_tree* tr);

double safe_t_hi_time(t_tree* tr);
double safe_t_lo_time(t_tree* tr);
double safe_t_mid_time(t_tree* tr);
double safe_t_rad_time(t_tree* tr);

double safe_t_hi_ra(t_tree* tr);
double safe_t_lo_ra(t_tree* tr);
double safe_t_mid_ra(t_tree* tr);
double safe_t_rad_ra(t_tree* tr);

double safe_t_hi_dec(t_tree* tr);
double safe_t_lo_dec(t_tree* tr);
double safe_t_mid_dec(t_tree* tr);
double safe_t_rad_dec(t_tree* tr);

double safe_t_hi_vra(t_tree* tr);
double safe_t_lo_vra(t_tree* tr);
double safe_t_mid_vra(t_tree* tr);
double safe_t_rad_vra(t_tree* tr);

double safe_t_hi_vdec(t_tree* tr);
double safe_t_lo_vdec(t_tree* tr);
double safe_t_mid_vdec(t_tree* tr);
double safe_t_rad_vdec(t_tree* tr);

double safe_t_hi_bright(t_tree* tr);
double safe_t_lo_bright(t_tree* tr);
double safe_t_mid_bright(t_tree* tr);
double safe_t_rad_bright(t_tree* tr);

double safe_t_hi_bound(t_tree* tr, int dim);
double safe_t_lo_bound(t_tree* tr, int dim);
double safe_t_mid_bound(t_tree* tr, int dim);
double safe_t_rad_bound(t_tree* tr, int dim);


/* Allow a few speedups */
#ifdef AMFAST

#define t_num_points(X)          (X->num_points)
#define t_split_dim(X)           (X->split_dim)
#define t_split_val(X)           (X->split_val)
#define t_is_leaf(X)             (X->split_dim == -1)
#define t_tracks(X)              (X->trcks)
#define t_right_child(X)         (X->right)
#define t_left_child(X)          (X->left)

#define t_hi_time(X)             (X->hi[T_TIME])
#define t_lo_time(X)             (X->lo[T_TIME])
#define t_mid_time(X)            (JK_MID(X->lo[T_TIME],X->hi[T_TIME]))
#define t_rad_time(X)            (JK_RAD(X->lo[T_TIME],X->hi[T_TIME]))
#define t_hi_ra(X)               (X->hi[T_R])
#define t_lo_ra(X)               (X->lo[T_R])
#define t_mid_ra(X)              (JK_MID(X->lo[T_R],X->hi[T_R]))
#define t_rad_ra(X)              (JK_RAD(X->lo[T_R],X->hi[T_R]))
#define t_hi_vra(X)              (X->hi[T_VR])
#define t_lo_vra(X)              (X->lo[T_VR])
#define t_mid_vra(X)             (JK_MID(X->lo[T_VR],X->hi[T_VR]))
#define t_rad_vra(X)             (JK_RAD(X->lo[T_VR],X->hi[T_VR]))
#define t_hi_dec(X)              (X->hi[T_D])
#define t_lo_dec(X)              (X->lo[T_D])
#define t_mid_dec(X)             (JK_MID(X->lo[T_D],X->hi[T_D]))
#define t_rad_dec(X)             (JK_RAD(X->lo[T_D],X->hi[T_D]))
#define t_hi_vdec(X)             (X->hi[T_VD])
#define t_lo_vdec(X)             (X->lo[T_VD])
#define t_mid_vdec(X)            (JK_MID(X->lo[T_VD],X->hi[T_VD]))
#define t_rad_vdec(X)            (JK_RAD(X->lo[T_VD],X->hi[T_VD]))
#define t_hi_bright(X)           (X->hi[T_BR])
#define t_lo_bright(X)           (X->lo[T_BR])
#define t_mid_bright(X)          (JK_MID(X->lo[T_BR],X->hi[T_BR]))
#define t_rad_bright(X)          (JK_RAD(X->lo[T_BR],X->hi[T_BR]))

#define t_hi_bound(X,i)          (X->hi[i])
#define t_lo_bound(X,i)          (X->lo[i])
#define t_mid_bound(X,i)         (JK_MID(X->lo[i],X->hi[i]))
#define t_rad_bound(X,i)         (JK_RAD(X->lo[i],X->hi[i]))

#else

#define t_num_points(X)          (safe_t_num_points(X))
#define t_split_dim(X)           (safe_t_split_dim(X))
#define t_split_val(X)           (safe_t_split_val(X))
#define t_is_leaf(X)             (safe_t_is_leaf(X))
#define t_tracks(X)              (safe_t_tracks(X))
#define t_right_child(X)         (safe_t_right_child(X))
#define t_left_child(X)          (safe_t_left_child(X))

#define t_hi_time(X)             (safe_t_hi_time(X))
#define t_lo_time(X)             (safe_t_lo_time(X))
#define t_mid_time(X)            (safe_t_mid_time(X))
#define t_rad_time(X)            (safe_t_rad_time(X))
#define t_hi_ra(X)               (safe_t_hi_ra(X))
#define t_lo_ra(X)               (safe_t_lo_ra(X))
#define t_mid_ra(X)              (safe_t_mid_ra(X))
#define t_rad_ra(X)              (safe_t_rad_ra(X))
#define t_hi_vra(X)              (safe_t_hi_vra(X))
#define t_lo_vra(X)              (safe_t_lo_vra(X))
#define t_mid_vra(X)             (safe_t_mid_vra(X))
#define t_rad_vra(X)             (safe_t_rad_vra(X))
#define t_hi_dec(X)              (safe_t_hi_dec(X))
#define t_lo_dec(X)              (safe_t_lo_dec(X))
#define t_mid_dec(X)             (safe_t_mid_dec(X))
#define t_rad_dec(X)             (safe_t_rad_dec(X))
#define t_hi_vdec(X)             (safe_t_hi_vdec(X))
#define t_lo_vdec(X)             (safe_t_lo_vdec(X))
#define t_mid_vdec(X)            (safe_t_mid_vdec(X))
#define t_rad_vdec(X)            (safe_t_rad_vdec(X))
#define t_hi_bright(X)           (safe_t_hi_bright(X))
#define t_lo_bright(X)           (safe_t_lo_bright(X))
#define t_mid_bright(X)          (safe_t_mid_bright(X))
#define t_rad_bright(X)          (safe_t_rad_bright(X))

#define t_hi_bound(X,i)          (safe_t_hi_bound(X,i))
#define t_lo_bound(X,i)          (safe_t_lo_bound(X,i))
#define t_mid_bound(X,i)         (safe_t_mid_bound(X,i))
#define t_rad_bound(X,i)         (safe_t_rad_bound(X,i))

#endif


double t_lo_vel_bound(t_tree* tr, int dim);

double t_hi_vel_bound(t_tree* tr, int dim);

/* --------------------------------------------------------------------- */
/* --- Simple Access Helper Functions ---------------------------------- */
/* --------------------------------------------------------------------- */

/* Gets a track's value associated with the given dimension. */
double t_tree_track_value(track* X, int dim);

/* Gets a track's velocity associated with the given dimension. */
double t_tree_track_velocity(track* X, int dim);

double t_tree_predict_value(track* X, int dim, double time);

/* Gets an observation's value associated with the given dimension. */
double t_tree_simple_obs_value(simple_obs* X, int dim);

ivec* mk_t_tree_tracks(t_tree* X);


/* --------------------------------------------------------------------- */
/* --- Simple I/O Functions -------------------------------------------- */
/* --------------------------------------------------------------------- */

void fprintf_t_tree_simple(FILE* f, char* pre, t_tree* tr, char* post);

void fprintf_t_tree_node(FILE* f, char* pre, t_tree* tr, char* post);



/* --------------------------------------------------------------------- */
/* --- Query Functions ------------------------------------------------- */
/* --------------------------------------------------------------------- */


/* 
  A note on t_tree query functions.  Because there are some many
  pieces of data, the query functions take a threshold dyv, which is
  T_NUM_DIMS thresholds that determine the range a point has to be in.
  If thresh(i) < 0.0 then that dimension is ignored.  DYV creater
  functions for some more common queries are given below.
*/

dyv* mk_t_tree_thresh(double time, double ra, double dec,
                      double vra, double vdec, double brightness);

dyv* mk_t_tree_RADEC_thresh(double thresh);

dyv* mk_t_tree_thresh_ignore_all();

dyv* mk_t_tree_query(double time, double ra, double dec,
                     double vra, double vdec, double brightness);

dyv* mk_t_tree_track_dyv(track* X);

dyv* mk_t_tree_accel(double time, double ra, double dec,
                     double vra, double vdec, double brightness);


/* --- Simple Range Search Queries ----------------------------------- */

ivec* mk_t_tree_near_point_slow(track_array* obs, ivec* inds, track* Q, dyv* thresh);

ivec* mk_t_tree_near_point(t_tree* tr, track_array* obs, track* Q, dyv* thresh);

/*
ivec* mk_t_tree_near_bounded_track_slow(track_array* obs, ivec* inds, track* Q,
                                        double t_start, double t_end, dyv* thresh);

ivec* mk_t_tree_near_bounded_track(t_tree* tr, track_array* obs, track* Q,
                                   double t_start, double t_end, dyv* thresh);
*/


/* --- Midpoint Range Search Queries --------------------------------- */

ivec* mk_t_tree_find_midpt_slow(track_array* arr, ivec* inds, track* Q,
                                double t_s, double t_e, dyv* thresh, dyv* accel);


ivec* mk_t_tree_find_midpt(t_tree* tr, track_array* arr, track* X,
                           double t_start, double t_end, dyv* thresh, dyv* accel);


/* --- Bounded Parameter Space Search -------------------------------- */

t_pbnds* mk_empty_t_pbnds(double t0);

t_pbnds* mk_copy_t_pbnds(t_pbnds* old);

void free_t_pbnds(t_pbnds* old);

double safe_t_pbnds_t0(t_pbnds* bnds);
double safe_t_pbnds_lo_a(t_pbnds* bnds, int dim);
double safe_t_pbnds_lo_v(t_pbnds* bnds, int dim);
double safe_t_pbnds_lo_x(t_pbnds* bnds, int dim);
double safe_t_pbnds_hi_a(t_pbnds* bnds, int dim);
double safe_t_pbnds_hi_v(t_pbnds* bnds, int dim);
double safe_t_pbnds_hi_x(t_pbnds* bnds, int dim);
double safe_t_pbnds_mid_a(t_pbnds* bnds, int dim);
double safe_t_pbnds_mid_v(t_pbnds* bnds, int dim);
double safe_t_pbnds_mid_x(t_pbnds* bnds, int dim);
double safe_t_pbnds_rad_a(t_pbnds* bnds, int dim);
double safe_t_pbnds_rad_v(t_pbnds* bnds, int dim);
double safe_t_pbnds_rad_x(t_pbnds* bnds, int dim);
double safe_t_pbnds_a_valid(t_pbnds* bnds, int dim);
double safe_t_pbnds_v_valid(t_pbnds* bnds, int dim);
double safe_t_pbnds_x_valid(t_pbnds* bnds, int dim);

#ifdef AMFAST

#define t_pbnds_t0(B)           (B->t0)
#define t_pbnds_lo_a(B,d)       (B->a_lo[d])
#define t_pbnds_lo_v(B,d)       (B->v_lo[d])
#define t_pbnds_lo_x(B,d)       (B->x_lo[d])
#define t_pbnds_hi_a(B,d)       (B->a_hi[d])
#define t_pbnds_hi_v(B,d)       (B->v_hi[d])
#define t_pbnds_hi_x(B,d)       (B->x_hi[d])
#define t_pbnds_mid_a(B,d)      ((B->a_hi[d] + B->a_lo[d])/2.0)
#define t_pbnds_mid_v(B,d)      ((B->v_hi[d] + B->v_lo[d])/2.0)
#define t_pbnds_mid_x(B,d)      ((B->x_hi[d] + B->x_lo[d])/2.0)
#define t_pbnds_rad_a(B,d)      ((B->a_hi[d] - B->a_lo[d])/2.0)
#define t_pbnds_rad_v(B,d)      ((B->v_hi[d] - B->v_lo[d])/2.0)
#define t_pbnds_rad_x(B,d)      ((B->x_hi[d] - B->x_lo[d])/2.0)
#define t_pbnds_a_valid(B,d)    (t_pbnds_rad_a(B,d) > -1e-20)
#define t_pbnds_v_valid(B,d)    (t_pbnds_rad_v(B,d) > -1e-20)
#define t_pbnds_x_valid(B,d)    (t_pbnds_rad_x(B,d) > -1e-20)

#else

#define t_pbnds_t0(B)           (safe_t_pbnds_t0(B))
#define t_pbnds_lo_a(B,d)       (safe_t_pbnds_lo_a(B,d))
#define t_pbnds_lo_v(B,d)       (safe_t_pbnds_lo_v(B,d))
#define t_pbnds_lo_x(B,d)       (safe_t_pbnds_lo_x(B,d))
#define t_pbnds_hi_a(B,d)       (safe_t_pbnds_hi_a(B,d))
#define t_pbnds_hi_v(B,d)       (safe_t_pbnds_hi_v(B,d))
#define t_pbnds_hi_x(B,d)       (safe_t_pbnds_hi_x(B,d))
#define t_pbnds_mid_a(B,d)      (safe_t_pbnds_mid_a(B,d))
#define t_pbnds_mid_v(B,d)      (safe_t_pbnds_mid_v(B,d))
#define t_pbnds_mid_x(B,d)      (safe_t_pbnds_mid_x(B,d))
#define t_pbnds_rad_a(B,d)      (safe_t_pbnds_rad_a(B,d))
#define t_pbnds_rad_v(B,d)      (safe_t_pbnds_rad_v(B,d))
#define t_pbnds_rad_x(B,d)      (safe_t_pbnds_rad_x(B,d))
#define t_pbnds_a_valid(B,d)    (safe_t_pbnds_a_valid(B,d))
#define t_pbnds_v_valid(B,d)    (safe_t_pbnds_v_valid(B,d))
#define t_pbnds_x_valid(B,d)    (safe_t_pbnds_x_valid(B,d))

#endif

double t_pbnds_track_x_ref(track* X, int dim);

double t_pbnds_track_v_ref(track* X, int dim);


#endif
