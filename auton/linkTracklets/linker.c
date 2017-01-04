/*
   File:        linker.c
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

#include "linker.h"
#include "MHT.h"

extern int NUM_FOR_QUAD;

#define TBT_VWEIGHT 10.0

int leaf_count = 0;
int supp_count = 0;
int pairs_count = 0;


/* Skip flip tracks the number of iterations until */
/* we test for pruning (it is not alway helpful to */
/* test at each level of the search).              */
int skip_flip = 2;


/* ---------------------------------------------------- */
/* --- Tracklet Preprocessing/Tree Functions ---------- */
/* ---------------------------------------------------- */

/* Creates a dym representation of the tracklets containing:        */
/* [t0, R_lo, R_hi, D_lo, D_hi, vR_lo, vR_hi, vD_lo, vD_hi]         */ 
/* All of the result entries and the threshold are given in RADIANS */
dym* mk_tracklet_bounds(simple_obs_array* obs, track_array* pairs, 
                        double thresh, double plate_width) {
  dym* res;
  track*      A;
  simple_obs* X;
  simple_obs* Y;
  double t1, r1, d1;
  double t2, r2, d2;
  double dt, dr, dd;
  int N = track_array_size(pairs);
  int i;

  /* Don't use a zero plate width */
  if(plate_width < 1e-10) { plate_width = 1e-10; }

  res = mk_dym(N,MTRACKLET_NTBP);

  /* Go through EACH tracklet and compute its bounds. */
  for(i=0;i<N;i++) {
    A = track_array_ref(pairs,i);

    X  = track_first(A,obs);
    t1 = simple_obs_time(X);
    r1 = simple_obs_RA(X);
    d1 = simple_obs_DEC(X);

    if(track_num_obs(A) >= 2) {
      Y  = track_last(A,obs);
      t2 = simple_obs_time(Y);
      r2 = simple_obs_RA(Y);
      d2 = simple_obs_DEC(Y);

      /* Watch out for tracklets that have */
      /* been flattened away.              */
      if(t2 - t1 < 1e-10) {
        t2 = t1 + plate_width;
        r2 = r1 + track_vRA(A)  * plate_width;
        d2 = d1 + track_vDEC(A) * plate_width;
      }

    } else {
      t2 = t1 + 1.0;
      r2 = r1;
      d2 = d1;
    }

    /* Insert the "easy" stuff time, pos. bounds */
    dym_set(res,i,TBP_T,t1);
    dym_set(res,i,TBP_R_L,15.0*r1*DEG_TO_RAD-thresh);
    dym_set(res,i,TBP_R_H,15.0*r1*DEG_TO_RAD+thresh);
    dym_set(res,i,TBP_D_L,d1*DEG_TO_RAD-thresh);
    dym_set(res,i,TBP_D_H,d1*DEG_TO_RAD+thresh);

    /* Compute and insert the velocity bounds. */
    /* Make sure to handle wrap around in RA.  */
    dt = (t2-t1);
    dr = (r2-r1);
    dd = (d2-d1);
    if(dr < -12.0) { dr += 24.0; }
    if(dr >  12.0) { dr -= 24.0; }

    dym_set(res,i,TBP_VR_L,(15.0*dr*DEG_TO_RAD-2.0*thresh)/dt);
    dym_set(res,i,TBP_VR_H,(15.0*dr*DEG_TO_RAD+2.0*thresh)/dt);
    dym_set(res,i,TBP_VD_L,(dd*DEG_TO_RAD-2.0*thresh)/dt);
    dym_set(res,i,TBP_VD_H,(dd*DEG_TO_RAD+2.0*thresh)/dt);
  }

  return res;
}


/* Creates a dym representation of the tracklets containing:           */
/* [t0, R_lo, R_hi, D_lo, D_hi, vR_lo, vR_hi, vD_lo, vD_hi] */ 
/* All of the result entries and the threshold are given in RADIANS    */
dym* mk_tracklet_bounds_no_vel(simple_obs_array* obs, track_array* pairs, double thresh) {
  dym* res;
  track*      A;
  simple_obs* X;
  double t1, r1, d1;
  int N = track_array_size(pairs);
  int i;

  res = mk_zero_dym(N,MTRACKLET_NTBP);

  /* Go through EACH tracklet and compute its bounds. */
  for(i=0;i<N;i++) {
    A = track_array_ref(pairs,i);

    X  = track_first(A,obs);
    t1 = simple_obs_time(X);
    r1 = simple_obs_RA(X);
    d1 = simple_obs_DEC(X);

    /* Insert the "easy" stuff time, pos. bounds */
    dym_set(res,i,TBP_T,t1);
    dym_set(res,i,TBP_R_L,15.0*r1*DEG_TO_RAD-thresh);
    dym_set(res,i,TBP_R_H,15.0*r1*DEG_TO_RAD+thresh);
    dym_set(res,i,TBP_D_L,d1*DEG_TO_RAD-thresh);
    dym_set(res,i,TBP_D_H,d1*DEG_TO_RAD+thresh);
  }

  return res;
}


/* --- Tree Memory Functions -------------------------- */

void tbt_fill_bounds(tbt* tr, dym* pts, ivec* inds) {
  int i, ind;

  /* Set the initial values. */
  if(ivec_size(inds) > 0) {
    tr->lo[TBT_T]  = dym_ref(pts,ivec_ref(inds,0),TBP_T);
    tr->lo[TBT_R]  = dym_ref(pts,ivec_ref(inds,0),TBP_R_L);
    tr->lo[TBT_D]  = dym_ref(pts,ivec_ref(inds,0),TBP_D_L);
    tr->lo[TBT_VR] = dym_ref(pts,ivec_ref(inds,0),TBP_VR_L);
    tr->lo[TBT_VD] = dym_ref(pts,ivec_ref(inds,0),TBP_VD_L);

    tr->hi[TBT_T]  = dym_ref(pts,ivec_ref(inds,0),TBP_T);
    tr->hi[TBT_R]  = dym_ref(pts,ivec_ref(inds,0),TBP_R_H);
    tr->hi[TBT_D]  = dym_ref(pts,ivec_ref(inds,0),TBP_D_H);
    tr->hi[TBT_VR] = dym_ref(pts,ivec_ref(inds,0),TBP_VR_H);
    tr->hi[TBT_VD] = dym_ref(pts,ivec_ref(inds,0),TBP_VD_H);
  }

  /* Expand the bounds to include all of the points. */
  for(i=1;i<ivec_size(inds);i++) {
    ind = ivec_ref(inds,i);

    if(tr->lo[TBT_T] > dym_ref(pts,ind,TBP_T))    { tr->lo[TBT_T] = dym_ref(pts,ind,TBP_T);    }
    if(tr->lo[TBT_R] > dym_ref(pts,ind,TBP_R_L))  { tr->lo[TBT_R] = dym_ref(pts,ind,TBP_R_L);  }
    if(tr->lo[TBT_D] > dym_ref(pts,ind,TBP_D_L))  { tr->lo[TBT_D] = dym_ref(pts,ind,TBP_D_L);  }
    if(tr->lo[TBT_VR]> dym_ref(pts,ind,TBP_VR_L)) { tr->lo[TBT_VR]= dym_ref(pts,ind,TBP_VR_L); }
    if(tr->lo[TBT_VD]> dym_ref(pts,ind,TBP_VD_L)) { tr->lo[TBT_VD]= dym_ref(pts,ind,TBP_VD_L); }

    if(tr->hi[TBT_T] < dym_ref(pts,ind,TBP_T))    { tr->hi[TBT_T] = dym_ref(pts,ind,TBP_T);    }
    if(tr->hi[TBT_R] < dym_ref(pts,ind,TBP_R_H))  { tr->hi[TBT_R] = dym_ref(pts,ind,TBP_R_H);  }
    if(tr->hi[TBT_D] < dym_ref(pts,ind,TBP_D_H))  { tr->hi[TBT_D] = dym_ref(pts,ind,TBP_D_H);  }
    if(tr->hi[TBT_VR]< dym_ref(pts,ind,TBP_VR_H)) { tr->hi[TBT_VR]= dym_ref(pts,ind,TBP_VR_H); }
    if(tr->hi[TBT_VD]< dym_ref(pts,ind,TBP_VD_H)) { tr->hi[TBT_VD]= dym_ref(pts,ind,TBP_VD_H); }
  }

  if((tr->hi[TBT_R] > 22.0)&&(tr->lo[TBT_R] < 2.0)) {
    tr->hi[TBT_R] = 24.0;
    tr->lo[TBT_R] =  0.0;
  }
}


tbt* mk_empty_tbt() {
  tbt* res = AM_MALLOC(tbt);
  int i;

  res->num_points = 0;

  for(i=0;i<TBT_DIM;i++) {
    res->hi[i]  = 0.0;
    res->lo[i]  = 0.0;
  }

  res->right = NULL;
  res->left  = NULL;
  res->pts   = NULL;

  return res;
}


