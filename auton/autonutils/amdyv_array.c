/* 
   File:        amdyv_array.c
   Author:      Andrew W. Moore
   Created:     Tue Jun 15 12:31:18 EDT 2004
   Description: Extensions and advanced amdm stuff (no direct dym data access)

   Copyright 1996, Schenley Park Research

   This file contains advanced utility functions involving dyv_arrays, dyvs and
   ivecs. It never accesses the data structures directly, so if the
   underlying representation of dyms and dyvs changes these won't need to.

   The prototypes of these functions are declared in amdyv_array.h

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

#include "amdyv_array.h"


/***** Now we'll play with dyv_arrays which are adjustable length
       arrays of dyvs
*****/

#define INITIAL_DYV_ARRAY_SIZE 10

dyv_array *mk_empty_dyv_array(void)
{
  dyv_array *da = AM_MALLOC(dyv_array);
  da -> size = 0;
  da -> array_size = INITIAL_DYV_ARRAY_SIZE;
  da -> array = AM_MALLOC_ARRAY(dyv_ptr,da->array_size);
  return(da);
}

void add_to_dyv_array_no_copy(dyv_array *da, dyv *dv)
/*
     Assume dyv_array is previously of size n. After this it is of size
   n+1, and the n+1'th element is dv.
*/
{
  if ( da -> size == da -> array_size )
  {
    int new_size = 2 + 2 * da->array_size;
    dyv **new_array = AM_MALLOC_ARRAY(dyv_ptr,new_size);
    int i;
    for ( i = 0 ; i < da -> array_size ; i++ )
      new_array[i] = da->array[i];
    AM_FREE_ARRAY(da->array,dyv_ptr,da->array_size);
    da -> array = new_array;
    da -> array_size = new_size;
  }
  da->array[da->size] = dv;
  da->size += 1;
}

void add_to_dyv_array(dyv_array *da, const dyv *dv)
{
/*
     Assume dyv_array is previously of size n. After this it is of size
   n+1, and the n+1'th element is a COPY of dv.
*/
  add_to_dyv_array_no_copy(da,(dv==NULL)?NULL:mk_copy_dyv(dv));
}

dyv_array *mk_const_dyv_array(dyv *base_vec, int size){
  int i;
  dyv_array *result = mk_empty_dyv_array();

  for (i=0; i<size; i++) {
    add_to_dyv_array(result, base_vec);
  }
  return result;
}

dyv_array *mk_rectangular_dyv_array( int numdyvs, int dyvlen)
{
  int i;
  dyv_array *dva;
  dyv*       temp;

  dva  = mk_dyv_array( numdyvs);
  temp = mk_dyv( dyvlen);

  for (i=0; i<numdyvs; ++i) dyv_array_set( dva, i, temp);

  free_dyv(temp);

  return dva;
}

dyv_array *mk_constant_rectangular_dyv_array( int numdyvs, int dyvlen, double val)
{
  int i;
  dyv_array *dva;
  dyv*       temp;

  dva  = mk_dyv_array( numdyvs);
  temp = mk_constant_dyv( dyvlen, val);

  for (i=0; i<numdyvs; ++i) dyv_array_set( dva, i, temp);

  free_dyv(temp);

  return dva;
}

dyv_array *mk_zero_dyv_array(int size){
  dyv *temp_dyv = mk_dyv(0);
  dyv_array *result = mk_const_dyv_array(temp_dyv, size);
  free_dyv(temp_dyv);
  return result;
}

int dyv_array_size(const dyv_array *da)
{
  return(da->size);
}

dyv *safe_dyv_array_ref(const dyv_array *da,int idx)
/*
     Returns a pointer (not a copy) to the index'th element stored in
   the dyv_array. Error if index < 0 or index >= size
*/
{
  dyv *result;
  if ( idx < 0 || idx >= dyv_array_size(da) )
  {
    result = NULL;
    my_error("dyv_array_ref");
  }
  else
    result = da->array[idx];
  return(result);
}

