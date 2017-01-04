/*
   File:        amdyv.h
   Author:      Andrew W. Moore
   Created:     Thu Sep 15 21:01:12 EDT 1994
   Updated:     amdm was split into amdyv, amdym and svd by Frank Dellaert, Aug 14 1997
   Description: Header for Dynamically allocated and deallocated vectors

   Copyright 1996, Schenley Park Research

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

#ifndef AMDYV_H
#define AMDYV_H

#include "standard.h"
#include "ambs.h"
#include "amiv.h"

/*
%%%%%% DYVs (DYnamic Vectors)

dyvs are just one dimensional matrices. You can conveniently allocate
and free them too using the same conventions as for dynamic matrices.

dyv *mk_dyv(size) CREATES and returns one of the fellows.

int dyv_size(dyv *dv) returns the number of elements in the dyv

double dyv_ref(dv,i) returns the value of the i'th element.
  Indexing begins at 0: 0 <= i < dyv_size(dv).

All the numerical operations on dym's have counterpart numerical
operations on dyv's:

void constant_dyv(dyv *r_d,double v);
void zero_dyv(dyv *r_d);
dyv *mk_constant_dyv(int size,double v);
dyv *mk_zero_dyv(int size);
void dyv_scalar_mult(dyv *d, double alpha, dyv *r_d);
dyv *mk_dyv_scalar_mult(dyv *d,double alpha);
void dyv_scalar_add(dyv *d, double alpha, dyv *r_d);
dyv *mk_dyv_scalar_add(dyv *d,double alpha);
void copy_dyv(const dyv *d, dyv *r_d);
dyv *mk_copy_dyv(dyv *d);
void dyv_plus(const dyv *d_1, const dyv *d_2, dyv *r_d);
dyv *mk_dyv_plus(dyv *a,dyv *b);
void dyv_subtract(dyv *d_1,dyv *d_2,dyv *r_d);
dyv *mk_dyv_subtract(dyv *a,dyv *b);
void dyv_sort(dyv *dv,dyv *r_dv);  It is fine, as always, if dy and r_dv are
                                   the same.
dyv *mk_dyv_sort(dyv *dv);

%%%%%%%%% Making small dyvs 
dyv *mk_dyv_1(double x) makes a 1-element dyv containing x as its only element
dyv *mk_dyv_2(double x,double y) 
         makes a 2-element dyv containing x as its 0th-indexed element
                                          y as its 1-index element
dyv *mk_dyv_3(double x,double y , double z) .... obvious.

%%%%%%%%% Complex vector operations

double dyv_scalar_product(dyv *a,dyv *b);
Returns a . b

double dyv_dsqd(dyv *a,dyv *b)
Returns (a - b).(a - b)

*/


typedef struct dyv_struct
{
  int dyv_code;
  int array_size;
  int size;
  double *farr;
} *dyv_ptr;

/*needed to allow forward declaration of type dyv without full 
  #include amdyv.h for the ASL C api*/
#ifndef DYV_TYPEDEF_DEFINED
#define DYV_TYPEDEF_DEFINED
typedef struct dyv_struct dyv;
#endif

dyv *mk_dyv(int size);

/* Acts as though you created a dyv of size 0,         */
/* but actually allocates an array of size "capacity". */
/* This is very useful if you want to use add_to_dyv   */
/* and happen to have a reasonable upper bound to the  */
/* number of elements you want to add.                 */
dyv *mk_empty_dyv(int capacity);

/* Note that this clears all the old data your dyv had */
void dyv_destructive_resize(dyv* dv, int size);

/* Warning: no type checking can be done by the compiler.  You *must*
   send the values as doubles for this to work correctly. */
dyv *mk_dyv_x(int size, ...);

void free_dyv(dyv *d);
int dyv_num_bytes(dyv *d);

void free_dyv_NaN_ok(dyv *d);

void dyv_malloc_report(void);

int safe_dyv_size(const dyv *d);
double safe_dyv_ref(const dyv *d, int i);
void safe_dyv_set(dyv *d,int i,double value);
void safe_dyv_increment(dyv *d,int i,double value);

#ifdef AMFAST

#define dyv_size(d) ((d)->size)
#define dyv_ref(d,i) ((d)->farr[i])
#define dyv_set(d,i,v) ((d)->farr[i] = (v))
#define dyv_increment(d,i,v) ((d)->farr[i] += (v))

#else /* If not AMFAST */

#define dyv_size(d) (safe_dyv_size(d))
#define dyv_ref(d,i) (safe_dyv_ref(d,i))
#define dyv_set(d,i,v) (safe_dyv_set(d,i,v))
#define dyv_increment(d,i,v) (safe_dyv_increment(d,i,v))

#endif /* #ifdef AMFAST */

