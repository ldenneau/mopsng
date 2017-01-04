/*
   File:        amdym.c
   Author:      Andrew W. Moore
   Created:     Thu Sep 15 21:01:13 EDT 1994
   Updated:     amdm was split into amdyv, amdym and svd by Frank Dellaert, Aug 14 1997
   Description: Dynamically allocated and deallocated matrices

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

#include "amma.h"      /* Fast, non-fragmenting, memory management */
#include "amar.h"      /* Obvious operations on 1-d arrays */
#include "amdym.h"     /* matrices */
#include "am_string.h"

#define DYM_CODE 4508
#define FREED_DYM_CODE 1234321

#define NOTHING_TO_DO


/* Prototypes for Private functions */


static void check_dym_access(const dym *d,int i, int j, const char *name);
static double dym_extreme(dym *dv, bool max);


#ifdef AMFAST

#define check_dym_code(d,name) NOTHING_TO_DO

#else /* if AMFAST is not defined... */

static void check_dym_code(const dym *d, const char *name); /* Private */

void check_dym_code(const dym *d, const char *name)
{
  if ( d == NULL )
  {
    fprintf(stderr,"NULL dym passed in operation %s\n",name);
    my_error("dym data structure");
  }
  if ( d->dym_code != DYM_CODE )
  {
    if (d->dym_code == FREED_DYM_CODE ) {
      my_errorf( "Looks like you're trying to do something to a dym\n"
                 "that you have previously freed\n");
    }
    else {
      fprintf(stderr,"Attempt to access a non-allocated DYnamic Matrix\n");
      fprintf(stderr,"This is in the operation %s\n",name);
      my_error("dym data structure error");
    }
  }
}

#endif /* #ifdef AMFAST */

/*
* check_dym_access is only called in safe_dym_xxx
* It used to be non-functional with AMFAST,
* but Frank Dellaert reinstated it June 30 1997
*/

void check_dym_access(const dym *d,int i, int j, const char *name)
{
  check_dym_code(d,name); /* non-functional if AMFAST */

  if ( i < 0 || i >= d->rows || j < 0 || j >= d->cols )
  {
    fprintf(stderr,"In operation \"%s\"\n",name);
    fprintf(stderr,"the dym (dynamic matrix) has rows = %d, cols = %d\n",
            d->rows,d->cols
           );
    fprintf(stderr,"You tried to use indices i=%d j=%d\n",i,j);
    fprintf(stderr,"Here is the dym that was involved:\n");
    fprintf_dym(stderr,"dm",d,"\n");
    my_error("check_dym_access");
  }
}


#ifndef AMFAST

void assert_dym_shape(dym *d,int rows, int cols,char *name)
{
  check_dym_code(d,name);

  if ( rows != d->rows || cols != d->cols )
  {
    fprintf(stderr,"In operation \"%s\"\n",name);
    fprintf(stderr,"the dym (dynamic matrix) has rows = %d, cols = %d\n",
            d->rows,d->cols
           );
    fprintf(stderr,"But should have been predefined with the shape:\n");
    fprintf(stderr,"rows = %d, cols = %d\n",rows,cols);
    my_error("assert_dym_shape");
  }
}

#endif /* #ifdef AMFAST */

static int Dyms_mallocked = 0;
static int Dyms_freed = 0;

dym *mk_dym(int rows,int cols)
{
  dym *result = AM_MALLOC(dym);
  result -> dym_code = DYM_CODE;
  result -> rows = rows;
  result -> cols = cols;
  result -> rows_allocated = rows;
  result -> tdarr = am_malloc_2d_realnums(rows,cols);
  Dyms_mallocked += 1;
  return(result);
}

void dym_destructive_resize(dym* x, int rows, int cols)
{
  int i;

  for ( i = 0 ; i < x -> rows ; i++ )
    {
      am_free_realnums(x->tdarr[i],x->cols);
      x->tdarr[i] = NULL;
    }

  AM_FREE_ARRAY(x->tdarr,double_ptr,x->rows_allocated);
  x->rows = rows;
  x->cols = cols;
  x->rows_allocated = rows;
  x->tdarr = am_malloc_2d_realnums(rows, cols);
}

dym *mk_dym_x( int rows, int cols, ...)
{
  /* Warning: no type checking can be done by the compiler.  You *must*
     send the values as doubles for this to work correctly. */
  int i, j;
  double val;
  va_list argptr;
  dym *dm;
  
  dm = mk_dym( rows, cols);

  va_start( argptr, cols);
  for (i=0; i<rows; ++i) {
    for (j=0; j<cols; ++j) {
      val = va_arg( argptr, double);
      dym_set( dm, i,j, val);
    }
  }
  va_end(argptr);

  return dm;
}

double dym_sum(const dym *x)
{
  int i,j;
  double sum = 0.0;
  for ( i = 0 ; i < dym_rows(x) ; i++ )
    for ( j = 0 ; j < dym_cols(x) ; j++ )
      sum += dym_ref(x,i,j);
  return sum;
}

bool dym_is_ill_defined(dym *x)
{
  return is_ill_defined(dym_sum(x));
}

void free_dym(dym *d)
{
  int i;
  check_dym_code(d,"free_dym");
#ifndef AMFAST
  if ( dym_is_ill_defined(d) ) 
  {
    int j;
    bool done = FALSE;
    for (i=0;(i<dym_rows(d))&&!done;i++)
      for (j=0;(j<dym_cols(d))&&!done;j++)
      {
	if (is_ill_defined(dym_ref(d,i,j)))
	{
	  printf("row %d col %d is ill defined\n",i,j);
	  done = TRUE;
	}
      }
    my_error("free_dym: dym contained NaN or Inf");
  }
#endif
  

  for ( i = 0 ; i < d -> rows ; i++ )
  {
    am_free_realnums(d->tdarr[i],d->cols);
    d->tdarr[i] = NULL;
  }

#ifndef AMFAST
  d->dym_code = FREED_DYM_CODE; /* Useful to warn us if we
                                   accidentally access this dym again,
                                   though will only be checked if not
                                   in AMFAST mode */
#endif

  AM_FREE_ARRAY(d->tdarr,double_ptr,d->rows_allocated);
  AM_FREE(d,dym);

  Dyms_freed += 1;
}

void free_dym_ill_ok(dym *d)
{
  int i;
  check_dym_code(d,"free_dym");

  for ( i = 0 ; i < d -> rows ; i++ )
  {
    am_free_realnums(d->tdarr[i],d->cols);
    d->tdarr[i] = NULL;
  }

  AM_FREE_ARRAY(d->tdarr,double_ptr,d->rows_allocated);
  AM_FREE(d,dym);

  Dyms_freed += 1;
}

void dym_malloc_report(void)
{
  dyv_malloc_report(); /* for backwards compatibility */
  if ( Dyms_mallocked )
  {
    fprintf(stdout,"# Dynamic Matrices (datatype dym) currently allocated:  %d\n",
           Dyms_mallocked - Dyms_freed
          );
    if ( Dyms_mallocked - Dyms_freed != 0 )
    {
      fprintf(stdout,"#       Number of dym allocations since program start:  %d\n",
             Dyms_mallocked
            );
      fprintf(stdout,"#       Number of dym frees       since program start:  %d\n#\n",
             Dyms_freed
            );
    }
  }
}

void add_row(dym *d)
{
  check_dym_code(d,"add_row");
  if ( d->rows_allocated < d->rows )
    my_error("oujbcowlbucv");
  if ( d->rows_allocated == d->rows )
  {
    int new_size = int_max(100,(int) (2.5 * d->rows_allocated));
    double **new_ar = AM_MALLOC_ARRAY(double_ptr,new_size);
    int i;
    for ( i = 0 ; i < new_size ; i++ )
      new_ar[i] = NULL;
    for ( i = 0 ; i < d->rows ; i++ )
      new_ar[i] = d->tdarr[i];

    AM_FREE_ARRAY(d->tdarr,double_ptr,d->rows_allocated);
    d->tdarr = new_ar;
    d->rows_allocated = new_size;
  }
  d->tdarr[d->rows] = am_malloc_realnums(d->cols);
  set_realnums_constant(d->tdarr[d->rows],d->cols,0.0);
  d->rows += 1;
}

void add_rows(dym *d, int num_rows)
{
  int r;
  check_dym_code(d,"add_rows");
  if ( d->rows_allocated < d->rows )
    my_error("dym's rows_allocated is less than dym's rows");
  if ( d->rows_allocated < (d->rows+num_rows) )
  {
    int new_size = int_max(100, (int)(2.5*d->rows+num_rows));
    double **new_ar = AM_MALLOC_ARRAY(double_ptr,new_size);
    int i;
    for ( i = 0 ; i < new_size ; i++ )
      new_ar[i] = NULL;
    for ( i = 0 ; i < d->rows ; i++ )
      new_ar[i] = d->tdarr[i];

    AM_FREE_ARRAY(d->tdarr,double_ptr,d->rows_allocated);
    d->tdarr = new_ar;
    d->rows_allocated = new_size;
  }

  for( r = 0; r < num_rows; r++ )
    {
      d->tdarr[d->rows+r] = am_malloc_realnums(d->cols);
      set_realnums_constant(d->tdarr[d->rows+r],d->cols,0.0);
    }
  d->rows += num_rows;
}

double safe_dym_ref(const dym *d, int i,int j)
{
  check_dym_access(d,i,j,"dym_ref");
  return(d->tdarr[i][j]);
}

void safe_dym_set(dym *d,int i,int j,double value)
{
  check_dym_access(d,i,j,"dym_set");
  d->tdarr[i][j] = value;
}

void safe_dym_increment(dym *d,int i,int j,double value)
{
  check_dym_access(d,i,j,"dym_increment");
  d->tdarr[i][j] += value;
}

void copy_dym_to_tdarr(dym *d,double **tdarr)
{
  copy_2d_realnums(d->tdarr,tdarr,d->rows,d->cols);
}
  
