/*
   File:        t_tree.c
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

#include "t_tree.h"

int t_tree_count;

/* --------------------------------------------------------------------- */
/* --- Useful Helper Functions ----------------------------------------- */
/* --------------------------------------------------------------------- */

void t_tree_fill_bounds(t_tree* tr, track_array* obs, ivec* inds) {
  track* X;
  double val;
  bool first = TRUE;
  int i;

  for(i=0;i<ivec_size(inds);i++) {
    X = track_array_ref(obs,ivec_ref(inds,i));

    if(X != NULL) {

      val = track_time(X);
      if(first || (tr->hi[T_TIME] < val)) { tr->hi[T_TIME] = val; }
      if(first || (tr->lo[T_TIME] > val)) { tr->lo[T_TIME] = val; }

      val = track_RA(X);
      if(first || (tr->hi[T_R] < val)) { tr->hi[T_R] = val; }
      if(first || (tr->lo[T_R] > val)) { tr->lo[T_R] = val; }

      val = track_DEC(X);
      if(first || (tr->hi[T_D] < val)) { tr->hi[T_D] = val; }
      if(first || (tr->lo[T_D] > val)) { tr->lo[T_D] = val; }

      val = track_vRA(X);
      if(first || (tr->hi[T_VR] < val)) { tr->hi[T_VR] = val; }
      if(first || (tr->lo[T_VR] > val)) { tr->lo[T_VR] = val; }

      val = track_vDEC(X);
      if(first || (tr->hi[T_VD] < val)) { tr->hi[T_VD] = val; }
      if(first || (tr->lo[T_VD] > val)) { tr->lo[T_VD] = val; }

      val = track_brightness(X);
      if(first || (tr->hi[T_BR] < val)) { tr->hi[T_BR] = val; }
      if(first || (tr->lo[T_BR] > val)) { tr->lo[T_BR] = val; }

      first = FALSE;
    }
  }
}


void t_tree_fill_bounds_from_simple_obs(t_tree* tr, simple_obs_array* obs, ivec* inds) {
  simple_obs* X;
  double val;
  bool first = TRUE;
  int i;

  tr->lo[T_VR] = 0.0;  tr->hi[T_VR] = 0.0;
  tr->lo[T_VD] = 0.0;  tr->hi[T_VD] = 0.0;

  for(i=0;i<ivec_size(inds);i++) {
    X = simple_obs_array_ref(obs,ivec_ref(inds,i));

    if(X != NULL) {

      val = simple_obs_time(X);
      if(first || (tr->hi[T_TIME] < val)) { tr->hi[T_TIME] = val; }
      if(first || (tr->lo[T_TIME] > val)) { tr->lo[T_TIME] = val; }

      val = simple_obs_RA(X);
      if(first || (tr->hi[T_R] < val)) { tr->hi[T_R] = val; }
      if(first || (tr->lo[T_R] > val)) { tr->lo[T_R] = val; }

      val = simple_obs_DEC(X);
      if(first || (tr->hi[T_D] < val)) { tr->hi[T_D] = val; }
      if(first || (tr->lo[T_D] > val)) { tr->lo[T_D] = val; }

      val = simple_obs_brightness(X);
      if(first || (tr->hi[T_BR] < val)) { tr->hi[T_BR] = val; }
      if(first || (tr->lo[T_BR] > val)) { tr->lo[T_BR] = val; }

      first = FALSE;
    }
  }
}


/* --------------------------------------------------------------------- */
/* --- Tree Memory Functions ------------------------------------------- */
/* --------------------------------------------------------------------- */

dyv* mk_t_tree_weights(double time, double ra, double dec,
           double vra, double vdec, double brightness) {
  dyv* res = mk_zero_dyv(T_NUM_DIMS);

  dyv_set(res,T_TIME,time);
  dyv_set(res,T_R,ra);
  dyv_set(res,T_D,dec);
  dyv_set(res,T_VR,vra);
  dyv_set(res,T_VD,vdec);
  dyv_set(res,T_BR,brightness);
  
  return res;
}


t_tree* mk_empty_t_tree() {
  t_tree* res = AM_MALLOC(t_tree);
  int i;

  res->num_points = 0;
  res->split_dim  = -1;
  res->split_val  = 0.0;
  res->from_tracks = TRUE;

  res->right   = NULL;
  res->left    = NULL;
  res->trcks   = NULL;

  for(i=0;i<T_NUM_DIMS;i++) {
    res->hi[i]  = 0.0;
    res->lo[i]  = 0.0;
  }

  return res;
}


void free_t_tree(t_tree* old) {
  if(old->right)   { free_t_tree(old->right); }
  if(old->left)    { free_t_tree(old->left);  }
  if(old->trcks)   { free_ivec(old->trcks); }

  AM_FREE(old,t_tree);
}


