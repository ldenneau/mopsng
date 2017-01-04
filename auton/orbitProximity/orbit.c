/*
   File:        orbit.c
   Author:      J. Kubica
   Created:     Fri Oct 3 11:27:56 EDT 2003
   Description: Data structures of astronomical orbits.

   Copyright 2003, The Auton Lab, CMU

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

#include "orbit.h"


/* --- Memory functions for the simple orbits ---------------------------- */

orbit* mk_orbit(double a, double e, double i, double w, double O, 
                double M, double H, double Epoch, double equinox) {
  orbit* nu = AM_MALLOC(orbit);

  nu->a  = a;
  nu->e  = e;
  nu->i  = i;
  nu->w  = w;
  nu->O  = O;
  nu->M  = M;
  nu->H  = H;
 
  nu->p  = a * (1 - e*e);
  nu->q  = nu->p / (1 + e);
  
  if(fabs(e-1.0) < 0.01) {
    nu->n = sqrt(GM_SUN);
  } else {
    if(e < 1.0) {
      nu->n = sqrt(GM_SUN/(a*a*a));
    } else {
      nu->n = sqrt(GM_SUN/((-a)*(-a)*(-a)));
    }
  }

  nu->t0     = Epoch - M / nu->n;
  nu->Epoch  = Epoch;
  nu->equinox = equinox;

  return nu;
}


orbit* mk_orbit2(double t0, double q, double e, double i,
		 double O, double w, double Epoch, double equinox) {
  orbit* nu = AM_MALLOC(orbit);
  double a;

  nu->q  = q;
  nu->t0 = t0;
  nu->e  = e;
  nu->i  = i;
  nu->w  = w;
  nu->O  = O;
  nu->Epoch   = Epoch;
  nu->equinox = equinox;

  nu->p = q*(1.0+e);
  nu->H = 0.0;

  if(fabs(e-1.0) < 0.01) {
    nu->n = sqrt(GM_SUN);
    a = 0.0;
  } else {
    a = nu->p / (1.0 - e*e);

    if(e < 1.0) {
      nu->n = sqrt(GM_SUN/(a*a*a));
    } else {
      nu->n = sqrt(GM_SUN/((-a)*(-a)*(-a)));
    }
  }

  nu->a = a;
  nu->M = nu->n * (Epoch - t0);
  
  return nu;
}


orbit* mk_copy_orbit(orbit* old) {
  orbit* nu = AM_MALLOC(orbit);

  nu->a  = old->a;
  nu->e  = old->e;
  nu->i  = old->i;
  nu->w  = old->w;
  nu->O  = old->O;
  nu->M  = old->M;
  nu->H  = old->H;
  nu->p  = old->p;
  nu->q  = old->q;
  nu->n  = old->n;
  nu->t0 = old->t0;
  nu->Epoch   = old->Epoch;
  nu->equinox = old->equinox;

  return nu;
}


orbit* mk_orbit_from_simulated_string(char* str, double equinox) {
  double a,e,i,O,w,M,H;
  orbit* nu;
  dyv* nums;
  char* temp;
  int j;

  /* extract a */
  temp = mk_copy_string(str);
  for(j=0;j<11;j++) { temp[j] = ' '; }
  nums = mk_dyv_from_string(temp,NULL);
  
  a = dyv_ref(nums,0);
  e = dyv_ref(nums,1);
  i = dyv_ref(nums,2) * DEG_TO_RAD;
  O = dyv_ref(nums,3) * DEG_TO_RAD;
  w = dyv_ref(nums,4) * DEG_TO_RAD;
  M = dyv_ref(nums,5) * DEG_TO_RAD;
  H = dyv_ref(nums,6);

  nu = mk_orbit(a,e,i,w,O,M,H,52860.0,equinox);
 
  free_string(temp);
  free_dyv(nums);
 
  return nu;
}


