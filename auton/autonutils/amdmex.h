/* 
   File:        amdmex.h
   Author:      Andrew W. Moore
   Created:     Oct 11, 1996
   Updated:     8 Dec 86
   Description: Extensions and advanced amdm stuff

   Copyright 1996, Schenley Park Research

   This file contains advanced utility functions involving dyvs dyms and
   ivecs. It never accesses the data structures directly, so if the
   underlying representation of dyms and dyvs changes these won't need to.

   The prototypes of these functions used to be declared at the end of amdm.h.
   Now it's amdmex.h

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

#ifndef AMDMEX_H
#define AMDMEX_H

#include "standard.h"
#include "ambs.h"
#include "amdyv.h"
#include "amdym.h"
#include "amdyv_array.h"  /* dyv_array's typedef is in this file now */
#include "am_string_array.h"

/* Useful symbols for dealing with 2-d rectangles... */
#define XCOMP 3333
#define YCOMP 4444
#define LO    5555
#define HI    6666

typedef struct dym_array
{
  int size;
  int array_size;
  dym **array;
} dym_array;


/* Returns the number of times val occurs in iv. */
int count_in_ivec( ivec *iv, int val);

ivec *mk_reverse_ivec(ivec *x);

void reverse_ivec(ivec *x);

dyv_array *mk_dyv_array_transpose(dyv_array *x);

/* Inverts ivec f, writing magic to values of inverse that don't occur in f.
   Passed ivec must be non-negative -- this is an unnecessary restriction,
   but 1) it seems like the usual case for calls to this function, and
   2) it avoid making the caller pass a "known good" magic value to use
   if the f is into and not onto [0,max(f)] */
ivec *mk_invert_nonneg_ivec(ivec* f);

void assert_is_permutation_ivec(const ivec* iv);

/* Assumes that f is a permutation of the identity ivec. */
ivec *mk_invert_permutation_ivec(ivec* f);

/*
  Efficiently remove elements from iv according to indices stored
  in the sivec siv.
*/
void ivec_remove_sivec( ivec *iv, ivec *siv);

/* Forall (i,j) such that ilo <= i < ihi
                          jlo <= j < jhi (note the strict inequality on right)

   We do a[i][j] += delta
*/
void dym_increment_block(dym *a,int ilo,int jlo,int ihi,int jhi,
				  double delta);


void indices_of_sorted_ivec(ivec *v,ivec *iv);

bool dym_weakly_dominates(const dym *dx, const dym *dy);
bool dym_equal(const dym *dx, const dym *dy);
dym *mk_subdym( const dym *dm, const ivec *goodrows, 
			 const ivec *goodcols);

void normalize_l1_dym_rows(dym *d);
void normalize_l1_dyv(dyv *d);

#ifndef AMFAST
#define dyv_partial_sum(dv,indices) (dyv_partial_sum_slow( dv, indices))
#else
#define dyv_partial_sum(dv,indices) (dyv_partial_sum_fast( dv, indices))
#endif

double dyv_partial_sum_slow( const dyv *dv, const ivec *indices);
double dyv_partial_sum_fast( const dyv *dv, const ivec *indices);

/* Makes a random dyv with magnitude 1.0 */
dyv *mk_random_unit_dyv(int size);

/* save the dyv on one line of a file in a format easy to load.
   the comment is just to make the file a small bit human readable.
   It may be NULL.
*/
void fprintf_dyv_for_load(FILE *s, const dyv *d, char *comment);
/* Warning this function only works if the next line of fp contains a dyv
   stored using the above function
*/
dyv *mk_dyv_from_file(FILE *fp, char **r_errmess);

void fprintf_formatted_oneline_dyv(FILE *s,const char *m1,
					    const dyv *d, 
					    const char *m2, int n);

/* Makes an ivec with random values. */
ivec *mk_random_ivec( int size, int min, int max);

/* Create array of random ivecs of equal length. */
ivec_array *mk_random_ivec_array( int numivecs, int width, int min, 
					   int max);


/* Makes a random subset of iv of size "k" */
ivec *mk_random_ivec_subset(ivec *iv,int k);

