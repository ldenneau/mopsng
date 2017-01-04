/* 
   File:        amdmex.c
   Author:      Andrew W. Moore
   Created:     Mon Apr 10 21:39:26 EDT 1995
   Updated:     8 Dec 96
   Description: Extensions and advanced amdm stuff (no direct dym data access)

   Copyright 1996, Schenley Park Research

   This file contains advanced utility functions involving dyvs dyms and
   ivecs. It never accesses the data structures directly, so if the
   underlying representation of dyms and dyvs changes these won't need to.

   The prototypes of these functions are declared at the end of amdm.h

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
#include <errno.h>
#include <math.h>
#include <float.h>

#include "amdmex.h"
#include "am_string.h"
#include "amma.h"
#include <assert.h>

/* Private prototypes */
static void maybe_prepend_errmess(const char *filename, const char *direction, const char *thing,char **r_errmess);
static void debugger_assert_is_sivec(const ivec *siv);
static int find_sivec_insert_index(const ivec *siv,int value);
static void paf(char *s,ivec *siv); /* Bleh */
static void pb(char *s,bool v); /* Should be public? Should be renamed? */
static string_array *mk_string_array_from_stream_lines(FILE *s,char **r_errmess);
void sivec_main(int argc,char *argv[]);


/* Returns the number of times val occurs in iv. */
int count_in_ivec( ivec *iv, int val)
{
  int i, count;
  count = 0;
  for (i=0; i<ivec_size( iv); ++i) if (ivec_ref( iv, i) == val) count += 1;
  return count;
}

ivec *mk_reverse_ivec(ivec *x)
{
  ivec *a = mk_ivec(ivec_size(x));
  int i;
  for ( i = 0 ; i < ivec_size(x) ; i++ )
    ivec_set(a,ivec_size(x)-i-1,ivec_ref(x,i));
  return a;
}


/* Reverse an ivec (in place) */
void reverse_ivec(ivec *x) {
  int temp;
  int s = 0;
  int e = ivec_size(x) - 1;
 
  while(s < e) {
    temp = ivec_ref(x,s);
    ivec_set(x,s,ivec_ref(x,e));
    ivec_set(x,e,temp); 

    s++;
    e--;
  }

}


/* Inverts ivec f, writing magic to values of inverse that don't occur in f.
   Passed ivec must be non-negative -- this is an unnecessary restriction,
   but 1) it seems like the usual case for calls to this function, and
   2) it avoid making the caller pass a "known good" magic value to use
   if the f is into and not onto [0,max(f)] */
ivec *mk_invert_nonneg_ivec( ivec *f)
{
  int size, max, i, val;
  ivec *finv;
  size  = ivec_size( f);
  max   = ivec_max( f);
  finv  = mk_constant_ivec( max+1, -1);
  /* Invert f. */
  for (i=0; i<size; ++i) {
    val = ivec_ref( f, i);


#ifndef AMFAST
    /* Check that val is non-negative. */
    if (val < 0) {
      my_errorf( "mk_invert_ivec_slow: ivec has negative value %d at pos %d.",
                 val, i);
    }
    /* Check that we haven't already assigned a value to finv[val]. */
    if (ivec_ref( finv, val) != -1) {
      my_errorf( "mk_inverse_ivec_slow: value %d occurs in f twice, "
                 "at indices %d and %d.",
                 val, ivec_ref( finv, val), i);
    }
#endif


    ivec_set( finv, val, i);
  }
  return finv;
}

void assert_is_permutation_ivec(const ivec *iv)
{
  int size, i, val, pos;
  ivec *iiv;

  size  = ivec_size( iv);
  iiv = mk_identity_ivec( size);

  /* "Cross-out" values of iiv that are found in iv.  Use -1-i for the
     "crossed-out" value. */
  for (i=0; i<size; ++i) {
    val = ivec_ref( iv, i);
    if (val >= size) {
      my_errorf( "assert_is_permutation_ivec: iv has length %d, "
                 "but contains value %d.\n", size, val);
    }

    pos = ivec_ref( iiv, val);
    if (pos < 0) {
      my_errorf( "assert_is_permutation_ivec: iv has value %d twice, "
                 "at pos %d and %d.", val, -pos-1, i);
    }

    ivec_set( iiv, val, -i-1);
  }

  /* Check that all values of iiv are "crossed-out". */
  for (i=0; i<size; ++i) {
    if (ivec_ref( iiv, i) >= 0) {
      my_errorf( "assert_is_permutation_ivec: iv has length %d, "
                 "but is missing value %d.", size, i);
    }
  }

  free_ivec(iiv);
  return;
}

/* Assumes that f is a permutation of the identity ivec. */
ivec *mk_invert_permutation_ivec( ivec *f)
{
  int size, i;
  ivec *finv;

#ifndef AMFAST
  assert_is_permutation_ivec( f);
#endif

  size  = ivec_size( f);
  finv  = mk_ivec( size);
  for (i=0; i<size; ++i) ivec_set( finv, ivec_ref(f, i), i);
  return finv;
}

/*
  Efficiently remove elements from iv according to indices stored
  in the sivec siv.  This is *not* a remove-by-value function.
  Its name is motivated by the similarity to the less-efficient
  ivec_remove_ivec() function.  This function operates "in-place".
  Note - ivec_remove_ivec has been modified to use this function - the
  old implementation was impossibly wasteful for large ivecs. (jkomarek 06/06)

  This function will ignore out-of-bound values in siv.  Thus
  siv can even be longer than iv, without causing an error.
*/
void ivec_remove_sivec( ivec *iv, ivec *siv)
{
  int destidx, ilen, sidx, exclidx, slen, srcidx, srcval;

#ifndef AMFAST
  if (!is_sivec(siv)) {
    my_error( "ivec_remove_sivec: siv is not a subset-ivec (sorted, unique).");
  }
#endif

  ilen = ivec_size( iv);
  slen = ivec_size( siv);

  /* Preliminary checks. */
  if (slen == 0) {
    /* Nothing to do. */
    return;
  }

  /* Normal case. */

  /* Advance sidx until exclidx is >= 0. */
  exclidx = -1;
  for (sidx=0; sidx<slen; sidx++) {
    exclidx = ivec_ref( siv, sidx);
    if (exclidx >= 0) break;
  }

  destidx = 0;
  for (srcidx=0; srcidx<ilen; ++srcidx) {
    if (srcidx != exclidx) {
      /* Copy element and advance destination index. */
      srcval = ivec_ref( iv, srcidx);
      ivec_set( iv, destidx, srcval);
      destidx += 1;
    }
    else {
      /* Advance to next exclusion index. */
      sidx += 1;
      if (sidx < slen) exclidx = ivec_ref( siv, sidx);
    }
  }

  iv->size = destidx;

  return;
}



/* Forall (i,j) such that ilo <= i < ihi
                          jlo <= j < jhi (note the strict inequality on right)

   We do a[i][j] += delta
*/
void dym_increment_block(dym *a,int ilo,int jlo,int ihi,int jhi,double delta)
{
  int i,j;
  for ( i = ilo ; i < ihi ; i++ )
    for ( j = jlo ; j < jhi ; j++ )
      dym_increment(a,i,j,delta);
}


void indices_of_sorted_ivec(ivec *v,ivec *iv)
/*
   NOTE: ivec structure (integer vectors) defined in sortind.ch
   PRE: v and iv must be same size. iv's contents irrelevant.
   POST: iv contains sorted indexes into v, so that
         forall iv[j] is the j'th smallest element of v.

         thus forall i,j, (i < j) => ivec_ref(v,iv[i]) <= ivec_ref(v,iv[j])
          and iv contains a permutation of [0 ... iv->size-1]
*/
{
  int *iarr = am_malloc_ints(ivec_size(v));
#ifndef AMFAST
  int i;
#endif
  indices_sort_integers(v->iarr,v->size,iarr);
  copy_iarr_to_ivec(iarr,v->size,iv);
  am_free_ints(iarr,v->size);
#ifndef AMFAST
  for ( i = 0 ; i < ivec_size(iv)-1 ; i++ )
  {
    int index1 = ivec_ref(iv,i);
    int index2 = ivec_ref(iv,i+1);
    if ( ivec_ref(v,index1) > ivec_ref(v,index2) )
    {
      fprintf_ivec(stdout,"v",v,"\n");
      fprintf_ivec(stdout,"iv",iv,"\n");
      printf("iv should be sorted indices of ivec, but consider\n"
	     "elements %d and %d of iv\n",i,i+1);
      my_error("mk_indices_of_sorted_ivec broken");
    }
  }
#endif
}

ivec *mk_indices_of_sorted_ivec(ivec *v)
{
  ivec *iv = mk_ivec(ivec_size(v));
  indices_of_sorted_ivec(v,iv);
  return(iv);
}


bool dym_weakly_dominates(const dym *dx, const dym *dy)
{
  dym *diff = mk_dym_subtract(dx,dy);
  bool result = ((dym_rows(dx)*dym_cols(dx))==0) ? TRUE : dym_min(diff) >= 0.0;
  free_dym(diff);
  return(result);
}

/* Sensible if args are NULL. False if different size */
bool dym_equal(const dym *x1, const dym *x2)
{
  bool result = TRUE;

  if ( EQ_PTR(x1,x2) )
    result = TRUE;
  else if ( x1 == NULL || x2 == NULL )
    result = FALSE;
  else if ( dym_rows(x1) != dym_rows(x2) )
    result = FALSE;
  else if ( dym_cols(x1) != dym_cols(x2) )
    result = FALSE;
  else
  {
    int i,j;
    for ( i = 0 ; result && i < dym_rows(x1) ; i++ ) 
      for ( j = 0 ; result && j < dym_cols(x1) ; j++ ) 
        result = result && dym_ref(x1,i,j) == dym_ref(x2,i,j); /* Note == on doubles */
  }
  return(result);
}

dym *mk_subdym( const dym *dm, const ivec *goodrows, const ivec *goodcols)
{
  return mk_dym_subset(dm, goodrows, goodcols);
}

void normalize_l1_dyv(dyv *d)
{
  dyv_scalar_mult(d,1.0/dyv_sum(d),d);
}

/* Set the sum of each row to be 1.0 */
void normalize_l1_dym_rows(dym *d)
{
  int i,j;
  double sum;
  for (i=0;i<dym_rows(d);i++)
  {
    for (sum=0.0,j=0;j<dym_cols(d);j++) sum += dym_ref(d,i,j);
    for (j=0;j<dym_cols(d);j++) dym_set(d,i,j,dym_ref(d,i,j)/sum);
  }
}

double dyv_partial_sum_slow( const dyv *dv, const ivec *indices)
{
  int imax, i, idx;
  double sum, val;  
  imax = ivec_size(indices);
  sum = 0.0;
  for (i=0; i<imax; ++i) {
    idx = ivec_ref(indices, i);
    val = dyv_ref(dv, idx);
    sum += val;
  }
  return sum;
}

double dyv_partial_sum_fast( const dyv *dv, const ivec *indices)
{
  int imax, i, *iarr;
  double sum, *farr;
  imax = ivec_size(indices);
  sum = 0.0;
  iarr = indices->iarr;
  farr = dv->farr;
  for (i=0; i<imax; ++i) sum += farr[iarr[i]];
  return sum;
}

/* PRE: all dyvs in x must have the same number of elements.

   dyv_ref(dyv_array_ref(result,i),j) == dyv_ref(dyv_array_ref(x,j),i)
*/
dyv_array *mk_dyv_array_transpose(dyv_array *x)
{
  dym *d = mk_dym_from_dyv_array(x);
  dyv_array *result = mk_dyv_array_from_dym_cols(d);
  free_dym(d);
  return result;
}

  
/* like fprintf_oneline_dyv except you say (with n) how many digits after the
   decimal you want for each number */
void fprintf_formatted_oneline_dyv(FILE *s,const char *m1, const dyv *d, 
				   const char *m2, int n)
{
  int i;
  char format[8];
  if ((n<0) || (n>100)) my_error("fprintf_formatted_oneline_dyv: bad n");
  sprintf(format,"%%.%df ",n);
  fprintf(s,"%s ",m1);
  for ( i = 0 ; i < dyv_size(d) ; i++ )
  {
    if (dyv_ref(d,i) < 0) fprintf(s,"-");
    else                  fprintf(s," ");
    fprintf(s,format,fabs(dyv_ref(d,i)));
  }
  fprintf(s,"%s",m2);
}

/* save the dyv on one line of a file in a format easy to load.
   the comment is just to make the file a small bit human readable.
   It may be NULL.
*/
void fprintf_dyv_for_load(FILE *s, const dyv *d, char *comment)
{
  int i;
  if (d)
  {
    fprintf(s,"%d ",dyv_size(d));
    for (i=0;i<dyv_size(d);i++) fprintf(s,"%8g ",dyv_ref(d,i));
  }
  else fprintf(s,"NULL");

  if (comment) fprintf(s,"%s\n",comment);
  else         fprintf(s,"\n");
}

/* Warning this function only works if the next line of fp contains a dyv
   stored using the above function
*/
dyv *mk_dyv_from_file(FILE *fp, char **r_errmess)
{
  bool ok = TRUE;
  string_array *sa = mk_string_array_from_line(fp);
  int size,i;
  dyv *res = NULL;

  if (string_array_size(sa) < 1)
    add_to_error_message(r_errmess,"Expected a dyv on a line with no non-whitespace\n");
  else
  {
    size = int_from_string(string_array_ref(sa,0),&ok);
    if ((!ok) && (!eq_string(string_array_ref(sa,0),"NULL")))
      add_to_error_message(r_errmess,"Expected an integer or NULL as the first word in a saved dyv\n");
    else
    {
      if (ok) res = mk_dyv(size);
      for (i=0;(i<size)&&(ok);i++)
      {
	dyv_set(res,i,double_from_string(string_array_ref(sa,i+1),&ok));
	if (!ok) add_to_error_message(r_errmess,"Error reading one of the elements of a dyv -- it is not parseable as a double\n");
      }
    }
  }
  if (sa) free_string_array(sa);

  return res;
}

/* Makes an ivec with random values. */
ivec *mk_random_ivec( int size, int min, int max)
{
  int i, range, val;
  ivec *riv;

  riv = mk_ivec( size);
  range = max - min + 1;

  for (i=0; i<size; ++i) {
    val = int_random( range) + min;
    ivec_set( riv, i, val);
  }

  return riv;
}

/* Create array of random ivecs of equal length. */
ivec_array *mk_random_ivec_array( int numivecs, int width, int min, int max)
{
  int i;
  ivec *iv;
  ivec_array *iva;

  iva = mk_empty_ivec_array();
  for (i=0; i<numivecs; ++i) {
    iv = mk_random_ivec( width, min, max);
    add_to_ivec_array( iva, iv);
    free_ivec( iv);
  }

  return iva;
}

/* Makes a random subset of iv of size "k" */
ivec *mk_random_ivec_subset(ivec *iv,int k)
{

  ivec *temp = mk_copy_ivec(iv);
  int r_num = ivec_size(temp) - k;
  if (r_num >0) {
    shuffle_ivec(temp);
    ivec_remove_last_n_elements(temp, r_num);
  }

  return temp;
}

ivec* mk_random_ivec_subset_fast(ivec* iv, int max_items_in_subset)
     /* Added 11/27/00 by SD */
     /* Takes O(max_items_in_subset) time rather than O(|iv|). */
     /* Temporarily scrambles iv but then unscrambles it at the end. */
     /* If max_items_in_subset > ivec_size(iv), returns copy of iv */
{
  ivec* result;
  ivec* temp;
  int i, old_size = ivec_size(iv);
  if (old_size <= max_items_in_subset)
    {
      result = mk_copy_ivec(iv);
      return result;
    } 

  temp = mk_ivec(max_items_in_subset);
  for (i = 0; i < max_items_in_subset; i++) {
      int x = ivec_ref(iv, i);
      int r = int_random(old_size - i) + i;
      int y = ivec_ref(iv, r);
      ivec_set(iv, i, y);
      ivec_set(iv, r, x);
      ivec_set(temp, i, r);
  }
  result = mk_ivec(max_items_in_subset);
  for (i = 0; i < max_items_in_subset; i++) {
    ivec_set(result, i, ivec_ref(iv, i));
  }
  for (i = max_items_in_subset-1; i >= 0; i--) {
    int ival = ivec_ref(iv, i);
    int r = ivec_ref(temp, i);
    int rval = ivec_ref(iv, r);
    ivec_set(iv, i, rval);
    ivec_set(iv, r, ival);
  }
  free_ivec(temp);
  return result;
}