orbit* mk_orbit_from_gorbit_string(char* str, double equinox) {
  double a,e,i,O,w,M,H,Epoch;
  orbit* nu = NULL;
  char* temp;
  dyv* nums;
  int L = (int)strlen(str);

  /* Check that the orbit is valid */
  if( (L >= 70)&&(str[68] == 'g')&&(str[1] != '*') ) {
    temp = mk_copy_string(str);
    temp[68] = ' ';
    nums = mk_dyv_from_string(temp,NULL);
  
    if(nums != NULL) {
      a = dyv_ref(nums,0);
      e = dyv_ref(nums,1);
      i = dyv_ref(nums,2) * DEG_TO_RAD;
      O = dyv_ref(nums,3) * DEG_TO_RAD;
      w = dyv_ref(nums,4) * DEG_TO_RAD;
      M = dyv_ref(nums,5) * DEG_TO_RAD;
      Epoch = dyv_ref(nums,6);
      H = dyv_ref(nums,11);
  
      nu = mk_orbit(a,e,i,w,O,M,H,Epoch,equinox);
      free_dyv(nums);
    } else {
      printf("ERROR with orbit line parsing.\n");
      printf("%s\n",str);
    }
 
    free_string(temp);
  }

  return nu;
}


orbit* mk_orbit_from_neworbit3_string(char* str, double equinox) {
  double a,e,i,O,w,M,Epoch;
  orbit* nu = NULL;
  char* temp;
  dyv* nums;
  int L = (int)strlen(str);
  bool valid;

  valid = (L >= 70) && (str[50] != 'n') && (str[60] != 'n');
  if(valid) {
    temp = mk_copy_string(str);
    temp[68] = ' ';
    temp[69] = ' ';
    nums = mk_dyv_from_string(temp,NULL);

    if(nums != NULL) {
      a = dyv_ref(nums,0);
      e = dyv_ref(nums,1);
      i = dyv_ref(nums,2) * DEG_TO_RAD;
      O = dyv_ref(nums,3) * DEG_TO_RAD;
      w = dyv_ref(nums,4) * DEG_TO_RAD;
      M = dyv_ref(nums,5) * DEG_TO_RAD;
      Epoch = dyv_ref(nums,6);

      if(a > 0.0) {
        nu = mk_orbit(a,e,i,w,O,M,0.0,Epoch,equinox);
      }

      free_dyv(nums);
    } else {
      printf("ERROR with orbit line parsing.\n");
      printf("%s\n",str);
    }
 
    free_string(temp);
  }

  return nu;
}


void free_orbit(orbit* old) {
  AM_FREE(old,orbit);
}



/* --- Orbit Access Functions -------------------------------------------- */

double safe_orbit_a(orbit* o) { return o->a; }
double safe_orbit_semi_major_axis(orbit* o) { return o->a; }

double safe_orbit_e(orbit* o) { return o->e; }
double safe_orbit_eccentricity(orbit* o) { return o->e; }

double safe_orbit_i(orbit* o) { return o->i; }
double safe_orbit_inclination(orbit* o) { return o->i; }

double safe_orbit_w(orbit* o)  { return o->w; }
double safe_orbit_arg_of_perihelion(orbit* o)  { return o->w; }

double safe_orbit_O(orbit* o)  { return o->O; }
double safe_orbit_ascending_node(orbit* o)  { return o->O; }

double safe_orbit_M(orbit* o)  { return o->M; }
double safe_orbit_mean_anomally(orbit* o)  { return o->M; }

double safe_orbit_H(orbit* o)  { return o->H; }
double safe_orbit_abs_magnitude(orbit* o) { return o->H; }

double safe_orbit_q(orbit* o) { return o->q; }
double safe_orbit_perhelion_dist(orbit* o) { return o->q; }

double safe_orbit_p(orbit* o) { return o->p; }
double safe_orbit_orbital_param(orbit* o) { return o->p; }

double safe_orbit_n(orbit* o) { return o->n; }
double safe_orbit_mean_motion(orbit* o) { return o->n; }

double safe_orbit_t0(orbit* o) { return o->t0; }
double safe_orbit_perihelion_time(orbit* o) { return o->t0; }

double safe_orbit_epoch(orbit* o) { return o->Epoch; }

double safe_orbit_equinox(orbit* o) { return o->equinox; }

double safe_orbit_period(orbit* o) { return ((2.0*PI)/o->n); }


/* --- Output functions -------------------------------------------- */

void printf_orbit(orbit* o) {
  fprintf_orbit(stdout,"",o,"\n");
}


void fprintf_orbit(FILE* f, char* prefix, orbit* o, char* suffix) {
  fprintf(f,prefix);
  fprintf(f,"%11.6f %11.6f %11.6f %11.6f %11.6f %11.6f %11.6f %7.3f ",
	  o->a,o->e,o->i,o->O,o->w,o->M,o->Epoch,o->H);
  fprintf(f,suffix);
}

