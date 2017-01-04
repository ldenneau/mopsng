/* *
   File:        amiv.c
   Author:      Andrew W. Moore
   Created:     Sat Apr  8 18:48:26 EDT 1995
   Updated:     29 March 97
   Description: Integer and Boolean Dynamic vectors

   Copyright 1997, Schenley Park Research

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

#include "amiv.h"
#include "am_string.h"
#include "am_string_array.h"
#include "amma.h"
#include "amdmex.h"

#define IVEC_CODE 20541

static int Ivecs_mallocked = 0;
static int Ivecs_freed = 0;


#ifdef AMFAST

#define NOTHING_TO_DO

#define check_ivec_code(iv,name) NOTHING_TO_DO

#else

static void check_ivec_code(const ivec *iv,char *name);

void check_ivec_code(const ivec *iv,char *name)
{
  if ( iv == NULL )
    my_errorf("NULL ivec passed in operation %s\n",name);
  if ( iv->ivec_code != IVEC_CODE )
  {
    my_errorf("Attempt to access a non-allocated Integer Vector\n"
              "This is in the operation %s\n",name);
  }
  if ( iv->array_size < iv->size )
    my_error("check_ivec_code: array_size and size muddled");
}

#endif

#ifdef AMFAST

#define check_ivec_access(iv, i, name) NOTHING_TO_DO

#else

static void check_ivec_access(const ivec *iv, int i, char *name);
void check_ivec_access(const ivec *iv, int i, char *name)
{
  check_ivec_code(iv, name);

  if ( i < 0 || i >= iv->size )
  {
    fprintf(stderr,"In operation \"%s\"\n",name);
    fprintf(stderr,"the ivec (int dynamic vector) has size = %d\n",iv->size);
    fprintf(stderr,"You tried to use index i=%d\n",i);
    fprintf(stderr,"Here is the ivec that was involved:\n");
    fprintf_ivec(stderr,"ivv",iv,"\n");
    my_error("check_ivec_access");
  }
  if ( iv->array_size < iv->size )
    my_error("check_ivec_code: array_size and size muddled");
}

#endif


static void assert_ivec_shape(ivec *iv,int size,char *name);
void assert_ivec_shape(ivec *iv,int size,char *name)
{
  check_ivec_code(iv,name);

  if ( size != iv->size )
  {
    fprintf(stderr,"In operation \"%s\"\n",name);
    fprintf(stderr,"the ivec (int dynamic vector) has size = %d\n", iv->size);
    fprintf(stderr,"But should have been predefined with the shape:\n");
    fprintf(stderr,"size = %d\n",size);
    my_error("assert_ivec_shape");
  }
}

ivec *mk_ivec(int size)
{
  ivec *result = AM_MALLOC(ivec);
  if ( size < 0 ) my_error("mk_ivec : size < 0 illegal");
  result -> ivec_code = IVEC_CODE;
  result -> array_size = size;
  result -> size = size;
  result -> iarr = am_malloc_ints(size);
  Ivecs_mallocked += 1;
  return(result);
}

/* Acts as though you created a ivec of size 0,        */
/* but actually allocates an array of size "capacity". */
/* This is very useful if you want to use add_to_ivec  */
/* and happen to have a reasonable upper bound to the  */
/* number of elements you want to add.                 */
ivec *mk_empty_ivec(int capacity) {
  ivec *result = AM_MALLOC(ivec);
  if ( capacity < 0 ) my_error("mk_empty_ivec : capacity < 0 illegal");
  result -> ivec_code = IVEC_CODE;
  result -> array_size = capacity;
  result -> size = 0;
  result -> iarr = am_malloc_ints(capacity);
  Ivecs_mallocked += 1;
  return(result);
}


ivec *mk_ivec_x( int size, ...)
{
  /* Warning: no type checking can be done by the compiler.  You *must*
     send the values as integers for this to work correctly. */
  int i, ival;
  va_list argptr;
  ivec *iv;
  
  iv = mk_ivec(size);

  va_start( argptr, size);
  for (i=0; i<size; ++i) {
    ival = va_arg( argptr, int);
    ivec_set( iv, i, ival);
  }
  va_end(argptr);

  return iv;
}

void free_ivec(ivec *iv)
{
  check_ivec_code(iv,"free_ivec");
  iv -> ivec_code = 7777;

  am_free_ints(iv->iarr,iv->array_size);
  AM_FREE(iv,ivec);

  Ivecs_freed += 1;
}

void ivec_destructive_resize(ivec* this, int size)
{
  am_free_ints(this->iarr, this->array_size);
  this->size=size;
  this->array_size=size;
  this->iarr = am_malloc_ints(size);
}

void fprintf_ivec(FILE *s,char *m1, const ivec *iv,char *m2)
{
  if ( iv == NULL )
    fprintf(s,"%s = (ivec *)NULL%s",m1,m2);
  else
  {
    char buff[256];
    check_ivec_code(iv,"fprintf_ivec");

    /* Modified so that we don't cause a seg fault when m1 is bigger than
       the buffer - WKW*/
    if( strlen(m1) <= 255 )
      {
	strncpy(buff,m1,strlen(m1)+1);
      }
    else
      {
	strncpy(buff,m1,sizeof(buff));
	buff[255] = '\0';
      }
    fprintf_ints(s,buff,iv->iarr,iv->size,m2);
  }
}

/* save the ivec on one line of a file in a format easy to load.
   the comment is just to make the file a small bit human readable.
   It may be NULL.
*/
void fprintf_ivec_for_load(FILE *s, const ivec *iv, char *comment)
{
  int i;

  if (iv)
  {
    fprintf(s,"%d ",ivec_size(iv));
    for (i=0;i<ivec_size(iv);i++) fprintf(s,"%d ",ivec_ref(iv,i));
  }
  else fprintf(s,"NULL");

  if (comment) fprintf(s,"%s\n",comment);
  else         fprintf(s,"\n");
}
  
/* save the ivec to a file, in comma-separated value (csv) form
*/
void fprintf_ivec_csv(FILE *s,ivec *x)
{
  int i;
  int size = ivec_size(x);

  for ( i = 0 ; i < size ; i++ )
    fprintf(s,"%d%s",ivec_ref(x,i),(i==size-1)?"\n":",");
}

/* Warning this function only works if the next line of fp contains an ivec
   stored using the above function
*/
ivec *mk_ivec_from_file(FILE *fp, char **r_errmess)
{
  bool ok = TRUE;
  string_array *sa = mk_string_array_from_line(fp);
  int size,i;
  ivec *res = NULL;

  if (string_array_size(sa) < 1)
    add_to_error_message(r_errmess,"Expected an ivec on a line with no non-whitespace\n");
  else
  {
    size = int_from_string(string_array_ref(sa,0),&ok);
    if ((!ok) && (!eq_string(string_array_ref(sa,0),"NULL")))
      add_to_error_message(r_errmess,"Expected an integer or NULL as the first word in a saved ivec\n");
    else
    {
      if (ok) res = mk_ivec(size);
      for (i=0;(i<size)&&(ok);i++)
      {
	ivec_set(res,i,int_from_string(string_array_ref(sa,i+1),&ok));
	if (!ok) add_to_error_message(r_errmess,"Error reading one of the elements of an ivec -- it is not parseable as an int\n");
      }
    }
  }
  if (sa) free_string_array(sa);

  return res;
}

