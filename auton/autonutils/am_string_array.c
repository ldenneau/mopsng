/* 	File: am_string_array.c 

   Copyright, Autonlab CMU

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

#include "standard.h"
#include "ambs.h"
#include "amiv.h"
#include "am_string_array.h"
#include "amma.h"
#include "am_string.h"
#include "amdyv_array.h"
#include "internal_state.h"

/* Private? */
void check_string_array_code(const string_array *sar, const char *name);

#define STRING_ARRAY_CODE 20542

string_array *mk_string_array(int size)
{
  am_memory_state_t *state_ptr = am_get_memory_state();
  string_array *result = AM_MALLOC(string_array);
  int i;

  result -> string_array_code = STRING_ARRAY_CODE;
  result -> size = size;
  result -> sarr_size = size;
  result -> sarr = AM_MALLOC_ARRAY(char_ptr,size);

  for ( i = 0 ; i < size ; i++ )
    result->sarr[i] = mk_copy_string("<UnDeFiNeD>");
  state_ptr->String_Arrays_mallocked += 1;
  return(result);
}

void free_string_array(string_array *sar)
{
  am_memory_state_t *state_ptr = am_get_memory_state();
  int i;
  check_string_array_code(sar,"free_string_array");
  sar -> string_array_code = 7777;

  for ( i = 0 ; i < sar->size ; i++ )
    if ( sar->sarr[i] != NULL )
      am_free_string(sar->sarr[i]);

  AM_FREE_ARRAY(sar->sarr,char_ptr,sar->sarr_size);
  AM_FREE(sar,string_array);

  state_ptr->String_Arrays_freed += 1;
}

string_array *mk_copy_string_array(const string_array *sa)
{
  string_array *nsa = mk_string_array(string_array_size(sa));
  int i;
  for ( i = 0 ; i < string_array_size(sa) ; i++ )
    string_array_set(nsa,i,string_array_ref(sa,i));
  return(nsa);
}

bool string_array_equal(string_array *a,string_array *b)
{
  int size = string_array_size(a);
  bool equal = size == string_array_size(b);
  int i;
  for ( i = 0 ; equal && i < size ; i++ )
    equal = equal && eq_string(string_array_ref(a,i),string_array_ref(b,i));
  return equal;
}

string_array *mk_string_array_1(const char *x)
{
  string_array *sa = mk_string_array(1);
  string_array_set(sa,0,x);
  return(sa);
}


void check_string_array_code(const string_array *sar, const char *name)
{
  if ( sar == NULL )
  {
    fprintf(stderr,"NULL string_array passed in operation %s\n",name);
    my_error("string_array data structure");
  }
  if ( sar->string_array_code != STRING_ARRAY_CODE )
  {
    fprintf(stderr,"Attempt to access a non-allocated String Array\n");
    fprintf(stderr,"This is in the operation %s\n",name);
    my_error("string_array data structure error");
  }
}

void check_string_array_access(const string_array *sar, int i, const char *name)
{
  check_string_array_code(sar,name);

  if ( i < 0 || i >= sar->size )
  {
    fprintf(stderr,"In operation \"%s\"\n",name);
    fprintf(stderr,"the string_array has size = %d\n",sar->size);
    fprintf(stderr,"You tried to use index i=%d\n",i);
    fprintf(stderr,"Here is the string_array that was involved:\n");
    fprintf_string_array(stderr,"sarv",sar,"\n");
    my_error("check_string_array_access");
  }
}

void assert_string_array_shape(string_array *sar,int size,const char *name)
{
  check_string_array_code(sar,name);

  if ( size != sar->size )
  {
    fprintf(stderr,"In operation \"%s\"\n",name);
    fprintf(stderr,"the string_array has size = %d\n", sar->size);
    fprintf(stderr,"But should have been predefined with the shape:\n");
    fprintf(stderr,"size = %d\n",size);
    my_error("assert_string_array_shape");
  }
}

string_array *mk_string_array_2(const char *x1,const char *x2)
{
  string_array *sa = mk_string_array(2);
  string_array_set(sa,0,x1);
  string_array_set(sa,1,x2);
  return(sa);
}

string_array *mk_string_array_3(const char *x1,const char *x2,const char *x3)
{
  string_array *sa = mk_string_array(3);
  string_array_set(sa,0,x1);
  string_array_set(sa,1,x2);
  string_array_set(sa,2,x3);
  return(sa);
}

string_array *mk_string_array_4(const char *x1,const char *x2,const char *x3,const char *x4)
{
  string_array *sa = mk_string_array(4);
  string_array_set(sa,0,x1);
  string_array_set(sa,1,x2);
  string_array_set(sa,2,x3);
  string_array_set(sa,3,x4);
  return(sa);
}

string_array *mk_string_array_5(const char *x1,const char *x2,const char *x3,const char *x4,const char *x5)
{
  string_array *sa = mk_string_array(5);
  string_array_set(sa,0,x1);
  string_array_set(sa,1,x2);
  string_array_set(sa,2,x3);
  string_array_set(sa,3,x4);
  string_array_set(sa,4,x5);
  return(sa);
}

string_array *mk_string_array_x( int size, ...)
{
  /* XXX: no type checking */
  int i;
  char *s;
  va_list argptr;
  string_array *sa;

  sa = mk_string_array( size);

  va_start( argptr, size);
  for (i=0; i<size; ++i) {
    s = va_arg( argptr, char *);
    string_array_set( sa, i, s); /* Copies string into string array. */
  }
  va_end(argptr);

  return sa;
}

/* Like mk_string_array_x(), above, but takes a va_list directly.
   Last element of arglist must be NULL. */
string_array *mk_valist_string_array( va_list arglist)
{
  /* XXX: no type checking */
  char *s;
  string_array *sa;

  sa = mk_string_array( 0);

  while (1) {
    s = va_arg( arglist, char *);
    if (s == NULL) break;
    add_to_string_array( sa, s);
  }

  return sa;
}

char **mk_array_from_string_array( string_array *sa)
{
  int size, i;
  char **array, *s;

  size = string_array_size( sa);
  array = AM_MALLOC_ARRAY( char *, size);
  for (i=0; i<size; ++i) {
    s = string_array_ref( sa, i);
    array[i] = mk_copy_string( s);
  }

  return array;
}

string_array *mk_string_array_from_char_array(char** ca,int size)
{
  string_array *sa = mk_string_array(size);
  int i;
  
  for(i = 0; i<size; i++)
    string_array_set(sa,i,ca[i]);

  return sa;
}

void swap_string_array(string_array *sa, int i, int j)
{
  char *tmp;

  if(i != j) {
    tmp = mk_copy_string(string_array_ref(sa,i));
    string_array_set(sa,i,string_array_ref(sa,j));
    string_array_set(sa,j,tmp);
    free_string(tmp);
  }
}


void qsort_string_array(string_array *sa, int left, int right)
{
	int last, i;
	if(left>=right) return; /*basis*/
	swap_string_array(sa, left, (left+right)/2);
	last = left;
	for(i=left+1;i<=right;i++)
	{
		if(strcmp(string_array_ref(sa,i),string_array_ref(sa,left)) < 0)
		{
			swap_string_array(sa,++last,i);
		}
	}
		swap_string_array(sa, left, last);
	qsort_string_array(sa, left, last-1);
	qsort_string_array(sa,last+1,right);
}

void sort_string_array(string_array *sa)
{
  qsort_string_array(sa,0, string_array_size(sa)-1);
  
  /*the built in qsort didn't work for some reason.*/
  /*qsort((char *)sa->sarr,sa->size,sizeof(char *),strcmp);*/
}

string_array *mk_sort_string_array(string_array *sa)
{
  string_array *result = mk_copy_string_array(sa);
  sort_string_array(result);
  return result;
}

string_array *mk_sort_string_array_remove_duplicates(string_array *sa_base)
{
  string_array *sorted = mk_sort_string_array(sa_base);
  string_array *result = mk_string_array(0);
  int i;
  char *s_prev = NULL;

  for ( i = 0 ; i < string_array_size(sorted) ; i++ )
  {
    char *s_this = string_array_ref(sorted,i);
    if ( s_prev == NULL || !eq_string(s_this,s_prev) )
      add_to_string_array(result,s_this);
    s_prev = s_this;
  }

  free_string_array(sorted);

  return result;
}

/*reverse string array order */
string_array *mk_reverse_string_array(string_array *sa)
{
    string_array *reversed = mk_string_array(0);
    int i;
    int size = string_array_size(sa);

    for (i = size - 1 ; i >=0 ; i--)
    {
	string_array_add(reversed, string_array_ref(sa, i));
    }

    return reversed;
}

void fprintf_string_array(FILE *s,const char *m1,const string_array *sar,
                          const char *m2)
{
  int i;

  if (sar == NULL)
    fprintf(s,"%s = <NULL string array>%s",m1,m2);
  else
  {
    check_string_array_code(sar,"fprintf_string_array");
    if ( string_array_size(sar) == 0 )
      fprintf(s,"%s = <empty string array>%s",m1,m2);
    else
    {
      for ( i = 0 ; i < string_array_size(sar) ; i++ )
	fprintf(s,"%s[%3d] = %s%s",
		m1,i,
		(string_array_ref(sar,i)==NULL) ? "NULL" : string_array_ref(sar,i),
		m2
		);
    }
  }
}

void pstring_array(string_array *sa)
{
  fprintf_string_array(stdout,"sa",sa,"\n");
}

void string_array_update_sizeof(string_array *sa, auton_sf *x)
{
  if ( sa != NULL )
  {
    int i;

    add_to_auton_sf(x,(unsigned long)sizeof(string_array));
    add_to_auton_sf(x,(unsigned long)(sa->sarr_size)*sizeof(char *));

    for ( i = 0; i < sa->size; i++ )
      string_update_sizeof(sa->sarr[i],x);
  }
}