double **mk_tdarr_from_dym(dym *d)
{
  double **result;
  check_dym_code(d,"make_copy_tdarr");
  result = am_malloc_2d_realnums(d->rows,d->cols);
  copy_dym_to_tdarr(d,result);
  return(result);
}

void copy_tdarr_to_dym(double **tdarr,int rows,int cols,dym *r_d)
{
  assert_dym_shape(r_d,rows,cols,"copy_tdarr_to_dym");
  copy_2d_realnums(tdarr,r_d->tdarr,rows,cols);
}
  
dym *mk_dym_from_tdarr(double **tdarr,int rows,int cols)
{
  dym *result = mk_dym(rows,cols);
  copy_tdarr_to_dym(tdarr,rows,cols,result);
  return(result);
}

/*
 * Copying dym rows to dym rows
 * Frank Dellaert, Aug 14 1997
 */
void copy_dym_row_to_dym_row(dym *src,int src_row, dym *dst,int dst_row)
{
  int i, ncols;
  check_dym_code(src,"copy_dym_row_to_dym_row");
  check_dym_code(dst,"copy_dym_row_to_dym_row");
  if ( src_row < 0 || src_row >= src->rows )
    my_error("copy_dym_row_to_dym_row: illegal source row");
  if ( dst_row < 0 || dst_row >= dst->rows )
    my_error("copy_dym_row_to_dym_row: illegal destination row");
  if ( src->cols != dst->cols )
    my_error("copy_dym_row_to_dym_row: different number of columns");
  ncols = src->cols;
  for ( i = 0 ; i < ncols ; i++ )
    dst->tdarr[dst_row][i] = src->tdarr[src_row][i];
}

void copy_dym_col_to_dym_col(dym *src,int src_col, dym *dst,int dst_col)
{
  int i, nrows;
  check_dym_code(src,"copy_dym_row_to_dym_row");
  check_dym_code(dst,"copy_dym_row_to_dym_row");
  if ( src_col < 0 || src_col >= src->cols )
    my_error("copy_dym_col_to_dym_col: illegal source col");
  if ( dst_col < 0 || dst_col >= dst->cols )
    my_error("copy_dym_col_to_dym_col: illegal destination col");
  if ( src->rows != dst->rows )
    my_error("copy_dym_col_to_dym_col: different number of rows");
  nrows = src->rows;
  for ( i = 0 ; i < nrows ; i++ ) 
    dst->tdarr[i][dst_col] = src->tdarr[i][src_col];
}

/***** Copying dyvs to and from rows and columns of dyms. And
       Making dyvs from rows and columns of dyms too. *********/
/* makes a dyv from dym; set row_wise flag to scan rowwise or columnwise */
void copy_dyv_from_dym(dyv *dv, dym *dm, int row_wise)
{
  int row, num_rows = dym_rows(dm);
  int col, num_cols = dym_cols(dm);

  my_assert(dyv_size(dv) == (num_rows*num_cols));
  for ( row = 0; row < num_rows; row++ )
    for ( col = 0; col < num_cols; col++ )
    {
      int index = ( row_wise ) ? (row*num_cols + col) : (col*num_rows + row);
      dyv_set(dv,index,dym_ref(dm,row,col));
    }
}

dyv *mk_dyv_from_dym(dym *dm, int row_wise)
{
  dyv *dv = mk_dyv(dym_rows(dm)*dym_cols(dm));

  copy_dyv_from_dym(dv,dm,row_wise);
  return dv;
}

void copy_dyv_to_dym_row(const dyv *dv,dym *dm,int row)
{
  check_dym_code(dm,"copy_dyv_to_dym_row");
  if ( row < 0 || row >= dm->rows )
    my_error("copy_dyv_to_dym_row: illegal row");
  assert_dyv_shape(dv,dm->cols,"copy_dyv_to_dym_row");
  memcpy(dm->tdarr[row], dv->farr, dm->cols * sizeof (double));
}

void copy_dyv_to_dym_col(const dyv *dv,dym *dm,int col)
{
  int i;
  check_dym_code(dm,"copy_dyv_to_dym_col");
  if ( col < 0 || col >= dm->cols )
    my_error("copy_dyv_to_dym_col: illegal col");
  assert_dyv_shape(dv,dm->rows,"copy_dyv_to_dym_col");
  for ( i = 0 ; i < dm->rows ; i++ )
    dm->tdarr[i][col] = dv->farr[i];
}

void copy_dym_row_to_dyv(const dym *dm,dyv *dv,int row)
{
  check_dym_code(dm,"copy_dym_row_to_dyv");
  if ( row < 0 || row >= dm->rows )
    my_error("copy_dyv_to_dym_row: illegal row");
  assert_dyv_shape(dv,dm->cols,"copy_dyv_to_dym_row");
  memcpy(dv->farr, dm->tdarr[row], dm->cols * sizeof (double));
}

dyv *mk_dyv_from_dym_row(const dym *dm,int row)
{
  dyv *result = mk_dyv(dm->cols);
  copy_dym_row_to_dyv(dm,result,row);
  return(result);
}

void copy_dym_col_to_dyv(const dym *dm, dyv *dv, int col)
{
  int i;
  check_dym_code(dm,"copy_dym_col_to_dyv");
  if ( col < 0 || col >= dm->cols )
    my_error("copy_dym_col_to_dyv: illegal col");
  assert_dyv_shape(dv,dm->rows,"copy_dym_col_to_dyv");
  for ( i = 0 ; i < dm->rows ; i++ )
    dv->farr[i] = dm->tdarr[i][col];
}

dyv *mk_dyv_from_dym_col(const dym *dm, int col)
{
  dyv *result = mk_dyv(dm->rows);
  copy_dym_col_to_dyv(dm,result,col);
  return(result);
}

/***** Copying farrs to and from rows and columns of dyms. And
       Making farrs from rows and columns of dyms too. *********/

void copy_farr_to_dym_row(double *farr,dym *dm,int row)
{
  int i;
  check_dym_code(dm,"copy_farr_to_dym_row");
  if ( row < 0 || row >= dm->rows )
    my_error("copy_farr_to_dym_row: illegal row");
  for ( i = 0 ; i < dm->cols ; i++ )
    dm->tdarr[row][i] = farr[i];
}

void copy_farr_to_dym_col(double *farr,dym *dm,int col)
{
  int i;
  check_dym_code(dm,"copy_farr_to_dym_col");
  if ( col < 0 || col >= dm->cols )
    my_error("copy_farr_to_dym_col: illegal col");
  for ( i = 0 ; i < dm->rows ; i++ )
    dm->tdarr[i][col] = farr[i];
}

void copy_dym_row_to_farr(dym *dm,double *farr,int row)
{
  int i;
  check_dym_code(dm,"copy_dym_row_to_farr");
  if ( row < 0 || row >= dm->rows )
    my_error("copy_farr_to_dym_row: illegal row");
  for ( i = 0 ; i < dm->cols ; i++ )
    farr[i] = dm->tdarr[row][i];
}

double *mk_farr_from_dym_row(dym *dm,int row)
{
  double *result = am_malloc_realnums(dm->cols);
  copy_dym_row_to_farr(dm,result,row);
  return(result);
}

void copy_dym_col_to_farr(dym *dm,double *farr,int col)
{
  int i;
  check_dym_code(dm,"copy_dym_col_to_farr");
  if ( col < 0 || col >= dm->cols )
    my_error("copy_dym_col_to_farr: illegal col");
  for ( i = 0 ; i < dm->rows ; i++ )
    farr[i] = dm->tdarr[i][col];
}

double *mk_farr_from_dym_col(dym *dm,int col)
{
  double *result = am_malloc_realnums(dm->rows);
  copy_dym_col_to_farr(dm,result,col);
  return(result);
}

/**** Making whole dyms from dyvs ******/

dym *mk_col_dym_from_dyv(dyv *dv)
{
  dym *result;
  check_dyv_code(dv,"mk_col_dym_from_dyv");
  result = mk_dym(dv->size,1);
  copy_dyv_to_dym_col(dv,result,0);
  return(result);
}

dym *mk_row_dym_from_dyv(dyv *dv)
{
  dym *result;
  check_dyv_code(dv,"mk_row_dym_from_dyv");
  result = mk_dym(1,dv->size);
  copy_dyv_to_dym_row(dv,result,0);
  return(result);
}

dym *mk_diag_dym_from_dyv(dyv *dv)
{
  dym *result;
  int i;

  check_dyv_code(dv,"mk_row_dym_from_dyv");
  result = mk_dym(dv->size,dv->size);
  zero_dym(result);

  for ( i = 0 ; i < dv->size ; i++ )
    result->tdarr[i][i] = dv->farr[i];

  return(result);
}

void add_dyv_to_dym_row(dym *r_a, dyv *b, int row)
{
  int i;
  check_dyv_code(b,"add_dyv_to_diag");

  for ( i = 0 ; i < dyv_size(b) ; i++ )
    dym_increment(r_a,row,i,dyv_ref(b,i));
}

void add_dyv_to_dym_col(dym *r_a, dyv *b, int col)
{
  int i;
  check_dyv_code(b,"add_dyv_to_diag");

  for ( i = 0 ; i < dyv_size(b) ; i++ )
    dym_increment(r_a,i,col,dyv_ref(b,i));
}

void add_dym_row_to_dyv(dym *a, dyv *b, int row)
{
  int i;
  check_dyv_code(b,"add_dym_row_to_dyv");
  check_dym_code(a,"add_dym_row_to_dyv");
  for (i=0;i<dyv_size(b);i++) dyv_increment(b,i,dym_ref(a,row,i));
}

void add_dym_col_to_dyv(dym *a, dyv *b, int col)
{
  int i;
  check_dyv_code(b,"add_dym_row_to_dyv");
  check_dym_code(a,"add_dym_row_to_dyv");
  for (i=0;i<dyv_size(b);i++) dyv_increment(b,i,dym_ref(a,i,col));
}

void add_dyv_to_diag(dym *r_a,dyv *b)
{
  int i;

  check_dyv_code(b,"add_dyv_to_diag");
  assert_dym_shape(r_a,dyv_size(b),dyv_size(b),"add_dyv_to_diag");

  for ( i = 0 ; i < dyv_size(b) ; i++ )
    dym_increment(r_a,i,i,dyv_ref(b,i));
}

