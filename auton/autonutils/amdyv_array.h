/* 
   File:        amdyv_array.h
   Author:      Andrew W. Moore
   Created:     June 15, 2004
   Description: Extensions and advanced amdm stuff

   Copyright 1996, Schenley Park Research

   This file contains utility functions involving dyv_arrays, dyvs and ivecs.

   The prototypes of these functions used to be declared in amdmex.h.
   Now it's amdyv_array.h

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

#ifndef AMDYV_ARRAY_H
#define AMDYV_ARRAY_H

#include "standard.h"
#include "ambs.h"
#include "amiv.h"
#include "amdyv.h"
#include "am_string_array.h"

typedef struct dyv_array
{
  int size;
  int array_size;
  dyv **array;
} dyv_array;



dyv_array *mk_empty_dyv_array(void);
void add_to_dyv_array(dyv_array *da, const dyv *dv);
void add_to_dyv_array_no_copy(dyv_array *da,dyv *dv);

dyv_array *mk_const_dyv_array(dyv *base_vec, int size);

dyv_array *mk_rectangular_dyv_array( int numdyvs, int dyvlen);

dyv_array *mk_constant_rectangular_dyv_array( int numdyvs, int dyvlen, double val);

/* Creates an dyv array with size entries each composed of an dyv of size 0  [e.g. mk_dyv(0)] */
dyv_array *mk_zero_dyv_array(int size);

int dyv_array_size(const dyv_array *da);
dyv *safe_dyv_array_ref(const dyv_array *da,int idx);

dyv_array *mk_array_of_zero_length_dyvs(int size);
dyv_array *mk_array_of_null_dyvs(int size);

dyv_array *mk_dyv_array( int size);
dyv_array *mk_dyv_array_subset( dyv_array *da, ivec *indices);

#ifdef AMFAST
#define dyv_array_ref(dva,i) ((dva)->array[i])
#else
#define dyv_array_ref(dva,i) safe_dyv_array_ref(dva,i)
#endif

void dyv_array_set(dyv_array *dva, int idx, const dyv *dv);
void dyv_array_set_no_copy( dyv_array *dva, int idx, dyv *dv);
void fprintf_dyv_array(FILE *s, const char *m1, const dyv_array *da, const char *m2);
void fprintf_oneline_dyv_array( FILE *f, const char *pre, const dyv_array *da,
				const char *post);
void free_dyv_array(dyv_array *da);
void free_dyv_array_container(dyv_array *da);

dyv_array *mk_copy_dyv_array(const dyv_array *da);
void dyv_array_remove(dyv_array *dva,int idx);
void dyv_array_remove_no_free(dyv_array *dva,int idx);

/* rows may be NULL denoting explain all */
void explain_dyv_array(dyv_array *da,ivec *rows);

void pdyv_array(dyv_array *da);

bool dyv_array_equal(dyv_array *da1,dyv_array *da2);

dyv *mk_dyv_array_sum(dyv_array *sps,ivec *rows);

ivec *mk_dyv_lengths(dyv_array *da);

int dyv_array_num_dims(dyv_array *da);

dyv *mk_sum_of_dyv_array(const dyv_array *da);

/* PRE: All the vectors mentioned in "rows" must have length
   equal to dyv_length.

   If "rows" is empty, returns a vector of length "dyv_length" 
   containing all zeroes. */
/* "rows" may be NULL, meaning "use all rows" */
dyv *mk_sum_of_dyv_array_rows(dyv_array *da,ivec *rows,
			      int dyv_length);


/* Returns a dyv_array of size "num_dyvs" in which the i'th
   element (forall 0 <= i < num_dyvs) is a vector of
   size "dyv_size" in which each element is 0.0 */
dyv_array *mk_dyv_array_of_zeroed_dyvs(int num_dyvs,int dyv_size);
  
void fprint_dyv_array_csv(FILE *s,dyv_array *x);


char *mk_string_from_dyv(dyv *d);
dyv *mk_dyv_from_string_array_with_error_message(const string_array *sa,
                                                 const char *format,
                                                 char **err_mess);
dyv *mk_dyv_from_string_array(const string_array *sa, const char *format );

string_array *mk_string_array_from_dyv(const dyv *d);

dyv *mk_dyv_from_string(const char *string, const char *format);
dyv *mk_dyv_from_string_with_error_message(const char *string,
                                           const char *format,
                                           char **err_mess);

/* Prints the contents of string_array cleverly much in the same way
   that "short ls" in unix displays filenames. It's cleverly designed so that when
   printed it uses less than "max_chars" characters per line. (it auto-chooses
   and sizes the columns). Implemented using dyvs, so here instead
   of in am_string_array.h */
void display_names(FILE *s,string_array *names);

#endif
