/*
   File:        hash.c
   Author:      Jeff Schneider
   Created:     Fri Feb 12 15:11:16 EST 1999
   Description: Hash table implementation

   Copyright (C) 1999, Jeff Schneider

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

#include "hash.h"
#include "amma.h"
#include "am_string.h"

static int *mk_copy_int(int i);

/* Headers for private functions */

static int hash_code_from_ivec(ivec *iv, int num_bins);
static int hash_code_from_int(const int *i, int num_bins);
static hlist *lookup_list_entry_node(hash_table *ht, hlist *hl, const char *key);
static hlist *lookup_hash_entry_node(hash_table *ht, const char *key);
static void increase_hash_size(hash_table *ht, int multiple);
static hlist *mk_copy_hlist(const hash_table *ht, const hlist *hl, void *(*data_copy_fn)(void *data));

static void free_hlist(hash_table *ht,hlist *hl,void (*data_free_fn)(void *data));
static void free_hlist_element(hash_table *ht, hlist *hl,
			       void (*data_free_fn)(void *data));




/* This function defaults to a character key type.  See set_hash_table_key_type
   to change this setting. */
static hash_table *mk_sized_empty_hash_table(int size,
				      void *(*data_copy_fn)(void *data),
				      void (*data_free_fn)(void *data),
				      int (*data_num_bytes_fn)(void *data),
				      int key_type)
{
  int i;
  hash_table *ht = AM_MALLOC(hash_table);
  ht->num_bins = size;
  ht->num_elements = 0;
  ht->key_type = key_type;
  ht->data_copy_fn = data_copy_fn;
  ht->data_free_fn = data_free_fn;
  ht->data_num_bytes_fn = data_num_bytes_fn;
  ht->array = AM_MALLOC_ARRAY( hlist *, ht->num_bins);
  for (i=0;i<ht->num_bins;i++) ht->array[i] = NULL;
  return ht;
}

/* key_type should be one of the types #defined in hash.h.  
   WARNING:  it is NOT ok to call this function after you've already begun
   storing data in the hash table.  The mapping to the data you've already
   stored will be lost.
*/
void set_hash_table_key_type(hash_table *ht, int key_type)
{
  if (ht->num_elements > 0)
    my_error("set_hash_table_key_type: don't change key_type after some data is stored in the hash table!\n");
  ht->key_type = key_type;
}

hash_table *mk_empty_hash_table(void *(*data_copy_fn)(void *data),
				void (*data_free_fn)(void *data),
				int (*data_num_bytes_fn)(void *data))
{
  return mk_sized_empty_hash_table(INITIAL_HASH_BINS,
				   data_copy_fn,data_free_fn,
				   data_num_bytes_fn,CHAR_KEY_TYPE);
}

/* Returns an integer called code such that
     0 <= code < num_bins
   designed to be a robust fast hash code for hashing null-terminated
   strings */
int hash_code_from_string(const char *s, int num_bins)
{
  unsigned int h = 0;
  unsigned int g;
  int i;
  int odd_number = (num_bins % 2 == 0) ? num_bins - 1 : num_bins;

  for (i=0;s[i];i++)
  {
    h = (h << 4) + s[i];
    g = h & 0xf0000000;
    if (g)
    {
      h = h ^ (g >> 24);
      h = h ^ g;
    }
  }
  return (h % odd_number);
}

int hash_code_from_ivec(ivec *iv, int num_bins)
{
  unsigned int h = 0;
  unsigned int g;
  int i;
  int odd_number = (num_bins % 2 == 0) ? num_bins - 1 : num_bins;

  for (i=0;i<ivec_size(iv);i++)
  {
    h = (h << 4) + ivec_ref(iv,i);
    g = h & 0xf0000000;
    if (g)
    {
      h = h ^ (g >> 24);
      h = h ^ g;
    }
  }
  return (h % odd_number);
}

int hash_code_from_int(const int *i, int num_bins)
{
  const unsigned int h = * (unsigned int *) i;
  const int odd_number = (num_bins % 2 == 0) ? num_bins - 1 : num_bins;

  return (h % odd_number);
}

hlist *lookup_list_entry_node(hash_table *ht, hlist *hl, const char *key)
{
  while(hl)
  {
    if (ht->key_type == CHAR_KEY_TYPE)
    {
      if (eq_string(key,hl->key)) return hl;
    }
    else if (ht->key_type == IVEC_KEY_TYPE)
    {
      if (ivec_equal((ivec *)key,(ivec *)hl->key)) return hl;
    }
    else if (ht->key_type == INT_KEY_TYPE)
    {
      if (*(int *)key == *(int *)hl->key) return hl;
    }
    else if (ht->key_type == PAULINT_KEY_TYPE)
    {
      if (*(int *)key == *(int *)hl->key) return hl;
    }
    hl = hl->next;
  }
  return NULL;
}