void privec(ivec *iv)
{
  fprintf_ivec(stdout,"iv",iv,"\n");
}

void ivec_update_sizeof(ivec *iv, auton_sf *x)
{
  if ( iv != NULL )
  {
    add_to_auton_sf(x,(unsigned long)sizeof(ivec));
    add_to_auton_sf(x,(unsigned long)(iv->array_size)*sizeof(int));
  }
}

int safe_ivec_ref(const ivec *iv, int i)
{
  check_ivec_access(iv,i,"ivec_ref");
  return(iv->iarr[i]);
}

void safe_ivec_set(ivec *iv,int i,int value)
{
  check_ivec_access(iv,i,"ivec_set");
  iv->iarr[i] = value;
}

void safe_ivec_increment(ivec *iv,int i,int value)
{
  check_ivec_access(iv,i,"ivec_increment");
  iv->iarr[i] += value;
}

void ivec_plus(ivec *d_1, ivec *d_2, ivec *r_d)
{
  int i;

  if ( d_1 -> size != d_2 -> size || d_1 -> size != r_d -> size)
  {
    fprintf_ivec(stderr,"d_1",d_1,"\n");
    fprintf_ivec(stderr,"d_2",d_2,"\n");
    fprintf_ivec(stderr,"r_d",r_d,"\n");
    my_error("ivec_plus: ivecs have different shape");
  }

  for ( i = 0 ; i < r_d -> size ; i++ )
    r_d -> iarr[i] = d_1->iarr[i] + d_2 -> iarr[i];
}

ivec *mk_ivec_plus(ivec *a,ivec *b)
{
  ivec *result = mk_ivec(a->size);
  ivec_plus(a,b,result);
  return(result);
}

void ivec_subtract(ivec *d_1,ivec *d_2,ivec *r_d)
{
  int i;

  if ( d_1 -> size != d_2 -> size || d_1 -> size != r_d -> size)
  {
    fprintf_ivec(stderr,"d_1",d_1,"\n");
    fprintf_ivec(stderr,"d_2",d_2,"\n");
    fprintf_ivec(stderr,"r_d",r_d,"\n");
    my_error("ivec_subtract: ivecs have different shape");
  }

  for ( i = 0 ; i < r_d -> size ; i++ )
    r_d -> iarr[i] = d_1->iarr[i] - d_2 -> iarr[i];
}

ivec *mk_ivec_subtract(ivec *a,ivec *b)
{
  ivec *result; 
  result = mk_ivec(a->size);
  ivec_subtract(a,b,result);
  return(result);
}

int safe_ivec_size(const ivec *iv)
{
  check_ivec_code(iv, "ivec_size");
  return(iv->size);
}

void shift_ivec( ivec *iv, int shift)
{
  int i;
  for (i=0; i<ivec_size(iv); ++i) ivec_increment(iv, i, shift);
  return;
}

ivec *mk_shifted_ivec( ivec *iv, int shift)
{
  ivec *niv;
  niv = mk_copy_ivec( iv);
  shift_ivec( niv, shift);
  return niv;
}

void copy_iarr_to_ivec(int *iarr,int size,ivec *r_iv)
{
  assert_ivec_shape(r_iv,size,"copy_iarr_to_ivec");
  copy_ints(iarr,r_iv->iarr,size);
}

ivec *mk_ivec_from_iarr(int *iarr,int size)
{
  ivec *result = mk_ivec(size);
  copy_iarr_to_ivec(iarr,size,result);
  return(result);
}

void copy_ivec_to_iarr(const ivec *iv, int *iarr)
{
  check_ivec_code(iv,"copy_ivec_to_iarr");
  copy_ints(iv->iarr,iarr,iv->size);
}
  
int *mk_iarr_from_ivec(const ivec *iv)
{
  int *result;
  check_ivec_code(iv,"make_copy_iarr");
  result = am_malloc_ints(iv->size);
  copy_ivec_to_iarr(iv,result);
  return(result);
}

/* Makes an ivec of size end - start:
   { start , start+1 , .... end-2 , end-1 } */
ivec *mk_sequence_ivec(int start_value,int end_value)
{
  int size = end_value - start_value;
  ivec *iv = mk_ivec(size);
  int i;
  for ( i = 0 ; i < size ; i++ )
    ivec_set(iv,i,start_value+i);
  return iv;
}

/*
   Allocates and returns an ivec of size size in which ivec[i] = i
   ivec[0] = 0
   ivec[1] = 1
    .
    .
   ivec[size-1] = size-1
*/
ivec *mk_identity_ivec(int size)
{
  return mk_sequence_ivec(0,size);
}

void shuffle_ivec(ivec *iv)
/*
   A random permutation of iv is returned
*/
{
  int size = ivec_size(iv);
  int i;
  for ( i = 0 ; i < size-1 ; i++ )
  {
    int j = int_random(size - i);
    if ( j > 0 )
    {
      int swap_me_1 = ivec_ref(iv,i);
      int swap_me_2 = ivec_ref(iv,i+j);
      ivec_set(iv,i,swap_me_2);
      ivec_set(iv,i+j,swap_me_1);
    }
  }
}

void constant_ivec(ivec *iv,int value)
{
  int i;
  for ( i = 0 ; i < ivec_size(iv) ; i++ )
    ivec_set(iv,i,value);
}

ivec *mk_constant_ivec(int size,int value)
{
  ivec *iv = mk_ivec(size);
  constant_ivec(iv,value);
  return(iv);
}

void zero_ivec(ivec *iv)
{
  memset(iv->iarr, 0, iv->size * sizeof (int));
}

ivec *mk_zero_ivec(int size)
{
  ivec *iv = mk_ivec(size);
  zero_ivec(iv);
  return iv;
}

void ivec_scalar_plus(const ivec *iv, int addme, ivec *riv)
{
  int i;
  for ( i = 0 ; i < ivec_size(iv) ; i++ )
    ivec_set(riv,i,ivec_ref(iv,i) + addme);
}

ivec *mk_ivec_scalar_plus(ivec *iv,int delta)
{
  ivec *result = mk_ivec(ivec_size(iv));
  ivec_scalar_plus(iv,delta,result);
  return(result);
}

void ivec_scalar_mult(const ivec *iv,int scale,ivec *riv)
{
  int i;
  for ( i = 0 ; i < ivec_size(iv) ; i++ )
    ivec_set(riv,i,ivec_ref(iv,i) * scale);
}

ivec *mk_ivec_scalar_mult(ivec *iv,int scale)
{
  ivec *result = mk_ivec(ivec_size(iv));
  ivec_scalar_mult(iv,scale,result);
  return(result);
}

