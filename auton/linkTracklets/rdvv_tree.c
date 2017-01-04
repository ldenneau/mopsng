/*
   File:        rdvv_tree.c
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

#include "rdvv_tree.h"

int rdvv_count  = 0;


/* --- Useful Helper Functions -------------------------- */

void rdvv_tree_fill_bounds(rdvv_tree* tr, track_array* obs, ivec* inds) {
  track* X;
  double val;
  bool first = TRUE;
  int i;

  for(i=0;i<ivec_size(inds);i++) {
    X = track_array_ref(obs,ivec_ref(inds,i));

    if(X != NULL) {

      val = track_time(X);
      if(first || (tr->hi[RDVV_T] < val)) { tr->hi[RDVV_T] = val; }
      if(first || (tr->lo[RDVV_T] > val)) { tr->lo[RDVV_T] = val; }

      val = track_RA(X);
      if(first || (tr->hi[RDVV_R] < val)) { tr->hi[RDVV_R] = val; }
      if(first || (tr->lo[RDVV_R] > val)) { tr->lo[RDVV_R] = val; }

      val = track_DEC(X);
      if(first || (tr->hi[RDVV_D] < val)) { tr->hi[RDVV_D] = val; }
      if(first || (tr->lo[RDVV_D] > val)) { tr->lo[RDVV_D] = val; }

      val = track_vRA(X);
      if(first || (tr->hi[RDVV_VR] < val)) { tr->hi[RDVV_VR] = val; }
      if(first || (tr->lo[RDVV_VR] > val)) { tr->lo[RDVV_VR] = val; }

      val = track_vDEC(X);
      if(first || (tr->hi[RDVV_VD] < val)) { tr->hi[RDVV_VD] = val; }
      if(first || (tr->lo[RDVV_VD] > val)) { tr->lo[RDVV_VD] = val; }

      first = FALSE;
    }
  }

  for(i=0;i<RDVV_NUM_DIMS;i++) {
    tr->mid[i] = RDVV_MID(tr->lo[i],tr->hi[i]);
    tr->rad[i] = RDVV_RAD(tr->lo[i],tr->hi[i]);
  }
}


/* --- Tree Memory Functions -------------------------- */

rdvv_tree* mk_empty_rdvv_tree() {
  rdvv_tree* res = AM_MALLOC(rdvv_tree);
  int i;

  res->num_points = 0;
  res->split_dim  = -1;
  res->split_val  = 0.0;

  res->right   = NULL;
  res->left    = NULL;
  res->trcks   = NULL;

  for(i=0;i<RDVV_NUM_DIMS;i++) {
    res->hi[i]  = 0.0;
    res->lo[i]  = 0.0;
    res->rad[i] = 0.0;
    res->mid[i] = 0.0;  
  }

  return res;
}


void free_rdvv_tree(rdvv_tree* old) {
  if(old->right)   { free_rdvv_tree(old->right); }
  if(old->left)    { free_rdvv_tree(old->left);  }
  if(old->trcks)   { free_ivec(old->trcks); }

  AM_FREE(old,rdvv_tree);
}