ivec *mk_ivec_from_dyv(dyv *d)
{
  ivec *iv = mk_ivec(dyv_size(d));
  int i;
  for ( i = 0 ; i < dyv_size(d) ; i++ )
    ivec_set(iv,i, (int)floor(0.5 + dyv_ref(d,i)));

  return(iv);
}

dyv *mk_dyv_from_ivec(const ivec *iv)
{
  dyv *d = mk_dyv(ivec_size(iv));
  int i;
  for ( i = 0 ; i < ivec_size(iv) ; i++ )
    dyv_set(d,i, ivec_ref(iv,i));

  return(d);
}

ivec *mk_ivec_from_args(char *key,int argc,char *argv[],ivec *def)
{
  dyv *d1 = mk_dyv_from_ivec(def);
  dyv *d2 = mk_dyv_from_args(key,argc,argv,d1);
  ivec *iv = mk_ivec_from_dyv(d2);
  free_dyv(d1);
  free_dyv(d2);
  return(iv);
}

void copy_dym_row(dym *source,int source_row,dym *dest,int dest_row)
{
  int i;
  for ( i = 0 ; i < dym_cols(source) ; i++ )
    dym_set(dest,dest_row,i,dym_ref(source,source_row,i));
}

void copy_dym_col(dym *source,int source_col,dym *dest,int dest_col)
{
  int i;
  for ( i = 0 ; i < dym_rows(source) ; i++ )
    dym_set(dest,i,dest_col,dym_ref(source,i,source_col));
}

void swap_dym_rows(dym *dm,int i,int j)
/* Swaps rows i and j. Doesn't screw up if same row */
{
  if ( i != j )
  {
    dyv *row_i = mk_dyv_from_dym_row(dm,i);
    dyv *row_j = mk_dyv_from_dym_row(dm,j);
    copy_dyv_to_dym_row(row_i,dm,j);
    copy_dyv_to_dym_row(row_j,dm,i);
    free_dyv(row_i);
    free_dyv(row_j);
  }
}

void swap_dym_cols(dym *dm,int i,int j)
/* Swaps rows i and j. Doesn't screw up if same row */
{
  if ( i != j )
  {
    dyv *col_i = mk_dyv_from_dym_col(dm,i);
    dyv *col_j = mk_dyv_from_dym_col(dm,j);
    copy_dyv_to_dym_col(col_i,dm,j);
    copy_dyv_to_dym_col(col_j,dm,i);
    free_dyv(col_i);
    free_dyv(col_j);
  }
}

/* rows may be NULL dneoting "use all rows" */
double sum_of_dyv_rows(const dyv *dv,const ivec *rows)
{
  double result = 0.0;
  int i;
  int num_rows = (rows==NULL) ? dyv_size(dv) : ivec_size(rows);

  for ( i = 0 ; i < num_rows ; i++ )
  {
    int row = (rows==NULL) ? i : ivec_ref(rows,i);
    result += dyv_ref(dv,row);
  }

  return(result);
}

/* rows may be NULL denoting "use all rows" */

void sum_of_dym_rows(dym *dm,ivec *rows, dyv *r_sum)
{
  int j;
  int num_rows = (rows == NULL) ? dym_rows(dm) : ivec_size(rows);

  assert_dyv_shape(r_sum, dym_cols(dm), "sum_of_dym_rows") ;
  for ( j = 0 ; j < num_rows ; j++ )
  {
    int row = (rows==NULL) ? j : ivec_ref(rows,j);
    dyv *this_row = mk_dyv_from_dym_row(dm,row);
    dyv_plus(r_sum,this_row,r_sum);
    free_dyv(this_row);
  }
}

/* rows may be NULL dneoting "use all rows" */
dyv *mk_sum_of_dym_rows(dym *dm,ivec *rows)
{
  dyv *sum = mk_zero_dyv(dym_cols(dm));
  sum_of_dym_rows(dm, rows, sum) ;

  return(sum);
}

/* rows may be NULL dneoting "use all rows" */
dyv *mk_sumsq_of_dym_rows(dym *dm,ivec *rows)
{
  int num_rows = (rows == NULL) ? dym_rows(dm) : ivec_size(rows);
  dyv *sum = mk_zero_dyv(dym_cols(dm));
  int j;
  for ( j = 0 ; j < num_rows ; j++ )
  {
    int row = (rows==NULL) ? j : ivec_ref(rows,j);
    dyv *this_row = mk_dyv_from_dym_row(dm,row);
    dyv_mult(this_row,this_row,this_row);
    dyv_plus(sum,this_row,sum);
    free_dyv(this_row);
  }
  return(sum);
}

/* cols may be NULL dneoting "use all cols" */
dyv *mk_sumsq_of_dym_cols(dym *dm,ivec *cols)
{
  int num_cols = (cols == NULL) ? dym_cols(dm) : ivec_size(cols);
  dyv *sum = mk_zero_dyv(dym_rows(dm));
  int j;
  for ( j = 0 ; j < num_cols ; j++ )
  {
    int col = (cols==NULL) ? j : ivec_ref(cols,j);
    dyv *this_col = mk_dyv_from_dym_col(dm,col);
    dyv_mult(this_col,this_col,this_col);
    dyv_plus(sum,this_col,sum);
    free_dyv(this_col);
  }
  return(sum);
}


/* rows may be NULL dneoting "use all rows" */
dyv *mk_mean_of_dym_rows(dym *dm,ivec *rows)
{
  int num_rows = (rows == NULL) ? dym_rows(dm) : ivec_size(rows);
  dyv *mean = mk_sum_of_dym_rows(dm,rows);
  dyv_scalar_mult(mean,1.0 / int_max(1,num_rows),mean);
  return(mean);
}

/* rows may be NULL dneoting "use all rows" */
dyv *mk_sdev_of_dym_rows(dym *dm,ivec *rows)
{
  int num_rows = (rows == NULL) ? dym_rows(dm) : ivec_size(rows);
  int size = dym_cols(dm);
  dyv *mean = mk_mean_of_dym_rows(dm,rows);
  dyv *result = mk_zero_dyv(size);

  int i,k;
  for ( i = 0 ; i < size ; i++ )
    for ( k = 0 ; k < num_rows ; k++ )
    {
      int row = (rows==NULL) ? k : ivec_ref(rows,k);
      double d = dym_ref(dm,row,i) - dyv_ref(mean,i);
      dyv_increment(result,i,d * d);
    }
  for ( i = 0 ; i < size ; i++ )
    dyv_set(result,i,sqrt(dyv_ref(result,i)/int_max(1,dym_rows(dm)-1)));

  free_dyv(mean);
  return result;
}

dyv *mk_sum_of_dym_cols(dym *dm)
{
  dyv *sum = mk_zero_dyv(dym_rows(dm));
  int i;
  for ( i = 0 ; i < dym_cols(dm) ; i++ )
  {
    dyv *col = mk_dyv_from_dym_col(dm,i);
    dyv_plus(sum,col,sum);
    free_dyv(col);
  }
  return(sum);
}

dyv *mk_mean_of_dym_cols(dym *dm)
{
  dyv *mean = mk_sum_of_dym_cols(dm);
  dyv_scalar_mult(mean,1.0 / int_max(1,dym_cols(dm)),mean);
  return(mean);
}

ivec *mk_ivec_from_dym_col(dym *a,int col)
{
  dyv *x = mk_dyv_from_dym_col(a,col);
  ivec *result = mk_ivec_from_dyv(x);
  free_dyv(x);
  return result;
}

ivec *mk_ivec_from_dym_row(dym *a,int row)
{
  dyv *x = mk_dyv_from_dym_row(a,row);
  ivec *result = mk_ivec_from_dyv(x);
  free_dyv(x);
  return result;
}
	

/***** Now we'll play with dym_arrays which are adjustable length
       arrays of dyms
*****/

#define INITIAL_DYM_ARRAY_SIZE 10

dym_array *mk_empty_dym_array(void)
{
  dym_array *da = AM_MALLOC(dym_array);
  da -> size = 0;
  da -> array_size = INITIAL_DYM_ARRAY_SIZE;
  da -> array = AM_MALLOC_ARRAY(dym_ptr,da->array_size);
  return(da);
}

dym_array *mk_const_dym_array(dym *base_vec, int size){
  int i;
  dym_array *result = mk_empty_dym_array();
  for (i=0; i<size; i++) {
    add_to_dym_array(result, base_vec);
  }
  return result;
}


void add_to_dym_array(dym_array *da,dym *dm)
/*
     Assume dym_array is previously of size n. After this it is of size
   n+1, and the n+1'th element is a COPY of dm.
*/
{
  if ( da -> size == da -> array_size )
  {
    int new_size = 2 + 2 * da->array_size;
    dym **new_array = AM_MALLOC_ARRAY(dym_ptr,new_size);
    int i;
    for ( i = 0 ; i < da -> array_size ; i++ )
      new_array[i] = da->array[i];
    AM_FREE_ARRAY(da->array,dym_ptr,da->array_size);
    da -> array = new_array;
    da -> array_size = new_size;
  }
  da->array[da->size] = mk_copy_dym(dm);
  da->size += 1;
}

int dym_array_size(dym_array *da)
{
  return(da->size);
}

dym *dym_array_ref(dym_array *da,int idx)
/*
     Returns a pointer (not a copy) to the index'th element stored in
   the dym_array. Error if index < 0 or index >= size
*/
{
  dym *result;
  if ( idx < 0 || idx >= dym_array_size(da) )
  {
    result = NULL;
    my_error("dym_array_ref");
  }
  else
    result = da->array[idx];
  return(result);
}
 
void dym_array_set(dym_array *da,int idx,dym *val)
{
  if ( idx < 0 || idx >= dym_array_size(da) )
  {
    my_error("dym_array_set");
  }
  else
  {
    if (da->array[idx] != NULL)
      free_dym(da->array[idx]);
    da->array[idx] = mk_copy_dym(val);
  }
}
 
void fprintf_dym_array(FILE *s,char *m1,dym_array *da,char *m2)
{
  if ( dym_array_size(da) == 0 )
    fprintf(s,"%s = <dym_array with zero entries>%s",m1,m2);
  else
  {
    int i;
    for ( i = 0 ; i < dym_array_size(da) ; i++ )
    {
      char buff[100];
      sprintf(buff,"%s[%2d]",m1,i);
      fprintf_dym(s,buff,dym_array_ref(da,i),m2);
    }
  }
}

void pdym_array(dym_array *x)
{
  fprintf_dym_array(stdout,"dym_array",x,"\n");
}

void free_dym_array(dym_array *da)
{
  int i;
  for ( i = 0 ; i < dym_array_size(da) ; i++ )
    free_dym(da->array[i]);
  AM_FREE_ARRAY(da->array,dym_ptr,da->array_size);
  AM_FREE(da,dym_array);
}

dym_array *mk_copy_dym_array(dym_array *da)
{
  dym_array *new_ar = mk_empty_dym_array();
  int i;

  for ( i = 0 ; i < dym_array_size(da) ; i++ )
    add_to_dym_array(new_ar,dym_array_ref(da,i));

  return(new_ar);
}


/* Makes two dyvs from reading numbers out of a string.
   If format is NULL, it assumes the string consists of numbers
   separated by spaces and/or commas (which are ignored, except as separators)
   
   If format is non-null, it must be zero-terminated string in which
   each character is an 'i' or an 'o' or a -. The string is broken, again by
   replacing commas with spaces then breaking into substrings. If the
   j'th format character is 'i' includes the numeric value of j'th
   substring at right end of input_dyv. The length of result dyv is the number
   of 'i's in format. If the j'th format character is 'o' includes
   at the right of the output_dyv.

   If format is NULL, uses all the fields, and puts all but the last in
   input_dyv, and the last in output_dyv.

   If there's an error (becasue a substring didn't represent a number,
   of the number of substrings wasn't sufficient to satisfy all the
   i's in the format string) returns NULL
*/
void mk_io_dyvs_from_string(
    char *string,
    char *format,
    dyv **r_in_dyv,
    dyv **r_out_dyv
  )
{
  dyv *in;
  dyv *out;

  if ( format == NULL )
  {
    dyv *both = mk_dyv_from_string(string,NULL);
    if ( both == NULL )
    {
      in = NULL;
      out = NULL;
    }
    else
    {
      int i;
      int in_size = dyv_size(both)-1;
      in = mk_dyv(in_size);
      out = mk_dyv_1(dyv_ref(both,in_size));
      for ( i = 0 ; i < in_size ; i++ )
        dyv_set(in,i,dyv_ref(both,i));
      free_dyv(both);
    }
  }
  else
  {
    char *in_format = mk_copy_string(format);
    char *out_format = mk_copy_string(format);
    int i;
    for ( i = 0 ; format[i] != '\0' ; i++ )
    {
      if ( format[i] == 'o' )
      {
        in_format[i] = '-';
        out_format[i] = 'i';
      }
      else if ( format[i] == 'i' )
        out_format[i] = '-';
    }
    in = mk_dyv_from_string(string,in_format);
    out = mk_dyv_from_string(string,out_format);
    free_string(in_format);
    free_string(out_format);
  }

  if ( in == NULL && out != NULL )
  {
    free_dyv(out);
    out= NULL;
  }
  
  if ( out == NULL && in != NULL )
  {
    free_dyv(in);
    in= NULL;
  }
  
  *r_in_dyv = in;
  *r_out_dyv = out;
}

/* Returns a dym_array of size "num_dyms" in which the i'th
   element (forall 0 <= i < num_dyms) is a matrix of
   size "dym_size x dym_size" in which each element is 0.0 */
dym_array *mk_dym_array_of_zeroed_dyms(int num_dyms,int dym_size)
{
  dym *zero = mk_zero_dym(dym_size,dym_size);
  dym_array *result = mk_empty_dym_array();
  int i;
  for ( i = 0 ; i < num_dyms ; i++ )
    add_to_dym_array(result,zero);
  free_dym(zero);
  return result;
}

/* Returns a dym_array of size "num_dyms" in which the i'th
   element (forall 0 <= i < num_dyms) is a matrix of
   size "dym_size_r x dym_size_c" in which each element is 0.0 */
dym_array *mk_dym_array_of_zeroed_nonrect_dyms(int num_dyms, int dym_size_r,
											   int dym_size_c)
{
  dym *zero = mk_zero_dym(dym_size_r,dym_size_c);
  dym_array *result = mk_empty_dym_array();
  int i;
  for ( i = 0 ; i < num_dyms ; i++ )
    add_to_dym_array(result,zero);
  free_dym(zero);
  return result;
}

void zero_dym_array(dym_array *da)
{
  int i;
  for (i=0;i<dym_array_size(da);i++) zero_dym(dym_array_ref(da,i));
}

dyv *mk_midpoint_dyv(dyv *a,dyv *b)
{
  dyv *sum = mk_dyv_plus(a,b);
  dyv_scalar_mult(sum,0.5,sum);
  return(sum);
}

double median_of_three(double x,double y,double z)
{
  double result;
  if ( x <= y && x <= z )
    result = real_min(y,z);
  else if ( y <= x && y <= z )
    result = real_min(x,z);
  else
    result = real_min(x,y);
  return(result);
}

