/*
   File:        rdt_tree.c
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

#include "rdt_tree.h"

#define RDT_MID(min,max)    (((min) + (max))/2.0)
#define RDT_RAD(min,max)    ((max) - RDT_MID(min,max))
#define RDT_SIMPLE_MIN(a,b) (((a) < (b)) ? (a) : (b))
#define RDT_SIMPLE_MAX(a,b) (((a) > (b)) ? (a) : (b))

int did_check;

/* --- Useful Helper Functions -------------------------- */

double rdt_tree_radius_given_anchor(simple_obs_array* arr, ivec* inds,
                                    double ra, double dec) {
  simple_obs* X;
  double radius = 0.0;
  double dist   = 0.0;
  int i;

  for(i=0;i<ivec_size(inds);i++) {
    X    = simple_obs_array_ref(arr,ivec_ref(inds,i));
    dist = angular_distance_RADEC(simple_obs_RA(X),ra,simple_obs_DEC(X),dec);
    if(dist > radius) { radius = dist; }
  }

  return radius;
}


void rdt_tree_fill_bounds(rdt_tree* tr, simple_obs_array* arr, ivec* inds) {
  simple_obs* X;
  double xv[RDT_DIM];
  int i, j;

  /* Set the initial values. */
  if(ivec_size(inds) > 0) {
    X = simple_obs_array_ref(arr,ivec_ref(inds,0));
    xv[RDT_T] = simple_obs_time(X);
    xv[RDT_D] = simple_obs_DEC(X);
    xv[RDT_R] = simple_obs_RA(X);
    xv[RDT_B] = simple_obs_brightness(X);

    for(j=0;j<RDT_DIM;j++) {
      tr->lo[j] = xv[j];
      tr->hi[j] = xv[j];
    }
  }

  /* Set the initial values. */
  for(i=1;i<ivec_size(inds);i++) {
    X = simple_obs_array_ref(arr,ivec_ref(inds,i));
    xv[RDT_T] = simple_obs_time(X);
    xv[RDT_D] = simple_obs_DEC(X);
    xv[RDT_R] = simple_obs_RA(X);
    xv[RDT_B] = simple_obs_brightness(X);

    for(j=0;j<RDT_DIM;j++) {
      if(xv[j] > tr->hi[j]) { tr->hi[j] = xv[j]; }
      if(xv[j] < tr->lo[j]) { tr->lo[j] = xv[j]; }
    }
  }

  if((tr->hi[RDT_R] > 22.0)&&(tr->lo[RDT_R] < 2.0)) {
    tr->hi[RDT_R] = 24.0;
    tr->lo[RDT_R] =  0.0;
  }

  /* Calculate the midpoints and radii */
  for(j=0;j<RDT_DIM;j++) {
    tr->mid[j] = (tr->hi[j] + tr->lo[j])/2.0;
    tr->rad[j] = (tr->hi[j] - tr->mid[j]);
  }
  tr->radius = rdt_tree_radius_given_anchor(arr,inds,tr->mid[RDT_R],tr->mid[RDT_D]);
}


/* --- Tree Memory Functions -------------------------- */

rdt_tree* mk_empty_rdt_tree() {
  rdt_tree* res = AM_MALLOC(rdt_tree);
  int i;

  res->num_points = 0;

  for(i=0;i<RDT_DIM;i++) {
    res->hi[i]  = 0.0;
    res->lo[i]  = 0.0;
    res->mid[i] = 0.0;
    res->rad[i] = 0.0;
  }
  res->radius = 0.0;

  res->right = NULL;
  res->left  = NULL;
  res->pts   = NULL;

  return res;
}


void free_rdt_tree(rdt_tree* old) {
  if(old->left)  { free_rdt_tree(old->left);  }
  if(old->right) { free_rdt_tree(old->right); }
  if(old->pts)   { free_ivec(old->pts); }

  AM_FREE(old,rdt_tree);
}



rdt_tree* mk_rdt_tree_recurse(simple_obs_array* obs, ivec* inds, 
                              dyv* widths, int max_leaf_pts) {
  simple_obs* X;
  rdt_tree* res;
  ivec *left, *right;
  double width;
  double val;
  double sv = 0.0;
  double sw = 0.0;
  int    sd = 0;
  int i;

  /* Set up the node */
  res  = mk_empty_rdt_tree();
  rdt_tree_fill_bounds(res, obs, inds);
  res->num_points = ivec_size(inds);
  width = (res->rad[RDT_T] + res->rad[RDT_R] + res->rad[RDT_D]);

  /* Determine if this node will be a leaf or internal */
  if((ivec_size(inds) < max_leaf_pts)||(width < 1e-10)) {
    res->pts = mk_copy_ivec(inds);
  } else {

    /* Pick the widest dimension and split it. */
    for(i=0;i<RDT_DIM;i++) {
      val = res->rad[i] / dyv_ref(widths,i);
      if(i == RDT_R) {
        val *= (15.0*fabs(cos(res->mid[RDT_D]*DEG_TO_RAD)));
        val *= DEG_TO_RAD;
      }
      if(i == RDT_D) {
        val *= DEG_TO_RAD;
      }

      if((i==0)||(val > sw)) {
        sw = val;
        sd = i;
        sv = res->mid[i];
      }
    }

    /* Actually divide up the points. */
    left = mk_ivec(0);
    right = mk_ivec(0);

    for(i=0;i<ivec_size(inds);i++) {
      X = simple_obs_array_ref(obs,ivec_ref(inds,i));
      val = 0.0;

      switch(sd) {
      case RDT_T:  val = simple_obs_time(X); break;
      case RDT_R:  val = simple_obs_RA(X);   break;
      case RDT_D:  val = simple_obs_DEC(X);  break;
      case RDT_B:  val = simple_obs_brightness(X);  break;
      }

      if(val < sv) {
        add_to_ivec(left,ivec_ref(inds,i));
      } else {
        add_to_ivec(right,ivec_ref(inds,i));
      }
    }

    /* Build the left and right sub-trees */
    res->left  = mk_rdt_tree_recurse(obs, left,  widths, max_leaf_pts);
    res->right = mk_rdt_tree_recurse(obs, right, widths, max_leaf_pts);

    free_ivec(left);
    free_ivec(right);
  }

  return res;
}


