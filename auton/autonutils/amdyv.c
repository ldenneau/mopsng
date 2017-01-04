
/*
   File:        amdyv.c
   Author:      Andrew W. Moore
   Created:     Thu Sep 15 21:01:13 EDT 1994
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

#include <stdio.h>
#include <math.h>

#include "amdyv.h"     /* Dynamically allocated and deallocated vectors */
#include "amma.h"      /* Fast, non-fragmenting, memory management */
#include "amar.h"      /* Obvious operations on 1-d arrays */
#include "am_string.h"
#include "am_string_array.h"
#include "internal_state.h"

/* Prototypes for private functions */
static void check_dyv_access(const dyv *d,int i, char *name);



#ifndef AMFAST

void check_dyv_code(const dyv *d, const char *name)
{
  if ( d == NULL )
  {
    fprintf(stderr,"NULL dyv passed in operation %s\n",name);
    my_error("dyv data structure");
  }
  if ( d->dyv_code != DYV_CODE )
  {
    fprintf(stderr,"Attempt to access a non-allocated DYnamic Vector\n");
    fprintf(stderr,"This is in the operation %s\n",name);
    my_error("dyv data structure error");
  }
  if ( d->array_size < 0 || d->size < 0 || d->size > d->array_size )
    my_error("d->array_size or d->size muddled. Found in check_dyv_code.");
}

#endif /* #ifdef AMFAST */


/*
* check_dyv_access is only called in safe_dyv_xxx
* It used to be non-functional with AMFAST,
* but Frank Dellaert reinstated it June 30 1997
*/

void check_dyv_access(const dyv *d,int i, char *name)
{
  check_dyv_code(d,name); /* non-functional if AMFAST */

  if ( i < 0 || i >= d->size )
  {
    fprintf(stderr,"In operation \"%s\"\n",name);
    fprintf(stderr,"the dyv (dynamic vector) has size = %d\n",d->size);
    fprintf(stderr,"You tried to use index i=%d\n",i);
    fprintf(stderr,"Here is the dyv that was involved:\n");
    fprintf_dyv(stderr,"dv",d,"\n");
    my_error("check_dyv_access");
  }
}


#ifndef AMFAST

void assert_dyv_shape(const dyv *d,int size,char *name)
{
  check_dyv_code(d,name);

  if ( size != d->size )
  {
    fprintf(stderr,"In operation \"%s\"\n",name);
    fprintf(stderr,"the dyv (dynamic vector) has size = %d\n", d->size);
    fprintf(stderr,"But should have been predefined with the shape:\n");
    fprintf(stderr,"size = %d\n",size);
    my_error("assert_dyv_shape");
  }
}

#endif /* #ifdef AMFAST */

dyv *mk_dyv(int size)
{
  am_memory_state_t *state_ptr = am_get_memory_state();
  dyv *result = AM_MALLOC(dyv);
  result -> dyv_code = DYV_CODE;
  result -> array_size = size;
  result -> size = size;
  result -> farr = am_malloc_realnums(size);
  state_ptr->Dyvs_mallocked += 1;
  return(result);
}


/* Acts as though you created a dyv of size 0,         */
/* but actually allocates an array of size "capacity". */
/* This is very useful if you want to use add_to_dyv   */
/* and happen to have a reasonable upper bound to the  */
/* number of elements you want to add.                 */
dyv *mk_empty_dyv(int capacity) {
  am_memory_state_t *state_ptr = am_get_memory_state();
  dyv *result = AM_MALLOC(dyv);
  if ( capacity < 0 ) my_error("mk_empty_ivec : capacity < 0 illegal");
  result -> dyv_code = DYV_CODE;
  result -> array_size = capacity;
  result -> size = 0;
  result -> farr = am_malloc_realnums(capacity);
  state_ptr->Dyvs_mallocked += 1;
  return(result);
}


void dyv_destructive_resize(dyv* dv, int size)
{
am_free_realnums(dv->farr, dv->array_size);
dv->array_size=size;
dv->size = size;
dv->farr = am_malloc_realnums(size);
}

dyv *mk_dyv_x( int size, ...)
{
  /* Warning: no type checking can be done by the compiler.  You *must*
     send the values as doubles for this to work correctly. */
  int i;
  double val;
  va_list argptr;
  dyv *dv;
  
  dv = mk_dyv( size);

  va_start( argptr, size);
  for (i=0; i<size; ++i) {
    val = va_arg( argptr, double);
    dyv_set( dv, i, val);
  }
  va_end(argptr);

  return dv;
}

double dyv_sum(const dyv *dv)
{
  double result = 0.0;
  int i;
  for ( i = 0 ; i < dyv_size(dv) ; i++ )
    result += dyv_ref(dv,i);
  return(result);
}

double dyv_product(const dyv *dv)
{
  double result = 1.0;
  int i;
  for ( i = 0 ; i < dyv_size(dv) ; i++ )
    result *= dyv_ref(dv,i);
  return(result);
}

bool dyv_is_ill_defined(dyv *x)
{
  return is_ill_defined(dyv_sum(x));
}

#ifndef AMFAST
void check_dyv_ok_slow(dyv *x)
{
  if ( dyv_is_ill_defined(x) ) my_error("dyv contained NaN's");
}
#endif

void free_dyv(dyv *d)
{
  am_memory_state_t *state_ptr = am_get_memory_state();
  check_dyv_code(d,"free_dyv");
#ifndef AMFAST
  if ( dyv_is_ill_defined(d) ) my_error("free_dyv: dyv contained NaN's");
#endif

  d -> dyv_code = 7777;

  am_free_realnums(d->farr,d->array_size);
  AM_FREE(d,dyv);

  state_ptr->Dyvs_freed += 1;
}