t_tree* mk_t_tree_recurse(track_array* obs, ivec* inds, dyv* widths, 
                          dyv* weights, int min_leaf_pts) {
  track* X;
  t_tree* res;
  ivec *left, *right;
  double split_width, val;
  int sd, i, N;
  
  /* Allocate space for the tree and calculate the bounds of the node */
  res = mk_empty_t_tree(); 
  N   = ivec_size(inds);
  t_tree_fill_bounds(res,obs,inds);
  res->num_points = N;
  
  /* Determine if this node will be a leaf or internal */
  if(ivec_size(inds) <= min_leaf_pts) {
    res->trcks = mk_copy_ivec(inds);
  } else {

    /* Pick the widest dimension and split it. */
    split_width = 0.0;
    sd          = -1;
    for(i=0;i<T_NUM_DIMS;i++) {
      val = (t_rad_bound(res,i) / dyv_ref(widths,i)) * dyv_ref(weights,i);
      if((i==0)||(val > split_width)) {
        split_width = val;
        sd = i;
      }
    }
    res->split_val = t_mid_bound(res,sd);
    res->split_dim = sd;

    /* Actually divide up the points. */
    left = mk_ivec(0);
    right = mk_ivec(0);

    for(i=0;i<ivec_size(inds);i++) {
      X = track_array_ref(obs,ivec_ref(inds,i));
      val = 0.0;

      switch(sd) {
      case T_TIME:  val = track_time(X); break;
      case T_R:     val = track_RA(X);   break;
      case T_D:     val = track_DEC(X);  break;
      case T_VR:    val = track_vRA(X);  break;
      case T_VD:    val = track_vDEC(X); break;
      case T_BR:    val = track_brightness(X); break;
      } 
    
      if(val < res->split_val) {
        add_to_ivec(left,ivec_ref(inds,i));
      } else {
        add_to_ivec(right,ivec_ref(inds,i));
      }
    }

    /* Build the left and right sub-trees */
    res->left  = mk_t_tree_recurse(obs,left,widths,weights,min_leaf_pts);
    res->right = mk_t_tree_recurse(obs,right,widths,weights,min_leaf_pts);

    free_ivec(left);
    free_ivec(right);
  }

  return res;
}


t_tree* mk_t_tree(track_array* arr, simple_obs_array* obs,
                  dyv* W, int max_leaf_pts) {
  t_tree *res;
  ivec *inds;
  dyv *width;
  double val;
  int N = track_array_size(arr);
  int i;

  /* Force all of the tracks to their starting time. */
  for(i=0;i<N;i++) { track_force_t0_first(track_array_ref(arr,i),obs); }

  /* Store all the indices for the tree. */
  inds  = mk_sequence_ivec(0,N);
  
  /* Calculate the initial width of each factor. */
  res  = mk_empty_t_tree();
  t_tree_fill_bounds(res, arr, inds);
  width = mk_zero_dyv(T_NUM_DIMS);
  for(i=0;i<T_NUM_DIMS;i++) {
    val = t_rad_bound(res,i);
    if(val < 1e-20) { val = 1e-20; }
    dyv_set(width,i,val);
  }
  free_t_tree(res);

  /* Build the tree. */
  res = mk_t_tree_recurse(arr, inds, width, W, max_leaf_pts);

  /* Free the used memory */
  free_dyv(width);
  free_ivec(inds);

  return res;
}


t_tree* mk_t_tree_recurse_obs(simple_obs_array* obs, ivec* inds, dyv* widths, 
                              dyv* weights, int min_leaf_pts) {
  simple_obs* X;
  t_tree* res;
  ivec *left, *right;
  double split_width, val;
  int sd, i, N;
  
  /* Allocate space for the tree and calculate the bounds of the node */
  res = mk_empty_t_tree(); 
  N = ivec_size(inds);
  t_tree_fill_bounds_from_simple_obs(res,obs,inds);
  res->num_points  = N;
  res->from_tracks = FALSE;
  
  /* Determine if this node will be a leaf or internal */
  if(ivec_size(inds) <= min_leaf_pts) {
    res->trcks = mk_copy_ivec(inds);
  } else {

    /* Pick the widest dimension and split it. */
    split_width = 0.0;
    sd = -1;
    for(i=0;i<T_NUM_DIMS;i++) {
      val = (t_rad_bound(res,i) / dyv_ref(widths,i)) * dyv_ref(weights,i);
      if((i==0)||(val > split_width)) {
        split_width = val;
        sd = i;
      }
    }
    res->split_val = t_mid_bound(res,sd);
    res->split_dim = sd;

    /* Actually divide up the points. */
    left = mk_ivec(0);
    right = mk_ivec(0);

    for(i=0;i<ivec_size(inds);i++) {
      X = simple_obs_array_ref(obs,ivec_ref(inds,i));
      val = 0.0;

      switch(sd) {
      case T_TIME: val = simple_obs_time(X); break;
      case T_R: val = simple_obs_RA(X); break;
      case T_D: val = simple_obs_DEC(X); break;
      case T_BR: val = simple_obs_brightness(X); break;
      } 
    
      if(val < res->split_val) {
        add_to_ivec(left,ivec_ref(inds,i));
      } else {
        add_to_ivec(right,ivec_ref(inds,i));
      }
    }

    /* Build the left and right sub-trees */
    res->left = mk_t_tree_recurse_obs(obs,left,widths,weights,min_leaf_pts);
    res->right = mk_t_tree_recurse_obs(obs,right,widths,weights,min_leaf_pts);

    free_ivec(left);
    free_ivec(right);
  }

  return res;
}


