
/*
   File:        amdym.h
   Author:      Andrew W. Moore
   Created:     Thu Sep 15 21:01:12 EDT 1994
   Updated:     amdm was split into amdyv, amdym and svd by Frank Dellaert, Aug 14 1997
   Updated:     "FANCY" conditionally compiled matlab routines were not conditionally
                declared - sir 8/6/2000
   
   Description: Header for Dynamically allocated and deallocated matrices

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

#ifndef AMDYM_H
#define AMDYM_H

#include "standard.h"
#include "ambs.h"
#include "amdyv.h"
/*
   Dynamic Matrices

   The following routines are not clever, just sensible, not-inefficient
   implementations of dynamic matrices. Here's an example of their use:

   #include "amdym.h"

   FILE *s = fopen(fname,"r");
   dym *dm = read_dym(s,NULL,NULL);  Creates a dynamic matrix from ascii file
   dym *dmt = mk_dym_transpose(dm);  Creates Its transpose
   dym *a = mk_dym_mult(dm,dmt);     Creates A square matrix

   fclose(s);
   free_dym(dm);
   free_dym(dmt);
   free_dym(a);

%%%%% Important conventions for using dyms.

Dyms are addressed by (row,column) pairs, 0 <= row < num-of-rows and
 0 <= col < number-of-columns.
Some of the dym functions dynamically allocate memory. All such functions
have names beginning with mk_

Two of the dym functions free memory. They have names beginning with
free_.

All other named functions simply copy data between already-allocated
memory.

Many functions have a "mk_" form and a conventional copy form.
The conventional form places the result in another argument of
the call (which must have been pre-initialized to have the correct
number of rows and columns). The name of an argument which gets stuff
copied into it usually begins with r_ . The mk_ form creates
an appropriately-shaped
dym, computes the result, and returns the new dym.

For example, here I explain two functions. The "copy" form of matrix
addition which copies the sum of d_1 and d_2 into r_d. And the "mk"
form dynamically creates and returns a matrix in which d_1 + d_2
are stored.
    void dym_plus(dym *d_1, dym *d_2, dym *r_d);
    dym *mk_dym_plus(dym *d_1,dym *d_2);
    Adds two matrices.

Notes on using dynamic matrices:
  Remember that you should always free any dynamic matrices you create.
  Otherwise, their memory will remain allocated for the rest of the
  time the program runs, which may mean you run out of memory unexpectedly
  early.

   The functions below all obey what we'll call the "matrix memory convention"
   This means that the arguments to the functions are not directly changed, or
   have any part over-written, except for the result matrix, which is usually
   the last argument to the function, called "r_mat". Furthermore it is
   legal, and fine, to call the function with the matrix to be changed as one
   of the input arguments. For instance, to double the values inside
   a matrix defined by matrix *a you could call
      dym_plus(a,a,a)
   and after calling each element of a would be twice its normal value.
      dym_mult(a,a,a) squares a etc.

%%%%% Basic operations on a dym (DYnamic Matrix)

   dym *mk_dym(int rows, int cols);    Dynamically allocates the dym.
                                       The initial values are undefined.

   double dym_ref(dym *dm,int i,int j);    Returns the (i,j)'th element of
                                          the matrix. May be implemented
                                          as a macro to improve speed. Indexing
                                          begins at zero: the ith element
              of the top row is dym_ref(dm,0,i), the ith element of the
              leftmost column is dym_ref(dm,i,0).

	      i=row, j = col

   double dym_set(dym *dm,int i, int j, double value);
       Sets the (i,j)'th element of the matrix to be value. See dym_ref
       for the indexing convenion.

   int dym_rows(dym *dm) gives the number of rows in the dym

   int dym_cols(dym *dm) gives the number of cols in the dym


%%%%% Numerical operations on a dym (DYnamic Matrix)
   
void constant_dym(dym *r_d,double value);
dym *mk_constant_dym(int rows,int cols,double value)
Sets all elements to be value

void zero_dym(dym *r_d);
dym *mk_zero_dym(int rows,int cols)
Sets all elements to be 0.0

void dym_scalar_mult(dym *d, double alpha, dym *r_d);
dym *mk_dym_scalar_mult(dym *d,double alpha);
Multiplies all elements of d by alpha. 

void dym_scalar_add(dym *d, double alpha, dym *r_d);
dym *mk_dym_scalar_add(dym *d,double alpha);
Adds alpha onto all elements of d. 


void copy_dym(dym *d, dym *r_d);
dym *mk_copy_dym(dym *d);
Creates a copy of the dym.

void dym_plus(dym *d_1, dym *d_2, dym *r_d);
dym *mk_dym_plus(dym *a,dym *b);
Adds two matrices.

void dym_subtract(dym *d_1,dym *d_2,dym *r_d);
dym *mk_dym_subtract(dym *a,dym *b);
Subtracts a matrix.

%%%%%%% Transformations between dyvs , dyms , 1-d arrays of doubles
%%%%%%% and 2-d arrays of doubles.

We define dym   = DYnamic Matrix
          dyv   = DYnamic Vector
          farr  = our notation for an array of doubles. If a is declared
                  by double *a or double a[100], then a is a farr
          tdarr = our notation for a 2-d array of doubles IMPLEMENTED
                  array of POINTERS to array of doubles.
                    For, example, something created by 
                    am_malloc_2d_realnums(rows,cols).
                  BUT NOT SOMETHING DECLARED BY a[100][30] which is
                  implemented as a bloacking of 3000 doubles by the
                  C compiler.
 
There are a huge number of functions to copy, and make items of one
of the above types into items of another of the above types.

You can do the obvious copying between the following types:

         Source ---> farr    dyv     tdarr     dym
 Dest |
      |
      V

    farr               -      y        -       (y)
    dyv                y      -       (y)      (y)
    tdarr              -     (y)       -        y
    dym               (y)    (y)       y        -

The (y)'s in parentheses have two kinds: copying to/from a column
of a 2-d structure to a 1-d structure or copying to/from a row
of a 2-d structure to a 1-d structure.

Thus, lets make a more complete table for 1-d copying and making:

         Source ---> farr    dyv    tdarr_row   tdarr_col dym_row dym_col
 Dest |
      |
      V

    farr              -       y        -           -        y       y
    dyv               y    copy_dyv    y           y        y       y
    tdarr_row         -       y        -           -        -       -
    tdarr_col         -       y        -           -        -       -
    dym_row           y       y        -           -        -       -
    dym_col           y       y        -           -        -       -

And for 2-d:
        Source ---> tdarr    dym
 Dest V
     tdarr            -       y
     farr             y     copy_dym

Including all the combinations of copying and making copies of, there
are at least 50 such functions. To make life simple, theres are
strong naming convention for all such functions:

to copy from a <Source> x, to a <Dest> y, call

        copy_<Source>_to_<Dest>(x,y)
 e.g;
       double y[10];
       dym *x = mk_constant_dyv(6,1.0); / Creates (1.0,1.0,1.0,1.0,1.0,1.0) /
       copy_dyv_to_farr(x,y);    /  Fills first 6 entries in y with 1.0  /

to make a new thing of type <Dest> from a thing of type <Source> do
       mk_<Dest>_from_<Source>(x)
  e.g;
       dym *x = mk_constant_dym(2,3,9.0);
                       /  Creates ( 9.0 , 9.0 , 9.0 )
                                  ( 9.0 , 9.0 , 9.0 )   /
       double *y = mk_tdarr_from_dym(x);    /  Creates
     
BUT: Some functions need integer numerical arguments too. These
always occur after the source and dest argument.

   To make a dyv from a farr you need to say how long the farr is,
    e.g. y = mk_dyv_from_farr(x,6) says to make a dyv of size 6 from the
          first 6 elements of the array of doubles x.
    y = mk_dyv_from_dym_row(x,4) says to make a dyv from the the row
          indexed by 4 (which is of course the fifth row from the top)
          of the dym x.
     
    See also:
        dym *mk_dym_from_tdarr(double **tdarr,int rows,int cols)
        dyv *mk_dyv_from_tdarr_row(double **tdarr,int row,int tdarr_cols);
           ..which copies from the row of tdarr indexed by "row", and is
             told that tdarr has "tdarr_cols" columns
        dyv *mk_dyv_from_tdarr_col(double **tdarr,int col,int tdarr_rows);

   Finally, it is possible to make whole dym's directly from dyvs and farrs
   in the following three ways:

     dym *mk_<DYMSHAPE>_dym_from_dyv(dyv *dv);
     where <DYMSHAPE> is one of: row (creates a single-row-dym),
                                 col (creates a single-column-dym),
                                 diag (creates a diagonal dym)

       and 
     dym *mk_<DYMSHAPE>_dym_from_farr(double *farr,int size);

%%%%%%%%% Making small dyms

And also you can make any of the 9 smallest matrices thusly:

dym *mk_dym_RC( ... ) where R and C are each either 1 , 2 or 3

and there are R * C arguments, which initialize (in lexical order
x00, x01, .. etc) the dym. E.G:

   dym *d = mk_dym_32(1.0,2.0,3.0,4.0,5.0,6.0);

 allocates memory and creates a dym = [ 1.0 2.0 ]
                                      [ 3.0 4.0 ]
                                      [ 5.0 6.0 ]

%%%%%%%%% Complex matrix operations
void dym_times_dyv(const dym *a, const dyv *b,dyv *result);
dyv *mk_dym_times_dyv(const dym *a, const dyv *b);
returns a b (matrix times  vector ---> vector)

void dym_mult(dym *d_1, dym *d_2, dym *r_d);
dym *mk_dym_mult(dym *a,dym *b);
return a b (matrix times matrix ---> matrix)

void dyv_outer_product(dyv *a, dyv *b, dym *r_d);
dym *mk_dyv_outer_product(dyv *a, dyv *b);
returns a^T b (vector times vector --> matrix)

void dym_transpose(dym *d, dym *r_d);
dym *mk_dym_transpose(dym *a);
returns a^T

void dym_scale_rows(dym *d,dyv *w_diag,dym *r_d);
dym *mk_dym_scale_rows(dym *d,dyv *w_diag);
each element in ith row is multiplied by dyv_ref(w_diag,i).
Equivalent to Diag(w_diag) * d


void dym_scale_cols(dym *d,dyv *w_diag,dym *r_d);
dym *mk_dym_scale_cols(dym *d,dyv *w_diag);
each element in ith column is multiplied by dyv_ref(w_diag,i).
Equivalent to d * Diag(w_diag)

bool is_dym_symmetric(dym *d);

void fprintf_dym(FILE *s,char *m1,dym *d,char *m2);

void fprintf_formatted_dym(FILE *s, dym *d, int places, int dec_places);

void fprintf_dym_and_confidence(
    FILE *s,
    char *m1,
    dym *d,
    dym *conf,
    bool huge_uncertainty,
    char *m2
  );
*/