int dyv_num_bytes(dyv *d)
{
  return sizeof(dyv) + d->array_size * sizeof(double);
}

void free_dyv_NaN_ok(dyv *d)
{
  am_memory_state_t *state_ptr = am_get_memory_state();
  check_dyv_code(d,"free_dyv");

  d -> dyv_code = 7777;

  am_free_realnums(d->farr,d->array_size);
  AM_FREE(d,dyv);

  state_ptr->Dyvs_freed += 1;
}

void dyv_malloc_report(void)
{
  am_memory_state_t *state_ptr = am_get_memory_state();
  if ( state_ptr->Dyvs_mallocked > 0 )
  {
    fprintf(stdout,"# Dynamic Vectors  (datatype dyv) currently allocated:  %d\n",
           state_ptr->Dyvs_mallocked - state_ptr->Dyvs_freed
          );
    if ( state_ptr->Dyvs_mallocked - state_ptr->Dyvs_freed != 0 )
    {
      fprintf(stdout,"#       Number of dyv allocations since program start:  %d\n",
             state_ptr->Dyvs_mallocked
            );
      fprintf(stdout,"#       Number of dyv frees       since program start:  %d\n#\n",
             state_ptr->Dyvs_freed
            );
    }
  }
}

double safe_dyv_ref(const dyv *d, int i)
{
  check_dyv_access(d,i,"dyv_ref");
  return(d->farr[i]);
}

void safe_dyv_set(dyv *d,int i,double value)
{
  check_dyv_access(d,i,"dyv_set");
  d->farr[i] = value;
}

void safe_dyv_increment(dyv *d,int i,double value)
{
  check_dyv_access(d,i,"dyv_increment");
  d->farr[i] += value;
}

void dyv_increase_length(dyv *d,int extra_size)
{
  for ( ; extra_size > 0 ; extra_size-- )
    add_to_dyv(d,0.0);
}

void dyv_decrease_length(dyv *d,int new_length)
{
  if(new_length > 0 && new_length < d->size)
    d->size = new_length;
}

void add_to_dyv(dyv *d,double new_val)
{
  if ( d->array_size < 0 || d->size > d->array_size )
    my_error("dyv size or array size has got muddles. Talk to AWM");

  if ( d->size == d->array_size )
  {
    int new_array_size = 2 * d->size + 2;
    double *farr_new = AM_MALLOC_ARRAY(double,new_array_size);
    memcpy(farr_new, d->farr, d->size * sizeof (double));
    AM_FREE_ARRAY(d->farr,double,d->size);
    d -> farr = farr_new;
    d -> array_size = new_array_size;
  }
  d->farr[d->size] = new_val;
  d -> size += 1;
}

void copy_dyv_to_farr(const dyv *d, double *farr)
{
  copy_realnums(d->farr,farr,d->size);
}
  
double *mk_farr_from_dyv(const dyv *d)
{
  double *result;
  check_dyv_code(d,"make_copy_farr");
  result = am_malloc_realnums(d->size);
  copy_dyv_to_farr(d,result);
  return(result);
}

void copy_farr_to_dyv(double *farr,int size,dyv *r_d)
{
  assert_dyv_shape(r_d,size,"copy_farr_to_dyv");
  copy_realnums(farr,r_d->farr,size);
}

dyv *mk_dyv_from_farr(double *farr,int size)
{
  dyv *result = mk_dyv(size);
  copy_farr_to_dyv(farr,size,result);
  return(result);
}

/***** Copying dyvs to and from rows and columns of tdarrs. And
       Making dyvs from rows and columns of tdarrs too. *********/

void copy_dyv_to_tdarr_row(dyv *dv,double **tdarr,int row)
{
  memcpy(tdarr[row], dv->farr, dv->size * sizeof (double));
}

void copy_dyv_to_tdarr_col(dyv *dv,double **tdarr,int col)
{
  int i;
  for ( i = 0 ; i < dv->size ; i++ )
    tdarr[i][col] = dv->farr[i];
}

void copy_tdarr_row_to_dyv(double **tdarr,dyv *dv,int row)
{
  memcpy(dv->farr, tdarr[row], dv->size * sizeof (double));
}

dyv *mk_dyv_from_tdarr_row(double **tdarr,int row,int tdarr_cols)
{
  dyv *result = mk_dyv(tdarr_cols);
  copy_tdarr_row_to_dyv(tdarr,result,row);
  return(result);
}

void copy_tdarr_col_to_dyv(double **tdarr,dyv *dv,int col)
{
  int i;
  for ( i = 0 ; i < dv->size ; i++ )
    dv->farr[i] = tdarr[i][col];
}

dyv *mk_dyv_from_tdarr_col(double **tdarr,int col,int tdarr_rows)
{
  dyv *result = mk_dyv(tdarr_rows);
  copy_tdarr_col_to_dyv(tdarr,result,col);
  return(result);
}

void constant_dyv(dyv *r_d,double v)
{
  check_dyv_code(r_d,"constant_dyv");
  set_realnums_constant(r_d->farr,r_d->size,v);
}

void zero_dyv(dyv *r_d)
{
  check_dyv_code(r_d,"zero_dyv");
  constant_dyv(r_d,0.0);
}

/* Returns TRUE iff d is a non-NULL dyv all of whose elements are zero.
   Added by Mary on 11 Dec 96.
*/
bool zero_dyvp(dyv *d)
{
  bool res = TRUE;

  if (d == NULL)
    res = FALSE;
  else
  {
    int i;

    for (i = 0; res && i < dyv_size(d); i++)
    {
      if (dyv_ref(d, i) != 0.0) res = FALSE;
    }
  }
  return (res);
}