/* Same as above, but takes O(k) time rather than O(ivec_size(iv)) */
ivec *mk_random_ivec_subset_fast(ivec* iv, int k);

ivec *mk_ivec_from_dyv(dyv *d);
dyv *mk_dyv_from_ivec(const ivec *iv);
ivec *mk_ivec_from_args(char *key,int argc,char *argv[],ivec *def);
void copy_dym_row(dym *source,int source_row,dym *dest,int dest_row);
void copy_dym_col(dym *source,int source_col,dym *dest,int dest_col);
void swap_dym_rows(dym *dm,int i,int j);
void swap_dym_cols(dym *dm,int i,int j);

/* rows may be NULL dneoting "use all rows" */
double sum_of_dyv_rows(const dyv *dv,const ivec *rows);

/* In the following functions, rows may be NULL denoting USE ALL ROWS */
dyv *mk_sum_of_dym_rows(dym *dm,ivec *rows);
void sum_of_dym_rows(dym *dm,ivec *rows, dyv *r_sum) ;

dyv *mk_sumsq_of_dym_rows(dym *dm,ivec *rows);
dyv *mk_mean_of_dym_rows(dym *dm,ivec *rows);
dyv *mk_sdev_of_dym_rows(dym *dm,ivec *rows);

dyv *mk_sumsq_of_dym_cols(dym *dm,ivec *cols);

void save_dym_array_to_file(FILE *s, dym_array *ma);

ivec *mk_ivec_from_dym_col(dym *a,int col);
ivec *mk_ivec_from_dym_row(dym *a,int row);

dyv *mk_sum_of_dym_cols(dym *dm);
dyv *mk_mean_of_dym_cols(dym *dm);

bool is_in_dyv(dyv *dv,double value, double error);
int find_index_in_dyv(dyv *dv,double value,double error);

dym_array *mk_empty_dym_array(void);

/* Creates an dym array with size entries each filled with the given dym. */

dym_array *mk_const_dym_array(dym *base_vec, int size);


void add_to_dym_array(dym_array *da,dym *dm);
int dym_array_size(dym_array *da);
dym *dym_array_ref(dym_array *da,int idx);
void dym_array_set(dym_array *da,int idx,dym *val);

void pdym_array(dym_array *x);
void fprintf_dym_array(FILE *s,char *m1,dym_array *da,char *m2);
void free_dym_array(dym_array *da);
dym_array *mk_copy_dym_array(dym_array *da);
void mk_io_dyvs_from_string(char *string,char *format,dyv **r_in_dyv,dyv **r_out_dyv);
dyv *mk_midpoint_dyv(dyv *a,dyv *b);
int find_index_of_kth_smallest(const dyv *x,int k);
double dyv_kth_smallest(const dyv *d,int k);
void save_dym_to_file(FILE *s, dym *m);

/* save and load a dyv or ivec from/to a stream.
   The actual data is stored as a dym. This
   way you can read a dyv or an ivec from a file and not
   have it scan all the way through the end - just
   as much as the saver had saved
*/   
void save_dyv_as_dym(FILE *s, const dyv *dv);
dyv *load_dyv_as_dym(FILE *s, char **err_msg);

void save_ivec_as_dym(FILE *s, const ivec *iv);
ivec *load_ivec_as_dym(FILE *s, char **err_msg);

string_array *mk_string_array_from_stream(FILE *s,char **r_errmess);

void save_string_array_to_stream(FILE *s, const string_array *sa);

/* Saves dyv to the file in a format that can later be read back with
   mk_dyv_from_file. */
void save_dyv_to_stream(FILE *s, const dyv *v);

/* Reads a dyv from a file.  The dyv must have been saved in the
   format produced by save_dyv_to_file.  The file pointer must be
   at the start of the dyv on entry, and will be at the end of the
   dyv on exit.  

   If this succeeds, r_errmess is set to NULL.  Otherwise it is
   set to a message explaining the problem, and mk_dyv_from_file
   returns NULL. */