void fprintf_string_array_contents_on_one_line(FILE *s,string_array *sar)
{
  int i;
  check_string_array_code(sar,"fprintf_string_array_contents");

  for ( i = 0 ; i < string_array_size(sar) ; i++ )
    fprintf(s,"%s ",
            (string_array_ref(sar,i)==NULL) ? "NULL" : string_array_ref(sar,i)
           );
}

void fprintf_string_array_contents(FILE *s,string_array *sar)
{
  int i;
  check_string_array_code(sar,"fprintf_string_array_contents");

  for ( i = 0 ; i < string_array_size(sar) ; i++ )
    fprintf(s,"%s\n",
            (string_array_ref(sar,i)==NULL) ? "NULL" : string_array_ref(sar,i)
           );
}

char *string_array_ref(const string_array *sar, int i)
{
  check_string_array_access(sar,i,"string_array_ref");
  return(sar->sarr[i]);
}

void string_array_set(string_array *sar,int i,const char *value)
/* value is COPIED in */
{
  check_string_array_access(sar,i,"string_array_set");
  if ( sar->sarr[i] != NULL )
    am_free_string(sar->sarr[i]);
  sar->sarr[i] = mk_copy_string(value);
}

int string_array_size(const string_array *sar)
{
  check_string_array_code(sar,"string_array_size");
  return(sar->size);
}

string_array *mk_string_array_from_array(char **sarr,int size)
{
  int i;
  string_array *result = mk_string_array(size);
  for ( i = 0 ; i < size ; i++ )
    string_array_set(result,i,sarr[i]);
  return(result);
}

/* only for people who really want to save time allocating memory.
   after calling this, you should forget about the memory in string without
   freeing it.
 */
void add_to_string_array_no_copy(string_array *sa, char *string)
{
  if ( sa -> size == sa -> sarr_size )
  {
    int new_sarr_size = 2 + 2 * sa->sarr_size;
    char **new_sarr = AM_MALLOC_ARRAY(char_ptr,new_sarr_size);
    int i;

    for ( i = 0 ; i < sa->size ; i++ ) new_sarr[i] = sa->sarr[i];

    AM_FREE_ARRAY(sa->sarr,char_ptr,sa->sarr_size);
    sa -> sarr_size = new_sarr_size;
    sa -> sarr = new_sarr;
  }

  sa -> size += 1;
  sa -> sarr[sa->size-1] = string;
}

void add_to_string_array(string_array *sa, const char *string)
{
  if ( sa -> size == sa -> sarr_size )
    {
    int new_sarr_size = 2 + 2 * sa->sarr_size;
    char **new_sarr = AM_MALLOC_ARRAY(char_ptr,new_sarr_size);
    int i;

    for ( i = 0 ; i < sa->size ; i++ )
      new_sarr[i] = sa->sarr[i];

    AM_FREE_ARRAY(sa->sarr,char_ptr,sa->sarr_size);
    sa -> sarr_size = new_sarr_size;
    sa -> sarr = new_sarr;
    }

  sa -> size += 1;
  sa -> sarr[sa->size-1] = (string==NULL) ? NULL : mk_copy_string(string);
}

void insert_in_string_array(string_array *sa, int pos, const char *string)
{
  int i;
  if (pos > sa->size)
    my_error("Illegal string_array index.");

  i = sa->size - 1;
  add_to_string_array(sa, NULL);
  for (; i >= pos; i--)
    sa->sarr[i + 1] = sa->sarr[i];

  sa->sarr[pos] = (string == NULL) ? NULL : mk_copy_string(string);
}

/* FIXME: A depreciated alternate name for add_to_string_array.  */
void string_array_add(string_array *sa,const char *string)
{
  add_to_string_array(sa,string);
}

void string_array_malloc_report(void)
{
  am_memory_state_t *state_ptr = am_get_memory_state();
  if ( state_ptr->String_Arrays_mallocked > 0 )
  {
    fprintf(stdout,"#         Number of string_arrays currently allocated: %d\n",
           state_ptr->String_Arrays_mallocked - state_ptr->String_Arrays_freed
          );
    if ( state_ptr->String_Arrays_mallocked - state_ptr->String_Arrays_freed != 0 )
    {
      fprintf(stdout,"#       Number of string_array allocs since prog start:  %d\n",
             state_ptr->String_Arrays_mallocked
            );
      fprintf(stdout,"#       Number of string_array frees  since prog start:  %d\n#\n",
            state_ptr->String_Arrays_freed
            );
    }
  }
}


void string_array_remove(string_array* sa, int idx)
{
  int i;
  if ( string_array_size(sa) <= 0 ) my_error("string_array_remove: empty string_array");
  if ( idx < 0 || idx >= string_array_size(sa) )
    my_error("string_array_remove: bad index");

  /* WKW - New version shuffles the pointers over by one instead
     of using string_array_set, which copies the (i+1)th element into the
     ith spot and then frees the (i+1)th element.  This is faster and
     it avoids some memory bugs in the old version */
  if( sa->sarr[idx] != NULL )
    {
    am_free_string(sa->sarr[idx]);
    sa->sarr[idx]=NULL;
    }

  for( i = idx ; i < (sa->size - 1); i++ )
    sa->sarr[i] = sa->sarr[i+1];

  sa->sarr[sa->size-1] = NULL; /* Might help catch some weird errors */
  sa -> size -= 1;
}


void string_array_remove_last_element(string_array* sa)
{ /* It'd be tempting to use a C99 function-macro to do this, if we could use C99 -- Pat */
  string_array_remove(sa,string_array_size(sa)-1);
}


string_array *mk_broken_string_using_seppers(const char *string, const char *seppers)
{
  string_array *sa = mk_string_array(0);
  int i = 0;
  while ( string != NULL && string[i] != '\0' )
    {
    int j = 0;
    while ( is_sepper(string[i],seppers) )
      i += 1;

    if ( string[i] != '\0' )
      {
      int part_size,backslashcount = 0;
      char *part,stopchar = ' ';
      int k;

      while ( string[i+j] != '\0' )
        {
	if(stopchar == ' ')
	  {
	  if(is_sepper(string[i+j],seppers))
            break;
	  else if((string[i+j]=='\"') && !(backslashcount%2))
            stopchar = '\"';
	  }
	else if(stopchar == '\"')
	  {
	  /* bug fix? this used to say stopchar = '\n' which made it put the
             whole rest of the line into one string once it had seen a double
             quote.  Now it only includes up to the next double quote. 8/24/99 JGS */
	  if((string[i+j] == '\"') && !(backslashcount %2))
            stopchar = ' ';
	  }

	if (string[i+j] == '\\') backslashcount++;
	else                     backslashcount=0;
	
        j++;
        }

      part_size = j+1;
      part = AM_MALLOC_ARRAY(char,part_size);

      for ( k = 0 ; k < j ; k++ )
        part[k] = string[i+k];
      if ( k != part_size-1 ) my_error("oaibxwibxpwibx");
      part[k] = '\0';
      string_array_add(sa,part);
      AM_FREE_ARRAY(part,char,part_size);
      }
    i = i+j;
    }
  return(sa);
}

string_array *mk_broken_string_using_seppers_only(const char *string,const char *seppers)
{
  string_array *sa = mk_string_array(0);
  char *p = strpbrk(string,seppers);
  char c;
  while(p){
    c = *p;
    *p = '\0';
    add_to_string_array(sa,string);
    *p = c;
    string = p+1;
    p = strpbrk(string,seppers);
  }
  add_to_string_array(sa,string);
  return sa;
}

string_array *mk_broken_string(const char *string)
{
  string_array *result = mk_broken_string_using_seppers(string,NULL);
  return(result);
}

string_array *mk_broken_quoteless_string(const char *string)
{
  char *quoteless = mk_quoteless_string(string);
  string_array *result = mk_broken_string_using_seppers(quoteless,NULL);
  free_string(quoteless);
  return(result);
}

string_array *mk_string_array_subset(const string_array *sa,const ivec *indices)
{
  int size = ivec_size(indices);
  string_array *result = mk_string_array(size);
  int i;
  for ( i = 0 ; i < size ; i++ )
    string_array_set(result,i,string_array_ref(sa,ivec_ref(indices,i)));
  return result;
}


char *mk_string_from_last_n_with_separator(string_array *sa,int n,
					   bool use_sep,char sep)
{
  char *result;

  my_assert(sep != '\0');

  if ( n < 0 )
  {
    my_error("odcbowdn");
    result = NULL;
  }
  else if ( n == 0 )
    result = mk_copy_string("");
  else
  {
    int i;
    int array_size = ( use_sep ) ? n : 1;
    int k = 0;
    int start = string_array_size(sa) - n;

/* array_size = sum of string lengths + n-1 seperators + 1 \0 character.
   (note wouldnt be true for n==0 but thats dealt with above)
*/
    for ( i = 0 ; i < n ; i++ )
      array_size += strlen(string_array_ref(sa,start+i));

    result = AM_MALLOC_ARRAY(char,array_size);

    for ( i = 0 ; i < n ; i++ )
    {
      int j;
      char *s = string_array_ref(sa,start+i);
      for ( j = 0 ; s[j] != '\0' ; j++ )
      {
        result[k] = s[j];
        k += 1;
      }
      if(i==n-1)
      {
        result[k] = '\0';
        k += 1;
      } 
      else if ( use_sep ) 
      {
        result[k] = sep;
        k += 1;
      }
    }

    if ( k != array_size ) my_error("lkaxcnowsdcbolub");
  }
  return(result);
}

