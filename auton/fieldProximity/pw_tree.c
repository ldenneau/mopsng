/*
   File:        pw_tree.c
   Author(s):   Kubica
   Created:     Mon June 8 2004
   Description: Tree data struture for holding piecewise linear trajectories.
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

#include "pw_tree.h"


double pw_linear_max_RADEC_dist(pw_linear* A, pw_linear* B, double ts,
                                double te, dyv* pw) {
  double max_dist = 0.0;
  double dist;
  int Na = pw_linear_size(A);
  int i;

  my_assert(Na == pw_linear_size(B));

  for(i=0;i<Na;i++) {
    if((pw_linear_x(A,i) >= ts)&&(pw_linear_x(A,i) <= te)) {
      dist = angular_distance_RADEC(pw_linear_y(A,i,0),
                                    pw_linear_y(B,i,0),
                                    pw_linear_y(A,i,1),
                                    pw_linear_y(B,i,1));
      if(dist > max_dist) { max_dist = dist; }
      if((pw != NULL)&&(dist > dyv_ref(pw,i))) {
        dyv_set(pw,i,dist);
      }
    }
  }

  return max_dist;
}


double pw_linear_max_RADEC_dist_knot(pw_linear* A, pw_linear* B, int knot) {
  double dist;

  dist = angular_distance_RADEC(pw_linear_y(A,knot,0),
                                pw_linear_y(B,knot,0),
                                pw_linear_y(A,knot,1),
                                pw_linear_y(B,knot,1));
  return dist;
}



/* --------------------------------------------------------------------- */
/* --- Tree Memory Functions ------------------------------------------- */
/* --------------------------------------------------------------------- */


pw_linear* mk_pw_tree_anchor_ave(pw_tree* tr, pw_linear_array* tarr, 
                                 ivec* inds, double ts, double te) {
  pw_linear *res, *X;
  dyv* vect = mk_dyv(2);
  double ra_mid, first_ra, de_mid, ra, t;
  int i, j, N, Na;

  Na  = pw_linear_size(pw_linear_array_ref(tarr,0));
  N   = ivec_size(inds);

  /* At each time find the "middle" track. */
  res = mk_empty_pw_linear(2);
  for(i=0;i<Na;i++) {
    X        = pw_linear_array_ref(tarr,ivec_ref(inds,0));
    first_ra = pw_linear_y(X,i,0);
    t        = pw_linear_x(X,i);
    ra_mid   = 0.0;
    de_mid   = 0.0;

    for(j=0;j<N;j++) {
      X = pw_linear_array_ref(tarr,ivec_ref(inds,j));

      /* Use RA's offset from the first point */
      ra = pw_linear_y(X,i,0);
      while(ra-first_ra >  12.0) { ra -= 24.0; }
      while(ra-first_ra < -12.0) { ra += 24.0; }

      ra_mid += ra;
      de_mid += pw_linear_y(X,i,1);
    }

    dyv_set(vect,0,ra_mid/(double)N);
    dyv_set(vect,1,de_mid/(double)N);
    pw_linear_add(res,t,vect);
  }
  
  free_dyv(vect);    

  return res;
}


void fill_pw_tree_bounds(pw_tree* tr, pw_linear_array* arr, pw_linear* A, 
                         ivec* inds, double ts, double te) {
  pw_linear* X;
  double dist;
  int i;

  /* Reset the radius */
  tr->radius = 0.0;
  if(tr->pw_radius != NULL) {
    free_dyv(tr->pw_radius);
  }
  tr->pw_radius = mk_zero_dyv(pw_linear_size(A));

  /* Check each pw_linear for a larger radius. */
  for(i=0;i<ivec_size(inds);i++) {
    X    = pw_linear_array_ref(arr,ivec_ref(inds,i));
    dist = pw_linear_max_RADEC_dist(X,A,ts,te,tr->pw_radius);

    if(dist > tr->radius) { tr->radius = dist; }
  }
}