void dyv_array_set(dyv_array *dva, int idx, const dyv *dv)
{
  if ((idx < 0) || (dva == NULL) || (idx >= dva->size))
        my_error("dyv_array_set: called with incompatible arguments");
  if (dva->array[idx] != NULL)
        free_dyv(dva->array[idx]);
  dva->array[idx] = (dv == NULL) ? NULL : mk_copy_dyv(dv);
}

/* Use this function at your peril! */
void dyv_array_set_no_copy( dyv_array *dva, int idx, dyv *dv)
{
  if ((idx < 0) || (dva == NULL) || (idx >= dva->size)) {
    my_errorf( "dyv_array_set_no_copy: index %d is out of bounds [0,%d]\n",
	       idx, dva->size);
  }
  if (dva->array[idx] != NULL) free_dyv(dva->array[idx]);
  dva->array[idx] = dv;
  return;
}

void fprintf_dyv_array(FILE *s, const char *m1, const dyv_array *da, const char *m2)
{
  if ( dyv_array_size(da) == 0 )
    fprintf(s,"%s = <dyv_array with zero entries>%s",m1,m2);
  else
  {
    int i;
    for ( i = 0 ; i < dyv_array_size(da) ; i++ )
    {
      char buff[100];
      sprintf(buff,"%s[%2d]",m1,i);
      fprintf_dyv(s,buff,dyv_array_ref(da,i),m2);
    }
  }
}

void fprintf_oneline_dyv_array( FILE *f, const char *pre, const dyv_array *da,
				const char *post)
{
  int size, numdigits, buffsize, i;
  char *buff;
  dyv *dv;

  size = dyv_array_size( da);
  if (size == 0) {
    fprintf( f, "%s = <dyv_array with zero entries>%s", pre, post);
  }
  else {
    /* Compute number of digits for indices. */
    if (size == 1) numdigits = 1;
    else numdigits = ( (int)log10( size-1)) + 1;

    /* Create buffer. */
    buffsize  = numdigits + strlen( pre) + strlen("[]") + 1;
    buff      = AM_MALLOC_ARRAY( char, buffsize);

    /* Print stuff. */
    for (i=0; i<size; i++) {
      sprintf( buff, "%s[%*d]", pre, numdigits, i);
      dv = dyv_array_ref( da, i);
      fprintf_oneline_dyv( f, buff, dv, post);
    }

    AM_FREE_ARRAY( buff, char, buffsize);
  }
}

void free_dyv_array(dyv_array *da)
{
  int i;
  for ( i = 0 ; i < dyv_array_size(da) ; i++ )
    if ( da->array[i] != NULL )
      free_dyv(da->array[i]);
  AM_FREE_ARRAY(da->array,dyv_ptr,da->array_size);
  AM_FREE(da,dyv_array);
}

/* BSA: added 2005-08-28 */
void free_dyv_array_container(dyv_array *da)
{
  AM_FREE_ARRAY(da->array,dyv_ptr,da->array_size);
  AM_FREE(da,dyv_array);
}

dyv_array *mk_copy_dyv_array(const dyv_array *da)
{
  dyv_array *new_ar = mk_empty_dyv_array();
  int i;

  for ( i = 0 ; i < dyv_array_size(da) ; i++ )
    add_to_dyv_array(new_ar,dyv_array_ref(da,i));

  return(new_ar);
}

void dyv_array_remove(dyv_array *dva,int idx)
{
  int i;
  dyv *dv = dyv_array_ref(dva,idx);
  if ( dv != NULL ) free_dyv(dv);
  for ( i = idx ; i < dva->size-1 ; i++ )
    dva->array[i] = dva->array[i+1];
  dva->array[dva->size-1] = NULL;
  dva->size -= 1;
}

void dyv_array_remove_no_free(dyv_array *dva,int idx)
{
  int i;
  for ( i = idx ; i < dva->size-1 ; i++ )
    dva->array[i] = dva->array[i+1];
  dva->array[dva->size-1] = NULL;
  dva->size -= 1;
}

dyv_array *mk_array_of_zero_length_dyvs(int size)
{
  dyv_array *dva = mk_empty_dyv_array();
  dyv *dv = mk_dyv(0);
  int i;

  for (i = 0; i < size; i++)
        add_to_dyv_array(dva, dv);
  free_dyv(dv);
  return(dva);
}