/* use_inds - is the indices to use (NULL to use ALL observations). */
/* force_t  - forces us to split on time first.                     */
rdt_tree* mk_rdt_tree(simple_obs_array* obs, ivec* use_inds, 
                      bool force_t, int max_leaf_pts) {
  rdt_tree *res;
  ivec    *inds;
  dyv     *width;

  /* Store all the indices for the tree. */
  if(use_inds != NULL) {
    inds = mk_copy_ivec(use_inds);
  } else {
    inds  = mk_sequence_ivec(0,simple_obs_array_size(obs));
  }

  /* Compute the bounds for the tree */
  res  = mk_empty_rdt_tree();
  rdt_tree_fill_bounds(res, obs, inds);
  width = mk_zero_dyv(RDT_DIM);
  
  dyv_set(width,RDT_B,1e10);       /* Ignore brightness for splitting */

  dyv_set(width,RDT_T,res->rad[RDT_T]);
  if(res->rad[RDT_T] < 1e-10) { dyv_set(width,RDT_T,1e-10); }
  if(force_t) { dyv_set(width,RDT_T,1e-10); }

  if(res->radius < 1e-10) { res->radius = 1e-10; }
  dyv_set(width,RDT_R,res->radius);
  dyv_set(width,RDT_D,res->radius);

  free_rdt_tree(res);

  /* Build the tree. */
  res  = mk_rdt_tree_recurse(obs, inds, width, max_leaf_pts);

  /* Free the used memory */
  free_dyv(width);
  free_ivec(inds);

  return res;
}


/* use_inds - is the indices to use (NULL to use ALL observations).    */
/* wt       - relative weighting to time (if > 0.0).                   */
/* wid      - if > 0.0, this is the width to initially split time to   */
/*            (i.e. split time to wid before splitting anything else). */
/* If both wt and wid are >0.0, wid is used.                           */
rdt_tree* mk_rdt_tree_wt(simple_obs_array* obs, ivec* use_inds, 
                         double wt, double wid, int max_leaf_pts) {
  rdt_tree *res;
  ivec    *inds;
  dyv     *width;

  /* Store all the indices for the tree. */
  if(use_inds != NULL) {
    inds = mk_copy_ivec(use_inds);
  } else {
    inds  = mk_sequence_ivec(0,simple_obs_array_size(obs));
  }

  /* Compute the bounds for the tree */
  res  = mk_empty_rdt_tree();
  rdt_tree_fill_bounds(res, obs, inds);
  width = mk_zero_dyv(RDT_DIM);
  
  dyv_set(width,RDT_B,1e10);       /* Ignore brightness for splitting */

  dyv_set(width,RDT_T,res->rad[RDT_T]);
  if(res->rad[RDT_T] < 1e-10) { dyv_set(width,RDT_T,1e-10); }
  if(wid > 0.0) {
    dyv_set(width,RDT_T,wid);
  } else {
    if(wt > 0.0) {
      dyv_set(width,RDT_T,dyv_ref(width,RDT_T)/wt);
    }
  }

  if(res->radius < 1e-10) { res->radius = 1e-10; }
  dyv_set(width,RDT_R,res->radius);
  dyv_set(width,RDT_D,res->radius);

  free_rdt_tree(res);

  /* Build the tree. */
  res  = mk_rdt_tree_recurse(obs, inds, width, max_leaf_pts);

  /* Free the used memory */
  free_dyv(width);
  free_ivec(inds);

  return res;
}




/* --------------------------------------------------------------------- */
/* --- Getter/Setter Functions ----------------------------------------- */
/* --------------------------------------------------------------------- */

int safe_rdt_tree_N(rdt_tree* tr) { return tr->num_points; }
int safe_rdt_tree_num_points(rdt_tree* tr) { return tr->num_points; }

bool safe_rdt_tree_is_leaf(rdt_tree* tr) { return (tr->left == NULL); }
ivec* safe_rdt_tree_pts(rdt_tree* tr) { return tr->pts; }

rdt_tree* safe_rdt_tree_right_child(rdt_tree* tr) { return tr->left; }
rdt_tree* safe_rdt_tree_left_child(rdt_tree* tr) { return tr->right; }

double safe_rdt_tree_RA(rdt_tree* p)     { return p->mid[RDT_R]; }
double safe_rdt_tree_DEC(rdt_tree* p)    { return p->mid[RDT_D]; }
double safe_rdt_tree_radius(rdt_tree* p) { return p->radius; }
double safe_rdt_tree_time(rdt_tree* p)   { return p->mid[RDT_T]; }