void copy_ivec(const ivec *iv,ivec *r_iv)
{
  if ( iv -> size != r_iv -> size )
  {
    fprintf_ivec(stderr,"iv",iv,"\n");
    fprintf_ivec(stderr,"r_iv",r_iv,"\n");
    my_error("copy_ivec: ivecs have different shape");
  }
  memcpy(r_iv->iarr, iv->iarr, iv->size * sizeof (int));
}

ivec *mk_copy_ivec(const ivec *iv)
{
  ivec *result = mk_ivec(ivec_size(iv));
  copy_ivec(iv,result);
  return(result);
}

void copy_ivec_subset(ivec *iv, int start, int size, ivec *r_iv)
{
  int i;
  for (i = start; i < start + size; i++)
    ivec_set(r_iv, i - start, ivec_ref(iv, i));
}

ivec *mk_copy_ivec_subset(ivec *iv, int start, int size)
{
  ivec *result = mk_ivec(size);
  copy_ivec_subset(iv, start, size, result);
  return (result);
}

int num_of_given_value( const ivec *iv,int value)
{
  int i;
  int result = 0;
  for ( i = 0 ; i < ivec_size(iv) ; i++ )
    if ( ivec_ref(iv,i) == value ) result += 1;

  return(result);
}

int num_zero_entries( const ivec *iv)
{
  return(num_of_given_value(iv,0));
}

int num_nonzero_entries( const ivec *iv)
{
  return(ivec_size(iv) - num_zero_entries(iv));
}

int ivec_arg_extreme(const ivec *iv,bool lowest)
{
  int extreme_index;
  int extremest_value;
  int i;

  if ( ivec_size(iv) <= 0 )
    my_error("Can't find min or max of empty ivec");
  
  extreme_index = 0;
  extremest_value = ivec_ref(iv,0);

  for ( i = 1 ; i < ivec_size(iv) ; i++ )
  {
    int v = ivec_ref(iv,i);
    if ( (lowest && v < extremest_value) || (!lowest && v > extremest_value) ) 
    {
      extremest_value = v;
      extreme_index = i;
    }
  }

  return(extreme_index);
}

int ivec_extreme(const ivec *iv,bool lowest)
{
  return ivec_ref(iv,ivec_arg_extreme(iv,lowest));
}

int ivec_min(const ivec *iv)
{
  return ivec_ref(iv,ivec_arg_extreme(iv,TRUE));
}

int ivec_max(const ivec *iv)
{
  return ivec_ref(iv,ivec_arg_extreme(iv,FALSE));
}

int ivec_argmin(const ivec *iv)
{
  return ivec_arg_extreme(iv,TRUE);
}

int ivec_argmax(const ivec *iv)
{
  return ivec_arg_extreme(iv,FALSE);
}

/* Sensible if args are NULL. False if different size */
bool ivec_equal(const ivec *x1,const ivec *x2)
{
  bool result = TRUE;

  if ( EQ_PTR(x1,x2) )
    result = TRUE;
  else if ( x1 == NULL || x2 == NULL )
    result = FALSE;
  else if ( ivec_size(x1) != ivec_size(x2) )
    result = FALSE;
  else
  {
    result = !memcmp(x1->iarr, x2->iarr, x1->size * sizeof (int));
  }
  return(result);
}

/* Sensible if args are NULL. */
int ivec_nonmatch(const ivec *x1,const ivec *x2)
{
  int ret;
  const int sx1 = x1 ? ivec_size(x1) : 0;
  const int sx2 = x2 ? ivec_size(x2) : 0;

  if ( EQ_PTR(x1,x2) )
	return 0;
  if(sx1 != sx2 || !sx1 || !sx2)
	return sx1 > sx2 ? sx1 : sx2;

  {
    int i;
    for ( i = 0, ret = 0 ; i < ivec_size(x1) ; i++ ) 
      ret += ivec_ref(x1,i) != ivec_ref(x2,i);
  }
  return ret;
}

ivec *mk_ivec_1(int x0)
{
  ivec *result = mk_ivec(1);
  ivec_set(result,0,x0);
  return(result);
}

ivec *mk_ivec_2(int x0,int x1)
{
  ivec *result = mk_ivec(2);
  ivec_set(result,0,x0);
  ivec_set(result,1,x1);
  return(result);
}

ivec *mk_ivec_3(int x0,int x1,int x2)
{
  ivec *result = mk_ivec(3);
  ivec_set(result,0,x0);
  ivec_set(result,1,x1);
  ivec_set(result,2,x2);
  return(result);
}

ivec *mk_ivec_4(int x0,int x1,int x2,int x3)
{
  ivec *result = mk_ivec(4);
  ivec_set(result,0,x0);
  ivec_set(result,1,x1);
  ivec_set(result,2,x2);
  ivec_set(result,3,x3);
  return(result);
}

ivec *mk_ivec_5(int x0,int x1,int x2,int x3,int x4)
{
  ivec *result = mk_ivec(5);
  ivec_set(result,0,x0);
  ivec_set(result,1,x1);
  ivec_set(result,2,x2);
  ivec_set(result,3,x3);
  ivec_set(result,4,x4);
  return(result);
}

ivec *mk_ivec_6(int x0,int x1,int x2,int x3,int x4,int x5)
{
  ivec *result = mk_ivec(6);
  ivec_set(result,0,x0);
  ivec_set(result,1,x1);
  ivec_set(result,2,x2);
  ivec_set(result,3,x3);
  ivec_set(result,4,x4);
  ivec_set(result,5,x5);
  return(result);
}

/**** Removal functions on ivecs ****/

/* Reduces the size of iv by one.
   ivec_ref(iv,index) disappears.
   Everything to the right of ivec_ref(iv,index) is copied one to the left.

   Formally: Let ivold be the ivec value beore calling this function
             Let ivnew be the ivec value after calling this function.

PRE: ivec_size(ivold) > 0
     0 <= index < ivec_size(ivold)

POST: ivec_size(ivnew) = ivec_size(ivold)-1
      for j = 0 , 1, 2 ... index-1  : 
         ivec_ref(ivnew,j) == ivec_ref(ivold,j)

      for j = index , index+1 , ... ivec_size(ivnew)-1:
         ivec_ref(ivnew,j) == ivec_ref(ivold,j+1)
*/
void ivec_remove(ivec *iv,int idx)
{

#ifndef AMFAST
  if ( ivec_size(iv) <= 0 ) my_error("ivec_remove: empty ivec");
  if ( idx < 0 || idx >= ivec_size(iv) )
    my_error("ivec_remove: bad index");
#endif /* #ifndef AMFAST */

  memmove(iv->iarr + idx, iv->iarr + idx + 1, (iv->size - idx - 1) * sizeof (int));
  iv -> size -= 1;
}

/* rewritten to just use ivec_remove_sivec which is very efficient.  Old
   implementation of this function would take forever if the ivecs involved
   (especially 'div') were large.  Even with the extra sort added, the 
   new implementation is tremendously faster than the old way.  jkomarek 06/06
*/
void ivec_remove_ivec(ivec *iv, ivec *div){
  ivec *sdiv = mk_sivec_from_ivec(div);
  ivec_remove_sivec(iv, sdiv);
  free_ivec(sdiv);
}

