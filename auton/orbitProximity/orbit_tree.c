/*
   File:        orbit_tree.c
   Author(s):   Kubica
   Created:     Tues Nov 16 2004
   Description: Functions to create and operate trees of orbits.
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

#include "orbit_tree.h"


/* Orbit Trees are KD trees designed for orbit parameters.        */
/* They contain information about a variety of factors (a,e,etc.) */
/* and the relative importance of these factors during tree       */
/* construction is controlled by a weight vector W.               */
/*    W[ORBTREE_Q] = 100.0 ->  Treat q as 100x more important     */
/*    W[ORBTREE_T] =   0.0 ->  Ignore T0                          */
dyv* mk_orb_tree_weights(double q, double e, double w, 
                         double O, double i, double t0) {
  dyv* res = mk_zero_dyv(ORBTREE_DIMS);

  dyv_set(res,ORBTREE_Q,q);
  dyv_set(res,ORBTREE_E,e);
  dyv_set(res,ORBTREE_W,w);
  dyv_set(res,ORBTREE_O,O);
  dyv_set(res,ORBTREE_I,i);
  dyv_set(res,ORBTREE_T,t0);

  return res;
}


/* --------------------------------------------------------------------- */
/* --- Tree Helper Functions ------------------------------------------- */
/* --------------------------------------------------------------------- */
void orb_tree_fill_bounds(orb_tree* tr, orbit_array* oarr, ivec* inds) {
  int N = orbit_array_size(oarr);
  int i, ind;
  orbit* o;
  bool setone = FALSE;

  if(inds != NULL) { N = ivec_size(inds); }

  for(i=0;i<N;i++) {
    if(inds != NULL) { ind = ivec_ref(inds,i); } else { ind = i; }

    o = orbit_array_ref(oarr,ind);
    if(o != NULL) {

      /* Is this a new lower bound (less than current bound or first seen) */
      if((setone==FALSE)||(tr->lo[ORBTREE_Q] > orbit_q(o))) { tr->lo[ORBTREE_Q] = orbit_q(o); }
      if((setone==FALSE)||(tr->lo[ORBTREE_E] > orbit_e(o))) { tr->lo[ORBTREE_E] = orbit_e(o); }
      if((setone==FALSE)||(tr->lo[ORBTREE_O] > orbit_O(o))) { tr->lo[ORBTREE_O] = orbit_O(o); }
      if((setone==FALSE)||(tr->lo[ORBTREE_W] > orbit_w(o))) { tr->lo[ORBTREE_W] = orbit_w(o); }
      if((setone==FALSE)||(tr->lo[ORBTREE_I] > orbit_i(o))) { tr->lo[ORBTREE_I] = orbit_i(o); }
      if((setone==FALSE)||(tr->lo[ORBTREE_T] > orbit_t0(o))) { tr->lo[ORBTREE_T] = orbit_t0(o); }

      /* Is this a new upper bound (> than current bound or first seen) */
      if((setone==FALSE)||(tr->hi[ORBTREE_Q] < orbit_q(o))) { tr->hi[ORBTREE_Q] = orbit_q(o); }
      if((setone==FALSE)||(tr->hi[ORBTREE_E] < orbit_e(o))) { tr->hi[ORBTREE_E] = orbit_e(o); }
      if((setone==FALSE)||(tr->hi[ORBTREE_O] < orbit_O(o))) { tr->hi[ORBTREE_O] = orbit_O(o); }
      if((setone==FALSE)||(tr->hi[ORBTREE_W] < orbit_w(o))) { tr->hi[ORBTREE_W] = orbit_w(o); }
      if((setone==FALSE)||(tr->hi[ORBTREE_I] < orbit_i(o))) { tr->hi[ORBTREE_I] = orbit_i(o); }
      if((setone==FALSE)||(tr->hi[ORBTREE_T] < orbit_t0(o))) { tr->hi[ORBTREE_T] = orbit_t0(o); }

      setone = TRUE;
    }
  }

}


/* --------------------------------------------------------------------- */
/* --- Tree Memory Functions ------------------------------------------- */
/* --------------------------------------------------------------------- */

orb_tree* mk_empty_orb_tree() {
  orb_tree* res = AM_MALLOC(orb_tree);
  int i;

  res->num_points = 0;
  res->left       = NULL;
  res->right      = NULL;
  res->trcks      = NULL;

  for(i=0;i<ORBTREE_DIMS;i++) {
    res->hi[i] = 0.0;
    res->lo[i] = 0.0;
  }

  return res;
}