void free_tbt(tbt* old) {
  if(old->left) { free_tbt(old->left); }
  if(old->right) { free_tbt(old->right); }
  if(old->pts) { free_ivec(old->pts); }

  AM_FREE(old,tbt);
}


tbt* mk_tbt_recurse(dym* pts, ivec* inds,
                    dyv* widths, int max_leaf_pts) {
  tbt* res;
  ivec *left, *right;
  double val;
  double sv = 0.0;
  double sw = 0.0;
  int    sd = 0;
  int i, ind;

  /* Set up the node */
  res  = mk_empty_tbt();
  tbt_fill_bounds(res, pts, inds);
  res->num_points = ivec_size(inds);

  /* Determine if this node will be a leaf or internal */
  if(ivec_size(inds) <= max_leaf_pts) {
    res->pts = mk_copy_ivec(inds);
  } else {

    /* Pick the widest dimension and split it. */
    for(i=0;i<TBT_DIM;i++) {
      val = (res->hi[i] - res->lo[i]) / dyv_ref(widths,i);

      if((i==0)||(val > sw)) {
        sw = val;
        sd = i;
        sv = (res->hi[i] + res->lo[i]) / 2.0;
      }
    }

    /* Actually divide up the points. */
    left = mk_ivec(0);
    right = mk_ivec(0);

    for(i=0;i<ivec_size(inds);i++) {
      ind = ivec_ref(inds,i);
      val = 0.0;

      switch(sd) {
      case TBT_T:  val = dym_ref(pts,ind,TBP_T); break;
      case TBT_R:  val = (dym_ref(pts,ind,TBP_R_L)+dym_ref(pts,ind,TBP_R_H))/2.0; break;
      case TBT_D:  val = (dym_ref(pts,ind,TBP_D_L)+dym_ref(pts,ind,TBP_D_H))/2.0; break;
      case TBT_VR: val = (dym_ref(pts,ind,TBP_VR_L)+dym_ref(pts,ind,TBP_VR_H))/2.0; break;
      case TBT_VD: val = (dym_ref(pts,ind,TBP_VD_L)+dym_ref(pts,ind,TBP_VD_H))/2.0; break;
      }

      if(val < sv) {
        add_to_ivec(left,ivec_ref(inds,i));
      } else {
        add_to_ivec(right,ivec_ref(inds,i));
      }
    }

    /* Check if the two sides are zero...     */
    /* If so, just evenly split the points... */
    if((ivec_size(left)==0)||(ivec_size(right)==0)) {
      free_ivec(left);
      free_ivec(right);

      left = mk_ivec(0);
      right = mk_ivec(0);

      for(i=0;i<ivec_size(inds);i++) {
        ind = ivec_ref(inds,i);
        if(i % 2 == 0) {
          add_to_ivec(left,ivec_ref(inds,i));
        } else {
          add_to_ivec(right,ivec_ref(inds,i));
        }
      }
    }

    /* Build the left and right sub-trees */
    if((ivec_size(left)==0)||(ivec_size(right)==0)) {
      res->pts = mk_copy_ivec(inds);
    } else {
      res->left  = mk_tbt_recurse(pts, left,  widths, max_leaf_pts);
      res->right = mk_tbt_recurse(pts, right, widths, max_leaf_pts);
    }

    free_ivec(left);
    free_ivec(right);
  }

  return res;
}


/* use_inds - is the indices to use (NULL to use ALL observations). */
/* force_t  - forces us to split on time first.                     */
tbt* mk_tbt(dym* pts, ivec* use_inds, bool force_t, int max_leaf_pts) {
  tbt *res;
  ivec    *inds;
  dyv     *width;

  /* Store all the indices for the tree. */
  if(use_inds != NULL) {
    inds = mk_copy_ivec(use_inds);
  } else {
    inds = mk_sequence_ivec(0,dym_rows(pts));
  }

  /* Compute the bounds for the tree */
  res  = mk_empty_tbt();
  tbt_fill_bounds(res, pts, inds);
  width = mk_constant_dyv(TBT_DIM,1e-10);

  /* Set the width for time splitting. */
  if(tbt_rad_time(res) > dyv_ref(width,TBT_T)) {
    dyv_set(width,TBT_T,tbt_rad_time(res));
  }
  if(force_t) { dyv_set(width,TBT_T,1e-10); } 

  /* Set the width for RA/DEC */
  if(tbt_rad_RA(res) > dyv_ref(width,TBT_R)) { 
    dyv_set(width,TBT_R,tbt_rad_RA(res));
    dyv_set(width,TBT_D,tbt_rad_RA(res));
  }
  if(tbt_rad_DEC(res) > dyv_ref(width,TBT_R)) {
    dyv_set(width,TBT_R,tbt_rad_DEC(res));
    dyv_set(width,TBT_D,tbt_rad_DEC(res));
  }

  /* Set the width for vRA and vDEC */
  if(tbt_rad_vRA(res) > dyv_ref(width,TBT_VR)) { 
    dyv_set(width,TBT_VR,tbt_rad_vRA(res)*TBT_VWEIGHT);
    dyv_set(width,TBT_VD,tbt_rad_vRA(res)*TBT_VWEIGHT);
  }
  if(tbt_rad_vDEC(res) > dyv_ref(width,TBT_VR)) {
    dyv_set(width,TBT_VR,tbt_rad_vDEC(res)*TBT_VWEIGHT);
    dyv_set(width,TBT_VD,tbt_rad_vDEC(res)*TBT_VWEIGHT);
  }

  free_tbt(res);

  /* Build the tree. */
  res  = mk_tbt_recurse(pts, inds, width, max_leaf_pts);

  /* Free the used memory */
  free_dyv(width);
  free_ivec(inds);

  return res;
}




/* --- Tree Getter/Setter Functions ------------------- */

int safe_tbt_N(tbt* tr) { return tr->num_points; }
int safe_tbt_num_points(tbt* tr) { return tr->num_points; }

bool safe_tbt_is_leaf(tbt* tr) { return (tr->left == NULL); }
ivec* safe_tbt_pts(tbt* tr) { return tr->pts; }

tbt* safe_tbt_right_child(tbt* tr) { return tr->right; }
tbt* safe_tbt_left_child(tbt* tr) { return tr->left; }

double safe_tbt_RA(tbt* p) { return (p->hi[TBT_R] + p->lo[TBT_R])/2.0; }
double safe_tbt_DEC(tbt* p) { return (p->hi[TBT_D] + p->lo[TBT_D])/2.0; }
double safe_tbt_time(tbt* p) { return (p->hi[TBT_T] + p->lo[TBT_T])/2.0; }

double safe_tbt_lo_time(tbt* p) { return p->lo[TBT_T]; }
double safe_tbt_hi_time(tbt* p) { return p->hi[TBT_T]; }
double safe_tbt_mid_time(tbt* p) { return (p->hi[TBT_T] + p->lo[TBT_T])/2.0; }
double safe_tbt_rad_time(tbt* p) { return (p->hi[TBT_T] - p->lo[TBT_T])/2.0; }

double safe_tbt_lo_RA(tbt* p) { return p->lo[TBT_R]; }
double safe_tbt_hi_RA(tbt* p) { return p->hi[TBT_R]; }
double safe_tbt_mid_RA(tbt* p) { return (p->hi[TBT_R] + p->lo[TBT_R])/2.0; }
double safe_tbt_rad_RA(tbt* p) { return (p->hi[TBT_R] - p->lo[TBT_R])/2.0; }

double safe_tbt_lo_DEC(tbt* p) { return p->lo[TBT_D]; }
double safe_tbt_hi_DEC(tbt* p) { return p->hi[TBT_D]; }
double safe_tbt_mid_DEC(tbt* p) { return (p->hi[TBT_D] + p->lo[TBT_D])/2.0; }
double safe_tbt_rad_DEC(tbt* p) { return (p->hi[TBT_D] - p->lo[TBT_D])/2.0; }

double safe_tbt_lo_vRA(tbt* p) { return p->lo[TBT_VR]; }
double safe_tbt_hi_vRA(tbt* p) { return p->hi[TBT_VR]; }
double safe_tbt_mid_vRA(tbt* p) { return (p->hi[TBT_VR] + p->lo[TBT_VR])/2.0; }
double safe_tbt_rad_vRA(tbt* p) { return (p->hi[TBT_VR] - p->lo[TBT_VR])/2.0; }

double safe_tbt_lo_vDEC(tbt* p) { return p->lo[TBT_VD]; }
double safe_tbt_hi_vDEC(tbt* p) { return p->hi[TBT_VD]; }
double safe_tbt_mid_vDEC(tbt* p) { return (p->hi[TBT_VD] + p->lo[TBT_VD])/2.0; }
double safe_tbt_rad_vDEC(tbt* p) { return (p->hi[TBT_VD] - p->lo[TBT_VD])/2.0; }