/* Shrinks d by one element by removing the rightmost element. 
   Example:

  Before: d == ( 3 1 4 1 5 )
    ivec_remove_last_element(d)
  After d == ( 3 1 4 1 )
*/
void ivec_remove_last_element(ivec *iv)
{
#ifndef AMFAST
  if (ivec_size(iv) <= 0) my_error("ivec_remove_last_elt: empty ivec");
#endif /* #ifndef AMFAST */

  iv->size -= 1;
}

void ivec_remove_last_n_elements(ivec *iv, int n)
{
  iv->size -= n;
  if (iv->size < 0) iv->size = 0;
}

/* Paul Komarek: Tue Oct 14 18:22:53 EDT 2003.  I replaced the old
   ivec_remove_value() at Andrew's request; I changed the old function's
   name to ivec_remove_value_old().  This version is O(n) instead of
   O(n^2).

   Addendum by Adam Goode: Mon Jan 30 12:22:54 EST 2006. Removed
   ivec_remove_value_old(). (Still available in version history,
   of course.)
*/
void ivec_remove_value(ivec *iv, int val) /* Removes all occurances of val */
{
  int ivsize, numfound, idx, i, ivval;

  /* This implementation is linear time (each element is read at most once,
     and set at most once).  The ivec implementation is (at worst)
     quadratic. */

  ivsize   = ivec_size( iv);
  numfound = 0;
  i        = 0;
  while (1) {
    /* compute src index */
    idx = i + numfound;
    if (idx >= ivsize) break;

    /* get src val and check */
    ivval = ivec_ref( iv, idx);
    if (ivval == val) {
      /* elem == val: don't copy */
      numfound += 1;
      continue;
    }
    else {
      /* elem != val: copy */
      ivec_set( iv, i, ivval);
      i += 1;
    }
  }

  iv->size -= numfound;
  return;
}

int ivec_sum(const ivec *iv)
{
  int result = 0;
  int i;
  for ( i = 0 ; i < ivec_size(iv) ; i++ ) {
    int next = ivec_ref(iv, i);
#ifndef AMFAST
    if (next > 0 && result > INT_MAX - next)
        my_error("integer overflow in ivec_sum");
    if (next < 0 && result < INT_MIN - next)
        my_error("integer underflow in ivec_sum");
#endif
    result += next;
  }
  return(result);
}

int ivec_product_orig(ivec *iv)
{
  if( ivec_size(iv) == 0 )
    {
      return 0;
    }
  else
    {
      int i,result = 1;
      for ( i = 0 ; i < ivec_size(iv) ; i++ )
	result *= ivec_ref(iv,i);
      return(result);   
    }
}

bool equal_ivecs(const ivec *iv1, const ivec *iv2)
{

  if ( ivec_size(iv1) != ivec_size(iv2) )
    my_error("in equal_ivecs, ivecs have different sizes");
    /* but why is this fatal?  Should just return FALSE? */

  return !memcmp(iv1->iarr, iv2->iarr, ivec_size(iv1) * sizeof (int));
}

void ivec_malloc_report(void)
{
  if ( Ivecs_mallocked > 0 )
  {
    fprintf(stdout,"# Integer Vectors  (datatype ivec) currently allocated: %d\n",
           Ivecs_mallocked - Ivecs_freed
          );
    if ( Ivecs_mallocked - Ivecs_freed != 0 )
    {
      fprintf(stdout,"#       Number of ivec allocations since program start:  %d\n",
             Ivecs_mallocked
            );
      fprintf(stdout,"#       Number of ivec frees       since program start:  %d\n#\n",
            Ivecs_freed
            );
    }
  }
}


void add_to_ivec(ivec *iv,int new_val)
{
  int size;

  check_ivec_code(iv,"add_to_ivec");

  if ( iv->array_size < 0 )
    my_error("An ivec should never have a negative array_size if\n"
	     "it was constructed using legal API calls");

  if ( iv->size > iv->array_size )
    my_error("An ivec should never have size greater than array_size if\n"
    "it was constructed using legal API calls");

  size = iv->size;

  if ( size == iv->array_size )
  {
    int new_array_size = 2 * size + 10;
    int *iarr_new = AM_MALLOC_ARRAY(int,new_array_size);

    memcpy(iarr_new, iv->iarr, size * sizeof (int));
    AM_FREE_ARRAY(iv->iarr,int,size);
    iv -> iarr = iarr_new;
    iv -> array_size = new_array_size;
  }
  iv->iarr[size] = new_val;
  iv -> size += 1;
}


void add_to_ivec_unique(ivec *iv,int val) {
  printf("AWM comments: You should probably be using sivecs\n");
  if (!is_in_ivec(iv, val)) add_to_ivec(iv, val);
}

/* Increases iv in length by 1 and shifts all elements
   with original index greater or equal to index one to the
   right and inserts val at index. */
void ivec_insert(ivec *iv,int idx,int val)
{
  add_to_ivec(iv,0);
  memmove(iv->iarr + idx + 1, iv->iarr + idx, (iv->size - idx - 1) * sizeof (int));
  ivec_set(iv,idx,val);
}

/* Creates a new ivec with the given element inserted.
   This is more efficient than copying and inserting. */
ivec *mk_ivec_insert( ivec *iv, int idx, int val)
{
  int newsize;
  ivec *result;

  newsize = ivec_size( iv) + 1;
  result = mk_ivec( newsize);

#ifndef AMFAST
  if (idx >= newsize || idx < 0) {
    my_errorf( "mk_ivec_insert: idx=%d is out-of-bounds [0,%d]",
               idx, newsize-1);
  }
#endif

/* The original ivec contains idx + (size - idx) total ints
   The new ivec contains idx + 1 + (size - idx) total ints */
  memcpy(result->iarr, iv->iarr, idx * sizeof (int));
  ivec_set( result, idx, val);
  memcpy(result->iarr + idx + 1, iv->iarr + idx, (iv->size - idx) * sizeof (int));
  
  return result;
}



/* Find least index i such that value = ivec_ref(iv,i).
  If not found, returns -1
*/
int find_index_in_ivec(const ivec *iv, int value)
{
  int i;

  for ( i = 0 ; i < ivec_size(iv) ; i++ )
    if (value == ivec_ref(iv,i)) 
      return i;

  return -1;
}

/* make and return an ivec containing the indices of iv that have value */
ivec *mk_indices_in_ivec(const ivec *iv, int value)
{
  int i;
  ivec *res = mk_ivec(0);
  for (i=0;i<ivec_size(iv);i++) if (ivec_ref(iv,i)==value) add_to_ivec(res,i);
  return res;
}

int find_index_in_ivec_hint(const ivec *iv, int value, int hint)
{
  int i;
  int sign = -1;
  int sz = ivec_size(iv);

  for (i=1; i<=sz; i++) {
	int disp = i/2;
	int idx = (hint + sign*disp);
    /* make sure idx is in bounds. Adding sz will take care of negative idx.*/
    idx = (idx + sz) % sz;
    if (value == ivec_ref(iv, idx)) 
      return idx;
    sign *= -1;
  }
  return -1;
}

