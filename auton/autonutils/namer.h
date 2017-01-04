/*
   File:        namer.h
   Author:      Andrew W. Moore
   Created:     Sat Mar 30 09:18:48 EST 2002
   Description: Maps between indexes and names

   Copyright 2002, Auton Lab

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


#ifndef NAMER_H
#define NAMER_H

#include "amut.h"
#include "hash.h"

/*

A namer is a useful constant-time-access-and-update 1-to-1 mapping
between strings and integers. It has the same kind of functionality
as string_arrays except that you can also go from 
string to index in constant time.

You can test whether a string is in the namer with

  bool is_in = namer_name_to_index(nm,string) >= 0

So a namer is simply a set of (integer,string) pairs in which
there exists exactly one pair for each integer i such
that 0 <= i < namer_num_indexes(nm)

Added by Jeremy (9/19/2003): Namers can be case sensitive or
downcase everything.
*/

typedef struct namer
{
  bool case_sense;
  string_array *index_to_name;
  hash_table *ht;
} namer;

namer *mk_copy_namer(namer *old);

/* Makes a namer containing zero strings */
namer *mk_empty_namer(bool case_sensitive);

void free_namer(namer *nr);

/* Tells you how many items in the namer */
int namer_num_indexes(namer *nr);

bool namer_case_sensitive(namer *nr);

/* Constant time operation */
char *namer_index_to_name(namer *nr,int idx);

/* Constant time operation */
int namer_name_to_index(namer *nr,char *name);

/* If name is already in the namer, this has no effect.

   If name is not in the namer, adds it and gives it the next
   available index. i.e.  namer_num_indexes(nm). */
/* Constant time operation */
void add_to_namer(namer *nr,char *name);
void remove_from_namer(namer *nr, char *name);
void rebuild_namer_hash_table(namer *nr);

void fprintf_namer(FILE *s,char *m1,namer *x,char *m2);

/* Prints out the namer. Useful in gdb if you want to type
   call pnamer(x) at the gdb prompt */
void pnamer(namer *x);

/* indexes may be NULL meaning "use all indexes" */
/* result[i] = namer_index_to_name(nm,indexes[i]) */
string_array *mk_string_array_from_namer(namer *nm,ivec *indexes);

/* indexes may be NULL meaning "use all indexes" */
string_array *mk_sorted_string_array_from_namer(namer *nm,ivec *indexes);

/* Does a neat 'ls style' print to stdout of all the names
   in indexes 
     indexes may be NULL meaning "use all indexes" */
void namer_display_names(namer *nm,ivec *indexes);

/* namer_index_to_name(result,i) = sa[i] */
namer *mk_namer_from_string_array(string_array *sa, bool case_sensitive);

/* result[i] = namer_name_to_index(nm,names[i]).
   Note result[i] = -1 if names[i] is not in the namer */
ivec *mk_indexes_from_namer(namer *nm,string_array *names);

void summarize_namer(namer *nm,char *itemname);

bool namer_equal(namer *a,namer *b);

/* Allows you to access the string_array mapping index to name in the
   string array, but you are NOT ALLOWED to update the string_array */
string_array *namer_index_to_name_array(namer *nm);

#endif /* #ifndef NAMER_H */