int find_index_of_kth_smallest(const dyv *x,int k)
{
  int size = dyv_size(x);
  ivec *indices = mk_identity_ivec(size);
  int result = -1;

  while ( result < 0 )
  {
    /* Invariant: we must find which member of indices has the
       kth smallest value and return it.

       size == size of indices */
    double pivot = median_of_three(dyv_ref(x,ivec_ref(indices,0)),
				   dyv_ref(x,ivec_ref(indices,size/2)),
				   dyv_ref(x,ivec_ref(indices,size-1)));
    ivec *lower = mk_ivec(0);
    ivec *equal = mk_ivec(0);
    ivec *higher = mk_ivec(0);
    int i;

    for ( i = 0 ; i < size ; i++ )
    {
      int idx = ivec_ref(indices,i);
      double v = dyv_ref(x,idx);
      if ( v < pivot )
        add_to_ivec(lower,idx);
      else if ( v > pivot )
        add_to_ivec(higher,idx);
      else
        add_to_ivec(equal,idx);
    }

    if ( k < ivec_size(lower) )
    {
      free_ivec(equal);
      free_ivec(higher);
      free_ivec(indices);
      indices = lower;
    }
    else if ( k < ivec_size(lower) + ivec_size(equal) )
    {
      result = ivec_ref(equal,0);
      free_ivec(lower);
      free_ivec(higher);
      free_ivec(indices);
      indices = equal;
    }
    else
    {
      k -= (ivec_size(lower) + ivec_size(equal));
      free_ivec(lower);
      free_ivec(equal);
      free_ivec(indices);
      indices = higher;
    }

    size = ivec_size(indices);
    if ( size == 0 )
      my_error("Can't happen");
    else if ( size == 1 )
      result = ivec_ref(indices,0);
  }

  free_ivec(indices);
  return(result);
}

double dyv_kth_smallest(const dyv *d,int k)
{
  int idx = find_index_of_kth_smallest(d,k);
  return(dyv_ref(d,idx));
}

double dyv_median(const dyv *d)
{
  int size = dyv_size(d);
  double result;

  if ( size == 0 )
  {
    my_error("dyv_median: zero size is not allowed");
    result = -77.8;
  }
  else if ( size % 2 == 1 )
  {
    int k = (size - 1)/2;
    result = dyv_kth_smallest(d,k);
  }
  else
  {
    int k = size/2;
    result = 0.5 * ( dyv_kth_smallest(d,k-1) + dyv_kth_smallest(d,k) );
  }
 
  return(result);
}



/*************************************************************************/


/* Saves m to the file in a format that can later be read back with
   mk_dym_from_file. */
void save_dym_to_file(FILE *s, dym *m)
{
  int i, j;
  int rows = dym_rows(m);
  int cols = dym_cols(m);

  fprintf(s, "DynamicMatrix: %d rows, %d columns\n", rows, cols);
  for (i = 0; i < rows; i++)
  {
    for (j = 0; j < cols; j++)
      fprintf(s, "%g ", dym_ref(m, i, j));
    if( cols )
      fprintf(s, "\n");
  }

  return;
}

/* Reads a dym from a file.  The dym must have been saved in the
   format produced by save_dym_to_file.  The file pointer must be
   at the start of the dym on entry, and will be at the end of the
   dym on exit.  

   If this succeeds, r_errmess is set to NULL.  Otherwise it is
   set to a message explaining the problem, and mk_dym_from_file
   returns NULL. */
dym *mk_dym_from_file(FILE *s, char **r_errmess)
{
  dym *m = NULL;
  string_array *sa;

  *r_errmess = NULL;
  sa = mk_string_array_from_line(s);  
  if (sa == NULL)
    *r_errmess = mk_copy_string("Failed when trying to read a matrix (started at end of file)");
  else if (string_array_size(sa) < 5 || 
           (strcmp(string_array_ref(sa, 0), "DynamicMatrix:") != 0) ||
           (strcmp(string_array_ref(sa, 2), "rows,") != 0) ||
           (strcmp(string_array_ref(sa, 4), "columns") != 0))
  {
    *r_errmess = mk_copy_string("Failed when reading matrix. "
                                "First line should be: DynamicMatrix: <n> rows, <m> columns");
  }
  if (!(*r_errmess))
  {
    int num_rows, num_cols=0;
    bool okayp;

    num_rows = int_from_string(string_array_ref(sa, 1), &okayp);
    if (okayp)
      num_cols = int_from_string(string_array_ref(sa, 3), &okayp);
    if (!okayp)
      *r_errmess = mk_copy_string("Failed when trying to read #rows/#columns of a matrix");
    else
    {
      int cur_row = 0;
      
      m = mk_dym(num_rows, num_cols);
      if( num_rows*num_cols )
	{
	  while (okayp && (cur_row < num_rows))
	    {
	      string_array *curline = mk_string_array_from_line(s);
	      
	      if (curline == NULL)
		{
		  okayp = FALSE;
		  *r_errmess = mk_copy_string("Failed when trying to read a matrix (reached end of file)");
		}
	      else if (string_array_size(curline) != num_cols)
		{
		  char buff[200];
		  okayp = FALSE;
		  
		  sprintf(buff, "Failed when trying to read a matrix: row %d has wrong number of columns", cur_row);
		  *r_errmess = mk_copy_string(buff);
		}
	      else
		{
		  int j;
		  
		  for (j = 0; j < num_cols; j++)
		    {         
		      dym_set(m, cur_row, j, 
			      double_from_string(string_array_ref(curline, j), &okayp));
		      if (!okayp)
			{
			  char buff[200];
			  
			  sprintf(buff, "Failed when trying to read element at row %d, column %d of matrix", 
				  cur_row, j);
			  *r_errmess = mk_copy_string(buff);
			}
		    }
		  cur_row++;
		}
	      if (curline != NULL) free_string_array(curline);
	    }
	}
    }
  }
  if (sa != NULL) free_string_array(sa);
  if ((*r_errmess != NULL) && (m != NULL))
    {
      free_dym(m);
      m = NULL;
    }
  return(m);
}

/* like mk_string_array_from_stream except there is one string per line and
   no attempt to recognize comments or other special characters
*/
string_array *mk_string_array_from_stream_lines(FILE *s,char **r_errmess)
{
  string_array *sa = mk_string_array(0);
  bool finished = FALSE;
  while ( !finished )
  {
    char *this_line = mk_string_from_line(s);
    if (!this_line) finished = TRUE;
    else add_to_string_array_no_copy(sa,this_line);
  }
  return(sa);
}

string_array *mk_string_array_from_stream(FILE *s,char **r_errmess)
{
  string_array *sa = mk_string_array_from_stream_tokens(s);
  *r_errmess = NULL;

  if ( sa == NULL )
    *r_errmess = mk_printf("Error reading tokens from file");
  return sa;
}

void save_string_array_to_stream(FILE *s, const string_array *sa)
{
  int i;
  for ( i = 0 ; i < string_array_size(sa) ; i++ )
    fprintf(s,"%s\n",string_array_ref(sa,i));
}

/* Saves dyv to the file in a format that can later be read back with
   mk_dyv_from_file. */
void save_dyv_to_stream(FILE *s, const dyv *v)
{
  string_array *sa = mk_string_array_from_dyv(v);
  save_string_array_to_stream(s,sa);
  free_string_array(sa);
}

/* Reads a dyv from a file.  The dyv must have been saved in the
   format produced by save_dyv_to_file.  The file pointer must be
   at the start of the dyv on entry, and will be at the end of the
   dyv on exit.  

   If this succeeds, r_errmess is set to NULL.  Otherwise it is
   set to a message explaining the problem, and mk_dyv_from_file
   returns NULL. */
dyv *mk_dyv_from_stream(FILE *s, char **r_errmess)
{
  string_array *sa = mk_string_array_from_stream(s,r_errmess);
  dyv *x = NULL;
  if ( sa != NULL )
    x = mk_dyv_from_string_array_with_error_message(sa,NULL,r_errmess);

  if ( sa != NULL ) free_string_array(sa);
  return x;
}

/* Saves ivec to the file in a format that can later be read back with
   mk_ivec_from_file. */
void save_ivec_to_stream(FILE *s, const ivec *v)
{
  dyv *x = mk_dyv_from_ivec(v);
  save_dyv_to_stream(s,x);
  free_dyv(x);
}

/* Reads a ivec from a file.  The ivec must have been saved in the
   format produced by save_ivec_to_file.  The file pointer must be
   at the start of the ivec on entry, and will be at the end of the
   ivec on exit.  

   If this succeeds, r_errmess is set to NULL.  Otherwise it is
   set to a message explaining the problem, and mk_ivec_from_file
   returns NULL. */
ivec *mk_ivec_from_stream(FILE *s, char **r_errmess)
{
  dyv *x = mk_dyv_from_stream(s,r_errmess);
  ivec *iv = (x == NULL) ? NULL : mk_ivec_from_dyv(x);
  if ( x != NULL ) free_dyv(x);
  return iv;
}

void maybe_prepend_errmess(const char *filename, const char *direction,
                           const char *thing,char **r_errmess)
{
  if ( *r_errmess != NULL )
  {
    char *s = mk_printf("Problem when I attempted to %s a %s from file %s: %s",
			direction,thing,filename,*r_errmess);
    free_string(*r_errmess);
    *r_errmess = s;
  }
}

/*** Supplementary string_array load/save operations ***/

/* like mk_string_array_from_filename except each line will be one string */
string_array *mk_string_array_from_filename_lines(const char *filename,char **r_errmess)
{
  FILE *s = fopen(filename,"r");
  string_array *x = NULL;
  *r_errmess = NULL;
  if ( s == NULL ) *r_errmess = mk_printf("Can't open file for reading");
  else
  {
    x = mk_string_array_from_stream_lines(s,r_errmess);
    fclose(s);
  }
  maybe_prepend_errmess(filename,"read","string_array",r_errmess);
  return x;
}

/* Loads a string_array from the given filename. 

    If succeeds, returns the string_array and sets *r_errmess = NULL.
    If fails, returns NULL and AM_MALLOCS a null-terminated error
      message string in *r_errmess
*/
string_array *mk_string_array_from_filename(const char *filename,char **r_errmess)
{
  FILE *s = fopen(filename,"r");
  string_array *x = NULL;
  *r_errmess = NULL;
  if ( s == NULL )
    *r_errmess = mk_printf("Can't open file for reading");
  else
  {
    x = mk_string_array_from_stream(s,r_errmess);
    fclose(s);
  }
  maybe_prepend_errmess(filename,"read","string_array",r_errmess);
  return x;
}

/* Loads string_array from filename. If there's a problem, prints an
   error to stdout and aborts the entire program. */
string_array *mk_string_array_from_filename_simple(const char *filename)
{
  char *errmess;
  string_array *x = mk_string_array_from_filename(filename,&errmess);
  if ( x == NULL )
    my_error(errmess);
  return x;
}

/* Saves string_array to filename. If there's a problem, prints an
   error to stdout and aborts the entire program. */
void save_string_array_to_filename(const char *filename, const string_array *x)
{
  FILE *s = fopen(filename,"w");
  if ( s == NULL )
    my_errorf("Can't open file %s for writing a string_array",filename);
  save_string_array_to_stream(s,x);
  fclose(s);
}

/*** End Supplementary string_array load/save operations ***/

/*** Supplementary dyv load/save operations ***/

/* Loads a dyv from the given filename. 

    If succeeds, returns the dyv and sets *r_errmess = NULL.
    If fails, returns NULL and AM_MALLOCS a null-terminated error
      message string in *r_errmess
*/
dyv *mk_dyv_from_filename(const char *filename,char **r_errmess)
{
  FILE *s = fopen(filename,"r");
  dyv *x = NULL;
  *r_errmess = NULL;
  if ( s == NULL )
    *r_errmess = mk_printf("Can't open file for reading");
  else
  {
    x = mk_dyv_from_stream(s,r_errmess);
    fclose(s);
  }
  maybe_prepend_errmess(filename,"read","dyv",r_errmess);
  return x;
}

/* Loads dyv from filename. If there's a problem, prints an
   error to stdout and aborts the entire program. */
dyv *mk_dyv_from_filename_simple(const char *filename)
{
  char *errmess;
  dyv *x = mk_dyv_from_filename(filename,&errmess);
  if ( x == NULL )
    my_error(errmess);
  return x;
}

/* Saves dyv to filename. If there's a problem, prints an
   error to stdout and aborts the entire program. */
void save_dyv_to_filename(const char *filename, const dyv *x)
{
  FILE *s = fopen(filename,"w");
  if ( s == NULL )
    my_errorf("Can't open file %s for writing a dyv",filename);
  save_dyv_to_stream(s,x);
  fclose(s);
}

/*** End Supplementary ivec load/save operations ***/

/*** Supplementary ivec load/save operations ***/

/* Loads a ivec from the given filename. 

    If succeeds, returns the ivec and sets *r_errmess = NULL.
    If fails, returns NULL and AM_MALLOCS a null-terminated error
      message string in *r_errmess
*/
ivec *mk_ivec_from_filename(const char *filename, char **r_errmess)
{
  FILE *s = fopen(filename,"r");
  ivec *x = NULL;
  *r_errmess = NULL;
  if ( s == NULL )
    *r_errmess = mk_printf("Can't open file for reading");
  else
  {
    x = mk_ivec_from_stream(s,r_errmess);
    fclose(s);
  }
  maybe_prepend_errmess(filename,"read","ivec",r_errmess);
  return x;
}

/* Loads ivec from filename. If there's a problem, prints an
   error to stdout and aborts the entire program. */
ivec *mk_ivec_from_filename_simple(const char *filename)
{
  char *errmess;
  ivec *x = mk_ivec_from_filename(filename,&errmess);
  if ( x == NULL )
    my_error(errmess);
  return x;
}

/* Saves ivec to filename. If there's a problem, prints an
   error to stdout and aborts the entire program. */
void save_ivec_to_filename(const char *filename, const ivec *x)
{
  FILE *s = fopen(filename,"w");
  if ( s == NULL )
    my_errorf("Can't open file %s for writing a ivec",filename);
  save_ivec_to_stream(s,x);
  fclose(s);
}

/*** End Supplementary ivec load/save operations ***/

/* Saves ma to the file in a format that can later be read back with
   mk_dym_array_from_file. */
void save_dym_array_to_file(FILE *s, dym_array *ma)
{
  int i;
  int size = dym_array_size(ma);
  dym *d;

  fprintf(s, "DynamicMatrixArray: %d components\n", size );
  for (i = 0; i < size; i++)
  {
    d = dym_array_ref( ma, i );
    save_dym_to_file( s, d );
  }

  return;
}


/* Reads a dym_array from a file.  The dym_array must have been saved in the
   format produced by save_dym_array_to_file.  The file pointer must be
   at the start of the dym_array on entry, and will be at the end of the
   dym_array on exit.  

   If this succeeds, r_errmess is set to NULL.  Otherwise it is
   set to a message explaining the problem, and mk_dym_array_from_file
   returns NULL. */
dym_array *mk_dym_array_from_file(FILE *s, char **r_errmess)
{
  dym_array *ma = NULL;
  string_array *sa;
  dym *d;
  int j;
  char *temp_string;

  *r_errmess = NULL;
  sa = mk_string_array_from_line(s);  
  if (sa == NULL)
    *r_errmess = mk_copy_string("Failed when trying to read a dynamic matrix array (started at end of file)");
  else if (string_array_size(sa) < 3 || 
           (strcmp(string_array_ref(sa, 0), "DynamicMatrixArray:") != 0) ||
           (strcmp(string_array_ref(sa, 2), "components") != 0))
    {
      *r_errmess = mk_copy_string("Failed when trying to read a dynamic matrix array. "
                                  "First line should have the format: "
                                  "DynamicMatrixArray: <n> components");
    }
  if (!(*r_errmess))
    {
      int size;
      bool okayp;
      
      size = int_from_string(string_array_ref(sa, 1), &okayp);
      if (!okayp)
	*r_errmess = mk_copy_string("Failed when trying to read #components of a dynamic matrix array");
      else
	{
	  ma = mk_empty_dym_array();
	  
	  for (j = 0; j < size; j++)
	    {         
	      if ( (d = mk_dym_from_file( s, &temp_string)) != NULL )
		add_to_dym_array( ma, d );
	      else
		{
		  *r_errmess = mk_printf("Failed when trying to read %d-th element "
                                         "of a dynamic matrix array: %s", j,temp_string);
		  free_string(temp_string);
		}
	      free_dym( d );
	    }
	}
    }
  
  if (sa != NULL) free_string_array(sa);
  if ((*r_errmess != NULL) && (ma != NULL))
    {
      free_dym_array(ma);
      ma = NULL;
    }
  return(ma);
}