/* Fills the pointer array with subtrees (of time width = 0) */
/* in ascending time order.  Requires force_t = TRUE         */
/* during tree construction.                                 */
void fill_plate_tbt_ptr_array(tbt* tr, tbt_ptr_array* arr) {
  if(tbt_is_leaf(tr) || (tbt_rad_time(tr) < 1e-10)) {
    tbt_ptr_array_add(arr,tr);
  } else {
    fill_plate_tbt_ptr_array(tbt_left_child(tr),arr);
    fill_plate_tbt_ptr_array(tbt_right_child(tr),arr);
  }
}


/* -------------------------------------------------------------------- */
/* --- TBT Tree Pointer Array ----------------------------------------- */
/* -------------------------------------------------------------------- */

tbt_ptr_array* mk_sized_empty_tbt_ptr_array(int size) {
  tbt_ptr_array* res = AM_MALLOC(tbt_ptr_array);
  int i;

  res->size     = 0;
  res->max_size = size;

  res->trs = AM_MALLOC_ARRAY(tbt*,size);
  for(i=0;i<size;i++) {
    res->trs[i] = NULL;
  }

  return res;
}


tbt_ptr_array* mk_empty_tbt_ptr_array() {
  return mk_sized_empty_tbt_ptr_array(10);
}


void free_tbt_ptr_array(tbt_ptr_array* old) {
  AM_FREE_ARRAY(old->trs,tbt*,old->max_size);
  AM_FREE(old,tbt_ptr_array);
}


void tbt_ptr_array_double_size(tbt_ptr_array* old) {
  tbt** nu_arr;
  int i;

  nu_arr = AM_MALLOC_ARRAY(tbt*,2*old->max_size+1);
  for(i=0;i<old->size;i++) {
    nu_arr[i] = old->trs[i];
  }
  for(i=old->size;i<2*old->max_size+1;i++) {
    nu_arr[i] = NULL;
  }

  AM_FREE_ARRAY(old->trs,tbt*,old->max_size);

  old->trs      = nu_arr;
  old->max_size = 2*old->max_size+1;
}


/* --- Access Functions ------------------------------------------- */

tbt* safe_tbt_ptr_array_ref(tbt_ptr_array* X, int index) {
  my_assert((X->max_size > index)&&(index >= 0));
  return X->trs[index];
}

void tbt_ptr_array_set(tbt_ptr_array* X, int index, tbt* A) {
  my_assert(index >= 0);

  while(index >= X->max_size) { tbt_ptr_array_double_size(X); }

  X->trs[index] = A;
  if(index >= X->size) { X->size = index+1; }
}

void tbt_ptr_array_add(tbt_ptr_array* X, tbt* A) {

  /* Double the size of the array (if needed) */
  if(X->size >= X->max_size) { tbt_ptr_array_double_size(X); }

  /* Actually set the array value */
  X->trs[X->size] = A;
  X->size += 1;
}

int safe_tbt_ptr_array_size(tbt_ptr_array* X) {
  return X->size;
}

int safe_tbt_ptr_array_max_size(tbt_ptr_array* X) {
  return X->max_size;
}



/* ---------------------------------------------------- */
/* --- Actual Tree Search Function (accel prune) ------ */
/* ---------------------------------------------------- */


