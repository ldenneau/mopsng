/*
File:         pw_linear.c
Author:       J. Kubica
Created:      Thu Nov. 11, 2004
Description:  A cached piecewise linear approximation.

Copyright 2004, The Auton Lab, CMU

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

#include "pw_linear.h"

/* ---------------------------------------------------- */
/* --- Functions for the piecewise linear approx. ----- */
/* ---------------------------------------------------- */

/* --- Memory Functions ------------------------------- */

/* Create an empty pw_linear structure of dimension D. */
pw_linear* mk_empty_pw_linear(int D) {
  return mk_sized_empty_pw_linear(PW_LINEAR_START_SIZE,D);
}


pw_linear* mk_sized_empty_pw_linear(int N, int D) {
  pw_linear* res = AM_MALLOC(pw_linear);
  int i;

  my_assert(D > 0);

  /* Allocate memory for the arrays */
  res->x = AM_MALLOC_ARRAY(double,N);
  res->y = AM_MALLOC_ARRAY(double*,N);
  for(i=0;i<N;i++) {
    res->y[i] = NULL;
  }

  /* Set the size */
  res->max_points = N;
  res->num_points = 0;
  res->D          = D;

  return res;
}


pw_linear* mk_copy_pw_linear(pw_linear* old) {
  pw_linear* nu;
  int N = old->num_points;
  int D = old->D;
  int i, j;

  nu = mk_sized_empty_pw_linear(N,D);

  for(i=0;i<N;i++) {
    if(old->y[i] != NULL) {
      nu->y[i] = AM_MALLOC_ARRAY(double,D);
      for(j=0;j<D;j++) {
        nu->y[i][j] = old->y[i][j];
      }
    }
    nu->x[i] = old->x[i];
  }

  nu->num_points = N;

  return nu;
}


/* Generates tracks with X0 between Lbnd and Ubnd     */
/* and velocity in each dimension generated as normal */
/* with initial sigma = sig_initV and modified each   */
/* step as sig_V. keepbnds indicates whether to use   */
/* the bounds for all time (1) or just x0 (0).        */
pw_linear* mk_random_pw_linear(dyv* times, dyv* Lbnd, dyv* Ubnd, 
                               dyv* sig_initV, dyv* sig_V, ivec* keepbnds) {
  pw_linear* res;
  dyv* pts;
  dyv* vel;
  double dt;
  int D = dyv_size(Lbnd);
  int T = dyv_size(times);
  int i, d;
 
  res = mk_empty_pw_linear(D);
  pts = mk_dyv(D);
  vel = mk_dyv(D);

  /* Generate the first point. */
  for(d=0;d<D;d++) {
    dyv_set(pts,d,range_random(dyv_ref(Lbnd,d),dyv_ref(Ubnd,d)));
    dyv_set(vel,d,gen_gauss()*dyv_ref(sig_initV,d));
  }
  pw_linear_add(res,dyv_ref(times,0),pts);

  /* Use the velocity to generate the remaining points. */
  for(i=1;i<T;i++) {
    dt = dyv_ref(times,i) - dyv_ref(times,i-1);
    for(d=0;d<D;d++) {
      dyv_set(pts,d,dyv_ref(pts,d)+dt*dyv_ref(vel,d));
      if(ivec_ref(keepbnds,d)==1) {
        if(dyv_ref(pts,d) > dyv_ref(Ubnd,d)) { 
          dyv_set(pts,d,dyv_ref(Ubnd,d));
          dyv_set(vel,d,0.0);
        }
        if(dyv_ref(pts,d) < dyv_ref(Lbnd,d)) { 
          dyv_set(pts,d,dyv_ref(Lbnd,d));
          dyv_set(vel,d,0.0);
        }
      }
      dyv_set(vel,d,dyv_ref(vel,d)+gen_gauss()*dyv_ref(sig_V,d));
    }
    pw_linear_add(res,dyv_ref(times,i),pts);
  }

  free_dyv(pts); 
  free_dyv(vel); 
 
  return res;
}