void free_orb_tree(orb_tree* old) {
  if(old->right) { free_orb_tree(old->right); }
  if(old->left)  { free_orb_tree(old->left); }
  if(old->trcks) { free_ivec(old->trcks); }
  AM_FREE(old,orb_tree);
}


orb_tree* mk_orb_tree_recurse(orbit_array* obs, ivec* inds, dyv* weights, int min_leaf_pts) {
  orbit* X;
  orb_tree* res;
  ivec *left, *right;
  double sw, sv, val, sum;
  int sd, i, N;

  /* Allocate space for the tree and calculate the bounds of the node */
  res = mk_empty_orb_tree();
  N   = ivec_size(inds);
  orb_tree_fill_bounds(res,obs,inds);
  res->num_points = N;

  /* Compute weighted sum of dimensions (early stopping for */
  /* very small regions).                                   */
  sum = 0.0;
  for(i=0;i<ORBTREE_DIMS;i++) {
    sum += orb_tree_rad(res,i) * dyv_ref(weights,i);
  }

  /* Determine if this node will be a leaf or internal */
  if((ivec_size(inds) <= min_leaf_pts)||(sum < 1e-20)) {
    res->trcks = mk_copy_ivec(inds);
  } else {

    /* Pick the widest dimension and split it. */
    sw = 0.0;
    sd = -1;
    for(i=0;i<ORBTREE_DIMS;i++) {
      val = orb_tree_rad(res,i) * dyv_ref(weights,i);
      if((i==0)||(val > sw)) {
        sw = val;
        sd = i;
      }
    }
    sv = orb_tree_mid(res,sd);

    /* Actually divide up the points. */
    left = mk_ivec(0);
    right = mk_ivec(0);

    for(i=0;i<ivec_size(inds);i++) {
      X   = orbit_array_ref(obs,ivec_ref(inds,i));
      val = 0.0;

      switch(sd) {
      case ORBTREE_Q: val = orbit_q(X); break;
      case ORBTREE_E: val = orbit_e(X); break;
      case ORBTREE_W: val = orbit_w(X); break;
      case ORBTREE_O: val = orbit_O(X); break;
      case ORBTREE_I: val = orbit_i(X); break;
      case ORBTREE_T: val = orbit_t0(X); break;
      }

      /* Split based on the midpoint of the split dimension */
      if(val < sv) {
        add_to_ivec(left,ivec_ref(inds,i));
      } else {
        add_to_ivec(right,ivec_ref(inds,i));
      }
    }

    /* Build the left and right sub-trees */
    res->left  = mk_orb_tree_recurse(obs,left,weights,min_leaf_pts);
    res->right = mk_orb_tree_recurse(obs,right,weights,min_leaf_pts);

    free_ivec(left);
    free_ivec(right);
  }

  return res;
}


orb_tree* mk_orb_tree(orbit_array* arr, dyv* W, int max_leaf_pts) {
  orb_tree *res;
  ivec      *inds;
  dyv       *width;
  dyv       *weights;
  double val;
  int    N = orbit_array_size(arr);
  int    i;
  /* Store all the indices for the tree. */
  inds  = mk_sequence_ivec(0,N);

  /* Calculate the initial width of each factor. */
  res  = mk_empty_orb_tree();
  orb_tree_fill_bounds(res, arr, inds);
  width = mk_zero_dyv(ORBTREE_DIMS);
  for(i=0;i<ORBTREE_DIMS;i++) { 
    val = orb_tree_rad(res,i);
    if(val < 1e-20) { val = 1e-20; }
    dyv_set(width,i,val);
  }
  free_orb_tree(res);

  /* Create the actual weight vector */
  weights = mk_zero_dyv(ORBTREE_DIMS);
  for(i=0;i<ORBTREE_DIMS;i++) {
    val = 1.0/dyv_ref(width,i);
    if(W != NULL) { val *= dyv_ref(W,i); }

    dyv_set(weights,i,val);
  }

  /* Build the tree. */
  res  = mk_orb_tree_recurse(arr, inds, weights, max_leaf_pts);

  /* Free the used memory */
  free_dyv(weights);
  free_dyv(width);
  free_ivec(inds);

  return res;
}

/* --------------------------------------------------------------------- */
/* --- Tree Access Functions ------------------------------------------- */
/* --------------------------------------------------------------------- */

int safe_orb_tree_size(orb_tree* tr)       { return tr->num_points;    }
int safe_orb_tree_num_points(orb_tree* tr) { return tr->num_points;    }
ivec* safe_orb_tree_orbits(orb_tree* tr)   { return tr->trcks;         }