/* REFINE the acceleration bounds (means bounds MUST */
/* have well define initial values.  Return TRUE iff */
/* the set of model trees is mutually consistent.    */
/* Note that the trees must be in increasing time    */
/* order.                                            */
bool quad_vtree_pairs_determine_abounds(tbt_ptr_array* mdl_pts,
                                        double* aR_min, double* aR_max,
                                        double* aD_min, double* aD_max) {
  tbt* A;
  tbt* B;
  double dt_min, dt_max, dt;
  double val2, val3;
  double acc, temp;
  double dr;
  int M = tbt_ptr_array_size(mdl_pts);
  int i, j;
  bool valid = TRUE;

  /* Check consistency in a pairwise fashion. */
  for(i=0;(i<M-1)&&(valid);i++) {
    for(j=i+1;(j<M)&&(valid);j++) {
      A = tbt_ptr_array_ref(mdl_pts,i);
      B = tbt_ptr_array_ref(mdl_pts,j);

      dt_min = tbt_lo_time(B) - tbt_hi_time(A);
      dt_max = tbt_hi_time(B) - tbt_lo_time(A);
      if(dt_min < TBT_MIN_TIME) { dt_min = TBT_MIN_TIME; }
      if(dt_max < dt_min) { valid = FALSE; }

      /* Do the velocity/velocity tests. */
      if(valid) {

        /* Determine the max accel in RA with both velocity bounds. */
        temp = tbt_hi_vRA(B) - tbt_lo_vRA(A);
        if(temp < 0.0) { acc = temp/dt_max; } else { acc = temp/dt_min; }
        if(aR_max[0] > acc) { aR_max[0] = acc; }

        /* Determine the min accel in RA with both velocity bounds. */
        temp = tbt_lo_vRA(B) - tbt_hi_vRA(A);
        if(temp < 0.0) { acc = temp/dt_min; } else { acc = temp/dt_max; }
        if(aR_min[0] < acc) { aR_min[0] = acc; }

        /* Determine the max accel in DEC with both velocity bounds. */
        temp = tbt_hi_vDEC(B) - tbt_lo_vDEC(A);
        if(temp < 0.0) { acc = temp/dt_max; } else { acc = temp/dt_min; }
        if(aD_max[0] > acc) { aD_max[0] = acc; }

        /* Determine the min accel in RA with both velocity bounds. */
        temp = tbt_lo_vDEC(B) - tbt_hi_vDEC(A);
        if(temp < 0.0) { acc = temp/dt_min; } else { acc = temp/dt_max; }
        if(aD_min[0] < acc) { aD_min[0] = acc; }

        valid = (aD_min[0] <= aD_max[0])&&(aR_min[0] <= aR_max[0]);
      }
  
      /* Do the velocity+position/position tests. */
      if(valid) {

        /* Find the maximum acc in RA */
        dr = tbt_hi_RA(B)-tbt_lo_RA(A);
        if(fabs(tbt_lo_vRA(A)) > 1e-20) {
          dt = 2.0*dr/tbt_lo_vRA(A);
          if(fabs(dt) < 1e-10) { dt = 1e-10; }
          if(dt < dt_min) { dt = dt_min; }
          if(dt > dt_max) { dt = dt_max; }
        } else {
          dt = dt_min;
        }
        acc  = 2.0*(dr-tbt_lo_vRA(A)*dt_min)/(dt_min*dt_min);
        val2 = 2.0*(dr-tbt_lo_vRA(A)*dt_max)/(dt_max*dt_max);
        val3 = 2.0*(dr-tbt_lo_vRA(A)*dt)/(dt*dt);
        if(val2 > acc) { acc = val2; };
        if(val3 > acc) { acc = val3; };
        if(aR_max[0] > acc) { aR_max[0] = acc; }

        /* Find the minimum acc in RA */
        dr = tbt_lo_RA(B)-tbt_hi_RA(A);
        if(fabs(tbt_hi_vRA(A)) > 1e-20) {
          dt = 2.0*dr/tbt_hi_vRA(A);
          if(fabs(dt) < 1e-10) { dt = 1e-10; }
          if(dt < dt_min) { dt = dt_min; }
          if(dt > dt_max) { dt = dt_max; }
        } else {
          dt = dt_min;
        }
        acc  = 2.0*(dr-tbt_hi_vRA(A)*dt_min)/(dt_min*dt_min);
        val2 = 2.0*(dr-tbt_hi_vRA(A)*dt_max)/(dt_max*dt_max);
        val3 = 2.0*(dr-tbt_hi_vRA(A)*dt)/(dt*dt);
        if(val2 < acc) { acc = val2; };
        if(val3 < acc) { acc = val3; };
        if(aR_min[0] < acc) { aR_min[0] = acc; }

        /* Find the maximum acc in DEC */
        if(fabs(tbt_lo_vDEC(A)) > 1e-20) { 
          dt = 2.0*(tbt_hi_DEC(B)-tbt_lo_DEC(A))/tbt_lo_vDEC(A);
          if(fabs(dt) < 1e-10) { dt = 1e-10; }
          if(dt < dt_min) { dt = dt_min; }
          if(dt > dt_max) { dt = dt_max; }
        } else {
          dt = dt_min;
        }
        acc  = 2.0*(tbt_hi_DEC(B)-tbt_lo_DEC(A)-tbt_lo_vDEC(A)*dt_min)/(dt_min*dt_min);
        val2 = 2.0*(tbt_hi_DEC(B)-tbt_lo_DEC(A)-tbt_lo_vDEC(A)*dt_max)/(dt_max*dt_max);
        val3 = 2.0*(tbt_hi_DEC(B)-tbt_lo_DEC(A)-tbt_lo_vDEC(A)*dt)/(dt*dt);
        if(val2 > acc) { acc = val2; };
        if(val3 > acc) { acc = val3; };
        if(aD_max[0] > acc) { aD_max[0] = acc; }        

        /* Find the minimum acc in DEC */
        if(fabs(tbt_hi_vDEC(A)) > 1e-20) { 
          dt = 2.0*(tbt_lo_DEC(B)-tbt_hi_DEC(A))/tbt_hi_vDEC(A);
          if(fabs(dt) < 1e-10) { dt = 1e-10; }
          if(dt < dt_min) { dt = dt_min; }
          if(dt > dt_max) { dt = dt_max; }
        } else {
          dt = dt_min;
        }
        acc  = 2.0*(tbt_lo_DEC(B)-tbt_hi_DEC(A)-tbt_hi_vDEC(A)*dt_min)/(dt_min*dt_min);
        val2 = 2.0*(tbt_lo_DEC(B)-tbt_hi_DEC(A)-tbt_hi_vDEC(A)*dt_max)/(dt_max*dt_max);
        val3 = 2.0*(tbt_lo_DEC(B)-tbt_hi_DEC(A)-tbt_hi_vDEC(A)*dt)/(dt*dt);
        if(val2 < acc) { acc = val2; };
        if(val3 < acc) { acc = val3; };
        if(aD_min[0] < acc) { aD_min[0] = acc; }        

        valid = (aD_min[0] <= aD_max[0])&&(aR_min[0] <= aR_max[0]);
      }

      /* Do the velocity+position/position tests. */
      if(valid) {
        
        /* Find the maximum acc in RA */
        dr = tbt_hi_RA(A)-tbt_lo_RA(B);
        if(fabs(tbt_hi_vRA(B)) > 1e-20) {
          dt = 2.0*dr/(-tbt_hi_vRA(B));
          if(fabs(dt) < 1e-10) { dt = 1e-10; }
          if(dt < dt_min) { dt = dt_min; }
          if(dt > dt_max) { dt = dt_max; }
        } else {
          dt = dt_min;
        }
        acc  = 2.0*(dr+tbt_hi_vRA(B)*dt_min)/(dt_min*dt_min);
        val2 = 2.0*(dr+tbt_hi_vRA(B)*dt_max)/(dt_max*dt_max);
        val3 = 2.0*(dr+tbt_hi_vRA(B)*dt)/(dt*dt);
        if(val2 > acc) { acc = val2; };
        if(val3 > acc) { acc = val3; };
        if(aR_max[0] > acc) { aR_max[0] = acc; }

        /* Find the minimum acc in RA */
        dr = tbt_lo_RA(A)-tbt_hi_RA(B);
        if(fabs(tbt_lo_vRA(B)) > 1e-20) {
          dt = 2.0*dr/(-tbt_lo_vRA(B));
          if(fabs(dt) < 1e-10) { dt = 1e-10; }
          if(dt < dt_min) { dt = dt_min; }
          if(dt > dt_max) { dt = dt_max; }
        } else {
          dt = dt_min;
        }
        acc  = 2.0*(dr+tbt_lo_vRA(B)*dt_min)/(dt_min*dt_min);
        val2 = 2.0*(dr+tbt_lo_vRA(B)*dt_max)/(dt_max*dt_max);
        val3 = 2.0*(dr+tbt_lo_vRA(B)*dt)/(dt*dt);
        if(val2 < acc) { acc = val2; };
        if(val3 < acc) { acc = val3; };
        if(aR_min[0] < acc) { aR_min[0] = acc; }

        /* Find the maximum acc in DEC */
        if(fabs(tbt_hi_vDEC(B)) > 1e-20) {
          dt = 2.0*(tbt_hi_DEC(A)-tbt_lo_DEC(B))/(-tbt_hi_vDEC(B));
          if(fabs(dt) < 1e-10) { dt = 1e-10; }
          if(dt < dt_min) { dt = dt_min; }
          if(dt > dt_max) { dt = dt_max; }
        } else {
          dt = dt_min;
        }
        acc  = 2.0*(tbt_hi_DEC(A)-tbt_lo_DEC(B)+tbt_hi_vDEC(B)*dt_min)/(dt_min*dt_min);
        val2 = 2.0*(tbt_hi_DEC(A)-tbt_lo_DEC(B)+tbt_hi_vDEC(B)*dt_max)/(dt_max*dt_max);
        val3 = 2.0*(tbt_hi_DEC(A)-tbt_lo_DEC(B)+tbt_hi_vDEC(B)*dt)/(dt*dt);
        if(val2 > acc) { acc = val2; };
        if(val3 > acc) { acc = val3; };
        if(aD_max[0] > acc) { aD_max[0] = acc; }

        /* Find the minimum acc in DEC */
        if(fabs(tbt_lo_vDEC(B)) > 1e-20) {
          dt = 2.0*(tbt_lo_DEC(A)-tbt_hi_DEC(B))/(-tbt_lo_vDEC(B));
          if(fabs(dt) < 1e-10) { dt = 1e-10; }
          if(dt < dt_min) { dt = dt_min; }
          if(dt > dt_max) { dt = dt_max; }
        } else {
          dt = dt_min;
        }
        acc  = 2.0*(tbt_lo_DEC(A)-tbt_hi_DEC(B)+tbt_lo_vDEC(B)*dt_min)/(dt_min*dt_min);
        val2 = 2.0*(tbt_lo_DEC(A)-tbt_hi_DEC(B)+tbt_lo_vDEC(B)*dt_max)/(dt_max*dt_max);
        val3 = 2.0*(tbt_lo_DEC(A)-tbt_hi_DEC(B)+tbt_lo_vDEC(B)*dt)/(dt*dt);
        if(val2 < acc) { acc = val2; };
        if(val3 < acc) { acc = val3; };
        if(aD_min[0] < acc) { aD_min[0] = acc; }

        valid = (aD_min[0] <= aD_max[0])&&(aR_min[0] <= aR_max[0]);
      }

    }
  }

  return valid;
}


/* REFINE the acceleration bounds (means bounds MUST */
/* have well define initial values.  Return TRUE iff */
/* the set of model trees is mutually consistent.    */
/* Note that the trees must be in increasing time    */
/* order.                                            */
bool quad_vtree_pairs_determine_abounds_flat(tbt_ptr_array* mdl_pts,
                                             double* aR_min, double* aR_max,
                                             double* aD_min, double* aD_max) {
  tbt *A, *B;
  double dt, dt2, dti, acc;
  int M = tbt_ptr_array_size(mdl_pts);
  int i, j;
  bool valid = TRUE;

  /* Check consistency in a pairwise fashion. */
  for(i=0;(i<M-1)&&(valid);i++) {
    for(j=i+1;(j<M)&&(valid);j++) {
      A   = tbt_ptr_array_ref(mdl_pts,i);
      B   = tbt_ptr_array_ref(mdl_pts,j);
      dt  = tbt_lo_time(B) - tbt_hi_time(A);
      dt2 = 2.0 / (dt * dt);
      dti = 1.0/dt;

      /* Determine the max accel in RA with both velocity bounds. */
      acc = (tbt_hi_vRA(B) - tbt_lo_vRA(A))*dti;
      if(aR_max[0] > acc) { aR_max[0] = acc; }

      /* Determine the min accel in RA with both velocity bounds. */
      acc = (tbt_lo_vRA(B) - tbt_hi_vRA(A))*dti;
      if(aR_min[0] < acc) { aR_min[0] = acc; }

      /* Determine the max accel in DEC with both velocity bounds. */
      acc = (tbt_hi_vDEC(B) - tbt_lo_vDEC(A))*dti;
      if(aD_max[0] > acc) { aD_max[0] = acc; }

      /* Determine the min accel in RA with both velocity bounds. */
      acc = (tbt_lo_vDEC(B) - tbt_hi_vDEC(A))*dti;
      if(aD_min[0] < acc) { aD_min[0] = acc; }

      valid = (aD_min[0] <= aD_max[0])&&(aR_min[0] <= aR_max[0]);

      /* Do the velocity+position/position tests. */
      if(valid) {

        /* Find the maximum acc in RA */
        acc  = dt2*((tbt_hi_RA(B)-tbt_lo_RA(A))-tbt_lo_vRA(A)*dt);
        if(aR_max[0] > acc) { aR_max[0] = acc; }        

        /* Find the minimum acc in RA */
        acc  = dt2*((tbt_lo_RA(B)-tbt_hi_RA(A))-tbt_hi_vRA(A)*dt);
        if(aR_min[0] < acc) { aR_min[0] = acc; }        

        /* Find the maximum acc in DEC */
        acc  = dt2*((tbt_hi_DEC(B)-tbt_lo_DEC(A))-tbt_lo_vDEC(A)*dt);
        if(aD_max[0] > acc) { aD_max[0] = acc; }        

        /* Find the minimum acc in DEC */
        acc  = dt2*((tbt_lo_DEC(B)-tbt_hi_DEC(A))-tbt_hi_vDEC(A)*dt);
        if(aD_min[0] < acc) { aD_min[0] = acc; }        

        valid = (aD_min[0] <= aD_max[0])&&(aR_min[0] <= aR_max[0]);

        /* Do the velocity+position/position tests. */
        if(valid) {
        
          /* Find the maximum acc in RA */
          acc = dt2*(tbt_hi_RA(A)-tbt_lo_RA(B)+tbt_hi_vRA(B)*dt);
          if(aR_max[0] > acc) { aR_max[0] = acc; }

          /* Find the minimum acc in RA */
          acc = dt2*(tbt_lo_RA(A)-tbt_hi_RA(B)+tbt_lo_vRA(B)*dt);
          if(aR_min[0] < acc) { aR_min[0] = acc; }

          /* Find the maximum acc in DEC */
          acc = dt2*(tbt_hi_DEC(A)-tbt_lo_DEC(B)+tbt_hi_vDEC(B)*dt);
          if(aD_max[0] > acc) { aD_max[0] = acc; }

          /* Find the minimum acc in DEC */
          acc = dt2*(tbt_lo_DEC(A)-tbt_hi_DEC(B)+tbt_lo_vDEC(B)*dt);
          if(aD_min[0] < acc) { aD_min[0] = acc; }

          valid = (aD_min[0] <= aD_max[0])&&(aR_min[0] <= aR_max[0]);
        }
      }

    }
  }

  return valid;
}