pw_tree* mk_empty_pw_tree() {
  pw_tree* res = AM_MALLOC(pw_tree);

  res->radius     = 0.0;
  res->pw_radius  = NULL;

  res->anchor     = NULL;
  res->num_points = 0;
  
  res->left  = NULL;
  res->right = NULL;
  res->trcks = NULL;

  return res;
}


pw_linear* split_anchor_all(pw_linear_array* arr, ivec* inds, pw_linear* A,
                            double ts, double te) {
  double maxd = 0.0;
  double dist;
  int max_ind = -1;
  int i;

  for(i=0;i<ivec_size(inds);i++) {
    dist = pw_linear_max_RADEC_dist(A,pw_linear_array_ref(arr,ivec_ref(inds,i)),ts,te,NULL);
    if(dist >= maxd) {
      maxd = dist;
      max_ind = ivec_ref(inds,i);
    }
  }

  return pw_linear_array_ref(arr,max_ind);
}


pw_linear* split_anchor(pw_linear_array* arr, ivec* inds, pw_linear* A,
                        int knot) {
  pw_linear* B;
  double maxd = 0.0;
  double dist;
  int max_ind = -1;
  int i;

  for(i=0;i<ivec_size(inds);i++) {
    B = pw_linear_array_ref(arr,ivec_ref(inds,i));
    dist = angular_distance_RADEC(pw_linear_y(A,knot,0),
                                  pw_linear_y(B,knot,0),
                                  pw_linear_y(A,knot,1),
                                  pw_linear_y(B,knot,1));
    if(dist >= maxd) {
      maxd = dist;
      max_ind = ivec_ref(inds,i);
    }
  }

  return pw_linear_array_ref(arr,max_ind);
}


pw_tree* mk_pw_tree_recurse(pw_linear_array* arr, ivec* inds, 
                            double ts, double te, int max_leaf_pts,
                            bool split_all_dim) {
  pw_tree* res;
  pw_linear *A, *LA, *RA, *T;
  ivec *L, *R;
  double dR, dL;
  double max_r = 0.0;
  int knot = 0;
  int i;

  /* Pick an anchor pw_linear... */
  res = mk_empty_pw_tree();
  if(ivec_size(inds) > 1) {
    A = mk_pw_tree_anchor_ave(res,arr,inds,ts,te);
  } else {
    A = mk_copy_pw_linear(pw_linear_array_ref(arr,ivec_ref(inds,0)));
  }
  fill_pw_tree_bounds(res,arr,A,inds,ts,te);
  res->num_points = ivec_size(inds);
  res->anchor = A;

  /* Determine if this node will be a leaf or internal */
  if(ivec_size(inds) <= max_leaf_pts) {
    res->trcks = mk_copy_ivec(inds);
  } else {
    /* Find the widest knot point... */
    for(i=0;i<dyv_size(res->pw_radius);i++) {
      if(dyv_ref(res->pw_radius,i) > max_r) {
        max_r = dyv_ref(res->pw_radius,i);
        knot = i;
      }
    }

    /* Find the two new anchor points... */
    if(ivec_size(inds) > 2) {
      if(split_all_dim == TRUE) {
        RA = split_anchor_all(arr,inds,A,ts,te);
        LA = split_anchor_all(arr,inds,RA,ts,te);
      } else {
        RA = split_anchor(arr,inds,A,knot);
        LA = split_anchor(arr,inds,RA,knot);
      }
    } else {
      RA = pw_linear_array_ref(arr,ivec_ref(inds,0));
      LA = pw_linear_array_ref(arr,ivec_ref(inds,1));
    }

    /* Actually divide up the points. */
    L = mk_ivec(0);
    R = mk_ivec(0);

    for(i=0;i<ivec_size(inds);i++) {
      T  = pw_linear_array_ref(arr,ivec_ref(inds,i));

      if(split_all_dim == TRUE) {
        dR = pw_linear_max_RADEC_dist(T,RA,ts,te,NULL);
        dL = pw_linear_max_RADEC_dist(T,LA,ts,te,NULL);
      } else {
        dR = angular_distance_RADEC(pw_linear_y(T,knot,0),
                                    pw_linear_y(RA,knot,0),
                                    pw_linear_y(T,knot,1),
                                    pw_linear_y(RA,knot,1));
        dL = angular_distance_RADEC(pw_linear_y(T,knot,0),
                                    pw_linear_y(LA,knot,0),
                                    pw_linear_y(T,knot,1),
                                    pw_linear_y(LA,knot,1));
      }

      if(dR < dL) {
        add_to_ivec(R,ivec_ref(inds,i));
      } else {
        add_to_ivec(L,ivec_ref(inds,i));
      }
    }

    /* Build the left and right sub-trees */
    res->left  = mk_pw_tree_recurse(arr,L,ts,te,max_leaf_pts,split_all_dim);
    res->right = mk_pw_tree_recurse(arr,R,ts,te,max_leaf_pts,split_all_dim);

    free_ivec(L);
    free_ivec(R);
  }

  return res;
}