void fprintf_orbit2(FILE* f, char* prefix, orbit* o, char* suffix) {
  fprintf(f,prefix);
  fprintf(f,"%11.6f %11.6f %11.6f %11.6f %11.6f %11.6f",
	  o->p,o->e,o->i,o->O,o->w,o->t0);
  fprintf(f,suffix);
}

void fprintf_orbit3(FILE* f, char* prefix, orbit* o, char* suffix) {
  fprintf(f,prefix);
  fprintf(f,"%11.6f %11.6f %11.6f %11.6f %11.6f %11.6f",
	  o->q,o->e,o->i,o->O,o->w,o->t0);
  fprintf(f,suffix);
}


/* ---------------------------------------------------- */
/* --- Functions for the orbit array ------------------ */
/* ---------------------------------------------------- */

orbit_array* mk_empty_orbit_array() {
  return mk_empty_orbit_array_sized(ORBIT_ARRAY_SIZE);
}

orbit_array* mk_empty_orbit_array_sized(int size) {
  orbit_array* res = AM_MALLOC(orbit_array);
  int i;

  res->size = 0;
  res->max_size = size;

  res->arr = AM_MALLOC_ARRAY(orbit*,size);
  for(i=0;i<size;i++) {
    res->arr[i] = NULL;
  }

  return res;
}

orbit_array* mk_copy_orbit_array(orbit_array* old) {
  orbit_array* res = mk_empty_orbit_array_sized(old->max_size);
  int i;

  res->size = old->size;
  for(i=0;i<res->size;i++) {
    if(old->arr[i]) {
      res->arr[i] = mk_copy_orbit(old->arr[i]);
    }
  }

  return res;
}


orbit_array* mk_orbit_array_subset(orbit_array* old, ivec* inds) {
  orbit_array* res = mk_empty_orbit_array_sized(ivec_size(inds));
  int i, ind;

  for(i=0;i<ivec_size(inds);i++) {
    ind = ivec_ref(inds,i);
    if((ind < old->size)&&(ind >= 0)&&(old->arr[ind])) {
      orbit_array_add(res,old->arr[ind]);
    }
  }

  return res;
}


orbit_array* mk_orbit_array_concat(orbit_array* A, orbit_array* B) {
  orbit_array* res = mk_empty_orbit_array_sized(A->size + B->size);
  int i;

  for(i=0;i<A->size;i++) {
    orbit_array_add(res,A->arr[i]);
  }
  for(i=0;i<B->size;i++) {
    orbit_array_add(res,B->arr[i]);
  }

  return res;
}


/* Loads in a series of orbits from a file.  Each line */
/* corresponds to a single orbit and has the form:     */
/* q e i O w t0 equinox                                */
/* q is given in AU; i,O,w are in degrees, and         */
/* t0,equinox are in MJD.                              */
/* If names != NULL and there is a 8th (optional) name */
/* column, it fills in the name array.                 */
orbit_array* mk_orbit_array_from_columned_file(char* filename, 
                                               string_array** names) {
  orbit_array*   res = NULL;
  orbit*         A;
  string_array*  strarr;
  FILE *fp = fopen(filename,"r");
  int size = 0;
  int line_number = 0;
  char* s;
  int i;

  if (!fp) {
    printf("ERROR: Unable to open orbit file (");
    printf(filename);
    printf(") for reading.\n");
  } else {

    /* Read through once to count the number of records */
    while((s = mk_next_interesting_line_string(fp,&line_number))) {
      size++;
      free_string(s);
    }

    fclose(fp);
    fp = fopen(filename,"r");

    /* Read through a second time extracting records */
    i   = 0;
    if(names != NULL) { names[0] = mk_string_array(0); }
    res = mk_empty_orbit_array_sized(size);
    while((s = mk_next_interesting_line_string(fp,&line_number))) {
      if((strlen(s) > 2)&&(s[0] != '#')) {

        strarr = mk_broken_string(s);
        if(string_array_size(strarr) >= 7) {
          A = mk_orbit2(atof(string_array_ref(strarr,5)),
                        atof(string_array_ref(strarr,0)),
                        atof(string_array_ref(strarr,1)),
                        atof(string_array_ref(strarr,2))*DEG_TO_RAD,
                        atof(string_array_ref(strarr,3))*DEG_TO_RAD,
                        atof(string_array_ref(strarr,4))*DEG_TO_RAD,
                        atof(string_array_ref(strarr,7)),
                        atof(string_array_ref(strarr,7)));
          orbit_array_add(res,A);
          free_orbit(A);

          if(names != NULL) {
            if(string_array_size(strarr) >= 9) {
              add_to_string_array(names[0],string_array_ref(strarr,8));
            } else {
              add_to_string_array(names[0],"");
            }
          }

	}
        free_string_array(strarr);
      }
      free_string(s);
    }

    fclose(fp);
  }

  return res;
}