dym *mk_add_dyv_to_diag(dym *a,dyv *b)
{
  dym *result = mk_copy_dym(a);
  add_dyv_to_diag(result,b);
  return(result);
}
  
/**** Making whole dyms from farrs ******/

dym *mk_col_dym_from_farr(double *farr,int farr_size)
{
  dym *result;
  result = mk_dym(farr_size,1);
  copy_farr_to_dym_col(farr,result,0);
  return(result);
}

dym *mk_row_dym_from_farr(double *farr, int farr_size)
{
  dym *result;
  result = mk_dym(1,farr_size);
  copy_farr_to_dym_row(farr,result,0);
  return(result);
}

dym *mk_diag_dym_from_farr(double *farr, int farr_size)
{
  dym *result;
  int i;

  result = mk_dym(farr_size,farr_size);
  zero_dym(result);

  for ( i = 0 ; i < farr_size ; i++ )
    result->tdarr[i][i] = farr[i];

  return(result);
}

void append_dyv_to_dym(dym* dm, dyv* row)
{
  check_dym_code(dm,"append_dyv_to_dym");
  check_dyv_code(row,"append_dyv_to_dym");
  add_row(dm);
  copy_dyv_to_dym_row(row, dm, dym_rows(dm)-1);
}

/* Appends dm2 to dm1 */
void append_dym_to_dym(dym* dm1, dym* dm2)
{
  int r;
  check_dym_code(dm1,"append_dym_to_dym");
  check_dym_code(dm2,"append_dym_to_dym");
  add_rows(dm1,dm2->rows);
  for( r = 0; r < dm2->rows; r++ )
    {
      copy_dym_row_to_dym_row(dm2,r,dm1,dm1->rows-dm2->rows+r);
    }
}


/***** Simple operations on dyms ******/

int dym_rows(const dym *d)
{
  check_dym_code(d,"dym_rows");
  return(d->rows);
}

int dym_cols(const dym *d)
{
  check_dym_code(d,"dym_cols");
  return(d->cols);
}

#ifdef NEVER

dym *old_formatted_dym_from_lex(lex *lx,char *fstring,char fcode,char *fname)
{
  int cols = num_char_occurs(fstring,fcode);
  dym *d = mk_dym(0,cols);
  int i;

  for ( i = 0 ; i < lx -> num_lines ; i++ )
  {
    int line_len = lex_line_length(lx,i);
    if ( line_len > 0 && lex_line_ref(lx,i,0)->type == NUMBER )
    {
      int dym_col = 0;
      int lex_col;
      add_row(d);

      for ( lex_col = 0 ; lex_col < line_len ; lex_col++ )
      {
        if ( fstring[lex_col] == fcode )
        {
          lextok *lxt = lex_line_ref(lx,i,lex_col);
          if ( lxt->type != NUMBER )
          {
            fprintf(stderr,"Syntax error attempting to read a number from\n");
            fprintf(stderr,"file \"%s\". Line %d item %d should be number.\n",
                    fname,i+1,lex_col+1);
            my_error("Non-number in datafile");
          }
          dym_set(d,dym_rows(d)-1,dym_col,lxt->number);
          dym_col += 1;
        }
      }

      if ( dym_col != cols )
      {
        int j;
        fprintf(stderr,"Syntax error attempting to read numbers from\n");
        fprintf(stderr,"file \"%s\". Line %d should contain %d number%s.\n",
                fname,i+1,cols,(cols==1)?"":"s");
        fprintf(stderr,"There should be numbers in following line items:\n");
        for ( j = 0 ; fstring[j] != '\0' ; j++ )
          if ( fstring[j] == fcode ) fprintf(stderr," %d ",j+1);
        fprintf(stderr,"\n");
        my_error("Too short a line in datafile");
      }
    }
  }

  return(d);
}
#endif

#ifdef FANCY
static char Amdm_error_string[1000];

bool basic_read_io_dyms(
    FILE *s,
    char *fname,
    char *format,
    bool last_col_is_output,
    dym **r_in_dym,
    dym **r_out_dym
  )
/*
   Returns TRUE if it succeeds.
   Returns FALSE if some kind of error, in which case the string 
     Amdm_error_string contains an error message. If returns an error,
     then allocates no memory, and all results are NULL.

   last_col_is_output is IGNORED except when format == NULL

   In this function format may be NULL or else a null-terminated string.

   If format is NULL and last_col_is_output is TRUE then you can treat
   the specification as though the function were called with format = iii..iio
   where the number of i's is one less than the number of numbers on the first
   numerical line of the file.
  
   If format is NULL and last_col_is_output is FALSE then you can treat
   the specification as though the function were called with format = iii..iii
   where the number of i's is the number of numbers on the first
   numerical line of the file.
  
   Let N be the number of characters in format.

   Then we read the file, assuming that every line which starts with a number
   as its first lexical item contains exactly N lexical items all numbers.
   Otherwise we'll signal a syntax error.

   What do we do with these numbers?

   The number on the i'th numerical row of the file, in the j'th numeric
   colum is either ignored, stored in *r_in_dym or stored in *r_out_dym.

   It is stored in *r_in_dym[i,k] if format[j] is the k'th 'i' character in
   format.

   It is stored in *r_out_dym[i,k] if format[j] is the k'th 'o' character in
   format.

   If format contains no i's , no dym is created, and *r_in_dym is set to NULL
   If format contains no o's , no dym is created, and *r_out_dym is set to NULL

    EXAMPLE:
    FILE STARTS HERE:
    .7 6 -9 2.1
    # Line starts with non-numeric so ignored
    Actually, this ignored too
    -1 3 4 3
    
If called with format = ii-o would produce

    *r_in_dym = [  0.7  6.0 ]    *r_out_dym = [ 2.1 ]
                [ -1.0  3.0 ]                 [ 3.0 ]

If called with format = --i- would produce

    *r_in_dym = [  -9.0 ]    *r_out_dym = NULL
                [   4.0 ]                 

If called with format = o-io would produce

    *r_in_dym = [  -9.0 ]    *r_out_dym = [  0.7 2.1 ]
                [   4.0 ]                 [ -1.0 3.0 ]

If called with format = iiio would produce

    *r_in_dym = [  0.7 6.0 -9.0 ]    *r_out_dym = [ 2.1 ]
                [ -1.0 3.0  4.0 ]                 [ 3.0 ]

If called with format = NULL and last_col_is_output == TRUE would also produce

    *r_in_dym = [  0.7 6.0 -9.0 ]    *r_out_dym = [ 2.1 ]
                [ -1.0 3.0  4.0 ]                 [ 3.0 ]

If called with format = iiii would produce

    *r_in_dym = [  0.7 6.0 -9.0 2.1 ]    *r_out_dym = NULL
                [ -1.0 3.0  4.0 3.0 ]                 

If called with format = NULL and last_col_is_output == FALSE would also produce

    *r_in_dym = [  0.7 6.0 -9.0 2.1 ]    *r_out_dym = NULL
                [ -1.0 3.0  4.0 3.0 ]                 


If called with format = o-ioo would produce ERROR because numeric lines
don't contain 5 numbers.

*/
{
  lex lx[1];
  bool file_ended = FALSE;
  dym *din = NULL;
  dym *dout = NULL;
  int line_number = 0;
  int din_cols = (format==NULL) ? 0 : num_char_occurs(format,'i');
  int dout_cols = (format==NULL) ? 0 : num_char_occurs(format,'o');
  int total_cols = (format==NULL) ? 0 : strlen(format);
  bool ok = TRUE;

  sprintf(Amdm_error_string,"Everything's okay");

  while ( ok && !file_ended )
  {
    file_ended = !lex_can_read_line(s,lx);
    line_number += 1;

    if ( !file_ended )
    {
      int line_len = lex_line_length(lx,0);

      if ( (line_number % 1000)==0 ) basic_message(fname,line_number);
 
      if ( line_len > 0 && lex_line_ref(lx,0,0)->type == NUMBER )
      {
        int din_col = 0;
        int dout_col = 0;
        int lex_col;

        if ( format == NULL )
        {
          dout_cols = (last_col_is_output) ? 1 : 0;
          din_cols = line_len - dout_cols;
          total_cols = din_cols + dout_cols;
        }

        if ( din == NULL && din_cols > 0 ) din = mk_dym(0,din_cols);
        if ( dout == NULL && dout_cols > 0 ) dout = mk_dym(0,dout_cols);

        if ( din_cols > 0 ) add_row(din);
        if ( dout_cols > 0 ) add_row(dout);

        for ( lex_col = 0 ; ok && lex_col < int_min(line_len,total_cols) ;
              lex_col++ 
            )
        {
          bool is_input,is_output;

          if ( format != NULL )
            is_input = format[lex_col] == 'i';
          else
            is_input = lex_col < din_cols;

          if ( format != NULL )
            is_output = format[lex_col] == 'o';
          else
            is_output = lex_col == din_cols;

          if ( is_input || is_output )
          {
            lextok *lxt = lex_line_ref(lx,0,lex_col);
            if ( lxt->type != NUMBER )
            {
              ok = FALSE;
              sprintf(Amdm_error_string,
                      "Line %d item %d of file %s unexpected non-number",
                      line_number,lex_col+1,fname
                     );
            }
            else if ( is_input )
            {
              dym_set(din,dym_rows(din)-1,din_col,lxt->number);
              din_col += 1;
            }
            else
            {
              dym_set(dout,dym_rows(dout)-1,dout_col,lxt->number);
              dout_col += 1;
            }
          }
        }

        if ( ok && line_len != total_cols )
        {
          sprintf(Amdm_error_string,
                  "Line %d of file %s should have %d item(s). But it has %d",
                   line_number,fname,total_cols,line_len
                 );
          ok = FALSE;
        }
      }
    }
    lex_free_contents(lx);
  }

  if ( ok && din == NULL && dout == NULL )
  {
    if ( format == NULL )
    {
      sprintf(Amdm_error_string,
              "File %s has no lines with numbers, and there's no format given",
              fname
             );
      ok = FALSE;
    }
    else
    {
      if ( din_cols > 0 ) din = mk_dym(0,din_cols);
      if ( dout_cols > 0 ) dout = mk_dym(0,dout_cols);
    }
  }

  if ( ok && line_number > 1000 ) basic_message(fname,line_number);

  if ( ok )
  {
    *r_in_dym = din;
    *r_out_dym = dout;
  }
  else
  {
    if ( din != NULL ) free_dym(din);
    if ( dout != NULL ) free_dym(dout);
    *r_in_dym = NULL;
    *r_out_dym = NULL;
  }

  return(ok);
}