char *mk_string_from_last_n(string_array *sa,int n)
{
  return mk_string_from_last_n_with_separator(sa,n,TRUE,' ');
}

char *mk_string_from_string_array_basic(string_array *sa,bool use_sep,char sep)
{
  return mk_string_from_last_n_with_separator(sa,string_array_size(sa),
					      use_sep,sep);
}

char *mk_string_from_string_array_with_separator(string_array *sa,char sep)
{
  return mk_string_from_string_array_basic(sa,TRUE,sep);
}

char *mk_string_from_string_array(string_array *sa)
{
  return mk_string_from_string_array_with_separator(sa,' ');
}

char *mk_string_from_string_array_no_gaps(string_array *sa)
{
  return mk_string_from_string_array_basic(sa,FALSE,'7');
}

int find_index_in_string_array(const string_array *sa, const char *string)
{
  int result = -1;
  int i;
  for ( i = 0 ; i < string_array_size(sa) && result < 0 ; i++ )
    if ( eq_string(string,string_array_ref(sa,i)) )
      result = i;
  return(result);
}

bool is_in_string_array(const string_array *sa, const char *string)
{
  return (find_index_in_string_array(sa,string) >= 0);
}

int caseless_find_index_in_string_array(string_array *sa,const char *string)
{
  int result = -1;
  int i;
  for ( i = 0 ; i < string_array_size(sa) && result < 0 ; i++ )
    if ( caseless_eq_string(string,string_array_ref(sa,i)) )
      result = i;
  return(result);
}

bool string_array_member(string_array *sa,const char *string)
{
  return(find_index_in_string_array(sa,string)>=0);
}

string_array *mk_string_array_segment(string_array *sa,
                                      int start_index,int end_index)
{
  int seg_size = end_index - start_index;
  string_array *seg = mk_string_array(seg_size);
  int i;
  for ( i = 0 ; i < seg_size ; i++ )
    string_array_set(seg,i,string_array_ref(sa,start_index+i));
  return seg;
}

string_matrix *mk_string_matrix(int rows,int cols)
{
  string_matrix *sm = AM_MALLOC(string_matrix);
  int i;
  sm -> array_size = rows;
  sm -> rows = rows;
  sm -> cols = cols;
  sm -> sas = AM_MALLOC_ARRAY(string_array_ptr,rows);
  for ( i = 0 ; i < rows ; i++ )
  {
    int j;
    sm->sas[i] = mk_string_array(cols);
    for ( j = 0 ; j < cols ; j++ )
      string_array_set(sm->sas[i],j,"");
  }
  return(sm);
}

int string_matrix_rows(string_matrix *sm)
{
  return(sm->rows);
}

int string_matrix_cols(string_matrix *sm)
{
  return(sm->cols);
}

char *string_matrix_ref(string_matrix *sm,int row,int col)
{
  if ( row < 0 || col < 0 || row >= sm->rows || col >= sm->cols )
    my_error("string_matrix_ref bad row and/or col");
  return(string_array_ref(sm->sas[row],col));
}

/* Makes a COPY of char *value */
void string_matrix_set(string_matrix *sm,int row,int col,const char *value)
{
  if ( row < 0 || col < 0 || row >= sm->rows || col >= sm->cols )
    my_errorf("string_matrix_set(sm,%d,%d) when sm_rows = %d and sm_cols = %d",
	      row,col,string_matrix_rows(sm),string_matrix_cols(sm));
  string_array_set(sm->sas[row],col,value);
}

/* Makes a COPY of char *value ... identical to string_matrix_set */
void sm_set(string_matrix *sm,int row,int col,const char *value)
{
  string_matrix_set(sm,row,col,value);
}

string_array *mk_string_array_from_string_matrix_row(string_matrix *sm,
						     int row)
{
#ifndef AMFAST
  my_assert( (row >=  0) && (row < string_matrix_rows(sm)) );
#endif

  return mk_copy_string_array(sm->sas[row]);
}

string_array *string_matrix_row_to_string_array(string_matrix *sm,
						int row)
{
#ifndef AMFAST
  my_assert( (row >=  0) && (row < string_matrix_rows(sm)) );
#endif

  return sm->sas[row];
}

string_array *mk_string_array_from_string_matrix_col(string_matrix *sm,
						     int col)
{
  int num_rows = string_matrix_rows(sm);
  int i;
  string_array *result = mk_string_array(num_rows);

  for ( i = 0 ; i < num_rows ; i++ )
  {
    char *string = string_matrix_ref(sm,i,col);
    string_array_set(result,i,string);
  }

  return result;
}

void fprintf_string_matrix(FILE *s,const char *m1,string_matrix *sm,const char *m2)
{
  if ( sm == NULL )
    fprintf(s,"%s = (string_matrix *)NULL%s",m1,m2);
  else if ( sm -> rows == 0 )
    fprintf(s,"%s = <string_matrix with 0 rows>%s",m1,m2);
  else
  {
    int i;
    for ( i = 0 ; i < string_matrix_rows(sm) ; i++ )
    {
      char buff[100];
      sprintf(buff,"%s[%2d]",m1,i);
      fprintf_string_array(s,buff,sm->sas[i],"\n");
    }
  }
}

void string_matrix_add_row(string_matrix *sm)
{
  int j;

  if ( sm->array_size < sm->rows )
    my_error("oujbcowlbucv");
  if ( sm->array_size == sm->rows )
  {
    int new_size = int_max(100,(int) (2.5 * sm->array_size));
    string_array **new_ar = AM_MALLOC_ARRAY(string_array_ptr,new_size);
    int i;
    for ( i = 0 ; i < new_size ; i++ )
      new_ar[i] = NULL;
    for ( i = 0 ; i < sm->rows ; i++ )
      new_ar[i] = sm->sas[i];

    AM_FREE_ARRAY(sm->sas,string_array_ptr,sm->array_size );
    sm->sas = new_ar;
    sm->array_size = new_size;
  }
  sm->sas[sm->rows] = mk_string_array(sm->cols);
  
  /* Initialize new row with empty string entries. */
  for ( j = 0 ; j < sm->cols ; j++ )
    string_array_set(sm->sas[sm->rows],j,"");

  sm->rows += 1;
}


void free_string_matrix(string_matrix *sm)
{
  int i;
  for ( i = 0 ; i < string_matrix_rows(sm) ; i++ )
    free_string_array(sm->sas[i]);
  AM_FREE_ARRAY(sm->sas,string_array_ptr,sm->array_size);
  AM_FREE(sm,string_matrix);
}

int max_string_length_in_column(string_matrix *sm,int col)
{
  int result = 0;
  int row;
  for ( row = 0 ; row < string_matrix_rows(sm) ; row++ )
    result = int_max(result,(int)strlen(string_matrix_ref(sm,row,col)));
  return(result);
}

char *string_before_col(string_array *seps,int col)
{
  char *result;

  if (seps == NULL)
    result = (col == 0) ? "" : " ";
  else
    result = string_array_ref(seps,col);
  return(result);
}

char *rightmost_string(string_array *seps)
{
  char *result = (seps==NULL)?"":
                 string_array_ref(seps,string_array_size(seps)-1);
  return(result);
}

/* each entry i is the i'th row of a plain text representation of the contents
   of the string matrix. The columns of the table are separated by
   the strings in "seps". seps[0] always appears before the left-hand
   column. seps[1] before the second column. seps[cols] always
   appears to the right of the final column. The number of
   entries in "seps" must be one more than the number of columns
   in sm. 

   Alternatively, seps may be NULL in which case one space is
   printed between each column. No spaces to left or right.
*/
string_array *mk_tabular_string_array(string_matrix *sm,string_array *seps)
{
  int chars_per_line = 0;
  char *buffer;
  int row,col;
  int rows = string_matrix_rows(sm);
  int cols = string_matrix_cols(sm);
  int buffer_size;
  string_array *result = mk_string_array(rows);

  my_assert(seps == NULL || string_array_size(seps) == cols+1);

  for ( col = 0 ; col < cols ; col++ )
  {
    chars_per_line += max_string_length_in_column(sm,col);
    chars_per_line += strlen(string_before_col(seps,col));
  }
  chars_per_line += strlen(rightmost_string(seps));

  buffer_size = chars_per_line + 1;
  buffer = AM_MALLOC_ARRAY(char,buffer_size);

  for ( row = 0 ; row < rows ; row++ )
  {
    int i = 0;
    int j;
    char *last_sep = rightmost_string(seps);
    for ( col = 0 ; col < cols ; col++ )
    {
      char *sep = string_before_col(seps,col);
      char *con = string_matrix_ref(sm,row,col);
      int chars_after_con = max_string_length_in_column(sm,col) - 
                            strlen(con);
      if ( chars_after_con < 0 ) my_error("uwocbxwloubc");
      for ( j = 0 ; sep[j] != '\0' ; j++ )
      {
        buffer[i] = sep[j];
        i += 1;
      }
      for ( j = 0 ; con[j] != '\0' ; j++ )
      {
        buffer[i] = con[j];
        i += 1;
      }
      for ( j = 0 ; j < chars_after_con ; j++ )
      {
        buffer[i] = ' ';
        i += 1;
      }
    }
    for ( j = 0 ; last_sep[j] != '\0' ; j++ )
    {
      buffer[i] = last_sep[j];
      i += 1;
    }
    if ( i != chars_per_line ) my_error("owbxolwubdcoufcbv");
    buffer[i] = '\0';
    string_array_set(result,row,buffer);
  }

  AM_FREE_ARRAY(buffer,char,buffer_size);

  return(result);
}