dyv *mk_dyv_from_stream(FILE *s, char **r_errmess);

/* Saves ivec to the file in a format that can later be read back with
   mk_ivec_from_file. */
void save_ivec_to_stream(FILE *s, const ivec *v);

/* Reads a ivec from a file.  The ivec must have been saved in the
   format produced by save_ivec_to_file.  The file pointer must be
   at the start of the ivec on entry, and will be at the end of the
   ivec on exit.  

   If this succeeds, r_errmess is set to NULL.  Otherwise it is
   set to a message explaining the problem, and mk_ivec_from_file
   returns NULL. */
ivec *mk_ivec_from_stream(FILE *s, char **r_errmess);

/*** Supplementary string_array load/save operations ***/

/* like mk_string_array_from_filename except each line will be one string in
   the array.  
*/
string_array *mk_string_array_from_filename_lines(const char *filename,
							   char **r_errmess);

/* Loads a string_array from the given filename. 

    If succeeds, returns the string_array and sets *r_errmess = NULL.
    If fails, returns NULL and AM_MALLOCS a null-terminated error
      message string in *r_errmess
*/
string_array *mk_string_array_from_filename(const char *filename, 
						     char **r_errmess);

/* Loads string_array from filename. If there's a problem, prints an
   error to stdout and aborts the entire program. */
string_array *mk_string_array_from_filename_simple(const char *filename);

/* Saves string_array to filename. If there's a problem, prints an
   error to stdout and aborts the entire program. */
void save_string_array_to_filename(const char *filename, 
					    const string_array *x);

/*** End Supplementary string_array load/save operations ***/

/*** Supplementary dyv load/save operations ***/

/* Loads a dyv from the given filename. 

    If succeeds, returns the dyv and sets *r_errmess = NULL.
    If fails, returns NULL and AM_MALLOCS a null-terminated error
      message string in *r_errmess
*/
dyv *mk_dyv_from_filename(const char *filename, char **r_errmess);

/* Loads dyv from filename. If there's a problem, prints an
   error to stdout and aborts the entire program. */
dyv *mk_dyv_from_filename_simple(const char *filename);

/* Saves dyv to filename. If there's a problem, prints an
   error to stdout and aborts the entire program. */
void save_dyv_to_filename(const char *filename, const dyv *x);

/*** End Supplementary dyv load/save operations ***/

/*** Supplementary ivec load/save operations ***/

/* Loads a ivec from the given filename. 

    If succeeds, returns the ivec and sets *r_errmess = NULL.
    If fails, returns NULL and AM_MALLOCS a null-terminated error
      message string in *r_errmess
*/
ivec *mk_ivec_from_filename(const char *filename, char **r_errmess);

/* Loads ivec from filename. If there's a problem, prints an
   error to stdout and aborts the entire program. */
ivec *mk_ivec_from_filename_simple(const char *filename);

/* Saves ivec to filename. If there's a problem, prints an
   error to stdout and aborts the entire program. */
void save_ivec_to_filename(const char *filename, const ivec *x);

/*** End Supplementary ivec load/save operations ***/

/*** Supplementary ivec load/save operations ***/

ivec *mk_sivec_from_stream_tokens(FILE *s);

ivec *mk_sivec_from_file_tokens(const char *const filename);

/*** End Supplementary ivec load/save operations ***/

dyv_array *mk_dyv_array_from_dym_rows(dym *dm) ;
dyv_array *mk_dyv_array_from_dym_cols(dym *dm) ;

/* PRE: all dyvs in x must have the same number of elements.
      : x must contains at least one dyv

   Makes dym by treating the i'th element of x as the ith row of matrix

   dym_ref(result,i,j) == dyv_ref(dyv_array_ref(x,i),j)
*/
dym *mk_dym_from_dyv_array(dyv_array *x);

/* PRE: all dyvs in x must have the same number of elements.

   dyv_ref(dyv_array_ref(result,i),j) == dyv_ref(dyv_array_ref(x,j),i)
*/

dym_array *mk_dym_array_from_file(FILE *s, char **r_errmess);