/* JS 9-7-95 
 * read_io_dyms is changed to allow MATLAB to call our software and pass
 * its matrices in directly.  The new parameter list is the same except
 * for the elimination of the file pointer.  If MATLAB did call this execution
 * and it passed in a matrix directly, fname will now contain 8 characters
 * to indicate it, the next set of 4 characters will be the integer number of
 * matrix rows, the next 4 will be the integer number of colums, and the next
 * 4 will be a double pointer to the values.  The old form of read_io_dyms 
 * exists as read_io_dyms_stream.  The new one should be used unless the caller
 * needs to pre-open the file and re-position the pointer in it before reaching
 * these routines.
 */
/*
   Returns TRUE if it succeeds.
   Returns FALSE if some kind of error, in which case the string 
     Amdm_error_string contains an error message. If returns an error,
     then allocates no memory, and all results are NULL.
*/
static int MATLAB;
bool can_read_io_dyms(char *fname,char *format,dym **r_in_dym,dym **r_out_dym)
{
  int i,ocols,icols,mcols,mrows,icount,ocount;
  double *vals;
  char *lformat;
  bool ok = TRUE;

  if (MATLAB&&(!strcmp(fname,"MATLAB"))&&(fname[7]==0x15))
  {
    mrows = *((int *)(fname+8));
    mcols = *((int *)(fname+12));
    vals = *((double **)(fname+16));
    if (format) 
    {
      if ((int) strlen(format) != mcols) 
        my_error("read_io_dyms: format length does not match matrix size");
      lformat = format;
    }
    else
    {
      lformat = AM_MALLOC_ARRAY(char,mcols+1);
      for (i=0;i<mcols-1;i++) lformat[i] = 'i';
      lformat[mcols-1] = 'o';
      lformat[mcols] = '\0';
    }
    for (i=0,icols=0,ocols=0;i< (int) strlen(lformat);i++)
    {
      if (lformat[i] == 'i') icols++;
      if (lformat[i] == 'o') ocols++;
    }
    if (icols) (*r_in_dym) = mk_dym(mrows,icols);
    else       (*r_in_dym) = NULL;
    if (ocols) (*r_out_dym) = mk_dym(mrows,ocols);
    else       (*r_out_dym) = NULL;
    for (i=0,icount=0,ocount=0;i< (int) strlen(lformat);i++)
    {
      if (lformat[i]=='i')
        copy_farr_to_dym_col(vals+(i*mrows),*r_in_dym,icount++);
      if (lformat[i]=='o')
        copy_farr_to_dym_col(vals+(i*mrows),*r_out_dym,ocount++);
    }
    if (!format) AM_FREE_ARRAY(lformat,char,mcols+1);
  }
  else
  {
    FILE *s = fopen(fname,"r");
    if ( s == NULL )
    {
      sprintf(Amdm_error_string,"Dataset file %s doesn't exist",fname);
      ok = FALSE;
      *r_in_dym = NULL;
      *r_out_dym = NULL;
    }
    else
      ok = read_io_dyms_stream(s,fname,format,r_in_dym,r_out_dym);
  }
  return(ok);
}

/* Just the same as read_io_dyms, except halts program if an error */
void read_io_dyms(char *fname,char *format,dym **r_in_dym,dym **r_out_dym)
{
  bool ok = can_read_io_dyms(fname,format,r_in_dym,r_out_dym);
  if ( !ok )
  {
    fprintf(stderr,"Error reading in dym(s):\n");
    my_error(Amdm_error_string);
  }
}

/*
bool read_io_dyms_stream(
    FILE *s,
    char *fname,
    char *format,
    dym **r_in_dym,
    dym **r_out_dym
  )

   Returns TRUE if it succeeds.
   Returns FALSE if some kind of error, in which case the string 
     Amdm_error_string contains an error message. If returns an error,
     then allocates no memory, and all results are NULL.

   In this function format may be NULL or else a null-terminated string.

   If format is NULL  then you can treat
   the specification as though the function were called with format = iii..iio
   where the number of i's is one less than the number of numbers on the first
   numerical line of the file.
  
   Let N be the number of characters in format.

   Then we read the file, assuming that every line which starts with a number
   as its first lexical item contains exactly N lexical items all numbers.
   Otherwise we'll signal a syntax error.

   What do we do with these numbers?

   The number on the i'th numerical row of the file, in the j'th numeric
   colum is either ignored, stored in *r_in_dym or stored in *r_out_dym.

   It is stored in *r_in_dym[i,k] if format[j] is the k'th 'i' character in
   format.

   It is stored in *r_out_dym[i,k] if format[j] is the k'th 'o' character in
   format.

   *The following is different from basic_read_io_dyms*

   If format contains no o's and no i's, *r_in_dym and *r_out_dym are
   both set to dyms of size 0x0.

   If format contains o's but no i's, *r_in_dym is set to a matrix with
   0 columns, but the same number of rows as *r_out_dym

   If format contains i's but no o's, *r_out_dym is set to a matrix with
   0 columns, but the same number of rows as *r_in_dym

    EXAMPLE:
    FILE STARTS HERE:
    .7 6 -9 2.1
    # Line starts with non-numeric so ignored
    Actually, this ignored too
    -1 3 4 3
    
If called with format = ii-o would produce

    *r_in_dym = [  0.7  6.0 ]    *r_out_dym = [ 2.1 ]
                [ -1.0  3.0 ]                 [ 3.0 ]

If called with format = --i- would produce

    *r_in_dym = [  -9.0 ]    *r_out_dym = [ ]
                [   4.0 ]                 [ ]

If called with format = o-io would produce

    *r_in_dym = [  -9.0 ]    *r_out_dym = [  0.7 2.1 ]
                [   4.0 ]                 [ -1.0 3.0 ]

If called with format = iiio would produce

    *r_in_dym = [  0.7 6.0 -9.0 ]    *r_out_dym = [ 2.1 ]
                [ -1.0 3.0  4.0 ]                 [ 3.0 ]

If called with format = NULL would also produce

    *r_in_dym = [  0.7 6.0 -9.0 ]    *r_out_dym = [ 2.1 ]
                [ -1.0 3.0  4.0 ]                 [ 3.0 ]

If called with format = o-ioo would produce ERROR because numeric lines
don't contain 5 numbers.

*/
bool read_io_dyms_stream(
    FILE *s,
    char *fname,
    char *format,
    dym **r_in_dym,
    dym **r_out_dym
  )
{
  bool last_col_is_output = TRUE;
  bool ok = 
    basic_read_io_dyms(s,fname,format,last_col_is_output,r_in_dym,r_out_dym);

  if ( ok && *r_in_dym == NULL && *r_out_dym == NULL )
  {
    *r_in_dym = mk_dym(0,0);
    *r_out_dym = mk_dym(0,0);
  }
  else if ( ok && *r_out_dym == NULL )
    *r_out_dym = mk_dym(dym_rows(*r_in_dym),0);
  else if ( ok && *r_in_dym == NULL )
    *r_in_dym = mk_dym(dym_rows(*r_out_dym),0);

  return(ok);
}

dym *read_dym(FILE *s,char *fname,char *format)
/*
   In this function format may be NULL or else a null-terminated string.

   If format is NULL then you can treat
   the specification as though the function were called with format = iii..iii
   where the number of i's is the number of numbers on the first
   numerical line of the file.
  
   Let N be the number of characters in format.

   Then we read the file, assuming that every line which starts with a number
   as its first lexical item contains exactly N lexical items all numbers.
   Otherwise we'll signal a syntax error.

   What do we do with these numbers?

   The number on the i'th numerical row of the file, in the j'th numeric
   colum is either ignored, stored in dym *result (the dym we make and return)

   It is stored in result[i,k] if format[j] is the k'th 'i' character in
   format.

   If format contains no i's , no dym is created, and *r_in_dym is set to NULL

    EXAMPLE:
    FILE STARTS HERE:
    .7 6 -9 2.1
    # Line starts with non-numeric so ignored
    Actually, this ignored too
    -1 3 4 3
    
If called with format = ii-- would produce

       result = [  0.7  6.0 ]
                [ -1.0  3.0 ]

If called with format = --i- would produce

       result = [  -9.0 ]    
                [   4.0 ]                 

If called with format = --i- would produce

       result = [  -9.0 ]
                [   4.0 ]

If called with format = iii- would produce

       result = [  0.7 6.0 -9.0 ]
                [ -1.0 3.0  4.0 ]

If called with format = iiii would produce

       result = [  0.7 6.0 -9.0 2.1 ]
                [ -1.0 3.0  4.0 3.0 ]                 

If called with format = NULL  would also produce

       result = [  0.7 6.0 -9.0 2.1 ]
                [ -1.0 3.0  4.0 3.0 ]                 


If called with format = o-ioo would produce ERROR because numeric lines
don't contain 5 numbers.

*/
{
  int i,mrows,mcols,icols,icount;
  double *vals;
  dym *din,*dout;
  bool last_col_is_output = FALSE;
  bool ok;
  char *lformat;

  if (MATLAB&&(!strcmp(fname,"MATLAB"))&&(fname[7]==0x15))
  {
    mrows = *((int *)(fname+8));
    mcols = *((int *)(fname+12));
    vals = *((double **)(fname+16));
    if (format) 
    {
      if ((int) strlen(format) != mcols) 
        my_error("read_io_dyms: format length does not match matrix size");
      lformat = format;
    }
    else
    {
      lformat = AM_MALLOC_ARRAY(char,mcols+1);
      for (i=0;i<mcols;i++) lformat[i] = 'i';
      lformat[mcols] = '\0';
    }
    for (i=0,icols=0;i< (int) strlen(lformat);i++)
      if (lformat[i] == 'i') icols++;
    if (icols) din = mk_dym(mrows,icols);
    else       din = NULL;
    for (i=0,icount=0;i< (int) strlen(lformat);i++)
      if (lformat[i]=='i')
        copy_farr_to_dym_col(vals+(i*mrows),din,icount++);
    if (!format) AM_FREE_ARRAY(lformat,char,mcols+1);
  }
  else
  {
    ok = basic_read_io_dyms(s,fname,format,last_col_is_output,&din,&dout);  
    if ( !ok )
    {
      fprintf(stderr,"read_dym: Error reading in dym:\n");
      my_error(Amdm_error_string);
    }
    
    if ( dout != NULL ) free_dym(dout);
    /* Need to do this if someone gave us a format with 'o's in it. We
       are meant to ignore outputs in this function.
       */
  }
  return(din);
}
#endif