/* REFINE the acceleration bounds (means bounds MUST */
/* have well define initial values.  Return TRUE iff */
/* the set of model trees is mutually consistent.    */
/* Note that the trees must be in increasing time    */
/* order.                                            */
bool quad_vtree_pair_determine_abounds_flat(tbt* A, tbt* B, double* aR_min, double* aR_max,
                                            double* aD_min, double* aD_max) {
  tbt *temp;
  double dt, acc, dt2, dti;
  bool valid = TRUE;

  if(tbt_time(B) < tbt_time(A)) {  
    temp = A;
    A    = B;
    B    = temp;
  }
  dt  = tbt_lo_time(B) - tbt_hi_time(A);
  dt2 = 2.0 / (dt * dt);
  dti = 1.0 / dt;
 
  /* Determine the max accel in RA with both velocity bounds. */
  acc = (tbt_hi_vRA(B) - tbt_lo_vRA(A)) * dti;
  if(aR_max[0] > acc) { aR_max[0] = acc; }

  /* Determine the min accel in RA with both velocity bounds. */
  acc = (tbt_lo_vRA(B) - tbt_hi_vRA(A)) * dti;
  if(aR_min[0] < acc) { aR_min[0] = acc; }

  /* Determine the max accel in DEC with both velocity bounds. */
  acc = (tbt_hi_vDEC(B) - tbt_lo_vDEC(A)) * dti;
  if(aD_max[0] > acc) { aD_max[0] = acc; }

  /* Determine the min accel in RA with both velocity bounds. */
  acc = (tbt_lo_vDEC(B) - tbt_hi_vDEC(A)) * dti;
  if(aD_min[0] < acc) { aD_min[0] = acc; }

  valid = (aD_min[0] <= aD_max[0])&&(aR_min[0] <= aR_max[0]);

  /* Do the velocity+position/position tests. */
  if(valid) {

    /* Find the maximum acc in RA */
    acc  = dt2*((tbt_hi_RA(B)-tbt_lo_RA(A))-tbt_lo_vRA(A)*dt);
    if(aR_max[0] > acc) { aR_max[0] = acc; }

    /* Find the minimum acc in RA */
    acc  = dt2*((tbt_lo_RA(B)-tbt_hi_RA(A))-tbt_hi_vRA(A)*dt);
    if(aR_min[0] < acc) { aR_min[0] = acc; }

    /* Find the maximum acc in DEC */
    acc  = dt2*((tbt_hi_DEC(B)-tbt_lo_DEC(A))-tbt_lo_vDEC(A)*dt);
    if(aD_max[0] > acc) { aD_max[0] = acc; }

    /* Find the minimum acc in DEC */
    acc  = dt2*((tbt_lo_DEC(B)-tbt_hi_DEC(A))-tbt_hi_vDEC(A)*dt);
    if(aD_min[0] < acc) { aD_min[0] = acc; }

    valid = (aD_min[0] <= aD_max[0])&&(aR_min[0] <= aR_max[0]);

    /* Do the velocity+position/position tests. */
    if(valid) {
      /* Find the maximum acc in RA */
      acc = dt2*(tbt_hi_RA(A)-tbt_lo_RA(B)+tbt_hi_vRA(B)*dt);
      if(aR_max[0] > acc) { aR_max[0] = acc; }

      /* Find the minimum acc in RA */
      acc = dt2*(tbt_lo_RA(A)-tbt_hi_RA(B)+tbt_lo_vRA(B)*dt);
      if(aR_min[0] < acc) { aR_min[0] = acc; }

      /* Find the maximum acc in DEC */
      acc = dt2*(tbt_hi_DEC(A)-tbt_lo_DEC(B)+tbt_hi_vDEC(B)*dt);
      if(aD_max[0] > acc) { aD_max[0] = acc; }

      /* Find the minimum acc in DEC */
      acc = dt2*(tbt_lo_DEC(A)-tbt_hi_DEC(B)+tbt_lo_vDEC(B)*dt);
      if(aD_min[0] < acc) { aD_min[0] = acc; }

      valid = (aD_min[0] <= aD_max[0])&&(aR_min[0] <= aR_max[0]);
    }
  }

  return valid;
}


/* Returns true iff the model tree is compatible with the support tree. */
/* Assumes the model tree and support tree do NOT overlap.              */
bool check_model_support_compat(tbt* mdl, tbt* sup,  double aR_min, double aR_max, 
                                double aD_min, double aD_max) {
  tbt *F, *L;
  double pHI, pLO;
  double dt;
  bool valid = TRUE;

  /* Put the two trees in time order. */
  if(tbt_time(mdl) < tbt_time(sup)) {
    F = mdl;
    L = sup;
  } else {
    F = sup;
    L = mdl;
  }
  dt = tbt_hi_time(L) - tbt_lo_time(F);
  
  /* Can we get from F to L in DEC? */
  pLO = tbt_lo_DEC(F) + (tbt_lo_vDEC(F) + 0.5*aD_min*dt)*dt;
  pHI = tbt_hi_DEC(F) + (tbt_hi_vDEC(F) + 0.5*aD_max*dt)*dt;
  valid = (pHI >= tbt_lo_DEC(L))&&(pLO <= tbt_hi_DEC(L));

  /* Can we get from F to L in RA? */
  if(valid) {
    pLO = tbt_lo_RA(F) + (tbt_lo_vRA(F) + 0.5*aR_min*dt)*dt;
    pHI = tbt_hi_RA(F) + (tbt_hi_vRA(F) + 0.5*aR_max*dt)*dt;
    valid = (pHI >= tbt_lo_RA(L))&&(pLO <= tbt_hi_RA(L));

    /* Can we get from F to L in vRA? */
    if(valid) {
      pLO = tbt_lo_vRA(F) + aR_min*dt;
      pHI = tbt_hi_vRA(F) + aR_max*dt;
      valid = (pHI >= tbt_lo_vRA(L))&&(pLO <= tbt_hi_vRA(L));

      /* Can we get from F to L in vDEC? */
      if(valid) {
        pLO = tbt_lo_vDEC(F) + aD_min*dt;
        pHI = tbt_hi_vDEC(F) + aD_max*dt;
        valid = (pHI >= tbt_lo_vDEC(L))&&(pLO <= tbt_hi_vDEC(L));
      }
    }
  }

  return valid;
}