t_tree* mk_t_tree_from_simple_obs(simple_obs_array* obs,
                                  dyv* W, int max_leaf_pts) {
  t_tree *res;
  ivec *inds;
  dyv *width;
  double val;
  int N = simple_obs_array_size(obs);
  int i;

  /* Store all the indices for the tree. */
  inds = mk_sequence_ivec(0,N);
  
  /* Calculate the initial width of each factor. */
  res = mk_empty_t_tree();
  t_tree_fill_bounds_from_simple_obs(res, obs, inds);
  width = mk_zero_dyv(T_NUM_DIMS);
  for(i=0;i<T_NUM_DIMS;i++) {
    val = t_rad_bound(res,i);
    if(val < 1e-20) { val = 1e-20; }
    dyv_set(width,i,val);
  }
  free_t_tree(res);

  /* Build the tree. */
  res = mk_t_tree_recurse_obs(obs, inds, width, W, max_leaf_pts);

  /* Free the used memory */
  free_dyv(width);
  free_ivec(inds);

  return res;
}



/* --------------------------------------------------------------------- */
/* --- Getter/Setter Functions ----------------------------------------- */
/* --------------------------------------------------------------------- */

int safe_t_num_points(t_tree* tr) {
  return tr->num_points;
}

int safe_t_split_dim(t_tree* tr) {
  return tr->split_dim;
}

double safe_t_split_val(t_tree* tr) {
  return tr->split_val;
}

bool safe_t_is_leaf(t_tree* tr) {
  return (tr->split_dim == -1);
}

ivec* safe_t_tracks(t_tree* tr) {
  return tr->trcks;
}

t_tree* safe_t_right_child(t_tree* tr) {
  return tr->right;
}

t_tree* safe_t_left_child(t_tree* tr) {
  return tr->left;
}

double safe_t_hi_time(t_tree* tr)  { return tr->hi[T_TIME]; }
double safe_t_lo_time(t_tree* tr)  { return tr->lo[T_TIME]; }
double safe_t_mid_time(t_tree* tr) { return JK_MID(tr->lo[T_TIME], tr->hi[T_TIME]); }
double safe_t_rad_time(t_tree* tr) { return JK_RAD(tr->lo[T_TIME], tr->hi[T_TIME]); }

double safe_t_hi_ra(t_tree* tr)  { return tr->hi[T_R]; }
double safe_t_lo_ra(t_tree* tr)  { return tr->lo[T_R]; }
double safe_t_mid_ra(t_tree* tr) { return JK_MID(tr->lo[T_R], tr->hi[T_R]); }
double safe_t_rad_ra(t_tree* tr) { return JK_RAD(tr->lo[T_R], tr->hi[T_R]); }

double safe_t_hi_vra(t_tree* tr)  { return tr->hi[T_VR]; }
double safe_t_lo_vra(t_tree* tr)  { return tr->lo[T_VR]; }
double safe_t_mid_vra(t_tree* tr) { return JK_MID(tr->lo[T_VR], tr->hi[T_VR]); }
double safe_t_rad_vra(t_tree* tr) { return JK_RAD(tr->lo[T_VR], tr->hi[T_VR]); }

double safe_t_hi_dec(t_tree* tr)  { return tr->hi[T_D]; }
double safe_t_lo_dec(t_tree* tr)  { return tr->lo[T_D]; }
double safe_t_mid_dec(t_tree* tr) { return JK_MID(tr->lo[T_D], tr->hi[T_D]); }
double safe_t_rad_dec(t_tree* tr) { return JK_RAD(tr->lo[T_D], tr->hi[T_D]); }

double safe_t_hi_vdec(t_tree* tr)  { return tr->hi[T_VD]; }
double safe_t_lo_vdec(t_tree* tr)  { return tr->lo[T_VD]; }
double safe_t_mid_vdec(t_tree* tr) { return JK_MID(tr->lo[T_VD], tr->hi[T_VD]); }
double safe_t_rad_vdec(t_tree* tr) { return JK_RAD(tr->lo[T_VD], tr->hi[T_VD]); }

double safe_t_hi_bright(t_tree* tr)  { return tr->hi[T_BR]; }
double safe_t_lo_bright(t_tree* tr)  { return tr->lo[T_BR]; }
double safe_t_mid_bright(t_tree* tr) { return JK_MID(tr->lo[T_BR], tr->hi[T_BR]); }
double safe_t_rad_bright(t_tree* tr) { return JK_RAD(tr->lo[T_BR], tr->hi[T_BR]); }


double safe_t_hi_bound(t_tree* tr, int dim) {
  my_assert((dim >= 0)&&(dim < T_NUM_DIMS));
  return tr->hi[dim];
}