double safe_rdt_tree_lo_time(rdt_tree* p) { return p->lo[RDT_T]; }
double safe_rdt_tree_hi_time(rdt_tree* p) { return p->hi[RDT_T]; }
double safe_rdt_tree_mid_time(rdt_tree* p) { return p->mid[RDT_T]; }
double safe_rdt_tree_rad_time(rdt_tree* p) { return p->rad[RDT_T]; }

double safe_rdt_tree_lo_RA(rdt_tree* p) { return p->lo[RDT_R]; }
double safe_rdt_tree_hi_RA(rdt_tree* p) { return p->hi[RDT_R]; }
double safe_rdt_tree_mid_RA(rdt_tree* p) { return p->mid[RDT_R]; }
double safe_rdt_tree_rad_RA(rdt_tree* p) { return p->rad[RDT_R]; }

double safe_rdt_tree_lo_DEC(rdt_tree* p) { return p->lo[RDT_D]; }
double safe_rdt_tree_hi_DEC(rdt_tree* p) { return p->hi[RDT_D]; }
double safe_rdt_tree_mid_DEC(rdt_tree* p) { return p->mid[RDT_D]; }
double safe_rdt_tree_rad_DEC(rdt_tree* p) { return p->rad[RDT_D]; }

double safe_rdt_tree_lo_bright(rdt_tree* p) { return p->lo[RDT_B]; }
double safe_rdt_tree_hi_bright(rdt_tree* p) { return p->hi[RDT_B]; }
double safe_rdt_tree_mid_bright(rdt_tree* p) { return p->mid[RDT_B]; }
double safe_rdt_tree_rad_bright(rdt_tree* p) { return p->rad[RDT_B]; }


/* --- Simple I/O Functions ------------------------------------ */

void fprintf_rdt_tree_pts_recurse(FILE* f, rdt_tree* tr, int depth) {
  ivec* inds;
  int i;

  /* Do the correct indenting */
  for(i=0;i<depth;i++) { fprintf(f,"-"); }

  if(rdt_tree_is_leaf(tr)) {
    inds = rdt_tree_pts(tr);
    fprintf(f," LEAF:");
    for(i=0;i<ivec_size(inds);i++) { fprintf(f,"%i ",ivec_ref(inds,i)); }
    fprintf(f,"\n");
  } else {
    fprintf(f," INTERNAL\n");
    fprintf_rdt_tree_pts_recurse(f,rdt_tree_left_child(tr),depth+1);
    fprintf_rdt_tree_pts_recurse(f,rdt_tree_right_child(tr),depth+1);
  }
}


void fprintf_rdt_tree_pts(FILE* f, rdt_tree* tr) {
  fprintf_rdt_tree_pts_recurse(f,tr,1);
}


void fprintf_rdt_tree_recurse(FILE* f, rdt_tree* tr, int depth) {
  ivec* inds;
  int i;

  /* Do the correct indenting */
  for(i=0;i<depth;i++) { fprintf(f,"-"); }

  if(rdt_tree_is_leaf(tr)) {
    inds = rdt_tree_pts(tr);
    fprintf_rdt_tree_node(f,"LEAF: ",tr," ");
    for(i=0;i<ivec_size(inds);i++) { fprintf(f,"%i ",ivec_ref(inds,i)); }
    fprintf(f,"\n");
  } else {
    fprintf_rdt_tree_node(f,"INT:  ",tr,"\n");
    fprintf_rdt_tree_recurse(f,rdt_tree_left_child(tr),depth+1);
    fprintf_rdt_tree_recurse(f,rdt_tree_right_child(tr),depth+1);
  }
}


void fprintf_rdt_tree(FILE* f, rdt_tree* tr) {
  fprintf_rdt_tree_recurse(f,tr,1);
}


void fprintf_rdt_tree_node(FILE* f, char* pre, rdt_tree* tr, char* post) {
  fprintf(f,pre);
  fprintf(f,"T=[%10.6f,%10.6f]  (%12.8f,%12.8f)  RAD=%12.8f  F=[%12.8f,%12.8f]",
          rdt_tree_lo_time(tr),rdt_tree_hi_time(tr),
          rdt_tree_RA(tr),rdt_tree_DEC(tr),rdt_tree_radius(tr),
          rdt_tree_lo_bright(tr),rdt_tree_hi_bright(tr));
  fprintf(f,post);
}


void fprintf_rdt_tree_pts_list(FILE* f, rdt_tree* tr) {
  ivec* inds;
  int i;

  if(rdt_tree_is_leaf(tr)) {
    inds = rdt_tree_pts(tr);
    for(i=0;i<ivec_size(inds);i++) { fprintf(f,"%i ",ivec_ref(inds,i)); }
  } else {
    fprintf_rdt_tree_pts_list(f,rdt_tree_left_child(tr));
    fprintf_rdt_tree_pts_list(f,rdt_tree_right_child(tr));
  }
}




/* --------------------------------------------------------------------- */
/* --- Query Functions ------------------------------------------------- */
/* --------------------------------------------------------------------- */

int rdt_tree_range_count_exh(simple_obs_array* arr, ivec* inds,
                             simple_obs* X, double ts, double te,
                             double thresh) {
  simple_obs* Y;
  double dist;
  int count = 0;
  int i;

  for(i=0;i<ivec_size(inds);i++) {
    Y  = simple_obs_array_ref(arr,ivec_ref(inds,i));
    
    if((ts <= simple_obs_time(Y))&&(te >= simple_obs_time(Y))) {
      dist = angular_distance_RADEC(simple_obs_RA(X),simple_obs_RA(Y),
                                    simple_obs_DEC(X),simple_obs_DEC(Y));
      if(dist <= thresh) { count++; }
    }
  }

  return count;
}