bool safe_orb_tree_leaf(orb_tree* tr)       { return tr->right == NULL; }
orb_tree* safe_orb_tree_left(orb_tree* tr)  { return tr->left;          }
orb_tree* safe_orb_tree_right(orb_tree* tr) { return tr->right;         }


double safe_orb_tree_max_q(orb_tree* tr) { return tr->hi[ORBTREE_Q]; }
double safe_orb_tree_max_e(orb_tree* tr) { return tr->hi[ORBTREE_E]; }
double safe_orb_tree_max_w(orb_tree* tr) { return tr->hi[ORBTREE_W]; }
double safe_orb_tree_max_O(orb_tree* tr) { return tr->hi[ORBTREE_O]; }
double safe_orb_tree_max_i(orb_tree* tr) { return tr->hi[ORBTREE_I]; }
double safe_orb_tree_max_t0(orb_tree* tr) { return tr->hi[ORBTREE_I]; }
double safe_orb_tree_max(orb_tree* tr, int dim) {
  my_assert((dim >= 0)&&(dim < ORBTREE_DIMS));
  return tr->hi[dim];
}

double safe_orb_tree_min_q(orb_tree* tr) { return tr->lo[ORBTREE_Q]; }
double safe_orb_tree_min_e(orb_tree* tr) { return tr->lo[ORBTREE_E]; }
double safe_orb_tree_min_w(orb_tree* tr) { return tr->lo[ORBTREE_W]; }
double safe_orb_tree_min_O(orb_tree* tr) { return tr->lo[ORBTREE_O]; }
double safe_orb_tree_min_i(orb_tree* tr) { return tr->lo[ORBTREE_I]; }
double safe_orb_tree_min_t0(orb_tree* tr) { return tr->lo[ORBTREE_I]; }
double safe_orb_tree_min(orb_tree* tr, int dim) {
  my_assert((dim >= 0)&&(dim < ORBTREE_DIMS));
  return tr->lo[dim];
}

double safe_orb_tree_mid_q(orb_tree* tr) { 
  return (tr->hi[ORBTREE_Q]+tr->lo[ORBTREE_Q])/2.0; 
}

double safe_orb_tree_mid_e(orb_tree* tr) { 
  return (tr->hi[ORBTREE_E]+tr->lo[ORBTREE_E])/2.0; 
}

double safe_orb_tree_mid_w(orb_tree* tr) { 
  return (tr->hi[ORBTREE_W]+tr->lo[ORBTREE_W])/2.0; 
}

double safe_orb_tree_mid_O(orb_tree* tr) { 
  return (tr->hi[ORBTREE_O]+tr->lo[ORBTREE_O])/2.0; 
}

double safe_orb_tree_mid_i(orb_tree* tr) { 
  return (tr->hi[ORBTREE_I]+tr->lo[ORBTREE_I])/2.0; 
}

double safe_orb_tree_mid_t0(orb_tree* tr) { 
  return (tr->hi[ORBTREE_T]+tr->lo[ORBTREE_T])/2.0; 
}

double safe_orb_tree_mid(orb_tree* tr, int dim) {
  my_assert((dim >= 0)&&(dim < ORBTREE_DIMS));
  return (tr->lo[dim]+tr->hi[dim])/2.0;
}

double safe_orb_tree_rad_q(orb_tree* tr) { 
  return (tr->hi[ORBTREE_Q]-tr->lo[ORBTREE_Q])/2.0; 
}

double safe_orb_tree_rad_e(orb_tree* tr) { 
  return (tr->hi[ORBTREE_E]-tr->lo[ORBTREE_E])/2.0; 
}

double safe_orb_tree_rad_w(orb_tree* tr) { 
  return (tr->hi[ORBTREE_W]-tr->lo[ORBTREE_W])/2.0; 
}

double safe_orb_tree_rad_O(orb_tree* tr) { 
  return (tr->hi[ORBTREE_O]-tr->lo[ORBTREE_O])/2.0; 
}

double safe_orb_tree_rad_i(orb_tree* tr) { 
  return (tr->hi[ORBTREE_I]-tr->lo[ORBTREE_I])/2.0; 
}

double safe_orb_tree_rad_t0(orb_tree* tr) { 
  return (tr->hi[ORBTREE_T]-tr->lo[ORBTREE_T])/2.0; 
}

double safe_orb_tree_rad(orb_tree* tr, int dim) {
  my_assert((dim >= 0)&&(dim < ORBTREE_DIMS));
  return (tr->hi[dim]-tr->lo[dim])/2.0;
}