void free_pw_linear(pw_linear* old) {
  int N = old->num_points;
  int D = old->D;
  int i;

  for(i=0;i<N;i++) {
    if(old->y[i] != NULL) {
      AM_FREE_ARRAY(old->y[i],double,D);
    }
  }
  AM_FREE_ARRAY(old->y,double*,old->max_points);
  AM_FREE_ARRAY(old->x,double,old->max_points);
  
  AM_FREE(old,pw_linear);
}


/* --- Helper Functions ----------------------------------- */

void pw_linear_double_size(pw_linear* old) {
  double** nu_y;
  double*  nu_x;
  int N = old->max_points;
  int D = old->D;
  int i, j;  

  /* Create new arrays and copy the data in */
  nu_y = AM_MALLOC_ARRAY(double*,2*N);
  nu_x = AM_MALLOC_ARRAY(double,2*N);
  for(i=0;i<old->num_points;i++) {
    if(old->y[i] != NULL) {
      nu_x[i] = old->x[i];
      nu_y[i] = AM_MALLOC_ARRAY(double,D);
      for(j=0;j<D;j++) {
        nu_y[i][j] = old->y[i][j];
      }
    }
  }
  for(i=old->num_points;i<2*N;i++) {
    nu_y[i] = NULL;
  }

  /* Free the old arrays */
  for(i=0;i<old->num_points;i++) {
    if(old->y[i] != NULL) {
      AM_FREE_ARRAY(old->y[i],double,D);
    }
  }
  AM_FREE_ARRAY(old->y,double*,N);
  AM_FREE_ARRAY(old->x,double,N);

  /* Assign the new data structures */
  old->x = nu_x;
  old->y = nu_y;
  old->max_points = 2*N;
}


/* Find the index of first x that is larger than   */
/* the given value (returns N if x is larger than  */
/* everything else in the array already).          */
int pw_linear_first_larger_x(pw_linear* pw, double val) {
  int res   = pw_linear_size(pw);
  int e     = pw_linear_size(pw)-1;
  int s     = 0;
  int mlast = e;
  int m     = 0;

  if(pw_linear_size(pw) > 0) {
  
    /* Do a binary search. */
    while(m != mlast) {
      mlast = m;
      m     = (int)((e+s)/2);
      if(pw->x[m] > val) { e = m; } else { s = m; }
    }
    res = e;

    /* Check the boundry condition that val */
    /* is not BIGGER than the last element. */
    if(pw->x[e] < val) { res = pw_linear_size(pw); }
  }

  return res;
}


/* --- Getter/Setter Functions ------------------------ */

int safe_pw_linear_D(pw_linear* pw) {
  return pw->D;
}

int safe_pw_linear_size(pw_linear* pw) { 
  return pw->num_points;
}

double safe_pw_linear_x(pw_linear* pw, int i) {
  my_assert((i >= 0)&&(i < pw->num_points));
  return pw->x[i];
}

double safe_pw_linear_y(pw_linear* pw, int i, int dim) {
  my_assert((i >= 0)&&(i < pw->num_points));
  my_assert((dim >= 0)&&(dim < pw->D));

  return pw->y[i][dim];
}

void pw_linear_add(pw_linear* pw, double x, dyv* y) {
  int i, j, ind;

  my_assert(dyv_size(y) == pw_linear_D(pw));

  /* Check if we need to double the array size */
  if(pw->num_points == pw->max_points) {
    pw_linear_double_size(pw);   
  }

  /* Add a new row. */
  pw->y[pw->num_points] = AM_MALLOC_ARRAY(double,pw->D);

  /* Find the correct index to insert */
  ind = pw_linear_first_larger_x(pw, x);

  /* Start at the last entry and shift right */
  for(i=pw->num_points-1; (i >= ind); i--) {
    pw->x[i+1] = pw->x[i];
    for(j=0;j<pw->D;j++) {
      pw->y[i+1][j] = pw->y[i][j];
    }
  }

  /* Set the new element at ind */
  pw->x[ind] = x;
  for(j=0;j<pw->D;j++) {
    pw->y[ind][j] = dyv_ref(y,j);
  }

  pw->num_points += 1.0;
}