dyv_array *mk_array_of_null_dyvs(int size)
{
  dyv_array *dva = mk_empty_dyv_array();
  int i;

  for (i = 0; i < size; i++)
    add_to_dyv_array(dva,NULL);

  return(dva);
}

dyv_array *mk_dyv_array( int size)
{
  int i;
  dyv_array *da;
 
  da = AM_MALLOC( dyv_array);
  da->size = size;
  da->array_size = size;
  da->array = AM_MALLOC_ARRAY( dyv_ptr, size);

  for (i=0; i<size; ++i) da->array[i] = NULL;

  return(da);
}

dyv_array *mk_dyv_array_subset( dyv_array *da, ivec *indices)
{
  int dasize, i, idx;
  dyv *dv;
  dyv_array *subda;

  dasize = ivec_size( indices);
  subda = mk_dyv_array( dasize);
  for (i=0; i<dasize; ++i) {
    idx = ivec_ref( indices, i);
    dv = dyv_array_ref( da, idx); /* Get from da[index]. */
    dyv_array_set( subda, i, dv);   /* Store in subda[i].  */
  }

  return subda;
}

/* rows may be NULL dneoting "use all rows" */
dyv *mk_sum_of_dyv_array(const dyv_array *da)
{
  const int num_rows = dyv_array_size(da);
  dyv *sum = mk_zero_dyv(dyv_size(dyv_array_ref(da, 0)));
  int j;
  for ( j = 0 ; j < num_rows ; j++ )
  {
    const dyv *this_row = dyv_array_ref(da, j);
    dyv_plus(sum, this_row, sum);
  }
  return(sum);
}

void pdyv_array(dyv_array *da)
{
  fprintf_dyv_array(stdout,"da",da,"\n");
}

bool dyv_array_equal(dyv_array *da1,dyv_array *da2)
{
  int size = dyv_array_size(da1);
  bool result = size == dyv_array_size(da2);
  int i;
  for ( i = 0 ; result && i < size ; i++ )
    result = dyv_equal(dyv_array_ref(da1,i),dyv_array_ref(da2,i));
  return result;
}

ivec *mk_dyv_lengths(dyv_array *da)
{
  int size = dyv_array_size(da);
  int i;
  ivec *lengths = mk_ivec(size);
  for ( i = 0 ; i < size ; i++ )
    ivec_set(lengths,i,dyv_size(dyv_array_ref(da,i)));
  return lengths;
}

int dyv_array_num_dims(dyv_array *da)
{
  ivec *lengths = mk_dyv_lengths(da);
  int result;
  my_assert(dyv_array_size(da) > 0);
  my_assert(ivec_min(lengths) == ivec_max(lengths));
  result = ivec_ref(lengths,0);
  free_ivec(lengths);
  return result;
}

dyv *mk_dyv_array_sum(dyv_array *sps,ivec *rows)
{
  int num_rows = ivec_size(rows);
  int num_dims = dyv_array_num_dims(sps);

  dyv *result = NULL;
  if ( num_rows <= 0 )
    result = mk_zero_dyv(num_dims);
  else
  {
    int i = 0;
    int row0 = ivec_ref(rows,0);
    result = mk_copy_dyv(dyv_array_ref(sps,row0));
    for ( i = 1 ; i < num_rows ; i++ )
    {
      int row = ivec_ref(rows,i);
      dyv *sp = dyv_array_ref(sps,row);
      dyv *increment = mk_dyv_plus(result,sp);
      free_dyv(result);
      result = increment;
    }
  }
  return result;
}

/* PRE: All the vectors mentioned in "rows" must have length
   equal to dyv_length.

   If "rows" is empty, returns a vector of length "dyv_length" 
   containing all zeroes. */
/* "rows" may be NULL, meaning "use all rows" */
dyv *mk_sum_of_dyv_array_rows(dyv_array *da,ivec *rows,
			      int dyv_length)
{
  int num_rows = (rows==NULL) ? dyv_array_size(da) : ivec_size(rows);
  dyv *result = mk_zero_dyv(dyv_length);
  int i;

  for ( i = 0 ; i < num_rows ; i++ )
  {
    int row = (rows==NULL) ? i : ivec_ref(rows,i);
    dyv *x = dyv_array_ref(da,row);
    dyv_plus(result,x,result);
  }
  
  return result;
}