string_array *mk_tabular_string_array_simple(string_matrix *sm,
					     const char *seps)
{
  int size = (int)strlen(seps);
  string_array *sep_sa = mk_string_array(size);
  int i;
  string_array *result;

  for ( i = 0 ; i < size ; i++ )
  {
    char x[2];
    x[0] = seps[i];
    x[1] = '\0';
    string_array_set(sep_sa,i,x);
  }
  result = mk_tabular_string_array(sm,sep_sa);

  free_string_array(sep_sa);
  return result;
}

int find_index_in_sorted_string_array(string_array *sa,const char *s)
{
  int result = -1;
  if ( string_array_size(sa) > 0 )
  {
    int i_lo = 0;
    int i_hi = string_array_size(sa)-1;
    char *s_lo = string_array_ref(sa,i_lo);
    char *s_hi = string_array_ref(sa,i_hi);
    if ( eq_string(s,s_lo) )
      result = i_lo;
    else if ( eq_string(s,s_hi) )
      result = i_hi;
    while ( result < 0 && i_hi > i_lo + 1 )
    {
      int i_mid = (i_lo + i_hi)/2;
      char *s_mid = string_array_ref(sa,i_mid);
      int cmp = strcmp(s,s_mid);
      /* cmp < 0 <=> s is lexicographically before s_mid */
      /* cmp = 0 <=> s equals s_mid */
      /* cmp > 0 <=> s is lexicographically after s_mid */
      if ( cmp == 0 )
        result = i_mid;
      else if ( cmp < 0 )
      {
        i_hi = i_mid;
        s_hi = s_mid;
      }
      else
      {
        i_lo = i_mid;
        s_lo = s_mid;
      }
    }
  }
  return(result);
}

int find_closest_index_in_sorted_string_array(string_array *sa,const char *s)
{
  int result = -1;
  if ( string_array_size(sa) > 0 )
  {
    int i_lo = 0;
    int i_hi = string_array_size(sa)-1;
    char *s_lo = string_array_ref(sa,i_lo);
    char *s_hi = string_array_ref(sa,i_hi);
    if ( strcmp(s,s_lo) <= 0 ) /* at or before first element */
      result = i_lo;
    else if ( eq_string(s,s_hi) ) /* at last element */
      result = i_hi;
    else if ( strcmp(s,s_hi) > 0 ) /* after last element */
      result = i_hi + 1;
    while ( result < 0 && i_hi > i_lo + 1 )
    {
      int i_mid = (i_lo + i_hi)/2;
      char *s_mid = string_array_ref(sa,i_mid);
      int cmp = strcmp(s,s_mid);
      /* cmp < 0 <=> s is lexicographically before s_mid */
      /* cmp = 0 <=> s equals s_mid */
      /* cmp > 0 <=> s is lexicographically after s_mid */
      if ( cmp == 0 )
        result = i_mid;
      else if ( cmp < 0 )
      {
        i_hi = i_mid;
        s_hi = s_mid;
      }
      else
      {
        i_lo = i_mid;
        s_lo = s_mid;
      }
    }
    result = result < 0 ? i_hi /* return index of next higher string */
                    : result;  /* index of match, or before first, or
                                  after last */
  }
  return result;
}

/* FIXME: This routine is painfully inefficient.  It actually recopies
   (read malloc and free ferociously) the strings all the way down
   the line to make an insertion.  This can be easily cured with 
   some pointer swapping instead. -- ??? */
void insert_in_sorted_string_array(string_array *sa,const char *s)
{
  int i = 0;
  int size = string_array_size(sa);
  while ( i < size && strcmp(string_array_ref(sa,i),s) < 0 )
    i += 1;
  if ( i == size )
    add_to_string_array(sa,s);
  else
  {
    int j;
    add_to_string_array(sa,"");
    for ( j = size-1 ; j >= i ; j-- )
      string_array_set(sa,j+1,string_array_ref(sa,j));
    string_array_set(sa,i,s);
  }
}

void maybe_insert_in_sorted_string_array(string_array *sa,const char *s)
{
  if ( find_index_in_sorted_string_array(sa,s) < 0 )
    insert_in_sorted_string_array(sa,s);
}

/* mk_int_string_array("plop",3) would return { "plop0" , "plop1" , "plop2" } */
string_array *mk_int_string_array(const char *prefix,int size)
{
  string_array *sa = mk_string_array(size);
  int i;
  for ( i = 0 ; i < size ; i++ )
  {
    char *s = mk_printf("%s%d",prefix,i);
    string_array_set(sa,i,s);
    free_string(s);
  }
  return sa;
}

void render_string_matrix(FILE *s,const char *comment,string_matrix *sm)
{
  string_array *sa = mk_tabular_string_array(sm,NULL);
  int i;
  for ( i = 0 ; i < string_array_size(sa) ; i++ )
    fprintf(s,"%s%s\n",comment,string_array_ref(sa,i));
  free_string_array(sa);
}

/* See comments for mk_tabular_string_array_simple regaring the meaning
   of "seps". */
void render_string_matrix_with_seps(FILE *s,string_matrix *sm,const char *seps)
{
  string_array *sa = mk_tabular_string_array_simple(sm,seps);
  int i;
  for ( i = 0 ; i < string_array_size(sa) ; i++ )
    fprintf(s,"%s\n",string_array_ref(sa,i));
  free_string_array(sa);
}

/* See comments for mk_tabular_string_array_simple regaring the meaning
   of "seps". 

   If the first string on a line is a single dash, then this version
   draws a complete line of dashes for the full width of the string
   matrix output. 
*/
void render_string_matrix_with_seps_and_dashes(FILE *s,string_matrix *sm,
					       const char *seps)
{
  string_array *sa = mk_tabular_string_array_simple(sm,seps);
  int num_dashes = (string_matrix_rows(sm) < 1) ? 80 :
                   (int)strlen(string_array_ref(sa,0));
  int i;
  for ( i = 0 ; i < string_array_size(sa) ; i++ )
  {
    if ( eq_string(string_matrix_ref(sm,i,0),"-") )
      fprint_dashes(s,num_dashes);
    else
      fprintf(s,"%s\n",string_array_ref(sa,i));
  }
  free_string_array(sa);
}

void string_matrix_real_set(string_matrix *sm,int row,int col,double value)
{
  char buff[100];
  sprintf(buff,"%g",value);
  string_matrix_set(sm,row,col,buff);
}

string_array *mk_string_array_from_line(FILE *s)
{
  char *st = mk_string_from_line(s);
  string_array *sa;
  if ( st == NULL )
    sa = NULL;
  else
    sa = mk_broken_string(st);

  if ( st != NULL ) am_free_string(st);
  return(sa);
}

void string_matrix_row_from_broken_string(string_matrix *sm, int row, const char *row_string)
{
  string_array *sa = mk_broken_string(row_string);
  int i;
  if ( string_array_size(sa) != string_matrix_cols(sm) )
    my_error("sm from broken row: wrong size");
  for ( i = 0 ; i < string_array_size(sa) ; i++ )
    string_matrix_set(sm,row,i,string_array_ref(sa,i));
  free_string_array(sa);
}

string_array *parse_string_array(string_array *sa, const char *sep)
{
  return parse_string_array_endpoints(sa, sep, 0, string_array_size(sa));
}

string_array *parse_string_array_endpoints(string_array *sa, const char *sep, int start, int end) {
  int i;
  ivec *subset = mk_ivec(0);
  char *string;
  string_array *temp_sa;
  string_array *result = mk_string_array(0);

  start = (start <0)?0:start;
  end = (end <string_array_size(sa))?end:string_array_size(sa);

  for (i=start; i<end; i++) {
    if (eq_string(string_array_ref(sa, i), sep)) {
      temp_sa = mk_string_array_subset(sa, subset);
      string =  mk_string_from_string_array(temp_sa);
      add_to_string_array(result, string);

      free_string(string);
      free_string_array(temp_sa);      
      free_ivec(subset);
      subset = mk_ivec(0);

    } else {
      add_to_ivec(subset, i);
    }
  }
  
  if (ivec_size(subset) > 0) {
    temp_sa = mk_string_array_subset(sa, subset);
    string =  mk_string_from_string_array(temp_sa);
    add_to_string_array(result, string);
    
    free_string(string);
    free_string_array(temp_sa);      
  }
  free_ivec(subset);
  return result;
}

string_array *mk_string_array_from_set_notation(const char *setnot)
{
  char *s = mk_copy_string(setnot);
  string_array *sa;
  int i;
  for ( i = 0 ; s[i] != '\0' ; i++ )
  {
    char c = s[i];
    if ( c == '[' || c == ']' || c == '{' || c == '}' || c == ',' )
      s[i] = ' ';
  }
  sa = mk_broken_string(s);
  free_string(s);
  return sa;
}

/************* NEW LINE PARSING CODE *************/

/* If line_format is WHITESPACE then the line is read SPACE_STYLE
   if line_format is COMMA      then the line is read COMMA_STYLE
   if line_format is BAR        then the line is read BAR_STYLE
   if lineformat is  ANY        then

        count the number of unquoted commas on the line (n_comma)
        and count the number of unquoted bars on the line (n_bar)

        if ( n_comma == 0 && n_bar == 0 ) use SPACE_FORMAT
        if ( n_comma >= n_bar )           use COMMA_FORMAT
        if ( n_bar > n_comma )            use BAR_FORMAT

   The line parser runs through a finite state machine. On
   each character it looks at the character type:

     S Space       - The character is the ' ' char
     C Comma       - The character is the ',' char
     A SingleQuote - The character is the '\'' char
     Q DoubleQuote - The character is the '\"' char
     T Token       - The character is something other than the above
     
   The line parser is building up an array of tokens. It begins with
   an empty array of tokens. It has a current token being built. It begins
   with the current token empty. After each character is read, it performs
   one of the following actions:

     ADD   Add the curent token to the array. Set the current token to empty
     PUSH  Put the current character at the end of the current token
     NIL   Do nothing
     DASH  Put a dash character at the end of the current token
     DP    Put a dash, then the current character at end of token
     UNKN  Add the UNKNOWN_STRING to the array. Clear current token


  COMMA_STYLE parsing:

       All whitespace to immediate left and right of commas is removed.
       All other contiguous blocks of whitespace are replaced with - symbols
         (outside quotes, N contiguous spaces are replaced with one -.
          inside quotes, N contiguous spaces are replaced with N -'s)
       The resulting tokens between commas are used.
       Empty string between commas is turned into UNKNOWN STRING

  BAR_STYLE parsing:
       Just like commas, except use bar (|) symbol instead of , symbol

  SPACE_STYLE parsing:

       All whitespace inside quotes are turned to dashes
       All other CONTIGUOUS blocks of whitespace are collapsed to one space
       Then the resulting tokens between whitespaces are used.
*/