/* --- Prediction Functions --------------------------- */

double predict_given_se(pw_linear* pw, double x, int s, int e, int dim) {
  double dx, dy, m, b;

  dx = pw->x[e] - pw->x[s];
  dy = pw->y[e][dim] - pw->y[s][dim];
  if(dx < 1e-20) { dx = 1e-20; }

  m = dy/dx;
  b = pw->y[s][dim] - m * pw->x[s];

  return m*x+b;
}


dyv* mk_pw_linear_predict(pw_linear* pw, double x) {
  dyv* res = mk_zero_dyv(pw->D);
  int s, e, d;

  my_assert(pw->num_points > 1);

  s = pw_linear_first_larger_x(pw, x) - 1;
  if(s < 0) { s = 0; }
  if(s > pw->num_points-2) { s = pw->num_points-2; }
  e = s+1;

  for(d=0;d<pw->D;d++) {
    dyv_set(res,d,predict_given_se(pw,x,s,e,d));
  }

  return res;
}


double pw_linear_predict(pw_linear* pw, double x, int dim) {
  double res = 0.0;
  int s, e;

  my_assert((dim >= 0)&&(dim < pw->D));
  my_assert(pw->num_points > 1);

  s = pw_linear_first_larger_x(pw, x) - 1;
  if(s < 0) { s = 0; }
  if(s > pw->num_points-2) { s = pw->num_points-2; }
  e = s+1;

  res = predict_given_se(pw,x,s,e,dim);

  return res;
}


/* Returns the knot points used... */
double pw_linear_predict_full(pw_linear* pw, double x, int dim, int* s, int *e) {
  double res = 0.0;

  my_assert((dim >= 0)&&(dim < pw->D));
  my_assert(pw->num_points > 1);

  s[0] = pw_linear_first_larger_x(pw, x) - 1;
  if(s[0] < 0) { s[0] = 0; }
  if(s[0] > pw->num_points-2) { s[0] = pw->num_points-2; }
  e[0] = s[0]+1;

  res = predict_given_se(pw,x,s[0],e[0],dim);

  return res;
}


/* Convert pw_linear to a "smooth" RA/DEC function. */
/* In other words each transition is assumed to be  */
/* < 12.0 in RA                                     */
void pw_linear_conv_RADEC(pw_linear* pw) {
  double last, curr;
  int i;

  if(pw_linear_size(pw) > 1) {
    last = pw_linear_y(pw,0,0);
    
    for(i=1;i<pw_linear_size(pw);i++) {
      curr = pw_linear_y(pw,i,0);
      while((curr - last) < -12.0) { curr += 24.0; }
      while((curr - last) >  12.0) { curr -= 24.0; }

      pw->y[i][0] = curr;
      curr = last;
    }
  }

}


/* --- I/O Functions ---------------------------------- */

void fprintf_pw_linear(FILE* f, char* pre, pw_linear* pw, char* post) {
  int i,d;

  fprintf(f,pre);
  
  for(i=0;i<pw->num_points;i++) {
    fprintf(f,"%10.6f) ",pw->x[i]);
    for(d=0;d<pw->D;d++) {
      fprintf(f,"%6.4f ",pw->y[i][d]);
    }
    fprintf(f,"\n");
  }

  fprintf(f,post);
}


/* ---------------------------------------------------- */
/* --- Functions for the piecewise linear array. ------ */
/* ---------------------------------------------------- */


pw_linear_array* mk_empty_pw_linear_array() {
  return mk_empty_pw_linear_array_sized(PW_LINEAR_ARRAY_SIZE);
}