string_array *mk_string_array_from_stream_tokens(FILE *s)
{
  string_array *sa = mk_string_array(0);
  bool finished = FALSE;
  while ( !finished )
  {
    string_array *this_line = mk_string_array_from_line(s);

    if ( this_line == NULL )
      finished = TRUE;
    else if ( string_array_size(this_line) > 0 &&
              string_array_ref(this_line,0)[0] != '#' )
    {
      int j;
      for ( j = 0 ; j < string_array_size(this_line) ; j++ )
        add_to_string_array(sa,string_array_ref(this_line,j));
    }
    if ( this_line != NULL ) free_string_array(this_line);
  }
  return(sa);
}

/* Returns NULL if can't open file */
string_array *mk_string_array_from_file_tokens(char *filename)
{
  FILE *s = fopen(filename,"r");
  string_array *sa = NULL;

  if ( s != NULL )
  {
    sa = mk_string_array_from_stream_tokens(s);
    fclose(s);
  }

  return(sa);
}

/* Returns NULL if can't open file */
char *mk_string_from_file_tokens(char *filename)
{
  string_array *sa = mk_string_array_from_file_tokens(filename);
  char *s = NULL;
  if ( sa != NULL )
  {
    s = mk_string_from_string_array(sa);
    free_string_array(sa);
  }
  return s;
}

void make_argc_argv_from_string_array( const string_array *sa,int *actual_argc,
                                      char ***actual_argv)
{
  int size = string_array_size(sa);
  int i;
  *actual_argv = AM_MALLOC_ARRAY(char *,size);
  for ( i = 0 ; i < size ; i++ )
    (*actual_argv)[i] = mk_copy_string(string_array_ref(sa,i));
  *actual_argc = size;  
}


/* This function AM_MALLOCS and RETURNS (in actual_argc and
   actual_argv) a new argc and argv. These are usually the same
   as argc and argv. But if argfile <filename> appears on the
   commandline, reads in all the tokens in <filename> and strings them
   together to make a longer argc argv. These new elements are
   appended onto the end of copies of the original argc argv.

   When you are done with these you should call

     free_loaded_argc_argv(actual_argc,actual_argv)

   Example:

     If the contents of file plop are:
        # This is a comment
        nsamples 35
        name Andrew

   ...and if someone runs the program foo with

       foo height 35 argfile plop noisy t

    Then after load_actual_argc_argv is called it will be as though
    the following command line was used:

       foo height 35 noisy t nsamples 35 name Andrew
*/
void load_actual_argc_argv(int argc,char *argv[],
                           int *actual_argc,char ***actual_argv)
{
  load_actual_argc_argv_with_keyword(argc, argv, actual_argc, actual_argv, "argfile");
}


void load_actual_argc_argv_with_keyword(int argc, char** argv, int* actual_argc, char*** actual_argv, char* keyword)
{
  string_array *sa = mk_string_array_from_argc_argv(argc,argv);
  int i = 0;
 
  while ( i < string_array_size(sa) && string_array_size(sa) < 5000 )
  {
    if ( caseless_eq_string(string_array_ref(sa,i), keyword) )
    {
      if ( i == string_array_size(sa)-1 )
        my_error("argfile must be followed by a filename on the commandline");
      else
      {
        char *filename = string_array_ref(sa,i+1);
        string_array *file_tokens = mk_string_array_from_file_tokens(filename);
        int j;
        string_array_remove(sa,i);
        string_array_remove(sa,i);

        fprintf(stdout,"Loaded the following command line "
                "extension from file %s:\n",filename);
  
        for ( j = 0 ; j < string_array_size(file_tokens) ; j++ )
	{
          fprintf(stdout,"%s ",string_array_ref(file_tokens,j));
          add_to_string_array(sa,string_array_ref(file_tokens,j));
	}
        fprintf(stdout,"\n");
        free_string_array(file_tokens);
      }
    }
    i += 1;
  }

  make_argc_argv_from_string_array(sa,actual_argc,actual_argv);
  free_string_array(sa);
}

void mk_filtered_argc_argv(int argc, char** argv, int* actual_argc, char*** actual_argv, char* remove)
{
  string_array *sa = mk_string_array_from_argc_argv(argc,argv);
  int i = 0;
 
  while ( i < string_array_size(sa) && string_array_size(sa) < 5000 )
  {
    if ( caseless_eq_string(string_array_ref(sa,i), remove) )
    {
      string_array_remove(sa,i);
    }
    i += 1;
  }

  make_argc_argv_from_string_array(sa,actual_argc,actual_argv);
  free_string_array(sa);
}

void free_loaded_argc_argv(int argc,char **argv)
{
  int i;
  for ( i = 0 ; i < argc ; i++ )
    free_string(argv[i]);
  AM_FREE_ARRAY(argv,char *,argc);
}


char **mk_copy_argv( int argc, char **argv)
{
  int i;
  char **newargv;
  newargv = AM_MALLOC_ARRAY( char *, argc);
  for (i=0; i<argc; ++i) newargv[i] = mk_copy_string( argv[i]);
  return newargv;
}

void free_argv_copy( int argc, char **argvcopy)
{
  /* Please do not free the *real* argv! */
  int i;
  if (argvcopy != NULL) {
    for (i=0; i<argc; ++i) free_string( argvcopy[i]);
    AM_FREE_ARRAY( argvcopy, char*, argc);
  }
  return;
}

void fprintf_argv( FILE *f, char *pre, int argc, char **argv, char *post)
{
  int i;
  fprintf( f, "%s[%s", pre, argc>0 ? argv[0] : "");
  for (i=1; i<argc; ++i) fprintf( f, ", %s", argv[i]);
  fprintf( f, "]%s", post);
  return;
}

void mk_argv_copy_append_val( int argc, char **argv, char *val,
                              int *newargc, char ***newargv)
{
  string_array *sa;

  /* Append val. */
  sa = mk_string_array_from_argc_argv( argc, argv);
  add_to_string_array( sa, val);

  make_argc_argv_from_string_array( sa, newargc, newargv);
  free_string_array( sa);
  return;
}

/* Doesn't affect argc. */
void mk_argv_copy_with_new_key_val( int argc, char **argv,
                                    char *key, char *val,
                                    char ***newargv)
{
  int idx, newargc;
  string_array *sa;

  sa = mk_string_array_from_argc_argv( argc, argv);
  idx = find_index_in_string_array( sa, key);
  if (idx < 0) {
    fprintf_string_array( stderr, "Offending String Array:", sa, "\n");
    my_errorf( "mk_argv_copy_with_new_key_val: "
               "Did not find key '%s' in string array above.", key);
  }

  /* Remove current value, and insert new value. */
  string_array_remove( sa, idx+1);
  insert_in_string_array( sa, idx+1, val);

  make_argc_argv_from_string_array( sa, &newargc, newargv);
  free_string_array( sa);
  return;
}

void mk_argv_copy_remove_key_val( int argc, char **argv, char *key,
                                  int *newargc, char ***newargv)
{
  int idx;
  string_array *sa;

  sa = mk_string_array_from_argc_argv( argc, argv);
  idx = find_index_in_string_array( sa, key);
  if (idx < 0) {
    fprintf_string_array( stderr, "Offending String Array:", sa, "\n");
    my_errorf( "mk_argv_copy_remove_key_val: "
               "Did not find key '%s' in string array above.", key);
  }

  /* Remove key and value. */
  string_array_remove( sa, idx);
  string_array_remove( sa, idx);

  make_argc_argv_from_string_array( sa, newargc, newargv);
  free_string_array( sa);
  return;
}

/*** Some basic file utilities. Should really be in amiv.ch ****/


void save_dyv_to_file_plain(char *fname,dyv *x,char **r_errmess)
{
  FILE *s = fopen(fname,"w");
  *r_errmess = NULL;
  if ( s == NULL )
    *r_errmess = mk_printf("Can't open %s for writing",fname);
  else
  {
    int i;
    for ( i = 0 ; i < dyv_size(x) ; i++ )
      fprintf(s,"%g ",dyv_ref(x,i));
    fclose(s);
  }
}

void execute_command(char *exec_string,char **r_errmess)
{
  *r_errmess = NULL;
  printf("Will execute command: %s\n",exec_string);
  system(exec_string);
  printf("Finished executing command: %s\n",exec_string);
}

dyv *mk_dyv_from_file_plain(char *fname,int size,char **r_errmess)
{
  FILE *s = fopen(fname,"r");
  dyv *x = NULL;
  *r_errmess = NULL;

  if ( s == NULL )
    *r_errmess = mk_printf("Can't open %s for reading",fname);
  else
  {
    string_array *sa = mk_string_array_from_stream_tokens(s);  
    x = mk_dyv_from_string_array_with_error_message(sa,NULL,r_errmess);
    if ( x != NULL && size >= 0 && dyv_size(x) != size )
    {
      *r_errmess = mk_printf("I wanted to load a vector of size %d from "
			     "file %s, but the one I got was of size %d",
			     size,fname,dyv_size(x));
      free_dyv(x);
      x = NULL;
    }
    free_string_array(sa);
  }
  return x;
}

/***********************************************************************
 Supplementary Load utilities for lines that start with a header
 ***********************************************************************/

static void line_header_check(string_array* sa, char* filename, 
			      char* header_str, int* linenum, 
			      bool must_have_value)
{
  if( !eq_string(string_array_ref(sa,0),header_str) )
    {
      fprintf(stdout,"Invalid header in file %s line %d - expecting %s but got %s\n", filename,*linenum,header_str,string_array_ref(sa,0));
      my_error("Error in loading file");
    }
  if( must_have_value && ( string_array_size(sa) < 2 ))
    {
      fprintf(stdout,"Invalid line in file %s line %d - expecting at least one  value for %s\n", filename,*linenum,header_str);
      my_error("Error in loading file");      
    }
}

int read_bool_from_file_with_header(FILE* fp, char* filename, char* header_str,
				    int* linenum)
{
  bool ok;
  string_array* sa;
  int temp = 0;
  char* bool_str;

  sa = mk_next_interesting_line(fp,linenum);
  line_header_check(sa,filename,header_str,linenum,TRUE);
  bool_str = string_array_ref(sa,1);
  temp = bool_from_string(bool_str,&ok);
  if( !ok )
    {
      fprintf(stdout,"Non-boolean value on line %d in file %s\n",
	      *linenum, filename);
      my_error("Error in loading file");
    }
  free_string_array(sa);
  return temp;
}

int read_int_from_file_with_header(FILE* fp, char* filename, char* header_str, 
				   int* linenum)
{
  bool ok;
  string_array* sa;
  int temp = 0;
  char* int_str;

  sa = mk_next_interesting_line(fp,linenum);
  line_header_check(sa,filename,header_str,linenum,TRUE);
  int_str = string_array_ref(sa,1);
  temp = int_from_string(int_str,&ok);
  if( !ok )
    {
      fprintf(stdout,"Non-numeric value on line %d in file %s\n",
	      *linenum, filename);
      my_error("Error in loading file");
    }
  free_string_array(sa);
  return temp;
}

double read_double_from_file_with_header(FILE* fp, char* filename, 
					 char* header_str, int* linenum)
{
  bool ok;
  string_array* sa;
  double temp = 0;
  char* double_str;

  sa = mk_next_interesting_line(fp,linenum);
  line_header_check(sa,filename,header_str,linenum,TRUE);
  double_str = string_array_ref(sa,1);
  temp = double_from_string(double_str,&ok);
  if( !ok )
    {
      fprintf(stdout,"Non-numeric value on line %d in file %s\n",
	      *linenum, filename);
      my_error("Error in loading file");
    }
  free_string_array(sa);
  return temp;
}

dyv* mk_dyv_from_file_with_header(FILE* fp, char* filename, char* header_str, 
				  int* linenum)
{
  int i;
  string_array* sa;
  dyv* temp;
  char* real_str;

  sa = mk_next_interesting_line(fp,linenum);
  line_header_check(sa,filename,header_str,linenum,FALSE);
  temp = mk_dyv(0);
  for( i = 1; i < string_array_size(sa); i++ )
    {
      real_str = string_array_ref(sa,i);
      if( is_a_number(real_str) )
	{
	  add_to_dyv(temp,atof(real_str));
	}
      else
	{
	  fprintf(stdout,"Non-numeric value on line %d in file %s\n",
		  *linenum, filename);
	  my_error("Error in loading from file");
	}
    }
  free_string_array(sa);
  return temp;
}

ivec* mk_ivec_from_file_with_header(FILE* fp, char* filename,
				    char* header_str, int* linenum)
{
  int i;
  string_array* sa;
  ivec* temp;
  char* int_str;

  sa = mk_next_interesting_line(fp,linenum);
  line_header_check(sa,filename,header_str,linenum,FALSE);
  temp = mk_ivec(0);
  for( i = 1; i < string_array_size(sa); i++ )
    {
      int_str = string_array_ref(sa,i);
      if( is_a_number(int_str) )
	{
	  add_to_ivec(temp,atoi(int_str));
	}
      else
	{
	  fprintf(stdout,"Non-numeric value on line %d in file %s\n",
		  *linenum, filename);
	  my_error("Error in loading from file");
	}
    }
  free_string_array(sa);
  return temp;
}

char* mk_string_from_file_with_header(FILE* fp,
				      char* filename,
				      char* header_str,
				      int* linenum)
{
  string_array* sa;
  char* result;

  sa = mk_next_interesting_line(fp,linenum);
  line_header_check(sa,filename,header_str,linenum,TRUE);
  result = mk_copy_string(string_array_ref(sa,1));
  free_string_array(sa);
  return result;
}

string_array* mk_string_array_from_file_with_header(FILE* fp,
						    char* filename,
						    char* header_str,
						    int* linenum)
{
  string_array* sa;

  sa = mk_next_interesting_line(fp,linenum);
  line_header_check(sa,filename,header_str,linenum,FALSE);
  string_array_remove(sa,0);
  return sa;
}

void ivec_write_with_header(FILE* fp, char* header, ivec* iv)
{
  int i;

  fprintf(fp,"%s ", header);
  for( i = 0; i < ivec_size(iv); i++ )
    {
      if( i == ( ivec_size(iv) - 1 ))
	fprintf(fp,"%d",ivec_ref(iv,i));
      else
	fprintf(fp,"%d ",ivec_ref(iv,i));
    }
  fprintf(fp,"\n");
}

void dyv_write_with_header(FILE* fp, char* header, dyv* dv)
{
  int i;

  fprintf(fp,"%s ", header);
  for( i = 0; i < dyv_size(dv); i++ )
    {
      if( i == ( dyv_size(dv) - 1 ))
	fprintf(fp,"%f",dyv_ref(dv,i));
      else
	fprintf(fp,"%f ",dyv_ref(dv,i));
    }
  fprintf(fp,"\n");
}

void string_array_write_with_header(FILE* fp, char* header, string_array* sa)
{
  int i;

  fprintf(fp,"%s ", header);
  for( i = 0; i < string_array_size(sa); i++ )
    {
      if( i == ( string_array_size(sa) - 1 ))
	fprintf(fp,"%s",string_array_ref(sa,i));
      else
	fprintf(fp,"%s ",string_array_ref(sa,i));
    }
  fprintf(fp,"\n");
}

/***********************************************************************/

/******* FILE UTILITIES *********/
/* mostly moved to am_string_array.c */

bool exload_white_space(char c)
{ /* XXX As far as I can tell, no active projects use this anymore. It's a good candidate for removal. */
  return(c == ' ' || c == '\t' || c == '\n' || c == '\r');
}

void add_to_x_from_string_array(dym *x,string_array *sa,char **r_errmess)
{
  dyv *newrow = mk_dyv_from_string_array_with_error_message(sa,NULL,r_errmess);
  if ( newrow != NULL )
  {
    if ( dyv_size(newrow) != dym_cols(x) )
      *r_errmess = mk_printf("I expected %d items in the row of datapoint "
			     "values, but in fact I found %d",
                             dym_cols(x),dyv_size(newrow));
    else
    {
      add_row(x);
      copy_dyv_to_dym_row(newrow,x,dym_rows(x)-1);
    }
  }
  if ( newrow != NULL ) free_dyv(newrow);
}