/* Returns a dym_array of size "num_dyms" in which the i'th
   element (forall 0 <= i < num_dyms) is a matrix of
   size "dym_size x dym_size" in which each element is 0.0 */
dym_array *mk_dym_array_of_zeroed_dyms(int num_dyms,int dym_size);

/* Returns a dym_array of size "num_dyms" in which the i'th
   element (forall 0 <= i < num_dyms) is a matrix of
   size "dym_size_r x dym_size_c" in which each element is 0.0 */
dym_array *mk_dym_array_of_zeroed_nonrect_dyms(int num_dyms,
							int dym_size_r,
							int dym_size_c);

void zero_dym_array(dym_array *da);

string_array *mk_string_array_from_stream_tokens(FILE *s);

/* Returns NULL if can't open file */
string_array *mk_string_array_from_file_tokens(char *filename);

/* Returns NULL if can't open file */
char *mk_string_from_file_tokens(char *filename);

void make_argc_argv_from_string_array( const string_array *sa,
						int *actual_argc,
						char ***actual_argv);

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
				    int *actual_argc,char **r_actual_argv[]);

void free_loaded_argc_argv(int argc,char **argv);

/* The following is for if you want to rename "argfile" to some other
   flag name */
void load_actual_argc_argv_with_keyword(int argc, char** argv, 
						 int* actual_argc, 
						 char*** actual_argv,
						 char* keyword);

/* Following is to make an argc/argv from what's provided with the given
   string removed */
void mk_filtered_argc_argv(int argc, char** argv, int* actual_argc,
				    char*** actual_argv, char* remove);

/* Functions to manipulate typical argv arrays for AUTON programs. */
char **mk_copy_argv( int argc, char **argv);
void free_argv_copy( int argc, char **argvcopy);
void fprintf_argv( FILE *f, char *pre, int argc, char **argv, 
			    char *post);
void mk_argv_copy_append_val( int argc, char **argv, char *val,
				       int *newargc, char ***newargv);
void mk_argv_copy_with_new_key_val( int argc, char **argv,
					     char *key, char *val,
					     char ***newargv);
void mk_argv_copy_remove_key_val( int argc, char **argv, char *key,
					   int *newargc, char ***newargv);

bool exload_white_space(char c);

void save_dyv_to_file_plain(char *fname,dyv *x,char **r_errmess);
void execute_command(char *exec_string,char **r_errmess);
dyv *mk_dyv_from_file_plain(char *fname,int size,char **r_errmess);

/* Supplementary load operations for lines with a header.
   For example, suppose a file looked like:

   Num_patients: 20
   Interesting_indices: 1 2 3
   Probability: 0.95
   Names: Larry Curly Moe

   The "headers" here are "Num_patients:","Interesting_indices:", and
   "Probability".  Notice that no spaces are allowed in these headers.
   For the first line, we obtain an int from the line.
   The second line gives us an ivec.  The third line gives back a double.
   Finally, for the fourth line we want the list of names, which is a string
   array without the header.  There are utility functions to do these 
   operations and more.
 */

int read_bool_from_file_with_header(FILE* fp, char* filename, 
					     char* header_str,
					     int* linenum);
int read_int_from_file_with_header(FILE* fp, char* filename, 
					    char* header_str, 
					    int* linenum);
double read_double_from_file_with_header(FILE* fp, char* filename, 
						  char* header_str, 
						  int* linenum);
dyv* mk_dyv_from_file_with_header(FILE* fp, char* filename, 
					   char* header_str, 
					   int* linenum);
ivec* mk_ivec_from_file_with_header(FILE* fp, char* filename,
					     char* header_str, int* linenum);

/* The string returned has memory allocated for it so remember to free it.  
   This function Returns NULL if line is not valid */
char* mk_string_from_file_with_header(FILE* fp,
					       char* filename,
					       char* header_str,
					       int* linenum);
string_array* mk_string_array_from_file_with_header(FILE* fp,
							     char* filename,
							     char* header_str,
							     int* linenum);

/* Corresponding functions to write to a file using a header*/