/* Finds leftmost index such that siv[index] > val. If none
   such, returns ivec_size(siv) */
int index_in_sorted_ivec(ivec *siv,int val)
{
  int result = -1;
  int i;
  for ( i = 0 ; i < ivec_size(siv) && result < 0 ; i++ )
    if ( ivec_ref(siv,i) > val )
      result = i;
  if ( result < 0 ) result = ivec_size(siv);
  return(result);
}

int find_in_sorted_ivec(ivec *siv, int val)
{
  int begin = 0;
  int end = ivec_size(siv);
  int cur = (begin+end)/2;
  while (end > begin)
  {
    if (ivec_ref(siv,cur) == val) return cur;
    else if (ivec_ref(siv,cur) < val) begin = cur+1;
    else                              end = cur;
    cur = (begin+end)/2;
  }
  return -1;
}

void add_to_sorted_ivec(ivec *siv, int val)
{
  int i;
  add_to_ivec(siv, 0);
  for (i = ivec_size(siv) - 2; i >= 0 && val < ivec_ref(siv, i); i--)
    ivec_set(siv, i + 1, ivec_ref(siv, i));
  ivec_set(siv, i + 1, val);
}

bool is_in_ivec( const ivec *iv,int value)
{
  return(find_index_in_ivec(iv,value) >= 0);
}

/* Returns list of indices at which value appears. */
ivec *mk_is_in_ivec( ivec *iv, int value)
{
  int size, num, i;
  ivec *where, *r_iv;

  /* Find locations of value. */
  size = ivec_size( iv);
  where = mk_ivec( size);
  num = 0;
  for (i=0; i<size; ++i) {
    if (value == ivec_ref( iv, i)) {
      ivec_set( where, num, i);
      num += 1;
    }
  }

  /* Copy useful part of where into r_iv. */
  r_iv = mk_ivec( num);
  for (i=0; i<num; ++i) ivec_set( r_iv, i, ivec_ref( where, i));
  free_ivec( where);

  return r_iv;
}

/* return the number of elements appearing in both ivecs */
int count_ivec_overlap(ivec *a, ivec *b)
{
  int i;
  int result = 0;

  for (i=0;i<ivec_size(a);i++) if (is_in_ivec(b,ivec_ref(a,i))) result++;

  return result;
}

/* It is fine for iv and r_iv to occupy the same memory
   (see amar.c:sort_ints). */
void ivec_sort(const ivec *iv,ivec *r_iv)
{
  check_ivec_code(iv,"ivec_sort (1st arg)");
  assert_ivec_shape(r_iv,iv->size,"ivec_sort");
  copy_ivec(iv,r_iv);
  sort_ints(r_iv->iarr,ivec_size(r_iv),r_iv->iarr);
}

ivec *mk_ivec_sort(const ivec *iv)
{
  ivec *result = mk_ivec(ivec_size(iv));
  ivec_sort(iv,result);
  return result;
}

/*
 Creates a ivec of indices such that indices[i] is the origional
 location (in the unsorted iv) of the ith smallest value.
 Used when you want the location of the sorted values instead of 
 the sorted vector.
*/
ivec *mk_sorted_ivec_indices(const ivec *iv)
{
  ivec* indices;
  int size = ivec_size(iv);
  int i;
  int* iarr_ind;
  int* iarr;

  check_ivec_code(iv,"mk_sorted_ivec_indices");
  iarr = mk_iarr_from_ivec(iv);
  iarr_ind = am_malloc_ints(size);

  indices_sort_integers(iarr,size,iarr_ind);
  indices = mk_ivec(size);

  for(i=0;i<size;i++)
  {
    ivec_set(indices,i,iarr_ind[i]);
  }

  am_free_ints(iarr_ind,size);
  am_free_ints(iarr,size);

  return indices;
}

ivec *mk_abs_ivec(ivec *iv)
{
  int size = ivec_size(iv);
  ivec *res = mk_ivec(size);
  int i;
  for (i = 0; i < size; i++) {
    int val = ivec_ref(iv,i);
    ivec_set(res,i,abs(val));
  }
  return res;
}

/* all the elements of v1 not already in r_v2 will be added to it */
void ivec_union(ivec *v1, ivec *r_v2)
{
  int i;
  for (i=0;i<ivec_size(v1);i++)
  {
    int val = ivec_ref(v1,i);
    if (!is_in_ivec(r_v2,val)) add_to_ivec(r_v2,val);
  }
}

/*Could be made faster*/
ivec *mk_ivec_union(ivec *v1, ivec *v2)
{
  ivec *result = NULL;
  result = mk_copy_ivec(v1);
  ivec_union(v2,result);
  return (result);
}

/*Pre: ivecs are ordered
*/

ivec *mk_ivec_union_ordered(ivec *v1, ivec *v2)
{
  ivec *result = NULL;
  int i1 = 0, i2 = 0;
  int num1 = 0, num2 = 0;

  if(v1 == NULL || v2 == NULL)
    my_error("Null arg to 'mk_ivec_union()'");

  result = mk_ivec(0);

  while(1)
  {
    if(i1 >  ivec_size(v1) - 1)
    {
      if(i2 > ivec_size(v2) -1)
	break;
      else
      {
	num1 = INT_MAX;
	num2 = ivec_ref(v2, i2);
      }
    }
    else if(i2 > ivec_size(v2)  - 1)
    {
      num2 = INT_MAX;
      num1 = ivec_ref(v1, i1);
    }
    else 
    {
      num1 = ivec_ref(v1, i1);
      num2 = ivec_ref(v2, i2);
    }
    
    if(num1 < num2)
    {
      add_to_ivec(result, num1);
      i1++;
    }
    else if(num2 < num1)
    {
      add_to_ivec(result, num2);
      i2++;
    }
    else
    {
      add_to_ivec(result, num1);
      i1++;
      i2++;
    }
  }
  return(result);
}
  

/*Pre ivecs are ordered*/
ivec *mk_ivec_diff_ordered(ivec *v1, ivec *v2)
{
  int i = 0;
  int j = 0;
  ivec *result = NULL;
  int	num1, num2 = INT_MAX;

  if(v1 == NULL)
    my_error("Null arg #1 to 'set_diff()'");

  if(v2 == NULL)
    result = mk_copy_ivec(v1);
  else
  {
    result = mk_ivec(0);
    while( i < ivec_size(v1) )
    {
		
      num1 = ivec_ref(v1, i);
      if( j < ivec_size(v2))
	num2 = ivec_ref(v2, j);
      else
	num2 = ivec_ref(v1, ivec_size(v1) -1 ) + 1;
	
      if(num1 > num2)
      {
	j++;
      }
      if(num1 < num2)
      {
	i++;
	add_to_ivec(result, num1);
      }
      if(num1 == num2)
      {
	i++;
	j++;
      }
    }	
  }
  return(result);
}	