void make_attnames_and_dym_from_filename(const char *filename,int argc,char *argv[],
                                         string_array **r_attnames,
                                         dym **r_x,char **r_errmess)
{
  FILE *s = fopen(filename,"r");
  bool finished = FALSE;
  int linenum = 0;
  *r_errmess = NULL;
  *r_attnames = NULL;
  *r_x = NULL;
  
  if ( s == NULL )
    *r_errmess = mk_printf("Can't open %s for reading",filename);
  
  while ( !finished && *r_errmess == NULL )
  {
    string_array *sa = mk_next_tokens(s,&linenum,AUTO_FORMAT);
    if ( sa == NULL )
      finished = TRUE;
    else if ( *r_attnames == NULL && all_numeric(sa) )
    {
      int num_atts = string_array_size(sa);
      *r_attnames = mk_default_attribute_names(num_atts);
      *r_x = mk_dym(0,num_atts);
      add_to_x_from_string_array(*r_x,sa,r_errmess);
    }
    else if ( *r_attnames == NULL )
    {
      int num_atts = string_array_size(sa);
      *r_attnames = mk_copy_string_array(sa);
      *r_x = mk_dym(0,num_atts);
    }
    else
      add_to_x_from_string_array(*r_x,sa,r_errmess);

    if ( sa != NULL ) free_string_array(sa);
  }

  if ( *r_x == NULL && *r_errmess == NULL )
    *r_errmess = mk_printf("No data, not even attribute names!");

  if ( *r_errmess != NULL )
  {
    if ( *r_x != NULL ) free_dym(*r_x);
    if ( *r_attnames != NULL ) free_string_array(*r_attnames);
    *r_x = NULL;
    *r_attnames = NULL;
    if ( s != NULL )
    {
      char buff[1000];
      sprintf(buff,"Error on line %d of file %s: %s",
	      linenum,filename,*r_errmess);
      free_string(*r_errmess);
      *r_errmess = mk_copy_string(buff);
    }
  }

  if ( s != NULL ) fclose(s);
}

dym *mk_dym_from_filename(const char *filename,char **r_errmess)
{
  string_array *attnames;
  dym *x;
  make_attnames_and_dym_from_filename(filename,0,NULL,&attnames,&x,r_errmess);
  if ( attnames != NULL ) free_string_array(attnames);
  return x;
}

dym *mk_dym_from_filename_simple(const char *filename)
{
  char *errmess = NULL;
  dym *x = mk_dym_from_filename(filename,&errmess);
  if ( x == NULL ) my_error(errmess);
  return x;
}

void save_attnames_and_dym(FILE *s,string_array *attnames,dym *x)
{
  int i,j;
  int rows = dym_rows(x);
  int cols = dym_cols(x);
  for ( j = 0 ; j < cols ; j++ )
    fprintf(s,"%s%s",string_array_ref(attnames,j),(j==cols-1)?"\n":",");
  for ( i = 0 ; i < rows ; i++ )
    for ( j = 0 ; j < cols ; j++ )
      fprintf(s,"%g%s",dym_ref(x,i,j),(j==cols-1)?"\n":",");
}

void save_dym_to_filename(const char *filename, const dym *x)
{
  FILE *s = safe_fopen(filename, "w");
  int i,j;
  int rows = dym_rows(x);
  int cols = dym_cols(x);

  for ( i = 0 ; i < rows ; i++ )
    for ( j = 0 ; j < cols ; j++ )
      fprintf(s,"%g%s",dym_ref(x,i,j),(j==cols-1)?"\n":",");
  fclose(s);
}

/* Find least index i such that value = dyv_ref(iv,i).
  If not found, returns -1
*/
int find_index_in_dyv(dyv *dv,double value,double error)
{
  int result = -1;
  int i;

  for ( i = 0 ; i < dyv_size(dv) && result < 0 ; i++ )
    if (fabs(value-dyv_ref(dv,i))<=error) 
      result = i;
  return(result);
}

bool is_in_dyv(dyv *dv,double value,double error)
{
  return(find_index_in_dyv(dv,value,error) >= 0);
}


ivec_array *mk_array_of_empty_ivecs(int size)
{
  ivec_array *iva = mk_empty_ivec_array();
  int i;
  ivec *empty_iv = mk_ivec(0);
  for ( i = 0 ; i < size ; i++ )
    add_to_ivec_array(iva,empty_iv);
  free_ivec(empty_iv);
  return(iva);
}

ivec_array *mk_sivec_array_from_ivec_array(ivec_array *iva)
{
  ivec_array *siva = mk_empty_ivec_array();
  int i;
  for ( i = 0 ; i < ivec_array_size(iva) ; i++ )
  {
    ivec *iv = ivec_array_ref(iva,i);
    ivec *siv = mk_sivec_from_ivec(iv);
    add_to_ivec_array(siva,siv);
    free_ivec(siv);
  }
  return siva;
}

ivec *mk_union_of_sivec_array(ivec_array *iva)
{
  ivec *all = mk_ivec_from_ivec_array(iva);
  ivec *result = mk_sivec_from_ivec(all);
  free_ivec(all);
  return result;
}

ivec *mk_union_of_sivec_array_subset(ivec_array *iva,ivec *indexes)
{
  ivec_array *sub = mk_ivec_array_subset(iva,indexes);
  ivec *result = mk_union_of_sivec_array(sub);
  free_ivec_array(sub);
  return result;
}


int index_of_longest_ivec(ivec_array *iva)
{
  int result = -1;
  int size = ivec_array_size(iva);
  if ( size == 0 )
    my_error("index_of_longest_ivec zero length");
  else
  {
    int i;
    result = 0;
    for ( i = 1 ; i < size ; i++ )
    {
      if ( ivec_size(ivec_array_ref(iva,result)) < 
           ivec_size(ivec_array_ref(iva,i)) )
        result = i;
    }
  }
  return(result);
}

/* Makes an ivec from decimal integers stored in file.  Reads one int
   from each line of file.  Unlike other ivec-file ops, it is
   self-contained and doesn't assume mysterious file formats buried
   beneath multiple levels of dyv file ops and string-array file ops.
   Just loads ints, one per line. */
ivec *mk_ivec_from_file_of_ints( const char *fname)
{
  FILE *f;
  int bufsize;
  long lval, lineno;
  char *buf, *rp, *endptr;
  ivec *result;
  
  f = safe_fopen( fname, "r");
  result = mk_empty_ivec( /* initial capacity */ 1024);
  
  /* Create buf.  No 64 bit int should need more than 20 decimal
     digits.  We could support arbitrary bases (hex or ocatal) by
     specifying a base argument of 0 to strtol(), but that could lead
     to surprises when 0-padded decimal ints are read in as octal
     values.
  */

  bufsize = 128; /* 127 chars + \0 */
  buf = AM_MALLOC_ARRAY( char, bufsize);

  lineno = 0L;
  rp = NULL;
  while (!feof(f)) {
    /* Read line. */
    lineno += 1;
    rp = fgets( buf, bufsize, f);
    if (rp != buf) break;

    /* Convert to int with error checking. */
    lval = strtol( buf, &endptr, 10); /* Force base 10. */
    if (endptr == buf) {
      printf( "mk_ivec_from_file_of_ints: base line %ld:\n%s\n", lineno, buf);
      my_errorf( "mk_ivec_from_file_of_ints: value '%s' could not be\n"
		 "converted.  File='%s', line=%d",
		 buf, fname, lineno);
    }
    if (errno == ERANGE) {
      printf( "mk_ivec_from_file_of_ints: bad line %ld:\n%s\n", lineno, buf);
      my_errorf( "mk_ivec_from_file_of_ints: value '%s' caused\n"
                 "over- or underflow.  File='%s', line=%d",
		 buf, fname, lineno);
    }
    if (lval > INT_MAX || lval < INT_MIN) {
      my_errorf( "mk_ivec_from_file_of_ints: line %d contains the long int\n"
                 "%ld, and this value cannot be stored correctly as an int.",
                 lval);
    }

    /* ivecs use signed ints for the size and indexing, so we use an int
       here.  This will correctly generate an error if we wrap. */
    add_to_ivec( result, (int) lval);
  }

  /* Check for premature termination. */
  if (rp == NULL && !feof(f)) {
    perror( "Error while reading file");
    my_errorf( "Error while reading file '%s'.  System error message is\n"
               "printed above.", fname);
  }

  /* Done. */
  AM_FREE_ARRAY( buf, char, bufsize);
  fclose(f);
  
  return result;
}

/* Similar to mk_ivec_from_file_of_ints(), above. */
dyv *mk_dyv_from_file_of_doubles( const char *fname)
{
  FILE *f;
  int bufsize;
  long lineno;
  double dval;
  char *buf, *rp, *endptr;
  dyv *result;
  
  f = safe_fopen( fname, "r");
  result = mk_empty_dyv( /* initial capacity */ 1024);
  
  /* Create buf. */

  bufsize = DBL_DIG * 3; /* More than enough for a double and the \0. */
  buf = AM_MALLOC_ARRAY( char, bufsize);

  lineno = 0L;
  rp = NULL;
  while (!feof(f)) {
    /* Read line. */
    lineno += 1;
    rp = fgets( buf, bufsize, f);
    if (rp != buf) break;

    /* Convert to double with error checking. */
    dval = strtod( buf, &endptr);
    if (endptr == buf) {
      printf( "mk_dyv_from_file_of_doubles: base line %ld:\n%s\n",
	      lineno, buf);
      my_errorf( "mk_dyv_from_file_of_doubles: value '%s' could not be"
		 "converted.\nFile='%s', line=%d",
		 buf, fname, lineno);
    }
    if (errno == ERANGE) {
      printf( "mk_dyv_from_file_of_doubles: bad line %ld:\n%s\n", lineno, buf);
      my_errorf( "mk_dyv_from_file_of_doubles: value '%s' caused\n"
                 "over- or underflow.  File='%s', line=%d",
		 buf, fname, lineno);
    }

    add_to_dyv( result, dval);
  }

  /* Check for premature termination. */
  if (rp == NULL && !feof(f)) {
    perror( "Error while reading file");
    my_errorf( "Error while reading file '%s'.  System error message is\n"
               "printed above.", fname);
  }

  /* Done. */
  AM_FREE_ARRAY( buf, char, bufsize);
  fclose(f);

  return result;
}

/***************** SIVEC ***************/

/* A sivec is a regular old ivec, except it is treated as a set of
   integers.

   An ivec is a legal sivec if it is sorted in increasing order with no
   duplicates.

   The following set of functions consititute a reasonable simple
   package of set-theory operations.

   Note that everything is as efficient as possible for a set package 
   except for adding a single element and deleting a single element, 
   which (because of our representation by means of sorted ivecs) could
   take time linear in set size. */

bool is_sivec(const ivec *iv)
{
  int size = ivec_size(iv);
  int i;
  bool result = TRUE;
  for ( i = 0 ; result && i < size-1 ; i++ )
    if ( ivec_ref(iv,i) >= ivec_ref(iv,i+1) )
      result = FALSE;
  return result;
}

bool is_sivec_array(const ivec_array *ia)
{
  int size = ivec_array_size(ia);
  int i;
  bool result = TRUE;
  for (i = 0; result && i < size; i++)
    result = result & is_sivec(ivec_array_ref(ia,i));
  return result;
}

#ifdef AMFAST

void debugger_assert_is_sivec(const ivec *siv) {}

#else

static int Num_big_sivec_checks = 0;

void debugger_assert_is_sivec(const ivec *siv)
{
  bool big = ivec_size(siv) > 10;

  if ( !big || Num_big_sivec_checks < 100 )
  {
    if ( !is_sivec(siv) )
    {
      fprintf_ivec(stdout,"siv",siv,"\n");
      my_error("The above ivec was not a sivec (SetIVEC) because it was not\n"
	       "sorted. It was passed to a sivec operation in amdmex.c\n");
    }
    if ( big ) 
      Num_big_sivec_checks += 1;
  }
}
#endif

/* Makes { 0 , 1 , ... size-1 } */
ivec *mk_identity_sivec(int size)
{
  return mk_identity_ivec(size);
}

/* Returns number of elements in sivec */
int sivec_size(const ivec *siv)
{
  return ivec_size(siv);
}

/* Returns the minimum value in sivec. 
   Time cost: Constant */
int sivec_min(const ivec *siv)
{
#ifndef AMFAST
  if ( ivec_size(siv) <= 0 )
    my_error("sivec_min : empty set");
#endif
  return ivec_ref(siv,0);
}

/* Returns the minimum value in sivec. 
   Time cost: Constant */
int sivec_max(const ivec *siv)
{
#ifndef AMFAST
  if ( ivec_size(siv) <= 0 )
    my_error("sivec_max : empty set");
#endif
  return ivec_ref(siv,ivec_size(siv)-1);
}

/* If siv has 0 elements returns 0
   If value > ivec_max(siv) returns size
   If value <= ivec_min(siv) returns 0
   Else returns index such that
      value <= ivec_ref(siv,index) 
      ivec_ref(siv,index-1) < value
      
   It returns the value such that ivec_insert(iv,index,value)
   would represent the set with value added to iv (assuming value
   wasn't already in iv). */
int find_sivec_insert_index(const ivec *siv,int value)
{
  int size = ivec_size(siv);
  int result;

  debugger_assert_is_sivec(siv);

  if ( size == 0 )
    result = 0;
  else
  {
    int loval = ivec_ref(siv,0);
    if ( value <= loval ) 
      result = 0;
    else
    {
      int lo = 0;
      int hi = size-1;
      int hival = ivec_ref(siv,hi);
      if ( value > hival )
	      result = size;
      else
      {
        while ( hi > lo + 1 )
        {
          int mid = (lo + hi) / 2;
          int midval = ivec_ref(siv,mid);
          if ( midval < value )
          {
            lo = mid;
            loval = midval;
          }
          else
          {
            hi = mid;
            hival = midval;
          }
        }
        if ( loval == value )
          result = lo;
        else
          result = hi;
      }
    }
  }
#ifndef AMFAST
  {
    bool ok;
    if ( size == 0 )
      ok = result == 0;
    else if ( value > sivec_max(siv) )
      ok = result == size;
    else if ( value <= sivec_min(siv) )
      ok = result == 0;
    else
      ok = value <= ivec_ref(siv,result) &&
           ivec_ref(siv,result-1) < value;

    if ( !ok )
      my_error("find_sivec_insert_index() : bug\n");
  }
#endif

  return result;
}

/* Adds the element while maintaining legal siveckiness.
   (If element already there, no change)
   Time cost: O(size)

   If pos is not NULL, then the position at which the value was inserted
   is stored there.
*/
void add_to_sivec_special( ivec *siv, int value, int *pos)
{
  int insert_index = find_sivec_insert_index(siv,value);
  if ( insert_index >= ivec_size(siv) )
    add_to_ivec(siv,value);
  else if ( value != ivec_ref(siv,insert_index) )
    ivec_insert(siv,insert_index,value);

  if (pos != NULL) *pos = insert_index;

  return;
}

ivec *mk_add_to_sivec_special( ivec *siv, int value, int *pos)
{
  ivec *result;
  result = mk_copy_ivec( siv);
  add_to_sivec_special( result, value, pos);
  return result;
}


/* Adds the element while maintaining legal siveckiness.
   (If element already there, no change)
   Time cost: O(size) */
void add_to_sivec(ivec *siv,int value)
{
  add_to_sivec_special( siv, value, NULL);
}

ivec *mk_add_to_sivec(ivec *siv,int value)
{
  return mk_add_to_sivec_special( siv, value, NULL);
}

/* Returns -1 if the value does not exist in siv.
   Else returns index such that
      value == ivec_ref(siv,value) 
  Time cost: O(log(size)) */
int index_in_sivec(const ivec *siv,int value)
{
  int idx = find_sivec_insert_index(siv,value);
  if ( idx >= ivec_size(siv) || 
       value != ivec_ref(siv,idx) )
    idx = -1;
  return idx;
}

/* Returns true iff siv contains value
   Time cost: O(log(size)) */
bool is_in_sivec(const ivec *siv,int value)
{
  return index_in_sivec(siv,value) >= 0;
}