int rdt_tree_range_count(rdt_tree* tr, simple_obs_array* arr,
                         simple_obs* X, double ts, double te, double thresh) {
  double dist;
  int count = 0;
  bool all = FALSE;
  bool valid;

  /* Make sure the time bounds overlap. */
  valid = (ts <= rdt_tree_hi_time(tr)+1e-10);
  valid = valid && (te >= rdt_tree_lo_time(tr)-1e-10);

  /* Check the proximity in declination. */
  valid = valid && (fabs(simple_obs_DEC(X)-rdt_tree_mid_DEC(tr)) -
                    rdt_tree_rad_DEC(tr) <= thresh * RAD_TO_DEG);

  if(valid == TRUE) {
    dist  = angular_distance_RADEC(simple_obs_RA(X),rdt_tree_RA(tr),
                                   simple_obs_DEC(X),rdt_tree_DEC(tr));
    valid = (dist - rdt_tree_radius(tr) <= thresh);

    /* Check if the threshold contains ALL space and all time. */
    all   = (dist + rdt_tree_radius(tr) <= thresh);
    all   = all && (rdt_tree_hi_time(tr) <= te + 1e-15);
    all   = all && (rdt_tree_lo_time(tr) >= ts - 1e-15);
  }

  if(valid == TRUE) {
    if(all == TRUE) {
      count = rdt_tree_num_points(tr);
    } else {
      if(rdt_tree_is_leaf(tr) == TRUE) {
        count = rdt_tree_range_count_exh(arr,rdt_tree_pts(tr),X,ts,te,thresh);
      } else {
        count = rdt_tree_range_count(rdt_tree_right_child(tr),
                                     arr,X,ts,te,thresh);
        count += rdt_tree_range_count(rdt_tree_left_child(tr),
                                      arr,X,ts,te,thresh);
      }
    }
  }

  return count;
}


/* --- Nearest Neighbor Query -------------------------------- */

void rdt_tree_NN_exh(simple_obs_array* arr, ivec* inds,
                     simple_obs* X, double ts, double te,
                     int* best_ind, double* best_dist) {
  simple_obs* Y;
  double dist;
  int i;

  for(i=0;i<ivec_size(inds);i++) {
    Y  = simple_obs_array_ref(arr,ivec_ref(inds,i));
    
    if((ts <= simple_obs_time(Y))&&(te >= simple_obs_time(Y))) {
      dist = angular_distance_RADEC(simple_obs_RA(X),simple_obs_RA(Y),
                                    simple_obs_DEC(X),simple_obs_DEC(Y));

      if(dist < best_dist[0]) {
        best_ind[0]  = ivec_ref(inds,i);
        best_dist[0] = dist; 
      }
    }
  }

}


/* Finds the nearest neighbor to X within the time range [ts, te] */
/* such that the neighbor is within distance thresh.  If no such  */
/* point exists, returns -1.  To do a pure nearest neighbor (no   */
/* threshold), use thresh <= -1.0.                                */
void rdt_tree_NN_recurse(rdt_tree* tr, simple_obs_array* arr,
                         simple_obs* X, double ts, double te, 
                         int* best_ind, double* thresh) {
  double distR, distL;
  rdt_tree* R_tr;
  rdt_tree* L_tr;
  bool valid;

  /* Make sure the time bounds overlap. */
  valid = (ts <= rdt_tree_hi_time(tr)+1e-10);
  valid = valid && (te >= rdt_tree_lo_time(tr)-1e-10);

  if(valid == TRUE) {
    if(rdt_tree_is_leaf(tr) == TRUE) {
      rdt_tree_NN_exh(arr,rdt_tree_pts(tr),X,ts,te,best_ind,thresh);
    } else {
      R_tr = rdt_tree_right_child(tr);
      L_tr = rdt_tree_left_child(tr);

      /* Make the distances immediately invalid if the time ranges to not overlap */
      if((ts > rdt_tree_hi_time(R_tr)+1e-10)||(te < rdt_tree_lo_time(R_tr)-1e-10)) {
        distR = thresh[0] + rdt_tree_radius(R_tr) + 2.0;
      } else {
        distR = angular_distance_RADEC(simple_obs_RA(X),rdt_tree_RA(R_tr),
                                       simple_obs_DEC(X),rdt_tree_DEC(R_tr));
      }
      if((ts > rdt_tree_hi_time(L_tr)+1e-10)||(te < rdt_tree_lo_time(L_tr)-1e-10)) {
        distL = thresh[0] + rdt_tree_radius(L_tr) + 2.0;
      } else {
        distL = angular_distance_RADEC(simple_obs_RA(X),rdt_tree_RA(L_tr),
                                       simple_obs_DEC(X),rdt_tree_DEC(L_tr));
      }

      /* Descend the "closer" neighbor first, but be  */
      /* careful NOT to explore an infeasible branch. */
      if(distR < distL) {
        if(distR - rdt_tree_radius(R_tr) <= thresh[0]) {
          rdt_tree_NN_recurse(R_tr,arr,X,ts,te,best_ind,thresh);
        }
        if(distL - rdt_tree_radius(L_tr) <= thresh[0]) {
          rdt_tree_NN_recurse(L_tr,arr,X,ts,te,best_ind,thresh);
        }
      } else {
        if(distL - rdt_tree_radius(L_tr) <= thresh[0]) {
          rdt_tree_NN_recurse(L_tr,arr,X,ts,te,best_ind,thresh);
        }
        if(distR - rdt_tree_radius(R_tr) <= thresh[0]) {
          rdt_tree_NN_recurse(R_tr,arr,X,ts,te,best_ind,thresh);
        }
      }
    }
  }

}