rdvv_tree* mk_rdvv_tree_recurse(track_array* obs, ivec* inds, dyv* widths, int min_leaf_pts) {
  track* X;
  rdvv_tree* res;
  ivec *left, *right;
  double split_width, val;
  int sd, i, N;
  
  /* Allocate space for the tree and calculate the bounds of the node */
  res = mk_empty_rdvv_tree(); 
  N   = ivec_size(inds);
  rdvv_tree_fill_bounds(res,obs,inds);
  res->num_points = N;
  
  /* Determine if this node will be a leaf or internal */
  if(ivec_size(inds) <= min_leaf_pts) {
    res->trcks = mk_copy_ivec(inds);
  } else {

    /* Pick the widest dimension and split it. */
    split_width = 0.0;
    sd          = -1;
    for(i=0;i<RDVV_NUM_DIMS;i++) {
      val = res->rad[i] / dyv_ref(widths,i);
      if((i==0)||(val > split_width)) {
        split_width = val;
        sd          = i;
      }
    }
    res->split_val = res->mid[sd];
    res->split_dim = sd;

    /* Actually divide up the points. */
    left = mk_ivec(0);
    right = mk_ivec(0);

    for(i=0;i<ivec_size(inds);i++) {
      X   = track_array_ref(obs,ivec_ref(inds,i));
      val = 0.0;

      switch(sd) {
      case RDVV_T:  val = track_time(X); break;
      case RDVV_R:  val = track_RA(X);   break;
      case RDVV_D:  val = track_DEC(X);  break;
      case RDVV_VR: val = track_vRA(X);  break;
      case RDVV_VD: val = track_vDEC(X); break;
      }
    
      if(val < res->split_val) {
        add_to_ivec(left,ivec_ref(inds,i));
      } else {
        add_to_ivec(right,ivec_ref(inds,i));
      }
    }

    /* Build the left and right sub-trees */
    res->left  = mk_rdvv_tree_recurse(obs, left,  widths, min_leaf_pts);
    res->right = mk_rdvv_tree_recurse(obs, right, widths, min_leaf_pts);

    free_ivec(left);
    free_ivec(right);
  }

  return res;
}


/* If favor time is true then we are more likely to split on time... */
rdvv_tree* mk_rdvv_tree(track_array* arr, simple_obs_array* obs, int min_leaf_pts,
                        bool favor_time) {
  rdvv_tree *res;
  ivec      *inds;
  dyv       *width;
  double val;
  int    N = track_array_size(arr);
  int    i;

  /* Force all of the tracks to their starting time. */
  for(i=0;i<N;i++) { track_force_t0_first(track_array_ref(arr,i),obs); }

  /* Store all the indices for the tree. */
  inds  = mk_sequence_ivec(0,N);
  
  /* Calculate the initial width of each factor. */
  res  = mk_empty_rdvv_tree();
  rdvv_tree_fill_bounds(res, arr, inds);
  width = mk_zero_dyv(RDVV_NUM_DIMS);
  for(i=0;i<RDVV_NUM_DIMS;i++) {
    val = res->rad[i];
    if(val < 1e-20) { val = 1e-20; }
    dyv_set(width,i,val);
  }
  free_rdvv_tree(res);

  if(favor_time) {
    dyv_set(width,RDVV_T,1e-20);
  }

  /* Build the tree. */
  res  = mk_rdvv_tree_recurse(arr, inds, width, min_leaf_pts);

  /* Free the used memory */
  free_dyv(width);
  free_ivec(inds);

  return res;
}


/* --- Getter/Setter Functions ----------------------------------------- */

int safe_rdvv_num_points(rdvv_tree* tr) {
  return tr->num_points;
}

int safe_rdvv_split_dim(rdvv_tree* tr) {
  return tr->split_dim;
}

double safe_rdvv_split_val(rdvv_tree* tr) {
  return tr->split_val;
}

bool safe_rdvv_is_leaf(rdvv_tree* tr) {
  return (tr->split_dim == -1);
}

ivec* safe_rdvv_tracks(rdvv_tree* tr) {
  return tr->trcks;
}

rdvv_tree* safe_rdvv_right_child(rdvv_tree* tr) {
  return tr->right;
}

rdvv_tree* safe_rdvv_left_child(rdvv_tree* tr) {
  return tr->left;
}

double safe_rdvv_hi_time(rdvv_tree* tr)  { return tr->hi[RDVV_T]; }
double safe_rdvv_lo_time(rdvv_tree* tr)  { return tr->lo[RDVV_T]; }
double safe_rdvv_mid_time(rdvv_tree* tr) { return tr->mid[RDVV_T]; }
double safe_rdvv_rad_time(rdvv_tree* tr) { return tr->rad[RDVV_T]; }

