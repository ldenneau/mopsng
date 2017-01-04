/*
   File:        linker.h
   Author:      J. Kubica
   Description: Asteroid linkage and tracking functions.

   Copyright, The Auton Lab, CMU

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

#ifndef LINKER_H
#define LINKER_H

#include "track.h"

/* Information for the tracklet bounds (i.e. hi/lo values */
/* for the tracklet parameters given an error threshold). */
#define MTRACKLET_NTBP   9   /* Number of tracklet bound parameters */
#define TBP_T            0   /* Tracklet time                       */
#define TBP_R_L          1   /* Low bound of RA                     */
#define TBP_R_H          2   /* High bound of RA                    */
#define TBP_D_L          3   /* Low bound of DEC                    */
#define TBP_D_H          4   /* High bound of DEC                   */
#define TBP_VR_L         5   /* Low bound of vRA                    */
#define TBP_VR_H         6   /* High bound of vRA                   */
#define TBP_VD_L         7   /* Low bound of vDEC                   */
#define TBP_VD_H         8   /* High bound of vDEC                  */

/* Information for the tracklet bound tree  */
/* (i.e. a tree built on the above bounds). */
#define TBT_MAX_LEAF_NODES 10
#define TBT_DIM            5
#define TBT_T              0  /* Time            */
#define TBT_R              1  /* RA              */
#define TBT_D              2  /* DEC             */
#define TBT_VR             3  /* RA              */
#define TBT_VD             4  /* DEC             */

/* The minimum time between model tree nodes (1 day) */
#define TBT_MIN_TIME     1.0


typedef struct tbt {
  int num_points;

  struct tbt* left;
  struct tbt* right;

  /* Bounds... */
  double hi[TBT_DIM];
  double lo[TBT_DIM];
  
  ivec *pts;
} tbt;


typedef struct tbt_ptr_array {
  int size;
  int max_size;

  tbt** trs;
} tbt_ptr_array;


/* ---------------------------------------------------- */
/* --- Tracklet Preprocessing/Tree Functions ---------- */
/* ---------------------------------------------------- */

/* Creates a dym representation of the tracklets containing:           */
/* [t0, RA_lo, RA_hi, DEC_lo, DEC_hi, vRA_lo, vRA_hi, vDEC_lo, vDEC_hi] */
/* All of the result entries and the threshold are given in RADIANS    */
dym* mk_tracklet_bounds(simple_obs_array* obs, track_array* pairs, 
                        double thresh, double plate_width);

/* Creates a dym representation of the tracklets containing:           */
/* [t0, R_lo, R_hi, D_lo, D_hi, vR_lo, vR_hi, vD_lo, vD_hi] */
/* All of the result entries and the threshold are given in RADIANS    */
dym* mk_tracklet_bounds_no_vel(simple_obs_array* obs, track_array* pairs, double thresh);

/* --- Tree Memory Functions -------------------------- */

tbt* mk_empty_tbt(void);

/* use_inds - is the indices to use (NULL to use ALL observations). */
/* force_t  - forces us to split on time first.                     */
tbt* mk_tbt(dym* pts, ivec* use_inds, bool force_t, int max_leaf_pts);

void free_tbt(tbt* old);

/* --- Tree Getter/Setter Functions ------------------- */

int safe_tbt_N(tbt* tr);
int safe_tbt_num_points(tbt* tr);

bool safe_tbt_is_leaf(tbt* tr);
ivec* safe_tbt_pts(tbt* tr);

tbt* safe_tbt_right_child(tbt* tr);
tbt* safe_tbt_left_child(tbt* tr);

double safe_tbt_RA(tbt* p);
double safe_tbt_DEC(tbt* p);
double safe_tbt_time(tbt* p);

double safe_tbt_lo_time(tbt* p);
double safe_tbt_hi_time(tbt* p);
double safe_tbt_mid_time(tbt* p);
double safe_tbt_rad_time(tbt* p);

double safe_tbt_lo_RA(tbt* p);
double safe_tbt_hi_RA(tbt* p);
double safe_tbt_mid_RA(tbt* p);
double safe_tbt_rad_RA(tbt* p);