ivec *mk_append_ivecs(ivec *a,ivec *b)
{
  if (!a && !b) return NULL;
  else if (!a && b) return mk_copy_ivec(b);
  else if (a && !b) return mk_copy_ivec(a);
  else
  {
    int size_a = ivec_size(a);
    int size_b = ivec_size(b);
    ivec *c = mk_ivec(size_a + size_b);
    memcpy(c->iarr, a->iarr, a->size * sizeof (int));
    memcpy(c->iarr + a->size, b->iarr, b->size * sizeof (int));
    return c;
  }
}

string_array *mk_string_array_from_ivec(ivec *iv)
{
  string_array *sa;
  
  if ( ivec_size(iv) == 0 )
  {
    sa = mk_string_array(1);
    string_array_set(sa,0,"empty");
  }
  else
  {
    int i;
    sa = mk_string_array(ivec_size(iv));
    for ( i = 0 ; i < ivec_size(iv) ; i++ )
    {
      char buff[100];
      sprintf(buff,"%d",ivec_ref(iv,i));
      string_array_set(sa,i,buff);
    }
  }
  return(sa);
}

/* Makes a string of numbers, each separated by whitespace.
   No quotes or anything. Just numbers. */
char *mk_string_from_ivec(ivec *iv)
{
  string_array *sa = mk_string_array_from_ivec(iv);
  char *s = mk_string_from_string_array(sa);
  free_string_array(sa);
  return(s);
}

/***** ivec_array time! ****/

#define INITIAL_ivec_ARRAY_SIZE 10

ivec_array *mk_ivec_array(int size)
{
  int i;
  ivec_array *iva;
  iva = AM_MALLOC( ivec_array);
  iva->size = size;
  iva->array_size = int_max( size, INITIAL_ivec_ARRAY_SIZE);
  iva->array = AM_MALLOC_ARRAY( ivec_ptr, iva->array_size);
  for (i = 0; i < iva->array_size; i++) iva->array[i] = NULL;
  return iva;
}

ivec_array *mk_empty_ivec_array(void)
{
  ivec_array *ivecarr = AM_MALLOC(ivec_array);
  ivecarr -> size = 0;
  ivecarr -> array_size = INITIAL_ivec_ARRAY_SIZE;
  ivecarr -> array = AM_MALLOC_ARRAY(ivec_ptr,ivecarr->array_size);
  return(ivecarr);
}

ivec_array *mk_const_ivec_array(ivec *base_vec, int size){
  int i;
  ivec_array *result = mk_empty_ivec_array();
  for (i=0; i<size; i++) {
    add_to_ivec_array(result, base_vec);
  }
  return result;
}

ivec_array *mk_zero_ivec_array(int size){
  ivec *temp_vec = mk_ivec(0);
  ivec_array *result = mk_const_ivec_array(temp_vec, size);
  free_ivec(temp_vec);
  return result;
}

/* does the same thing as mk_zero_ivec_array */
ivec_array *mk_array_of_zero_length_ivecs(int size)
{
  ivec_array *iva = mk_empty_ivec_array();
  ivec *iv = mk_ivec(0);
  int i;

  for (i = 0; i < size; i++)
        add_to_ivec_array(iva, iv);
  free_ivec(iv);
  return(iva);
}

/* Added by Jeremy - makes an ivec array of length size(lengths)
   where ivec number i an ivec of ivec_ref(lengths,i) zeros */
ivec_array *mk_ivec_array_of_given_lengths(ivec* lengths)
{
  int i;
  ivec_array *iva = AM_MALLOC(ivec_array);

  iva->size = ivec_size(lengths);
  iva->array_size = int_max( iva->size, INITIAL_ivec_ARRAY_SIZE);

  iva->array = AM_MALLOC_ARRAY( ivec_ptr, iva->array_size );

  for(i=0;i<iva->size;i++)
    iva->array[i] = mk_zero_ivec(ivec_ref(lengths,i));
  for (i = iva->size; i < iva->array_size; i++) 
    iva->array[i] = NULL;

  return iva;
}

/* Create a rectangular array of unitialized ivecs. */
ivec_array *mk_rectangular_ivec_array( int numivecs, int iveclen)
{
  int i;
  ivec_array *iva;
  iva = mk_ivec_array( numivecs);
  for (i=0; i<numivecs; ++i) {
    iva->array[i] = mk_ivec(iveclen);
  }
  return iva;
}

void add_val_to_given_ivec_in_ivec_array(ivec_array *iva,int idx,int val)
{
  my_assert(iva->size > idx);
  if (iva->array[idx] == NULL)
    iva->array[idx] = mk_ivec_1(val);
  else
    add_to_ivec(iva->array[idx],val);
}

void add_to_ivec_array(ivec_array *ivecarr,const ivec *this_ivec)
/*
   Assume ivec_array is previously of size n. After this it is of size
   n+1, and the n+1'th element is a COPY of this_ivec.
*/
{
  if ( ivecarr -> size == ivecarr -> array_size )
  {
    int new_size = 2 + 2 * ivecarr->array_size;
    ivec **new_array = AM_MALLOC_ARRAY(ivec_ptr,new_size);
    memcpy(new_array, ivecarr->array, ivecarr->array_size * sizeof (ivec *));
    AM_FREE_ARRAY(ivecarr->array,ivec_ptr,ivecarr->array_size);
    ivecarr -> array = new_array;
    ivecarr -> array_size = new_size;
  }
  ivecarr->array[ivecarr->size] = (this_ivec==NULL) ? NULL : mk_copy_ivec(this_ivec);
  ivecarr->size += 1;
}


/* Adds an ivec of length 0 */
void add_empty_ivec_to_ivec_array(ivec_array *ivecarr) {
  ivec* temp = mk_ivec(0);
  add_to_ivec_array(ivecarr,temp);
  free_ivec(temp);
}


void ivec_array_insert(ivec_array *iv_a, int idx, ivec *this_ivec)
{
  my_assert(idx < ivec_array_size(iv_a) + 1);
  my_assert(idx >= 0 );
  add_to_ivec_array(iv_a, NULL);
  memmove(iv_a->array + idx + 1, iv_a->array + idx, (iv_a->size - idx - 1) * sizeof (ivec *));
  iv_a->array[idx] = NULL;
  ivec_array_set(iv_a,idx,this_ivec);
}

int ivec_array_size(const ivec_array *ivecarr)
{
  return(ivecarr->size);
}

/* Returns the sum of all ivec_size(...) values of all ivecs
   in iva */
int sum_of_ivec_array_sizes(ivec_array *iva)
{
  int result = 0;
  int i;
  for ( i = 0 ; i < ivec_array_size(iva) ; i++ )
    result += ivec_size(ivec_array_ref(iva,i));
  return result;
}

ivec *safe_ivec_array_ref(const ivec_array *ivecarr,int idx)
/*
     Returns a pointer (not a copy) to the index'th element stored in
   the ivec_array. Error if index < 0 or index >= size
*/
{
  ivec *result;
  if ( idx < 0 || idx >= ivec_array_size(ivecarr) )
  {
    char msg[500];
    sprintf( msg, "ivec_array_ref: index=%d is out of bounds [0,%d].",
	     idx, ivec_array_size(ivecarr)-1 );
    result = NULL;
    my_error(msg);
  }
  else
    result = ivecarr->array[idx];
  return(result);
}