double safe_rdvv_hi_ra(rdvv_tree* tr)  { return tr->hi[RDVV_R]; }
double safe_rdvv_lo_ra(rdvv_tree* tr)  { return tr->lo[RDVV_R]; }
double safe_rdvv_mid_ra(rdvv_tree* tr) { return tr->mid[RDVV_R]; }
double safe_rdvv_rad_ra(rdvv_tree* tr) { return tr->rad[RDVV_R]; }

double safe_rdvv_hi_dec(rdvv_tree* tr)  { return tr->hi[RDVV_D]; }
double safe_rdvv_lo_dec(rdvv_tree* tr)  { return tr->lo[RDVV_D]; }
double safe_rdvv_mid_dec(rdvv_tree* tr) { return tr->mid[RDVV_D]; }
double safe_rdvv_rad_dec(rdvv_tree* tr) { return tr->rad[RDVV_D]; }

double safe_rdvv_hi_vra(rdvv_tree* tr)  { return tr->hi[RDVV_VR]; }
double safe_rdvv_lo_vra(rdvv_tree* tr)  { return tr->lo[RDVV_VR]; }
double safe_rdvv_mid_vra(rdvv_tree* tr) { return tr->mid[RDVV_VR]; }
double safe_rdvv_rad_vra(rdvv_tree* tr) { return tr->rad[RDVV_VR]; }

double safe_rdvv_hi_vdec(rdvv_tree* tr)  { return tr->hi[RDVV_VD]; }
double safe_rdvv_lo_vdec(rdvv_tree* tr)  { return tr->lo[RDVV_VD]; }
double safe_rdvv_mid_vdec(rdvv_tree* tr) { return tr->mid[RDVV_VD]; }
double safe_rdvv_rad_vdec(rdvv_tree* tr) { return tr->rad[RDVV_VD]; }


double safe_rdvv_hi_bound(rdvv_tree* tr, int dim) {
  my_assert((dim >= 0)&&(dim < RDVV_NUM_DIMS));

  return tr->hi[dim];
}

double safe_rdvv_lo_bound(rdvv_tree* tr, int dim) {
  my_assert((dim >= 0)&&(dim < RDVV_NUM_DIMS));

  return tr->lo[dim];
}

double safe_rdvv_mid_bound(rdvv_tree* tr, int dim) {
  my_assert((dim >= 0)&&(dim < RDVV_NUM_DIMS));

  return tr->mid[dim];
}

double safe_rdvv_rad_bound(rdvv_tree* tr, int dim) {
  my_assert((dim >= 0)&&(dim < RDVV_NUM_DIMS));

  return tr->rad[dim];
}


/* --- Simple I/O Functions -------------------------------------------- */

void fprintf_rdvv_tree_simple_recurse(FILE* f, rdvv_tree* t, int level) {
  int i;

  for(i=0;i<level;i++) { fprintf(f,"-"); }
  fprintf(f," [%i pts, split=%i (%f)]\n",rdvv_num_points(t),rdvv_split_dim(t),rdvv_split_val(t));
  if(rdvv_is_leaf(t)==FALSE) {
    fprintf_rdvv_tree_simple_recurse(f,rdvv_left_child(t),level+1);
    fprintf_rdvv_tree_simple_recurse(f,rdvv_right_child(t),level+1);
  } else {
    for(i=0;i<level;i++) { fprintf(f,"-"); }
    fprintf_ivec(f,"   ",t->trcks,"\n");
  }
}

void fprintf_rdvv_tree_simple(FILE* f, char* pre, rdvv_tree* tr, char* post) {
  fprintf(f,pre);
  fprintf_rdvv_tree_simple_recurse(f,tr,0);
  fprintf(f,post);
}

