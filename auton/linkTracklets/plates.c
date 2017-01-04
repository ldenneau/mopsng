/*
   File:        plates.c
   Author:      J. Kubica
   Created:     Wed, Sept. 8, 2004
   Description: Primatives for the tests (tracks and plates)

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

#include "plates.h"

/* ------------------------------------------------------------------- */
/* -------- Plate Functions ------------------------------------------ */
/* ------------------------------------------------------------------- */

/* --- Memory Functions ---------------------------------------------- */

rd_plate* mk_empty_rd_plate() {
  rd_plate* res = AM_MALLOC(rd_plate);

  res->id = NULL;

  res->time   = 0.0;
  res->ra     = 0.0;
  res->dec    = 0.0;
  res->radius = 0.0;

  return res;
}


rd_plate* mk_rd_plate(char* id, double time, double ra, double dec, double radius) {
  rd_plate* res = AM_MALLOC(rd_plate);

  res->id = NULL;
  if(id != NULL) {
    res->id = mk_copy_string(id);
  }

  while(ra < 0.0)  { ra += 24.0; }
  while(ra > 24.0) { ra -= 24.0; }

  res->time   = time;
  res->ra     = ra;
  res->dec    = dec;
  res->radius = radius;

  return res;
}


rd_plate* mk_rd_plate_simple(double time, double ra, double dec, double radius) {
  return mk_rd_plate(NULL,time,ra,dec,radius);
}


/* Takes a string with "ID TIME RA DEC RADIUS" */
/* Time is given in decimal MJD.               */
/* RAdegrees is true iff RA is given in degrees.  Otherwise uses */
/* RA in hours.                                                */
rd_plate* mk_rd_plate_from_string(char* strA, bool RAdegrees) {
  char* idstr   = NULL;
  char* str     = mk_copy_string(strA);
  rd_plate* res = NULL;
  dyv* params   = NULL;
  int L = strlen(str);
  int i = 0;

  /* Extract the ID string. */
  if(L > 1) {

    /* Go to the end of the ID or line. */
    while((i < L)&&(str[i] != ' ')) { i++; }
    if(i==L) { i--; }

    /* Remove the ID string. */
    if(i > 0) {
      idstr = mk_substring(str,0,i);
      while(i >= 0) {
	str[i] = ' ';
        i--;
      }
    }

    /* Extract the other parameters */
    params = mk_dyv_from_string(str,NULL);
  
    if((params != NULL)&&(dyv_size(params) >= 4)) {
      if(RAdegrees) {
	dyv_set(params,1,dyv_ref(params,1)/15.0);
      }

      res = mk_rd_plate(idstr,dyv_ref(params,0),dyv_ref(params,1),
			dyv_ref(params,2),dyv_ref(params,3)*DEG_TO_RAD);
  
    } else {
      printf("WARNING - Error in Input Line: %s\n",strA);
    }
  } else {
    printf("WARNING - Error in Input: %s\n",strA);
  }

  if(idstr)  { free_string(idstr); }
  if(params) { free_dyv(params);   }
  free_string(str);

  return res;
}


rd_plate* mk_random_rd_plate(double tmin, double tmax,
                             double r_min, double r_max,
                             double d_min, double d_max,
                             double radius) {
  double r = range_random(r_min,r_max);
  double d = range_random(d_min,d_max);
  double t = range_random(tmin,tmax);

  return mk_rd_plate_simple(t,r,d,radius);
}


rd_plate* mk_copy_rd_plate(rd_plate* old) {
  rd_plate* res = AM_MALLOC(rd_plate);
  
  if(old->id != NULL) {
    res->id = mk_copy_string(old->id);
  } else {
    res->id = NULL;
  }

  res->ra     = old->ra;
  res->dec    = old->dec;
  res->radius = old->radius;
  res->time   = old->time;

  return res;
}


void free_rd_plate(rd_plate* old) {
  if(old->id != NULL) { free_string(old->id); }
  AM_FREE(old,rd_plate);
}


/* --- Getter/Setter Functions --------------------------------------- */

char* safe_rd_plate_id(rd_plate* p)      { return p->id; }
double safe_rd_plate_time(rd_plate* p)   { return p->time; }
double safe_rd_plate_RA(rd_plate* p)     { return p->ra; }
double safe_rd_plate_DEC(rd_plate* p)    { return p->dec; }
double safe_rd_plate_radius(rd_plate* p) { return p->radius; }


/* --- I/O Functions ------------------------------------------------- */

void fprintf_rd_plate(FILE* f, char* pre, rd_plate* X, char* post) {
  fprintf(f,pre);
  if(X->id != NULL) {
    fprintf(f,X->id);
  } else {
    fprintf(f,"000000000000");
  }
  fprintf(f," %15.8f %12.6f %12.6f %12.6f",
          X->time,X->ra,X->dec,X->radius*RAD_TO_DEG);
  fprintf(f,post);
}



/* ------------------------------------------------------------------- */
/* -------- Plate Array Functions ------------------------------------ */
/* ------------------------------------------------------------------- */

rd_plate_array* mk_empty_rd_plate_array(int size) {
  rd_plate_array* res = AM_MALLOC(rd_plate_array);
  int i;

  res->size = 0;
  res->max_size = size;

  res->rd_plates = AM_MALLOC_ARRAY(rd_plate*,size);
  for(i=0;i<size;i++) {
    res->rd_plates[i] = NULL;
  }

  return res;
}