pw_linear_array* mk_empty_pw_linear_array_sized(int size) {
  pw_linear_array* res = AM_MALLOC(pw_linear_array);
  int i;

  res->size = 0;
  res->max_size = size;

  res->arr = AM_MALLOC_ARRAY(pw_linear*,size);
  for(i=0;i<size;i++) {
    res->arr[i] = NULL;
  }

  return res;
}


pw_linear_array* mk_copy_pw_linear_array(pw_linear_array* old) {
  pw_linear_array* res = mk_empty_pw_linear_array_sized(old->max_size);
  int i;

  res->size = old->size;
  for(i=0;i<res->size;i++) {
    if(old->arr[i]) {
      res->arr[i] = mk_copy_pw_linear(old->arr[i]);
    }
  }

  return res;
}


pw_linear_array* mk_pw_linear_array_subset(pw_linear_array* old, ivec* inds) {
  pw_linear_array* res = mk_empty_pw_linear_array_sized(ivec_size(inds));
  int i, ind;

  for(i=0;i<ivec_size(inds);i++) {
    ind = ivec_ref(inds,i);
    if((ind < old->size)&&(ind >= 0)&&(old->arr[ind])) {
      pw_linear_array_add(res,old->arr[ind]);
    }
  }

  return res;
}


pw_linear_array* mk_pw_linear_array_concat(pw_linear_array* A, pw_linear_array* B) {
  pw_linear_array* res = mk_empty_pw_linear_array_sized(A->size + B->size);
  int i;

  for(i=0;i<A->size;i++) {
    pw_linear_array_add(res,A->arr[i]);
  }
  for(i=0;i<B->size;i++) {
    pw_linear_array_add(res,B->arr[i]);
  }

  return res;
}


void free_pw_linear_array(pw_linear_array* old) {
  int i;

  for(i=0;i<old->max_size;i++) {
    if(old->arr[i]) { free_pw_linear(old->arr[i]); }
  }

  AM_FREE_ARRAY(old->arr,pw_linear*,old->max_size);
  AM_FREE(old,pw_linear_array);
}


/* Generates tracks with X0 between Lbnd and Ubnd     */
/* and velocity in each dimension generated as normal */
/* with initial sigma = sig_initV and modified each   */
/* step as sig_V. keepbnds indicates whether to use   */
/* the bounds for all time (1) or just x0 (0).        */
pw_linear_array* mk_random_pw_linear_array(int N, dyv* times, dyv* Lbnd, 
                                           dyv* Ubnd, dyv* sig_initV, 
                                           dyv* sig_V, ivec* keepbnds) {
  pw_linear_array* res = mk_empty_pw_linear_array_sized(N+1);
  pw_linear* trck;
  int i;

  for(i=0;i<N;i++) {
    trck = mk_random_pw_linear(times,Lbnd,Ubnd,sig_initV,sig_V,keepbnds);
    pw_linear_array_add(res,trck);
    free_pw_linear(trck);
  }

  return res;
}


pw_linear* safe_pw_linear_array_ref(pw_linear_array* X, int index) {
  my_assert((X->max_size > index)&&(index >= 0));
  return X->arr[index];
}


pw_linear* safe_pw_linear_array_first(pw_linear_array* X) {
  return pw_linear_array_ref(X,0);
}


pw_linear* safe_pw_linear_array_last(pw_linear_array* X) {
  return pw_linear_array_ref(X,X->size-1);
}


void pw_linear_array_double_size(pw_linear_array* old) {
  pw_linear** nu_arr;
  int i;

  nu_arr = AM_MALLOC_ARRAY(pw_linear*,2*old->max_size+1);
  for(i=0;i<old->size;i++) {
    nu_arr[i] = old->arr[i];
  }
  for(i=old->size;i<2*old->max_size+1;i++) {
    nu_arr[i] = NULL;
  }

  AM_FREE_ARRAY(old->arr,pw_linear*,old->max_size);

  old->arr  = nu_arr;
  old->max_size = 2*old->max_size+1;
}