static void hlist_update_sizeof(hlist *hl, auton_sf *x, int key_type,
				int (*data_num_bytes)(void *data))
{
  if ( hl != NULL )
  {
    add_to_auton_sf(x,(unsigned long)sizeof(hlist));

    if ( key_type == CHAR_KEY_TYPE )
      string_update_sizeof(hl->key,x);
    else if ( key_type == IVEC_KEY_TYPE )
      ivec_update_sizeof((ivec *)(hl->key),x);
    else if ( key_type == INT_KEY_TYPE )
      add_to_auton_sf(x,(unsigned long)sizeof(int));
    else if ( key_type == PAULINT_KEY_TYPE )
      add_to_auton_sf(x,(unsigned long)sizeof(int));
    else
      my_errorf("\nERROR: unknown key_type %d in hlist_update_sizeof.",
		key_type);

    add_to_auton_sf(x,data_num_bytes(hl->data));
    hlist_update_sizeof(hl->next,x,key_type,data_num_bytes);
  }
}

hlist *lookup_hash_entry_node(hash_table *ht, const char *key)
{
  int idx = 0;

  if (ht->key_type == CHAR_KEY_TYPE)
    idx = hash_code_from_string(key,ht->num_bins);
  else if (ht->key_type == IVEC_KEY_TYPE)
    idx = hash_code_from_ivec((ivec *)key,ht->num_bins);
  else if (ht->key_type == INT_KEY_TYPE)
    idx = hash_code_from_int((int *)key,ht->num_bins);
  else if (ht->key_type == PAULINT_KEY_TYPE) {
    idx = hash_code_from_int((int *)key,ht->num_bins);
  }
  else
    my_error("lookup_hash_entry_node: invalid hash table key_type\n");
  return lookup_list_entry_node(ht,ht->array[idx],key);
}

void *lookup_hash_entry(hash_table *ht, const char *key)
{
  hlist *node = lookup_hash_entry_node(ht,key);
  if (node) return node->data;
  else      return NULL;
}

void *lookup_hash_entry_by_intkey( hash_table *ht, int intkey)
{
  void *data;
  data = lookup_hash_entry( ht, (char *) &intkey);
  return data;
}

/* Throws my_errorf() on an illegal reference. */
int lookup_int_hash_entry_by_intkey( hash_table *ht, int intkey)
{
  void *data;
  int ival;
  data = lookup_hash_entry( ht, (char *) &intkey);
  if ( data == NULL) {
    my_errorf( "lookup_int_hash_entry_by_intkey: bad intkey %d.", intkey);
  }
  ival = *(int *) data;
  return ival;
}

/* This function increases the number of bins in a hash table by taking the
   following steps:
   1. Create a new, empty, larger hash table
   2. Go through the elements of the old table and re-enter them into the new
   3. Swap the guts of the two structures so the new one lives in the old shell
   4. Free the new structure (which holds the old guts)
 */
void increase_hash_size(hash_table *ht, int multiple)
{
  hash_table *new_ht = mk_sized_empty_hash_table(ht->num_bins*multiple,
						 ht->data_copy_fn,
						 ht->data_free_fn,
						 ht->data_num_bytes_fn,
						 ht->key_type);
  int i,temp;
  hlist **tempa;

  for (i=0;i<ht->num_bins;i++)
  {
    hlist *hl = ht->array[i];
    while(hl)
    {
      add_hash_entry(new_ht,hl->key,hl->data);
      hl=hl->next;
    }
  }
  temp=ht->num_bins;ht->num_bins=new_ht->num_bins;new_ht->num_bins=temp;
  temp=ht->num_elements;ht->num_elements=new_ht->num_elements;new_ht->num_elements=temp;
  tempa=ht->array;ht->array=new_ht->array;new_ht->array=tempa;
  free_hash_table(new_ht);
}