double safe_t_lo_bound(t_tree* tr, int dim) {
  my_assert((dim >= 0)&&(dim < T_NUM_DIMS));
  return tr->lo[dim];
}

double safe_t_mid_bound(t_tree* tr, int dim) {
  my_assert((dim >= 0)&&(dim < T_NUM_DIMS));
  return JK_MID(tr->lo[dim],tr->hi[dim]);
}

double safe_t_rad_bound(t_tree* tr, int dim) {
  my_assert((dim >= 0)&&(dim < T_NUM_DIMS));
  return JK_RAD(tr->lo[dim],tr->hi[dim]);
}

double t_lo_vel_bound(t_tree* tr, int dim) {
  double res = 0.0;

  switch(dim) {
  case T_TIME: res = -1.0; break;
  case T_R: res = t_lo_bound(tr,T_VR); break;
  case T_D: res = t_lo_bound(tr,T_VD); break;
  case T_VR: res = 0.0; break;
  case T_VD: res = 0.0; break;
  case T_BR: res = 0.0; break;
  }  
 
  return res;
}


double t_hi_vel_bound(t_tree* tr, int dim) {
  double res = 0.0;

  switch(dim) {
  case T_TIME: res = 1.0; break;
  case T_R: res = t_hi_bound(tr,T_VR); break;
  case T_D: res = t_hi_bound(tr,T_VD); break;
  case T_VR: res = 0.0; break;
  case T_VD: res = 0.0; break;
  case T_BR: res = 0.0; break;
  }

  return res;
}


/* --------------------------------------------------------------------- */
/* --- Simple Access Helper Functions ---------------------------------- */
/* --------------------------------------------------------------------- */

/* Gets a track's value associated with the given dimension. */
double t_tree_track_value(track* X, int dim) {
  double res = 0.0;

  switch(dim) {
  case T_TIME: res = track_time(X); break;
  case T_R: res = track_RA(X); break;
  case T_VR: res = track_vRA(X); break;
  case T_D: res = track_DEC(X);  break;
  case T_VD: res = track_vDEC(X); break;
  case T_BR: res = track_brightness(X); break;
  }

  return res;
}


/* Gets a track's velocity associated with the given dimension. */
double t_tree_track_velocity(track* X, int dim) {
  double res = 0.0;

  switch(dim) {
  case T_TIME: res = 1.0; break;
  case T_R: res = track_vRA(X); break;
  case T_VR: res = track_aRA(X); break;
  case T_D: res = track_vDEC(X); break;
  case T_VD: res = track_aDEC(X); break;
  case T_BR: res = 0.0; break;
  }

  return res;
}


double t_tree_predict_value(track* X, int dim, double time) {
  double res = 0.0;
  double dt = time - track_time(X);
  double a,b,c;

  switch(dim) {
  case T_TIME: res = time; break;
  case T_R: track_RA_DEC_prediction(X,dt,&res,&a); break;
  case T_VR: track_RDVV_prediction(X,dt,&a,&b,&res,&c); break;
  case T_D: track_RA_DEC_prediction(X,dt,&a,&res); break;
  case T_VD: track_RDVV_prediction(X,dt,&a,&b,&c,&res); break;
  case T_BR: res = track_brightness(X); break;
  }

  return res;

}


/* Gets an observation's value associated with the given dimension. */
double t_tree_simple_obs_value(simple_obs* X, int dim) {
  double res = 0.0;

  switch(dim) {
  case T_TIME: res = simple_obs_time(X); break;
  case T_R: res = simple_obs_RA(X); break;
  case T_D: res = simple_obs_DEC(X); break;
  case T_BR: res = simple_obs_brightness(X); break;
  }

  return res;
}


ivec* mk_t_tree_tracks(t_tree* X) {
  ivec* res;
  ivec* L;
  ivec* R;

  if(t_is_leaf(X)) {
    res = mk_copy_ivec(t_tracks(X));
  } else {
    L = mk_t_tree_tracks(t_left_child(X));
    R = mk_t_tree_tracks(t_right_child(X));

    res = mk_ivec_union(L,R);

    free_ivec(L);
    free_ivec(R);
  }

  return res;
}


/* --------------------------------------------------------------------- */
/* --- Simple I/O Functions -------------------------------------------- */
/* --------------------------------------------------------------------- */

void fprintf_t_tree_simple_recurse(FILE* f, t_tree* t, int level) {
  int i;

  for(i=0;i<level;i++) { fprintf(f,"-"); }
  fprintf(f," [%i pts, split=%i (%f)]\n",t_num_points(t),t_split_dim(t),t_split_val(t));
  if(t_is_leaf(t)==FALSE) {
    fprintf_t_tree_simple_recurse(f,t_left_child(t),level+1);
    fprintf_t_tree_simple_recurse(f,t_right_child(t),level+1);
  } else {
    for(i=0;i<level;i++) { fprintf(f,"-"); }
    fprintf_ivec(f,"   ",t->trcks,"\n");
  }
}