/* Returns a dyv_array of size "num_dyvs" in which the i'th
   element (forall 0 <= i < num_dyvs) is a vector of
   size "dyv_size" in which each element is 0.0 */
dyv_array *mk_dyv_array_of_zeroed_dyvs(int num_dyvs,int dyv_size)
{
  dyv *zero = mk_zero_dyv(dyv_size);
  dyv_array *result = mk_empty_dyv_array();
  int i;
  for ( i = 0 ; i < num_dyvs ; i++ )
    add_to_dyv_array(result,zero);
  free_dyv(zero);
  return result;
}
  
void fprint_dyv_array_csv(FILE *s,dyv_array *x)
{
  int i;
  for ( i = 0 ; i < dyv_array_size(x) ; i++ )
    fprint_dyv_csv(s,dyv_array_ref(x,i));
}

string_array *mk_string_array_from_dyv(const dyv *d)
{
  string_array *sa = mk_string_array(dyv_size(d));
  int i;
  for ( i = 0 ; i < dyv_size(d) ; i++ )
  {
    char *s = mk_printf("%g",dyv_ref(d,i));
    string_array_set(sa,i,s);
    free_string(s);
  }
  return(sa);
}

/* Makes a string of numbers, each separated by whitespace.
   No quotes or anything. Just numbers. */
/* Changed by AWM March 16th 1997 to turn an empty
   dyv into a 1-element string: "empty" */
char *mk_string_from_dyv(dyv *d)
{
  string_array *sa = mk_string_array_from_dyv(d);
  char *s = mk_string_from_string_array(sa);
  free_string_array(sa);
  return(s);
}

/* 
   This function tries to create and return a dyv containing the specified
   numbers in the string_array sa.  In the simple case, format == NULL and
   we return a dyv with the same number of elements as sa, the ith element
   being the numeric value of the ith string in sa.  If format is non-NULL,
   then it should have the same length as sa, and contain only the characters
   'i' and '-'.  An 'i' signifies that the corresponding string in sa 
   should be included in the result dyv; a '-' specifies that it should be
   skipped.  Thus for a non-NULL format, the jth element of the result
   dyv is the numeric value of the jth non-ignored string in sa.

   If there is an error (because a substring didn't represent a number,
   or the format's length didn't equal the size of sa), this function
   returns NULL and sets err_mess to a newly created string describing the
   problem....  If the function succeeds, it will always return a
   non-NULL dyv (though the dyv could, in theory, have size 0) and will
   set err_mess to NULL.

   Added by AWM March 16th 1997: If the string consists of the single token "empty"
   "zero" or "none" in any case, a zero-length dyv is returned without complaint.
*/
dyv *mk_dyv_from_string_array_with_error_message(const string_array *sa,
                                                 const char *format,
                                                 char **err_mess)
{
  bool ok = TRUE;
  int sa_size = string_array_size(sa);
  dyv *result = NULL;
  bool empty;  
  *err_mess = NULL;

  if ( sa_size == 0 )
    empty = TRUE;
  else if ( sa_size == 1 )
  {
    char *f = string_array_ref(sa,0);
    empty = caseless_eq_string(f,"empty") || caseless_eq_string(f,"zero") || caseless_eq_string(f,"none");
  }
  else
    empty = FALSE;

  if ( format != NULL && !empty && (((int) strlen(format)) != sa_size))
  {
    char buff[200];

    ok = FALSE;
    sprintf(buff, "Expected %d fields, not %d.",
            (int) strlen(format),
            sa_size);
    *err_mess = mk_copy_string(buff);
  }
  else if ( empty )
    result = mk_dyv(0);
  else
  {
    int i;     /* Indexes string_array */
    int j = 0; /* indexes result */
    int result_size;

    if ( format == NULL )
      result_size = sa_size;
    else
      result_size = num_of_char_in_string(format, 'i');

    result = mk_dyv(result_size);

    for ( i = 0 ; ok && i < sa_size ; i++ )
    {
      bool use_me = (format == NULL || format[i] == 'i');
      if ( use_me )
      {
        char *s = string_array_ref(sa,i);
        if ( !is_a_number(s) )
        {
          char buff[200];

          ok = FALSE;
          sprintf(buff, "Expected a number in column %d, not '%s'.",
                  i + 1,
                  s);
          *err_mess = mk_copy_string(buff);
        }
        else
        {
          dyv_set(result,j,atof(s));
          j += 1;
        }
      }
    }

    if ( ok && j != result_size )
      my_error("Assert j must be result_size");
  }

  if ( !ok && result != NULL )
  {
    free_dyv(result);
    result = NULL;
  }

  return(result);
}