dyv *mk_constant_dyv(int size,double v)
{
  dyv *result = mk_dyv(size);
  constant_dyv(result,v);
  return(result);
}

dyv *mk_zero_dyv(int size)
{
  dyv *result = mk_dyv(size);
  zero_dyv(result);
  return(result);
}

/********* Standard operations of dyvs *********/

void fprintf_dyv(FILE *s, const char *m1, const dyv *d, const char *m2)
{
  if ( d == NULL )
    fprintf(s,"%s = (dyv *)NULL%s",m1,m2);
  else if ( d->dyv_code != DYV_CODE )
  {
    fprintf(stderr,"fprintf_dyv(s,\"%s\",d,\"\\n\"\n",m1);
    my_error("fprintf_dyv called with a non-allocated dyv (DYnamic Vector)");
  }
  else if ( d->size <= 0 )
    fprintf(s,"%s = <Dyv of size %d>%s",m1,d->size,m2);
  else
  {
    int i;
    buftab bt;
    int cols = 1;

    init_buftab(&bt,d->size,cols + 4);

    for ( i = 0 ; i < d->size ; i++ )
    {
      char buff[100];
      set_buftab(&bt,i,0,(i == (d->size-1)/2) ? m1 : "");
      set_buftab(&bt,i,1,(i == (d->size-1)/2) ? "=" : "");
      set_buftab(&bt,i,2,"(");

      sprintf(buff," %g ",d->farr[i]);
      set_buftab(&bt,i,3,buff);

      set_buftab(&bt,i,3+cols,")");
    }

    fprint_buftab(s,&bt);
    free_buftab_contents(&bt);
  }
  fprintf(s,"\n");
}

void pdyv(dyv *d)
{
  fprintf_dyv(stdout,"dyv",d,"\n");
}

void dyv_mult(const dyv *d1, const dyv *d2, dyv *rd)
{
  int i;
  int n = d1->size;

  assert_dyv_shape(rd,d1->size,"dyv_mult");
  assert_dyv_shape(rd,d2->size,"dyv_mult");
  for (i=0;i<n;i++) rd->farr[i] = d1->farr[i] * d2->farr[i];
}

dyv *mk_dyv_mult(const dyv *d1, const dyv *d2)
{
  dyv *result;
  check_dyv_code(d1,"mk_dyv_mult");
  check_dyv_code(d2,"mk_dyv_mult");
  result = mk_dyv(d1->size);
  dyv_mult(d1,d2,result);
  return result;
}

void dyv_scalar_mult(const dyv *d, double alpha, dyv *r_d)
{
  int i, n = d->size;
  assert_dyv_shape(r_d,n,"dyv_scalar_mult");
  for ( i = 0 ; i < n ; i++ )
    r_d -> farr[i] = d->farr[i] * alpha;
}

dyv *mk_dyv_scalar_mult(const dyv *d, double alpha)
{
  dyv *result;
  check_dyv_code(d,"mk_dyv_scalar_mult");
  result = mk_dyv(d->size);
  dyv_scalar_mult(d,alpha,result);
  return(result);
}

/* same as dyv_scalar_add but modifies the dyv directly 
 * Added by AliceZ 06/04/2006 */
void this_dyv_scalar_add(dyv *d, double alpha)
{
  int i;
  for (i = 0; i < d->size; i++) 
    d->farr[i] += alpha;
}

void dyv_scalar_add(dyv *d, double alpha, dyv *r_d)
{
  int i;
  assert_dyv_shape(r_d,d->size,"dyv_scalar_add");
  for ( i = 0 ; i < r_d -> size ; i++ )
    r_d -> farr[i] = d->farr[i] + alpha;
}

dyv *mk_dyv_scalar_add(dyv *d,double alpha)
{
  dyv *result;
  check_dyv_code(d,"mk_dyv_scalar_add");
  result = mk_dyv(d->size);
  dyv_scalar_add(d,alpha,result);
  return(result);
}

void dyv_madd( double factor, dyv *dv1, dyv *dv2, dyv *r_dv)
{
  /* This functions hopes to take advantage of the popular madd instruction. */
  int i;
  double v1, v2, result;

  for (i=0; i<dyv_size(dv1); ++i) {
    v1 = dyv_ref( dv1, i);
    v2 = dyv_ref( dv2, i);
    result = factor*v1 + v2;
    dyv_set( r_dv, i, result);
  }

  return;
}

dyv *mk_dyv_madd( double factor, dyv *dv1, dyv *dv2)
{
  dyv *result;
  result = mk_dyv( dyv_size(dv1));
  dyv_madd( factor, dv1, dv2, result);
  return result;
}


void copy_dyv(const dyv *d, dyv *r_d)
{
  assert_dyv_shape(r_d,d->size,"copy_dyv");
  dyv_scalar_mult(d,1.0,r_d);
}
    
dyv *mk_copy_dyv(const dyv *d)
{
  check_dyv_code(d,"mk_copy_dyv");
  return(mk_dyv_scalar_mult(d,1.0));
}
    
void dyv_plus(const dyv *d_1, const dyv *d_2, dyv *r_d)
{
  int i;
  check_dyv_code(d_1,"dyv_plus (1st arg)");
  check_dyv_code(d_2,"dyv_plus (2nd arg)");

  if ( d_1 -> size != d_2 -> size )
  {
    fprintf_dyv(stderr,"d_1",d_1,"\n");
    fprintf_dyv(stderr,"d_2",d_2,"\n");
    my_error("dyv_plus: dyvs (DYnamic Vectors) different shape");
  }

  assert_dyv_shape(r_d,d_1->size,"dyv_plus");
  for ( i = 0 ; i < r_d -> size ; i++ )
    r_d -> farr[i] = d_1->farr[i] + d_2 -> farr[i];
}