void fprintf_t_tree_simple(FILE* f, char* pre, t_tree* tr, char* post) {
  fprintf(f,pre);
  fprintf_t_tree_simple_recurse(f,tr,0);
  fprintf(f,post);
}


void fprintf_t_tree_node(FILE* f, char* pre, t_tree* tr, char* post) {
  fprintf(f,pre);
  fprintf(f,"Node with %i points and %i children\n",t_num_points(tr),
          (t_is_leaf(tr) ? 0 : 2));
  fprintf(f," TIME bounds    = [%10.8f, %10.8f], mid = %10.8f, rad = %10.8f\n",
          t_lo_time(tr),t_hi_time(tr),t_mid_time(tr),t_rad_time(tr));
  fprintf(f," RA bounds   = [%10.8f, %10.8f], mid = %10.8f, rad = %10.8f\n",
          t_lo_ra(tr),t_hi_ra(tr),t_mid_ra(tr),t_rad_ra(tr));
  fprintf(f," DEC bounds  = [%10.8f, %10.8f], mid = %10.8f, rad = %10.8f\n",
          t_lo_dec(tr),t_hi_dec(tr),t_mid_dec(tr),t_rad_dec(tr));
  fprintf(f," vRA bounds  = [%10.8f, %10.8f], mid = %10.8f, rad = %10.8f\n",
          t_lo_vra(tr),t_hi_vra(tr),t_mid_vra(tr),t_rad_vra(tr));
  fprintf(f," vDEC bounds = [%10.8f, %10.8f], mid = %10.8f, rad = %10.8f\n",
          t_lo_vdec(tr),t_hi_vdec(tr),t_mid_vdec(tr),t_rad_vdec(tr));
  fprintf(f,post);
}



/* --------------------------------------------------------------------- */
/* --- Query Functions ------------------------------------------------- */
/* --------------------------------------------------------------------- */

dyv* mk_t_tree_thresh(double time, double ra, double dec,
                      double vra, double vdec, double brightness) {
  dyv* res = mk_constant_dyv(T_NUM_DIMS,-1.0);

  dyv_set(res,T_TIME,time);
  dyv_set(res,T_R,ra);
  dyv_set(res,T_D,dec);
  dyv_set(res,T_VR,vra);
  dyv_set(res,T_VD,vdec);
  dyv_set(res,T_BR,brightness);
  
  return res;
}


dyv* mk_t_tree_RADEC_thresh(double thresh) {
  dyv* res = mk_constant_dyv(T_NUM_DIMS,-1.0);

  dyv_set(res,T_R,thresh/15.0);
  dyv_set(res,T_D,thresh);

  return res;
}



dyv* mk_t_tree_thresh_ignore_all() {
  return mk_constant_dyv(T_NUM_DIMS,-1.0);
}


dyv* mk_t_tree_query(double time, double ra, double dec,
                      double vra, double vdec, double brightness) {
  dyv* res = mk_zero_dyv(T_NUM_DIMS);

  dyv_set(res,T_TIME,time);
  dyv_set(res,T_R,ra);
  dyv_set(res,T_D,dec);
  dyv_set(res,T_VR,vra);
  dyv_set(res,T_VD,vdec);
  dyv_set(res,T_BR,brightness);
  
  return res;
}


dyv* mk_t_tree_track_dyv(track* X) {
  dyv* res = mk_constant_dyv(T_NUM_DIMS,-1.0);

  dyv_set(res,T_TIME,track_time(X));
  dyv_set(res,T_R,track_RA(X));
  dyv_set(res,T_D,track_DEC(X));
  dyv_set(res,T_VR,track_vRA(X));
  dyv_set(res,T_VD,track_vDEC(X));
  dyv_set(res,T_BR,track_brightness(X));

  return res;
}


dyv* mk_t_tree_accel(double time, double ra, double dec,
                     double vra, double vdec, double brightness) {
  dyv* res = mk_zero_dyv(T_NUM_DIMS);

  dyv_set(res,T_TIME,time);
  dyv_set(res,T_R,ra);
  dyv_set(res,T_D,dec);
  dyv_set(res,T_VR,vra);
  dyv_set(res,T_VD,vdec);
  dyv_set(res,T_BR,brightness);
  
  return res;
}



/* --- Simple Range Search Queries ----------------------------------- */


ivec* mk_t_tree_near_point_slow(track_array* obs, ivec* inds, track* Q, dyv* thresh) {
  double val1, val2, diff, thrsh;
  track* X;
  ivec*  res;
  int N = ivec_size(inds);
  int i, j, ind;
  bool prune;

  res = mk_ivec(0);

  for(i=0;i<N;i++) {
    ind = i;
    if(inds) { ind = ivec_ref(inds,i); }

    X     = track_array_ref(obs,ind);
    prune = FALSE;

    t_tree_count++;

    /* Check EACH of the dimensions... */
    for(j=0;(j<T_NUM_DIMS)&&(prune==FALSE);j++) {
      thrsh = dyv_ref(thresh,j);
      if((j == T_TIME)&&(thrsh < 1e-6)) { thrsh = 1e-6; }

      if(thrsh > 1e-20) {
        val1 = t_tree_track_value(X,j);
        val2 = t_tree_track_value(Q,j);
        diff = fabs(val1-val2);

        /* Handle wrap arounds in RA */
        if(j==T_R) { while(diff > 12.0) { diff = fabs(diff-24.0); } }
        
        /* Decide whether to prune... */
        prune = (diff > thrsh);
      }
    }

    if(prune==FALSE) {
      add_to_ivec(res,ind);
    }
  }

  return res;
}