/* Returns TRUE iff all characters in line_string are c (note special
   case: returns TRUE if string hads length zero) */
bool line_contains_only_character_c(const char *line_string,char c)
{
  bool result = TRUE;
  int i;
  for ( i = 0 ; result && line_string[i] != '\0' ; i++ )
    result = line_string[i] == c;
  return result;
}

/* Returns TRUE iff all characters in line_string are '-' (note special
   case: returns TRUE if string hads length zero) */
bool line_contains_only_dashes(const char *line_string)
{
  return line_contains_only_character_c(line_string,'-');
}

/* A line is interesting if its not all white space and
the leftmost non whitespace character isnt # */
bool line_string_is_interesting(const char *line_string)
{
  int i;
  char first_non_whitespace = ' ';
  char second_non_whitespace = ' ';
  bool result;

  for ( i = 0 ; first_non_whitespace == ' ' && line_string[i] != '\0' ; i++ )
  {
    if ( line_string[i] != ' ' && line_string[i] != '\t' && 
         line_string[i] != '\r' )
      first_non_whitespace = line_string[i];
    if (first_non_whitespace != '\0') second_non_whitespace = line_string[i+1];
  }
  result = ( first_non_whitespace != ' ' );
  /* we allow the special sequence '##' to be a "machine readable comment */
  if ((first_non_whitespace == '#') && (second_non_whitespace != '#'))
    result = FALSE;

  if ( result && line_contains_only_dashes(line_string) )
    result = FALSE;

  return(result);
}

/* Searches the file for the next line that isn't all whitespace and
   that doesn't have # as its first non-whitespace character. 

   If no-such line before file-end, returns NULL */
char *mk_next_interesting_line_string(FILE *s,int *line_number)
{
  char *line_string = NULL;
  bool finished = FALSE;

  while ( !finished )
  {
    line_string = mk_string_from_line(s);
    *line_number += 1;
    if ( line_string == NULL )
      finished = TRUE;
    else
      finished = line_string_is_interesting(line_string);

    if ( !finished && line_string != NULL )
    {
      free_string(line_string);
      line_string = NULL;
    }
  }

  return(line_string);
}

/* As above excepts breaks resulting line into a string array of tokens... */
string_array *mk_next_interesting_line(FILE *s,int *line_number)
{
  char *str = mk_next_interesting_line_string(s,line_number);
  string_array *sa = (str == NULL) ? NULL : mk_broken_string(str);
  if ( str != NULL ) free_string(str);
  return(sa);
}

bool contains_a_number(string_array *sa)
{
  bool result = FALSE;
  int i;
  for ( i = 0 ; !result && i < string_array_size(sa) ; i++ )
    result = is_a_number(string_array_ref(sa,i));
  return(result);
}

#define UQ_START    0
#define UQ_MIDDLE   1
#define UQ_INSIDE_Q 2
#define UQ_INSIDE_A 3
#define UQ_STOP     4

void line_count_unquoted(const char *string,
			 int *r_num_unquoted_commas,
			 int *r_num_unquoted_bars)
{
  int state = UQ_START;
  int i = 0;

  *r_num_unquoted_commas = 0;
  *r_num_unquoted_bars = 0;

  while ( state != UQ_STOP )
  {
    char c = string[i];
    if ( c == '\0' )
      state = UQ_STOP;
    else
    {
      switch ( state )
      {
        case UQ_START:
          if ( c == ' ' ) state = UQ_START;
          else if ( c == '\"' ) state = UQ_INSIDE_Q;
          else if ( c == '\'' ) state = UQ_INSIDE_A;
          else if ( c == ',' )
          {
	    *r_num_unquoted_commas += 1;
	    state = UQ_START;
	  }
          else if ( c == '|' )
          {
	    *r_num_unquoted_bars += 1;
	    state = UQ_START;
	  }
          else state = UQ_MIDDLE;
        break;
        case UQ_MIDDLE:
          if ( c == ' ' ) state = UQ_START;
          else if ( c == ',' )
          {
	    *r_num_unquoted_commas += 1;
	    state = UQ_START;
	  }
          else if ( c == '|' )
          {
	    *r_num_unquoted_bars += 1;
	    state = UQ_START;
	  }
          else state = UQ_MIDDLE;
        break;
        case UQ_INSIDE_A:
          if ( c == '\'' ) state = UQ_START;
          else state = UQ_INSIDE_A;
        break;
        case UQ_INSIDE_Q:
          if ( c == '\"' ) state = UQ_START;
          else state = UQ_INSIDE_Q;
        break;
        default: my_error("wiudbiuwb"); break;
      }
    }
    i += 1;
  }
}

string_array *mk_parse_data_line(const char *string,int line_format)
{
  char separator;

  switch ( line_format )
  {
    case WHITESPACE_FORMAT: separator = ' '; break;
    case COMMA_FORMAT: separator = ','; break;
    case BAR_FORMAT: separator = '|'; break;
    case AUTO_FORMAT:
    {
      int num_unquoted_commas;
      int num_unquoted_bars;
      line_count_unquoted(string,&num_unquoted_commas,&num_unquoted_bars);
      if ( num_unquoted_commas == 0 && num_unquoted_bars == 0 )
	separator = ' ';
      else if ( num_unquoted_commas >= num_unquoted_bars )
	separator = ',';
      else
	separator = '|';

      break;
    }
    default:
      separator = '?';
      my_error("Unknown line_format");
      break;
  }

  return mk_string_array_from_parse(string,separator);
}

string_array *mk_next_tokens(FILE *s,int *line_number,int line_format)
{
  char *line_string = mk_next_interesting_line_string(s,line_number);
  string_array *tokens;
  
  if ( line_string == NULL )
    tokens = NULL;
  else
    tokens = mk_parse_data_line(line_string,line_format);

  if ( line_string != NULL )
    free_string(line_string);

  return(tokens);
}

string_array *mk_default_attribute_names(int num_atts)
{
  string_array *sa = mk_string_array(num_atts);
  int i;
  for ( i = 0 ; i < num_atts ; i++ )
  {
    char buff[100];
    sprintf(buff,"x%d",i+1);
    string_array_set(sa,i,buff);
  }
  return(sa);
} 

bool all_numeric(string_array *sa)
{
  int i;
  bool result = TRUE;
  for ( i = 0 ; result && i < string_array_size(sa) ; i++ )
    result = is_a_number(string_array_ref(sa,i));
  return result;
}

/* Return TRUE if and only if all items in sa can be parsed as numbers */
bool are_numbers(string_array *sa)
{
  bool result = TRUE;
  int i;

  for ( i = 0 ; result && i < string_array_size(sa) ; i++ )
  {
    if ( !is_a_number(string_array_ref(sa,i)) )
      result = FALSE;
  }

  return result;
}

void fprint_string_array_csv(FILE *s,string_array *x)
{
  int i;
  int size = string_array_size(x);

  for ( i = 0 ; i < size ; i++ )
    fprintf(s,"%s%s",string_array_ref(x,i),(i==size-1)?"\n":",");
}

/* private struct */
typedef struct parse_state
{
  int id;
  int s_action;  int s_next;
  int c_action;  int c_next;
  int a_action;  int a_next;
  int q_action;  int q_next;
  int t_action;  int t_next;
  int end_action;
} parse_state;

#define CGO  0
#define C1   1
#define C2   2
#define CQS  3
#define CQ   4
#define CAS  5
#define CA   6
#define SGO  7
#define S1   8
#define SQST 9
#define SQ   10
#define SAST 11
#define SA   12

#define ADD  0
#define PUSH 1
#define NIL  2
#define DASH 3
#define DP   4
#define UNKN 5
parse_state Parse_array[] =
/* State   S(act,next)  C(act,next)  A(act,next)  Q(act,next)  T(act,next) END(act)*/
{{ CGO ,   NIL ,CGO  ,  UNKN,CGO  ,  NIL ,CAS  ,  NIL ,CQS  ,  PUSH,C1   , UNKN},
 { C1  ,   NIL ,C2   ,  ADD ,CGO  ,  PUSH,C1   ,  PUSH,C1   ,  PUSH,C1   , ADD },
 { C2  ,   NIL ,C2   ,  ADD ,CGO  ,  DP  ,C1   ,  DP  ,C1   ,  DP  ,C1   , ADD },
 { CQS ,   DASH,CQ   ,  PUSH,CQ   ,  PUSH,CQ   ,  NIL ,CGO  ,  PUSH,CQ   , UNKN},
 { CQ  ,   DASH,CQ   ,  PUSH,CQ   ,  PUSH,CQ   ,  NIL ,C1   ,  PUSH,CQ   , ADD },
 { CAS ,   DASH,CA   ,  PUSH,CA   ,  NIL ,CGO  ,  PUSH,CA   ,  PUSH,CA   , UNKN},
 { CA  ,   DASH,CA   ,  PUSH,CA   ,  NIL ,C1   ,  PUSH,CA   ,  PUSH,CA   , ADD },
 { SGO ,   NIL ,SGO  ,  PUSH,S1   ,  NIL ,SAST ,  NIL ,SQST ,  PUSH,S1   , NIL },
 { S1  ,   ADD ,SGO  ,  PUSH,S1   ,  PUSH,S1   ,  PUSH,S1   ,  PUSH,S1   , ADD },
 { SQST,   DASH,SQ   ,  PUSH,SQ   ,  PUSH,SQ   ,  UNKN,SGO  ,  PUSH,SQ   , UNKN},
 { SQ  ,   DASH,SQ   ,  PUSH,SQ   ,  PUSH,SQ   ,  ADD ,SGO  ,  PUSH,SQ   , ADD },
 { SAST,   DASH,SA   ,  PUSH,SA   ,  UNKN,SA   ,  PUSH,SA   ,  PUSH,SA   , UNKN},
 { SA  ,   DASH,SA   ,  PUSH,SA   ,  ADD ,SA   ,  PUSH,SA   ,  PUSH,SA   , ADD }};

