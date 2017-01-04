/*
   File:        plate_tree.h
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

#include "plate_tree.h"

/* --------------------------------------------------------------------- */
/* --- Tree Memory Functions ------------------------------------------- */
/* --------------------------------------------------------------------- */

double plate_tree_radius_given_anchor(rd_plate_array* arr, ivec* inds, 
				      double ra, double dec) {
  rd_plate* X;
  double radius = 0.0;
  double dist   = 0.0;
  int i;

  for(i=0;i<ivec_size(inds);i++) {
    X    = rd_plate_array_ref(arr,ivec_ref(inds,i));
    dist = angular_distance_RADEC(rd_plate_RA(X),ra,rd_plate_DEC(X),dec);
    dist += rd_plate_radius(X);
    if(dist > radius) { radius = dist; }
  }

  return radius;
}


void fill_plate_tree_bounds(plate_tree* tr, rd_plate_array* arr, ivec* inds,
			    double* t_wid, double* t_mid,
			    double* r_wid, double* r_mid,
			    double* d_wid, double* d_mid) {
  double r_lo = 0.0;
  double r_hi = 0.0;
  double d_lo = 0.0;
  double d_hi = 0.0;
  double r,d,t,rad;
  rd_plate* X;
  int i, ind;

  /* Set the initial values. */
  if(ivec_size(inds) > 0) {
    X = rd_plate_array_ref(arr,ivec_ref(inds,0));
    t = rd_plate_time(X);
    d = rd_plate_DEC(X);
    r = rd_plate_RA(X);

    tr->t_lo = t; tr->t_hi = t;
    d_hi = d; d_lo = d;
    r_hi = r; r_lo = r;
  }
        
  for(i=1;i<ivec_size(inds);i++) {
    ind = ivec_ref(inds,i);
    X   = rd_plate_array_ref(arr,ind);
    t   = rd_plate_time(X);
    d   = rd_plate_DEC(X);
    r   = rd_plate_RA(X);

    if(t > tr->t_hi) { tr->t_hi = t; }
    if(t < tr->t_lo) { tr->t_lo = t; }
    
    if(r > r_hi) { r_hi = r; }
    if(r < r_lo) { r_lo = r; }

    if(d > d_hi) { d_hi = d; }
    if(d < d_lo) { d_lo = d; }
  }

  t_mid[0] = (tr->t_hi + tr->t_lo)/2.0;
  t_wid[0] = (tr->t_hi - tr->t_lo)/2.0;
  d_mid[0] = (d_hi + d_lo)/2.0;
  d_wid[0] = (d_hi - d_lo)/2.0;
  r_mid[0] = (r_hi + r_lo)/2.0;
  r_wid[0] = (r_hi - r_lo)/2.0 * cos(d_mid[0] * DEG_TO_RAD);
  
  tr->ra     = r_mid[0];
  tr->dec    = d_mid[0];
  tr->radius = 0.0;

  for(i=0;i<ivec_size(inds);i++) {
    X   = rd_plate_array_ref(arr,ivec_ref(inds,i));
    rad = angular_distance_RADEC(rd_plate_RA(X),tr->ra,rd_plate_DEC(X),tr->dec);
    rad += rd_plate_radius(X);
    if(rad > tr->radius) { tr->radius = rad; }
  }
}


plate_tree* mk_empty_plate_tree() {
  plate_tree* res = AM_MALLOC(plate_tree);
 
  res->t_hi = 0.0;
  res->t_lo = 0.0;
  
  res->ra     = 0.0;
  res->dec    = 0.0;
  res->radius = 0.0;

  res->num_points =  0;  
  res->left       = NULL;
  res->right      = NULL;
  res->plates     = NULL;

  return res;
}


/* wt, wr, wd represent weight/width for time, RA, and DEC */
plate_tree* mk_plate_tree_recurse(rd_plate_array* arr, ivec* inds, 
                                  double wt, double wr, double wd,
                                  int max_leaf_pts) {
  rd_plate* X;
  plate_tree* res;
  ivec *left, *right;
  double t_wid, t_mid;
  double r_wid, r_mid;
  double d_wid, d_mid;
  double sv, sw, val;
  int sd, i;

  /* Allocate space for the tree and calculate the bounds of the node */
  res = mk_empty_plate_tree();
  fill_plate_tree_bounds(res,arr,inds,&t_wid,&t_mid,
                         &r_wid,&r_mid,&d_wid,&d_mid);
  res->num_points = ivec_size(inds);
  
  /* Determine if this node will be a leaf or internal */
  if((ivec_size(inds) <= max_leaf_pts)||(t_wid+r_wid+d_wid < 1e-10)) {
    res->plates = mk_copy_ivec(inds);
  } else {
    sd = 0; sw = t_wid*wt; sv = t_mid;
    if(r_wid*wr > sw) { sd = 1; sw = r_wid*wr; sv = r_mid; }
    if(d_wid*wd > sw) { sd = 2; sw = d_wid*wd; sv = d_mid; }

    left  = mk_ivec(0);
    right = mk_ivec(0);
    val   = 0.0;

    for(i=0;i<ivec_size(inds);i++) {
      X   = rd_plate_array_ref(arr,ivec_ref(inds,i));
      
      switch(sd) {
      case 0: val = rd_plate_time(X); break;
      case 1: val = rd_plate_RA(X); break;
      case 2: val = rd_plate_DEC(X); break;
      }

      if(val < sv) {
	add_to_ivec(left,ivec_ref(inds,i));
      } else {
	add_to_ivec(right,ivec_ref(inds,i));
      }
    }

    /* Build the left and right sub-trees */
    res->left  = mk_plate_tree_recurse(arr,left,wt,wr,wd,max_leaf_pts);
    res->right = mk_plate_tree_recurse(arr,right,wt,wr,wd,max_leaf_pts);

    free_ivec(left);
    free_ivec(right);
  }

  return res;
}