/* --------------------------------------------------------------------- */
/* --- Tree Display Functions ------------------------------------------ */
/* --------------------------------------------------------------------- */

void fprintf_orb_tree_recurse(FILE* f, orb_tree* tr, int level) {
  int i;

  for(i=0;i<level;i++) { fprintf(f,"--"); }
  if(orb_tree_leaf(tr)) { 
    fprintf(f," LEAF: ");
  } else {
    fprintf(f," INTL: ");
  }

  fprintf(f,"%i orbits {q=[%f,%f], e=[%f,%f], w=[%f,%f], O=[%f,%f], i=[%f,%f], t0=[%f,%f]}\n",
         orb_tree_num_points(tr),orb_tree_min_q(tr),orb_tree_max_q(tr),
	 orb_tree_min_e(tr),orb_tree_max_e(tr),
	 orb_tree_min_w(tr),orb_tree_max_w(tr),
	 orb_tree_min_O(tr),orb_tree_max_O(tr),
	 orb_tree_min_i(tr),orb_tree_max_i(tr),
	 orb_tree_min_t0(tr),orb_tree_max_t0(tr));

  if(orb_tree_leaf(tr) == FALSE) {
    fprintf_orb_tree_recurse(f,orb_tree_left(tr),level+1);
    fprintf_orb_tree_recurse(f,orb_tree_right(tr),level+1);
  }
}

void fprintf_orb_tree(FILE* f, orb_tree* tr) {
  fprintf_orb_tree_recurse(f,tr,0);
}


/* --------------------------------------------------------------------- */
/* --- Tree Search Functions ------------------------------------------- */
/* --------------------------------------------------------------------- */

/* Use thresh = -1.0 for ignore. */
dyv* mk_orb_tree_search_thresh(double qthresh, double ethresh, 
                               double ithresh, double Othresh, 
                               double wthresh, double t0thresh) {
  dyv* res = mk_constant_dyv(ORBTREE_DIMS,-1.0);

  dyv_set(res,ORBTREE_Q,qthresh);
  dyv_set(res,ORBTREE_E,ethresh);
  dyv_set(res,ORBTREE_W,wthresh);
  dyv_set(res,ORBTREE_O,Othresh);
  dyv_set(res,ORBTREE_I,ithresh);
  dyv_set(res,ORBTREE_T,t0thresh);

  return res;
}


/* Exhaustively test ALL orbits in 'inds' for proximity against */
/* the thresholds in 'thresh' (used as tree search base case)   */
/* Use inds = NULL to test all orbits.                          */
void orb_tree_range_search_exh(orbit* q, orbit_array* orbs, ivec* inds, 
                               dyv* thresh, ivec* res, bool verb) {
  orbit* X;
  bool valid;
  double dist;
  double nPassages, fFractionalPassage;
  int N = orbit_array_size(orbs);
  int i, j;
 
  if(inds != NULL) { N = ivec_size(inds); }

  for(i=0;i<N;i++) {
    if(inds != NULL) { j = ivec_ref(inds,i); } else { j = i; }
    X = orbit_array_ref(orbs,j);
    valid = TRUE;

    /* Test the time of perihelion */
    if(valid && (dyv_ref(thresh,ORBTREE_T) >= 1e-20)) {
      dist      = fabs(orbit_t0(X) - orbit_t0(q))/orbit_period(q);
      nPassages = (double)((int)dist);   
      fFractionalPassage = dist - nPassages; 

      if(verb) { printf("t0(%f,%f,%f)=",dist,nPassages,fFractionalPassage); }

      valid = (fFractionalPassage < dyv_ref(thresh,ORBTREE_T)*(nPassages+1.0));
    }
    if(verb) { printf("%i ",valid); }

    /* Test the remaining dimensions. */
    if(valid && (dyv_ref(thresh,ORBTREE_Q) >= 1e-20)) {
      dist  = fabs(orbit_q(X) - orbit_q(q));
      if(verb) { printf("q(%f)=",dist); }
      valid = (dist < dyv_ref(thresh,ORBTREE_Q));
    }
    if(verb) { printf("%i ",valid); }
    if(valid && (dyv_ref(thresh,ORBTREE_E) >= 1e-20)) {
      dist  = fabs(orbit_e(X) - orbit_e(q));
      if(verb) { printf("e(%f)=",dist); }
      valid = (dist < dyv_ref(thresh,ORBTREE_E));
    }
    if(verb) { printf("%i ",valid); }
    if(valid && (dyv_ref(thresh,ORBTREE_I) >= 1e-20)) {
      dist  = fabs(orbit_i(X) - orbit_i(q));
      if(verb) { printf("i(%f,",dist); }
      while(dist > PI) { dist = fabs(dist - 2*PI); }
      if(verb) { printf(",%f)=",dist); }
      valid = (dist < dyv_ref(thresh,ORBTREE_I));
    }
    if(verb) { printf("%i ",valid); }
    if(valid && (dyv_ref(thresh,ORBTREE_O) >= 1e-20)) {
      dist  = fabs(orbit_O(X) - orbit_O(q));
      if(verb) { printf("O(%f,",dist); }
      while(dist > PI) { dist = fabs(dist - 2*PI); }
      if(verb) { printf(",%f)=",dist); }
      valid = (dist < dyv_ref(thresh,ORBTREE_O));
    }
    if(verb) { printf("%i ",valid); }
    if(valid && (dyv_ref(thresh,ORBTREE_W) >= 1e-20)) {
      if(orbit_e(q) > 0.05) {
	dist  = fabs(orbit_w(X) - orbit_w(q));
        if(verb) { printf("w(%f,",dist); }
	while(dist > PI) { dist = fabs(dist - 2*PI); }
        if(verb) { printf(",%f)=",dist); }
	valid = (dist < dyv_ref(thresh,ORBTREE_W));
      }
    }
    if(verb) { printf("%i\n",valid); }

    if(valid) {
      add_to_ivec(res,j);
    }
  }

}