double safe_tbt_lo_DEC(tbt* p);
double safe_tbt_hi_DEC(tbt* p);
double safe_tbt_mid_DEC(tbt* p);
double safe_tbt_rad_DEC(tbt* p);

double safe_tbt_lo_vRA(tbt* p);
double safe_tbt_hi_vRA(tbt* p);
double safe_tbt_mid_vRA(tbt* p);
double safe_tbt_rad_vRA(tbt* p);

double safe_tbt_lo_vDEC(tbt* p);
double safe_tbt_hi_vDEC(tbt* p);
double safe_tbt_mid_vDEC(tbt* p);
double safe_tbt_rad_vDEC(tbt* p);

/* Allow a few speedups */
#ifdef AMFAST

#define tbt_N(X)               (X->num_points)
#define tbt_num_points(X)      (X->num_points)
#define tbt_is_leaf(X)         (X->left == NULL)
#define tbt_pts(X)             (X->pts)
#define tbt_right_child(X)     (X->right)
#define tbt_left_child(X)      (X->left)

#define tbt_RA(X)              ((X->hi[TBT_R] + X->lo[TBT_R])/2.0)
#define tbt_DEC(X)             ((X->hi[TBT_D] + X->lo[TBT_D])/2.0)
#define tbt_time(X)            ((X->hi[TBT_T] + X->lo[TBT_T])/2.0)

#define tbt_lo_time(X)         (X->lo[TBT_T])
#define tbt_hi_time(X)         (X->hi[TBT_T])
#define tbt_mid_time(X)        ((X->hi[TBT_T] + X->lo[TBT_T])/2.0)
#define tbt_rad_time(X)        ((X->hi[TBT_T] - X->lo[TBT_T])/2.0)

#define tbt_lo_RA(X)           (X->lo[TBT_R])
#define tbt_hi_RA(X)           (X->hi[TBT_R])
#define tbt_mid_RA(X)          ((X->hi[TBT_R] + X->lo[TBT_R])/2.0)
#define tbt_rad_RA(X)          ((X->hi[TBT_R] - X->lo[TBT_R])/2.0)

#define tbt_lo_DEC(X)          (X->lo[TBT_D])
#define tbt_hi_DEC(X)          (X->hi[TBT_D])
#define tbt_mid_DEC(X)         ((X->hi[TBT_D] + X->lo[TBT_D])/2.0)
#define tbt_rad_DEC(X)         ((X->hi[TBT_D] - X->lo[TBT_D])/2.0)

#define tbt_lo_vRA(X)          (X->lo[TBT_VR])
#define tbt_hi_vRA(X)          (X->hi[TBT_VR])
#define tbt_mid_vRA(X)         ((X->hi[TBT_VR] + X->lo[TBT_VR])/2.0)
#define tbt_rad_vRA(X)         ((X->hi[TBT_VR] - X->lo[TBT_VR])/2.0)

#define tbt_lo_vDEC(X)         (X->lo[TBT_VD])
#define tbt_hi_vDEC(X)         (X->hi[TBT_VD])
#define tbt_mid_vDEC(X)        ((X->hi[TBT_VD] + X->lo[TBT_VD])/2.0)
#define tbt_rad_vDEC(X)        ((X->hi[TBT_VD] - X->lo[TBT_VD])/2.0)

#else

#define tbt_N(X)               (safe_tbt_num_points(X))
#define tbt_num_points(X)      (safe_tbt_num_points(X))
#define tbt_is_leaf(X)         (safe_tbt_is_leaf(X))
#define tbt_pts(X)             (safe_tbt_pts(X))
#define tbt_right_child(X)     (safe_tbt_right_child(X))
#define tbt_left_child(X)      (safe_tbt_left_child(X))

#define tbt_RA(X)              (safe_tbt_RA(X))
#define tbt_DEC(X)             (safe_tbt_DEC(X))
#define tbt_time(X)            (safe_tbt_time(X))