void t_tree_near_point_recurse(t_tree* tr, track_array* obs, 
                               dyv* Q, track* query, dyv* thresh, ivec* res) {
  double val1, val2, diff, thrsh;
  ivec*  temp;
  int i, j, old;
  bool prune = FALSE;

  t_tree_count++;
  old = ivec_size(res);

  /* Check EACH dimension for pruning ops... */
  for(j=0;(j<T_NUM_DIMS)&&(prune==FALSE);j++) {

    thrsh = dyv_ref(thresh,j);
    if((j == T_TIME)&&(thrsh < 1e-6)) { thrsh = 1e-6; }

    if(thrsh > 1e-20) {
      val1 = t_mid_bound(tr,j);
      val2 = dyv_ref(Q,j);
      diff = fabs(val1-val2);

      /* Handle wrap arounds in RA */
      if(j==T_R) { while(diff > 12.0) { diff = fabs(diff-24.0); } }
        
      /* Decide whether to prune... */
      prune = (diff > thrsh + t_rad_bound(tr,j));
    }
  }

  /* If we cannot prune this branch... then recurse. */
  if(prune==FALSE) {
    if(t_is_leaf(tr)) {
      temp = mk_t_tree_near_point_slow(obs, t_tracks(tr), query, thresh);
      for(i=0;i<ivec_size(temp);i++) { add_to_ivec(res,ivec_ref(temp,i)); }
      free_ivec(temp);
    } else {
      t_tree_near_point_recurse(t_left_child(tr),obs,Q,query,thresh,res);
      t_tree_near_point_recurse(t_right_child(tr),obs,Q,query,thresh,res);
    }
  }
}


ivec* mk_t_tree_near_point(t_tree* tr, track_array* obs, track* Q, dyv* thresh) {
  ivec* res   = mk_ivec(0);
  dyv*  query = mk_t_tree_track_dyv(Q);

  t_tree_near_point_recurse(tr, obs, query, Q, thresh, res);
  free_dyv(query);

  return res;
}



/* --- Midpoint Range Search Queries --------------------------------- */

/* Find all tracks Y such that the midpoint_distance(X,Y) is <= thresh  */
/* and t_s <= Y.time <= t_e.  If inds == NULL look at all tracks.       */
ivec* mk_t_tree_find_midpt_slow(track_array* arr, ivec* inds, track* X,
                                double t_s, double t_e, dyv* thresh, dyv* accel) {
  track* Y;
  ivec* res = mk_ivec(0);
  double ta, tb, tm, td;
  double vala, valb, acc, diff;
  double thrsh;
  int i, j, N, ind;
  bool prune;

  ta = track_time(X);
  N  = track_array_size(arr);
  if(inds) { N = ivec_size(inds); }

  for(i=0;i<N;i++) {
    t_tree_count++;
    prune = FALSE;

    /* Get the index of the new candidate... */
    ind = i;
    if(inds) { ind = ivec_ref(inds,i); }

    /* Get the candidate track and it's time */
    Y  = track_array_ref(arr,ind);
    tb = track_time(Y);
    td = fabs(track_time(X) - tb);
    tm = (track_time(X) + track_time(Y))/2.0;

    prune = (tb > t_e) || (tb < t_s) || (td < 1e-10);

    /* Try EACH of the dimensions looking for prunning ops. */
    for(j=0;(j<T_NUM_DIMS)&&(prune==FALSE);j++) {
      
      thrsh = dyv_ref(thresh,j);
      if(j==T_TIME) { thrsh = -1.0; }

      if(thrsh > 1e-20) {
        
        /* Project the tracks forward/backward. */
        vala = t_tree_predict_value(X,j,tm);
        valb = t_tree_predict_value(Y,j,tm);
        diff = fabs(valb-vala);

        /* Handle wrap around in R */
        if(j==T_R) {
          while(diff > 12.0) { diff = fabs(diff - 24.0); }
        }

        /* Decide whether to prune. */
        prune = (diff > thrsh);

        /* check acceleration */
        if((prune==FALSE)&&(dyv_ref(accel,j) > 0.0)) {
          vala = t_tree_track_velocity(X,j);
          valb = t_tree_track_velocity(Y,j);
          acc  = fabs((valb - vala)/td);

          prune = (acc > dyv_ref(accel,j));
        }
      }
    }

    if(prune == FALSE) {
      add_to_ivec(res,ind);
    }
  }

  return res;
}