void ivec_array_set(ivec_array *iva, int idx, const ivec *iv)
{
  if ((idx < 0) || (iva == NULL) || (idx >= iva->size))
        my_error("ivec_array_set: called with incompatible arguments");
  if (iva->array[idx] != NULL)
        free_ivec(iva->array[idx]);
  iva->array[idx] = (iv == NULL) ? NULL : mk_copy_ivec(iv);
}

void fprintf_ivec_array(FILE *s,char *m1,ivec_array *ivecarr,char *m2)
{
  if ( ivec_array_size(ivecarr) == 0 )
    fprintf(s,"%s = <ivec_array with zero entries>%s",m1,m2);
  else
  {
    int i;
    for (i=0; i<ivec_array_size(ivecarr); i++){
      char buff[1024];
      ivec *iv = ivec_array_ref(ivecarr,i);
      sprintf(buff,"%s[%2d]",m1,i);
      if ( iv == NULL )
        fprintf(s,"%s = NULL%s",buff,m2);
      else
        fprintf_ivec(s,buff,iv,m2);
    }
  }
}

void pivec_array(ivec_array *iva)
{
  fprintf_ivec_array(stdout,"iva",iva,"\n");
}

void ivec_array_update_sizeof(ivec_array *iv_a, auton_sf *x)
{
  if ( iv_a != NULL )
  {
    int i;

    add_to_auton_sf(x,(unsigned long)sizeof(ivec_array));
    add_to_auton_sf(x,(unsigned long)(iv_a->array_size)*sizeof(ivec *));

    for ( i = 0; i < iv_a->size; i++ )
      ivec_update_sizeof(iv_a->array[i],x);
  }
}

void free_ivec_array(ivec_array *ivecarr)
{
  int i;
  for ( i = 0 ; i < ivec_array_size(ivecarr) ; i++ )
    if ( ivecarr->array[i] != NULL )
      free_ivec(ivecarr->array[i]);
  AM_FREE_ARRAY(ivecarr->array,ivec_ptr,ivecarr->array_size);
  AM_FREE(ivecarr,ivec_array);
}

int ivec_array_num_bytes(ivec_array *iva)
{
  int result = sizeof(ivec_array) + iva->array_size * sizeof(ivec *);
  int i;

  for ( i = 0 ; i < ivec_array_size(iva) ; i++ )
    result += ivec_num_bytes(ivec_array_ref(iva,i));

  return result;
}

ivec_array *mk_copy_ivec_array(const ivec_array *ivecarr)
{
  ivec_array *new_ar = mk_ivec_array(ivec_array_size(ivecarr));
  int i;

  for ( i = 0 ; i < ivec_array_size(ivecarr) ; i++ )
    ivec_array_set(new_ar,i,ivec_array_ref(ivecarr,i));

  return(new_ar);
}

bool ivec_array_equal(ivec_array *iva1,ivec_array *iva2)
{
  int size = ivec_array_size(iva1);
  bool result = size == ivec_array_size(iva2);
  int i;
  for ( i = 0 ; result && i < size ; i++ )
    result = ivec_equal(ivec_array_ref(iva1,i),ivec_array_ref(iva2,i));
  return result;
}

void ivec_array_remove(ivec_array *iva,int idx)
{
  ivec *iv = ivec_array_ref(iva,idx);
  if ( iv != NULL ) free_ivec(iv);
  memmove(iva->array + idx, iva->array + idx + 1, (iva->size - idx - 1) * sizeof (ivec *));
  iva->array[iva->size-1] = NULL;
  iva->size -= 1;
}

void ivec_array_remove_last_element( ivec_array *iva)
{
  int size;
  ivec *iv;

#ifndef AMFAST
  if (ivec_array_size(iva) <= 0) {
    my_error("ivec_array_remove_last_elementt: empty ivec_array");
  }
#endif /* #ifndef AMFAST */

  size = ivec_array_size( iva);
  iv   = ivec_array_ref(iva, size-1);
  if (iv != NULL) free_ivec(iv);
  iva->size -= 1;

  return;
}

void ivec_array_remove_last_n_elements( ivec_array *iva, int n)
{
  int size, newsize, i;
  ivec *iv;

  /* Adjust size. */
  size    = ivec_array_size(iva);
  newsize = size - n;
  if (newsize < 0) newsize = 0;

  /* Free ivecs. */
  for (i=size-1; i>=newsize; --i) {
    iv = ivec_array_ref(iva, i);
    if (iv != NULL) free_ivec(iv);
  }

  iva->size = newsize;

  return;
}

/* Returns an ivec_array of length equal to ivec_size(rows)
   in which result[i] = iva[rows[i]] */
ivec_array *mk_ivec_array_subset(const ivec_array *iva,const ivec *rows)
{
  int size, row, i;
  ivec *iv;
  ivec_array *result;

  if ( rows == NULL)
    result = mk_copy_ivec_array(iva);
  else
  {
    size = ivec_size(rows);
    result = mk_ivec_array( size);

    for (i=0; i < size; i++) {
      row = ivec_ref( rows, i);
      iv = ivec_array_ref( iva, row);
      ivec_array_set( result, i, iv);
    }
  }

  return result;
}

/* x := x with y appended on the end */
void append_to_ivec(ivec *x,ivec *y)
{
  int x_new_size = x->size + y->size;
  int *x_new_iarr = am_malloc_ints(x_new_size);
  memcpy(x_new_iarr, x->iarr, x->size * sizeof (int));
  memcpy(x_new_iarr + x->size, y->iarr, y->size * sizeof (int));
  am_free_ints(x->iarr,x->array_size);
  x->iarr = x_new_iarr;
  x->size = x_new_size;
  x->array_size = x_new_size;
}

/* Return x with y appended on the end */
ivec *mk_ivec_append(ivec *x,ivec *y)
{
  ivec *z = mk_copy_ivec(x);
  append_to_ivec(z,y);
  return z;
}


ivec *mk_ivec_from_ivec_array(ivec_array *iva)
{
  int size = sum_of_ivec_array_sizes(iva);
  int cur_size = 0;
  ivec *new_iv = mk_ivec(size);
  int i;
  for (i = 0; i < ivec_array_size(iva); ++i) {
    ivec *iv = ivec_array_ref(iva, i);
    memcpy(new_iv->iarr + cur_size, iv->iarr, iv->size * sizeof (int));
    cur_size += iv->size;
  }
  return new_iv;
}

/* Returns the max value in any of the ivecs in iva.
   PRE: Contains at least one non-zero-length ivec */
int ivec_array_max_value(ivec_array *iva)
{
  int result = -1;
  bool started = FALSE;
  int i;

  for ( i = 0 ; i < ivec_array_size(iva) ; i++ )
  {
    ivec *iv = ivec_array_ref(iva,i);
    if ( ivec_size(iv) > 0 )
    {  
      int m = ivec_max(iv);
      if ( started )
	result = int_max(result,m);
      else
      {
	result = m;
	started = TRUE;
      }
    }
  }

  if ( !started )
    my_error("ivec_array_max_value: no entries or all zero-length entries");

  return result;
}

