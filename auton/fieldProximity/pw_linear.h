/*
File:         pw_linear.h
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

#ifndef PW_LINEAR_H
#define PW_LINEAR_H

#include "utils.h"
#include "namer.h"

#define PW_LINEAR_START_SIZE     10
#define PW_LINEAR_ARRAY_SIZE     10

typedef struct pw_linear {
  int num_points;
  int max_points;
  int D;

  double*  x;          /* independent variable (1-d vector)                  */
  double** y;          /* dependent variables (N by D array)                 */
} pw_linear;

typedef struct pw_linear_array {
  int max_size;
  int size;

  pw_linear** arr; 
} pw_linear_array;


/* A piecewise linear function is ANY function that */
/* takes in an independent variable x and produces  */
/* a D dimensional vector y.                        */


/* ---------------------------------------------------- */
/* --- Functions for the piecewise linear approx. ----- */
/* ---------------------------------------------------- */


/* --- Memory Functions ------------------------------- */

/* Create an empty pw_linear structure of dimension D. */
pw_linear* mk_empty_pw_linear(int D);

pw_linear* mk_sized_empty_pw_linear(int N, int D);

pw_linear* mk_copy_pw_linear(pw_linear* old);

void free_pw_linear(pw_linear* old);


/* Generates tracks with X0 between Lbnd and Ubnd     */
/* and velocity in each dimension generated as normal */
/* with initial sigma = sig_initV and modified each   */
/* step as sig_V. keepbnds indicates whether to use   */
/* the bounds for all time (1) or just x0 (0).        */
pw_linear* mk_random_pw_linear(dyv* times, dyv* Lbnd, dyv* Ubnd,
                               dyv* sig_initV, dyv* sig_V, ivec* keepbnds);


/* --- Getter/Setter Functions ------------------------ */

int safe_pw_linear_D(pw_linear* pw);

int safe_pw_linear_size(pw_linear* pw);

double safe_pw_linear_x(pw_linear* pw, int i);

double safe_pw_linear_y(pw_linear* pw, int i, int dim);


#ifdef AMFAST

#define pw_linear_D(X)           (X->D)
#define pw_linear_size(X)        (X->num_points)
#define pw_linear_x(X,i)         (X->x[(i)])
#define pw_linear_y(X,i,d)       (X->y[(i)][(d)])

#else

#define pw_linear_D(X)           (safe_pw_linear_D(X))
#define pw_linear_size(X)        (safe_pw_linear_size(X))
#define pw_linear_x(X,i)         (safe_pw_linear_x(X,i))
#define pw_linear_y(X,i,d)       (safe_pw_linear_y(X,i,d))

#endif

void pw_linear_add(pw_linear* pw, double x, dyv* y);


/* --- Prediction Functions --------------------------- */

/* Find the index of first x that is larger than   */
/* the given value (returns N if x is larger than  */
/* everything else in the array already).          */
int pw_linear_first_larger_x(pw_linear* pw, double val);

dyv* mk_pw_linear_predict(pw_linear* pw, double x);

double pw_linear_predict(pw_linear* pw, double x, int dim);

/* Returns the knot points used... */
double pw_linear_predict_full(pw_linear* pw, double x, int dim, int* s, int *e);

/* Convert pw_linear to a "smooth" RA/DEC function. */
/* In other words each transition is assumed to be  */
/* < 12.0 in RA                                     */
void pw_linear_conv_RADEC(pw_linear* pw);


/* --- I/O Functions ---------------------------------- */

void fprintf_pw_linear(FILE* f, char* pre, pw_linear* pw, char* post);


/* ---------------------------------------------------- */
/* --- Functions for the piecewise linear array. ------ */
/* ---------------------------------------------------- */

pw_linear_array* mk_empty_pw_linear_array(void);

pw_linear_array* mk_empty_pw_linear_array_sized(int size);

pw_linear_array* mk_copy_pw_linear_array(pw_linear_array* old);

pw_linear_array* mk_pw_linear_array_subset(pw_linear_array* old, ivec* inds);

pw_linear_array* mk_pw_linear_array_concat(pw_linear_array* A, pw_linear_array* B);

void free_pw_linear_array(pw_linear_array* old);

/* Generates tracks with X0 between Lbnd and Ubnd     */
/* and velocity in each dimension generated as normal */
/* with initial sigma = sig_initV and modified each   */
/* step as sig_V. keepbnds indicates whether to use   */
/* the bounds for all time (1) or just x0 (0).        */
pw_linear_array* mk_random_pw_linear_array(int N, dyv* times, dyv* Lbnd,
                                           dyv* Ubnd, dyv* sig_initV,
                                           dyv* sig_V, ivec* keepbnds);

pw_linear* safe_pw_linear_array_ref(pw_linear_array* X, int index);

pw_linear* safe_pw_linear_array_first(pw_linear_array* X);

pw_linear* safe_pw_linear_array_last(pw_linear_array* X);

void pw_linear_array_set(pw_linear_array* X, int index, pw_linear* A);

void pw_linear_array_add(pw_linear_array* X, pw_linear* A);

void pw_linear_array_add_many(pw_linear_array* X, pw_linear_array* nu);

int safe_pw_linear_array_size(pw_linear_array* X);

int safe_pw_linear_array_max_size(pw_linear_array* X);

int pw_linear_array_number_nonnull(pw_linear_array* X);

/* Allow a few speedups */
#ifdef AMFAST

#define pw_linear_array_size(X) (X->size)
#define pw_linear_array_max_size(X) (X->max_size)
#define pw_linear_array_ref(X,i) (X->arr[i])
#define pw_linear_array_first(X) (X->arr[0])
#define pw_linear_array_last(X)  (X->arr[X->size-1])

#else

#define pw_linear_array_size(X) (safe_pw_linear_array_size(X))
#define pw_linear_array_max_size(X) (safe_pw_linear_array_max_size(X))
#define pw_linear_array_ref(X,i) (safe_pw_linear_array_ref(X,i))
#define pw_linear_array_first(X) (safe_pw_linear_array_first(X))
#define pw_linear_array_last(X)  (safe_pw_linear_array_last(X))

#endif

void fprintf_pw_linear_array(FILE* f, pw_linear_array* X);

/* Reads in an array of pw_linears where each line contains */
/* TRACK_ID TIME RA DEC where TRACK_ID is an 8 character ID */
/* names - an empty name to store the track id to inds.     */
/*         CAN BE set to NULL (and thus ignored).           */
/* RAdegrees - true off RA is given in degrees.  Otherwise  */
/*             is if given in hours.                        */
pw_linear_array* mk_load_RA_DEC_pw_linear_array(char* filename, namer* names, bool RAdegrees);

void add_to_RA_DEC_pw_linear_array(pw_linear_array* res, char* filename,
                                   namer* ids_to_inds, bool RAdegrees);

#endif