/*
   A note on the AMFAST flag.

   This code can be compiled in two ways.

   The default uses no AMFAST, and many operations and array limits are
   checked. Matrices are always checked that they're the same size.
   Matrices values are initialized to -7.777e27 as a hint that something
   is wrong for someone who doesn't initialize their stuff properly.
   The data structures have a flag to test whether they represent legally
   allocated vectors and matrices. dyv_ref() and other vector and matrix
   update routines are implemented as functions.

   The above all takes time, so if AMFAST is set:
     - None of the above checks take place
     - The data structures don't have the code flag set
     - some of the access functions are replaced by macros. You
       can be assured that dym_set(dm,i,j,value) is as fast as a direct
       C array update.
*/

typedef struct dym_struct
{
  int dym_code;
  int rows;
  int cols;
  int rows_allocated;
  double **tdarr;
} dym, *dym_ptr;

dym *mk_dym(int rows,int cols);

/* Note that you lose all your data when you do this. It's designed to
save you from needing to pass a double pointer if you want to size
your dym in a child function.  */
void dym_destructive_resize(dym* x, int rows, int cols);

/* Warning: no type checking can be done by the compiler.  You *must*
   send the values as doubles for this to work correctly. */
dym *mk_dym_x( int rows, int cols, ...);

void free_dym(dym *d);