void save_dym(FILE *s,dym *d)
{
  int i,j;
  for ( i = 0 ; i < dym_rows(d) ; i++ )
    for ( j = 0 ; j < dym_cols(d) ; j++ )
      fprintf(s,"%12g%s",dym_ref(d,i,j),
              (j==dym_cols(d)-1) ? "\n" : " "
             );
}

void save_io_dyms(FILE *s,dym *ins,dym *outs)
{
  int i,j;
  if ( dym_rows(ins) != dym_rows(outs) )
    my_error("owslbcdoacpbifb");

  for ( i = 0 ; i < dym_rows(ins) ; i++ )
  {
    for ( j = 0 ; j < dym_cols(ins) ; j++ )
      fprintf(s,"%12g%s",dym_ref(ins,i,j),
              (j==dym_cols(ins)-1) ? "    " : " "
             );
    for ( j = 0 ; j < dym_cols(outs) ; j++ )
      fprintf(s,"%12g%s",dym_ref(outs,i,j),
              (j==dym_cols(outs)-1) ? "\n" : " "
             );
  }
}

#define DYM_SVD_NULL_THRESH (1e-18)

/* NOTE:  ***SEE DYM NOTES IN AMDYM.H ***** */

void constant_dym(dym *r_d,double v)
{
  int i,j;
  check_dym_code(r_d,"constant_dym");
  for ( i = 0 ; i < r_d -> rows ; i++ )
    for ( j = 0 ; j < r_d -> cols ; j++ )
      r_d->tdarr[i][j] = v;
}

void zero_dym(dym *r_d)
{
  
  check_dym_code(r_d,"zero_dym");
  constant_dym(r_d,0.0);
}

dym *mk_constant_dym(int rows,int cols,double v)
{
  dym *result = mk_dym(rows,cols);
  constant_dym(result,v);
  return(result);
}

dym *mk_zero_dym(int rows,int cols)
{
  dym *result = mk_dym(rows,cols);
  zero_dym(result);
  return(result);
}

/**** Standard operations on dyms ****/

void dym_scalar_mult(const dym *d, double alpha, dym *r_d)
{
  int i,j;
  assert_dym_shape(r_d,d->rows,d->cols,"dym_scalar_mult");
  for ( i = 0 ; i < r_d -> rows ; i++ )
    for ( j = 0 ; j < r_d -> cols ; j++ )
      r_d -> tdarr[i][j] = d->tdarr[i][j] * alpha;
}

dym *mk_dym_scalar_mult(const dym *d,double alpha)
{
  dym *result;
  check_dym_code(d,"mk_dym_scalar_mult");
  result = mk_dym(d->rows,d->cols);
  dym_scalar_mult(d,alpha,result);
  return(result);
}

void dym_scalar_add(dym *d, double alpha, dym *r_d)
{
  int i,j;
  assert_dym_shape(r_d,d->rows,d->cols,"dym_scalar_add");
  for ( i = 0 ; i < r_d -> rows ; i++ )
    for ( j = 0 ; j < r_d -> cols ; j++ )
      r_d -> tdarr[i][j] = d->tdarr[i][j] + alpha;
}

dym *mk_dym_scalar_add(dym *d,double alpha)
{
  dym *result;
  check_dym_code(d,"mk_dym_scalar_add");
  result = mk_dym(d->rows,d->cols);
  dym_scalar_add(d,alpha,result);
  return(result);
}

void copy_dym(dym *d, dym *r_d)
{
  assert_dym_shape(r_d,d->rows,d->cols,"copy_dym");
  dym_scalar_mult(d,1.0,r_d);
}
    
dym *mk_copy_dym(const dym *d)
{
  check_dym_code(d,"mk_copy_dym");
  return(mk_dym_scalar_mult(d,1.0));
}
    
void dym_plus(const dym *d_1, const dym *d_2, dym *r_d)
{
  int i,j;
  if ( d_1 -> rows != d_2 -> rows ||
       d_1 -> cols != d_2 -> cols 
     )
  {
    fprintf_dym(stderr,"d_1",d_1,"\n");
    fprintf_dym(stderr,"d_2",d_2,"\n");
    my_error("dym_plus: dyms (DYnamic Matrices) different shape");
  }

  assert_dym_shape(r_d,d_1->rows,d_1->cols,"dym_plus");
  for ( i = 0 ; i < r_d -> rows ; i++ )
    for ( j = 0 ; j < r_d -> cols ; j++ )
      r_d -> tdarr[i][j] = d_1->tdarr[i][j] + d_2 -> tdarr[i][j];
}

void dym_minus(const dym *d_1, const dym *d_2, dym *r_d)
{
  int i,j;
  if ( d_1 -> rows != d_2 -> rows ||
       d_1 -> cols != d_2 -> cols 
     )
  {
    fprintf_dym(stderr,"d_1",d_1,"\n");
    fprintf_dym(stderr,"d_2",d_2,"\n");
    my_error("dym_plus: dyms (DYnamic Matrices) different shape");
  }

  assert_dym_shape(r_d,d_1->rows,d_1->cols,"dym_plus");
  for ( i = 0 ; i < r_d -> rows ; i++ )
    for ( j = 0 ; j < r_d -> cols ; j++ )
      r_d -> tdarr[i][j] = d_1->tdarr[i][j] - d_2 -> tdarr[i][j];
}

dym *mk_dym_plus(const dym *a,const dym *b)
{
  dym *result = mk_dym(a->rows,a->cols);
  dym_plus(a,b,result);
  return(result);
}

void dym_subtract(const dym *d_1,const dym *d_2,dym *r_d)
{
  dym *a = mk_dym_scalar_mult(d_2,-1.0);
  dym_plus(d_1,a,r_d);
  free_dym(a);
}

dym *mk_dym_subtract(const dym *a,const dym *b)
{
  dym *result = mk_dym(a->rows,a->cols);
  dym_subtract(a,b,result);
  return(result);
}

void dym_times_dyv(const dym *a, const dyv *b, dyv *result)
{
  int i;
  dyv *temp = mk_dyv(a->rows); 
             /* We need a copy in case b and result share memory */

  if ( a->cols != b -> size )
    my_error("dym_times_dyv: sizes wrong");
  assert_dyv_shape(result,a->rows,"dym_times_dyv");

  for ( i = 0 ; i < a->rows ; i++ )
  {
    double sum = 0.0;
    int j;
    for ( j = 0 ; j < a->cols ; j++ )
      sum += dym_ref( a, i, j) * dyv_ref( b, j);
    dyv_set( temp, i, sum);
  }

  copy_dyv(temp,result);
  free_dyv(temp);
}

dyv *mk_dym_times_dyv(const dym *a, const dyv *b)
{
  dyv *result = mk_dyv(a->rows);
  dym_times_dyv(a,b,result);
  return(result);
}

void dyv_outer_product(dyv *a, dyv *b, dym *r_d)
{
  int i,j;
  assert_dym_shape(r_d,a->size,b->size,"dyv_outer_product");
  for (i=0;i<a->size;i++)
    for (j=0;j<b->size;j++)
      dym_set(r_d,i,j,dyv_ref(a,i)*dyv_ref(b,j));
}

dym *mk_dyv_outer_product(dyv *a, dyv *b)
{
  dym *result = mk_dym(a->size,b->size);
  dyv_outer_product(a,b,result);
  return result;
}

void dym_mult(dym *d_1, dym *d_2, dym *r_d)
{
  dym *a = mk_dym_mult(d_1,d_2);
             /* Note we have to first do the multiplying to the result
                a, in case the routine was called with d_1's memory
                = r_d's memory or d_2's memory = r_d's memory */
  assert_dym_shape(r_d,d_1 -> rows,d_2 -> cols,"dym_mult");

  copy_dym(a,r_d);
  free_dym(a);
}

dym *mk_dym_mult(const dym *a, const dym *b)
{
  int nrows = dym_rows(a), ncols = dym_cols(b);
  int acols = dym_cols(a);
  dym *c = mk_dym(nrows,ncols);
  int i,j;

  if ( acols != b->rows )
  {
    fprintf_dym(stderr,"a",a,"\n");
    fprintf_dym(stderr,"b",b,"\n");
    my_error("dym_mult: dyms (DYnamic Matrices) wrong shape\n");
  }

  for ( i = 0 ; i < nrows ; i++ )
    for ( j = 0 ; j < ncols ; j++ )
    {
      double sum = 0.0;
      int k;

      for ( k = 0 ; k < acols ; k++ )
        sum += a->tdarr[i][k] * b->tdarr[k][j];

      c->tdarr[i][j] = sum;
    }

  return c;
}