pw_tree* mk_pw_tree(pw_linear_array* arr, double ts, double te, 
                    int max_leaf_pts, bool split_all_dim) {
  pw_tree  *res;
  ivec     *inds;
  int N = pw_linear_array_size(arr);

  /* Store all the indices for the tree. */
  inds  = mk_sequence_ivec(0,N);

  /* Build the tree. */
  res  = mk_pw_tree_recurse(arr, inds, ts, te, max_leaf_pts, split_all_dim);

  /* Free the used memory */
  free_ivec(inds);

  return res;
}

void free_pw_tree(pw_tree* old) {
  if(old->anchor)    { free_pw_linear(old->anchor); }
  if(old->left)      { free_pw_tree(old->left);     }
  if(old->right)     { free_pw_tree(old->right);    }
  if(old->trcks)     { free_ivec(old->trcks);       }
  if(old->pw_radius) { free_dyv(old->pw_radius);    }

  AM_FREE(old,pw_tree);
}




/* --------------------------------------------------------------------- */
/* --- Getter/Setter Functions ----------------------------------------- */
/* --------------------------------------------------------------------- */

int safe_pw_tree_N(pw_tree* tr) { return tr->num_points; }
int safe_pw_tree_num_points(pw_tree* tr)   { return tr->num_points; }

bool safe_pw_tree_is_leaf(pw_tree* tr) { return (tr->left == NULL); }
ivec* safe_pw_tree_tracks(pw_tree* tr) { return tr->trcks; }

pw_tree* safe_pw_tree_right_child(pw_tree* tr) { return tr->right; }
pw_tree* safe_pw_tree_left_child(pw_tree* tr)  { return tr->left; }

double safe_pw_tree_radius(pw_tree* tr) { return tr->radius; }
double safe_pw_tree_pw_radius(pw_tree* tr, int i) {
  return dyv_ref(tr->pw_radius,i);
}

pw_linear* safe_pw_tree_anchor(pw_tree* tr) { return tr->anchor; }
double safe_pw_tree_anchor_predict(pw_tree* tr, int dim, double t) {
  return pw_linear_predict(tr->anchor,t,dim);
}


/* --- Simple I/O Functions ------------------------------------ */

void fprintf_pw_tree_pts_recurse(FILE* f, pw_tree* tr, int depth) {
  ivec* inds;
  int i;

  /* Do the correct indenting */
  for(i=0;i<depth;i++) { fprintf(f,"-"); }

  if(pw_tree_is_leaf(tr)) {
    inds = pw_tree_tracks(tr);
    fprintf(f," LEAF:");
    for(i=0;i<ivec_size(inds);i++) { fprintf(f,"%i ",ivec_ref(inds,i)); }
    fprintf(f,"\n");
  } else {
    fprintf(f," INTERNAL\n");
    fprintf_pw_tree_pts_recurse(f,pw_tree_left_child(tr),depth+1);
    fprintf_pw_tree_pts_recurse(f,pw_tree_right_child(tr),depth+1);
  }
}


void fprintf_pw_tree_pts(FILE* f, pw_tree* tr) {
  fprintf_pw_tree_pts_recurse(f,tr,1);
}