/* Finds the nearest neighbor to X within the time range [ts, te] */
/* such that the neighbor is within distance thresh.  If no such  */
/* point exists, returns -1.  To do a pure nearest neighbor (no   */
/* threshold), use thresh <= -1.0.                                */
int rdt_tree_NN(rdt_tree* tr, simple_obs_array* arr,
                simple_obs* X, double ts, double te, 
                double thresh) {
  double best_dist = thresh;
  int    best_ind  = -1;

  /* If no threshold use 2*PI */
  if(best_dist <= -0.0001) { best_dist = 2.0*PI+1.0; }

  /* Do the search */
  rdt_tree_NN_recurse(tr,arr,X,ts,te,&best_ind,&best_dist);

  return best_ind;
}


/* --- Range Search Queries -------------------------------- */


void rdt_tree_range_search_exh(simple_obs_array* arr, ivec* inds,
                               simple_obs* X, double ts, double te,
                               double thresh, ivec* res) {
  simple_obs* Y;
  double dist;
  int i;

  for(i=0;i<ivec_size(inds);i++) {
    Y  = simple_obs_array_ref(arr,ivec_ref(inds,i));

    if((ts <= simple_obs_time(Y))&&(te >= simple_obs_time(Y))) {
      dist = angular_distance_RADEC(simple_obs_RA(X),simple_obs_RA(Y),
                                    simple_obs_DEC(X),simple_obs_DEC(Y));
      if(dist <= thresh) {
        add_to_ivec(res,ivec_ref(inds,i)); 
      }

    }
  }
}


int rdt_tree_range_search_recurse(rdt_tree* tr, simple_obs_array* arr,
                                  simple_obs* X, double ts, double te,
                                  double thresh, ivec* res) {
  double dist, ddist;
  int pruned = 0;
  bool valid;

  /* Make sure the time bounds overlap. */
  valid = (ts <= rdt_tree_hi_time(tr)+1e-10);
  valid = valid && (te >= rdt_tree_lo_time(tr)-1e-10);

  if(valid == TRUE) {

    /* Try just the distance in declination... */
    ddist = fabs(simple_obs_DEC(X)-rdt_tree_mid_DEC(tr))*DEG_TO_RAD;
    valid = (ddist <= rdt_tree_rad_DEC(tr)*DEG_TO_RAD + thresh);

    if(valid == TRUE) {
      dist  = angular_distance_RADEC(simple_obs_RA(X),rdt_tree_RA(tr),
                                   simple_obs_DEC(X),rdt_tree_DEC(tr));
      valid = (dist <= rdt_tree_radius(tr) + thresh);
    }
  }

  if(valid == TRUE) {
    if(rdt_tree_is_leaf(tr) == TRUE) {
      rdt_tree_range_search_exh(arr,rdt_tree_pts(tr),X,ts,te,thresh,res);
    } else {
      pruned =  rdt_tree_range_search_recurse(rdt_tree_right_child(tr),arr,X,
                                              ts,te,thresh,res);
      pruned += rdt_tree_range_search_recurse(rdt_tree_left_child(tr),arr,X,
                                              ts,te,thresh,res);
    }
  } else {
    pruned = rdt_tree_num_points(tr);
  }

  return pruned;
}


ivec* mk_rdt_tree_range_search(rdt_tree* tr, simple_obs_array* arr, 
                               simple_obs* X, double ts, double te, 
                               double thresh) {
  ivec* res = mk_ivec(0);
  int pruned;

  pruned = rdt_tree_range_search_recurse(tr,arr,X,ts,te,thresh,res);

  return res;
}


/* --- Moving Point Queries -------------------------------- */


void rdt_tree_moving_pt_query_exh(simple_obs_array* arr, ivec* inds,
                                  simple_obs* X, double ts, double te,
                                  double minv, double maxv, double thresh, 
                                  ivec* res) {
  simple_obs* Y;
  double dist;
  double dt;
  int i;

  for(i=0;i<ivec_size(inds);i++) {
    Y  = simple_obs_array_ref(arr,ivec_ref(inds,i));
    did_check++;

    if((ts <= simple_obs_time(Y))&&(te >= simple_obs_time(Y))) {
      dist = angular_distance_RADEC(simple_obs_RA(X),simple_obs_RA(Y),
                                    simple_obs_DEC(X),simple_obs_DEC(Y));
      dt   = fabs(simple_obs_time(Y)-simple_obs_time(X));

      if((dist <= (maxv*dt) + thresh)&&(dist >= (minv*dt - thresh))) {
        add_to_ivec(res,ivec_ref(inds,i)); 
      }

    }
  }
}