/* The data is copied in */
void add_hash_entry(hash_table *ht, const char *key, void *data)
{
  int idx = -7777;
  hlist *hl;

  if (ht->num_elements > (3 * ht->num_bins)) increase_hash_size(ht,10);

  if (ht->key_type == CHAR_KEY_TYPE)
    idx = hash_code_from_string(key,ht->num_bins);
  else if (ht->key_type == IVEC_KEY_TYPE)
    idx = hash_code_from_ivec((ivec *)key,ht->num_bins);
  else if (ht->key_type == INT_KEY_TYPE)
    idx = hash_code_from_int((int *)key,ht->num_bins);
  else if (ht->key_type == PAULINT_KEY_TYPE)
    idx = hash_code_from_int((int *)key,ht->num_bins);
  else
    my_error("add_hash_entry: invalid hash table key_type\n");

  hl = AM_MALLOC(hlist);
  if (ht->key_type == CHAR_KEY_TYPE) 
    hl->key = mk_copy_string(key);
  else if (ht->key_type == IVEC_KEY_TYPE)
    hl->key = (char *)mk_copy_ivec((ivec *)key);
  else if (ht->key_type == INT_KEY_TYPE)
    hl->key = (char *)mk_copy_int(* (int *)key);
  else if (ht->key_type == PAULINT_KEY_TYPE)
    hl->key = (char *)mk_copy_int(* (int *)key);
  hl->data = ht->data_copy_fn(data);
  hl->next = ht->array[idx];
  ht->array[idx] = hl;
  ht->num_elements++;
}

void add_hash_entry_by_intkey( hash_table *ht, int intkey, void *data)
{
  add_hash_entry( ht, (char *) &intkey, data);
}

/* This works so long as the copy routine actually copies the data. */
void add_int_hash_entry_by_intkey( hash_table *ht, int intkey, int val)
{
  add_hash_entry_by_intkey( ht, intkey, (void *) &val);
}

void *update_hash_entry(hash_table *ht, const char *key, void *data)
{
  hlist *hl = lookup_hash_entry_node(ht,key);
  if (hl)
  {
    ht->data_free_fn(hl->data);
    hl->data = ht->data_copy_fn(data);
    return data;
  }
  else return NULL;
}

void *update_hash_entry_by_intkey( hash_table *ht, int intkey, void *data)
{
  void *samedata;
  samedata = update_hash_entry( ht, (char *) &intkey, data);
  return samedata;
}

hlist *mk_copy_hlist(const hash_table *ht, const hlist *hl,
		     void *(*data_copy_fn)(void *data))
{
  if (hl)
  {
    hlist *new_hl = AM_MALLOC(hlist);
    if (ht->key_type == CHAR_KEY_TYPE)
      new_hl->key = mk_copy_string(hl->key);
    else if (ht->key_type == IVEC_KEY_TYPE)
      new_hl->key = (char *)mk_copy_ivec((ivec *)hl->key);
    else if (ht->key_type == INT_KEY_TYPE)
      new_hl->key = (char *)mk_copy_int(* (int *)hl->key);
    else if (ht->key_type == PAULINT_KEY_TYPE)
      new_hl->key = (char *)mk_copy_int(* (int *)hl->key);
    else
      my_error("mk_copy_hlist: invalid hash table key_type\n");
    new_hl->data = data_copy_fn(hl->data);
    new_hl->next = mk_copy_hlist(ht,hl->next,data_copy_fn);
    return new_hl;
  }
  else return NULL;
}

hash_table *mk_copy_hash_table(const hash_table *ht)
{
  int i;
  hash_table *new_ht = AM_MALLOC(hash_table);
  new_ht->num_bins = ht->num_bins;
  new_ht->num_elements = ht->num_elements;
  new_ht->key_type = ht->key_type;
  new_ht->data_copy_fn = ht->data_copy_fn;
  new_ht->data_free_fn = ht->data_free_fn;
  new_ht->data_num_bytes_fn = ht->data_num_bytes_fn;
  new_ht->array = AM_MALLOC_ARRAY( hlist *, ht->num_bins);
  for (i=0;i<new_ht->num_bins;i++)
    new_ht->array[i] = mk_copy_hlist(ht,ht->array[i],new_ht->data_copy_fn);
  return new_ht;
}

void free_hlist_element(hash_table *ht, hlist *hl,
			void (*data_free_fn)(void *data))
{
  data_free_fn(hl->data);
  if (ht->key_type == CHAR_KEY_TYPE) free_string(hl->key);
  else if (ht->key_type == IVEC_KEY_TYPE) free_ivec((ivec *)hl->key);
  else if (ht->key_type == INT_KEY_TYPE) AM_FREE((int *)hl->key, int);
  else if (ht->key_type == PAULINT_KEY_TYPE) AM_FREE((int *)hl->key, int);
  else my_error("free_hlist_element: invalid hash table key_type\n");
  AM_FREE(hl,hlist);
}