void dym_transpose( const dym *d, dym *r_d)
{
  dym *a = mk_dym(d->cols,d->rows);
             /* Note we have to first do the transpose to the result
                a, in case the routine was called with d's memory
                = r_d's memory */
  int i,j;

  assert_dym_shape(r_d,d->cols,d->rows,"dym_transpose");

  for ( i = 0 ; i < d -> rows ; i++ )
    for ( j = 0 ; j < d -> cols ; j++ )
      a->tdarr[j][i] = d->tdarr[i][j];

  copy_dym(a,r_d);
  free_dym(a);
}

dym *mk_dym_transpose( const dym *a)
{
  dym *result = mk_dym(a->cols,a->rows);
  dym_transpose(a,result);
  return(result);
}

  
void dym_scale_rows(dym *d,dyv *w_diag,dym *r_d)
/*
    Diag(w_diag) * d is copied to r_d
*/
{
  int i,j;
  assert_dym_shape(r_d,d->rows,d->cols,"dym_scale_rows::r_d");
  assert_dyv_shape(w_diag,d->rows,"dym_scale_rows::w_diag");
  for ( i = 0 ; i < d->rows ; i++ )
    for ( j = 0 ; j < d->cols ; j++ )
      r_d->tdarr[i][j] = d->tdarr[i][j] * w_diag->farr[i];
}

dym *mk_dym_scale_rows(dym *d,dyv *w_diag)
/*
    Returns Diag(w_diag) * d
*/
{
  dym *result = mk_dym(d->rows,d->cols);
  assert_dyv_shape(w_diag,d->rows,"mk_dym_scale_rows::w_diag");
  dym_scale_rows(d,w_diag,result);
  return(result);
}

void dym_normalize_rows(dym *d,dym *r_d)
{
  int i,j;
  assert_dym_shape(r_d,d->rows,d->cols,"dym_scale_rows::r_d");
  for ( i = 0 ; i < d->rows ; i++ ) {
    double norm = dym_sum_row(d,i);
    for ( j = 0 ; j < d->cols ; j++ )
      r_d->tdarr[i][j] = d->tdarr[i][j] / norm;
  }
}

dym *mk_dym_normalize_rows(dym *d)
{
  dym *result = mk_dym(d->rows,d->cols);
  dym_normalize_rows(d,result);
  return(result);
}


void dym_scale_cols(dym *d,dyv *w_diag,dym *r_d)
/*
    d * Diag(w_diag) is copied to r_d
*/
{
  int i,j;
  assert_dym_shape(r_d,d->rows,d->cols,"dym_scale_cols::r_d");
  assert_dyv_shape(w_diag,d->cols,"dym_scale_cols::w_diag");
  for ( i = 0 ; i < d->rows ; i++ )
    for ( j = 0 ; j < d->cols ; j++ )
      r_d->tdarr[i][j] = d->tdarr[i][j] * w_diag->farr[j];
}

dym *mk_dym_scale_cols(dym *d,dyv *w_diag)
/*
    Returns d * Diag(w_diag)
*/
{
  dym *result = mk_dym(d->rows,d->cols);
  assert_dyv_shape(w_diag,d->cols,"mk_dym_scale_cols::w_diag");
  dym_scale_cols(d,w_diag,result);
  return(result);
}

void copy_dym_to_farr(dym *d,double *farr)
/* Copies either a column dym or a 1-row dym to an array of doubles.
   It is an error to call with neither rows nor columns equal to 1
*/
{
  if ( d->rows != 1 && d->cols != 1 )
  {
    fprintf_dym(stderr,"d",d,"\n");
    my_error("dym_to_realnums(): should be 1-columns or 1-row");
  }
  else if ( d->rows == 1 )
    copy_realnums(d->tdarr[0],farr,d->cols);
  else
  {
    int i;
    for ( i = 0 ; i < d->rows ; i++ )
      farr[i] = d->tdarr[i][0];
  }
}

bool is_same_realnum(double x,double y)
{
  double eps = 1e-3;

  bool result = TRUE;

  if ( fabs(x) < eps && fabs(y) < eps )
    result = TRUE;
  else if ( fabs(x) < eps && fabs(y) > 2.0 * eps )
    result = FALSE;
  else if ( fabs(y) < eps && fabs(x) > 2.0 * eps  )
    result = FALSE;
  else if ( x < 0.0 && y > 0.0 )
    result = FALSE;
  else if ( y < 0.0 && x > 0.0 )
    result = FALSE;
  else if ( fabs(x) > fabs(y) )
    result = fabs(x) < (1.0 + eps) * fabs(y);
  else
    result = fabs(y) < (1.0 + eps) * fabs(x);

  return(result);
}

bool is_dym_symmetric(const dym *d)
{
  int i,j;
  bool result = TRUE;

  if ( d->rows != d->cols )
  {
    fprintf_dym(stderr,"d = ",d,"\n");
    my_error("dym.c is_dym_symmetric(d). Requires a square dym.\n");
  }

  for ( i = 0 ; result && i < d->rows ; i++ )
    for ( j = 0 ; result && j < i ; j++ )
    {
      result = is_same_realnum(d->tdarr[i][j],d->tdarr[j][i]);
      if ( Verbosity > 70.0 )
        printf("i = %d, j = %d, result = %d\n",i,j,result);
    }

  if ( !result ) printf("Not Symmetric\n");

  return(result);
}

void enforce_dym_symmetry(dym *d)
/*
   Let d' = matrix after execution
   Let d = matrix before execution

   d' = 0.5 * ( d + d^T )

   so d'[i][j] = 0.5 * (d[i][j] + d[j][i])
*/
{
  int i,j;
  if ( dym_rows(d) != dym_cols(d) )
  {
    fprintf_dym(stderr,"d",d,"\n");
    my_error("enforce_dym_symmetry(): the bove dym is not square");
  }

  for ( i = 0 ; i < d->rows ; i++ )
    for ( j = 0 ; j < i ; j++ )
      dym_set(d,i,j,(dym_ref(d,i,j) + dym_ref(d,j,i))/2.0);

  for ( i = 0 ; i < d->rows ; i++ )
    for ( j = i+1 ; j < d->rows ; j++ )
      dym_set(d,i,j,dym_ref(d,j,i));
}


/******* printing dyms (DYnamic Matrices) *********/

void fprintf_formatted_dym(FILE *s, dym *d, int places, int dec_places)
{
  int i,j;
  char format[8];

  if (places<0 || places>100 || dec_places <0 || dec_places>100)
    my_error("fprintf_formatted_dym: bad places or dec_places");
  sprintf(format,"%%%d.%df ",places,dec_places);
  for (i=0;i<dym_rows(d);i++)
  {
    for (j=0;j<dym_cols(d);j++)
      fprintf(s,format,dym_ref(d,i,j));
    fprintf(s,"\n");
  }
}

void fprintf_dym(FILE *s, const char *m1, const dym *d, const char *m2)
{
  if ( d == NULL )
    fprintf(s,"%s = (dym *)NULL%s",m1,m2);
  else if ( d->dym_code != DYM_CODE )
  {
    fprintf(stderr,"fprintf_dym(s,\"%s\",d,\"\\n\"\n",m1);
    my_error("fprintf_dym called with a non-allocated dym (DYnamic Matrix)");
  }
  else if ( d->rows <= 0 || d->cols <= 0 )
    fprintf(s,"%s = <Dym with %d row%s and %d column%s>%s",
            m1,d->rows,(d->rows==-1)?"":"s",
            d->cols,(d->cols==-1)?"":"s",m2
           );
  else
  {
    int i;
    buftab bt;

    init_buftab(&bt,d->rows,d->cols + 4);

    for ( i = 0 ; i < d->rows ; i++ )
    {
      int j;
      set_buftab(&bt,i,0,(i == (d->rows-1)/2) ? m1 : "");
      set_buftab(&bt,i,1,(i == (d->rows-1)/2) ? "=" : "");
      set_buftab(&bt,i,2,"[");

      for ( j = 0 ; j < d -> cols ; j++ )
      {
        char buff[100];
        sprintf(buff," %g ",d->tdarr[i][j]);
        set_buftab(&bt,i,3+j,buff);
      }

      set_buftab(&bt,i,3+d->cols,"]");
    }

    fprint_buftab(s,&bt);
    free_buftab_contents(&bt);
  }
  fprintf(s,"\n");
}


void pdym(dym *d)
{
  fprintf_dym(stdout,"dym",d,"\n");
}

void fprintf_dym_and_confidence(
    FILE *s,
    char *m1,
    dym *d,
    dym *conf,
    bool huge_uncertainty,
    char *m2
  )
{
  if ( d == NULL )
    fprintf(s,"%s = (dym *)NULL%s",m1,m2);
  else if ( d->rows <= 0 || d->cols <= 0 )
    fprintf(s,"%s = <Dym with %d row%s and %d column%s>%s",
            m1,d->rows,(d->rows==1)?"":"s",
            d->cols,(d->cols==1)?"":"s",m2
           );
  else if ( d->rows != conf->rows || d->cols != conf->cols )
    my_error("fprintf_dym_and_confidence(). d and conf differ in shape");
  else
  {
    int i;
    buftab bt;

    init_buftab(&bt,d->rows,3 * d->cols + 4);

    for ( i = 0 ; i < d->rows ; i++ )
    {
      int j;
      set_buftab(&bt,i,0,(i == (d->rows-1)/2) ? m1 : "");
      set_buftab(&bt,i,1,(i == (d->rows-1)/2) ? "=" : "");
      set_buftab(&bt,i,2,"[");

      for ( j = 0 ; j < d -> cols ; j++ )
      {
        char buff[100];
        sprintf(buff," %g",d->tdarr[i][j]);
        set_buftab(&bt,i,3 + 3 * j,buff);
        set_buftab(&bt,i,4 + 3 * j,"+/-");
        if ( huge_uncertainty )
          sprintf(buff,"huge uncertainty ");
        else
          sprintf(buff,"%g ",conf->tdarr[i][j]);
        set_buftab(&bt,i,5 + 3 * j,buff);
      }

      set_buftab(&bt,i,3 + 3 * d->cols,"]");
    }

    fprint_buftab(s,&bt);
    free_buftab_contents(&bt);
  }
  fprintf(s,"\n");
}