dyv *mk_dyv_plus(const dyv *a,const dyv *b)
{
  dyv *result = mk_dyv(a->size);
  dyv_plus(a,b,result);
  return(result);
}

void dyv_subtract(const dyv *d_1,const dyv *d_2,dyv *r_d)
{
  int i;
  check_dyv_code(d_1,"dyv_subtract (1st arg)");
  check_dyv_code(d_2,"dyv_subtract (2nd arg)");

  if ( d_1 -> size != d_2 -> size )
  {
    fprintf_dyv(stderr,"d_1",d_1,"\n");
    fprintf_dyv(stderr,"d_2",d_2,"\n");
    my_error("dyv_subtract: dyvs (DYnamic Vectors) different shape");
  }

  assert_dyv_shape(r_d,d_1->size,"dyv_subtract");
  for ( i = 0 ; i < r_d -> size ; i++ )
    r_d -> farr[i] = d_1->farr[i] - d_2 -> farr[i];
}

dyv *mk_dyv_subtract(const dyv *a,const dyv *b)
{
  dyv *result; 
  check_dyv_code(a,"mk_dyv_subtract (1st arg)");
  check_dyv_code(b,"mk_dyv_subtract (2nd arg)");
  result = mk_dyv(a->size);
  dyv_subtract(a,b,result);
  return(result);
}

void dyv_abs( const dyv *dv, dyv *r_dv)
{
  int i;
  assert_dyv_shape( r_dv, dyv_size( dv), "dyv_abs");
  for (i=0; i<dyv_size( dv); ++i) dyv_set( r_dv, i, fabs(dyv_ref( dv, i)));
  return;
}

dyv *mk_dyv_abs( const dyv *dv)
{
  dyv *absdv;
  absdv = mk_dyv( dyv_size( dv));
  dyv_abs( dv, absdv);
  return absdv;
}

/***** More complex operations ******/

double dyv_scalar_product(const dyv *a,const dyv *b)
{
  int i,n= a->size;
  double result = 0.0;
  if ( b -> size != n)
  {
    fprintf(stderr,"dyv_scalar_product: sizes differ\n");
    wait_for_key();
    fprintf_dyv(stderr,"a",a,"\n");
    fprintf_dyv(stderr,"b",b,"\n");
    my_error("dyv_scalar_product: sizes differ");
  }

  for ( i = 0 ; i < n ; i++ )
    result += a->farr[i] * b->farr[i];
  return(result);
}

/* FD rewrote this on aug 17: does not call mk_dyv_subtract anymore */
double dyv_dsqd(const dyv *a,const dyv *b)
{
  int i,n= a->size;
  double result = 0.0;
  if ( b -> size != n)
  {
    fprintf(stderr,"dyv_dsqd: sizes differ\n");
    wait_for_key();
    fprintf_dyv(stderr,"a",a,"\n");
    fprintf_dyv(stderr,"b",b,"\n");
    my_error("dyv_dsqd: sizes differ");
  }

  for ( i = 0 ; i < n ; i++ )
    {
      double dip = a->farr[i] - b->farr[i];
      result += dip*dip;
    }
  return(result);
}

double pnorm( dyv *v, double p)
{
  int size, i;
  double sum, val;
  size = dyv_size( v);
  sum = 0.0;
  for (i=0; i<size; ++i) {
    val = dyv_ref( v, i);
    sum += pow( val, p);
  };
  val = pow( sum, 1.0/p);
  return val;
}

double dyv_magnitude(const dyv *a)
{
  double result = sqrt(dyv_scalar_product(a,a));
  return(result);
}

double dyv_mean(const dyv *dv)
{
  check_dyv_code(dv,"dyv_mean");
  return(doubles_mean(dv->farr,dv->size));
}

double dyv_sdev(const dyv *dv)
{
  double sum_sq = 0.0;
  int i;
  double mean = dyv_mean(dv);
  double result;

  for ( i = 0 ; i < dv->size ; i++ )
    sum_sq += real_square(dv->farr[i] - mean);
    
  result = sqrt(sum_sq / int_max(dv->size-1,1));

  return(result);
}

double dyv_min(const dyv *dv)
{
  if ( dyv_size(dv) < 1 )
    my_error("dyv_min: empty dyv");
  return(doubles_min(dv->farr,dv->size));
}

double dyv_max(const dyv *dv)
{
  if ( dyv_size(dv) < 1 )
    my_error("dyv_max: empty dyv");
  return(doubles_max(dv->farr,dv->size));
}

int dyv_argmin(const dyv *dv)
{
  if ( dyv_size(dv) < 1 )
    my_error("dyv_argmin: empty dyv");
  return(doubles_argmin(dv->farr,dv->size));
}

int dyv_argmax(const dyv *dv)
{
  if ( dyv_size(dv) < 1 )
    my_error("dyv_argmax: empty dyv");
  return(doubles_argmax(dv->farr,dv->size));
}


/* Computes reciprocal of each element in dv, with one exception: if
   the element is zero, it will also be zero in the result. */

dyv *mk_dyv_reciprocal(const dyv *dv)
{
  int dim = dyv_size(dv) ;
  dyv *new_dv = mk_dyv(dim) ;

  dyv_reciprocal(dv, new_dv) ;
  return new_dv ;
}