/* Element-wise summation of ivecs in iva.  All ivecs in iva must have the
   same length. */
ivec *mk_ivec_array_sum( ivec_array *iva)
{
  int i, numiv, leniv;
  ivec *sumiv, *tmpiv;

  /* Check trivial cases. */
  if (iva == NULL) return NULL;
  numiv = ivec_array_size( iva);
  if (numiv == 0) return NULL;

  /* Get length of first array element. */
  leniv = ivec_size( ivec_array_ref( iva, 0));
  sumiv = mk_zero_ivec( leniv);

  for (i=0; i<numiv; ++i) {
    tmpiv = ivec_array_ref( iva, i);

    /* Check ivec length. */
    if (ivec_size( tmpiv) != leniv) {
      my_errorf( "mk_sum_ivec_array: error:\n"
		 "iva[%d] has len %d, but iva[0] has len %d.",
		 i, ivec_size( tmpiv), leniv);
    }

    /* Add. */
    ivec_plus( sumiv, tmpiv, sumiv);
  }

  return sumiv;
}

/*
// Added by Artur

   Debugged by AWM (original version only worked if #cols == #rows)
//----------------------------------------------
*/
ivec_array *mk_transpose_ivec_array(ivec_array *a)
{
  ivec_array *result = mk_empty_ivec_array();
  ivec *empty = mk_ivec(0);
  int size = ivec_array_size(a);
  int row;

  for ( row = 0 ; row < size ; row++ )
  {
    ivec *p = ivec_array_ref(a,row);
    int plen = ivec_size(p);
    int j;

    for ( j = 0 ; j < plen ; j++ )
    {
      int col = ivec_ref(p,j);
      while ( ivec_array_size(result) <= col )
	add_to_ivec_array(result,empty);

      add_to_ivec(ivec_array_ref(result,col),row);
    }
  }
  
  free_ivec(empty);
  return(result);
}



/* when coding variable dimension arrays, these can be used to convert the
   indices of the array onto the index for the 1-d array representation.
   dimsizes tells how many elements there are in each dimension of the
   array.  Both ivecs have a size equal to the number of dimensions in the
   array.
*/
void index_from_indices(ivec *indices, ivec *dimsizes, int *idx)
{
  int i,size;

  *idx = 0;
  size = 1;
  for (i=dimsizes->size-1;i>=0;i--)
  {
    *idx += ivec_ref(indices,i)*size;
    size *= ivec_ref(dimsizes,i);
  }
}

void indices_from_index(int idx, ivec *dimsizes, ivec *indices)
{
  int i;

  for (i=dimsizes->size-1;i>=0;i--)
  {
    ivec_set(indices,i,idx % ivec_ref(dimsizes,i));
    idx = idx/ivec_ref(dimsizes,i);
  }
}

/* Sets indices to be the next indices given the size of each dimension and
 * a set of them to be frozen at their current value.  Freeze may be NULL to
 * indicate that none are frozen.  This function returns 1 to indicate valid
 * next indices, and 0 to indicate that there are no more.  All non-frozen
 * indices should be set to 0 before the first call in order to get all valid
 * indices.
 */
int next_indices(ivec *indices, ivec *sizes, ivec *freeze)
{
  int done = 0;
  int cur = indices->size-1;

  if (!ivec_size(indices)) return 0;

  while(!done)
  {
    if (!freeze || !ivec_ref(freeze,cur))
    {
      ivec_set(indices,cur,(ivec_ref(indices,cur)+1)%ivec_ref(sizes,cur));
      if (ivec_ref(indices,cur)) done = 1;
    }
    if ((--cur < 0) && (!done)) return 0;
  }
  return 1;
}

/* I am SURE this is already implemented elsewhere, but I can't find it.
   You are interested in cycling through all the subsets of size 
   ivec_size(subset) of a set of n integers.  Initialize subset to the
   identity ivec.  Then call this each time you want the next subset and
   quit when it returns FALSE meaning you used them all.  If you call this
   function without starting from the identity ivec (or even worse starting
   from an ivec whose values aren't strictly increasing) its your own fault
   and something bad will happen to you.
*/
bool next_subset(ivec *subset, int n) {
  bool done = FALSE;
  int size = ivec_size(subset);
  int cur = size - 1;
  int i;
  
  while ((!done) && (cur >= 0)) {
    if (ivec_ref(subset,cur) < (n - (size - cur))) {
      ivec_increment(subset,cur,1);
      for (i=cur+1;i<size;i++) 
        ivec_set(subset,i,ivec_ref(subset,i-1)+1);
      done = TRUE;
    }
    else cur--;
  }
  if (cur < 0)
    done = TRUE;
  return done;
}

int ivec_num_bytes(ivec *iv)
{
  return(sizeof(ivec) + iv->array_size * sizeof(int));
}

ivec *mk_copy_ivec_fast(ivec *iv)
{
  ivec *result = mk_ivec(ivec_size(iv));
  memcpy(result->iarr, iv->iarr, iv->size * sizeof (int));

  return(result);
}


/* Returns ivec of size ivec_size(rows) in which
    result[i] = x[rows[i]] */
ivec *mk_ivec_subset(ivec *x,ivec *rows)
{
  int size = ivec_size(rows);
  ivec *y = mk_ivec(size);
  int i;
  for ( i = 0 ; i < size ; i++ )
    ivec_set(y,i,ivec_ref(x,ivec_ref(rows,i)));
  return y;
}

void ivec_set_subset(ivec *x, ivec *indxs, ivec *vals) 
{
  int i;
  my_assert(ivec_size(indxs) == ivec_size(vals));
  for (i = 0; i < ivec_size(indxs); i++)
    ivec_set(x,ivec_ref(indxs,i),ivec_ref(vals,i));
}

void ivec_set_subset_const( ivec *iv, ivec *indxs, int val)
{
  int numidx, i, idx;
  numidx = ivec_size( indxs);
  for (i=0; i<numidx; ++i) {
    idx = ivec_ref( indxs, i);
    ivec_set( iv, idx, val);
  }
  return;
}

/*same as above except it only sets the subset of the subset that is 0 */
void ivec_set_zero_subset_const(ivec *iv, ivec *indxs, int val)
{
  int numidx, i, idx;
  numidx = ivec_size(indxs);
  for (i=0; i<numidx; ++i) {
    idx = ivec_ref(indxs, i);
    if (ivec_ref(iv,idx) == 0)
      ivec_set(iv, idx, val);
  }
}

/* test whether sub is a subset of sup */
bool is_ivec_subset(ivec *sup, ivec *sub)
{
  bool result = TRUE;
  int i;

  for (i=0;i<ivec_size(sub)&&result;i++) 
    result = is_in_ivec(sup,ivec_ref(sub,i));

  return result;
}

int ivec_array_num_elems_at_index(ivec_array *iva, int rownum)
{
  return ivec_size(ivec_array_ref(iva,rownum));
}