void fprintf_dym_dym(FILE *s,char *m1,dym *d1,char *m2,dym *d2,char *m3)
{
  int maxrows = int_max(dym_rows(d1),dym_rows(d2));
  int crow = int_min(int_min(dym_rows(d1)-1,dym_rows(d2)-1),(maxrows-1)/2);
  buftab bt[1];
  int i;

  init_buftab(bt,maxrows,6 + dym_cols(d1) + dym_cols(d2));

  for ( i = 0 ; i < maxrows ; i++ )
  {
    int j;

    if ( i < dym_rows(d1) )
    {
      set_buftab(bt,i,0,(i == crow) ? m1 : "");
      set_buftab(bt,i,1,"[");

      for ( j = 0 ; j < dym_cols(d1) ; j++ )
      {
        char buff[100];
        sprintf(buff," %g",dym_ref(d1,i,j));
        set_buftab(bt,i,2 + j,buff);
      }

      set_buftab(bt,i,2 + dym_cols(d1),"]");
    }

    if ( i < dym_rows(d2) )
    {
      set_buftab(bt,i,3 + dym_cols(d1),(i == crow) ? m2 : "");
      set_buftab(bt,i,4 + dym_cols(d1),"[");

      for ( j = 0 ; j < dym_cols(d2) ; j++ )
      {
        char buff[100];
        sprintf(buff," %g",dym_ref(d2,i,j));
        set_buftab(bt,i,5 + dym_cols(d1) + j,buff);
      }

      set_buftab(bt,i,5 + dym_cols(d1) + dym_cols(d2),"]");
    }
  }

  fprint_buftab(s,bt);
  free_buftab_contents(bt);
  fprintf(s,"\n");
}

void fprintf_dym_dyv(FILE *s,char *m1,dym *d1,char *m2,dyv *d2,char *m3)
{
  dym *dm = mk_col_dym_from_dyv(d2);
  fprintf_dym_dym(s,m1,d1,m2,dm,m3);
  free_dym(dm);
}


double dym_extreme(dym *dv, bool max)
{ /* FIXME Having 2 seperate loops, one for max, other for !max, would
     probably make this code much much faster */
  int i,j;
  double res = -77.0;
  if ( dym_rows(dv)*dym_cols(dv) < 1 ) my_error("dym_extreme: empty dym");
  for (i=0;i<dv->rows;i++)
    for (j=0;j<dv->cols;j++)
      if ((i == 0 && j == 0) || (max && dym_ref(dv,i,j) > res)
	  || (!max && dym_ref(dv,i,j) < res))
	res = dym_ref(dv,i,j);
  return res;
}

/*makes a dyv corresponding to the min value in each col*/
dyv *mk_min_dyv(dym* d)
{
  dyv* mindyv = mk_dyv(dym_cols(d));
  int col, row;
  if ( dym_rows(d)*dym_cols(d) < 1 ) my_error("dym_extreme: empty dym");
  for (col=0;col<d->cols;col++)
    {
      for (row=0;row<d->rows;row++)
	{
	  if(row==0||dym_ref(d, row, col) < dyv_ref(mindyv, col))
	    {
	      dyv_set(mindyv, col, dym_ref(d, row, col));
	    }
	}
    }
  return mindyv;
}

/*makes a dyv corresponding to the max value in each col*/
dyv *mk_max_dyv(dym* d)
{  
  dyv* maxdyv = mk_dyv(dym_cols(d));
  int col, row;
  if ( dym_rows(d)*dym_cols(d) < 1 ) my_error("dym_extreme: empty dym");
  for (col=0;col<d->cols;col++)
    {
      for (row=0;row<d->rows;row++)
	{
	  if(row==0||dym_ref(d, row, col) > dyv_ref(maxdyv, col))
	    {
	      dyv_set(maxdyv, col, dym_ref(d, row, col));
	    }
	}
    }
  return maxdyv;
}


double dym_min(dym *dv)
{
  return(dym_extreme(dv, FALSE));
}

double dym_max(dym *dv)
{
  return(dym_extreme(dv, TRUE));
}

double dym_row_max(dym *dm,int row)
{
  int j=0;
  double res;
  if(dym_rows(dm)*dym_cols(dm)<1) 
    my_error("dym_row_max: empty dym");
  if(row <0 || row > dym_rows(dm)) 
    my_error("dym_row_max: illegal row number");
  res=dym_ref(dm,row,j);
  for(j=1;j<dym_cols(dm);j++)
  {
    if(dym_ref(dm,row,j) > res)
      res=dym_ref(dm,row,j);
  }
  return res;
}

int dym_row_argmax(dym *dm,int row) {
  int j=0;
  int res = j;
  double best;
  if(dym_rows(dm)*dym_cols(dm)<1) 
    my_error("dym_row_max: empty dym");
  if(row <0 || row > dym_rows(dm)) 
    my_error("dym_row_max: illegal row number");

  best=dym_ref(dm,row,j);
  for(j=1;j<dym_cols(dm);j++) {
    if(dym_ref(dm,row,j) > best) {
      best = dym_ref(dm,row,j);
      res=j;
    }
  }
  return res;
}

double dym_row_min(dym *dm,int row)
{
  int j=0;
  double res;
  if(dym_rows(dm)*dym_cols(dm)<1) 
    my_error("dym_row_min: empty dym");
  if(row <0 || row > dym_rows(dm)) 
    my_error("dym_row_min: illegal row number");
  res=dym_ref(dm,row,j);
  for(j=1;j<dym_cols(dm);j++)
  {
    if(dym_ref(dm,row,j) < res)
      res=dym_ref(dm,row,j);
  }
  return res;
}

int dym_row_argmin(dym *dm,int row) {
  int j=0;
  int res =j;
  double best; 

  if(dym_rows(dm)*dym_cols(dm)<1) 
    my_error("dym_row_min: empty dym");
  if(row <0 || row > dym_rows(dm)) 
    my_error("dym_row_min: illegal row number");
  best=dym_ref(dm,row,j);
  
  for(j=1;j<dym_cols(dm);j++) {
    if(dym_ref(dm,row,j) < best) {
      res = j;
      best=dym_ref(dm,row,j);
    }
  }
  return res;
}

double dym_col_max(dym *dm,int col)
{
  int i=0;
  double res;
  if(dym_rows(dm)*dym_cols(dm)<1) 
    my_error("dym_col_max: empty dym");
  if(col <0 || col > dym_cols(dm)) 
    my_error("dym_col_max: illegal col number");
  res=dym_ref(dm,i,col);
  for(i=1;i<dym_rows(dm);i++)
  {
    if(dym_ref(dm,i,col) > res)
      res=dym_ref(dm,i,col);
  }
  return res;
}

double dym_col_min(dym *dm,int col)
{
  int i=0;
  double res;
  if(dym_rows(dm)*dym_cols(dm)<1) 
    my_error("dym_col_min: empty dym");
  if(col <0 || col > dym_cols(dm)) 
    my_error("dym_col_min: illegal col number");
  res=dym_ref(dm,i,col);
  for(i=1;i<dym_rows(dm);i++)
  {
    if(dym_ref(dm,i,col) < res)
      res=dym_ref(dm,i,col);
  }
  return res;
}


dym *mk_dym_11(double x00)
{
  dym *result = mk_dym(1,1);
  dym_set(result,0,0,x00);
  return(result);
}

dym *mk_dym_12(double x00, double x01)
{
  dym *result = mk_dym(1,2);
  dym_set(result,0,0,x00);
  dym_set(result,0,1,x01);
  return(result);
}

dym *mk_dym_13(double x00, double x01, double x02)
{
  dym *result = mk_dym(1,3);
  dym_set(result,0,0,x00);
  dym_set(result,0,1,x01);
  dym_set(result,0,2,x02);
  return(result);
}

dym *mk_dym_21(double x00,
               double x10)
{
  dym *result = mk_dym(2,1);
  dym_set(result,0,0,x00);
  dym_set(result,1,0,x10);
  return(result);
}

dym *mk_dym_22(double x00, double x01,
               double x10, double x11)
{
  dym *result = mk_dym(2,2);
  dym_set(result,0,0,x00);
  dym_set(result,0,1,x01);
  dym_set(result,1,0,x10);
  dym_set(result,1,1,x11);
  return(result);
}

dym *mk_dym_23(double x00, double x01, double x02,
               double x10, double x11, double x12)
{
  dym *result = mk_dym(2,3);
  dym_set(result,0,0,x00);
  dym_set(result,0,1,x01);
  dym_set(result,0,2,x02);
  dym_set(result,1,0,x10);
  dym_set(result,1,1,x11);
  dym_set(result,1,2,x12);
  return(result);
}

dym *mk_dym_31(double x00,
               double x10,
               double x20)
{
  dym *result = mk_dym(3,1);
  dym_set(result,0,0,x00);
  dym_set(result,1,0,x10);
  dym_set(result,2,0,x20);
  return(result);
}

dym *mk_dym_32(double x00, double x01,
               double x10, double x11,
               double x20, double x21)
{
  dym *result = mk_dym(3,2);
  dym_set(result,0,0,x00);
  dym_set(result,0,1,x01);
  dym_set(result,1,0,x10);
  dym_set(result,1,1,x11);
  dym_set(result,2,0,x20);
  dym_set(result,2,1,x21);
  return(result);
}

dym *mk_dym_33(double x00, double x01, double x02,
               double x10, double x11, double x12,
               double x20, double x21, double x22)
{
  dym *result = mk_dym(3,3);
  dym_set(result,0,0,x00);
  dym_set(result,0,1,x01);
  dym_set(result,0,2,x02);
  dym_set(result,1,0,x10);
  dym_set(result,1,1,x11);
  dym_set(result,1,2,x12);
  dym_set(result,2,0,x20);
  dym_set(result,2,1,x21);
  dym_set(result,2,2,x22);
  return(result);
}