void sivec_remove_at_index(ivec *siv,int idx)
{
  ivec_remove(siv,idx);
}

/* Does nothing if value is not in siv.
   If value is in siv, the sivec is updated to
   represent siv \ { value } */
void sivec_remove_value(ivec *siv,int value)
{
  int idx = find_sivec_insert_index(siv,value);
  if ( idx < ivec_size(siv) && value == ivec_ref(siv,idx) )
    ivec_remove(siv,idx);
}
  
/* Returns answer to A subset-of B?
   Returns true if and only if the set of integers in a is
   a subset of the set of integers in b */
bool sivec_subset(const ivec *siva,const ivec *sivb)
{
  bool result = ivec_size(siva) <= ivec_size(sivb) ;
  int ai = 0;
  int bi = 0;
  debugger_assert_is_sivec(siva);
  debugger_assert_is_sivec(sivb);

  while ( result && ai < ivec_size(siva) )
  {
    int aval = ivec_ref(siva,ai);
    while ( bi < ivec_size(sivb) && ivec_ref(sivb,bi) < aval )
      bi += 1;
    if ( bi >= ivec_size(sivb) || ivec_ref(sivb,bi) > aval )
      result = FALSE;
    else
      ai += 1;
  }
  return result;
}


bool sivec_equal(const ivec *siva,const ivec *sivb)
{
  return ivec_equal(siva,sivb);
}

/* Returns TRUE iff A is a subset of B and A != B */
bool sivec_strict_subset(const ivec *siva,const ivec *sivb)
{
  return sivec_subset(siva,sivb) && !sivec_equal(siva,sivb);
}

ivec *mk_sivec_union(const ivec *siva,const ivec *sivb)
{
  ivec *result = mk_ivec(0);
  int ai = 0;
  int bi = 0;
  debugger_assert_is_sivec(siva);
  debugger_assert_is_sivec(sivb);
  while ( ai < ivec_size(siva) && bi < ivec_size(sivb) )
  {
    int aval = ivec_ref(siva,ai);
    int bval = ivec_ref(sivb,bi);
    if ( aval == bval )
    {
      add_to_ivec(result,aval);
      ai += 1;
      bi += 1;
    }
    else if ( aval < bval )
    {
      add_to_ivec(result,aval);
      ai += 1;
    }
    else
    {
      add_to_ivec(result,bval);
      bi += 1;
    }
  }

  while ( ai < ivec_size(siva) )
  {
    add_to_ivec(result,ivec_ref(siva,ai));
    ai += 1;
  }

  while ( bi < ivec_size(sivb) )
  {
    add_to_ivec(result,ivec_ref(sivb,bi));
    bi += 1;
  }

  debugger_assert_is_sivec(result);
  return result;
}

/* Returns A \ B.
   This is { x : x in A and x not in B } */
ivec *mk_sivec_difference(const ivec *siva,const ivec *sivb)
{
  ivec *result = mk_ivec(0);
  int ai = 0;
  int bi = 0;
  my_assert(is_sivec(siva));
  my_assert(is_sivec(sivb));
  while ( ai < ivec_size(siva) && bi < ivec_size(sivb) )
  {
    while ( ai < ivec_size(siva) && ivec_ref(siva,ai) < ivec_ref(sivb,bi) )
    {
      add_to_ivec(result,ivec_ref(siva,ai));
      ai += 1;
    }
    if ( ai < ivec_size(siva) )
    {
      while ( bi < ivec_size(sivb) && ivec_ref(sivb,bi) < ivec_ref(siva,ai) )
        bi += 1;
      while ( ai < ivec_size(siva) && 
	      bi < ivec_size(sivb) && 
	      ivec_ref(siva,ai) == ivec_ref(sivb,bi) )
      {
	ai += 1;
	bi += 1;
      }
    }
  }
  while ( ai < ivec_size(siva) )
  {
    add_to_ivec(result,ivec_ref(siva,ai));
    ai += 1;
  }
  debugger_assert_is_sivec(result);
  return result;
}

/* Returns {0,...,size_of_everything-1} \ siva. */
ivec *mk_sivec_complement( const ivec *siva, int size_of_everything)
{
  ivec *sivb, *result;
  sivb = mk_identity_ivec( size_of_everything);
  result = mk_sivec_difference( sivb, siva);
  free_ivec( sivb);
  return result;
}

/* A sivseg represents a set of integers defined by
   { ivec_ref(siv->iv,lo) , ... ivec_ref(siv->iv,hi) }
*/
typedef struct sivseg
{
  ivec *iv;
  int lo;
  int hi;
} sivseg;



#ifdef AMFAST
#define sivseg_lo(ss) (ss->lo)
#else
#define sivseg_lo(ss) (slow_sivseg_lo(ss))

static int slow_sivseg_lo(sivseg *ss)
{
  return ss->lo;
}

#endif

#ifdef AMFAST
#define sivseg_hi(ss) (ss->hi)
#else
#define sivseg_hi(ss) (slow_sivseg_hi(ss))

static int slow_sivseg_hi(sivseg *ss)
{
  return ss->hi;
}

#endif

#ifdef AMFAST
#define sivseg_size(ss) (sivseg_hi(ss) - sivseg_lo(ss) + 1)
#else
#define sivseg_size(ss) (slow_sivseg_size(ss))

static int slow_sivseg_size(sivseg *ss)
{
  return sivseg_hi(ss) - sivseg_lo(ss) + 1;
}

#endif

#ifdef AMFAST
#define sivseg_index_to_value(ss,index) (ivec_ref(ss->iv,index))
#else
#define sivseg_index_to_value(ss,index) (slow_sivseg_index_to_value(ss,index))

static int slow_sivseg_index_to_value(sivseg *ss,int index)
{
  my_assert(index >= sivseg_lo(ss));
  my_assert(index <= sivseg_hi(ss));
  return ivec_ref(ss->iv,index);
}

#endif

#ifdef AMFAST
#define sivseg_lo_value(ss) (sivseg_index_to_value(ss,sivseg_lo(ss)))
#else
#define sivseg_lo_value(ss) (slow_sivseg_lo_value(ss))

static int slow_sivseg_lo_value(sivseg *ss)
{
  return sivseg_index_to_value(ss,sivseg_lo(ss));
}

#endif

#ifdef AMFAST
#define sivseg_hi_value(ss) (sivseg_index_to_value(ss,sivseg_hi(ss)))
#else
#define sivseg_hi_value(ss) (slow_sivseg_hi_value(ss))

static int slow_sivseg_hi_value(sivseg *ss)
{
  return sivseg_index_to_value(ss,sivseg_hi(ss));
}

#endif

static int sivseg_value_to_index(sivseg *ss,int value,bool *r_exists)
{
  int lo = sivseg_lo(ss);
  int hi = sivseg_hi(ss);
  int loval = sivseg_index_to_value(ss,lo);
  int hival = sivseg_index_to_value(ss,hi);
  int result = -7777;

  if ( value < loval )
  {
    result = lo-1;
    *r_exists = FALSE;
  }
  else if ( value > hival )
  {
    result = hi;
    *r_exists = FALSE;
  }
  else if ( value == loval )
  {
    result = lo;
    *r_exists = TRUE;
  }
  else if ( value == hival )
  {
    result = hi;
    *r_exists = TRUE;
  }
  else
  {
    bool found = FALSE;

    while ( hi > lo+1 && !found )
    {
      int mid = (lo + hi) / 2;
      int midval = sivseg_index_to_value(ss,mid);
   
      my_assert(mid > lo);
      my_assert(mid < hi);
      my_assert(loval < value);
      my_assert(hival > value);

      if ( midval == value )
      {
	found = TRUE;
	result = mid;
      }
      else if ( midval < value )
      {
	lo = mid;
	loval = midval;
      }
      else
      {
	hi = mid;
	hival = midval;
      }
    }

    if ( found )
      *r_exists = TRUE;
    else
    {
      *r_exists = FALSE;
      result = lo;
    }
  }

#ifdef DOING_ASSERTS
  if ( *r_exists )
    my_assert(sivseg_index_to_value(ss,result) == value);
  else
  {
    my_assert(result >= sivseg_lo(ss)-1);
    my_assert(result <= sivseg_hi(ss));
    if ( result == sivseg_lo(ss) - 1 )
      my_assert(value < sivseg_lo_value(ss));
    else
    {
      my_assert(sivseg_index_to_value(ss,result) < value);
      if ( result < sivseg_hi(ss) )
	my_assert(sivseg_index_to_value(ss,result+1) > value);
    }
  }
#endif

  return result;
}

typedef struct sivres
{
  ivec *intersection;
  int intersection_size;
} sivres;

static void sivres_update(sivres *sr,int value)
{
  if ( sr->intersection == NULL )
    sr -> intersection_size += 1;
  else
  {
    my_assert(ivec_size(sr->intersection) == 0 || 
	     ivec_ref(sr->intersection,ivec_size(sr->intersection)-1) < value);
    add_to_ivec(sr->intersection,value);
  }
}

static void sivseg_cut(sivseg *old,sivseg *left,sivseg *right,
		       int left_hi,int right_lo)
{
  left -> iv = old -> iv;
  left -> lo = sivseg_lo(old);
  left -> hi = left_hi;

  right -> iv = old -> iv;
  right -> lo = right_lo;
  right -> hi = sivseg_hi(old);
}

/* #define VERBOSE */


#ifdef VERBOSE
static char *mk_string_from_sivseg(sivseg *ss)
{
  ivec *iv = mk_ivec_from_sivseg(ss);
  char *s = mk_string_from_ivec(iv);
  free_ivec(iv);
  return s;
}

static char *mk_spaces_string(int n)
{
  int size = n + 1;
  char *s = AM_MALLOC_ARRAY(char,size);
  int i;
  for ( i = 0 ; i < n ; i++ )
    s[i] = ' ';
  s[n] = '\0';
  return s;
}


static ivec *mk_ivec_from_sivseg(sivseg *ss)
{
  int i;
  int size = sivseg_size(ss);
  ivec *result = mk_ivec(size);

  for ( i = 0 ; i < size ; i++ )
    ivec_set(result,i,sivseg_index_to_value(ss,sivseg_lo(ss)+i));

  return result;
}


#endif

static void sivseg_search(sivseg *a,sivseg *b,sivres *sr
#ifdef VERBOSE
		   ,int depth
#endif
		   )
{
#ifdef VERBOSE
  char *sa = mk_string_from_sivseg(a);
  char *sb = mk_string_from_sivseg(b);
  char *si = mk_string_from_ivec(sr->intersection);
  char *spaces = mk_spaces_string(2 * depth);

  printf("%sEntry a = %s\n",spaces,sa);
  printf("%sEntry b = %s\n",spaces,sb);
  printf("%sEntry i = %s\n",spaces,si);
  wait_for_key();
#endif

  if ( sivseg_size(a) == 0 || sivseg_size(b) == 0 )
  {
    /* do nothing */
  }
  else if ( sivseg_lo_value(a) > sivseg_hi_value(b) )
  {
    /* do nothing */
  }
  else if ( sivseg_lo_value(b) > sivseg_hi_value(a) )
  {
    /* do nothing */
  }
  else
  {
    if ( sivseg_size(a) > sivseg_size(b) )
    {
      sivseg *c = a;
      a = b;
      b = c;
    }

    if ( sivseg_size(a) <= 2 )
    {
      int j;
      for ( j = sivseg_lo(a) ; j <= sivseg_hi(a) ; j++ )
      {
	bool exists;
	int value = sivseg_index_to_value(a,j);
	(void) sivseg_value_to_index(b,value,&exists);
	if ( exists )
	  sivres_update(sr,value);
      }
    }
    else
    {
      int mid = ( sivseg_lo(a) + sivseg_hi(a) ) / 2;
      int midval = sivseg_index_to_value(a,mid);
      bool exists;
      int bindex = sivseg_value_to_index(b,midval,&exists);
      sivseg aleft[1];
      sivseg aright[1];
      sivseg bleft[1];
      sivseg bright[1];

      sivseg_cut(a,aleft,aright,mid-1,mid+1);
      sivseg_cut(b,bleft,bright,(exists)?bindex-1:bindex,bindex+1);

      if ( sivseg_hi(bleft) >= sivseg_lo(bleft) )
	sivseg_search(aleft,bleft,sr
#ifdef VERBOSE
		      ,depth+1
#endif
		      );

      if ( exists )
	sivres_update(sr,midval);

      if ( sivseg_hi(bright) >= sivseg_lo(bright) )
	sivseg_search(aright,bright,sr
#ifdef VERBOSE
		      ,depth+1
#endif
		      );
    }
  }

#ifdef VERBOSE
  free_string(si);
  si = mk_string_from_ivec(sr->intersection);

  printf("%sExit  a = %s\n",spaces,sa);
  printf("%sExit  b = %s\n",spaces,sb);
  printf("%sExit  i = %s\n",spaces,si);
  wait_for_key();

  free_string(sa);
  free_string(sb);
  free_string(si);
#endif
}

static ivec *mk_new_sivec_intersection(const ivec *a,const ivec *b)
{
  sivres sr[1];
  sivseg ass[1];
  sivseg bss[1];
  /*
  my_assert(is_sivec(a));
  my_assert(is_sivec(b));
  */
  sr -> intersection = mk_ivec(0);
  sr -> intersection_size = -77;

  ass -> iv = (ivec *) a;
  ass -> lo = 0;
  ass -> hi = ivec_size(a)-1;

  bss -> iv = (ivec *) b;
  bss -> lo = 0;
  bss -> hi = ivec_size(b)-1;

  sivseg_search(ass,bss,sr
#ifdef VERBOSE
		,0
#endif
		);

  my_assert(sr->intersection_size == -77);

  return sr -> intersection;
}

/*
static int new_sivec_intersection_size(ivec *a,ivec *b)
{
  sivres sr[1];
  sivseg ass[1];
  sivseg bss[1];

  my_assert(is_sivec(a));
  my_assert(is_sivec(b));

  sr -> intersection = NULL;
  sr -> intersection_size = 0;

  ass -> iv = a;
  ass -> lo = 0;
  ass -> hi = ivec_size(a)-1;

  bss -> iv = b;
  bss -> lo = 0;
  bss -> hi = ivec_size(b)-1;

  sivseg_search(ass,bss,sr
#ifdef VERBOSE
		,0
#endif
		);

  return sr -> intersection_size;
}
*/


static ivec *mk_sivec_intersection_basic(const ivec *siva,const ivec *sivb)
{
  ivec *bigresult, *result;
  int ai = 0;
  int bi = 0;
  int asize, bsize, nextidx;
  debugger_assert_is_sivec(siva);
  debugger_assert_is_sivec(sivb);

  nextidx = 0;
  asize = ivec_size( siva);
  bsize = ivec_size( sivb);
  bigresult = mk_ivec( int_min( asize, bsize));

  while ( ai < ivec_size(siva) && bi < ivec_size(sivb) )
  {
    while ( ai < ivec_size(siva) && ivec_ref(siva,ai) < ivec_ref(sivb,bi) )
      ai += 1;
    if ( ai < ivec_size(siva) )
    {
      while ( bi < ivec_size(sivb) && ivec_ref(sivb,bi) < ivec_ref(siva,ai) )
        bi += 1;
      if ( bi < ivec_size(sivb) && ivec_ref(siva,ai) == ivec_ref(sivb,bi) )
      {
	ivec_set( bigresult, nextidx, ivec_ref( siva, ai));
	nextidx += 1;
	ai += 1;
	bi += 1;
      }
    }
  }

  result = mk_copy_ivec_subset( bigresult, 0, nextidx);
  free_ivec( bigresult);

  debugger_assert_is_sivec(result);
  return result;
}

ivec *mk_sivec_intersection(const ivec *siva,const ivec *sivb)
{
  int asize = ivec_size(siva);
  int bsize = ivec_size(sivb);
  int max_size = int_max(asize,bsize);
  int min_size = int_min(asize,bsize);
  ivec *result;

  if ( min_size * 100 < max_size )
    result = mk_new_sivec_intersection(siva,sivb);
  else
    result = mk_sivec_intersection_basic(siva,sivb);

  return result;
}

