/* *
   File:         amma.h
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

#ifndef AMMA_H

#define AMMA_H

#ifdef USE_BOEHM_GC
#ifndef AMFAST
#define GC_DEBUG
#endif
#include "gc.h"
#endif

#include "standard.h"
#include "ambs.h"
#include "amma_type.h"
#include "auton_sf.h"

#ifdef USE_TALLOC
#ifndef off_t
#define off_t long
#endif
#include "talloc.h"
#endif



/* Choose your memory manager! */
#ifdef USE_OS_MEMORY_MANAGEMENT      /* system malloc/free */

#define _AM_MALLOC(t) ((t *) malloc(sizeof(t)))
#define _AM_MALLOC_ARRAY(t,len) ((t *) malloc((len) * sizeof(t)))

#define _AM_FREE(thing,type) free(thing)
#define _AM_FREE_ARRAY(thing,type,len) free(thing)


#elif defined USE_BOEHM_GC                  /* garbage collection */

#define _AM_MALLOC(t) ((t *) GC_MALLOC(sizeof(t)))
#define _AM_MALLOC_ARRAY(t,len) ((t *) GC_MALLOC((len) * sizeof(t)))

#define _AM_FREE(thing,type) /* GC_FREE(thing) */
#define _AM_FREE_ARRAY(thing,type,len) /* GC_FREE(thing) */


#elif defined USE_TALLOC                    /* from Samba 4 */

void *am_get_talloc_context(void);

#define _AM_MALLOC(t) talloc(am_get_talloc_context(), t)
#define _AM_MALLOC_ARRAY(t,len) talloc_array(am_get_talloc_context(), t, len)

#define _AM_FREE(thing, type) talloc_free(thing)
#define _AM_FREE_ARRAY(thing,type,len) talloc_free(thing)


#else                               /* Classic code from 1992 */

#define CLASSIC_AMMA

#define _AM_MALLOC(t) ((t *) _am_malloc(sizeof(t)))
#define _AM_MALLOC_ARRAY(t,len) ((t *) _am_malloc( (int)((len) * sizeof(t))))

#define _AM_FREE(thing,type) (_am_free((char *)(thing),sizeof(type)))
#define _AM_FREE_ARRAY(thing,type,len) \
  (_am_free((char *)(thing), (int)(sizeof(type) * (len))))

#ifdef AMFAST
char *am_extended_malloc(int size);
void am_extended_free(char *memory, int size);
#define _am_free(memory,size) am_extended_free(memory,size) 
#define _am_malloc(size) am_extended_malloc(size)
#else
char *_am_malloc(int size);
void _am_free(char *memory, int size);
#endif

#endif /* memory schemes */

#define AM_MALLOC _AM_MALLOC
#define AM_MALLOC_ARRAY _AM_MALLOC_ARRAY

#define AM_FREE _AM_FREE
#define AM_FREE_ARRAY _AM_FREE_ARRAY


/* Call this function with am_malloc_number set to N if you are trying
   to debug a memory leak and if am_malloc_report() has warned you that
   on am_malloc call number N there was a leak. */
void memory_leak_stop(int am_malloc_number);

/* Call this function at the start of main(int argc,char *argv[]) thus:

     memory_leak_check_args(argc,argv)

   Then, if ever am_malloc_report tells you that you have a memory leak on
   the <N>th call the am_malloc, simply include 
      memleak <N>
   on the command line.
*/
void memory_leak_check_args(int argc,char *argv[]);

/* Safely back some memory, if we can, without leaking */
void am_free_freelist_to_os(void);

/* Give as much memory as we can find (hopefully all)  
   back to the operating sys via free().
   When working with HALLOC or TALLOC, it will free EVERYTHING that has
   been AM_MALLOCed, otherwise it won't.

   Do not call this function unless you are going to reset AM_MALLOC!
*/
void am_free_to_os(void);

bool am_all_freed(void);
void basic_am_malloc_report(void);
   /* If possible don't use this. Use am_malloc_report() from ammarep.c
      instead
   */


char *pc_memory_report(void);

/***************************************************************************/
/* MEMORY CHECKING FUNCTIONS -- these need global vars from this file.     */
/* See comments in amma_check.h.                                           */
/***************************************************************************/

/****************************/
/* Configuration accessors. */
/****************************/

void am_check_heap_set_perc( double perc);
void am_check_free_space_set_perc( double perc);
void am_check_already_freed_set( bool val);

/***********************/
/* Checking functions. */
/***********************/

int am_check_heap( void);
int am_check_heap_sometimes( double perc, int *r_ran);
void am_check_heap_sometimes_fatal( double perc, int *r_ran, 
				    const char *msg);

int am_check_free_space( void);
int am_check_free_space_sometimes( double perc, int *r_ran);
void am_check_free_space_sometimes_fatal( double perc, int *r_ran,
					  const char *msg);

/* Declared but not defined - sir 8/6/2000
extern void am_malloc_test(void); */

/* #ifdef PC_MVIS_PLATFORM */
/* #include <stdarg.h> */
/* void eprintf ( FILE * file , char * lpszFormat, ...); */
/* #define fprintf eprintf */
/* #endif */

#endif /* #ifndef AMMA_H */