/* This function is identical to mk_dyv_from_string_array_with_error_message
   except that it doesn't create an error message.  */
dyv *mk_dyv_from_string_array(const string_array *sa, const char *format)
{
  char *tem_str;
  dyv *res = mk_dyv_from_string_array_with_error_message(sa, format, &tem_str);

  if (tem_str != NULL) free_string(tem_str);
  return(res);
}

/* 
   This function tries to create and return a dyv by reading the specified
   numbers from string.  Each comma or section of whitespace in the string
   is interpreted as a separator between two fields of the string.  In the
   simple case, format == NULL and this function assumes that string 
   simply contains a series of numbers (separated by commas or whitespace),
   and returns a dyv whose ith element is the ith such number.  
   
   If format is non-NULL, then it should have the same length as there are
   fields in string, and in addition format should contain only the characters
   'i' and '-'.  An 'i' signifies that the corresponding field in string
   should be included in the result dyv; a '-' specifies that it should be
   skipped.  Thus for a non-NULL format, the jth element of the result
   dyv is the numeric value of the jth non-ignored field in string.  (In
   theory, the ignored fields of string could contain non-numbers.)

   If there is an error (because a non-number is found in string where a 
   number was expected, or because the format's length didn't equal the 
   number of fields in the string), this function returns NULL and sets
   err_mess to a newly created string describing the problem.  If the 
   function succeeds, it will always return a non-NULL dyv (though the dyv 
   could, in theory, have size 0), and will set err_mess to NULL.

   Added by AWM March 16th 1997: If the string consists of the single token "empty"
   "zero" or "none" in any case, a zero-length dyv is returned without complaint.

   If the string starts/ends with a single or double quote (maybe because
   some other thing like string_from_command needed it to bind
   space-separated values together), they will be removed so the number
   reader doesn't choke on them.
*/
dyv *mk_dyv_from_string_with_error_message(const char *string,
                                           const char *format,
                                           char **err_mess)
{
  char *lstring = mk_copy_string(string);
  int len = strlen(lstring);
  string_array *sa = NULL;
  dyv *res = NULL;

  if ((lstring[0] == '\"') || (lstring[0] == '\''))     lstring[0] = ' ';
  if ((lstring[len-1]=='\"') || (lstring[len-1]=='\'')) lstring[len-1] = ' ';

  sa = mk_broken_string_using_seppers(lstring,",");
  res = mk_dyv_from_string_array_with_error_message(sa, format, err_mess);

  free_string(lstring);
  free_string_array(sa);
  return(res);
}

/* This function is identical to mk_dyv_from_string_with_error_message
   except that it doesn't create an error message.  */
dyv *mk_dyv_from_string(const char *string, const char *format)
{
  char *tem_str;
  dyv *res = mk_dyv_from_string_with_error_message(string, format, &tem_str);

  if (tem_str != NULL) free_string(tem_str);
  return(res);
}

/* Prints the contents of string_array cleverly much in the same way
   that "short ls" in unix displays filenames. It's cleverly designed so that when
   printed it uses less than "max_chars" characters per line. (it auto-chooses
   and sizes the columns) */
void display_names(FILE *s,string_array *names)
{
  if ( string_array_size(names) <= 0 )
    fprintf(s,"<empty set>\n");
  else
  {
    string_matrix *sm = mk_ls_style_string_matrix(names,78);
    render_string_matrix(s,"",sm);
    free_string_matrix(sm);
  }
}