void orb_tree_range_search(orbit* q, orb_tree* tr, orbit_array* orbs, 
                           dyv* thresh, ivec* res) {
  bool valid = TRUE;
  double dist;

  /* Check for pruning oppurtinities... */
  if(valid && (dyv_ref(thresh,ORBTREE_Q) >= 1e-20)) {
    dist  = fabs(orb_tree_mid_q(tr) - orbit_q(q));
    valid = (dist - orb_tree_rad_q(tr) < dyv_ref(thresh,ORBTREE_Q));
  }
  if(valid && (dyv_ref(thresh,ORBTREE_E) >= 1e-20)) {
    dist  = fabs(orb_tree_mid_e(tr) - orbit_e(q));
    valid = (dist - orb_tree_rad_e(tr) < dyv_ref(thresh,ORBTREE_E));
  }
  if(valid && (dyv_ref(thresh,ORBTREE_I) >= 1e-20)) {
    dist  = fabs(orb_tree_mid_i(tr) - orbit_i(q));
    while(dist > PI) { dist = fabs(dist - 2*PI); }
    valid = (dist - orb_tree_rad_i(tr) < dyv_ref(thresh,ORBTREE_I));
  }
  if(valid && (dyv_ref(thresh,ORBTREE_O) >= 1e-20)) {
    dist  = fabs(orb_tree_mid_O(tr) - orbit_O(q));
    while(dist > PI) { dist = fabs(dist - 2*PI); }
    valid = (dist - orb_tree_rad_O(tr) < dyv_ref(thresh,ORBTREE_O));
  }
  if(valid && (dyv_ref(thresh,ORBTREE_W) >= 1e-20)) {
    if(orbit_e(q) > 0.1) {
      dist  = fabs(orb_tree_mid_w(tr) - orbit_w(q));
      while(dist > PI) { dist = fabs(dist - 2*PI); }
      valid = (dist - orb_tree_rad_w(tr) < dyv_ref(thresh,ORBTREE_W));
    }
  }

  /* If we did not prune then we can continue searching... */
  if(valid) {
    if(orb_tree_leaf(tr)) {
      orb_tree_range_search_exh(q,orbs,orb_tree_orbits(tr),thresh,res,FALSE);
    } else {
      orb_tree_range_search(q,orb_tree_left(tr),orbs,thresh,res);
      orb_tree_range_search(q,orb_tree_right(tr),orbs,thresh,res);
    }
  }
}


ivec* mk_orb_tree_range_search(orbit* q, orb_tree* tr, 
                               orbit_array* orbs, dyv* thresh) {
  ivec* res = mk_ivec(0);
  orb_tree_range_search(q,tr,orbs,thresh,res);
  return res;
}

/* Exhaustive search (for testing) */
ivec* mk_orb_tree_range_search_exh(orbit* q, orbit_array* orbs, 
                                   dyv* thresh, bool verb) {
  ivec* res = mk_ivec(0);
  orb_tree_range_search_exh(q,orbs,NULL,thresh,res,verb);
  return res;
}