int rdt_tree_moving_pt_query_recurse(rdt_tree* tr, simple_obs_array* arr,
                                     simple_obs* X, double ts, double te,
                                     double minv, double maxv, double thresh,
                                     ivec* res) {
  double dtmax = 0.0;
  double dtmin = 0.0;
  double dts, dte;
  double dist, ddist;
  int pruned = 0;
  bool valid;

  /* Make sure the time bounds overlap. */
  valid = (ts <= rdt_tree_hi_time(tr)+1e-10);
  valid = valid && (te >= rdt_tree_lo_time(tr)-1e-10);
  did_check++;

  if(valid == TRUE) {

    /* Find the maximum and minimum times for movement. */
    if(ts < rdt_tree_lo_time(tr)) { ts = rdt_tree_lo_time(tr); }
    if(te > rdt_tree_hi_time(tr)) { te = rdt_tree_hi_time(tr); }

    dts = fabs(simple_obs_time(X)-ts);   /* Time gap from interval start */
    dte = fabs(simple_obs_time(X)-te);   /* and interval end.            */
  
    dtmax = dts;
    dtmin = dts;
    if(dtmax < dte) { dtmax = dte; }
    if(dtmin > dte) { dtmin = dte; }

    /* Try just the distance in declination... */
    ddist = fabs(simple_obs_DEC(X)-rdt_tree_mid_DEC(tr))*DEG_TO_RAD;
    valid = (ddist <= rdt_tree_rad_DEC(tr)*DEG_TO_RAD + (maxv*dtmax) + thresh);

    if(valid == TRUE) {
      dist  = angular_distance_RADEC(simple_obs_RA(X),rdt_tree_RA(tr),
                                   simple_obs_DEC(X),rdt_tree_DEC(tr));
      valid = (dist <= rdt_tree_radius(tr) + (maxv*dtmax) + thresh);

      /* Note MAY be a problem if the obs time is within the */
      /* node's time.                                        */
      valid = valid && (dist >= (minv*dtmin) - thresh - rdt_tree_radius(tr));
    }
  }

  if(valid == TRUE) {
    if(rdt_tree_is_leaf(tr) == TRUE) {
      rdt_tree_moving_pt_query_exh(arr,rdt_tree_pts(tr),X,ts,te,minv,maxv,
                                   thresh,res);
    } else {
      pruned =  rdt_tree_moving_pt_query_recurse(rdt_tree_right_child(tr),arr,X,
                                                 ts,te,minv,maxv,thresh,res);
      pruned += rdt_tree_moving_pt_query_recurse(rdt_tree_left_child(tr),arr,X,
                                                 ts,te,minv,maxv,thresh,res);
    }
  } else {
    pruned = rdt_tree_num_points(tr);
  }

  return pruned;
}


/* Find all observations occurring between times ts and te */
/* such that if X was allowed to move upto maxv then it    */
/* could endup within distance of thresh of the point.     */
/* Both maxv and thresh are given in radians.              */
ivec* mk_rdt_tree_moving_pt_query(rdt_tree* tr, simple_obs_array* arr, 
                                  simple_obs* X, double ts, double te, 
                                  double minv, double maxv, double thresh) {
  ivec* res = mk_ivec(0);
  int pruned;

  if((simple_obs_time(X) >= ts)&&(simple_obs_time(X) <= te)) {
    printf("Warning! Code MIGHT have issues if obs time is inside\n");
    printf("the node's time %f is in [%f,%f]\n",simple_obs_time(X),ts,te);
    wait_for_key();
  }

  did_check = 0;
  pruned = rdt_tree_moving_pt_query_recurse(tr,arr,X,ts,te,minv,maxv,thresh,res);

  return res;
}




/* ------- Line Segment Based Queries ---------------------------------- */

/* Threshold is specified in radians. */
void rdt_tree_near_line_seg_brute(simple_obs_array* arr, ivec* inds,
                                  dym* segs, int ts, int te, 
                                  double thresh, ivec* res) {
  simple_obs* X;
  double tx, r, d, a, dist;
  int N = ivec_size(inds);
  int i, t;
  bool t_mtch;

  /* Test each observation against the line segments. */
  for(i=0;i<N;i++) {
    X  = simple_obs_array_ref(arr,ivec_ref(inds,i));
    tx = simple_obs_time(X);
    t_mtch = FALSE;

    /* Find the correct matching time... */
    for(t=ts;(t < te)&&(t_mtch==FALSE);t++) {
      t_mtch = (tx >= dym_ref(segs,t,0))&&(tx <= dym_ref(segs,t+1,0));
    }
    t--;

    /* If there was a matching time use the end points  */
    /* to predict the line's position and calculate the */
    /* distance to the observation.                     */
    if(t_mtch == TRUE) {
      a = dym_ref(segs,t+1,0) - dym_ref(segs,t,0);
      if(a < 1e-10) { a = 1e-10; }
      a = (tx - dym_ref(segs,t,0))/a;

      r = (1.0-a)*dym_ref(segs,t,1)+a*dym_ref(segs,t+1,1);
      d = (1.0-a)*dym_ref(segs,t,2)+a*dym_ref(segs,t+1,2);

      dist = angular_distance_RADEC(simple_obs_RA(X),r,simple_obs_DEC(X),d);
      if(dist < thresh) {
        add_to_ivec(res,ivec_ref(inds,i));
      }
    }
  }

}