void dyv_reciprocal(const dyv *dv, dyv *r_dv)
{
  int i ;
  int dim = dyv_size(dv) ;
  assert_dyv_shape(r_dv, dim, "dyv_reciprocal") ;

  for (i = 0; i < dim; ++i) {
    double val = dyv_ref(dv, i) ;
    dyv_set(r_dv, i, val ? 1.0/val : 0) ;
  }
}

/* Element-wise multiplication */

void dyv_element_times(const dyv *dv1, const dyv *dv2, dyv *r_dv)
{
  int i ;
  int dim = dyv_size(dv1) ;
  assert_dyv_shape(dv2, dim, "dyv_reciprocal") ;
  assert_dyv_shape(r_dv, dim, "dyv_reciprocal") ;

  for (i = 0; i < dim; ++i) {
    double val = dyv_ref(dv1, i) * dyv_ref(dv1, i) ;
    dyv_set(r_dv, i, val) ;
  }
}

dyv *mk_dyv_1(double x0)
{
  dyv *result = mk_dyv(1);
  dyv_set(result,0,x0);
  return(result);
}

dyv *mk_dyv_2(double x0,double x1)
{
  dyv *result = mk_dyv(2);
  dyv_set(result,0,x0);
  dyv_set(result,1,x1);
  return(result);
}

dyv *mk_dyv_3(double x0,double x1,double x2)
{
  dyv *result = mk_dyv(3);
  dyv_set(result,0,x0);
  dyv_set(result,1,x1);
  dyv_set(result,2,x2);
  return(result);
}

dyv *mk_dyv_4(double x0,double x1,double x2,double x3)
{
  dyv *result = mk_dyv(4);
  dyv_set(result,0,x0);
  dyv_set(result,1,x1);
  dyv_set(result,2,x2);
  dyv_set(result,3,x3);
  return(result);
}

dyv *mk_dyv_5(double x0,double x1,double x2,double x3,double x4)
{
  dyv *result = mk_dyv(5);
  dyv_set(result,0,x0);
  dyv_set(result,1,x1);
  dyv_set(result,2,x2);
  dyv_set(result,3,x3);
  dyv_set(result,4,x4);
  return(result);
}

dyv *mk_dyv_6(double x0,double x1,double x2,double x3,double x4,double x5)
{
  dyv *result = mk_dyv(6);
  dyv_set(result,0,x0);
  dyv_set(result,1,x1);
  dyv_set(result,2,x2);
  dyv_set(result,3,x3);
  dyv_set(result,4,x4);
  dyv_set(result,5,x5);
  return(result);
}

dyv *mk_user_input_dyv(char *message,int dims)
{
  dyv *res = mk_dyv(dims);
  int i = 0;
  char buff[100];
  for ( i = 0 ; i < dims ; i++ )
  {
    sprintf(buff,"%s (Dyv Component %d)> ",message,i);
    dyv_set(res,i,input_realnum(buff));
  }

  return(res);
}

dyv *mk_basic_dyv_from_args(const char *name,int argc,char *argv[],int size)
{
  int index = index_of_arg(name,argc,argv);
  dyv *result;

  if ( index < 0 )
    result = NULL;
  else
  {
    int i;
    bool ok = TRUE;
    result = mk_dyv(size);
    for ( i = 0 ; i < size && ok ; i++ )
    {
      int j = index + i + 1;
      if ( j >= argc || !is_a_number(argv[j]) )
        ok = FALSE;
      else
        dyv_set(result,i,atof(argv[j]));
    }

    if ( !ok )
    {
      free_dyv(result);
      result = NULL;
    }
  }

  return(result);
}

/* COPIES in deflt (if so required) */
dyv *mk_dyv_from_args(char *name,int argc,char *argv[],dyv *deflt)
{
  bool name_there = index_of_arg(name,argc,argv) >= 0;
  dyv *result;
  if ( !name_there )
  {
    if (deflt) result = mk_copy_dyv(deflt);
    else result = NULL;
  }
  else
  {
    result = mk_basic_dyv_from_args(name,argc,argv,dyv_size(deflt));
    if ( result == NULL )
    {
      fprintf(stderr,"COMMAND LINE USER ERROR (it's YOUR fault)\n");
      fprintf(stderr,"...when attempting to read a dyv identified by\n");
      fprintf(stderr,"the name \"%s\". Perhaps a non-number, or the\n",name);
      fprintf(stderr,"command line finished before all args found?\n");
      fprintf_dyv(stderr,"deflt_dyv",deflt,"\n");
      my_error("mk_dyv_from_args()");
    }
  }

  return(result);
}

/* as above except the dyv is expected to occupy a single argument rather
   than a sequence of them
 */
dyv *mk_dyv_from_args1(char *name,int argc,char *argv[],dyv *deflt)
{
  int index = index_of_arg(name,argc,argv)+1;
  dyv *result;
  int i, nfound = 0, nsize = 100;
  double *farr,*tmp;
  char *tok, *valstr;
  int valstrlen;

  if (index < 1)
  {
    if (!deflt) return NULL;
    else        return (mk_copy_dyv(deflt));
  }
  /* make copy of our value-string, which strtok will modify */
  valstr = mk_copy_string(argv[index]);
  valstrlen = strlen(valstr);
  
  farr = AM_MALLOC_ARRAY(double,nsize);
  tok = strtok(valstr," ");
  if (!tok)
    result=NULL;
  else {
    sscanf(tok,"%lf",&farr[nfound++]);
    while((tok = strtok(NULL," ")))
      {
        sscanf(tok,"%lf",&farr[nfound++]);
        if (nfound == (nsize-1))
	  {
	    tmp = AM_MALLOC_ARRAY(double,nsize*2);
	    for(i=0;i<nfound;i++) tmp[i] = farr[i];
	    AM_FREE_ARRAY(farr,double,nsize);
	    nsize *= 2;
	    farr = tmp;
	  }
      }
    result = mk_dyv_from_farr(farr,nfound);
    AM_FREE_ARRAY(farr,double,nsize);
  }
  /* its unfortunate that strtok corrupts valstr.  the only way to safely
     free it with our free fn is to put back non null characters over it 
  */
  for (i=0;i<valstrlen;i++) valstr[i] = ' ';
  free_string(valstr);
  return result;
}