void free_dym_ill_ok(dym *d);

void dym_malloc_report(void);

void add_row(dym *d);
void add_rows(dym *d, int num_rows);

double safe_dym_ref(const dym *d, int i,int j);
void safe_dym_set(dym *d,int i,int j,double value);
void safe_dym_increment(dym *d,int i,int j,double value);

bool is_same_realnum(double x,double y); /* XXX This doesn't really belong here */

#ifdef AMFAST

#define dym_ref(d,i,j) ((d)->tdarr[i][j])
#define dym_set(d,i,j,v) (d)->tdarr[i][j] = (v)
#define dym_increment(d,i,j,v) (d)->tdarr[i][j] += (v)

#else /* If not AMFAST */

#define dym_ref(d,i,j) (safe_dym_ref(d, i, j))
#define dym_set(d,i,j,v) (safe_dym_set(d,i,j,v))
#define dym_increment(d,i,j,v) (safe_dym_increment(d,i,j,v))

#endif /* #ifdef AMFAST */


void copy_dym_to_tdarr(dym *d,double **tdarr);

double **mk_tdarr_from_dym(dym *d);

void copy_tdarr_to_dym(double **tdarr,int rows,int cols,dym *r_d);

dym *mk_dym_from_tdarr(double **tdarr,int rows,int cols);