string_array *mk_string_array_from_parse(const char *s,char separator)
{
  bool space_separated = separator == ' ';
  int state = (space_separated) ? SGO : CGO;
  bool finished = FALSE;
  int s_ptr = 0;
  string_array *tokarray = mk_string_array(0);
  int currtok_size = strlen(s) + 1;
  char *currtok = AM_MALLOC_ARRAY(char,currtok_size);
  int currtok_ptr = 0;

  while ( !finished )
  {
    parse_state *ps = &(Parse_array[state]);
    char c = s[s_ptr];
    int action;
    int next;

    if ( state != ps->id ) my_error("Parse_array misspecified");

    if ( c == '\0' )
    {
      finished = TRUE;
      next = -1;
      action = ps->end_action;
    }
    else if ( c == ' '  ) { action = ps->s_action ; next = ps->s_next; }
    else if ( c == '\t'  ) { action = ps->s_action ; next = ps->s_next; }
    else if ( c == separator ) { action = ps->c_action ; next = ps->c_next; }
    else if ( c == '\'' ) { action = ps->a_action ; next = ps->a_next; }
    else if ( c == '\"' ) { action = ps->q_action ; next = ps->q_next; }
    else                  { action = ps->t_action ; next = ps->t_next; }

    switch ( action )
    {
      case ADD :
        currtok[currtok_ptr] = '\0';
        add_to_string_array(tokarray,currtok);
        currtok_ptr = 0;
      break;
      case PUSH:
        currtok[currtok_ptr] = c;
        currtok_ptr += 1;
      break;
      case NIL :
        /* skip */
      break;
      case DASH:
        currtok[currtok_ptr] = '_';
        currtok_ptr += 1;
      break;
      case DP  :
        currtok[currtok_ptr] = '_';
        currtok_ptr += 1;
        currtok[currtok_ptr] = c;
        currtok_ptr += 1;
      break;
      case UNKN:
        add_to_string_array(tokarray,"?");
        currtok_ptr = 0;
      break;
      default: my_error("ljdnlkjs"); break;
    }

    state = next;
    s_ptr += 1;
  }

  AM_FREE_ARRAY(currtok,char,currtok_size);
  return(tokarray);
}

/* Returns a string array of values for the given key, from the CL.
   Usually the sa will just have one entry, but
   for quoted key values, splits it into the string array, also,
   [ and ] and "" act as generalized quotes for this purpose. */
string_array *mk_string_array_from_args(const char *key,int argc,char** argv)
{
  string_array* returner;

  returner = mk_maybe_string_array_from_args(key, argc, argv);
  if(returner == NULL)
    returner = mk_string_array(0);

  return returner;
}


string_array *mk_maybe_string_array_from_args(const char *key,int argc,char** argv)
{
  int idx = index_of_arg(key,argc,argv);
  string_array *sa;

  if ( idx < 0 || idx >= argc-1 )
    return NULL;

  else
  {
    char *token = argv[idx+1];
    bool many = token[0] == '[';
    if ( !many )
      sa = mk_string_array_from_set_notation(token);
    else
    {
      string_array *collect = mk_string_array_1(token);
      char *s;
      bool finished = FALSE;
      int i;
      for ( i = idx+1 ; i < argc && !finished ; i++ )
      {
	char *next = argv[i];
	int size = (int) strlen(next);
	add_to_string_array(collect,next);
	finished = size > 0 && next[size-1] == ']';
      }
      s = mk_string_from_string_array(collect);
      sa = mk_string_array_from_set_notation(s);
      free_string_array(collect);
      free_string(s);
    }
  }
  return sa;
}

string_array *mk_string_array_from_argc_argv(int argc,char *argv[])
{
  string_array *sa = mk_string_array(argc);
  int i;
  for ( i = 0 ; i < argc ; i++ )
    string_array_set(sa,i,argv[i]);
  return(sa);
}

/* Makes and returns a string array of given size in which
   every entry contains a copy of the string stored in value */
string_array *mk_constant_string_array(int size,const char *value)
{
  string_array *sa = mk_string_array(size);
  int i;
  for ( i = 0 ; i < size ; i++ )
    string_array_set(sa,i,value);
  return sa;
}

/* Parse a boolean from a string array. */
bool bool_from_argarray( const char *key, string_array *argarray, bool defval)
{
  bool result;
  int fakeargc, i;
  char **fakeargv;

  fakeargc = string_array_size( argarray);
  fakeargv = mk_array_from_string_array( argarray);
  result = bool_from_args( key, fakeargc, fakeargv, defval);

  for( i=0; i<fakeargc; ++i) free_string( fakeargv[i]);
  AM_FREE_ARRAY( fakeargv, char *, fakeargc);

  return result;
}

/* Parse a int from a string array. */
int int_from_argarray( const char *key, string_array *argarray, int defval)
{
  int result, fakeargc, i;
  char **fakeargv;

  fakeargc = string_array_size( argarray);
  fakeargv = mk_array_from_string_array( argarray);
  result = int_from_args( key, fakeargc, fakeargv, defval);

  for( i=0; i<fakeargc; ++i) free_string( fakeargv[i]);
  AM_FREE_ARRAY( fakeargv, char *, fakeargc);

  return result;
}

/* Parse a double from a string array. */
double double_from_argarray( const char *key, string_array *argarray,
                             double defval)
{
  int fakeargc, i;
  double result;
  char **fakeargv;

  fakeargc = string_array_size( argarray);
  fakeargv = mk_array_from_string_array( argarray);
  result = double_from_args( key, fakeargc, fakeargv, defval);

  for( i=0; i<fakeargc; ++i) free_string( fakeargv[i]);
  AM_FREE_ARRAY( fakeargv, char *, fakeargc);

  return result;
}

/* Parse a string from a string array. */
char *mk_string_from_argarray( const char *key, string_array *argarray,
			       const char *defval)
{
  int size, idx;
  char *result;

  size = string_array_size( argarray);
  idx = find_index_in_string_array( argarray, key);

  if (idx == size-1) {
    my_errorf( "mk_string_from_argarray: key '%s' found at end of arguments\n",
	       "but we require a value after the key.",
	       key);
  }

  if (idx < 0) result = mk_copy_string( defval);
  else result = mk_copy_string( string_array_ref( argarray, idx+1));

  return result;
}

/* Split argument array into our options and extra options.  This
   allows one project to parse its options, then pass the unparsed
   options to another project's parse function. */
void mk_split_option_array( string_array *keys_with_args,
                            string_array *keys_without_args,
                            string_array *argarray,
                            string_array **my_opts,
                            string_array **extra_opts)
{
  int i, numargs;
  char *s;
  string_array *sa_mine, *sa_extra;
  
  numargs = string_array_size( argarray);

  /* Arrays to hold our args and extra args. */
  sa_mine = mk_string_array( 0);
  sa_extra = mk_string_array( 0);

  /* Sort args between ours (i.e. one of the keys listed above), and not
     ours.  Note that an extra options which takes an arg will be copied
     in-order to the sa_extra array, unless the arg val is the same as
     one of our keys. */
  for (i=0; i<numargs; ++i) {
    /* Token. */
    s = string_array_ref( argarray, i);

    if (string_array_member( keys_without_args, s)) {
      /* Token is lonely. */
      add_to_string_array( sa_mine, s);
    }

    else if (string_array_member( keys_with_args, s)) {
      /* Token should be followed by argument. */
      add_to_string_array( sa_mine, s);
      if (i+1 == numargs) {
        /* Print error if argument is missing. */
        my_errorf( "mk_active_train_option_arrays:\n"
                   "option '%s' is missing its required argument\n",
                   s);
      }
      s = string_array_ref( argarray, i+1);
      add_to_string_array( sa_mine, s);

      /* Increment i an extra time. */
      i = i + 1;
    }
    else {
      /* Extra arg. */
      add_to_string_array( sa_extra, s);
    }
  }

  /* Assign arg arrays to my_opts and extra_opts. */
  *my_opts = sa_mine;
  *extra_opts = sa_extra;

  return;
}



/*----------------------------------------------------------------*/