void pw_linear_array_set(pw_linear_array* X, int index, pw_linear* A) {
  my_assert(index >= 0);

  while(index >= X->max_size) { pw_linear_array_double_size(X); }

  if(X->arr[index]) { free_pw_linear(X->arr[index]); }
  if(index >= X->size) { X->size = index+1; }

  if(A != NULL) {
    X->arr[index] = mk_copy_pw_linear(A);
  } else {
    X->arr[index] = NULL;
  }
}


void pw_linear_array_add(pw_linear_array* X, pw_linear* A) {
  pw_linear_array_set(X,X->size,A);
}


void pw_linear_array_add_many(pw_linear_array* X, pw_linear_array* nu) {
  int i;

  for(i=0;i<pw_linear_array_size(nu);i++) {
    if(pw_linear_array_ref(nu,i) != NULL) {
      pw_linear_array_add(X,pw_linear_array_ref(nu,i));
    }
  }
}

int safe_pw_linear_array_size(pw_linear_array* X) {
  return X->size;
}


int safe_pw_linear_array_max_size(pw_linear_array* X) {
  return X->max_size;
}


int pw_linear_array_number_nonnull(pw_linear_array* X) {
  int count = 0;
  int i;

  for(i=0;i<pw_linear_array_size(X);i++) {
    if(pw_linear_array_ref(X,i) != NULL) {
      count++;
    }
  }

  return count;
}

void fprintf_pw_linear_array(FILE* f, pw_linear_array* X) {
  int i;

  for(i=0;i<X->size;i++) {
    fprintf(f,"%6i) ",i);
    if(X->arr[i]) {
      fprintf_pw_linear(f,"",X->arr[i],"\n");
    } else {
      fprintf(f,"EMPTY!\n");
    }
  }
}


/* Reads in an array of pw_linears where each line contains */
/* TRACK_ID TIME RA DEC where TRACK_ID is an 8 character ID */
/* names - an empty name to store the track id to inds.     */
/*         CAN BE set to NULL (and thus ignored).           */
/* RAdegrees - true off RA is given in degrees.  Otherwise  */
/*             is if given in hours.                        */
pw_linear_array* mk_load_RA_DEC_pw_linear_array(char* filename, namer* names, bool RAdegrees) {
  pw_linear_array* res = NULL;
  pw_linear*       indiv;
  namer*           ids_to_inds;
  FILE *fp = fopen(filename,"r");
  int line_number = 0;
  int i, ind, L;
  char *s;
  char *id = NULL;
  dyv *pt, *nums;
  bool givennamer;

  if(names == NULL) {
    givennamer  = FALSE;
    ids_to_inds = mk_empty_namer(FALSE);
  } else {
    my_assert(namer_num_indexes(names)==0);

    givennamer  = TRUE;
    ids_to_inds = names;
  }  

  if (!fp) {
    printf("ERROR: Unable to open PW LINEAR file (");
    printf(filename);
    printf(") for reading.\n");
  } else {

    /* Allocate memory */
    res         = mk_empty_pw_linear_array_sized(10);
    pt          = mk_dyv(2);

    while((s = mk_next_interesting_line_string(fp,&line_number))) {
      /* Extract the ID string. */
      L = strlen(s); i = 0;
      if(L > 1) {

        while((i < L)&&(s[i] != ' ')) { i++; }
        if(i==L) { i--; }

        if(i > 0) {
          id = mk_substring(s,0,i);
          while(i >= 0) {
            s[i] = ' ';
            i--;
          }
        } else {
          id = mk_printf("FAKEID");
        }
        ind = namer_name_to_index(ids_to_inds,id);

        /* Extract the coordinates... */
        nums = mk_dyv_from_string(s,NULL);
        if((nums != NULL)&&(dyv_size(nums) >= 3)) {
          if(RAdegrees) {
            dyv_set(pt,0,dyv_ref(nums,1)/15.0);
          } else {
            dyv_set(pt,0,dyv_ref(nums,1));
          }
          dyv_set(pt,1,dyv_ref(nums,2));

          /* It we have seen this track before augment */
          /* it otherwise create a new track.          */
          if(ind > -1) {          
            indiv = pw_linear_array_ref(res,ind);
            pw_linear_add(indiv,dyv_ref(nums,0),pt);
          } else {
            indiv = mk_empty_pw_linear(2);
            pw_linear_add(indiv,dyv_ref(nums,0),pt);

            pw_linear_array_add(res,indiv);
            add_to_namer(ids_to_inds,id);
           
            free_pw_linear(indiv);
          }

          free_dyv(nums);
        } else {
          printf("WARNING: Bad Input In Line: %s\n",s);
        }
      } else {
        printf("WARNING: Bad Input Line: %s\n",s);
      }

      if(id != NULL) { free_string(id); id = NULL; }
      free_string(s);
    }

    free_dyv(pt);
  }

  /* Make sure nothing weird happens at the 24.0->0.0 line */
  if(res != NULL) {
    for(i=0;i<pw_linear_array_size(res);i++) {
      pw_linear_conv_RADEC(pw_linear_array_ref(res,i));
    }
  }

  /* Free memory */
  if(givennamer==FALSE) { free_namer(ids_to_inds); }

  return res;
}