/*Added by Dan: Something I've wanted for a LONG time!*/
#define dyv_array_ref_ref(d,i,j) dyv_ref(dyv_array_ref(d,i),j)
#define dyv_array_ref_set(d,i,j,x) dyv_set(dyv_array_ref(d,i),j,x)
#define dyv_array_ref_size(d,i) dyv_size(dyv_array_ref(d,i))

#define add_to_dyv_array_ref(da,i,x) add_to_dyv(dyv_array_ref(da,i),x)

void dyv_increase_length(dyv *d,int extra_size);
void dyv_decrease_length(dyv *d,int new_length);
void add_to_dyv(dyv *d,double new_val);

void dyv_remove(dyv *d,int index); /* Reduces size by 1, removes index'th 
                                      element. All elements to right of
                                      delete point copied one to left.
                                      See comments in amdm.c more details */
void dyv_remove_last_element(dyv *d); /* Reduce size by 1, remove 
                                        last rightmost elt */

/* Increases dv in length by 1 and shifts all elements
   with original index greater or equal to index one to the
   right and inserts val at index. */
void dyv_insert(dyv *dv,int index,double val);

void copy_dyv_to_farr(const dyv *d, double *farr);

double *mk_farr_from_dyv(const dyv *d);

void copy_farr_to_dyv(double *farr,int size,dyv *r_d);

dyv *mk_dyv_from_farr(double *farr,int size);

void copy_dyv_to_tdarr_row(dyv *dv,double **tdarr,int row);

void copy_dyv_to_tdarr_col(dyv *dv,double **tdarr,int col);

void copy_tdarr_row_to_dyv(double **tdarr,dyv *dv,int row);

dyv *mk_dyv_from_tdarr_row(double **tdarr,int row,int tdarr_cols);

void copy_tdarr_col_to_dyv(double **tdarr,dyv *dv,int col);

dyv *mk_dyv_from_tdarr_col(double **tdarr,int col,int tdarr_rows);


void constant_dyv(dyv *r_d,double v);

void zero_dyv(dyv *r_d);

bool zero_dyvp(dyv *d);

dyv *mk_constant_dyv(int size,double v);

dyv *mk_zero_dyv(int size);

void dyv_mult(const dyv *d1, const dyv *d2, dyv *rd);
dyv *mk_dyv_mult(const dyv *d1, const dyv *d2);

void dyv_scalar_mult(const dyv *d, double alpha, dyv *r_d);

dyv *mk_dyv_scalar_mult(const dyv *d,double alpha);

/* Same as dyv_scalar_add but modifies the dyv directly.
 * Added by AliceZ 06/04/2006 */
void this_dyv_scalar_add(dyv *d, double alpha);

void dyv_scalar_add(dyv *d, double alpha, dyv *r_d);

dyv *mk_dyv_scalar_add(dyv *d,double alpha);

void dyv_madd( double factor, dyv *dv1, dyv *dv2, dyv *r_dv);

dyv *mk_dyv_madd( double factor, dyv *dv1, dyv *dv2); 

void copy_dyv(const dyv *d, dyv *r_d);

dyv *mk_copy_dyv(const dyv *d);

void dyv_plus(const dyv *d_1, const dyv *d_2, dyv *r_d);

dyv *mk_dyv_plus(const dyv *a,const dyv *b);

void dyv_subtract(const dyv *d_1,const dyv *d_2,dyv *r_d);

dyv *mk_dyv_subtract(const dyv *a,const dyv *b);

void dyv_abs( const dyv *dv, dyv *r_dv);

dyv *mk_dyv_abs( const dyv *dv);

double dyv_scalar_product(const dyv *a,const dyv *b);

double dyv_dsqd(const dyv *a,const dyv *b);

double pnorm( dyv *v, double p);

double dyv_magnitude(const dyv *a);

int paul_index_in_sorted_dyv( dyv *dv, double key);
int index_in_sorted_dyv(dyv *a,double x);

void dyv_sort(const dyv *dv,dyv *r_dv);

dyv *mk_dyv_sort(const dyv *dv);

/*
 Creates a dyv of indices such that indices[i] is the origional
 location (in the unsorted dv) of the ith smallest value.
 Used when you want the location of the sorted values instead of
 the sorted vector itself.
*/
dyv *mk_sorted_dyv_indices(dyv *dv);

ivec *mk_ivec_sorted_dyv_indices(dyv *dv);


double dyv_sum(const dyv *dv);
double dyv_product(const dyv *dv);

double dyv_mean(const dyv *dv);
  /* Mean of all elements in dv */

double dyv_median(const dyv *dv); /* added by Artur Dubrawski on Aug 02 1996, efficiented by AWM */
  /* Median of all elements in dv */

double dyv_sdev(const dyv *dv);
  /* Sdev of all elements in dv */

double dyv_min(const dyv *dv);
  /* Min of all elements in dv. ERROR if dv sized zero */

double dyv_max(const dyv *dv);
  /* Max of all elements in dv. ERROR if dv sized zero */

int dyv_argmin(const dyv *dv);

int dyv_argmax(const dyv *dv);