/* Returns first index i such that dv[i] >= key.  If the last element of
   dv is strictly less than the key, then dyv_size(dv) is returned. */
int index_in_sorted_dyv( dyv *dv, double key)
{
  int idxlo, idxhi, idx;
  double val;

  /* idxhi is always >= desired idx.  idxlo is always <= desired idx. */

  idxlo = 0;
  idxhi = dyv_size( dv);
  if (idxhi == 0) return 0;

  while (1) {
    /* Get next value. */
    idx = (idxlo + idxhi) / 2;
    val = dyv_ref( dv, idx);

    /* Adjust bounds. */
    if (val >= key) idxhi = idx ;  /* idx is too high. */
    else idxlo = idx + 1;          /* idx is too low.  */

    /* If the boundaries meet, we're done. */
    if (idxhi == idxlo) break;
  }

  return idxlo;
}


/* We're not quite sure what this code computed.  On the bright
   side, since it isn't commented we can easily comment the
   entire function out. */
/* 
int index_in_sorted_dyv(dyv *d,double t){
  int i1 = dyv_size(d), i0 = 0;
  int i = (i1-i0)>>1;
  double n2 = dyv_ref(d,i), n1 = (i)? dyv_ref(d,i-1):-1;
  while((i1-i0)>1 && !(n2>=t && (!i || n1<t))){
    if(n2>t){
      i1 = i;
      i = i0+((i-i0)>>1);
    } else {
      i0 = i+1;
      i = i+((i1-i)>>1);
    }
  }
  return i;
}
*/

void dyv_sort(const dyv *dv,dyv *r_dv)
{
  int size;
  double *farr;

  check_dyv_code(dv,"dyv_sort (1st arg)");
  assert_dyv_shape(r_dv,dv->size,"dyv_sort");

  size = dyv_size(dv);
  farr = mk_farr_from_dyv(dv);
  sort_realnums(farr,size,farr);
  copy_farr_to_dyv(farr,size,r_dv);
  am_free_realnums(farr,size);
}


dyv *mk_dyv_sort(const dyv *dv)
{
  dyv *result;
  check_dyv_code(dv,"mk_dyv_sort");
  result = mk_dyv(dyv_size(dv));
  dyv_sort(dv,result);
  return(result);
}

/*
 Creates a dyv of indices such that indices[i] is the origional
 location (in the unsorted dv) of the ith smallest value.
 Used when you want the location of the sorted values instead of
 the sorted vector itself.
*/
dyv *mk_sorted_dyv_indices(dyv *dv)
{
  dyv* indices;
  int size = dyv_size(dv);
  int i;
  int* iarr; 
  double* farr;

  check_dyv_code(dv,"mk_sorted_dyv_indices");
  farr = mk_farr_from_dyv(dv);
  iarr = am_malloc_ints(size); 

  indices_sort_realnums(farr,size,iarr);
  indices = mk_dyv(size);

  for(i=0;i<size;i++)
  {
    dyv_set(indices,i,iarr[i]);
  }

  am_free_realnums(farr,size);
  am_free_ints(iarr,size);

  return indices;
}

ivec *mk_ivec_sorted_dyv_indices(dyv *dv)
{
  ivec* indices;
  int size = dyv_size(dv);
  double* farr;

  check_dyv_code(dv,"mk_sorted_dyv_indices");
  farr = mk_farr_from_dyv(dv);

  indices = mk_ivec(size);
  indices_sort_realnums(farr,size,indices->iarr);

  am_free_realnums(farr,size);

  return indices;
}


/**** Removal functions on dyvs ****/

/* Reduces the size of d by one.
   dyv_ref(d,index) disappears.
   Everything to the right of dyv_ref(d,index) is copied one to the left.

   Formally: Let dold be the dyv value beore calling this function
             Let dnew be the dyv value after calling this function.

PRE: dyv_size(dold) > 0
     0 <= index < dyv_size(dold)

POST: dyv_size(dnew) = dyv_size(dold)-1
      for j = 0 , 1, 2 ... index-1  : 
         dyv_ref(dnew,j) == dyv_ref(dold,j)

      for j = index , index+1 , ... dyv_size(dnew)-1:
         dyv_ref(dnew,j) == dyv_ref(dold,j+1)
*/
void dyv_remove(dyv *d,int index)
{
  int i;
  int dsize = dyv_size(d);

#ifndef AMFAST
  if ( dsize <= 0 ) my_error("dyv_remove: empty dyv");
  if ( index < 0 || index >= dsize ) my_error("dyv_remove: bad index");
#endif /* #ifndef AMFAST */

  for ( i = index ; i < dsize - 1 ; i++ )
    dyv_set(d,i,dyv_ref(d,i+1));
  d -> size -= 1;
}

/* Shrinks d by one element by removing the rightmost element. 
   Example:

  Before: d == ( 3 1 4 1 5 )
    dyv_remove_last_element(d)
  After d == ( 3 1 4 1 )
*/
void dyv_remove_last_element(dyv *d)
{
#ifndef AMFAST
  if (dyv_size(d) <= 0) my_error("dyv_remove_last_elt: empty dyv");
#endif /* #ifndef AMFAST */
  d->size -= 1;
}