/* Uses an AUGMENTED matrix with rows:  */
/* [time_i ra_i dec_i x_i y_i z_i]      */
void rdt_tree_near_line_seg_recurse(rdt_tree* tr, simple_obs_array* arr,
                                    dym* segs, int ts, int te, 
                                    double thresh, ivec* res) {
  bool canprune = FALSE;
  bool matches  = FALSE;
  double xs, xe, ys, ye, zs, ze, a;
  double rn, dn, xn, yn, zn;
  double rp, dp, dist;
  double amin, amax, abot;
  int N = dym_rows(segs);
  int t;

  /* Check pruning and update ts, te */
  if(dym_ref(segs,ts,0) > rdt_tree_hi_time(tr)) { canprune = TRUE; }
  while((ts < N)&&(dym_ref(segs,ts,0) < rdt_tree_lo_time(tr))) { ts++; }
  if(ts > 0) { ts--; }

  if(dym_ref(segs,te,0) < rdt_tree_lo_time(tr)) { canprune = TRUE; }
  while((te >= 0)&&(dym_ref(segs,te,0) > rdt_tree_hi_time(tr))) { te--; }
  if(te < dym_rows(segs)-1) { te++; }

  /* Scan through the segments looking for an intersection... */
  if(canprune==FALSE) {

    /* Compute the rectangular coordinates of the anchor. */
    rn = rdt_tree_RA(tr);
    dn = rdt_tree_DEC(tr);
    xn = cos(rn*15.0*DEG_TO_RAD) * cos(dn*DEG_TO_RAD);
    yn = sin(rn*15.0*DEG_TO_RAD) * cos(dn*DEG_TO_RAD);
    zn = sin(dn*DEG_TO_RAD);

    for(t=ts;(t < te)&&(matches==FALSE);t++) {  

      /* Set the bounds on time to make pruning tight. */
      amin = (rdt_tree_lo_time(tr)-dym_ref(segs,t,0));
      amin = amin/(dym_ref(segs,t+1,0)-dym_ref(segs,t,0));
      amax = (rdt_tree_hi_time(tr)-dym_ref(segs,t,0));
      amax = amax/(dym_ref(segs,t+1,0)-dym_ref(segs,t,0));
      if(amax > 1.0) { amax = 1.0; }
      if(amin < 0.0) { amin = 0.0; }
      
      xs = dym_ref(segs,t,3); xe = dym_ref(segs,t+1,3);
      ys = dym_ref(segs,t,4); ye = dym_ref(segs,t+1,4);
      zs = dym_ref(segs,t,5); ze = dym_ref(segs,t+1,5);

      /* Calculate where along the arc the 'closest' point is. */
      /* Watch for stationary segments.                        */
      abot = (xs*xs-2.0*xs*xe+xe*xe+ys*ys-2.0*ys*ye+ye*ye+zs*zs-2.0*zs*ze+ze*ze);
      if(fabs(abot) < 1e-10) { 
        a = (amax+amin)/2.0;
      } else {
        a = (xs*xs-xs*xe-xn*xs+xn*xe+ys*ys-ys*ye-yn*ys+yn*ye+zs*zs-zs*ze-zn*zs+zn*ze)/abot;
        if(a > amax) { a = amax; }
        if(a < amin) { a = amin; }
      }

      /* Predict the closest point and compute the distance. */
      rp = (1.0-a)*dym_ref(segs,t,1)+a*dym_ref(segs,t+1,1);
      dp = (1.0-a)*dym_ref(segs,t,2)+a*dym_ref(segs,t+1,2);
      dist = angular_distance_RADEC(rn,rp,dn,dp);
 
      matches = (dist - rdt_tree_radius(tr) < thresh);
    }

    canprune = (matches == FALSE);
  }

  /* If we were unable to prune continue with the depth first search. */
  if(canprune == FALSE) {
    if(rdt_tree_is_leaf(tr) == TRUE) {
      rdt_tree_near_line_seg_brute(arr,rdt_tree_pts(tr),segs,ts,te,thresh,res);
    } else {
      rdt_tree_near_line_seg_recurse(rdt_tree_left_child(tr),arr,segs,ts,te,thresh,res);
      rdt_tree_near_line_seg_recurse(rdt_tree_right_child(tr),arr,segs,ts,te,thresh,res);
    }
  }
}


/* Find all observations that come within threshold 'thresh' */
/* The line segments are defined by a matrix with 3 columns  */
/* and each row is [time_i ra_i dec_i] for knot point i      */
/* Thresh is the threshold in RADIANS.                       */
/* Automatically smooths RA to handle wrap around.           */
ivec* mk_rdt_tree_near_line_segs(rdt_tree* tr, simple_obs_array* arr,
                                dym* segs, double thresh) {
  dym* segs2;
  double r, d, rlast;
  int i;
  ivec* res = mk_ivec(0);

  /* Augment the angular coordinates of the knots */
  /* with rectangular coordinates.                */
  rlast = dym_ref(segs,0,1);
  segs2 = mk_dym(dym_rows(segs),6);
  for(i=0;i<dym_rows(segs);i++) {
    r = dym_ref(segs,i,1);
    d = dym_ref(segs,i,2);

    /* Shift r around to handle wraparound. */  
    while(r-rlast >  12.0) { r -= 24.0; }
    while(r-rlast < -12.0) { r += 24.0; }
    rlast = r;

    dym_set(segs2,i,0,dym_ref(segs,i,0));
    dym_set(segs2,i,1,r);
    dym_set(segs2,i,2,d);

    r *= 15.0*DEG_TO_RAD;
    d *= DEG_TO_RAD;
    
    dym_set(segs2,i,3,cos(r)*cos(d));
    dym_set(segs2,i,4,sin(r)*cos(d));
    dym_set(segs2,i,5,sin(d));
  }

  /* Do the actual search.   */
  rdt_tree_near_line_seg_recurse(tr,arr,segs2,0,dym_rows(segs2)-1,thresh,res);

  /* Free up the used memory. */ 
  free_dym(segs2);

  return res;
}


