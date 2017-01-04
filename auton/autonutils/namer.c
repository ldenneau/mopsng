/*
   File:        namer.c
   Author:      Andrew W. Moore
   Created:     Sat Mar 30 09:18:48 EST 2002
   Description: Maps between indexes and names

   Copyright 2002, Andrew W. Moore

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

#include "namer.h"
#include "hash.h"

namer *mk_copy_namer(namer *old)
{
  namer *nu = AM_MALLOC(namer);

  nu -> index_to_name = mk_copy_string_array(old->index_to_name);
  nu -> ht = mk_copy_hash_table(old->ht);
  nu -> case_sense = old -> case_sense;

  return nu;
}

namer *mk_empty_namer(bool case_sensitive)
{
  namer *nr = AM_MALLOC(namer);
  nr -> case_sense = case_sensitive;
  nr -> index_to_name = mk_string_array(0);
  nr -> ht = mk_empty_hash_table(vint_copy,vint_free,int_num_bytes);

  return nr;
}

void free_namer(namer *nr)
{
  free_string_array(nr->index_to_name);
  free_hash_table(nr->ht);
  AM_FREE(nr,namer);
}

int namer_num_indexes(namer *nr)
{
  return string_array_size(nr->index_to_name);
}

bool namer_case_sensitive(namer *nr) {
  return nr->case_sense;
}

char *namer_index_to_name(namer *nr,int index)
{
  return string_array_ref(nr->index_to_name,index);
}

int namer_name_to_index(namer *nr,char *sname)
{
  char *dc_name;
  int *ip;
  int result;

  /*CHANGE*/
  if(nr->case_sense) {
    ip = (int *) lookup_hash_entry(nr->ht,sname);
    result = (ip == NULL) ? -1 : *ip;

    my_assert(result < 0 || eq_string(sname,namer_index_to_name(nr,result)));
  } else {
    dc_name = mk_downcase_string(sname); 
    ip = (int *) lookup_hash_entry(nr->ht,dc_name);
    result = (ip == NULL) ? -1 : *ip;

    my_assert(result < 0 || eq_string(dc_name,namer_index_to_name(nr,result)));
    free_string(dc_name);
  }

  return result;
}


/* Does nothing if name already there */
void add_to_namer(namer *nr,char *sname)
{
  char *dc_name;

  if(nr->case_sense) {
   if ( namer_name_to_index(nr,sname) < 0 ) {
      int data[1];
      data[0] = namer_num_indexes(nr);
      add_hash_entry(nr->ht,sname,(void *) data);
      add_to_string_array(nr->index_to_name,sname);
   }
  } else {
    dc_name = mk_downcase_string(sname);
    if ( namer_name_to_index(nr,dc_name) < 0 ) {
      int data[1];
      data[0] = namer_num_indexes(nr);
      add_hash_entry(nr->ht,dc_name,(void *) data);
      add_to_string_array(nr->index_to_name,dc_name);
    }
    free_string(dc_name);
  }
}

void remove_from_namer(namer *nr, char *sname)
{
    char *dc_name;
    int index;

    /*find the entry*/
    if (nr->case_sense){
	index = namer_name_to_index(nr, sname);
    }
    else{
	dc_name = mk_downcase_string(sname);
	index = namer_name_to_index(nr, dc_name);
	free_string(dc_name);
    }
	
    if (index >= 0)
    {
	/*remove from string_array*/
	string_array_remove(nr->index_to_name, index);

	/*recreate the hash table now that array indexing has changed*/
	rebuild_namer_hash_table(nr);
    } 
}

/*reconstruct the hash table using the current state of the 
  'index_to_name' array.  Need to do this after removing a 
  namer entry*/
void rebuild_namer_hash_table(namer *nr)
{
    int i;
    free_hash_table(nr->ht);    
    nr->ht = mk_empty_hash_table(vint_copy,vint_free,int_num_bytes);
    for (i = 0; i < string_array_size(nr->index_to_name); i++)
    {
	int data[1];
	char *str = string_array_ref(nr->index_to_name, i);

	data[0] = i;
	add_hash_entry(nr->ht, str, (void *)data);
    }
}

namer *mk_namer_from_string_array(string_array *sa, bool case_sensitive)
{
  int i;
  namer *nm = mk_empty_namer(case_sensitive);
  for ( i = 0 ; i < string_array_size(sa) ; i++ )
    add_to_namer(nm,string_array_ref(sa,i));
  return nm;
}

void fprintf_namer(FILE *s,char *m1,namer *x,char *m2)
{
  char *buff;

  buff = mk_printf("%s -> index_to_name",m1);
  fprintf_string_array(s,buff,x->index_to_name,m2);
  free_string(buff);
}

void pnamer(namer *x)
{
  fprintf_namer(stdout,"namer",x,"\n");
}

/* Allows you to access the string_array mapping index to name in the
   string array, but you are NOT ALLOWED to update the string_array */
string_array *namer_index_to_name_array(namer *nm)
{
  return nm->index_to_name;
}

/* indexes may be NULL meaning "use all indexes" */
string_array *mk_string_array_from_namer(namer *nm,ivec *indexes)
{
  int size = (indexes==NULL) ? namer_num_indexes(nm) : ivec_size(indexes);
  string_array *names = mk_string_array(size);
  int i;
  for ( i = 0 ; i < size ; i++ )
  {
    int index = (indexes==NULL) ? i : ivec_ref(indexes,i);
    string_array_set(names,i,namer_index_to_name(nm,index));
  }
  return names;
}

/* indexes may be NULL meaning "use all indexes" */
string_array *mk_sorted_string_array_from_namer(namer *nm,ivec *indexes)
{
  string_array *unsorted = mk_string_array_from_namer(nm,indexes);
  string_array *sorted = mk_sort_string_array(unsorted);
  my_assert(string_array_size(sorted) == string_array_size(unsorted));
  free_string_array(unsorted);
  return sorted;
}

/* indexes may be NULL meaning "use all indexes" */
void namer_display_names(namer *nm,ivec *indexes)
{
  string_array *names = mk_string_array_from_namer(nm,indexes);
  display_names(stdout,names);
  free_string_array(names);
}

void summarize_namer(namer *nm,char *itemname)
{
  printf("\n%ss:\n",itemname);
  namer_display_names(nm,NULL);
}

/* result[i] = namer_name_to_index(nm,names[i]).
   Note result[i] = -1 if names[i] is not in the namer */
ivec *mk_indexes_from_namer(namer *nm,string_array *names)
{
  int size = string_array_size(names);
  int i;
  ivec *indexes = mk_ivec(size);
  for ( i = 0 ; i < size ; i++ )
  {
    char *name = string_array_ref(names,i);
    int index = namer_name_to_index(nm,name);
    ivec_set(indexes,i,index);
  }
  return indexes;
}

bool namer_equal(namer *a,namer *b)
{
  bool equal = TRUE;

  equal = equal && string_array_equal(a->index_to_name,b->index_to_name);
  equal = equal && (a->case_sense == b->case_sense);
  /* Don't need to do  hash_table *ht; */

  return equal;
}










