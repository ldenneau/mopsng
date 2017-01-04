#ifndef INTERNAL_STATE_H_DEFINED
#define INTERNAL_STATE_H_DEFINED

/* 
 * File: internal_state.h
 * Autors: Adam Goode, Jeanie Komarek
 * Date: Feb. 7, 2006
 * Description:
 * internal state structure.  Use instead of creating individual global
 * variables in utils.  Values initialized in internal_state.c.  
 *
 * Global variables in utils cause problems when utils is linked into
 * an application (ex. ASL) both statically and dynamically.  On
 * windows, each individual dll and executable that links with utils
 * statically will have its own copy of the global variables.  To make
 * this easier to maintain, internal_state contains all the global
 * vars (most of which are related to memory allocation) that would
 * need to be consistant across multiple liraries linked together into
 * the same program.  am_set_internal_state can be used to make sure
 * the main program and the libraries it links with are all using the
 * same internal_state pointer.
 * 
 * It is also possible to explicitely create a separate copy of some
 * variables (such as memory variables) for a child thread - allowing
 * for easy memory cleanup if an error ocurrs in the tread.  See
 * am_init_thread_specific_data() below.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifdef USE_PTHREADS
  #include <pthread.h> /* MUST be the first include in this file */
#endif

#include <setjmp.h>

#include "command.h"
#include "ambs.h"

/*--------------------------------------------------------------*/
typedef void(*my_error_ptr_t)(const char *);
typedef void(*my_print_ptr_t)(int level, const char *);
/*--------------------------------------------------------------*/

/* --------------- Memory allocation variables ------------------ */
/* Always access this data by calling am_get_memory_state() */
typedef struct _am_memory_state {
  /* used by amma.c */
  double Check_heap_perc;
  double Check_free_space_perc;

#ifndef AMFAST
  amnode *Free_amnodes;
  amnode *Amnode_buckets[NUM_COUNT_BUCKETS];
  int Num_items_in_buckets;
  bool Amnode_buckets_started;
#endif

  int Total_mallocked;
  int Total_freed;
  int Max_mallocked;
  int Current_large_mallocked;
  int Num_zeroes_allocated;
  int Num_mallocs;
  char Dummy_memory;

  free_list *Frees[MAX_AM_MALLOC_SIZE];
  int Num_allocated[MAX_AM_MALLOC_SIZE];

  bool Started;

  /* A serial number of sorts, for use with objects. This number will change
   * if reset_malloc_state is called.
   *
   * Recommended usage:
   *  - When creating an object, copy this value into a field of
   *    the object
   *  - Upon each operation on that object, compare the saved value
   *    with this value
   *  - If there is a mismatch, refuse to perform any operation except
   *    destruction of the object (since all memory allocated previously
   *    is now invalid)
  */
  unsigned int generation_number;

  free_list *Free_frees;
  int Frees_allocated;

  bool Check_already_freed;

  int Num_previous_am_mallocs;
  int Stop_at_nth_malloc;
  int Stop_at_defined_by_argc_argv;
#ifdef AMFAST
  bool More_than_two_billion_mallocs;
#endif

#if defined USE_TALLOC
  void *malloc_ctx;
#endif

  /* used by am_string_array.c */
  int String_Arrays_mallocked;
  int String_Arrays_freed;

  /* used by amdyv.c */
  int Dyvs_mallocked;
  int Dyvs_freed;

} am_memory_state_t;


/* ------ All global data ------------------ */
typedef struct _am_internal_state {
  am_memory_state_t *memory_state; /*always access with am_get_memory_state()*/

  /* used by ambs.c */
  ui_mode user_interaction_mode;
  int Verbosity; /* hope to replace global Verbosity var with this some day, 
		    but still don't want people to use it directly.  Instead
		    use Global_set_verbosity() and my_printf.*/

  /* for my_error longjmp */
  jmp_buf error_return_target;
  my_error_ptr_t error_ptr;
  const char *last_error;

  /* override my_printf behavior */
  my_print_ptr_t print_ptr;

  bool graphics_initialized;

#ifdef USE_PTHREADS
  pthread_key_t thread_specific_state_pthread_key;
  pthread_mutex_t pthread_am_malloc_mutex;  
#endif
#ifdef WIN32
  CRITICAL_SECTION win32_am_malloc_mutex;
#endif

} am_internal_state_t;

/* --------- thread specific vars --------------------- */
/* If a new variable needs to be made thread specific, you must
   provide get/set accessor functions for that variable and make sure no
   code is accessing the variable in the internal_state structure
   directly.
*/
typedef struct _am_thread_specific_state {
  am_memory_state_t *memory_state; 
  my_error_ptr_t error_ptr;  /* different handling of my_error for runing alg*/

  bool in_aggressive_abort;
  bool graphics_initialized; /* temporary, needed for ASL until switch to 
                                using new graphics*/
} am_thread_specific_state_t;

/*--------------------------------------------------------------*/
/*--------------------------------------------------------------*/
/*--------------------------------------------------------------*/

/* Access to global state data.  If there is an access function
   specific to the data you need to access, you must use it instead of
   accessing the am_internal_state_t struct members directly.*/
am_internal_state_t *am_get_internal_state(void);
am_memory_state_t *am_get_memory_state(void);
my_error_ptr_t am_get_my_error_ptr(void);
bool am_get_graphics_initialized(void);
void am_set_graphics_initialized(bool is_initialized);

/*--------------------------------------------------------------*/
/* Purpose:
   synchronize internal state between multiple libraries where
   utils is both dynamically and statically linked in (matters
   on windows platform) 
*/
void am_set_internal_state(am_internal_state_t *);
AMEXPORT void am_set_internal_state_exported(am_internal_state_t *);

/* Purpose: 
   Call from within a child thread to provide a different
   copy of some state variables (esp. memory) for each thread in a
   multi-threaded process.  To use this, you must build with TALLOC,
   or BOEHM_GC memory management.  If the thread my_errors or
   needs to be aborted, the thread can call
   destroy_thread_specific_data() to clean up all of the thread
   specific memory.
*/
void am_init_thread_specific_state();
void am_destroy_thread_specific_state();
am_thread_specific_state_t *am_get_thread_specific_state();

/* Purpose: 
   Clean up all global state data and free all memory allocated.  Any
   subsequent call requiring the use of the internal_state structure
   will cause a new structure to be created automatically.
   Note that this will leak tons of memory if not using TALLOC, 
   or BOEHM_GC
*/
void am_destroy_internal_state();

/* Temporarily switch to using the main thread memory. Returns a pointer
   to the thread specific memory that should be restored.*/
void *am_switch_to_main_thread_memory();
/* Restore the thread specific memory using the pointer returned by
   am_switch_to_main_thread_memory()
*/
void am_switch_to_thread_specific_memory(void *thread_mem);

/*--------------------------------------------------------------*/
/* Functions to allow the my_error, my_warning, etc. behavior to be
   overridden.
*/
AMEXPORT void set_my_error_ptr(my_error_ptr_t p);
AMEXPORT void set_my_print_ptr(my_print_ptr_t p);

/*--------------------------------------------------------------*/

#endif