void test_rdt_tree_ls(int argc,char *argv[]) {
  simple_obs_array* obs;
  rdt_tree*         tr;
  char* fname   = string_from_args("file",argc,argv,NULL);
  double thresh = double_from_args("thresh",argc,argv,10.0)/3600.0*DEG_TO_RAD;
  ivec* true_groups;
  ivec* true_pairs;
  ivec* inds;
  ivec* res;
  dym* segs;

  obs = mk_simple_obs_array_from_file(fname,0.5,&true_groups,&true_pairs);
  if(simple_obs_array_size(obs) <= 500) {
    fprintf_simple_obs_array(stdout,obs);
  }
  inds = mk_sequence_ivec(0,simple_obs_array_size(obs));

  if(simple_obs_array_size(obs) <= 500) {
    tr = mk_rdt_tree(obs,NULL,FALSE,3);
  } else {
    tr = mk_rdt_tree(obs,NULL,FALSE,3);
  }

  segs = mk_dym(6,3);
  dym_set(segs,0,0,52872.0000);
  dym_set(segs,0,1,24.0371138889);
  dym_set(segs,0,2,-1.66375000);
  dym_set(segs,1,0,52876.0000);
  dym_set(segs,1,1,23.997866667);
  dym_set(segs,1,2,-1.66727778);
  dym_set(segs,2,0,52879.5000);
  dym_set(segs,2,1,23.954341667);
  dym_set(segs,2,2,-1.69350000);
  dym_set(segs,3,0,52880.5000);
  dym_set(segs,3,1,23.952341667);
  dym_set(segs,3,2,-1.69550000);
  dym_set(segs,4,0,52884.0000);
  dym_set(segs,4,1,23.904197222);
  dym_set(segs,4,2,-1.74230556);
  dym_set(segs,5,0,52884.5000);
  dym_set(segs,5,1,23.904197222);
  dym_set(segs,5,2,-1.74230556);

  /* Try the "brute" force approach... */
  res = mk_ivec(0);
  rdt_tree_near_line_seg_brute(obs,inds,segs,0,10,thresh,res);
  fprintf_ivec(stdout,"",res,"\n");
  free_ivec(res);

  res = mk_rdt_tree_near_line_segs(tr,obs,segs,thresh);
  fprintf_ivec(stdout,"",res,"\n");
  free_ivec(res);  

  free_dym(segs);  
  free_rdt_tree(tr);  
  free_simple_obs_array(obs);
  free_ivec(true_groups);
  free_ivec(true_pairs);
  free_ivec(inds);
}




/* -------------------------------------------------------------------- */
/* --- RDT Tree Pointer Array ----------------------------------------- */
/* -------------------------------------------------------------------- */

rdt_tree_ptr_array* mk_sized_empty_rdt_tree_ptr_array(int size) {
  rdt_tree_ptr_array* res = AM_MALLOC(rdt_tree_ptr_array);
  int i;

  res->size     = 0;
  res->max_size = size;

  res->trs = AM_MALLOC_ARRAY(rdt_tree*,size);
  for(i=0;i<size;i++) {
    res->trs[i] = NULL;
  }

  return res;
}


rdt_tree_ptr_array* mk_empty_rdt_tree_ptr_array() {
  return mk_sized_empty_rdt_tree_ptr_array(10);
}


void free_rdt_tree_ptr_array(rdt_tree_ptr_array* old) {
  AM_FREE_ARRAY(old->trs,rdt_tree*,old->max_size);
  AM_FREE(old,rdt_tree_ptr_array);
}


void rdt_tree_ptr_array_double_size(rdt_tree_ptr_array* old) {
  rdt_tree** nu_arr;
  int i;

  nu_arr = AM_MALLOC_ARRAY(rdt_tree*,2*old->max_size+1);
  for(i=0;i<old->size;i++) {
    nu_arr[i] = old->trs[i];
  }
  for(i=old->size;i<2*old->max_size+1;i++) {
    nu_arr[i] = NULL;
  }

  AM_FREE_ARRAY(old->trs,rdt_tree*,old->max_size);

  old->trs      = nu_arr;
  old->max_size = 2*old->max_size+1;
}


/* --- Access Functions ------------------------------------------- */

rdt_tree* safe_rdt_tree_ptr_array_ref(rdt_tree_ptr_array* X, int index) {
  my_assert((X->max_size > index)&&(index >= 0));
  return X->trs[index];
}

void rdt_tree_ptr_array_set(rdt_tree_ptr_array* X, int index, rdt_tree* A) {
  my_assert(index >= 0);

  while(index >= X->max_size) { rdt_tree_ptr_array_double_size(X); }

  X->trs[index] = A;
  if(index >= X->size) { X->size = index+1; }
}

void rdt_tree_ptr_array_add(rdt_tree_ptr_array* X, rdt_tree* A) {

  /* Grow the bounds (if needed). */
  if(X->size >= X->max_size) { rdt_tree_ptr_array_double_size(X); }

  /* Make the change and increase the length. */
  X->trs[X->size] = A;
  X->size += 1;
}

int safe_rdt_tree_ptr_array_size(rdt_tree_ptr_array* X) {
  return X->size;
}

int safe_rdt_tree_ptr_array_max_size(rdt_tree_ptr_array* X) {
  return X->max_size;
}