/* wt, wr, wd are the relative weights of time, RA, and DEC  */
/* a high value means pay a lot of attention a value of zero */
/* means that attribute will be ignored for splitting.       */
plate_tree* mk_plate_tree(rd_plate_array* arr, double wt,
			  double wr, double wd, int max_leaf_pts) {
  plate_tree  *res;
  ivec   *inds;
  double t_wid, t_mid;
  double r_wid, r_mid;
  double d_wid, d_mid;
  int    N = rd_plate_array_size(arr);

  /* Store all the indices for the tree. */
  inds  = mk_sequence_ivec(0,N);

  /* Calculate the initial width of each factor. */
  res   = mk_empty_plate_tree();
  fill_plate_tree_bounds(res,arr,inds,&t_wid,&t_mid,
                         &r_wid,&r_mid,&d_wid,&d_mid);
  if(t_wid < 1e-20) { t_wid = 1e-20; }
  if(r_wid < 1e-20) { r_wid = 1e-20; }
  if(d_wid < 1e-20) { d_wid = 1e-20; }
  wt /= t_wid;
  wr /= r_wid;
  wd /= d_wid;
  free_plate_tree(res);

  /* Build the tree. */
  res  = mk_plate_tree_recurse(arr, inds, wt, wr, wd, max_leaf_pts);

  /* Free the used memory */
  free_ivec(inds);

  return res;
}


void free_plate_tree(plate_tree* old) {
  if(old->left)  { free_plate_tree(old->left);  }
  if(old->right) { free_plate_tree(old->right); }
  if(old->plates) { free_ivec(old->plates);  }

  AM_FREE(old,plate_tree);
}




/* --------------------------------------------------------------------- */
/* --- Getter/Setter Functions ----------------------------------------- */
/* --------------------------------------------------------------------- */

int safe_plate_tree_N(plate_tree* tr) { return tr->num_points; }
int safe_plate_tree_num_points(plate_tree* tr)   { return tr->num_points; }

bool safe_plate_tree_is_leaf(plate_tree* tr) { return (tr->left == NULL); }
ivec* safe_plate_tree_rd_plates(plate_tree* tr) { return tr->plates; }

plate_tree* safe_plate_tree_right_child(plate_tree* tr) { return tr->left; }
plate_tree* safe_plate_tree_left_child(plate_tree* tr)  { return tr->right; }

double safe_plate_tree_RA(plate_tree* p)   { return p->ra; }
double safe_plate_tree_DEC(plate_tree* p)  { return p->dec; }
double safe_plate_tree_lo_time(plate_tree* p) { return p->t_lo; }
double safe_plate_tree_hi_time(plate_tree* p) { return p->t_hi; }
double safe_plate_tree_radius(plate_tree* p)  { return p->radius; }


/* --- Simple I/O Functions ------------------------------------ */

void fprintf_plate_tree_pts_recurse(FILE* f, plate_tree* tr, int depth) {
  ivec* inds;
  int i;

  /* Do the correct indenting */
  for(i=0;i<depth;i++) { fprintf(f,"-"); }

  if(plate_tree_is_leaf(tr)) {
    inds = plate_tree_rd_plates(tr);
    fprintf(f," LEAF:");
    for(i=0;i<ivec_size(inds);i++) { fprintf(f,"%i ",ivec_ref(inds,i)); }
    fprintf(f,"\n");
  } else {
    fprintf(f," INTERNAL\n");
    fprintf_plate_tree_pts_recurse(f,plate_tree_left_child(tr),depth+1);
    fprintf_plate_tree_pts_recurse(f,plate_tree_right_child(tr),depth+1);
  }
}


void fprintf_plate_tree_pts(FILE* f, plate_tree* tr) {
  fprintf_plate_tree_pts_recurse(f,tr,1);
}


void fprintf_plate_tree_recurse(FILE* f, plate_tree* tr, int depth) {
  ivec* inds;
  int i;

  /* Do the correct indenting */
  for(i=0;i<depth;i++) { fprintf(f,"-"); }

  if(plate_tree_is_leaf(tr)) {
    inds = plate_tree_rd_plates(tr);
    fprintf_plate_tree_node(f,"LEAF: ",tr," ");
    for(i=0;i<ivec_size(inds);i++) { fprintf(f,"%i ",ivec_ref(inds,i)); }
    fprintf(f,"\n");
  } else {
    fprintf_plate_tree_node(f,"INT:  ",tr,"\n"); 
    fprintf_plate_tree_recurse(f,plate_tree_left_child(tr),depth+1);
    fprintf_plate_tree_recurse(f,plate_tree_right_child(tr),depth+1);
  }
}


void fprintf_plate_tree(FILE* f, plate_tree* tr) {
  fprintf_plate_tree_recurse(f,tr,1);
}


void fprintf_plate_tree_node(FILE* f, char* pre, plate_tree* tr, char* post) {
  fprintf(f,pre);
  fprintf(f,"T=[%10.6f,%10.6f]  (%12.8f,%12.8f)  RAD=%12.8f",
          plate_tree_lo_time(tr),plate_tree_hi_time(tr),
          plate_tree_RA(tr),plate_tree_DEC(tr),plate_tree_radius(tr));
  fprintf(f,post);
}


void test_plate_tree() {
  rd_plate_array* arr;
  plate_tree* tr;

  arr = mk_load_rd_plate_array("test.plates",FALSE);
  fprintf_rd_plate_array(stdout,arr);

  tr = mk_plate_tree(arr,2.0,1.0,1.0,1);
  fprintf_plate_tree(stdout,tr);

  free_plate_tree(tr);
  free_rd_plate_array(arr);
}