#define tbt_lo_time(X)         (safe_tbt_lo_time(X))
#define tbt_hi_time(X)         (safe_tbt_hi_time(X))
#define tbt_mid_time(X)        (safe_tbt_mid_time(X))
#define tbt_rad_time(X)        (safe_tbt_rad_time(X))

#define tbt_lo_RA(X)           (safe_tbt_lo_RA(X))
#define tbt_hi_RA(X)           (safe_tbt_hi_RA(X))
#define tbt_mid_RA(X)          (safe_tbt_mid_RA(X))
#define tbt_rad_RA(X)          (safe_tbt_rad_RA(X))

#define tbt_lo_DEC(X)          (safe_tbt_lo_DEC(X))
#define tbt_hi_DEC(X)          (safe_tbt_hi_DEC(X))
#define tbt_mid_DEC(X)         (safe_tbt_mid_DEC(X))
#define tbt_rad_DEC(X)         (safe_tbt_rad_DEC(X))

#define tbt_lo_vRA(X)          (safe_tbt_lo_vRA(X))
#define tbt_hi_vRA(X)          (safe_tbt_hi_vRA(X))
#define tbt_mid_vRA(X)         (safe_tbt_mid_vRA(X))
#define tbt_rad_vRA(X)         (safe_tbt_rad_vRA(X))

#define tbt_lo_vDEC(X)         (safe_tbt_lo_vDEC(X))
#define tbt_hi_vDEC(X)         (safe_tbt_hi_vDEC(X))
#define tbt_mid_vDEC(X)        (safe_tbt_mid_vDEC(X))
#define tbt_rad_vDEC(X)        (safe_tbt_rad_vDEC(X))

#endif

/* Fills the pointer array with subtrees (of time width = 0) */
/* in ascending time order.  Requires force_t = TRUE         */
/* during tree construction.                                 */
void fill_plate_tbt_ptr_array(tbt* tr, tbt_ptr_array* arr);


/* -------------------------------------------------------------------- */
/* --- TBT Tree Pointer Array ----------------------------------------- */
/* -------------------------------------------------------------------- */

tbt_ptr_array* mk_sized_empty_tbt_ptr_array(int size);

tbt_ptr_array* mk_empty_tbt_ptr_array(void);

void free_tbt_ptr_array(tbt_ptr_array* old);

/* --- Access Functions ------------------------------------------- */

tbt* safe_tbt_ptr_array_ref(tbt_ptr_array* X, int index);

void tbt_ptr_array_set(tbt_ptr_array* X, int index, tbt* A);

void tbt_ptr_array_add(tbt_ptr_array* X, tbt* A);

int safe_tbt_ptr_array_size(tbt_ptr_array* X);

int safe_tbt_ptr_array_max_size(tbt_ptr_array* X);

/* Allow a few speedups */
#ifdef AMFAST

#define tbt_ptr_array_size(X)     (X->size)
#define tbt_ptr_array_max_size(X) (X->max_size)
#define tbt_ptr_array_ref(X,i)    (X->trs[i])

#else

#define tbt_ptr_array_size(X)     (safe_tbt_ptr_array_size(X))
#define tbt_ptr_array_max_size(X) (safe_tbt_ptr_array_max_size(X))
#define tbt_ptr_array_ref(X,i)    (safe_tbt_ptr_array_ref(X,i))

#endif


/* -------------------------------------------------------------------- */
/* --- Actual Search Functions ---------------------------------------- */
/* -------------------------------------------------------------------- */

track_array* mk_vtrees_tracks(simple_obs_array* obs, track_array* pairs,
                              double thresh, double acc_r, double acc_d,
                              int min_sup, int K, double fit_rd, double pred_fit,
                              bool endpts, double plate_width,
                              double last_start_obs_time, double first_end_obs_time);

/* A sequential search with accel only based pruning. */
/* For each starting track, find EACH possible ending */
/* track and search all of the tracks in between.     */
track_array* mk_sequential_accel_only_tracks(simple_obs_array* obs, track_array* pairs,
                                             double thresh, double acc_r, double acc_d,
                                             int min_sup, double fit_rd, double pred_fit,
                                             double plate_width,
                                             double last_start_obs_time,
                                             double first_end_obs_time);

#endif