/* Increases dv in length by 1 and shifts all elements
   with original index greater or equal to index one to the
   right and inserts val at index. */
void dyv_insert(dyv *dv,int index,double val)
{
  add_to_dyv(dv,0);
  memmove(dv->farr + index + 1, dv->farr + index,
                (dv->size - index - 1) * sizeof (double));
  dyv_set(dv,index,val);
}

/*
 * following 2 dyv functions are DEFINED in amdym.c
 * since they have code common with fprintf_dym
 
void fprintf_dyv(FILE *s,char *m1,dyv *d,char *m2)
void pdyv(dyv *d)

 */

/* Makes and returns the vector a * a_weight + b * b_weight */
dyv *mk_dyv_scalar_combine(dyv *a,dyv *b,double a_weight,double b_weight)
{
  int size = dyv_size(a);
  dyv *result = mk_dyv(size);
  int i;
  if ( size != dyv_size(b) ) my_error("mk_dyv_scalar_combine");
  for ( i = 0 ; i < size ; i++ )
    dyv_set(result,i,dyv_ref(a,i) * a_weight + dyv_ref(b,i) * b_weight);
  return result;
}

/* Returns the number of distinct values in "values" */
int dyv_num_unique_values(dyv *values)
{
  dyv *svals = mk_dyv_sort(values);
  int num_unique = 0;
  int i;
  for ( i = 0 ; i < dyv_size(svals) ; i++ )
  {
    if ( i == 0 )
      num_unique += 1;
    else
    {
      double this_val = dyv_ref(svals,i);
      double prev_val = dyv_ref(svals,i-1);
      my_assert(this_val >= prev_val);
      if ( this_val > prev_val )
	num_unique += 1;
    }
  }
  free_dyv(svals);
  return num_unique;
}

/* declared static because only used by dyv_equal, it appears - jostlund, 6/15/04 */
static bool double_equal(double x,double y)
{
  bool result = TRUE;
  double very_very_small = 1e-7;
  double very_small = 1e-4;

  if ( fabs(x) == 0.0 )
    result = fabs(y) < very_very_small;
  else if ( fabs(y) == 0.0 )
    result = fabs(x) < very_very_small;
  else
  {
    double s = fabs(x) + fabs(y);
    /* Is (x / s) very different from (y / s) by more than very_small? */
    result = fabs(x - y) <= s * very_small;
  }

  return result;
}

/* Sensible if args are NULL. False if different size */
bool dyv_equal(const dyv *x1, const dyv *x2)
{
  bool result = TRUE;

  if ( EQ_PTR(x1,x2) )
    result = TRUE;
  else if ( x1 == NULL || x2 == NULL )
    result = FALSE;
  else if ( dyv_size(x1) != dyv_size(x2) )
    result = FALSE;
  else
  {
    int i;
    for ( i = 0 ; result && i < dyv_size(x1) ; i++ ) 
      result = result && double_equal(dyv_ref(x1,i),dyv_ref(x2,i));
  }
  return(result);
}
  
void fprint_dyv_csv(FILE *s,dyv *x)
{
  int i;
  int size = dyv_size(x);

  for ( i = 0 ; i < size ; i++ )
    fprintf(s,"%g%s",dyv_ref(x,i),(i==size-1)?"\n":",");
}

void fprintf_oneline_dyv(FILE *s,const char *m1, const dyv *d, const char *m2)
{
  int i;
  fprintf(s,"%s ",m1);
  for ( i = 0 ; i < dyv_size(d) ; i++ )
    fprintf(s,"%8g%s",dyv_ref(d,i),(i==dyv_size(d)-1) ? "" : " ");
  fprintf(s,"%s",m2);
}

void indices_of_sorted_dyv(const dyv *dv,ivec *iv)
/*
   NOTE: ivec structure (integer vectors) defined in sortind.ch
   PRE: dv and iv must be same size. iv's contents irrelevant.
   POST: iv contains sorted indexes into dv, so that
         forall iv[j] is the j'th smallest element of dv.

         thus forall i,j, (i < j) => dyv_ref(dv,iv[i]) <= dyv_ref(dv,iv[j])
          and iv contains a permutation of [0 ... iv->size-1]
*/
{
  int *iarr = am_malloc_ints(dyv_size(dv));
#ifndef AMFAST
  int i;
#endif
  indices_sort_realnums(dv->farr,dv->size,iarr);
  copy_iarr_to_ivec(iarr,dv->size,iv);
  am_free_ints(iarr,dv->size);
#ifndef AMFAST
  for ( i = 0 ; i < ivec_size(iv)-1 ; i++ )
  {
    int index1 = ivec_ref(iv,i);
    int index2 = ivec_ref(iv,i+1);
    if ( dyv_ref(dv,index1) > dyv_ref(dv,index2) )
    {
      fprintf_dyv(stdout,"dv",dv,"\n");
      fprintf_ivec(stdout,"iv",iv,"\n");
      printf("iv should be sorted indices of dyv, but consider\n"
	     "elements %d and %d of iv\n",i,i+1);
      my_error("mk_indices_of_sorted_dyv broken");
    }
  }
#endif
}

dyv *mk_sorted_dyv(dyv *x)
{
  dyv *y = mk_copy_dyv(x);
  dyv_sort(x,y);
  return y;
}

ivec *mk_indices_of_sorted_dyv(const dyv *dv)
{
  ivec *iv = mk_ivec(dyv_size(dv));
  indices_of_sorted_dyv(dv,iv);
  return(iv);
}