void add_to_RA_DEC_pw_linear_array(pw_linear_array* res, char* filename, 
                                   namer* ids_to_inds, bool RAdegrees) {
  pw_linear*       indiv;
  FILE *fp = fopen(filename,"r");
  int line_number = 0;
  int i, ind, L;
  char *s;
  char *id = NULL;
  dyv *pt, *nums;

  if (!fp) {
    printf("ERROR: Unable to open PW LINEAR file (");
    printf(filename);
    printf(") for reading.\n");
  } else {

    /* Allocate memory */
    pt = mk_dyv(2);

    while((s = mk_next_interesting_line_string(fp,&line_number))) {
 
      /* Extract the ID string. */
      L = strlen(s); i = 0;
      if(L > 1) {

        while((i < L)&&(s[i] != ' ')) { i++; }
        if(i==L) { i--; }

        if(i > 0) {
          id = mk_substring(s,0,i);
          while(i >= 0) {
            s[i] = ' ';
            i--;
          }
        } else {
          id = mk_printf("FAKEID");
        }
        ind = namer_name_to_index(ids_to_inds,id);

        /* Extract the coordinates... */
        nums = mk_dyv_from_string(s,NULL);
        if((nums != NULL)&&(dyv_size(nums) >= 3)) {

          if(RAdegrees) {
            dyv_set(pt,0,dyv_ref(nums,1)/15.0);
          } else {
            dyv_set(pt,0,dyv_ref(nums,1));
          }
          dyv_set(pt,1,dyv_ref(nums,2));

          /* It we have seen this track before augment */
          /* it otherwise create a new track.          */
          if(ind > -1) {          
            indiv = pw_linear_array_ref(res,ind);
            pw_linear_add(indiv,dyv_ref(nums,0),pt);
          } else {
            indiv = mk_empty_pw_linear(2);
            pw_linear_add(indiv,dyv_ref(nums,0),pt);

            pw_linear_array_add(res,indiv);
            add_to_namer(ids_to_inds,id);
           
            free_pw_linear(indiv);
          }

          free_dyv(nums);
        } else {
          printf("WARNING: Bad Input In Line: %s\n",s);
        }
      } else {
        printf("WARNING: Bad Input Line: %s\n",s);
      }

      if(id != NULL) { free_string(id); id = NULL; }
      free_string(s);
    }

    free_dyv(pt);
  }
}