void fprintf_rdvv_tree_node(FILE* f, char* pre, rdvv_tree* tr, char* post) {
  fprintf(f,pre);
  fprintf(f,"Node with %i points and %i children\n",rdvv_num_points(tr),
          (rdvv_is_leaf(tr) ? 0 : 2));
  fprintf(f," T bounds    = [%10.8f, %10.8f], mid = %10.8f, rad = %10.8f\n",
          rdvv_lo_time(tr),rdvv_hi_time(tr),rdvv_mid_time(tr),rdvv_rad_time(tr));
  fprintf(f," RA bounds   = [%10.8f, %10.8f], mid = %10.8f, rad = %10.8f\n",
          rdvv_lo_ra(tr),rdvv_hi_ra(tr),rdvv_mid_ra(tr),rdvv_rad_ra(tr));
  fprintf(f," DEC bounds  = [%10.8f, %10.8f], mid = %10.8f, rad = %10.8f\n",
          rdvv_lo_dec(tr),rdvv_hi_dec(tr),rdvv_mid_dec(tr),rdvv_rad_dec(tr));
  fprintf(f," vRA bounds  = [%10.8f, %10.8f], mid = %10.8f, rad = %10.8f\n",
          rdvv_lo_vra(tr),rdvv_hi_vra(tr),rdvv_mid_vra(tr),rdvv_rad_vra(tr));
  fprintf(f," vDEC bounds = [%10.8f, %10.8f], mid = %10.8f, rad = %10.8f\n",
          rdvv_lo_vdec(tr),rdvv_hi_vdec(tr),rdvv_mid_vdec(tr),rdvv_rad_vdec(tr));
  fprintf(f,post);
}



/* --- Query Functions ------------------------------------------------- */


/* Find all tracks Y such that the midpoint_distance(X,Y) is <= thresh  */
/* and t_s <= Y.time <= t_e.  If inds == NULL look at all tracks.       */
ivec* mk_rdvv_find_midpt_slow(track_array* arr, ivec* inds, track* X, 
                              double t_s, double t_e, double thresh) {
  track* Y;
  ivec* res = mk_ivec(0);
  double ta, tb, tm, td, dist;
  double ra, da, rb, db;
  int i, N, ind;

  ta = track_time(X);
  N  = track_array_size(arr);
  if(inds) { N = ivec_size(inds); }
  
  for(i=0;i<N;i++) {
    rdvv_count++;

    /* Get the index of the new candidate... */
    ind = i;
    if(inds) { ind = ivec_ref(inds,i); }

    /* Get the candidate track and it's time */
    Y  = track_array_ref(arr,ind);
    tb = track_time(Y);
    td = fabs(track_time(X) - tb);

    if((tb < t_e)&&(tb > t_s)&&(td > 1e-20)) {

      /* Calculate the midpoint time and project both tracks to then */
      tm = (track_time(X) + track_time(Y))/2.0;
      track_RA_DEC_prediction(X,tm - ta,&ra,&da);
      track_RA_DEC_prediction(Y,tm - tb,&rb,&db);

      /* Find the distance between the tracks. */
      dist = simple_obs_euclidean_dist_given(ra,rb,da,db);

      if(dist < thresh) {
        add_to_ivec(res,ind);
      }
    }
  }

  return res;
}