void copy_dym_row_to_dym_row(dym *src,int src_row, dym *dst,int dst_row);
void copy_dym_col_to_dym_col(dym *src,int src_col, dym *dst,int dst_col);

void copy_dyv_to_dym_row(const dyv *dv,dym *dm,int row);

void copy_dyv_to_dym_col(const dyv *dv,dym *dm,int col);

void copy_dym_row_to_dyv(const dym *dm,dyv *dv,int row);


/* copies values from dym to dyv; 
   set row_wise flag to scan rowwise or columnwise */
void copy_dyv_from_dym(dyv *dv, dym *dm, int row_wise);

/* makes a dyv from dym; 
   set row_wise flag to scan rowwise or columnwise */
dyv *mk_dyv_from_dym(dym *d, int row_wise);

dyv *mk_dyv_from_dym_row(const dym *dm,int row);

void copy_dym_col_to_dyv(const dym *dm, dyv *dv, int col);

dyv *mk_dyv_from_dym_col(const dym *dm, int col);

void copy_farr_to_dym_row(double *farr,dym *dm,int row);

void copy_farr_to_dym_col(double *farr,dym *dm,int col);

void copy_dym_row_to_farr(dym *dm,double *farr,int row);

void copy_dym_to_farr(dym *d,double *farr);

double *mk_farr_from_dym_row(dym *dm,int row);

void copy_dym_col_to_farr(dym *dm,double *farr,int col);

double *mk_farr_from_dym_col(dym *dm,int col);

dym *mk_col_dym_from_dyv(dyv *dv);

dym *mk_row_dym_from_dyv(dyv *dv);

dym *mk_diag_dym_from_dyv(dyv *dv);

void add_dyv_to_dym_row(dym *r_a, dyv *b, int row);
void add_dyv_to_dym_col(dym *r_a, dyv *b, int col);
void add_dym_row_to_dyv(dym *a, dyv *b, int row);
void add_dym_col_to_dyv(dym *a, dyv *b, int col);

void add_dyv_to_diag(dym *r_a,dyv *b);

dym *mk_add_dyv_to_diag(dym *a,dyv *b);

dym *mk_col_dym_from_farr(double *farr,int farr_size);

dym *mk_row_dym_from_farr(double *farr, int farr_size);

dym *mk_diag_dym_from_farr(double *farr, int farr_size);

void append_dyv_to_dym(dym* dm, dyv* row);

/* Appends dm2 to dm1 */
void append_dym_to_dym(dym* dm1, dym* dm2);

int dym_rows(const dym *d);

int dym_cols(const dym *d);

void save_dym(FILE *s,dym *d);

void save_io_dyms(FILE *s,dym *ins,dym *outs);

/* Following function tries to read dyms. If any syntax error or file
   problem, returns FALSE, and all results are NULL. If okay,
   returns TRUE, and *r_in_dym and *r_out_dym point to dyms read in.
*/


#ifdef FANCY
bool can_read_io_dyms(
    char *fname,
    char *format,
    dym **r_in_dym,
    dym **r_out_dym
  );


dym *read_dym(FILE *s,char *fname,char *format);
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

#endif

void constant_dym(dym *r_d,double v);

void zero_dym(dym *r_d);

dym *mk_constant_dym(int rows,int cols,double v);

dym *mk_zero_dym(int rows,int cols);

void dym_scalar_mult(const dym *d, double alpha, dym *r_d);

dym *mk_dym_scalar_mult(const dym *d,double alpha);

void dym_scalar_add(dym *d, double alpha, dym *r_d);

dym *mk_dym_scalar_add(dym *d,double alpha);

void copy_dym(dym *d, dym *r_d);

dym *mk_copy_dym(const dym *d);

void dym_plus(const dym *d_1, const dym *d_2, dym *r_d);
void dym_minus(const dym *d_1, const dym *d_2, dym *r_d);

dym *mk_dym_plus(const dym *a,const dym *b);