/* This is ONLY called after we have hit a set of compatible model leaf */
/* nodes and only called with a set of valid support nodes.             */
void quad_vtree_pairs_leaf_check(simple_obs_array* obs, track_array* pairs,
                                 tbt_ptr_array* mdl_pts, tbt_ptr_array* sup_pts,
                                 int min_sup, double fit_rd, double pred_fit,
                                 double last_start_obs_time, double first_end_obs_time,
                                 track_array* res) {
  simple_obs* X;
  double rp, dp, tp;
  double dist, t;
  track* T;
  track* base   = NULL;
  dyv*  scores  = mk_dyv(0);
  dyv*  times   = NULL;
  dyv*  stimes  = NULL;
  ivec* mtchs   = mk_ivec(0);
  ivec* inds    = NULL;
  ivec* sorted;
  ivec* temp;
  int M = tbt_ptr_array_size(mdl_pts);
  int S = tbt_ptr_array_size(sup_pts);
  bool overlap;
  int i, j, ind;
  int pos_sup = 0;
  int count = 0;

  pairs_count++;

  /* Check the time bounds. */
  if((last_start_obs_time < tbt_lo_time(tbt_ptr_array_ref(mdl_pts,0))) ||
     (first_end_obs_time > tbt_hi_time(tbt_ptr_array_ref(mdl_pts,M-1)))) {
    printf("PRUNING LEAVES ON TIME.\n");
    return;
  }

  /* Create the base track from the given tracklets. */
  inds = mk_ivec(0);
  for(i=0;i<M;i++) {
    ind  = ivec_ref(tbt_pts(tbt_ptr_array_ref(mdl_pts,i)),0);
    temp = track_individs(track_array_ref(pairs,ind));
    for(j=0;j<ivec_size(temp);j++) { add_to_ivec(inds,ivec_ref(temp,j)); }
  }
  base   = mk_track_from_N_inds(obs,inds);
  count  = M;  
  free_ivec(inds);

  /* Check each support point... */
  for(i=0;i<S;i++) {    
    ind = ivec_ref(tbt_pts(tbt_ptr_array_ref(sup_pts,i)),0);
    T   = track_array_ref(pairs,ind);

    X    = track_first(T,obs);
    tp   = simple_obs_time(X);
    track_RA_DEC_prediction(base,tp-track_time(base),&rp,&dp);
    dist = angular_distance_RADEC(rp,simple_obs_RA(X),dp,simple_obs_DEC(X));

    X     = track_last(T,obs);
    tp    = simple_obs_time(X);
    track_RA_DEC_prediction(base,tp-track_time(base),&rp,&dp);
    dist += angular_distance_RADEC(rp,simple_obs_RA(X),dp,simple_obs_DEC(X));

    if(dist/2.0 < pred_fit) {
      add_to_dyv(scores,dist);
      add_to_ivec(mtchs,ind);
      pos_sup++;
    }

    leaf_count++;
  }

  /* Only consider which support points to add, */
  /* if there are enough support points in the  */
  /* first place.                               */
  if(count+pos_sup >= min_sup) {

    /* Create a list of all of the tracks' times. */
    /* Note the indices of a track are already    */
    /* sorted by time.                            */
    inds = mk_copy_ivec(track_individs(base));
    times = mk_zero_dyv(ivec_size(inds));
    for(i=0;i<ivec_size(inds);i++) {
      dyv_set(times,i,simple_obs_time(simple_obs_array_ref(obs,ivec_ref(inds,i))));
    }

    /* Add the compatible tracks in order of good fit. */
    sorted = mk_ivec_sorted_dyv_indices(scores);
    for(i=0;i<ivec_size(mtchs);i++) {
      overlap = FALSE;
      T       = track_array_ref(pairs,ivec_ref(mtchs,ivec_ref(sorted,i)));
      temp    = track_individs(T);

      /* Check if the new tracklet overlaps anything in the track. */
      for(j=0;j<ivec_size(temp);j++) {
        t   = simple_obs_time(simple_obs_array_ref(obs,ivec_ref(temp,j)));
        ind = index_in_sorted_dyv(times,t-1e-10);
        if((ind < dyv_size(times))&&(fabs(dyv_ref(times,ind)-t) < 1e-10)) {
          overlap = TRUE;
        }
      }

      /* If the tracklet does NOT overlap the track.      */
      /* Add it to the result track and resort the times. */
      if(overlap==FALSE) {
        for(j=0;j<ivec_size(temp);j++) {
          t = simple_obs_time(simple_obs_array_ref(obs,ivec_ref(temp,j)));
          add_to_dyv(times,t);
          add_to_ivec(inds,ivec_ref(temp,j));
        }
        count++;

        stimes = mk_sorted_dyv(times);
        free_dyv(times);
        times = stimes;
      }
        
    }
    free_ivec(sorted);

    /* Finally, if we have found enough DISJOINT */
    /* tracklets, create the result track.       */
    if(count >= min_sup) {
      free_track(base);
      base = mk_track_from_N_inds(obs,inds);

      if(fit_rd > mean_sq_track_residual(base,obs)) {
        track_array_add(res,base); 
      }
    }

    free_dyv(times);
    free_ivec(inds);
  }

  if(base != NULL) { free_track(base); }
  free_ivec(mtchs); 
  free_dyv(scores);
}


int test_and_add_support_final(tbt_ptr_array* mdl_pts, tbt* sup_tr, tbt_ptr_array* nu_support,
                               double aminR, double amaxR, double aminD, double amaxD) {
  tbt* F = tbt_ptr_array_ref(mdl_pts,0);
  tbt* L = tbt_ptr_array_ref(mdl_pts,1);
  tbt* mdl_tr;
  tbt *A, *B;
  bool split    = FALSE;
  bool valid    = TRUE;
  bool all_leaf = TRUE;
  int maxpts = 0;
  int count  = 0;
  int M = tbt_ptr_array_size(mdl_pts);
  int i;
  double minR = aminR;
  double maxR = amaxR;
  double minD = aminD;
  double maxD = amaxD;
  double dt, dti, dt2, acc;
  double alpha, wid;

  supp_count++;

  /* Test the support tree for validity against each model tree. */
  for(i=0;(i<M)&&(valid);i++) {

    /* Load the model tree and compute the operation order. */
    mdl_tr = tbt_ptr_array_ref(mdl_pts,i);
    if(tbt_time(sup_tr) < tbt_time(mdl_tr)) {
      A = sup_tr; B = mdl_tr;
    } else {
      B = sup_tr; A = mdl_tr;
    }
    dt  = tbt_lo_time(B) - tbt_hi_time(A);
    dt2 = 2.0 / (dt * dt);
    dti = 1.0/dt;

    /* Check the split and leaf conditions. */
    all_leaf = all_leaf && tbt_is_leaf(mdl_tr);
    /*split    = split || (tbt_N(mdl_tr) < 4*tbt_N(sup_tr));*/
    if(maxpts < tbt_N(mdl_tr)) { maxpts = tbt_N(mdl_tr); }

    /* Do the velocity+position/position tests. */
    if(valid) {
      acc  = dt2*((tbt_hi_RA(B)-tbt_lo_RA(A))-tbt_lo_vRA(A)*dt);
      if(maxR > acc) { maxR = acc; }
      acc  = dt2*((tbt_lo_RA(B)-tbt_hi_RA(A))-tbt_hi_vRA(A)*dt);
      if(minR < acc) { minR = acc; }
      acc  = dt2*((tbt_hi_DEC(B)-tbt_lo_DEC(A))-tbt_lo_vDEC(A)*dt);
      if(maxD > acc) { maxD = acc; }
      acc  = dt2*((tbt_lo_DEC(B)-tbt_hi_DEC(A))-tbt_hi_vDEC(A)*dt);
      if(minD < acc) { minD = acc; }
      valid = (minD <= maxD)&&(minR <= maxR);

      /* Do the velocity+position/position tests. */
      if(valid) {
        acc = dt2*(tbt_hi_RA(A)-tbt_lo_RA(B)+tbt_hi_vRA(B)*dt);
        if(maxR > acc) { maxR = acc; }
        acc = dt2*(tbt_lo_RA(A)-tbt_hi_RA(B)+tbt_lo_vRA(B)*dt);
        if(minR < acc) { minR = acc; }
        acc = dt2*(tbt_hi_DEC(A)-tbt_lo_DEC(B)+tbt_hi_vDEC(B)*dt);
        if(maxD > acc) { maxD = acc; }
        acc = dt2*(tbt_lo_DEC(A)-tbt_hi_DEC(B)+tbt_lo_vDEC(B)*dt);
        if(minD < acc) { minD = acc; }
        valid = (minD <= maxD)&&(minR <= maxR);

        /* Determine the accel bounds with both velocity bounds. */
        if(valid) {
          acc = (tbt_hi_vRA(B) - tbt_lo_vRA(A))*dti;
          if(maxR > acc) { maxR = acc; }
          acc = (tbt_lo_vRA(B) - tbt_hi_vRA(A))*dti;
          if(minR < acc) { minR = acc; }
          acc = (tbt_hi_vDEC(B) - tbt_lo_vDEC(A))*dti;
          if(maxD > acc) { maxD = acc; }
          acc = (tbt_lo_vDEC(B) - tbt_hi_vDEC(A))*dti;
          if(minD < acc) { minD = acc; }
          valid = (minR <= maxR)&&(minD <= maxD);
        }
      }
    }

  }

  /* Determine splitting by the relative width of the node... */
  if(valid) {
    alpha = (tbt_time(sup_tr)-tbt_time(F))/(tbt_time(L)-tbt_time(F));
    
    wid   = (1.0-alpha)*tbt_rad_RA(F) + alpha*tbt_rad_RA(L);
    split = split || (wid < 4.0*tbt_rad_RA(sup_tr));

    wid   = (1.0-alpha)*tbt_rad_vRA(F) + alpha*tbt_rad_vRA(L);
    split = split || (wid < 4.0*tbt_rad_vRA(sup_tr));

    wid   = (1.0-alpha)*tbt_rad_DEC(F) + alpha*tbt_rad_DEC(L);
    split = split || (wid < 4.0*tbt_rad_DEC(sup_tr));

    wid   = (1.0-alpha)*tbt_rad_vDEC(F) + alpha*tbt_rad_vDEC(L);
    split = split || (wid < 4.0*tbt_rad_vDEC(sup_tr));
  }

  if(valid) {
    /* If we are at all leaves, split unless sup_tr is a leaf. */
    split = (split || all_leaf) && (tbt_is_leaf(sup_tr) == FALSE);

    if(split) {
      count = test_and_add_support_final(mdl_pts,tbt_right_child(sup_tr),nu_support,
                                         minR,maxR,minD,maxD);
      count += test_and_add_support_final(mdl_pts,tbt_left_child(sup_tr),nu_support,
                                          minR,maxR,minD,maxD);
    } else {
      tbt_ptr_array_add(nu_support,sup_tr);
      count = 1;
    }
  }

  return count;
}


