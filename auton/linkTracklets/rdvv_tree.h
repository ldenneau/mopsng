/*
   File:        rdvv_tree.h
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

#ifndef RDVV_TREE_H
#define RDVV_TREE_H

#include "track.h"

#define RDVV_MAX_LEAF_NODES 25
#define RDVV_T   0
#define RDVV_R   1
#define RDVV_D   2
#define RDVV_VR  3
#define RDVV_VD  4
#define RDVV_NUM_DIMS 5

#define RDVV_MAX_ACC  0.15

#define RDVV_MID(min,max)    (((min) + (max))/2.0)
#define RDVV_RAD(min,max)    ((max) - RDVV_MID(min,max))
#define RDVV_SIMPLE_MIN(a,b) (((a) < (b)) ? (a) : (b))
#define RDVV_SIMPLE_MAX(a,b) (((a) > (b)) ? (a) : (b))

typedef struct rdvv_tree {
  int num_points;
  int split_dim;
  double split_val;

  struct rdvv_tree* left;
  struct rdvv_tree* right;

  double hi[RDVV_NUM_DIMS];
  double lo[RDVV_NUM_DIMS];
  double mid[RDVV_NUM_DIMS];
  double rad[RDVV_NUM_DIMS];

  ivec *trcks;
} rdvv_tree;


/* --- Tree Memory Functions -------------------------- */

rdvv_tree* mk_empty_rdvv_tree();

/* If favor time is true then we are more likely to split on time... */
rdvv_tree* mk_rdvv_tree(track_array* arr, simple_obs_array* obs, int min_leaf_pts,
                        bool favor_time);

void free_rdvv_tree(rdvv_tree* old);


/* --- Getter/Setter Functions ----------------------------------------- */

int safe_rdvv_num_points(rdvv_tree* tr);

int safe_rdvv_split_dim(rdvv_tree* tr);

double safe_rdvv_split_val(rdvv_tree* tr);

bool safe_rdvv_is_leaf(rdvv_tree* tr);

ivec* safe_rdvv_tracks(rdvv_tree* tr);

rdvv_tree* safe_rdvv_right_child(rdvv_tree* tr);

rdvv_tree* safe_rdvv_left_child(rdvv_tree* tr);

double safe_rdvv_hi_time(rdvv_tree* tr);
double safe_rdvv_lo_time(rdvv_tree* tr);
double safe_rdvv_mid_time(rdvv_tree* tr);
double safe_rdvv_rad_time(rdvv_tree* tr);

double safe_rdvv_hi_ra(rdvv_tree* tr);
double safe_rdvv_lo_ra(rdvv_tree* tr);
double safe_rdvv_mid_ra(rdvv_tree* tr);
double safe_rdvv_rad_ra(rdvv_tree* tr);

double safe_rdvv_hi_dec(rdvv_tree* tr);
double safe_rdvv_lo_dec(rdvv_tree* tr);
double safe_rdvv_mid_dec(rdvv_tree* tr);
double safe_rdvv_rad_dec(rdvv_tree* tr);

double safe_rdvv_hi_vra(rdvv_tree* tr);
double safe_rdvv_lo_vra(rdvv_tree* tr);
double safe_rdvv_mid_vra(rdvv_tree* tr);
double safe_rdvv_rad_vra(rdvv_tree* tr);

double safe_rdvv_hi_vdec(rdvv_tree* tr);
double safe_rdvv_lo_vdec(rdvv_tree* tr);
double safe_rdvv_mid_vdec(rdvv_tree* tr);
double safe_rdvv_rad_vdec(rdvv_tree* tr);

double safe_rdvv_hi_bound(rdvv_tree* tr, int dim);
double safe_rdvv_lo_bound(rdvv_tree* tr, int dim);
double safe_rdvv_mid_bound(rdvv_tree* tr, int dim);
double safe_rdvv_rad_bound(rdvv_tree* tr, int dim);


/* Allow a few speedups */
#ifdef AMFAST

#define rdvv_num_points(X)                      (X->num_points)
#define rdvv_split_dim(X)                       (X->split_dim)
#define rdvv_split_val(X)                       (X->split_val)
#define rdvv_is_leaf(X)                         (X->split_dim == -1)
#define rdvv_tracks(X)                          (X->trcks)
#define rdvv_right_child(X)                     (X->right)
#define rdvv_left_child(X)                      (X->left)