void dym_subtract(const dym *d_1,const dym *d_2,dym *r_d);

dym *mk_dym_subtract(const dym *a,const dym *b);

void dym_times_dyv(const dym *a, const dyv *b, dyv *result);

dyv *mk_dym_times_dyv(const dym *a,const dyv *b);

void dym_mult(dym *d_1, dym *d_2, dym *r_d);

dym *mk_dym_mult(const dym *a, const dym *b);

void dyv_outer_product(dyv *a, dyv *b, dym *r_d);

dym *mk_dyv_outer_product(dyv *a, dyv *b);

void dym_transpose( const dym *d, dym *r_d);

dym *mk_dym_transpose( const dym *a);

void dym_scale_rows(dym *d,dyv *w_diag,dym *r_d);

dym *mk_dym_scale_rows(dym *d,dyv *w_diag);

void dym_scale_cols(dym *d,dyv *w_diag,dym *r_d);

dym *mk_dym_scale_cols(dym *d,dyv *w_diag);

bool is_dym_symmetric(const dym *d);

void enforce_dym_symmetry(dym *d); /* XXX ??? */

void pdym(dym *d);

void fprintf_dym(FILE *s, const char *m1, const dym *d, const char *m2);

void fprintf_dym_and_confidence(
    FILE *s,
    char *m1,
    dym *d,
    dym *conf,
    bool huge_uncertainty,
    char *m2
  );

void fprintf_formatted_dym(FILE *s, dym *d, int places, int dec_places);

void fprintf_dym_dym(FILE *s,char *m1,dym *d1,char *m2,dym *d2,char *m3);

void fprintf_dym_dyv(FILE *s,char *m1,dym *d1,char *m2,dyv *d2,char *m3);

/*makes a dyv corresponding to the min value in each col*/
dyv *mk_min_dyv(dym* d);
/*makes a dyv corresponding to the max value in each col*/
dyv *mk_max_dyv(dym* d);

double dym_min(dym *dv);
double dym_max(dym *dv);

double dym_row_max(dym *dm,int row);

double dym_row_min(dym *dm,int row);

int dym_row_argmax(dym *dm,int row);

int dym_row_argmin(dym *dm,int row);

double dym_col_max(dym *dm,int col);

double dym_col_min(dym *dm,int col);


dym *mk_dym_11(double x00);
dym *mk_dym_12(double x00, double x01);
dym *mk_dym_13(double x00, double x01, double x02);
dym *mk_dym_21(double x00,
               double x10);
dym *mk_dym_22(double x00, double x01,
               double x10, double x11);
dym *mk_dym_23(double x00, double x01, double x02,
               double x10, double x11, double x12);
dym *mk_dym_31(double x00,
               double x10,
               double x20);
dym *mk_dym_32(double x00, double x01,
               double x10, double x11,
               double x20, double x21);
dym *mk_dym_33(double x00, double x01, double x02,
               double x10, double x11, double x12,
               double x20, double x21, double x22);

void dym_ptq(dym *p,dym *q,dym *r_dym);
/*
   Sets r_dym to contain p^T q.
*/

dym *mk_dym_ptq(dym *p,dym *q);
/*
   Makes and returns p^T q
*/

/* p^t q p */
void dym_ptqp(dym *p,dym *q,dym *r_dym);
dym *mk_dym_ptqp(dym *p,dym *q);

void dym_transpose_times_dyv(dym *p,dyv *v,dyv *r_dyv);

dyv *mk_dym_transpose_times_dyv(dym *p,dyv *v);

dym *mk_identity_dym(int dims);

dym *mk_dym_from_string(char *s);

dym *mk_dym_from_args(const char *name,int argc,char *argv[],dym *deflt);
/* COPIES in deflt (if so required) */

double dym_sum(const dym *x);

double dym_sum_row(const dym *x, int row);

double dym_sum_col(const dym *x, int col);

void dym_remove_row(dym *x, int row);

/* Returns TRUE if any elements are NaN or Inf */
bool dym_is_ill_defined(dym *x);

#ifdef AMFAST
#define assert_dym_shape(d,rows,cols,name)
#else
void assert_dym_shape(dym *d,int rows, int cols,char *name);
#endif /* #ifdef AMFAST */

void dym_sort(dym *d, int sortcol, bool ascending);

dym *mk_dym_normalize_rows(dym *d);

#endif /* #ifndef AMDYM_H */