rd_plate_array* mk_copy_rd_plate_array(rd_plate_array* old) {
  rd_plate_array* res = mk_empty_rd_plate_array(old->max_size);
  int i;

  res->size = old->size;
  for(i=0;i<res->size;i++) {
    if(old->rd_plates[i]) {
      res->rd_plates[i] = mk_copy_rd_plate(old->rd_plates[i]);
    }
  }

  return res;
}


rd_plate_array* mk_rd_plate_array_subset(rd_plate_array* old, ivec* inds) {
  rd_plate_array* res = mk_empty_rd_plate_array(ivec_size(inds));
  int i, ind;

  for(i=0;i<ivec_size(inds);i++) {
    ind = ivec_ref(inds,i);
    if((ind < old->size)&&(ind >= 0)&&(old->rd_plates[ind])) {
      rd_plate_array_add(res,old->rd_plates[ind]);
    }
  }

  return res;
}


rd_plate_array* mk_random_rd_plate_array(int N, double t_min, double t_max,
					 double r_min, double r_max,
					 double d_min, double d_max,double radius) {
  rd_plate_array* res = mk_empty_rd_plate_array(N+1);
  rd_plate* plt;
  int i;
  
  for(i=0;i<N;i++) {
    plt = mk_random_rd_plate(t_min,t_max,r_min,r_max,d_min,d_max,radius);
    rd_plate_array_add(res,plt);
    free_rd_plate(plt);
  }

  return res;
}


/* RAdegrees is true iff RA is given in degrees.  Otherwise uses */
/* RA in hours.                                                */
rd_plate_array* mk_load_rd_plate_array(char* filename, bool RAdegrees) {
  rd_plate_array* res = NULL;
  rd_plate* indiv;
  FILE *fp = fopen(filename,"r");
  int line_number = 0;
  char *s;

  if (!fp) {
    printf("ERROR: Unable to open PLATES file (");
    printf(filename);
    printf(") for reading.\n");
  } else {
    res = mk_empty_rd_plate_array(10);

    while((s = mk_next_interesting_line_string(fp,&line_number))) {
      indiv = mk_rd_plate_from_string(s,RAdegrees);

      if(indiv != NULL) {
        rd_plate_array_add(res,indiv);
        free_rd_plate(indiv);
      }

      free_string(s);
    }
  }

  return res;
}


rd_plate_array* mk_rd_plate_array_concat(rd_plate_array* A, rd_plate_array* B) {
  rd_plate_array* res = mk_empty_rd_plate_array(A->size + B->size);
  int i;

  for(i=0;i<A->size;i++) {
    rd_plate_array_add(res,A->rd_plates[i]);
  }
  for(i=0;i<B->size;i++) {
    rd_plate_array_add(res,B->rd_plates[i]);
  }

  return res;
}


void free_rd_plate_array(rd_plate_array* old) {
  int i;

  for(i=0;i<old->max_size;i++) {
    if(old->rd_plates[i]) { free_rd_plate(old->rd_plates[i]); }
  }

  AM_FREE_ARRAY(old->rd_plates,rd_plate*,old->max_size);
  AM_FREE(old,rd_plate_array);
}


rd_plate* safe_rd_plate_array_ref(rd_plate_array* X, int index) {
  my_assert((X->max_size > index)&&(index >= 0));
  return X->rd_plates[index];
}


rd_plate* safe_rd_plate_array_first(rd_plate_array* X) {
  return rd_plate_array_ref(X,0);
}


rd_plate* safe_rd_plate_array_last(rd_plate_array* X) {
  return rd_plate_array_ref(X,X->size-1);
}


void rd_plate_array_double_size(rd_plate_array* old) {
  rd_plate** nu_arr;
  int i;

  nu_arr = AM_MALLOC_ARRAY(rd_plate*,2*old->max_size+1);
  for(i=0;i<old->size;i++) {
    nu_arr[i] = old->rd_plates[i];
  }
  for(i=old->size;i<2*old->max_size+1;i++) {
    nu_arr[i] = NULL;
  }

  AM_FREE_ARRAY(old->rd_plates,rd_plate*,old->max_size);

  old->rd_plates  = nu_arr;
  old->max_size = 2*old->max_size+1;
}


void rd_plate_array_set(rd_plate_array* X, int index, rd_plate* A) {
  my_assert(index >= 0);

  while(index >= X->max_size) { rd_plate_array_double_size(X); }

  if(X->rd_plates[index]) { free_rd_plate(X->rd_plates[index]); }
  if(index >= X->size) { X->size = index+1; }

  if(A != NULL) {
    X->rd_plates[index] = mk_copy_rd_plate(A);
  } else {
    X->rd_plates[index] = NULL;
  }
}


void rd_plate_array_add(rd_plate_array* X, rd_plate* A) {
  rd_plate_array_set(X,X->size,A);
}


int safe_rd_plate_array_size(rd_plate_array* X) {
  return X->size;
}


int safe_rd_plate_array_max_size(rd_plate_array* X) {
  return X->max_size;
}


int rd_plate_array_number_nonnull(rd_plate_array* X) {
  int count = 0;
  int i;

  for(i=0;i<rd_plate_array_size(X);i++) {
    if(rd_plate_array_ref(X,i) != NULL) {
      count++;
    }
  }

  return count;
}


/* --- I/O Functions ------------------------------------------------- */

void fprintf_rd_plate_array(FILE* f, rd_plate_array* tarr) {
  int i;

  for(i=0;i<rd_plate_array_size(tarr);i++) {
    fprintf(f,"%8i) ",i);
    fprintf_rd_plate(f,"",rd_plate_array_ref(tarr,i),"\n");
  }
}