void free_hlist(hash_table *ht,hlist *hl,void (*data_free_fn)(void *data))
{
  if (hl->next) free_hlist(ht,hl->next,data_free_fn);
  free_hlist_element(ht,hl,data_free_fn);
}

static int hlist_num_bytes(hlist *hl,int (*data_num_bytes_fn)(void *data),
			   int key_type)
{
  int result = 0;
  while ( hl != NULL )
  {
    result += sizeof(hlist) + data_num_bytes_fn(hl->data);

    if (key_type == CHAR_KEY_TYPE) 
      result += 1 + (int) strlen(hl->key);
    else if (key_type == IVEC_KEY_TYPE) 
      result += ivec_num_bytes((ivec *) hl->key);
    else if (key_type == INT_KEY_TYPE) 
      result += sizeof(int);
    else if (key_type == PAULINT_KEY_TYPE) 
      result += sizeof(int);
    else 
      my_error("hlist_num_bytes: invalid hash table key_type\n");

    hl = hl->next;
  }

  return result;
}

void hash_table_update_sizeof(hash_table *ht, auton_sf *x)
{
  if ( ht != NULL )
  {
    int i;

    add_to_auton_sf(x,(unsigned long)sizeof(hash_table));
    add_to_auton_sf(x,(unsigned long)(ht->num_bins)*sizeof(hlist *));

    for ( i = 0; i < ht->num_bins; i++ )
      hlist_update_sizeof(ht->array[i],x,ht->key_type,ht->data_num_bytes_fn);
  }
}

void free_hash_table(hash_table *ht)
{
  int i;
  for (i=0;i<ht->num_bins;i++) 
    if (ht->array[i]) free_hlist(ht,ht->array[i],ht->data_free_fn);
  AM_FREE_ARRAY( ht->array, hlist *, ht->num_bins);
  AM_FREE(ht,hash_table);
}

int hash_table_num_bytes(hash_table *ht)
{
  int result = sizeof(hash_table) + ht->num_bins * sizeof(hlist *);
  int i;
  for ( i = 0 ; i < ht->num_bins ; i++ ) 
    result += hlist_num_bytes(ht->array[i],ht->data_num_bytes_fn,
			      ht->key_type);
  return result;
}

int hash_table_num_elements(hash_table *ht)
{
  return ht->num_elements;
}
    
void delete_hash_entry(hash_table *ht, const char *key)
{
  int idx = 0;   
  hlist *prev = NULL;
  hlist *cur = NULL;

  if (ht->key_type == CHAR_KEY_TYPE)
    idx = hash_code_from_string(key,ht->num_bins);
  else if (ht->key_type == IVEC_KEY_TYPE)
    idx = hash_code_from_ivec((ivec *)key,ht->num_bins);
  else if (ht->key_type == INT_KEY_TYPE)
    idx = hash_code_from_int((int *)key,ht->num_bins);
  else if (ht->key_type == PAULINT_KEY_TYPE)
    idx = hash_code_from_int((int *)key,ht->num_bins);
  else
    my_error("delete_hash_entry: invalid hash table key_type\n");

  cur = ht->array[idx];

  while (cur && strcmp(key,cur->key))
  {
    prev = cur;
    cur = cur->next;
  }
  if (cur) /* that means we found it */
  {
    if (prev) prev->next = cur->next;
    else ht->array[idx] = cur->next;
    free_hlist_element(ht,cur,ht->data_free_fn);
    ht->num_elements--;
  }
}

void delete_hash_entry_by_intkey( hash_table *ht, int intkey)
{
  delete_hash_entry( ht, (char *) &intkey);
}


/* A debugging function for those who are concerned about how well the
   hash function and hash table are working.
*/
#define PHTS_HIST_SIZE 6
void print_hash_table_stats(hash_table *ht)
{
  int i;
  ivec *counts = mk_zero_ivec(PHTS_HIST_SIZE);
  printf("%d bins and %d elements => %.2f full\n",
	 ht->num_bins,ht->num_elements,
	 (double)ht->num_elements/(double)ht->num_bins);
  printf("Note: it is reasonable for the fullness to be above or below one,\nbut probably not more than an order of magnitude either way.\n");

  for (i=0;i<ht->num_bins;i++)
  {
    int count = 0;
    hlist *hl = ht->array[i];
    while (hl) {count++; hl=hl->next;}
    if (count < PHTS_HIST_SIZE) ivec_increment(counts,count,1);
    else                        ivec_increment(counts,PHTS_HIST_SIZE-1,1);
  }
  printf("Elements/Bin Percentage_of_bins_with Number_of_bins_with\n");
  for (i=0;i<PHTS_HIST_SIZE;i++)
  {
    if (i<PHTS_HIST_SIZE-1) printf("%8d ",i);
    else                    printf("%7d+ ",i);
    printf("%10.2f%%  ",(double)(100.0*ivec_ref(counts,i)/(double)ht->num_bins));
    printf("   %10d\n",ivec_ref(counts,i));
  }
  free_ivec(counts);
}