ivec* mk_rdvv_find_midpt_recurse(rdvv_tree* tr, track_array* arr, track* X, 
                                 double t_s, double t_e, double thresh, int verb) {
  ivec* res = NULL;
  ivec* tempA;
  ivec* tempB;
  bool prune     = FALSE;
  double r_min, r_max, r_mid, r_rad; 
  double d_min, d_max, d_mid, d_rad;
  double aminR, amaxR, aminD, amaxD;
  double ts, te, tq;
  double a,b,c,d;
  double dist;

  /* Decide whether or not to prune: check time */
  prune = ((tr->lo[RDVV_T] > t_e)||(tr->hi[RDVV_T] < t_s));
  ts    = RDVV_SIMPLE_MAX(t_s, tr->lo[RDVV_T]);
  te    = RDVV_SIMPLE_MIN(t_e, tr->hi[RDVV_T]);
  tq    = track_time(X);

  /* Check again on time -> if box is down to one time step
     and that time step is the query step then prune.  */
  if((prune == FALSE)&&(te-ts <= 1e-10)) {
    prune = (fabs(ts-tq) < 1e-10);
  }


  /*if(verb > 0) {
    printf("\nTime ----------------------------------------------------\n");
    printf("Query = [%20.10f, %20.10f]\n", t_s, t_e);
    printf("Box   = [%20.10f, %20.10f]\n", tr->lo[RDVV_T], tr->hi[RDVV_T]);
    printf("New   = [%20.10f, %20.10f]\n", ts, te);
    printf("Prune = %i\n",prune);
    }*/

  /* Decide to bound velocity IFF the track is not */
  /* INSIDE the query box. */
  if((prune == FALSE)&&((tq < ts)||(tq > te))) {

    rdvv_count += 4;

    /* Place bounds on the RA accel */
    a = (tr->lo[RDVV_VR] - track_vRA(X))/(ts - tq);
    b = (tr->hi[RDVV_VR] - track_vRA(X))/(ts - tq);
    c = (tr->lo[RDVV_VR] - track_vRA(X))/(te - tq);
    d = (tr->hi[RDVV_VR] - track_vRA(X))/(te - tq);
    aminR = RDVV_SIMPLE_MIN(RDVV_SIMPLE_MIN(a,b),RDVV_SIMPLE_MIN(c,d));
    amaxR = RDVV_SIMPLE_MAX(RDVV_SIMPLE_MAX(a,b),RDVV_SIMPLE_MAX(c,d));

    aminR = RDVV_SIMPLE_MAX(aminR,-RDVV_MAX_ACC/15.0);
    amaxR = RDVV_SIMPLE_MIN(amaxR, RDVV_MAX_ACC/15.0);

    /*if(verb > 0) {
      printf("\nRA ------------------------------------------------------\n");
      printf("%10.8f %10.8f %10.8f %10.8f\n",a,b,c,d);
      printf("Accel Bounds = [%20.10f, %20.10f]\n",aminR,amaxR);
      }*/

    /* Place bounds on the RA positions that could be hit... */
    a = track_RA(X) + (ts-tq)*track_vRA(X) + 0.5*(ts-tq)*(ts-tq)*aminR;
    b = track_RA(X) + (ts-tq)*track_vRA(X) + 0.5*(ts-tq)*(ts-tq)*amaxR;
    c = track_RA(X) + (te-tq)*track_vRA(X) + 0.5*(te-tq)*(te-tq)*aminR;
    d = track_RA(X) + (te-tq)*track_vRA(X) + 0.5*(te-tq)*(te-tq)*amaxR;
    r_min = RDVV_SIMPLE_MIN(RDVV_SIMPLE_MIN(a,b),RDVV_SIMPLE_MIN(c,d));
    r_max = RDVV_SIMPLE_MAX(RDVV_SIMPLE_MAX(a,b),RDVV_SIMPLE_MAX(c,d));
    r_mid = RDVV_MID(r_min,r_max);
    r_rad = RDVV_RAD(r_min,r_max);

    dist = fabs(r_mid - tr->mid[RDVV_R]);
    while(dist > 12.0) { dist = fabs(dist - 24.0); }
    prune = (dist - r_rad - tr->rad[RDVV_R] > thresh/15.0);

    /*if(verb > 0) {
      printf("ts-tq = %f,   te-tq = %f\n",ts-tq,te-tq);
      printf("ts, amin: %f = %f + %f*t + 0.5*t*t*%f\n",a,track_RA(X),track_vRA(X),aminR);
      printf("ts, amax: %f = %f + %f*t + 0.5*t*t*%f\n",b,track_RA(X),track_vRA(X),amaxR);
      printf("te, amin: %f = %f + %f*t + 0.5*t*t*%f\n",c,track_RA(X),track_vRA(X),aminR);
      printf("te, amax: %f = %f + %f*t + 0.5*t*t*%f\n",d,track_RA(X),track_vRA(X),amaxR);
      printf("RA Bounds = [%20.10f, %20.10f] = [%20.10f, %20.10f]\n",r_min,r_max,r_mid,r_rad);
      printf("Dist = %f - %f - %f\n",dist,r_rad,tr->rad[RDVV_R]);
      printf("Prune = %i\n",prune);
      }*/

  }

  /* Decide to bound velocity IFF the track is not */
  /* INSIDE the query box. */
  if((prune == FALSE)&&((tq < ts)||(tq > te))) {

    /* Place bounds on the DEC accel */
    a = (tr->lo[RDVV_VD] - track_vDEC(X))/(ts - tq);
    b = (tr->hi[RDVV_VD] - track_vDEC(X))/(ts - tq);
    c = (tr->lo[RDVV_VD] - track_vDEC(X))/(te - tq);
    d = (tr->hi[RDVV_VD] - track_vDEC(X))/(te - tq);
    aminD = RDVV_SIMPLE_MIN(RDVV_SIMPLE_MIN(a,b),RDVV_SIMPLE_MIN(c,d));
    amaxD = RDVV_SIMPLE_MAX(RDVV_SIMPLE_MAX(a,b),RDVV_SIMPLE_MAX(c,d));

    aminD = RDVV_SIMPLE_MAX(aminD,-RDVV_MAX_ACC);
    amaxD = RDVV_SIMPLE_MIN(amaxD,RDVV_MAX_ACC);

    /*if(verb > 0) {
      printf("DEC -----------------------------------------------------\n");
      printf("%10.8f %10.8f %10.8f %10.8f\n",a,b,c,d);
      printf("Accel Bounds = [%20.10f, %20.10f]\n",aminD,amaxD);
      }*/

    /* Place bounds on the DEC positions that could be hit... */
    a = track_DEC(X) + (ts-tq)*track_vDEC(X) + 0.5*(ts-tq)*(ts-tq)*aminD;
    b = track_DEC(X) + (ts-tq)*track_vDEC(X) + 0.5*(ts-tq)*(ts-tq)*amaxD;
    c = track_DEC(X) + (te-tq)*track_vDEC(X) + 0.5*(te-tq)*(te-tq)*aminD;
    d = track_DEC(X) + (te-tq)*track_vDEC(X) + 0.5*(te-tq)*(te-tq)*amaxD;
    d_min = RDVV_SIMPLE_MIN(RDVV_SIMPLE_MIN(a,b),RDVV_SIMPLE_MIN(c,d));
    d_max = RDVV_SIMPLE_MAX(RDVV_SIMPLE_MAX(a,b),RDVV_SIMPLE_MAX(c,d));
    d_mid = RDVV_MID(d_min,d_max);
    d_rad = RDVV_RAD(d_min,d_max);

    dist = fabs(d_mid - tr->mid[RDVV_D]);
    prune = (dist - d_rad - tr->rad[RDVV_D] > thresh);

    /*if(verb > 0) {
      printf("%10.8f %10.8f %10.8f %10.8f\n",a,b,c,d);
      printf("DEC Bounds = [%20.10f, %20.10f] = [%20.10f, %20.10f]\n",d_min,d_max,d_mid,d_rad);
      printf("Dist = %f - %f - %f\n",dist,d_rad,tr->rad[RDVV_D]);
      printf("Prune = %i\n",prune);
      }*/
  }

  /*if(verb > 0) {
    wait_for_key();
    }*/

  /* Either prune or recursively solve the problem. */
  if(prune == TRUE) {
    res = NULL;
  } else { 
    if(tr->split_dim > -1) {
      tempA = mk_rdvv_find_midpt_recurse(tr->left,arr,X,t_s,t_e,thresh,verb);
      tempB = mk_rdvv_find_midpt_recurse(tr->right,arr,X,t_s,t_e,thresh,verb);

      if((tempA != NULL)&&(tempB != NULL)) { 
        res = mk_ivec_union_ordered(tempA,tempB); 
        free_ivec(tempA);
        free_ivec(tempB);
      } else {
        if(tempA != NULL) { res = tempA; }
        if(tempB != NULL) { res = tempB; }
      }

    } else {
      res = mk_rdvv_find_midpt_slow(arr, tr->trcks, X, t_s, t_e, thresh);
      if(ivec_size(res) == 0) {
        free_ivec(res);
        res = NULL;
      }
    }
  } 

  /*  if(prune && (ivec_size(res) > 0)) {
      fprintf_track(stdout,"Q: ",X,"\n");
      for(j=0;j<ivec_size(res);j++) {
        fprintf_track(stdout,"E: ",track_array_ref(arr,ivec_ref(res,j)),"\n");
      }

      tempA = mk_rdvv_find_midpt_recurse(tr,arr,X,t_s,t_e,thresh,1);
      free_ivec(tempA);
      wait_for_key();
      }*/

  return res;
}