/* 
   Suppose we have an ivec {0,1,2} and a header "Header:".  This will write
   out the ivec to a file as:
   Header: 0 1 2
*/
void ivec_write_with_header(FILE* fp, char* header, ivec* iv);

void dyv_write_with_header(FILE* fp, char* header, dyv* dv);

void string_array_write_with_header(FILE* fp, char* header, 
					     string_array* sa);

/********* DATFILE PARSING UTILITIES ************/
/* mostly moved to am_string_array.[ch], since */
/* they don't depend on anything but string_arrays */

void make_attnames_and_dym_from_filename(const char *filename,
						  int argc,char *argv[],
						  string_array **r_attnames,
						  dym **r_x,char **r_errmess);

dym *mk_dym_from_file(FILE *s, char **r_errmess);
dym *mk_dym_from_filename(const char *filename,char **r_errmess);
dym *mk_dym_from_filename_simple(const char *filename);
void save_attnames_and_dym(FILE *s,string_array *attnames,dym *x);
void save_dym_to_filename(const char *filename, const dym *x);
void add_to_x_from_string_array(dym *x,string_array *sa,
					 char **r_errmess);

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

bool is_sivec(const ivec *iv);
bool is_sivec_array(const ivec_array* ia);

/* Makes { 0 , 1 , ... size-1 } */
ivec *mk_identity_sivec(int size);

/* Returns number of elements in sivec */
int sivec_size(const ivec *siv);

/* Returns the minimum value in sivec. 
   Time cost: Constant */
int sivec_min(const ivec *siv);

/* Returns the maximum value in sivec. 
   Time cost: Constant */
int sivec_max(const ivec *siv);

/* Adds the element while maintaining legal siveckiness.
   (If element already there, no change)
   Time cost: O(size)

   If pos is not NULL, then the position at which the value was inserted
   is stored there.
*/
void add_to_sivec_special(ivec *siv, int value, int *pos);
ivec *mk_add_to_sivec_special(ivec *siv, int value, int *pos);

void add_to_sivec(ivec *siv,int value);
ivec *mk_add_to_sivec(ivec *siv,int value);



/* Returns -1 if the value does not exist in siv.
   Else returns index such that
      value == ivec_ref(siv,value) 
  Time cost: O(log(size)) */
int index_in_sivec(const ivec *siv,int value);

/* Returns true iff siv contains value
   Time cost: O(log(size)) */
bool is_in_sivec(const ivec *siv,int value);

/* Does nothing if value is not in siv.
   If value is in siv, the sivec is updated to
   represent siv \ { value } */
void sivec_remove_value(ivec *siv,int value);
  
/* Returns answer to A subset-of B?
   Returns true if and only if the set of integers in a is
   a subset of the set of integers in b */
bool sivec_subset(const ivec *siva,const ivec *sivb);

bool sivec_equal(const ivec *siva,const ivec *sivb);

/* Returns TRUE iff A is a subset of B and A != B */
bool sivec_strict_subset(const ivec *siva,const ivec *sivb);

ivec *mk_sivec_union(const ivec *siva,const ivec *sivb);

/* Returns A \ B.
   This is { x : x in A and x not in B } */
ivec *mk_sivec_difference(const ivec *siva,const ivec *sivb);

/* Returns {0,...,size_of_everything-1} \ siva. */
ivec *mk_sivec_complement( const ivec *siva, int size_of_everything);

ivec *mk_sivec_intersection(const ivec *siva,const ivec *sivb);

int size_of_sivec_intersection(const ivec *siva,const ivec *sivb);

/* Returns TRUE iff A intersect B is empty. O(size) time */
bool sivec_disjoint(ivec *siva,ivec *sivb);

ivec *mk_sivec_from_ivec(const ivec *iv);

ivec *mk_ivec_from_string(char *s);

/* Turns a space separated string into a sivec.
   Example: "3 1 4 1 5 9" ====> { 1 , 3 , 4 , 5 , 9 } */
ivec *mk_sivec_from_string(char *s);