void free_orbit_array(orbit_array* old) {
  int i;

  for(i=0;i<old->max_size;i++) {
    if(old->arr[i]) { free_orbit(old->arr[i]); }
  }

  AM_FREE_ARRAY(old->arr,orbit*,old->max_size);
  AM_FREE(old,orbit_array);
}


orbit* safe_orbit_array_ref(orbit_array* X, int index) {
  my_assert((X->max_size > index)&&(index >= 0));
  return X->arr[index];
}


orbit* safe_orbit_array_first(orbit_array* X) {
  return orbit_array_ref(X,0);
}


orbit* safe_orbit_array_last(orbit_array* X) {
  return orbit_array_ref(X,X->size-1);
}

void orbit_array_double_size(orbit_array* old) {
  orbit** nu_arr;
  int i;

  nu_arr = AM_MALLOC_ARRAY(orbit*,2*old->max_size+1);
  for(i=0;i<old->size;i++) {
    nu_arr[i] = old->arr[i];
  }
  for(i=old->size;i<2*old->max_size+1;i++) {
    nu_arr[i] = NULL;
  }

  AM_FREE_ARRAY(old->arr,orbit*,old->max_size);

  old->arr  = nu_arr;
  old->max_size = 2*old->max_size+1;
}

void orbit_array_set(orbit_array* X, int index, orbit* A) {
  my_assert(index >= 0);

  while(index >= X->max_size) { orbit_array_double_size(X); }

  if(X->arr[index]) { free_orbit(X->arr[index]); }
  if(index >= X->size) { X->size = index+1; }

  if(A != NULL) {
    X->arr[index] = mk_copy_orbit(A);
  } else {
    X->arr[index] = NULL;
  }
}


void orbit_array_add(orbit_array* X, orbit* A) {
  orbit_array_set(X,X->size,A);
}

void orbit_array_add_many(orbit_array* X, orbit_array* nu) {
  int i;

  for(i=0;i<orbit_array_size(nu);i++) {
    if(orbit_array_ref(nu,i) != NULL) {
      orbit_array_add(X,orbit_array_ref(nu,i));
    }
  }
}

int safe_orbit_array_size(orbit_array* X) {
  return X->size;
}


int safe_orbit_array_max_size(orbit_array* X) {
  return X->max_size;
}

int orbit_array_number_nonnull(orbit_array* X) {
  int count = 0;
  int i;

  for(i=0;i<orbit_array_size(X);i++) {
    if(orbit_array_ref(X,i) != NULL) {
      count++;
    }
  }

  return count;
}

void fprintf_orbit_array(FILE* f, orbit_array* X) {
  int i;

  for(i=0;i<X->size;i++) {
    fprintf(f,"%6i) ",i);
    if(X->arr[i]) {
      fprintf_orbit(f,"",X->arr[i],"\n");
    } else {
      fprintf(f,"EMPTY!\n");
    }
  }
}

void fprintf_orbit_array2(FILE* f, orbit_array* X) {
  int i;

  for(i=0;i<X->size;i++) {
    fprintf(f,"%6i) ",i);
    if(X->arr[i]) {
      fprintf_orbit2(f,"",X->arr[i],"\n");
    } else {
      fprintf(f,"EMPTY!\n");
    }
  }
}

void fprintf_orbit_array3(FILE* f, orbit_array* X) {
  int i;

  fprintf(f,"  num  |       q           e           i      ");
  fprintf(f,"     O           w          t0     \n");
  fprintf(f,"----------------------------------------------");
  fprintf(f,"------------------------------------\n");

  for(i=0;i<X->size;i++) {
    fprintf(f,"%6i | ",i);
    if(X->arr[i]) {
      fprintf_orbit3(f,"",X->arr[i],"\n");
    } else {
      fprintf(f,"EMPTY!\n");
    }
  }
}