ivec* mk_rdvv_find_midpt(rdvv_tree* tr, track_array* arr, track* X,
                          double t_start, double t_end, double thresh) {
  ivec* res = NULL;
  
  res = mk_rdvv_find_midpt_recurse(tr,arr,X,t_start,t_end,thresh,0);
  
  if(res == NULL) {
    res = mk_ivec(0);
  }

  return res;
}


dym* mk_test_rdvv_tree2(track_array* arr, simple_obs_array* obs, ivec* true_groups,
                        double thresh) {
  dym     *res;
  rdvv_tree *tr;
  double time_length = 5.0;
  track* X;
  ivec  *resE, *resL;
  ivec  *inds;
  ivec  *temp;
  int N = track_array_size(arr);
  int i;
  bool failed;

  /* Set up the data structures for the pairs find. */
  tr   = mk_rdvv_tree(arr, obs, RDVV_MAX_LEAF_NODES, FALSE);
  inds = mk_sequence_ivec(0,N);
  res  = mk_zero_dym(N,4);

  /* Go through ALL of the points and try to find their neighbors. */
  for(i=0;i<N;i++) {
    X = track_array_ref(arr,i);

    /* Try an exhaustive search... */
    start_wc_timer();
    resE = mk_rdvv_find_midpt_slow(arr,inds,X,track_time(X)-time_length,
                                   track_time(X)+time_length,thresh);
    dym_set(res,i,0,stop_wc_timer());
    dym_set(res,i,1,ivec_size(inds));

    /* Try the LS-tree search... */
    rdvv_count  = 0;
    start_wc_timer();
    resL = mk_rdvv_find_midpt(tr, arr, X, track_time(X)-time_length,
                              track_time(X)+time_length, thresh);
    dym_set(res,i,2,stop_wc_timer());
    dym_set(res,i,3,rdvv_count);

    temp = mk_ivec_union(resE,resL);
    failed = (ivec_size(temp) != ivec_size(resE))||(ivec_size(temp) != ivec_size(resL));
    free_ivec(temp);

    if(failed) {
      printf("Trial %i\n",i);
      fprintf_ivec(stdout,"EX ",resE,"\n");
      fprintf_ivec(stdout,"LS ",resL,"\n");
      wait_for_key();
    }

    free_ivec(resE);
    free_ivec(resL);
  }


  /* Free the rdvv_tree */
  free_ivec(inds);
  free_rdvv_tree(tr);

  return res;
}


void rdvv_test_main(int argc,char *argv[]) {
  char* fname        = string_from_args("filename",argc,argv,NULL);
  double fit_thresh  = double_from_args("fit_thresh",argc,argv,0.10);
  simple_obs_array* obs;
  ivec_array* obs_to_track;
  ivec* true_groups;
  track_array* t1;
  dym* temp;

  if(fname == NULL) {
    printf("ERROR: No filename given.\n");
  } else {
    obs         = mk_simple_obs_array_from_MPC_file(fname,0);
    true_groups = mk_simple_obs_groups_from_labels(obs, TRUE);

    obs_to_track = mk_simple_obs_pairing_from_true_groups(obs,true_groups,0.5);
    t1 = mk_track_array_from_matched_simple_obs(obs,obs_to_track,2);
    free_ivec_array(obs_to_track);
    
    temp = mk_test_rdvv_tree2(t1, obs, true_groups, fit_thresh);
    fprintf_dym(stdout,"",temp,"\n");
    free_dym(temp);

    free_track_array(t1);
    free_ivec(true_groups);
    free_simple_obs_array(obs);
  }
}