#define rdvv_hi_time(X)                         (X->hi[RDVV_T])
#define rdvv_lo_time(X)                         (X->lo[RDVV_T])
#define rdvv_mid_time(X)                        (X->mid[RDVV_T])
#define rdvv_rad_time(X)                        (X->rad[RDVV_T])
#define rdvv_hi_ra(X)                           (X->hi[RDVV_R])
#define rdvv_lo_ra(X)                           (X->lo[RDVV_R])
#define rdvv_mid_ra(X)                          (X->mid[RDVV_R])
#define rdvv_rad_ra(X)                          (X->rad[RDVV_R])
#define rdvv_hi_dec(X)                          (X->hi[RDVV_D])
#define rdvv_lo_dec(X)                          (X->lo[RDVV_D])
#define rdvv_mid_dec(X)                         (X->mid[RDVV_D])
#define rdvv_rad_dec(X)                         (X->rad[RDVV_D])
#define rdvv_hi_vra(X)                          (X->hi[RDVV_VR])
#define rdvv_lo_vra(X)                          (X->lo[RDVV_VR])
#define rdvv_mid_vra(X)                         (X->mid[RDVV_VR])
#define rdvv_rad_vra(X)                         (X->rad[RDVV_VR])
#define rdvv_hi_vdec(X)                         (X->hi[RDVV_VD])
#define rdvv_lo_vdec(X)                         (X->lo[RDVV_VD])
#define rdvv_mid_vdec(X)                        (X->mid[RDVV_VD])
#define rdvv_rad_vdec(X)                        (X->rad[RDVV_VD])

#define rdvv_hi_bound(X,i)                      (X->hi[i])
#define rdvv_lo_bound(X,i)                      (X->lo[i])
#define rdvv_mid_bound(X,i)                     (X->mid[i])
#define rdvv_rad_bound(X,i)                     (X->rad[i])

#else

#define rdvv_num_points(X)    (safe_rdvv_num_points(X))
#define rdvv_split_dim(X)     (safe_rdvv_split_dim(X))
#define rdvv_split_val(X)     (safe_rdvv_split_val(X))
#define rdvv_is_leaf(X)       (safe_rdvv_is_leaf(X))
#define rdvv_tracks(X)        (safe_rdvv_tracks(X))
#define rdvv_right_child(X)   (safe_rdvv_right_child(X))
#define rdvv_left_child(X)    (safe_rdvv_left_child(X))

#define rdvv_hi_time(X)       (safe_rdvv_hi_time(X))
#define rdvv_lo_time(X)       (safe_rdvv_lo_time(X))
#define rdvv_mid_time(X)      (safe_rdvv_mid_time(X))
#define rdvv_rad_time(X)      (safe_rdvv_rad_time(X))
#define rdvv_hi_ra(X)         (safe_rdvv_hi_ra(X))
#define rdvv_lo_ra(X)         (safe_rdvv_lo_ra(X))
#define rdvv_mid_ra(X)        (safe_rdvv_mid_ra(X))
#define rdvv_rad_ra(X)        (safe_rdvv_rad_ra(X))
#define rdvv_hi_dec(X)        (safe_rdvv_hi_dec(X))
#define rdvv_lo_dec(X)        (safe_rdvv_lo_dec(X))
#define rdvv_mid_dec(X)       (safe_rdvv_mid_dec(X))
#define rdvv_rad_dec(X)       (safe_rdvv_rad_dec(X))
#define rdvv_hi_vra(X)				(safe_rdvv_hi_vra(X))
#define rdvv_lo_vra(X)        (safe_rdvv_lo_vra(X))
#define rdvv_mid_vra(X)       (safe_rdvv_mid_vra(X))
#define rdvv_rad_vra(X)       (safe_rdvv_rad_vra(X))
#define rdvv_hi_vdec(X)       (safe_rdvv_hi_vdec(X))
#define rdvv_lo_vdec(X)       (safe_rdvv_lo_vdec(X))
#define rdvv_mid_vdec(X)      (safe_rdvv_mid_vdec(X))
#define rdvv_rad_vdec(X)      (safe_rdvv_rad_vdec(X))

#define rdvv_hi_bound(X,i)    (safe_rdvv_hi_bound(X,i))
#define rdvv_lo_bound(X,i)    (safe_rdvv_lo_bound(X,i))
#define rdvv_mid_bound(X,i)   (safe_rdvv_mid_bound(X,i))
#define rdvv_rad_bound(X,i)   (safe_rdvv_rad_bound(X,i))

#endif


/* --- Simple I/O Functions -------------------------------------------- */

void fprintf_rdvv_tree_simple(FILE* f, char* pre, rdvv_tree* tr, char* post);

void fprintf_rdvv_tree_node(FILE* f, char* pre, rdvv_tree* tr, char* post);


/* --- Query Functions ------------------------------------------------- */


/* Find all tracks Y such that the midpoint_distance(X,Y) is <= thresh  */
/* and t_s <= Y.time <= t_e.  If inds == NULL look at all tracks.       */
ivec* mk_rdvv_find_midpt_slow(track_array* arr, ivec* inds, track* X,
                              double t_s, double t_e, double thresh);


ivec* mk_rdvv_find_midpt(rdvv_tree* tr, track_array* arr, track* X,
                         double t_start, double t_end, double thresh);

#endif