/*
static ivec *mk_sivec_intersection_awm(const ivec *siva,const ivec *sivb)
{
  ivec *result = NULL;

  debugger_assert_is_sivec(siva);
  debugger_assert_is_sivec(sivb);

  if ( ivec_size(siva) == 0 || ivec_size(sivb) == 0 )
    result = mk_ivec(0);
  else if ( ivec_size(siva) == sivec_max(siva)+1 )
  {
    if ( sivec_max(sivb) < ivec_size(siva) )
      result = mk_copy_ivec(sivb);
  }
  else if ( ivec_size(sivb) == sivec_max(sivb)+1 )
  {
    if ( sivec_max(siva) < ivec_size(sivb) )
      result = mk_copy_ivec(siva);
  }
  else
    result = mk_sivec_intersection_basic(siva,sivb);

  return result;
}


static int old_size_of_sivec_intersection(const ivec *siva,const ivec *sivb)
{
  int result = 0;
  int ai = 0;
  int bi = 0;
  debugger_assert_is_sivec(siva);
  debugger_assert_is_sivec(sivb);
  while ( ai < ivec_size(siva) && bi < ivec_size(sivb) )
  {
    while ( ai < ivec_size(siva) && ivec_ref(siva,ai) < ivec_ref(sivb,bi) )
      ai += 1;
    if ( ai < ivec_size(siva) )
    {
      while ( bi < ivec_size(sivb) && ivec_ref(sivb,bi) < ivec_ref(siva,ai) )
        bi += 1;
      if ( bi < ivec_size(sivb) && ivec_ref(siva,ai) == ivec_ref(sivb,bi) )
      {
	result += 1;
	ai += 1;
	bi += 1;
      }
    }
  }
#ifndef AMFAST
  {
    ivec *c = mk_sivec_intersection(siva,sivb);
    my_assert(result == ivec_size(c));
    free_ivec(c);
  }
#endif

  return result;
}

*/


/* define subset(siv,lo,hi) == { siv[lo] , siv[lo+1] , ... siv[hi] }
   thus size subset = hi - lo + 1.

   This function returns TRUE if and only if value is in subset(siv,lo,hi)
*/
static bool is_in_sivec_between(const ivec *siv,int lo,int hi,int value)
{
  bool result;
  int vlo = ivec_ref(siv,lo);
  int vhi = ivec_ref(siv,hi);

  if ( vlo == value || vhi == value )
    result = TRUE;
  else if ( lo == hi )
    result = FALSE;
  else if ( value < vlo )
    result = FALSE;
  else if ( value > vhi )
    result = FALSE;
  else
  {
    result = FALSE;
    while ( !result && hi > lo+1 )
    {
      int mid = (lo + hi) / 2;
      int vmid = ivec_ref(siv,mid);
      if ( vmid == value )
	result = TRUE;
      else if ( vmid < value )
      {
	lo = mid;
	vlo = vmid;
      }
      else
      {
	hi = mid;
	vhi = vmid;
      }
    }
  }

#ifdef DOING_ASSERTS
  {
    int index = index_in_sivec(siv,value);
    if ( index < 0 )
      my_assert(!result);
    else if ( index >= lo && index <= hi )
      my_assert(result);
    else
      my_assert(!result);
  }
#endif

  return result;
}
    
/* define subset(siv,lo,hi) == { siv[lo] , siv[lo+1] , ... siv[hi] }
   thus size subset = hi - lo + 1.

   This function returns TRUE if and only if value is in subset(siv,lo,hi)
*/
static int sio(const ivec *a,int la,int ha,const ivec *b,int lb,int hb)
{
  int result;

  if ( la == ha )
  {
    int va = ivec_ref(a,la);
    if ( lb == hb )
      result = ( va == ivec_ref(b,lb) ) ? 1 : 0;
    else
      result = ( is_in_sivec_between(b,lb,hb,va) ) ? 1 : 0;
  }
  else if ( lb == hb )
  {
    int vb = ivec_ref(b,lb);
    result = ( is_in_sivec_between(a,la,ha,vb) ) ? 1 : 0;
  }
  else
  {
    int vla = ivec_ref(a,la);
    int vlb = ivec_ref(b,lb);
    int vha = ivec_ref(a,ha);
    int vhb = ivec_ref(b,hb);
    
    if ( vha < vlb )
      result = 0;
    else if ( vhb < vla )
      result = 0;
    else
    {
      int ma = (la + ha) / 2;
      int mb = (lb + hb) / 2;
      result = sio(a,la,ma,b,lb,mb) + 
	       sio(a,la,ma,b,mb+1,hb) + 
               sio(a,ma+1,ha,b,lb,mb) + 
               sio(a,ma+1,ha,b,mb+1,hb);
    }
  }

  return result;
}

int size_of_sivec_intersection(const ivec *siva,const ivec *sivb)
{
  int result;
  debugger_assert_is_sivec(siva);
  debugger_assert_is_sivec(sivb);

  if ( ivec_size(siva) == 0 )
    result = 0;
  else if ( ivec_size(sivb) == 0 )
    result = 0;
  else
    result = sio(siva,0,ivec_size(siva)-1,sivb,0,ivec_size(sivb)-1);

#ifdef NEVER
  my_assert(result == old_size_of_sivec_intersection(siva,sivb));
#endif

  return result;
}

/* Returns TRUE iff A intersect B is empty. O(size) time */
bool sivec_disjoint(ivec *siva,ivec *sivb)
{
  bool result = TRUE;
  int ai = 0;
  int bi = 0;
  debugger_assert_is_sivec(siva);
  debugger_assert_is_sivec(sivb);
  while ( result && ai < ivec_size(siva) && bi < ivec_size(sivb) )
  {
    while ( ai < ivec_size(siva) && ivec_ref(siva,ai) < ivec_ref(sivb,bi) )
      ai += 1;
    if ( ai < ivec_size(siva) )
    {
      while ( bi < ivec_size(sivb) && ivec_ref(sivb,bi) < ivec_ref(siva,ai) )
        bi += 1;
      if ( bi < ivec_size(sivb) && ivec_ref(siva,ai) == ivec_ref(sivb,bi) )
	result = FALSE;
    }
  }
  return result;
}

ivec *mk_sivec_from_ivec(const ivec *iv)
{
  ivec *siv = mk_ivec(0);
  if ( ivec_size(iv) > 0 )
  {
    ivec *sorted = mk_ivec_sort(iv);
    int i;
    int prev = ivec_ref(sorted,0);
    add_to_ivec(siv,prev);
    for ( i = 1 ; i < ivec_size(sorted) ; i++ )
    {
      int value = ivec_ref(sorted,i);
#ifndef AMFAST
      if ( value < prev ) 
	my_error("No way");
      else
#endif     
           if ( value != prev ) {
 	     add_to_ivec(siv,value);
             prev = value;
      }
    }
    free_ivec(sorted);
  }
  return siv;
}

ivec *mk_ivec_from_string(char *s)
{
  dyv *dv = mk_dyv_from_string(s,NULL);
  ivec *iv = mk_ivec_from_dyv(dv);
  free_dyv(dv);
  return iv;
}

/* Turns a space separated string into a sivec.
   Example: "3 1 4 1 5 9" ====> { 1 , 3 , 4 , 5 , 9 } */
ivec *mk_sivec_from_string(char *s)
{
  ivec *iv = mk_ivec_from_string(s);
  ivec *siv = mk_sivec_from_ivec(iv);
  free_ivec(iv);
  return siv;
}

void paf(char *s,ivec *siv)
{
  fprintf_ivec(stdout,s,siv,"\n");
  free_ivec(siv);
}
    
void pb(char *s,bool v)
{
  printf("%s = %s\n",s,(v)?"True":"False");
}

ivec *mk_sivec_1(int v1)
{
  ivec *iv = mk_ivec_1(v1);
  ivec *siv = mk_sivec_from_ivec(iv);
  free_ivec(iv);
  return siv;
}

ivec *mk_sivec_2(int v1,int v2)
{
  ivec *iv = mk_ivec_2(v1,v2);
  ivec *siv = mk_sivec_from_ivec(iv);
  free_ivec(iv);
  return siv;
}

ivec *mk_sivec_3(int v1,int v2,int v3)
{
  ivec *iv = mk_ivec_3(v1,v2,v3);
  ivec *siv = mk_sivec_from_ivec(iv);
  free_ivec(iv);
  return siv;
}

ivec *mk_sivec_4(int v1,int v2,int v3,int v4)
{
  ivec *iv = mk_ivec_4(v1,v2,v3,v4);
  ivec *siv = mk_sivec_from_ivec(iv);
  free_ivec(iv);
  return siv;
}

ivec *mk_sivec_5(int v1,int v2,int v3,int v4,int v5)
{
  ivec *iv = mk_ivec_5(v1,v2,v3,v4,v5);
  ivec *siv = mk_sivec_from_ivec(iv);
  free_ivec(iv);
  return siv;
}

void sivec_main(int argc,char *argv[])
{
  if ( argc < 4 )
    printf("%s %s \"<integers>\" \"integers\"\n",argv[0],argv[1]);
  else
  {
    ivec *siva = mk_sivec_from_string(argv[2]);
    ivec *sivb = mk_sivec_from_string(argv[3]);
    fprintf_ivec(stdout,"siva",siva,"\n");
    fprintf_ivec(stdout,"sivb",sivb,"\n");
    paf("union",mk_sivec_union(siva,sivb));
    paf("difference",mk_sivec_difference(siva,sivb));
    paf("intersection",mk_sivec_intersection(siva,sivb));
    
    pb("subset",sivec_subset(siva,sivb));
    pb("equal",sivec_equal(siva,sivb));
    pb("strict_subset",sivec_strict_subset(siva,sivb));
    pb("disjoint",sivec_disjoint(siva,sivb));

    add_to_sivec(siva,5);
    sivec_remove_value(sivb,5);

    fprintf_ivec(stdout,"siva U { 5 }",siva,"\n");
    fprintf_ivec(stdout,"sivb \\ { 5 }",sivb,"\n");
    free_ivec(siva);
    free_ivec(sivb);
  }
}

/*----------------------- returns a random permutation of v */
void shuffle_dyv(dyv *v)
{
  int size = dyv_size(v);
  int i;
  for ( i = 0 ; i < size ; i++ )
  {
    int j = int_random(size-i);
    if ( j > 0 )
    {
      double swap_me_1 = dyv_ref(v,i);
      double swap_me_2 = dyv_ref(v,j+i);
      dyv_set(v,i,swap_me_2);
      dyv_set(v,j+i,swap_me_1);
    }
  }
  return;
}

/* Returns a dyv in which result[i] = q means
   "the i'th element of source is the q'th smallest element in source"

   result[i] = 1 means "the i'th element of source is its smallest element"
   result[i] = dyv_size(source)
               means "the i'th element of source is its biggest element"

   The big deal about this function is that it treats ties fairly, so
   that, for example, if exactly two elements (i and j) tie for smallest
   element we'll have
     result[i] = result[j] = 1.5

   Note that it is always the case that dym_sum(result) = N choose 2
   where N is the length of source */
dyv *mk_ranks_fast(dyv *source)
{
  int n = dyv_size(source);
  dyv *ranks = mk_dyv(n);

  if( n > 0 ) /* precaution against zero-dimensional source dyvs */
  {
    ivec *ind = mk_indices_of_sorted_dyv( source );
    ivec *ties = mk_zero_ivec( n );
    int i,j;
    double rank=1.0;
    int itsatie = 0;

    for(j=n-1;j>0;j--)
    {
     if( dyv_ref(source,ivec_ref(ind,j)) == dyv_ref(source,ivec_ref(ind,j-1)) )
     {
       itsatie++;
       ivec_set( ties, j, itsatie );
     }
     else
     {
       if( itsatie )
       {
          ivec_set( ties, j, itsatie+1 );
          itsatie=0;
       }
     }       
    }
    if( itsatie )
      ivec_set( ties, 0, itsatie+1 );
       
    j=0;
    rank = 1.0;
    while( j<n )
    {
      if( (itsatie = ivec_ref(ties,j)) )
      {
        double medrank = rank+0.5*(itsatie-1);
        for(i=j;i<j+itsatie;i++)
        {
          dyv_set(ranks,ivec_ref(ind,i),medrank);
        }
        rank+=(double)itsatie;
        j+=itsatie;
     }
      else
      {
       dyv_set(ranks,ivec_ref(ind,j),rank);
        rank+=1.0;
        j++;
      }
    }

    free_ivec( ind );
    free_ivec( ties );
  }

  return ranks;
}



char *mk_explain_named_dyv_seps(dyv *values)
{
  int size = dyv_size(values);
  int seps_num_bytes = size + 2;
  char *seps = AM_MALLOC_ARRAY(char,seps_num_bytes);
  int i;

  for ( i = 0 ; i < seps_num_bytes-1 ; i++ )
    seps[i] = ' ';
  seps[0] = '|';
  seps[seps_num_bytes-2] = '|';
  seps[seps_num_bytes-1] = '\0';

  return seps;
}

string_matrix *mk_explain_named_dyv_string_matrix(string_array *names,
						  dyv *values)
{
  int size = dyv_size(values);
  string_matrix *sm = mk_string_matrix(0,size);
  int row = sm_rows(sm);
  int i;

  sm_set_string(sm,row,0,"-");
  row = sm_rows(sm);

  for ( i = 0 ; i < size ; i++ )
    sm_set_string(sm,row,i,string_array_ref(names,i));

  row = sm_rows(sm);

  for ( i = 0 ; i < size ; i++ )
    sm_set_double(sm,row,i,dyv_ref(values,i));

  row = sm_rows(sm);
  sm_set_string(sm,row,0,"-");

  return sm;
}

void explain_named_dyv(string_array *names,dyv *values)
{
  char *mk_explain_named_dyv_seps(dyv *values);
  string_matrix *mk_explain_named_dyv_string_matrix(string_array *names,
						    dyv *values);

  char *seps = mk_explain_named_dyv_seps(values);
  string_matrix *sm = mk_explain_named_dyv_string_matrix(names,values);

  render_string_matrix_with_seps_and_dashes(stdout,sm,seps);

  free_string(seps);
  free_string_matrix(sm);
}


/* forall i, rx[i] := x[i] + a */
void dyv_add_constant(dyv *x,double a,dyv *rx)
{
  int i;
  for ( i = 0 ; i < dyv_size(x) ; i++ )
    dyv_set(rx,i,dyv_ref(x,i)+a);
}

dyv *mk_dyv_add_constant(dyv *x, double a)
{
  dyv *res = mk_dyv(dyv_size(x));
  dyv_add_constant(x,a,res);
  return res;
}

/* Returns dyv of size ivec_size(rows) in which
    result[i] = x[rows[i]] */
dyv *mk_dyv_subset( const dyv *x, const ivec *rows)
{
  int size = ivec_size(rows);
  dyv *y = mk_dyv(size);
  int i;
  for ( i = 0 ; i < size ; i++ )
    dyv_set(y,i,dyv_ref(x,ivec_ref(rows,i)));
  return y;
}


/* Writes dv[a] to dv[b-1] into r_dyv.  If a or b is negative, then
   it is replaced by by len+a or len+b. Returns the number of elements
   written.  This is more-or-less equivalent to Python slice syntax.
*/
int dyv_slice( const dyv *dv, int a, int b, dyv *r_dv)
{
  int dvlen, lb, ub, r_dvlen, srcidx, dstidx;
  double dval;

  /* Set lower- and (strict) upper-bounds. */
  dvlen = dyv_size( dv);
  if (a < 0) lb = int_max(0, dvlen+a);
  else lb = int_min(dvlen, a);
  if (b < 0) ub = int_max(0, dvlen+b);
  else ub = int_min(dvlen, b);
  r_dvlen = ub - lb;

  /* Copy elements. */
  for (srcidx=lb,dstidx=0;  srcidx<ub;  ++srcidx,++dstidx) {
    dval = dyv_ref( dv, srcidx);
    dyv_set( r_dv, dstidx, dval);
  }

  return r_dvlen;
}