/* Computes reciprocal of each element in dv, with one exception: if
   the element is zero, it will also be zero in the result. */

dyv *mk_dyv_reciprocal(const dyv *dv) ;
void dyv_reciprocal(const dyv *dv, dyv *r_dv) ;

/* Elementwise multiplication */

void dyv_element_times(const dyv *dv1, const dyv *dv2, dyv *r_dv) ;

dyv *mk_dyv_1(double x0);
dyv *mk_dyv_2(double x0,double x1);
dyv *mk_dyv_3(double x0,double x1,double x2);
dyv *mk_dyv_4(double x0,double x1,double x2,double x3);
dyv *mk_dyv_5(double x0,double x1,double x2,double x3,double x4);
dyv *mk_dyv_6(double x0,double x1,double x2,double x3,double x4,double x5);

double xt_diag_x_value(dyv *x, dyv *a);

dyv *mk_user_input_dyv(char *message,int dims);

dyv *mk_basic_dyv_from_args(const char *name,int argc,char *argv[],int size);

dyv *mk_dyv_from_args(char *name,int argc,char *argv[],dyv *deflt);

/* COPIES in deflt (if so required) */
/* The usage on the commane line is dyvname "v1 v2 ...", that is a 
   space seperated list in parens */
dyv *mk_dyv_from_args1(char *name,int argc,char *argv[],dyv *deflt);

void fprintf_dyv(FILE *s, const char *m1, const dyv *d, const char *m2);
void pdyv(dyv *d);

#ifdef AMFAST
#define assert_dyv_shape(d,size,name)
#define check_dyv_code(d,name)
#else
void assert_dyv_shape(const dyv *d,int size,char *name);
void check_dyv_code(const dyv *d, const char *name);
#endif /* #ifdef AMFAST */

/* Returns TRUE if any of x's elements are NaN */
/* Declared but not defined - sir 8/6/2000
bool dyv_isnan(dyv *x);*/

/* Returns TRUE if any elements are NaN or Inf */
bool dyv_is_ill_defined(dyv *x);

#ifdef AMFAST
#define check_dyv_ok(x) /* do nothing */
#else
void check_dyv_ok_slow(dyv *x);
#define check_dyv_ok(x) check_dyv_ok_slow(x)
#endif

/* Makes and returns the vector a * a_weight + b * b_weight */
dyv *mk_dyv_scalar_combine(dyv *a,dyv *b,double a_weight,double b_weight);

/* Returns the number of distinct values in "values" */
int dyv_num_unique_values(dyv *values);

/* returns TRUE if dyvs are equal */
bool dyv_equal(const dyv *dx, const dyv *dy);

void fprintf_oneline_dyv(FILE *s,const char *m1, const dyv *d, const char *m2);
void fprint_dyv_csv(FILE *s,dyv *x);

/* After calling, ivec_ref(iv,k) points to the k'th smallest
   element in dv (ties handles arbitrarily) */
void indices_of_sorted_dyv(const dyv *dv,ivec *iv);

dyv *mk_sorted_dyv(dyv *x);

/* After calling, ivec_ref(result,k) points to the k'th smallest
   element in dv (ties handles arbitrarily) */
ivec *mk_indices_of_sorted_dyv(const dyv *dv);

ivec *mk_indices_of_sorted_ivec(ivec *v); /* Artur */

dyv *mk_reverse_dyv(dyv *x);
void reverse_dyv(dyv *x);        /* JK: Reverse dyv in place */

/* x := x with y appended on the end */
void append_to_dyv(dyv *x,dyv *y);

/* Return x with y appended on the end */
dyv *mk_dyv_append(dyv *x,dyv *y);

/* Makes a dyv of the same size as lo and hi (which must
   both be the same size such that
   dyv_ref(result,i) is uniformly randomly distributed between
   dyv_ref(lo,i) and dyv_ref(hi,i)
*/
dyv *mk_dyv_range_random(dyv *lo, dyv *hi);

/* Forall i such that ilo <= i < ihi (note the strict inequality on right)

   We do a[i] += delta
*/
void dyv_increment_block(dyv *a,int ilo,int ihi,double delta);

void random_unit_dyv(dyv *dv);
dyv *mk_norm_dyv(const dyv *d);
dyv *mk_normalize_dyv(const dyv *d);
void normalize_dyv(const dyv *src,dyv *dest);

/* Added by Jeremy... normalize a single dyv WITHOUT
   the overhead of memory allocation/deallocation */
void normalize_this_dyv(dyv* d);

dyv *mk_random_unit_dyv(int size);

dyv *mk_dyv_cusum(dyv *x);

/* iv and dv start out as parallel structures (they must be the same size).
   we will sort iv so that its corresponding dv entries are increasing.
   just to maintain the "parallel structures", we will also sort dv.
*/
void sort_ivec_by_dyv(ivec *iv, dyv *dv);


#define DYV_CODE 4509

#endif /* #ifndef AMDYV_H */
