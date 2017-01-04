/* *
   File:         amma_type.h
   Author:       Andrew W. Moore
   Created:      4th September 1992
   Description:  My own memory allocation and deallocation.

   Copyright (C) Andrew W. Moore, 1992

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

#ifndef AMMA_TYPE_H
#define AMMA_TYPE_H



#include "standard.h"
#include "ambs.h"

typedef struct amnode
{
  int count;
  int size;
  struct amnode *next;
  char *memory;
} amnode;

typedef struct free_list_struct
{
  char *memory;
  struct free_list_struct *next;
} free_list;



/* Maximum number of bytes per alloc. */
#define MAX_AM_MALLOC_SIZE (1 << 12)

/* Number of buckets in amnode hash. */
#define NUM_COUNT_BUCKETS 1000001

/* Post-memory guard area is half this. */
#define BYTES_PER_TAG 8

/* Magic number written (as a char) to free lists when using
   VERY_CAREFUL_CHECKING. */
#define FREE_MAGIC 77



/* Size of memory allocated for a given request. */
#ifdef AMFAST
#  define compute_extended_size(basesize) (basesize)
#  define compute_userptr(mem) (mem)
#else
#  define compute_extended_size(basesize) \
  (basesize + BYTES_PER_TAG + BYTES_PER_TAG/2)
#  define compute_userptr(mem) \
  (mem + BYTES_PER_TAG)
#endif



/* Set and retrieve the allocation number from a block of memory */
#ifndef AMFAST
#define memory_count_ref(memory) ((memory[4] & 0xFF) << 24 |  \
                                  (memory[5] & 0xFF) << 16 | \
                                  (memory[6] & 0xFF) <<  8 | \
                                  (memory[7] & 0xFF))

#define memory_count_set( previous_am_mallocs, memory) { \
  memory[4] = ((previous_am_mallocs & 0xFF000000) >> 24); \
  memory[5] = ((previous_am_mallocs & 0x00FF0000) >> 16); \
  memory[6] = ((previous_am_mallocs & 0x0000FF00) >> 8); \
  memory[7] = ((previous_am_mallocs & 0x000000FF)); \
}
#endif



#endif /* #ifndef AMMA_TYPE_H */