/* If b >= a >= 0, creates "slice" of dv on [a,b-1].  If a or b < 0, then
   it is replaced by len+a or len+b.  Indices which exceed bounds are
   replaced by appropriate indices.  Therefore this function always succeeds,
   sometimes returning an empty dyv.
*/
dyv *mk_dyv_slice( const dyv *dv, int a, int b)
{
  int dvlen, lb, ub, r_dvlen;
  dyv *r_dv;

  /* Set lower- and (strict) upper-bounds. */
  dvlen = dyv_size( dv);
  if (a < 0) lb = int_max(0, dvlen+a);
  else lb = int_min(dvlen, a);
  if (b < 0) ub = int_max(0, dvlen+b);
  else ub = int_min(dvlen, b);
  r_dvlen = ub - lb;

  r_dv = mk_dyv( r_dvlen);
  dyv_slice( dv, lb, ub, r_dv);
  return r_dv;
}

/* Returns sorted dyv with unique values from dv. */
dyv *mk_dyv_unique( dyv *dv)
{
  int size, pos, i;
  double oldval, val;
  dyv *sorted, *uniq, *uniqtmp;

  size = dyv_size( dv);
  if (size == 0) {
    uniq = mk_dyv( 0);
  }
  else {
    sorted = mk_dyv_sort( dv);
    uniqtmp =mk_dyv( size);

    /* Initialize result. */
    pos = 0;
    oldval = dyv_ref( sorted, 0);
    dyv_set( uniqtmp, pos++, oldval);
    
    /* Get unique values. */
    for (i=1; i<size; ++i) {
      val = dyv_ref( sorted, i);
      if (val != oldval) dyv_set( uniqtmp, pos++, val);
    }
    free_dyv( sorted);

    /* Shrink dyv. */
    uniq = mk_dyv_slice( uniqtmp, 0, pos);
    free_dyv( uniqtmp);
  }

  return uniq;
}



/* Two handy conversions, based on mk_dyv_array_from_file. Added by David
   Cohn, 26 Aug 02.
*/

dyv_array *mk_dyv_array_from_dym_rows(dym *dm)
{
  if (dm == NULL)
    return NULL ;
  else {
    dyv_array *da = mk_empty_dyv_array();

    int num_rows = dym_rows(dm) ;
    int j ;
    for (j = 0; j < num_rows; j++) {
      dyv *d = mk_dyv_from_dym_row(dm, j) ;
      add_to_dyv_array( da, d ) ;
      free_dyv(d);
    }
    return(da);
  }
}

dyv_array *mk_dyv_array_from_dym_cols(dym *dm)
{
  if (dm == NULL)
    return NULL ;
  else {
    dyv_array *da = mk_empty_dyv_array();

    int num_cols = dym_cols(dm) ;
    int j ;
    for (j = 0; j < num_cols; j++) 
    {
      dyv *d = mk_dyv_from_dym_col(dm, j) ;
      add_to_dyv_array( da, d ) ;
      free_dyv(d);
    }
    return(da);
  }
}

#ifdef AWM_THINKS_NO_LONGER_NEEDED

/* Saves da to the file in a format that can later be read back with
   mk_dyv_array_from_file. */
void save_dyv_array_to_file(FILE *s, const dyv_array *da)
{
  int i;
  int size = dyv_array_size(da);
  dyv *d;

  fprintf(s, "DynamicVectorArray: %d components\n", size );
  for (i = 0; i < size; i++)
  {
    d = dyv_array_ref( da, i );
    save_dyv_to_stream( s, d );
  }

  return;
}

/* Reads a dyv_array from a file.  The dyv_array must have been saved in the
   format produced by save_dyv_array_to_file.  The file pointer must be
   at the start of the dyv_array on entry, and will be at the end of the
   dyv_array on exit.  

   If this succeeds, r_errmess is set to NULL.  Otherwise it is
   set to a message explaining the problem, and mk_dyv_array_from_file
   returns NULL. */
dyv_array *mk_dyv_array_from_file(FILE *s, char **r_errmess)
{
  dyv_array *da = NULL;
  string_array *sa;
  dyv *d;
  int j;
  char *temp_string;

  *r_errmess = NULL;
  sa = mk_string_array_from_line(s);  
  if (sa == NULL)
    *r_errmess = mk_copy_string("Failed when trying to read a dynamic matrix array (started at end of file)");
  else if (string_array_size(sa) < 3 || 
           (strcmp(string_array_ref(sa, 0), "DynamicVectorArray:") != 0) ||
           (strcmp(string_array_ref(sa, 2), "components") != 0))
    {
      *r_errmess = mk_copy_string("Failed when trying to read a dynamic matrix array. "
                                  "First line should have the format: "
                                  "DynamicMatrixArray: <n> components");
    }
  if (!(*r_errmess))
    {
      int size;
      bool okayp;
      
      size = int_from_string(string_array_ref(sa, 1), &okayp);
      if (!okayp)
	*r_errmess = mk_copy_string("Failed when trying to read #components of a dynamic matrix array");
      else
	{
	  da = mk_empty_dyv_array();
	  
	  for (j = 0; j < size; j++)
	    {         
	      if ( (d = mk_dyv_from_stream( s, &temp_string)) != NULL )
            add_to_dyv_array( da, d );
	      else
		{
		  *r_errmess = mk_printf("Failed when trying to read %d-th element "
                                         "of a dynamic matrix array: %s", j,temp_string);
		  free_string(temp_string);
		}
	      free_dyv( d );
	    }
	}
    }
  
  if (sa != NULL) free_string_array(sa);
  if ((*r_errmess != NULL) && (da != NULL))
    {
      free_dyv_array(da);
      da = NULL;
    }
  return(da);
}

#endif

/* PRE: all dyvs in x must have the same number of elements.
      : x must contains at least one dyv

   Makes dym by treating the i'th element of x as the ith row of matrix

   dym_ref(result,i,j) == dyv_ref(dyv_array_ref(x,i),j)
*/
dym *mk_dym_from_dyv_array(dyv_array *x)
{
  dym *result;
  int i;
  int rows = dyv_array_size(x);
  int cols;

  if ( rows == 0 )
    my_errorf("mk_dym_from_dyv_array: should have at least one element");

  cols = dyv_size(dyv_array_ref(x,0));

  result = mk_dym(rows,cols);

  for ( i = 0 ; i < rows ; i++ )
    copy_dyv_to_dym_row(dyv_array_ref(x,i),result,i);

  return result;
}

ivec *mk_ivec_intersection(ivec *a,ivec *b)
{
  ivec *sa = mk_sivec_from_ivec(a);
  ivec *sb = mk_sivec_from_ivec(b);
  ivec *result = mk_sivec_intersection(sa,sb);
  free_ivec(sa);
  free_ivec(sb);
  return result;
}

/*
  mk_dym_subset - builds a dym out of a subset of rows
  and columns from a matrix x such that if:
    r_given = rows(i)
    c_given = cols(j)
    val = x(r_given,c_given)
  then:
    new_dym(i,j) = val;
  Note: this can also be used to permute rows and columns.

  rows may be NULL meaning "use all rows"
  cols may be NULL meaning "use all columns"
*/
dym *mk_dym_subset( const dym *x, const ivec *rows, const ivec *cols)
{
  int Rn = (rows==NULL) ? dym_rows(x) : ivec_size(rows);
  int Cn = (cols==NULL) ? dym_cols(x) : ivec_size(cols);
  int r, c;
  dym* nu;

  nu = mk_zero_dym(Rn,Cn);

  for(r=0;r<Rn;r++)
  {
    int row = (rows==NULL) ? r : ivec_ref(rows,r);
    for(c=0;c<Cn;c++)
    {
      int col = (cols==NULL) ? c : ivec_ref(cols,c);
      dym_set(nu,r,c,dym_ref(x,row,col));
    }
  }

  return nu;
}


dym *mk_dym_from_col_subset(dym *x, ivec *cols)
{
  int i,j;
  dym *res = mk_dym(dym_rows(x),ivec_size(cols));
  for (i=0;i<ivec_size(cols);i++)
  {
    int col = ivec_ref(cols,i);
    for (j=0;j<dym_rows(x);j++) dym_set(res,j,i,dym_ref(x,j,col));
  }
  return res;
}


ivec* mk_find_all_indices_in_dyv(dyv* vect, double val, double tol)
{
  double upper = val + tol;
  double lower = val - tol;
  int N = dyv_size(vect);
  int count = 0;
  int i;
  ivec* res;

  /* Search done in 2 passes to prevent having to add to an ivec */
  for(i=0;i<N;i++)
    {
      if( (dyv_ref(vect,i) <= upper) && (dyv_ref(vect,i) >= lower ) )
	{
	  count++;
	}
    }

  res = mk_ivec(count);

  count = 0;
  for(i=0;i<N;i++)
    {
      if( (dyv_ref(vect,i) <= upper) && (dyv_ref(vect,i) >= lower ) )
	{
	  ivec_set(res,count,i);
	  count++;
	}
    }

  return res;
}


ivec* mk_find_all_indices_in_ivec(ivec* vect, int val)
{
  int N = ivec_size(vect);
  int count = 0;
  int i;
  ivec* res;

  /* Search done in 2 passes to prevent having to add to an ivec */
  for(i=0;i<N;i++)
    {
      if( ivec_ref(vect,i) == val)
	{
	  count++;
	}
    }

  res = mk_ivec(count);

  count = 0;
  for(i=0;i<N;i++)
    {
      if( ivec_ref(vect,i) == val)
	{
	  ivec_set(res,count,i);
	  count++;
	}
    }

  return res;
}

void save_dyv_as_dym(FILE *s, const dyv *dv)
{
  dym *dm = mk_dym(dyv_size(dv), 1);
  copy_dyv_to_dym_col(dv, dm, 0);
  save_dym_to_file(s, dm);
  free_dym(dm);
}

dyv *load_dyv_as_dym(FILE *s, char **err)
{
  dyv *ret;
  dym *dm = mk_dym_from_file(s, err);
  if(*err)
    return NULL;
  if(dym_cols(dm) > 1) {
    *err = mk_printf("more than one column");
    return NULL;
  }
  ret = mk_dyv_from_dym_col(dm, 0);
  free_dym(dm);
  return ret;
}

ivec *load_ivec_as_dym(FILE *s, char **err)
{
  ivec *ret;
  dyv *dv = load_dyv_as_dym(s, err);
  if(*err)
    return NULL;
  ret = mk_ivec_from_dyv(dv);
  free_dyv(dv);
  return ret;
}

void save_ivec_as_dym(FILE *s, const ivec *iv)
{
  dyv *dv = mk_dyv_from_ivec(iv);
  save_dyv_as_dym(s, dv);
  free_dyv(dv);
}


/***************************************************************************/

dyv *mk_dyv_extremes_from_center( dyv *dv, double center, int numextremes,
				  ivec **indices)
{
  int size, i, lasthigh, high, low;
  double vallow, valhigh;
  ivec *redirect;
  dyv *sortdv, *subdv;


  size = dyv_size( dv);

  sortdv = mk_sorted_dyv( dv);
  subdv  = mk_dyv( numextremes);
  if (indices != NULL) {
    *indices = mk_ivec( numextremes);
    redirect = mk_indices_of_sorted_dyv( dv);
  }
  else redirect = NULL;

  /* Check for NaN values. */
  for (i=0; i<size; ++i) {
    if (am_isnan(dyv_ref(sortdv, i))) {
      fprintf( stderr, "mk_dyv_extremes: warning, dyv has nan values.\n");
      break;
    }
  }

  /* Get extreme values. */
  high    = size-1;
  low     = 0;
  lasthigh = 0;
  for (i=0; i<numextremes; ++i) {

    if (high > low) {
      valhigh = dyv_ref( sortdv, high);
      vallow  = dyv_ref( sortdv, low);
    }
    else {
      valhigh = center;
      vallow  = center;
    }

    if (lasthigh) { /* prefer low extreme on equality */
      if (fabs(vallow-center) >= fabs(valhigh-center)) {
	dyv_set( subdv, i, vallow);
	if (indices != NULL) {
	  if (low < size) ivec_set( *indices, i, ivec_ref( redirect, low));
	  else ivec_set( *indices, i, -1);
	}
	low += 1;
	lasthigh = 0;
      }
      else {
	dyv_set( subdv, i, valhigh);
	if (indices != NULL) ivec_set( *indices, i, ivec_ref( redirect, high));
	if (indices != NULL) {
	  if (high >= 0) ivec_set( *indices, i, ivec_ref( redirect, high));
	  else ivec_set( *indices, i, -1);
	}
	high -= 1;
	lasthigh = 1;
      }
    }

    else { /* prefer high extreme on equality */
      if (fabs(vallow-center) > fabs(valhigh-center)) {
	dyv_set( subdv, i, vallow);
	if (indices != NULL) {
	  if (low < size) ivec_set( *indices, i, ivec_ref( redirect, low));
	  else ivec_set( *indices, i, -1);
	}
	low += 1;
	lasthigh = 0;
      }
      else {
	dyv_set( subdv, i, valhigh);
	if (indices != NULL) {
	  if (high >= 0) ivec_set( *indices, i, ivec_ref( redirect, high));
	  else ivec_set( *indices, i, -1);
	}
	high -= 1;
	lasthigh = 1;
      }
    }
  }

  free_ivec( redirect);
  free_dyv( sortdv);

  return subdv;
}

dyv *mk_dyv_extremes( dyv *dv, int numextremes, ivec **indices)
{
  return mk_dyv_extremes_from_center( dv, 0.0, numextremes, indices);
}

void fprintf_oneline_dyv_extremes_from_center( FILE *f, char *pre, dyv *dv,
					       char *post, double center,
					       int numvals)
{
  ivec *indices;
  dyv *extdv;

  extdv = mk_dyv_extremes_from_center( dv, center, numvals, &indices);
  fprintf_oneline_dyv( f, pre, extdv, post);
  fprintf_ivec( f, pre, indices, post);
  free_dyv( extdv);
  free_ivec( indices);
  return;
}

void fprintf_oneline_dyv_extremes( FILE *f, char *pre, dyv *dv, char *post,
				   int numvals)
{
  fprintf_oneline_dyv_extremes_from_center( f, pre, dv, post, 0.0, numvals);
  return;
}

void fprintf_subdym( FILE *f, char *pre, dyv *dv, dym *d, char *post,
		     const ivec *goodrows, const ivec *goodcols)
{
  dym *subdym;
  subdym = mk_subdym( d, goodrows, goodcols);
  fprintf_dym( f, pre, subdym, post);
  free_dym( subdym);
  return;
}

ivec *mk_sivec_from_stream_tokens(FILE *s)
{
  ivec *ret = mk_ivec(0);
  bool finished = FALSE;
  while ( !finished )
  {
    string_array *this_line = mk_string_array_from_line(s);

    if ( this_line == NULL )
      finished = TRUE;
    else if ( string_array_size(this_line) > 0 &&
              string_array_ref(this_line,0)[0] != '#' )
    {
      int j;
      for ( j = 0 ; j < string_array_size(this_line) ; j++ ) {
        int num;
        int nfields = sscanf(string_array_ref(this_line,j), "%d", &num);
        if(nfields < 1) {
          my_errorf("Bad number %s", string_array_ref(this_line,j));
        }
        if(num < 0) {
          my_errorf("Numbers shuold be positive, you gave %d\n", num);
        }

        add_to_sivec(ret,num);
      }
    }
    if ( this_line != NULL ) free_string_array(this_line);
  }
  return(ret);
}

/* Returns NULL if can't open file */
ivec *mk_sivec_from_file_tokens(const char *const filename)
{
  FILE *s = fopen(filename,"r");
  ivec *ret = NULL;

  if ( s != NULL )
  {
    ret = mk_sivec_from_stream_tokens(s);
    fclose(s);
  }

  return(ret);
}

/* Increases the number of rows in dm by 1 and copies the contents
   of dv into that new bottom row. Note this means that 
     dyv_size(dv) must equal dym_cols(dm)
   in order that dv will fit (if this is violated there'll be a my_error */
void add_dyv_to_dym(dym *dm,dyv *dv)
{
  add_row(dm);
  copy_dyv_to_dym_row(dv,dm,dym_rows(dm)-1);
}