string_array *mk_split_string(const char *str, const char *delimiter)
{
    string_array *ret = mk_string_array(0);
    const char *begin = str;
    const char *c = str;

    if (!str || !delimiter || strlen(delimiter) == 0) return ret;
        
    while ((c = strstr(begin, delimiter)))
    {
	if (c - begin == 0) /*two delimiters in a row.  Add an empty string*/
	{
	    string_array_add(ret, "");
	}
	else /*add the sub string*/
	{
	    char *sub_str = AM_MALLOC_ARRAY(char, c-begin+1);
	    strncpy(sub_str, begin, c-begin);
	    sub_str[c-begin] = '\0';
	    
	    string_array_add(ret, sub_str);
	    AM_FREE_ARRAY(sub_str, char, c-begin+1);
	}
	c+= strlen(delimiter);
	begin = c;
    }
    if (!begin) /*str ended in a delimeter.  Add an empty string*/
    {
	string_array_add(ret, "");
    }
    else /*add the last string*/
    {
	char *tmp = mk_copy_string(begin);
	string_array_add(ret, tmp);
	free_string(tmp);
    }

    return ret;
}


char *mk_join_string_array(const string_array *sa, const char *delimiter)
{
    char *ret = mk_copy_string("");
    char *tmp;
    int i;
    int size = string_array_size(sa);
    for (i = 0; i < size; i++)
    {
	const char *delim = (i == 0) ? "" : delimiter;

	tmp = mk_printf("%s%s%s", ret, delim, string_array_ref(sa, i));
	free_string(ret);
	ret = tmp;
    }
    return ret;
}

/*----------------------------------------------------------------*/

/***************** sosarray ***************/

bool string_less(const char *s1,const char *s2)
{
  return strcmp(s1,s2) < 0;
}

bool string_greater(const char *s1,const char *s2)
{
  return string_less(s2,s1);
}

bool string_leq(const char *s1,const char *s2)
{
  return !string_greater(s1,s2);
}

bool string_geq(const char *s1,const char *s2)
{
  return !string_less(s1,s2);
}

/* A sosarray is a regular old string_array, except it is treated as a set of
   integers.

   An string_array is a legal sosarray if it is sorted in increasing order with no
   duplicates.

   The following set of functions consititute a reasonable simple
   package of set-theory operations.

   Note that everything is as efficient as possible for a set package 
   except for adding a single element and deleting a single element, 
   which (because of our representation by means of sorted string_arrays) could
   take time linear in set size. */

bool is_sosarray(string_array *sa)
{
  int size = string_array_size(sa);
  int i;
  bool result = TRUE;
  for ( i = 0 ; result && i < size-1 ; i++ )
    if ( string_geq(string_array_ref(sa,i),string_array_ref(sa,i+1)) )
      result = FALSE;
  return result;
}

#ifdef AMFAST
#define debugger_assert_is_sosarray(sosarr)
#else
void debugger_assert_is_sosarray(string_array *sosarr); /* Move this to top? */

void debugger_assert_is_sosarray(string_array *sosarr)
{
  if ( !is_sosarray(sosarr) )
  {
    fprintf_string_array(stdout,"sosarr",sosarr,"\n");
    printf("The above string_array was not a sosarray (sorted string array) because it was not\n"
           "sorted. It was passed to a sosarray operation in amdmex.c\n"
           "Use the debugger to find the ofending call\n");
  }
}
#endif

/* Returns number of elements in sosarray */
int sosarray_size(string_array *sosarr)
{
  return string_array_size(sosarr);
}

char *sosarray_first(string_array *sosarr)
{
  return string_array_ref(sosarr,0);
}

char *sosarray_last(string_array *sosarr)
{
  return string_array_ref(sosarr,string_array_size(sosarr)-1);
}

/* If sosarr has 0 elements returns 0
   If value > string_array_max(sosarr) returns size
   If value <= string_array_min(sosarr) returns 0
   Else returns index such that
      value <= string_array_ref(sosarr,index) 
      string_array_ref(sosarr,index-1) < value
      
   It returns the value such that string_array_insert(sa,index,value)
   would represent the set with value added to sa (assuming value
   wasn't already in sa). */