void t_tree_find_midpt_recurse(t_tree* tr, track_array* arr, track* X,
                               double t_s, double t_e, dyv* thresh, dyv* accel,
                               ivec* res) {
  ivec* temp;
  bool prune     = FALSE;
  double ts, te, tq, tqs, tqe;
  double x, v, amin, amax, xmin, xmax;
  double thrsh;
  double a,b,c,d;
  double dist;
  int i, j, old;

  old = ivec_size(res);

  /* Decide whether or not to prune: check time */
  ts    = JK_SIMPLE_MAX(t_s,t_lo_time(tr));
  te    = JK_SIMPLE_MIN(t_e,t_hi_time(tr));
  tq    = track_time(X);
  tqs   = (ts-tq);
  tqe   = (te-tq);
  prune = (ts > te) || ((te-ts <= 1e-10) && (fabs(ts-tq) < 1e-10));

  /* Try EACH of the dimensions looking for prunning ops. */
  for(j=0;(j<T_NUM_DIMS)&&(prune==FALSE);j++) {

    thrsh = dyv_ref(thresh,j);
    if(j==T_TIME) { thrsh = -1.0; }

    if(thrsh > 1e-20) {
      v = t_tree_track_velocity(X,j);
      x = t_tree_track_value(X,j);

      /* Find the bounds on acceleration. */
      a = (t_lo_vel_bound(tr,j) - v)/tqs;
      b = (t_hi_vel_bound(tr,j) - v)/tqs;
      c = (t_lo_vel_bound(tr,j) - v)/tqe;
      d = (t_hi_vel_bound(tr,j) - v)/tqe;
      amin = JK_SIMPLE_MIN(JK_SIMPLE_MIN(a,b),JK_SIMPLE_MIN(c,d));
      amax = JK_SIMPLE_MAX(JK_SIMPLE_MAX(a,b),JK_SIMPLE_MAX(c,d));

      if(dyv_ref(accel,j) > 0.0) {
        amin = JK_SIMPLE_MAX(amin,-dyv_ref(accel,j));
        amax = JK_SIMPLE_MIN(amax,dyv_ref(accel,j));
      }

      /* Find the bounds on the x values that can be reached */
      a = x + tqs*v + 0.5*tqs*tqs*amin;
      b = x + tqe*v + 0.5*tqe*tqe*amin;
      c = x + tqs*v + 0.5*tqs*tqs*amax;
      d = x + tqe*v + 0.5*tqe*tqe*amax;
      xmin = JK_SIMPLE_MIN(JK_SIMPLE_MIN(a,b),JK_SIMPLE_MIN(c,d));
      xmax = JK_SIMPLE_MAX(JK_SIMPLE_MAX(a,b),JK_SIMPLE_MAX(c,d));

      /* If the point is INSIDE the time range then allow it */
      /* to consider the bounds of the box it is in...       */
      if((tq >= ts)&&(tq <= te)) {
        xmin = JK_SIMPLE_MIN(xmin,t_lo_bound(tr,j));
        xmax = JK_SIMPLE_MAX(xmax,t_hi_bound(tr,j));
      }

      /* Determine pruning... */
      dist = fabs(JK_MID(xmin,xmax) - t_mid_bound(tr,j));
      if(j==T_R) { while(dist > 12.0) { dist = fabs(dist - 24.0); } }
      prune = (dist - JK_RAD(xmin,xmax) - t_rad_bound(tr,j) > thrsh);
    }
  } 


  /* If we cannot prune this branch... then recurse. */
  if(prune==FALSE) {
    if(t_is_leaf(tr)) {
      temp = mk_t_tree_find_midpt_slow(arr, t_tracks(tr), X, t_s, t_e, thresh, accel);
      for(i=0;i<ivec_size(temp);i++) { add_to_ivec(res,ivec_ref(temp,i)); }
      free_ivec(temp);
    } else {
      t_tree_find_midpt_recurse(t_left_child(tr),arr,X,t_s,t_e,thresh,accel,res);
      t_tree_find_midpt_recurse(t_right_child(tr),arr,X,t_s,t_e,thresh,accel,res);
    }
  }
}


ivec* mk_t_tree_find_midpt(t_tree* tr, track_array* arr, track* X,
                           double t_start, double t_end, dyv* thresh, dyv* accel) {
  ivec* res   = mk_ivec(0);

  t_tree_find_midpt_recurse(tr,arr,X,t_start,t_end,thresh,accel,res);

  return res;
}


/* ------------------------------------------------------------------ */
/* --- Do a search for all points that might hit a region of pspace.  */
/* ------------------------------------------------------------------ */

t_pbnds* mk_empty_t_pbnds(double t0) {
  t_pbnds* res = AM_MALLOC(t_pbnds);
  int i;

  res->t0 = t0;
  for(i=0;i<TB_NUM_DIMS;i++) {
    res->a_hi[i] = 0.0; res->a_lo[i] = 1.0;
    res->v_hi[i] = 0.0; res->v_lo[i] = 1.0;
    res->x_hi[i] = 0.0; res->x_lo[i] = 1.0;
  }

  return res;
}