void dym_ptq(dym *p,dym *q,dym *r_dym)
/*
   Sets r_dym to contain p^T q.

   r_dym must have the following shape: dym_cols(p) rows, and dym_cols(q) cols.
   dym_rows(p) must equal dym_rows(q);

   Note. (RQ)[i][j] = sum-over-k-of R[i][k] * Q[k][j]

   Put P^T = R, then (P^T)[i][k] = P[k][i], so

     (P^T)Q[i][j] = sum-over-k-of P[k][i] * Q[k][j]
*/
{
  dym *a = mk_dym(dym_cols(p),dym_cols(q));
             /* Note we have to first do the multiplying to the result
                a, in case the routine was called with d_1's memory
                = r_d's memory or d_2's memory = r_d's memory */

             /* Efficiency: could make this faster by comparing pointers and
                only foing copy if necessary
                Ditto true for multiply, invert, transpose etc
             */
  int i,j;

  if ( dym_rows(p) != dym_rows(q) )
  {
    fprintf_dym(stderr,"p",p,"\n");
    fprintf_dym(stderr,"q",q,"\n");
    my_error("dym_ptq: dyms (DYnamic Matrices) wrong shape\n");
  }

  assert_dym_shape(r_dym,dym_cols(p),dym_cols(q),"dym_ptq");

  for ( i = 0 ; i < dym_rows(a) ; i++ )
    for ( j = 0 ; j < dym_cols(a) ; j++ )
    {
      double sum = 0.0;
      int k;

      for ( k = 0 ; k < dym_rows(p) ; k++ )
        sum += dym_ref(p,k,i) * dym_ref(q,k,j);

      dym_set(a,i,j,sum);
    }

  copy_dym(a,r_dym);
  free_dym(a);
}

dym *mk_dym_ptq(dym *p,dym *q)
/*
   Makes and returns p^T q
*/
{
  dym *result = mk_dym(dym_cols(p),dym_cols(q));
  dym_ptq(p,q,result);
  return(result);
}

/*
   Sets r_dym to contain p^T q p.

   r_dym must have the following shape: dym_cols(p) rows, and dym_cols(q) cols.
   dym_rows(p) must equal dym_rows(q);

   Note. (RQ)[i][j] = sum-over-k-of R[i][k] * Q[k][j]

   Put P^T = R, then (P^T)[i][k] = P[k][i], so

     (P^T)Q[i][j] = sum-over-k-of P[k][i] * Q[k][j]
*/
void dym_ptqp(dym *p,dym *q,dym *r_dym)
{
  dym *a = mk_dym_ptq(p,q);
  dym_mult(a,p,r_dym);
  free_dym(a);
  return;
}

/* Makes and returns p^T q p */
dym *mk_dym_ptqp(dym *p,dym *q)
{
  dym *result = mk_dym(dym_cols(p),dym_cols(p));
  dym_ptqp(p,q,result);
  return(result);
}

/* Returns p^T v in r_dyv.
   PRE: dym_rows(p) == dyv_size(v)
        dyv_cols(p) == dyv_size(r_dyv);
                                                                             
  (Rv)[i] = sum-over-k-of R[i][k] v[k]
  if P^T = R then R[i][k] = P[k][i] so

  (P^T v)[i] = sum-over-k-of P[k][i] v[k]
*/
void dym_transpose_times_dyv(dym *p,dyv *v, dyv *r_dyv)
{
  int v_size, r_size, i, k;
  double factor, val;
  dyv *temp;

  v_size = dyv_size( v);
  r_size = dyv_size( r_dyv);

  temp = mk_zero_dyv( r_size);

  assert_dym_shape( p, v_size, r_size, "dym_transpose_times_dyv_slow");

  for (k=0; k<v_size; ++k) {
    factor = dyv_ref( v, k);
    for (i=0; i<r_size; ++i) {
      val = dym_ref( p, k, i);
      dyv_increment( temp, i, factor * val);
    }
  }

  copy_dyv(temp,r_dyv);
  free_dyv(temp);
  return;
}

#if 0
void dym_transpose_times_dyv_fast(dym *p,dyv *v,dyv *r_dyv)
{
  int v_size, r_size, i, k;
  double val;
  dyv *temp;
  double *v_farr, *temp_farr, *p_row;
  double **p_tdarr;

  v_size = dyv_size( v);
  r_size = dyv_size( r_dyv);

  temp = mk_zero_dyv( r_size);
  temp_farr = temp->farr;

  assert_dym_shape( p, v_size, r_size, "dym_transpose_times_dyv_fast");

  v_farr  = v->farr;
  p_tdarr = p->tdarr;
  for (k=0; k<v_size; ++k) {
    val   = v_farr[k];
    p_row = p_tdarr[k];
    for (i=0; i<r_size; ++i) temp_farr[i] += val * p_row[i];
  }

  copy_dyv(temp,r_dyv);
  free_dyv(temp);
  return;
}
#endif

dyv *mk_dym_transpose_times_dyv(dym *p,dyv *v)
/* Returns p^T v */
{
  dyv *result = mk_dyv(dym_cols(p));
  dym_transpose_times_dyv(p,v,result);
  return(result);
}

dym *mk_identity_dym(int dims)
{
  dyv *dv = mk_constant_dyv(dims,1.0);
  dym *result = mk_diag_dym_from_dyv(dv);
  free_dyv(dv);
  return(result);
}

dym *mk_dym_from_string(char *s)
{
  int state = 0;
  int maxcol = 0;
  int icol = 0;
  int irow = 1;
  int idx,i = 0;
  double val;
  dym *res;

  /* find the size of the matrix */
  while(s[i])
  {
    if ((s[i] == ';') || (s[i] == '\n'))
    {
      irow++;
      if (state == 1) {icol++; state = 0;}
      if (icol > maxcol) maxcol = icol;
      icol = 0;
    }
    else if (am_isspace(s[i]))
    {
      if (state == 1) {icol++; state = 0;}
    }
    else state = 1;
    i++;
  }
  if (state == 1) icol++;
  if (icol > maxcol) maxcol = icol;

  res = mk_dym(irow,maxcol);

  /* fill in the array */
  i = idx = irow = icol = 0;
  while(s[i])
  {
    if ((s[i] == ';') || (s[i] == '\n'))
    {
      if (state == 1)
      {
	sscanf(s+idx,"%lf",&val);
	dym_set(res,irow,icol,val);
      }
      idx = i+1;
      state = 0;
      icol = 0;
      irow++;
    }
    else if (am_isspace(s[i]))
    {
      if (state == 1) 
      {
	sscanf(s+idx,"%lf",&val);
	dym_set(res,irow,icol,val);
	icol++; 
	state = 0;
      }
      idx = i;
    }
    else state = 1;
    i++;
  }
  if (state == 1) 
  {
    sscanf(s+idx,"%lf",&val);
    dym_set(res,irow,icol,val);
  }
    
  return res;
}

dym *mk_dym_from_args(const char *name,int argc,char *argv[],dym *deflt)
/* COPIES in deflt (if so required) */
{
  bool name_there = index_of_arg(name,argc,argv) >= 0;
  dym *result;
  if ( !name_there )
    result = mk_copy_dym(deflt);
  else
  {
    int size = dym_rows(deflt) * dym_cols(deflt);
    dyv *dv = mk_basic_dyv_from_args(name,argc,argv,size);
    int i,j;
    if ( dv == NULL )
    {
      fprintf(stderr,"COMMAND LINE USER ERROR (it's YOUR fault)\n");
      fprintf(stderr,"...when attempting to read a dym identified by\n");
      fprintf(stderr,"the name \"%s\". Perhaps a non-number, or the\n",name);
      fprintf(stderr,"command line finished before all args found?\n");
      fprintf_dym(stderr,"deflt_dym",deflt,"\n");
      my_error("mk_dym_from_args()");
    }

    result = mk_dym(dym_rows(deflt),dym_cols(deflt));
    for ( i = 0 ; i < dym_rows(deflt) ; i++ )
      for ( j = 0 ; j < dym_cols(deflt) ; j++ )
        dym_set(result,i,j,dyv_ref(dv,i * dym_cols(deflt) + j));

    free_dyv(dv);
  }

  return(result);
}

double dym_sum_row(const dym *x, int row)
{
  dyv *dv = mk_dyv_from_dym_row(x,row);
  double result = dyv_sum(dv);
  free_dyv(dv);
  return result;
}

double dym_sum_col(const dym *x, int col)
{
  dyv *dv = mk_dyv_from_dym_col(x, col);
  double result = dyv_sum(dv);
  free_dyv(dv);
  return result;
}

void dym_remove_row(dym *x, int row)
{
  int i;

#ifndef AMFAST
  check_dym_code(x,"dym_remove_row");
  if ((row < 0) || (row >= dym_rows(x))) 
    my_error("dym_remove_row: invalid row");
#endif

  am_free_realnums(x->tdarr[row],x->cols);
  for (i=row;i<x->rows-1;i++) x->tdarr[i] = x->tdarr[i+1];
  x->rows--;
}

static int DymSortCol = 0;

static int dymcompa(const void *a, const void *b)
{
  double *aa = *(double **)a;
  double *bb = *(double **)b;
  if (aa[DymSortCol] > bb[DymSortCol])       return 1;
  else if (aa[DymSortCol] == bb[DymSortCol]) return 0;
  else                                       return -1;
}

static int dymcompd(const void *a, const void *b)
{
  double *aa = *(double **)a;
  double *bb = *(double **)b;
  if (aa[DymSortCol] > bb[DymSortCol])       return -1;
  else if (aa[DymSortCol] == bb[DymSortCol]) return 0;
  else                                       return 1;
}

/* sort rows based on values in the given column */
void dym_sort(dym *d, int sortcol, bool ascending)
{
#ifndef AMFAST
  if ((sortcol<0)||(sortcol>=dym_cols(d))) 
    my_error("dym_sort: invalid sortcol");
#endif

  DymSortCol = sortcol;
  if (ascending) qsort((void *)d->tdarr,dym_rows(d),sizeof(double *),dymcompa);
  else           qsort((void *)d->tdarr,dym_rows(d),sizeof(double *),dymcompd);
}