void tracklets_linker_recurse(simple_obs_array* obs, track_array* pairs,
                              tbt_ptr_array* mdl_pts, tbt_ptr_array* sup_pts,
                              double aR_min, double aR_max, double aD_min, double aD_max,
                              int min_sup, track_array* res, double fit_rd, double pred_fit,
                              double last_start_obs_time, double first_end_obs_time) {
  tbt_ptr_array* nu_support = NULL;
  tbt*           first = tbt_ptr_array_ref(mdl_pts,0);
  tbt*           last  = tbt_ptr_array_ref(mdl_pts,1);
  tbt*           curr;
  double aminD = aD_min;
  double amaxD = aD_max;
  double aminR = aR_min;
  double amaxR = aR_max;
  double tlast = 0.0;
  int split_ind = -1;
  double split_val = -1.0;
  int S = tbt_ptr_array_size(sup_pts);
  int count = 0;
  int added;
  int i;
  bool all_leaf = TRUE;
  bool valid    = TRUE;
  bool madenu   = FALSE;

  /* Check the time constraints. */
  valid = (tbt_lo_time(first) <= last_start_obs_time);
  valid = valid && (tbt_hi_time(last) >= first_end_obs_time);
  if (!valid) {
    printf("Pruning on time (%f vs %f) OR (%f vs %f)\n",
           tbt_lo_time(first), last_start_obs_time,
           tbt_hi_time(last), first_end_obs_time);
  }

  valid = valid && quad_vtree_pairs_determine_abounds_flat(mdl_pts,&aminR,&amaxR,
                                                           &aminD,&amaxD);

  /* Prune the support trees using the new vbounds (if possible). */
  if(valid==TRUE) {

    /* Check if all of the model trees are at leaves. */
    /* And find the largest (nonleaf) tree to split.  */
    all_leaf  = tbt_is_leaf(first) && tbt_is_leaf(last);
    if(tbt_is_leaf(first) == FALSE) {
      split_val = tbt_rad_RA(first)*tbt_rad_DEC(first);
      split_ind = 0;
    }
    if(tbt_is_leaf(last) == FALSE) {
      if(split_val < tbt_rad_RA(last)*tbt_rad_DEC(last)) {
        split_val = tbt_rad_RA(last)*tbt_rad_DEC(last);
        split_ind = 1;
      }
    }

    /* Allocate space for the new array. */
    skip_flip--;
    if((skip_flip==0) || all_leaf) {
      skip_flip = 2;

      nu_support = mk_sized_empty_tbt_ptr_array(S);
      count      = 2;
      madenu     = TRUE;

      /* Check if we can remove the whole tree... */
      for(i=0; i<S; i++) {  
        curr  = tbt_ptr_array_ref(sup_pts,i);
        added = test_and_add_support_final(mdl_pts,curr,nu_support,aminR,amaxR,aminD,amaxD);
      
        if(added >= 1) {
          if(tlast < tbt_time(curr)) { tlast = tbt_time(curr); count++; }
        }
      }  
    } else {
      nu_support = sup_pts;
      count      = min_sup;
      madenu     = FALSE;
    }
  }

  /* If we have enough support, keep going. */
  if(count >= min_sup) {

    if(all_leaf) {
      quad_vtree_pairs_leaf_check(obs,pairs,mdl_pts,nu_support,
                                  min_sup,fit_rd,pred_fit,
                                  last_start_obs_time,first_end_obs_time,res);
    } else {
      curr = tbt_ptr_array_ref(mdl_pts,split_ind);

      tbt_ptr_array_set(mdl_pts,split_ind,tbt_right_child(curr));
      tracklets_linker_recurse(obs,pairs,mdl_pts,nu_support,aminR,amaxR,
                               aminD,amaxD,min_sup,res,fit_rd,pred_fit,
                               last_start_obs_time,first_end_obs_time);
      tbt_ptr_array_set(mdl_pts,split_ind,curr);

      tbt_ptr_array_set(mdl_pts,split_ind,tbt_left_child(curr));
      tracklets_linker_recurse(obs,pairs,mdl_pts,nu_support,aminR,amaxR,
                               aminD,amaxD,min_sup,res,fit_rd,pred_fit,
                               last_start_obs_time,first_end_obs_time);
      tbt_ptr_array_set(mdl_pts,split_ind,curr);
    }

  }

  if(madenu && (nu_support != NULL)) { free_tbt_ptr_array(nu_support); }
}


/* Set up the support points and check time bounds validity. */
void tracklets_linker_prerecurse(simple_obs_array* obs, track_array* pairs,
                                 tbt_ptr_array* mdl_pts, tbt_ptr_array* sup_pts,
                                 double acc_r, double acc_d, int min_sup, 
                                 track_array* res, double fit_rd,
                                 double pred_fit, bool endpts,
                                 double last_start_obs_time,
                                 double first_end_obs_time) {
  tbt_ptr_array* nu_support = NULL;
  tbt* sup_tr;
  tbt* mdl_tr;
  double ts, te;
  int M = tbt_ptr_array_size(mdl_pts);
  int S = tbt_ptr_array_size(sup_pts);
  int i, j;
  bool valid  = TRUE;
  bool svalid = TRUE;

  /* Check that the model trees have the minimum time  */
  /* separation and the time bounds.                   */
  if (last_start_obs_time > 0) {
    valid = valid && (tbt_lo_time(tbt_ptr_array_ref(mdl_pts,0)) <= last_start_obs_time);
  }
  if (first_end_obs_time > 0) {
    valid = valid && (tbt_hi_time(tbt_ptr_array_ref(mdl_pts,M-1)) >= first_end_obs_time);
  }
  for(i=0;(i<M-1)&&(valid);i++) {
    valid = (tbt_mid_time(tbt_ptr_array_ref(mdl_pts,i+1)) -
             tbt_mid_time(tbt_ptr_array_ref(mdl_pts,i)) >= TBT_MIN_TIME);
  }

  /* Next check that the model trees do not overlap the support trees AND */
  /* that the support trees agree with the endpoint constraints.          */
  if(valid) {
    nu_support = mk_sized_empty_tbt_ptr_array(S);

    for(i=0;i<S;i++) {
      svalid = TRUE;
      sup_tr = tbt_ptr_array_ref(sup_pts,i);

      if(endpts) {
        ts = tbt_hi_time(tbt_ptr_array_ref(mdl_pts,0));
        te = tbt_lo_time(tbt_ptr_array_ref(mdl_pts,M-1));
        svalid = (tbt_lo_time(sup_tr) > ts)&&(tbt_hi_time(sup_tr) < te);
      }

      for(j=0;(j<M)&&(svalid);j++) {
        mdl_tr = tbt_ptr_array_ref(mdl_pts,j);
        svalid = (tbt_lo_time(sup_tr) > tbt_hi_time(mdl_tr));
        svalid = svalid || (tbt_hi_time(sup_tr) < tbt_lo_time(mdl_tr));
      }

      if(svalid) { tbt_ptr_array_add(nu_support,sup_tr); }
    }

    if(tbt_ptr_array_size(nu_support)+M >= min_sup) {
      tracklets_linker_recurse(obs, pairs, mdl_pts, nu_support, -acc_r, acc_r,
                               -acc_d, acc_d, min_sup, res, fit_rd, pred_fit,
                               last_start_obs_time, first_end_obs_time);
    }

    free_tbt_ptr_array(nu_support);
  }
}