t_pbnds* mk_copy_t_pbnds(t_pbnds* old) {
  t_pbnds* res = AM_MALLOC(t_pbnds);
  int i;

  for(i=0;i<TB_NUM_DIMS;i++) {
    res->x_hi[i] = old->x_hi[i]; 
    res->v_hi[i] = old->v_hi[i]; 
    res->a_hi[i] = old->a_hi[i]; 
    res->x_lo[i] = old->x_lo[i]; 
    res->v_lo[i] = old->v_lo[i]; 
    res->a_lo[i] = old->a_lo[i]; 
  }

  return res;
}


void free_t_pbnds(t_pbnds* old) {
  AM_FREE(old,t_pbnds);
}


double safe_t_pbnds_t0(t_pbnds* bnds) {
  return bnds->t0;
}

double safe_t_pbnds_lo_x(t_pbnds* bnds, int dim) {
  my_assert((dim >= 0)&&(dim < TB_NUM_DIMS));
  return t_pbnds_lo_x(bnds,dim);
}

double safe_t_pbnds_lo_v(t_pbnds* bnds, int dim) {
  my_assert((dim >= 0)&&(dim < TB_NUM_DIMS));
  return t_pbnds_lo_v(bnds,dim);
}

double safe_t_pbnds_lo_a(t_pbnds* bnds, int dim) {
  my_assert((dim >= 0)&&(dim < TB_NUM_DIMS));
  return t_pbnds_lo_a(bnds,dim);
}

double safe_t_pbnds_hi_x(t_pbnds* bnds, int dim) {
  my_assert((dim >= 0)&&(dim < TB_NUM_DIMS));
  return t_pbnds_hi_x(bnds,dim);
}

double safe_t_pbnds_hi_v(t_pbnds* bnds, int dim) {
  my_assert((dim >= 0)&&(dim < TB_NUM_DIMS));
  return t_pbnds_hi_v(bnds,dim);
}

double safe_t_pbnds_hi_a(t_pbnds* bnds, int dim) {
  my_assert((dim >= 0)&&(dim < TB_NUM_DIMS));
  return t_pbnds_hi_a(bnds,dim);
}

double safe_t_pbnds_mid_a(t_pbnds* bnds, int dim)  {
  my_assert((dim >= 0)&&(dim < TB_NUM_DIMS));
  return (t_pbnds_hi_a(bnds,dim)+t_pbnds_lo_a(bnds,dim))/2.0;
}

double safe_t_pbnds_mid_v(t_pbnds* bnds, int dim)  {
  my_assert((dim >= 0)&&(dim < TB_NUM_DIMS));
  return (t_pbnds_hi_v(bnds,dim)+t_pbnds_lo_v(bnds,dim))/2.0;
}

double safe_t_pbnds_mid_x(t_pbnds* bnds, int dim)  {
  my_assert((dim >= 0)&&(dim < TB_NUM_DIMS));
  return (t_pbnds_hi_x(bnds,dim)+t_pbnds_lo_x(bnds,dim))/2.0;
}

double safe_t_pbnds_rad_a(t_pbnds* bnds, int dim)  {
  my_assert((dim >= 0)&&(dim < TB_NUM_DIMS));
  return (t_pbnds_hi_a(bnds,dim)-t_pbnds_lo_a(bnds,dim))/2.0;
}

double safe_t_pbnds_rad_v(t_pbnds* bnds, int dim)  {
  my_assert((dim >= 0)&&(dim < TB_NUM_DIMS));
  return (t_pbnds_hi_v(bnds,dim)-t_pbnds_lo_v(bnds,dim))/2.0;
}

double safe_t_pbnds_rad_x(t_pbnds* bnds, int dim)  {
  my_assert((dim >= 0)&&(dim < TB_NUM_DIMS));
  return (t_pbnds_hi_x(bnds,dim)-t_pbnds_lo_x(bnds,dim))/2.0;
}

double safe_t_pbnds_a_valid(t_pbnds* bnds, int dim) {
  my_assert((dim >= 0)&&(dim < TB_NUM_DIMS));
  return (t_pbnds_rad_a(bnds,dim) > -1e-20);
}

double safe_t_pbnds_v_valid(t_pbnds* bnds, int dim) {
  my_assert((dim >= 0)&&(dim < TB_NUM_DIMS));
  return (t_pbnds_rad_v(bnds,dim) > -1e-20);
}

double safe_t_pbnds_x_valid(t_pbnds* bnds, int dim) {
  my_assert((dim >= 0)&&(dim < TB_NUM_DIMS));
  return (t_pbnds_rad_x(bnds,dim) > -1e-20);
}

double t_pbnds_track_x_ref(track* X, int dim) {
  double res = 0.0;

  switch(dim) {
  case TB_R: res = track_RA(X);  break;
  case TB_D: res = track_DEC(X); break;
  }

  return res;
}

double t_pbnds_track_v_ref(track* X, int dim) {
  double res = 0.0;

  switch(dim) {
  case TB_R: res = track_vRA(X);  break;
  case TB_D: res = track_vDEC(X); break;
  }

  return res;
}