/* Some basic copy and free functions */

int *mk_copy_int(int i)
{
  int *a = AM_MALLOC( int);
  *a = i;
  return a;
}

void *vint_copy(void *data)
{
  int *a = AM_MALLOC( int);
  *a = *((int *)data);
  return (void *)a;
}

void vint_free(void *data)
{
  AM_FREE((int *) data, int);
}

void *vdouble_copy(void *data)
{
  double *d = AM_MALLOC(double);
  *d = *((double *)data);
  return (void *)d;
}

void vdouble_free(void *data)
{
  AM_FREE((double *) data, double);
}

/* String-Hash is a simple set-of-strings in which membership
   test and addition of a string and deletion is a constant-time
   operation. Implemented using the hash structure defined above. */

int int_num_bytes(void *x)
{
  return sizeof(int);
}

int double_num_bytes(void *x)
{
  return sizeof(double);
}

void * vdyv_copy(void * data)
{
  dyv * new = mk_copy_dyv( (dyv *)data );
  return new;
}

void vdyv_free(void * data)
{
  free_dyv( (dyv *)data );
}

int vdyv_num_bytes(void *data)
{
  return dyv_num_bytes((dyv *)data);
}

void * vivec_copy(void * data)
{
  ivec * new = mk_copy_ivec( (ivec *)data );
  return new;
}

void vivec_free(void * data)
{
  free_ivec( (ivec *)data );
}

int vivec_num_bytes(void *data)
{
  return ivec_num_bytes((ivec *)data);
}

void * vstring_copy(void * data)
{
  char * new = mk_copy_string( (char *)data );
  return new;
}

void vstring_free(void * data)
{
  free_string( (char *)data );
}

int vstring_num_bytes(void *data)
{
  return string_num_bytes((char *)data);
}

stringhash *mk_empty_stringhash(void)
{
  stringhash *sh = AM_MALLOC(stringhash);
  sh -> ht = mk_empty_hash_table(vint_copy,vint_free,int_num_bytes);
  return sh;
}

void free_stringhash(stringhash *sh)
{
  free_hash_table(sh->ht);
  AM_FREE(sh,stringhash);
}

stringhash *mk_copy_stringhash(stringhash *old)
{
  stringhash *sh = AM_MALLOC(stringhash);
  sh->ht = mk_copy_hash_table(old->ht);
  return sh;
}

void add_to_stringhash(stringhash *sh,const char *string)
{
  if ( !is_in_stringhash(sh,string) )
  {
    int fake_data[1];
    fake_data[0] = 1;
    add_hash_entry(sh->ht,string,(void *) fake_data);
  }
}

void remove_from_stringhash(stringhash *sh,const char *string)
{
  delete_hash_entry(sh->ht,string);
}

bool is_in_stringhash(stringhash *sh,const char *string)
{
  void *x = lookup_hash_entry(sh->ht,string);
  return x != NULL;
}

string_array *mk_string_array_from_stringhash(stringhash *sh)
{
  string_array *sa = mk_string_array(0);
  int i;
  for ( i = 0 ; i < sh->ht->num_bins ; i++ )
  {
    hlist *hl;
    for ( hl = sh->ht->array[i] ; hl != NULL ; hl = hl -> next )
      add_to_string_array(sa,hl->key);
  }
  return sa;
}

string_array *mk_sorted_string_array_from_stringhash(stringhash *sh)
{
  string_array *sa = mk_string_array_from_stringhash(sh);
  sort_string_array(sa);
  return sa;
}

void hash_table_iterator(const hash_table *ht, hash_table_callback callback, void *extra)
{
  int i;

  for (i=0;i<ht->num_bins;i++)
  {
    const hlist *hl = ht->array[i];
    while (hl) {
      callback(hl->key, hl->data, extra);
      hl=hl->next;
    }
  }
}

string_array *mk_hash_table_keys(hash_table *ht)
{
  string_array *sa = mk_string_array(0);
  int i;

  for (i=0;i<ht->num_bins;i++)
  {
    const hlist *hl = ht->array[i];
    while (hl) {
      add_to_string_array(sa,hl->key);
      hl=hl->next;
    }
  }
  return sa;
}