int find_sosarray_insert_index(string_array *sosarr,const char *string)
{
  int size = string_array_size(sosarr);
  int result;

  debugger_assert_is_sosarray(sosarr);

  if ( size == 0 )
    result = 0;
  else
  {
    char *loval = string_array_ref(sosarr,0);
    if ( string_leq(string,loval) ) 
      result = 0;
    else
    {
      int lo = 0;
      int hi = size-1;
      char *hival = string_array_ref(sosarr,hi);
      if ( string_greater(string,hival) )
	      result = size;
      else
      {
        while ( hi > lo + 1 )
        {
          int mid = (lo + hi) / 2;
          char *midval = string_array_ref(sosarr,mid);
          if ( string_less(midval,string) )
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
        if ( eq_string(loval,string) )
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
    else if ( string_greater(string,sosarray_last(sosarr)) )
      ok = result == size;
    else if ( string_leq(string,sosarray_first(sosarr)) )
      ok = result == 0;
    else
      ok = string_leq(string,string_array_ref(sosarr,result)) &&
           string_less(string_array_ref(sosarr,result-1),string);

    if ( !ok )
      my_error("find_sosarray_insert_index() : bug\n");
  }
#endif

  return result;
}

/* Adds the element while maintaining legal sosarraykiness.
   (If element already there, no change)
   Time cost: O(size) */
void add_to_sosarray(string_array *sosarr,const char *string)
{
  int insert_index = find_sosarray_insert_index(sosarr,string);
  if ( insert_index >= string_array_size(sosarr) )
    add_to_string_array(sosarr,string);
  else if ( !eq_string(string,string_array_ref(sosarr,insert_index)) )
    insert_in_string_array(sosarr,insert_index,string);
}

/* Returns -1 if the string does not exist in sosarr.
   Else returns index such that
      string == string_array_ref(sosarr,string) 
  Time cost: O(log(size)) */
int index_in_sosarray(string_array *sosarr,const char *string)
{
  int idx = find_sosarray_insert_index(sosarr,string);
  if ( idx >= string_array_size(sosarr) || 
       !eq_string(string,string_array_ref(sosarr,idx)) )
    idx = -1;
  return idx;
}

/* Returns true iff sosarr contains string
   Time cost: O(log(size)) */
bool is_in_sosarray(string_array *sosarr,const char *string)
{
  return index_in_sosarray(sosarr,string) >= 0;
}

void sosarray_remove_at_index(string_array *sosarr,int idx)
{
  string_array_remove(sosarr,idx);
}

/* Does nothing if string is not in sosarr.
   If string is in sosarr, the sosarray is updated to
   represent sosarr \ { string } */
void sosarray_remove_string(string_array *sosarr,const char *string)
{
  int idx = find_sosarray_insert_index(sosarr,string);
  if ( idx < string_array_size(sosarr) && eq_string(string,string_array_ref(sosarr,idx)) )
    string_array_remove(sosarr,idx);
}
  
/* Returns answer to A subset-of B?
   Returns true if and only if the set of integers in a is
   a subset of the set of integers in b */
bool sosarray_subset(string_array *sosarra,string_array *sosarrb)
{
  bool result = string_array_size(sosarra) <= string_array_size(sosarrb) ;
  int ai = 0;
  int bi = 0;
  debugger_assert_is_sosarray(sosarra);
  debugger_assert_is_sosarray(sosarrb);

  while ( result && ai < string_array_size(sosarra) )
  {
    char *aval = string_array_ref(sosarra,ai);
    while ( string_array_ref(sosarrb,bi) < aval && bi < string_array_size(sosarrb) )
      bi += 1;
    if ( bi >= string_array_size(sosarrb) || 
	     string_greater(string_array_ref(sosarrb,bi),aval) )
      result = FALSE;
    else
      ai += 1;
  }
  return result;
}

bool equal_string_array(string_array *sa1,string_array *sa2)
{
  int size = string_array_size(sa1);
  bool result = size = string_array_size(sa2);
  int i;
  for ( i = 0 ; result && i < size ; i++ )
    result = eq_string(string_array_ref(sa1,i),string_array_ref(sa2,i));
  return result;
}

bool sosarray_equal(string_array *sosarra,string_array *sosarrb)
{
  return equal_string_array(sosarra,sosarrb);
}

/* Returns TRUE iff A is a subset of B and A != B */
bool sosarray_strict_subset(string_array *sosarra,string_array *sosarrb)
{
  return sosarray_subset(sosarra,sosarrb) && !sosarray_equal(sosarra,sosarrb);
}

string_array *mk_sosarray_union(string_array *sosarra,string_array *sosarrb)
{
  string_array *result = mk_string_array(0);
  int ai = 0;
  int bi = 0;
  debugger_assert_is_sosarray(sosarra);
  debugger_assert_is_sosarray(sosarrb);
  while ( ai < string_array_size(sosarra) && bi < string_array_size(sosarrb) )
  {
    char *aval = string_array_ref(sosarra,ai);
    char *bval = string_array_ref(sosarrb,bi);
    if ( eq_string(aval,bval) )
    {
      add_to_string_array(result,aval);
      ai += 1;
      bi += 1;
    }
    else if ( string_less(aval,bval) )
    {
      add_to_string_array(result,aval);
      ai += 1;
    }
    else
    {
      add_to_string_array(result,bval);
      bi += 1;
    }
  }

  while ( ai < string_array_size(sosarra) )
  {
    add_to_string_array(result,string_array_ref(sosarra,ai));
    ai += 1;
  }

  while ( bi < string_array_size(sosarrb) )
  {
    add_to_string_array(result,string_array_ref(sosarrb,bi));
    bi += 1;
  }

  debugger_assert_is_sosarray(result);
  return result;
}

/* Returns A \ B.
   This is { x : x in A and x not in B } */
string_array *mk_sosarray_difference(string_array *sosarra,string_array *sosarrb)
{
  string_array *result = mk_string_array(0);
  int ai = 0;
  int bi = 0;
  debugger_assert_is_sosarray(sosarra);
  debugger_assert_is_sosarray(sosarrb);
  while ( ai < string_array_size(sosarra) && bi < string_array_size(sosarrb) )
  {
    while ( ai < string_array_size(sosarra) && 
	        string_less(string_array_ref(sosarra,ai),string_array_ref(sosarrb,bi)) )
    {
      add_to_string_array(result,string_array_ref(sosarra,ai));
      ai += 1;
    }
    if ( ai < string_array_size(sosarra) )
    {
      while ( bi < string_array_size(sosarrb) && 
	          string_less(string_array_ref(sosarrb,bi),string_array_ref(sosarra,ai)) )
        bi += 1;
      while ( ai < string_array_size(sosarra) && 
	      bi < string_array_size(sosarrb) && 
	      eq_string(string_array_ref(sosarra,ai),string_array_ref(sosarrb,bi)) )
      {
	    ai += 1;
	    bi += 1;
      }
    }
  }
  while ( ai < string_array_size(sosarra) )
  {
    add_to_string_array(result,string_array_ref(sosarra,ai));
    ai += 1;
  }
  debugger_assert_is_sosarray(result);
  return result;
}

string_array *mk_sosarray_intersection(string_array *sosarra,string_array *sosarrb)
{
  string_array *result = mk_string_array(0);
  int ai = 0;
  int bi = 0;
  debugger_assert_is_sosarray(sosarra);
  debugger_assert_is_sosarray(sosarrb);
  while ( ai < string_array_size(sosarra) && bi < string_array_size(sosarrb) )
  {
    while ( ai < string_array_size(sosarra) && 
	        string_less(string_array_ref(sosarra,ai),string_array_ref(sosarrb,bi)) )
      ai += 1;
    if ( ai < string_array_size(sosarra) )
    {
      while ( bi < string_array_size(sosarrb) && 
	          string_less(string_array_ref(sosarrb,bi),string_array_ref(sosarra,ai)) )
        bi += 1;
      if ( bi < string_array_size(sosarrb) && 
	       eq_string(string_array_ref(sosarra,ai),string_array_ref(sosarrb,bi)) )
      {
	add_to_string_array(result,string_array_ref(sosarra,ai));
	ai += 1;
	bi += 1;
      }
    }
  }
  debugger_assert_is_sosarray(result);
  return result;
}

/* Returns TRUE iff A intersect B is empty. O(size) time */
bool sosarray_disjoint(string_array *sosarra,string_array *sosarrb)
{
  bool result = TRUE;
  int ai = 0;
  int bi = 0;
  debugger_assert_is_sosarray(sosarra);
  debugger_assert_is_sosarray(sosarrb);
  while ( result && ai < string_array_size(sosarra) && bi < string_array_size(sosarrb) )
  {
    while ( ai < string_array_size(sosarra) && 
	        string_less(string_array_ref(sosarra,ai),string_array_ref(sosarrb,bi)) )
      ai += 1;
    if ( ai < string_array_size(sosarra) )
    {
      while ( bi < string_array_size(sosarrb) && 
	          string_less(string_array_ref(sosarrb,bi),string_array_ref(sosarra,ai)) )
        bi += 1;
      if ( bi < string_array_size(sosarrb) && 
	       eq_string(string_array_ref(sosarra,ai),string_array_ref(sosarrb,bi)) )
	result = FALSE;
    }
  }
  return result;
}

string_array *mk_sosarray_from_string_array(string_array *sa)
{
  string_array *sosarr = mk_string_array(0);
  if ( string_array_size(sa) > 0 )
  {
    string_array *sorted = mk_sort_string_array(sa);
    int i;
    char *prev = string_array_ref(sorted,0);
    add_to_string_array(sosarr,prev);
    for ( i = 1 ; i < string_array_size(sorted) ; i++ )
    {
      char *string = string_array_ref(sorted,i);
      if ( string_less(string,prev) ) 
    	my_error("No way");
      else if ( !eq_string(string,prev) )
	    add_to_string_array(sosarr,string);

      prev = string;
    }
    free_string_array(sorted);
  }
  return sosarr;
}

/* Turns a space separated string into a sosarray.
   Example: "3 1 4 1 5 9" ====> { 1 , 3 , 4 , 5 , 9 } */
string_array *mk_sosarray_from_string(const char *s)
{
  string_array *sa = mk_string_array_from_string(s);
  string_array *sosarr = mk_sosarray_from_string_array(sa);
  free_string_array(sa);
  return sosarr;
}

string_array *mk_string_array_from_string(const char *s)
{
  dyv *dv = mk_dyv_from_string(s,NULL);
  string_array *sa = mk_string_array_from_dyv(dv);
  free_dyv(dv);
  return sa;
}

/* Make a nice string for printing with the string_array formed into
   columns.  Set width to the number of characters wide to
   make the output string.  This will determine the number of columns
   created.
*/
char *mk_format_string_array_into_columns(string_array *sa, int width)
{
  int num_strings = sa ? string_array_size(sa) : 0;
  int longest_string = 0;
  int i, j;
  int num_cols = 3;
  int buf_siz = 0;
  char *tmp;
  char *ret = mk_copy_string("");

  if (num_strings == 0) return ret;

  for (i = 0; i < num_strings; i++) {
    int len = strlen(string_array_ref(sa, i));
    if (len > longest_string)
      longest_string = len;
  }
  
  buf_siz = longest_string ? longest_string : 1;
  num_cols = width / buf_siz;
  if (num_cols < 1) num_cols = 1;
  
  if (num_cols >= num_strings)
  {
    /*all on one line, just space them out a little*/
    free_string(ret);
    tmp = mk_join_string_array(sa, "  ");
    ret = mk_printf("  %s", tmp);
    free_string(tmp);
  }
  else
  {
    /*make columns*/
    i = 0;
    while (i < num_strings)
    {
      for (j = 0; j < num_cols && i < num_strings; j++)
      {		
	tmp = mk_printf("%s  %-*s", ret, buf_siz, 
			string_array_ref(sa, i));
	free_string(ret);
	ret = tmp;
	i++;
      }
      tmp = mk_printf("%s\n", ret);
      free_string(ret);
      ret = tmp;
    }
  }
  return ret;
}

/*************** The following stuff is for displaying 
                 string arrays in tabular form **********/

/* Makes an 'LS' style string matrix. That means it takes the
   string array and puts each element into cells of a string
   matrix. The string matrix has "cols" columns. The order in
   which string_array elements are placed is

      sa[0]      sa[r+0]    ....    sa[(cols-1)r+0]
      sa[1]      sa[r+1]    ....    sa[(cols-1)r+1]
        :                                  :
        :                                  :
        :                                  :
      sa[r-1]    sa[2r-1]   ....    sa[(cols-1)r-1]

    where r is the least r such that r*cols >= string_array_size

   ...and some of the rightmost column might be filled with
      empty cells.
*/
string_matrix *mk_ls_style_string_matrix_given_cols(string_array *names,int cols)
{
  int row;
  int size = string_array_size(names);
  int rows = (size + cols - 1) / cols;
  string_matrix *sm = mk_string_matrix(rows,cols);
  for ( row = 0 ; row < rows ; row++ )
  {
    int col;
    for ( col = 0 ; col < cols ; col++ )
    {
      int idx = row + col * rows;
      char *name = (idx < size) ?
                   string_array_ref(names,idx) : "";
      string_matrix_set(sm,row,col,name);
    }
  }
  return sm;
}

/* Returns the max string length in sa */
int string_array_max_length(string_array *sa)
{
  int result = 0;
  int i;
  for ( i = 0 ; i < string_array_size(sa) ; i++ )
    result = int_max(result,(int)strlen(string_array_ref(sa,i)));
  return result;
}

/* Makes an 'LS' style string matrix cleverly designed so that when
   printed it uses less than "max_chars" characters per line. (it auto-chooses
   and sizes the columns) */
string_matrix *mk_ls_style_string_matrix(string_array *names,int max_chars)
{
  bool ok = FALSE;
  string_matrix *prev_sm = NULL;
  int cols = 1;

  if ( string_array_size(names) <= 0 )
    my_error("mk_ls_style_string_matrix_given_cols : empty set of names");

  while ( !ok )
  {
    string_matrix *sm = mk_ls_style_string_matrix_given_cols(names,cols);
    string_array *tabular = mk_tabular_string_array(sm,NULL);
    int chars = string_array_max_length(tabular);
    if ( chars > max_chars )
      ok = TRUE;
    else if ( prev_sm != NULL && string_matrix_rows(prev_sm) <= 1 )
      ok = TRUE;

    if ( cols == 1 || !ok )
    {
      if ( prev_sm != NULL ) free_string_matrix(prev_sm);
      prev_sm = sm;
      cols += 1;
    }
    else
      free_string_matrix(sm);
    free_string_array(tabular);
  }
  return prev_sm;
}

int sm_rows(string_matrix *sm)
{
  return string_matrix_rows(sm);
}

void sm_set_string(string_matrix *sm,int row,int col,const char *string)
{
  while ( string_matrix_rows(sm) <= row )
    string_matrix_add_row(sm);
  string_matrix_set(sm,row,col,string);
}

void sm_set_double(string_matrix *sm,int row,int col,double x)
{
  char *s = mk_printf("%g",x);
  sm_set_string(sm,row,col,s);
  free_string(s);
}

void sm_set_int(string_matrix *sm,int row,int col,double n)
{
  sm_set_double(sm,row,col,(double)n);
}

string_array *mk_append_string_arrays(string_array *a,string_array *b)
{
  string_array *sa = mk_copy_string_array(a);
  int i;
  for ( i = 0 ; i < string_array_size(b) ; i++ )
    add_to_string_array(sa,string_array_ref(b,i));
  return sa;
}