/* iv and dv start out as parallel structures (they must be the same size).
   we will sort iv so that its corresponding dv entries are increasing.
   just to maintain the "parallel structures", we will also sort dv.
*/
void sort_ivec_by_dyv(ivec *iv, dyv *dv)
{
  int i;
  ivec *indices = mk_indices_of_sorted_dyv(dv);
  ivec *liv = mk_ivec(ivec_size(iv));
  dyv *ldv = mk_dyv(dyv_size(dv));

  for (i=0;i<ivec_size(iv);i++)
  {
    ivec_set(liv,i,ivec_ref(iv,ivec_ref(indices,i)));
    dyv_set(ldv,i,dyv_ref(dv,ivec_ref(indices,i)));
  }
  copy_ivec(liv,iv);
  copy_dyv(ldv,dv);

  free_ivec(indices); free_ivec(liv); free_dyv(ldv);
}

dyv *mk_reverse_dyv(dyv *x)
{
  dyv *a = mk_dyv(dyv_size(x));
  int i;
  for ( i = 0 ; i < dyv_size(x) ; i++ )
    dyv_set(a,dyv_size(x)-i-1,dyv_ref(x,i));
  return a;
}

void reverse_dyv(dyv *x)
{
  double temp;
  int s = 0;
  int e = dyv_size(x)-1;

  while(s < e) {
    temp = dyv_ref(x,s);
    dyv_set(x,s,dyv_ref(x,e));
    dyv_set(x,e,temp); 

    s++;
    e--;
  }

}


/* Makes a dyv of the same size as lo and hi (which must
   both be the same size such that
   dyv_ref(result,i) is uniformly randomly distributed between
   dyv_ref(lo,i) and dyv_ref(hi,i)
*/
dyv *mk_dyv_range_random(dyv *lo, dyv *hi)
{
  int i, n=dyv_size(lo);
  dyv *r = mk_dyv(n);

  assert_dyv_shape(hi,n,"mk_dyv_range_random"); 

  for (i=0; i<n; i++) {
    dyv_set(r,i,range_random(dyv_ref(lo,i),dyv_ref(hi,i)));
  }
  return r;
}

/* x := x with y appended on the end */
void append_to_dyv(dyv *x,dyv *y)
{
  int i;
  for ( i = 0 ; i < dyv_size(y) ; i++ )
    add_to_dyv(x,dyv_ref(y,i));
}

/* Return x with y appended on the end */
dyv *mk_dyv_append(dyv *x,dyv *y)
{
  dyv *z = mk_copy_dyv(x);
  append_to_dyv(z,y);
  return z;
}
/* Forall i such that ilo <= i < ihi (note the strict inequality on right)

   We do a[i] += delta
*/
void dyv_increment_block(dyv *a,int ilo,int ihi,double delta)
{
  int i;
  for ( i = ilo ; i < ihi ; i++ )
    dyv_increment(a,i,delta);
}

void random_unit_dyv(dyv *dv)
/*
   Fills up dv with a uniformly randomly chosen vector
   for which the magnitude is 1.
*/
{
  int i;
  double mag = 0.0;

  while ( mag < 1e-6 ) /* This loop will only be executed once, 999999
                          times out of 1000000 */
  {
    for ( i = 0 ; i < dyv_size(dv) ; i++ )
      dyv_set(dv,i,gen_gauss());
    mag = dyv_magnitude(dv);
  }

  dyv_scalar_mult(dv,1.0/mag,dv);
}

/* Returns d in which all components are divided by the dyv's
   sum, so the dyv sums to 1. If d is the zero vector, so is the result
   add asserts
*/
dyv *mk_norm_dyv(const dyv *d)
{
  dyv *result = mk_dyv(dyv_size(d));
  int i;
  double sum = dyv_sum(d);
  if ( sum > 1e-20 )
  {
    for ( i = 0 ; i < dyv_size(d) ; i++ )
      dyv_set(result,i,dyv_ref(d,i)/sum);
  }
  return(result);
}

/* Returns d in which all components are divided by the dyv's
   magnitude. If d is the zero vector, so is the result
*/
dyv *mk_normalize_dyv(const dyv *d)
{
  dyv *result = mk_dyv(dyv_size(d));
  int i;
  double mag = dyv_magnitude(d);
  if ( mag > 1e-20 )
  {
    for ( i = 0 ; i < dyv_size(d) ; i++ )
      dyv_set(result,i,dyv_ref(d,i)/mag);
  }
  return(result);
}

/* It's okay for the args to be the same dyv */
void normalize_dyv(const dyv *src,dyv *dest)
{
  dyv *norm = mk_normalize_dyv(src);
  copy_dyv(norm,dest);
  free_dyv(norm);
}


/* Added by Jeremy... normalize a single dyv WITHOUT
   the overhead of memory allocation/deallocation */
void normalize_this_dyv(dyv* d) {
  double sum = 0.0;
  int L = dyv_size(d);
  int i;

  for ( i = 0 ; i < L ; i++ ) {
    sum += dyv_ref(d,i);
  }

  if(sum > 1e-50) {
    for ( i = 0 ; i < L ; i++ ) {
      dyv_set(d,i,dyv_ref(d,i)/sum);
    }
  }
}

dyv *mk_random_unit_dyv(int size)
{
  dyv *result = mk_dyv(size);
  random_unit_dyv(result);
  return(result);
}
    
int safe_dyv_size(const dyv *d)
{
  check_dyv_code(d,"dyv_size");
  return(d->size);
}

dyv *mk_dyv_cusum(dyv *x)
{
  int i, size = dyv_size(x);
  dyv *y = mk_dyv(size);
  double sum = 0.0;
  for (i = 0; i < size;i++) {
    sum += dyv_ref(x,i);
    dyv_set(y,i,sum);
  }
  return y;
}