ivec *mk_sivec_1(int v1);
ivec *mk_sivec_2(int v1,int v2);
ivec *mk_sivec_3(int v1,int v2,int v3);
ivec *mk_sivec_4(int v1,int v2,int v3,int v4);
ivec *mk_sivec_5(int v1,int v2,int v3,int v4,int v5);

ivec_array *mk_array_of_empty_ivecs(int size);

ivec_array *mk_sivec_array_from_ivec_array(ivec_array *iva);

int index_of_longest_ivec(ivec_array *iva);

/* Makes an ivec from decimal integers stored in file.  Reads one int
   from each line of file.  Unlike other ivec-file ops, it is
   self-contained and doesn't assume mysterious file formats buried
   beneath multiple levels of dyv file ops and string-array file ops.
   Just loads ints, one per line. */
ivec *mk_ivec_from_file_of_ints( const char *fname);

/* Similar to mk_ivec_from_file_of_ints(), above. */
dyv *mk_dyv_from_file_of_doubles( const char *fname);


ivec *mk_union_of_sivec_array(ivec_array *iva);

ivec *mk_union_of_sivec_array_subset(ivec_array *iva,ivec *indexes);


/*----------------------- returns a random permutation of v */
void shuffle_dyv(dyv *v);

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
   where N is the length of source.
*/
dyv *mk_ranks_fast(dyv *source);


char *mk_explain_named_dyv_seps(dyv *values);
string_matrix *mk_explain_named_dyv_string_matrix(string_array *names,
						  dyv *values);
void explain_named_dyv(string_array *names,dyv *values);

/* forall i, rx[i] := x[i] + a */
void dyv_add_constant(dyv *x,double a,dyv *rx);
dyv *mk_dyv_add_constant(dyv *x, double a);

/* Returns dyv of size ivec_size(rows) in which
    result[i] = x[rows[i]] */
dyv *mk_dyv_subset( const dyv *x, const ivec *srows);

/* Writes dv[a] to dv[b-1] into r_dyv.  If a or b is negative, then
   it is replaced by by len+a or len+b. Returns the number of elements
   written.
*/
int dyv_slice( const dyv *dv, int a, int b, dyv *r_dv);

/* If b >= a >= 0, creates "slice" of dv on [a,b-1].  If a or b < 0, then
   it is replaced by len+a or len+b.  Indices which exceed bounds are
   replaced by appropriate indices.  Therefore this function always succeeds,
   sometimes returning an empty dyv.
*/
dyv *mk_dyv_slice( const dyv *dv, int a, int b);

/* Returns sorted dyv with unique values from dv. */
dyv *mk_dyv_unique( dyv *dv);

ivec *mk_ivec_intersection(ivec *a,ivec *b);

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
dym *mk_dym_subset( const dym *x, const ivec *rows, const ivec *cols);

dym *mk_dym_from_col_subset(dym *x, ivec *cols);

ivec* mk_find_all_indices_in_ivec(ivec* vect, int val);

ivec* mk_find_all_indices_in_dyv(dyv* vect, double val, double tol);

dyv *mk_dyv_extremes_from_center( dyv *dv, double center, 
					   int numextremes,
					   ivec **indices);
dyv *mk_dyv_extremes( dyv *dv, int numextremes, ivec **indices);
void fprintf_oneline_dyv_extremes_from_center( FILE *f, char *pre, 
							dyv *dv,
							char *post,
							double center,
							int numvals);
void fprintf_oneline_dyv_extremes( FILE *f, char *pre, dyv *dv, 
					    char *post,
					    int numvals);

void fprintf_subdym( FILE *f, char *pre, dyv *dv, dym *d, char *post,
			      const ivec *goodrows, const ivec *goodcols);


double median_of_three(double x,double y,double z);

/* Increases the number of rows in dm by 1 and copies the contents
   of dv into that new bottom row. Note this means that 
     dyv_size(dv) must equal dym_cols(dm)
   in order that dv will fit (if this is violated there'll be a my_error */
void add_dyv_to_dym(dym *dm,dyv *dv);

#endif /* #ifndef AMDMEX_H */