track_array* mk_vtrees_tracks(simple_obs_array* obs, track_array* pairs,
                              double thresh, double acc_r, double acc_d,
                              int min_sup, int K, double fit_rd, double pred_fit,
                              bool endpts, double plate_width,
                              double last_start_obs_time, double first_end_obs_time) {
  track_array*   res = mk_empty_track_array(10);
  dym*           tb_arr;
  tbt_ptr_array* tr_arr;
  tbt_ptr_array* mdl;
  tbt*           tr;
  int            T;
  int            i,j;

  skip_flip = 2;

  /* Turn the tracklets into bounding boxes and build a tree  */
  /* on the points.  Turn this tree into an array of subtrees */
  /* with time width = 0.                                     */
  // printf(">> Bounding the tracklets ("); printf(curr_time()); printf(")\n");
  tb_arr = mk_tracklet_bounds(obs,pairs,thresh,plate_width);
  tr = mk_tbt(tb_arr,NULL,TRUE,1);
  tr_arr = mk_empty_tbt_ptr_array();
  fill_plate_tbt_ptr_array(tr,tr_arr);
  T = tbt_ptr_array_size(tr_arr);
  free_dym(tb_arr);

  // printf(">> Starting the VTREE run ("); printf(curr_time()); printf(")\n");
  mdl = mk_sized_empty_tbt_ptr_array(2);
  for(i=0;i<T;i++) {
    for(j=i+1;j<T;j++) {
      tbt_ptr_array_set(mdl,0,tbt_ptr_array_ref(tr_arr,i));
      tbt_ptr_array_set(mdl,1,tbt_ptr_array_ref(tr_arr,j));      

      tracklets_linker_prerecurse(obs, pairs, mdl, tr_arr, acc_r, acc_d,
                                  min_sup, res, fit_rd, pred_fit, endpts,
                                  last_start_obs_time, first_end_obs_time);
    }
  }

  free_tbt_ptr_array(tr_arr);
  free_tbt_ptr_array(mdl);
  free_tbt(tr);

  return res;
}



/* ---------------------------------------------------- */
/* --- Sequential Tree Search Function (accel prune) -- */
/* ---------------------------------------------------- */


/* Find the LAST point recursively (accel only pruning) */
void seq_accel_findsecond(simple_obs_array* obs, track_array* pairs,
                          tbt_ptr_array* all_trs, tbt_ptr_array* mdl_pts,
                          double acc_r, double acc_d, int min_sup,
                          double thresh, double fit_rd, double pred_fit,
                          double last_start_obs_time, double first_end_obs_time,
                          track_array* res) {
  tbt_ptr_array* supp = NULL; 
  tbt*   first = tbt_ptr_array_ref(mdl_pts,0);
  tbt*   last  = tbt_ptr_array_ref(mdl_pts,1);
  tbt*   curr  = NULL;
  double aminD = -acc_d;
  double amaxD =  acc_d;
  double aminR = -acc_r;
  double amaxR =  acc_r;
  bool   valid = TRUE;
  int    count = 2;
  int    added = 0;
  int    T     = tbt_ptr_array_size(all_trs);
  int    i;

  /* Is the first tree compatible with the last tree? */
  valid = (tbt_mid_time(tbt_ptr_array_ref(mdl_pts,1)) -
           tbt_mid_time(tbt_ptr_array_ref(mdl_pts,0)) >= TBT_MIN_TIME);
  if(valid) {
    valid = quad_vtree_pairs_determine_abounds_flat(mdl_pts,&aminR,&amaxR,&aminD,&amaxD);
  }

  /* If the two trees are valid (accel-wise) */
  if(valid) {

    if(tbt_is_leaf(last)) {

      /* Build a new support list. */
      supp = mk_sized_empty_tbt_ptr_array(T);
      for(i=0;i<T;i++) {
        curr = tbt_ptr_array_ref(all_trs,i);
        if((tbt_mid_time(curr) < tbt_mid_time(last))&&
           (tbt_mid_time(first) < tbt_mid_time(curr))) {
          added = test_and_add_support_final(mdl_pts,curr,supp,aminR,amaxR,aminD,amaxD);
          if(added >= 1) { count++; }
        }
      }

      /* Actually do the leaf test. */
      if(count >= min_sup) {
        quad_vtree_pairs_leaf_check(obs,pairs,mdl_pts,supp,
                                    min_sup,fit_rd,pred_fit,
                                    last_start_obs_time,
                                    first_end_obs_time,res);
      }

      free_tbt_ptr_array(supp);
    } else {
      tbt_ptr_array_set(mdl_pts,1,tbt_right_child(last));
      seq_accel_findsecond(obs,pairs,all_trs,mdl_pts,
                           acc_r,acc_d,min_sup,thresh,fit_rd,pred_fit,
                           last_start_obs_time,first_end_obs_time,
                           res);
      tbt_ptr_array_set(mdl_pts,1,tbt_left_child(last));
      seq_accel_findsecond(obs,pairs,all_trs,mdl_pts,
                           acc_r,acc_d,min_sup,thresh,fit_rd,pred_fit,
                           last_start_obs_time,first_end_obs_time,
                           res);
      tbt_ptr_array_set(mdl_pts,1,last);
    }

  }

}


/* Find the FIRST point recursively (no pruning). */
void seq_accel_findfirst(simple_obs_array* obs, track_array* pairs,
                         tbt_ptr_array* all_trs, tbt_ptr_array* mdl_pts,
                         double acc_r, double acc_d, int min_sup, 
                         double thresh, double fit_rd, double pred_fit,
                         double last_start_obs_time, double first_end_obs_time,
                         track_array* res) {
  tbt* curr = tbt_ptr_array_ref(mdl_pts,0);

  if(tbt_is_leaf(curr)) {
    seq_accel_findsecond(obs,pairs,all_trs,mdl_pts,
                         acc_r,acc_d,min_sup,thresh,fit_rd,pred_fit,
                         last_start_obs_time,first_end_obs_time,res);
  } else {
    tbt_ptr_array_set(mdl_pts,0,tbt_right_child(curr));
    seq_accel_findfirst(obs,pairs,all_trs,mdl_pts,
                        acc_r,acc_d,min_sup,thresh,fit_rd,pred_fit,
                        last_start_obs_time,first_end_obs_time,res);
    tbt_ptr_array_set(mdl_pts,0,tbt_left_child(curr));
    seq_accel_findfirst(obs,pairs,all_trs,mdl_pts,
                        acc_r,acc_d,min_sup,thresh,fit_rd,pred_fit,
                        last_start_obs_time,first_end_obs_time,res);
    tbt_ptr_array_set(mdl_pts,0,curr);
  }

}

/* A sequential search with accel only based pruning. */
/* For each starting track, find EACH possible ending */
/* track and search all of the tracks in between.     */
track_array* mk_sequential_accel_only_tracks(simple_obs_array* obs, track_array* pairs,
                                             double thresh, double acc_r, double acc_d,
                                             int min_sup, double fit_rd, double pred_fit,
                                             double plate_width,
                                             double last_start_obs_time,
                                             double first_end_obs_time) {
  track_array*   res = mk_empty_track_array(10);
  dym*           tb_arr;
  tbt_ptr_array* tr_arr;
  tbt_ptr_array* mdl_pts;
  tbt*           tr;
  int            i, j;
  int            T;

  supp_count = 0;
  leaf_count = 0;
  pairs_count = 0;

  /* Turn the tracklets into bounding boxes and build a tree  */
  /* on the points.  Turn this tree into an array of subtrees */
  /* with time width = 0.                                     */
  tb_arr = mk_tracklet_bounds(obs,pairs,thresh,plate_width);
  tr = mk_tbt(tb_arr,NULL,TRUE,1);
  tr_arr = mk_empty_tbt_ptr_array();
  fill_plate_tbt_ptr_array(tr,tr_arr);
  T = tbt_ptr_array_size(tr_arr);
  free_dym(tb_arr);

  for(i=0;i<T;i++) {
    printf("Time %6i (%15f, %15f) has %i points.\n",i,
           tbt_mid_time(tbt_ptr_array_ref(tr_arr,i)),
           tbt_rad_time(tbt_ptr_array_ref(tr_arr,i)),
           tbt_N(tbt_ptr_array_ref(tr_arr,i)));
  }

  /* Explore each PAIR of time steps that are well    */
  /* spaced enough to have the enough support points. */
  mdl_pts = mk_sized_empty_tbt_ptr_array(2);
  for(i=0;i<T;i++) {
    for(j=(i+min_sup-1);j<T;j++) {
      tbt_ptr_array_set(mdl_pts,0,tbt_ptr_array_ref(tr_arr,i));
      tbt_ptr_array_set(mdl_pts,1,tbt_ptr_array_ref(tr_arr,j));

      seq_accel_findfirst(obs,pairs,tr_arr,mdl_pts,acc_r,acc_d,min_sup,
                          thresh,fit_rd,pred_fit,
                          last_start_obs_time,first_end_obs_time,res);
  
    }
  }

  printf("Stopped at %i leaf pairs.\n",pairs_count);

  free_tbt_ptr_array(mdl_pts);
  free_tbt_ptr_array(tr_arr);
  free_tbt(tr);

  return res;
}


