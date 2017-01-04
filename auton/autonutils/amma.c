/*
   File:         amma.c
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

/* #define TRACE_MEMORY 0x849ce78 */

#ifdef WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "amma_type.h"
#include "amma_check.h"
#include "amma.h"
#include "am_string.h"
#include "backtrace.h"

#include "internal_state.h"


#ifdef USE_TALLOC
void *am_get_talloc_context(void) {
  am_memory_state_t *state_ptr = am_get_memory_state();
  if (state_ptr->malloc_ctx != NULL) {
    return state_ptr->malloc_ctx;
  } else {
    return state_ptr->malloc_ctx = talloc_init("state_ptr: %p", (void *)state_ptr);
  }
}
#endif

/* Prototypes for private functions */
static free_list *alloc_free_node(void);
static void free_free_node(free_list *fr);
static char *am_basic_malloc(int size);
static bool in_free_list(free_list *fl,char *memory);
static bool free_list_has_duplicates(free_list *fl);
static int free_length(free_list *fr);

#ifndef AMFAST
static void memory_leak_stop_message(void);
#endif


char *am_basic_malloc(int size)
{
  am_memory_state_t *state_ptr = am_get_memory_state();
  char *result;
  if ( size < 0 )
  {
    result = NULL;
    my_errorf("am_basic_malloc(%d) Unable to allocate memory.\n",size);
  }
  else if ( size == 0 )
    result = NULL;
  else
  {
    result = (char *) malloc((unsigned int) size);
    if ( result == NULL )
    {
      basic_am_malloc_report();
      my_printf_stderr(
              "Oh no!!! We're out of memory! In total you mallocked %d "
              "bytes\n",
	      state_ptr->Total_mallocked
	      );
      my_printf_stderr("However, I returned %d bytes via free()\n",
	      state_ptr->Total_freed);
      my_printf_stderr("So we have currently got %d bytes in use.\n",
              state_ptr->Total_mallocked - state_ptr->Total_freed
	      );
      my_printf_stderr(
              "We got back NULL from a call to O/S malloc when you asked\n");
      my_printf_stderr("for something of size %d\n",size);     
      wait_for_key();
      my_error("out of memory");
    }
    state_ptr->Total_mallocked += size;
    state_ptr->Max_mallocked = int_max(state_ptr->Max_mallocked,
				       state_ptr->Total_mallocked - state_ptr->Total_freed);
    state_ptr->Num_mallocs += 1;
  }

  return(result);
}

#ifndef AMFAST
/* Prototypes for non AMFAST private functions */
static amnode *alloc_amnode(void);
static void free_amnode(amnode *amn);
static amnode *add_to_amnode(amnode *am,int count,int size,char *memory);
static amnode *remove_from_amnode(amnode *am,int count,int size,bool *r_found);
static void add_count_to_hash(int count,int size, char *memory);
static bool can_remove_count_from_hash(int count,int size);
static int lowest_count_in_hash(void);
static char *am_extended_malloc(int base_size);
static void am_extended_free(char *memory, int base_size);

amnode *alloc_amnode(void)
{
  am_memory_state_t *state_ptr = am_get_memory_state();
  amnode *am;
  if ( state_ptr->Free_amnodes == NULL )
  {
    am = (amnode *) am_basic_malloc(sizeof(amnode));
    if ( am == NULL ) my_error("out of memory");
    am -> count = -7777;
    am -> size = -7777;
    am -> memory = NULL;
  }
  else
  {
    am = state_ptr->Free_amnodes;
    state_ptr->Free_amnodes = am->next;
  }
  return am;
}

void free_amnode(amnode *amn)
{
  am_memory_state_t *state_ptr = am_get_memory_state();
  amn->next = state_ptr->Free_amnodes;
  amn->count = -777;
  amn->size = -777;
  amn->memory = NULL;
  state_ptr->Free_amnodes = amn;
}

amnode *add_to_amnode(amnode *am,int count,int size,char *memory)
{
  amnode *result = alloc_amnode();
  result -> count = count;
  result -> size = size;
  result -> next = am;
  result -> memory = memory;
  return result;
}

amnode *remove_from_amnode(amnode *am,int count,int size,bool *r_found)
{
  amnode *result = NULL;
  amnode *removed = NULL;
  if ( am == NULL )
    *r_found = FALSE;
  else if ( am->count == count )
  {
    *r_found = TRUE;
    result = am->next;
    removed = am;
  }
  else
  {
    result = am;
    while ( am->next != NULL && am->next->count != count )
      am = am->next;
    if ( am->next == NULL )
    {
      result = NULL;
      *r_found = FALSE;
    }
    else
    {
      amnode *new_next = am->next->next;
      *r_found = TRUE;
      removed = am->next;
      am->next = new_next;
    }
  }

  if ( removed != NULL )
  {
    if ( size != removed->size )
    {
      my_errorf( "You're freeing memory you say has size %d but I say it "
                 "was am_mallocked with size %d", size,removed->size);
    }
    free_amnode(removed);
  }

  return result;
}

void add_count_to_hash(int count,int size,char *memory)
{
  am_memory_state_t *state_ptr = am_get_memory_state();
  int hash = count % NUM_COUNT_BUCKETS;

  if ( !state_ptr->Amnode_buckets_started )
  {
    int i;
    for ( i = 0 ; i < NUM_COUNT_BUCKETS ; i++ )
      state_ptr->Amnode_buckets[i] = NULL;
    state_ptr->Amnode_buckets_started = TRUE;
    state_ptr->Num_items_in_buckets = 0;
  }
  state_ptr->Amnode_buckets[hash] = 
    add_to_amnode(state_ptr->Amnode_buckets[hash],count,size,memory);
  state_ptr->Num_items_in_buckets += 1;
}

bool can_remove_count_from_hash(int count,int size)
{
  am_memory_state_t *state_ptr = am_get_memory_state();
  int hash = count % NUM_COUNT_BUCKETS;
  bool found;
  amnode *newlist;

  if ( !state_ptr->Amnode_buckets_started ) my_error("no way");
  
  if ( state_ptr->Amnode_buckets[hash] == NULL )
    my_error("Serious error freeing something...it looks legal but in "
             "fact nothing of that size is allocated");

  newlist = remove_from_amnode(state_ptr->Amnode_buckets[hash],count,size,&found);
  if ( found )
    state_ptr->Amnode_buckets[hash] = newlist;

  state_ptr->Num_items_in_buckets -= 1;

  return found;
}
    
int lowest_count_in_hash(void)
{
  am_memory_state_t *state_ptr = am_get_memory_state();
  int result = -1;
  int i;
  for ( i = 0 ; i < NUM_COUNT_BUCKETS ; i++ )
  {
    amnode *am;
    for ( am = state_ptr->Amnode_buckets[i] ; am != NULL ; am = am->next )
      if ( result < 0 || result > am->count )
        result = am->count;
  }
  return result;
}
#endif /* AMFAST */


free_list *alloc_free_node(void)
{
  am_memory_state_t *state_ptr = am_get_memory_state();
  free_list *result;
  if ( state_ptr->Free_frees != NULL )
  {
    result = state_ptr->Free_frees;
    state_ptr->Free_frees = state_ptr->Free_frees -> next;
  }
  else
    result = (free_list *) am_basic_malloc(sizeof(free_list));

  state_ptr->Frees_allocated += 1;

  return(result);
}

void free_free_node(free_list *fr)
{
  am_memory_state_t *state_ptr = am_get_memory_state();
  state_ptr->Frees_allocated -= 1;
  fr -> next = state_ptr->Free_frees;
  state_ptr->Free_frees = fr;
}


/* Call this function with am_malloc_number set to N if you are trying
   to debug a memory leak and if am_malloc_report() has warned you that
   on am_malloc call number N there was a leak. */
void memory_leak_stop(int am_malloc_number)
{
  am_memory_state_t *state_ptr = am_get_memory_state();
#ifdef AMFAST
  my_printf(0, "In order to do memory-leak debugging you must have\n");
  my_printf(0, "ALL the sources and auton libraries compiled in non-AMFAST mode.\n");
  my_error("You cannot use memory_leak_stop() if running in AMFAST mode.\n");
#endif
  if ( am_malloc_number < state_ptr->Num_previous_am_mallocs )
  {
    my_printf(0, "It is useless to call memory_leak_stop(%d) at this point\n",
	      am_malloc_number);
    my_printf(0, "in the code. You have already made %d am_malloc calls.\n",
	      state_ptr->Num_previous_am_mallocs);
    my_error("memory_leak_stop()");
  }
  state_ptr->Stop_at_nth_malloc = am_malloc_number;
  state_ptr->Stop_at_defined_by_argc_argv = FALSE;
  my_printf(0, "Will stop the program and let you enter debugger at am_malloc number %d\n",
	    am_malloc_number);
}
/* Call this function at the start of main(int argc,char *argv[]) thus:

     memory_leak_check_args(argc,argv)

   Then, if ever am_malloc_report tells you that you have a memory leak on
   the <N>th call the am_malloc, simply include 
      memleak <N>
   on the command line.
*/
void memory_leak_check_args(int argc,char *argv[])
{
  extern int Ignore_next_n;
  am_memory_state_t *state_ptr = am_get_memory_state();

  Ignore_next_n = int_from_args("num_waitforkey_skips",argc,argv,0);

  if ( index_of_arg("memleak",argc,argv) > 0 )
  {
    int am_malloc_number = int_from_args("memleak",argc,argv,-1);
    if ( am_malloc_number < 0 )
      my_printf(0, "Ignoring memleak %d on command line (because -ve value)\n",
             am_malloc_number);
    else
    {
#ifdef AMFAST
      my_printf(0,"In order to do memory-leak debugging you must have\n");
      my_printf(0,"ALL the sources and auton libraries compiled in non-AMFAST mode.\n");
      my_printf(0,"You cannot have memleak %d on the command line\n"
              "if running in AMFAST mode.\n",am_malloc_number);
      my_error("memory_leak_check_args()");
#endif
      memory_leak_stop(am_malloc_number);
      state_ptr->Stop_at_defined_by_argc_argv = TRUE;
    }
  }
}

#ifndef AMFAST
void memory_leak_stop_message(void)
{
  my_printf(0,"Entered memory_leak_stop_message(). Set a breakpoint for\n");
  my_printf(0,"this function if you want you stop when the memory-leaking\n");
  my_printf(0,"am_malloc() is called.\n");
}
#endif



/* If not AMFAST...

      Returns a block of memory of size "base_size + BYTES_PER_TAG bytes".
      In the first four bytes is a count of the number of
      previous calls to am_extended_malloc that had occured
      before this one. This is used in hunting memory leaks.

      At the moment Bytes_per_tag is eight. What do we do with the remaining
      four bytes? Nothing. They are there to help prevent doubles array
      alignment problems.

   If AMFAST

      Returns a block of memory of size "size".
*/
char *am_extended_malloc(int base_size)
{
  am_memory_state_t *state_ptr = am_get_memory_state();
  int ext_size = compute_extended_size( base_size);
  char *memory;

  int nhp2_size = next_highest_power_of_two(ext_size);
  /* We will actually malloc a block of memory of this size,
     UNLESS nhp2_size > MAX_AM_MALLOC_SIZE, in which case
     we'll only malloc a block of size "size" */

#ifndef AMFAST

  const int max_per_bucket = 100;

  static bool already_complained = FALSE;

  if ( !already_complained &&
       state_ptr->Num_items_in_buckets >= max_per_bucket * NUM_COUNT_BUCKETS )
  {
    my_printf(0,"*** Warning: You have mallocked so much that on average you\n"
	   "*** have more than %d Malloc Node pointers per bucket. You\n"
	   "*** should consider increasing NUM_COUNT_BUCKETS in amma.c\n"
	   "*** from its current value of %d. Note this only affects\n"
	   "** performance of the DEBUG version.\n",max_per_bucket,
	   NUM_COUNT_BUCKETS);
    already_complained = TRUE;
  }

  if ( state_ptr->Num_previous_am_mallocs == state_ptr->Stop_at_nth_malloc )
  {
    int mnum = state_ptr->Num_previous_am_mallocs;

#ifdef BTDEBUG
    {
      btinfo *bt;
      bt = mk_btinfo( 20 /* max num stack frames */);
      fprintf_btinfo( stderr, "\n", bt, "\n");
      free_btinfo( bt);
    }
    my_printf(0, "The call stack above has been generated to help identify ");
    my_printf(0, "the memleak location.\n\n");
#endif

    my_printf(0,"I am stopping the program because I am about to\n");
    my_printf(0,"perform am_malloc number %d, and because you\n",mnum);
    my_printf(0,"had requested me to stop here by including\n\n");
    if ( state_ptr->Stop_at_defined_by_argc_argv )
      my_printf(0,"    memleak %d\n\non the command line.",mnum);
    else
      my_printf(0,"    memory_leak_stop(%d)\n\nin your C-code.\n",mnum);
    my_printf(0,"Assuming you are hunting for memory leaks, you should\n");
    my_printf(0,"now enter the debugger (^C in gdb), (select the BREAK\n");
    my_printf(0,"menu item in Visual C++) and inspect the C-program calling\n");
    my_printf(0,"stack. Your memory-leaking call will be on the stack.\n");
    my_printf(0,"OR set a break point at memory_leak_stop_message() and hit return.\n");
    really_wait_for_key();
    memory_leak_stop_message();
    my_printf(0,"Will continue the program when you next hit return.\n");
    really_wait_for_key();
  }

  if ( state_ptr->Num_previous_am_mallocs > 2000000000 )
  {
    my_errorf("You have done more than two billion am_mallocs. That\n"
	      "is fine in AMFAST mode but in debug mode there is grave\n"
	      "danger because there are some signed ints counting the\n"
	      "number of MALLOCS");
  }

#endif

  if ( !state_ptr->Started )
  {    
    int i;
    for ( i = 0 ; i < MAX_AM_MALLOC_SIZE ; i++ )
    {
      state_ptr->Frees[i] = NULL;
      state_ptr->Num_allocated[i] = 0;
    }
    state_ptr->Started = TRUE;
  }
#ifdef WIN32
  /*EnterCriticalSection(&am_get_internal_state()->win32_am_malloc_mutex);*/
#endif
#ifdef USE_PTHREADS
  pthread_mutex_lock(&am_get_internal_state()->pthread_am_malloc_mutex);
#endif

#ifdef VERY_CAREFUL_CHECKING
  am_check_free_space_sometimes_fatal( state_ptr->Check_free_space_perc, NULL,
                                       "am_extended_malloc");
#endif
# if !defined(AMFAST) && defined(VERY_CAREFUL_CHECKING)
  am_check_heap_sometimes_fatal( state_ptr->Check_heap_perc, NULL, "am_extended_malloc");
#endif

  if ( base_size < 0 )
  {
    my_printf_stderr("am_malloc(%d) illegal\n",base_size);
    memory = NULL;
    my_error("amma.c, am_malloc()");
  }
  else if ( ext_size == 0 )
  {
    memory = &state_ptr->Dummy_memory;
    state_ptr->Num_zeroes_allocated += 1;
  }
  else if ( nhp2_size >= MAX_AM_MALLOC_SIZE )
  {
    /* Note, malloc size is "size" not nhp2_size for these large memory sizes*/
    memory = am_basic_malloc(ext_size);
    state_ptr->Current_large_mallocked += ext_size;
  }
  else if ( state_ptr->Frees[nhp2_size] == NULL )
  {
    state_ptr->Num_allocated[ext_size] += 1;
    memory = am_basic_malloc(nhp2_size);
  }
  else
  {
    free_list *old = state_ptr->Frees[nhp2_size];
#ifndef AMFAST
    am_check_free_list_fatal( old, nhp2_size, "am_extended_malloc");
#endif
    memory = old -> memory;
    state_ptr->Frees[nhp2_size] = old -> next;
    old -> next = NULL;
    free_free_node(old);
    state_ptr->Num_allocated[ext_size] += 1;
  }

  my_assert(memory != NULL);

#ifdef WIN32
  /*LeaveCriticalSection(&am_get_internal_state()->win32_am_malloc_mutex);*/
#endif
#ifdef USE_PTHREADS
  pthread_mutex_unlock(&am_get_internal_state()->pthread_am_malloc_mutex);
#endif

#ifndef AMFAST
  am_set_pre_guard( memory);
  am_set_post_guard( memory, ext_size);
  memory_count_set( state_ptr->Num_previous_am_mallocs, memory);
  add_count_to_hash(state_ptr->Num_previous_am_mallocs,base_size,memory);
#endif
  state_ptr->Num_previous_am_mallocs += 1;

  return(memory);
}

/* Returns a pointer to a block of memory of
   size "size".

   If AMFAST... that's the end of the story (macro in amma.h)

   If not AMFAST, The BYTES_PER_TAG bytes before this block of memory
       are also allocated, a contain a count of the number of counts
       to am_malloc that preceded this one, as well as a guard code.
       BYTES_PER_TAG/2 bytes are allocated after the the allocated
       memory, for a second guard.
*/
#ifndef AMFAST
#if defined CLASSIC_AMMA || defined HALLOC
char *_am_malloc(int size)
{
  char *mem_minus_BYTES_PER_TAG = am_extended_malloc(size);

  /* We only want to add the initial tag area.  The post-memory tag
     is not relevant to the result pointer. */
  char *result = mem_minus_BYTES_PER_TAG + BYTES_PER_TAG;

#ifdef VERY_CAREFUL_CHECKING
  static bool warned = FALSE;
  if ( !warned )
  {
    my_printf(0,"*** WARNING! You have defined the VERY_CAREFUL_CHECKING\n"
	   "*** memory checking flag. That will help detect\n"
	   "*** bugs in your memory management, but it WILL slow things\n"
	   "*** down.\n");
    warned = TRUE;
  }
#endif

#ifdef TRACE_MEMORY  
  if ( TRACE_MEMORY == (long)result )
    my_printf(0, "am_malloc(%d) returns %lX (count = %d)\n",
            size,(long)result,Num_previous_am_mallocs-1);
#endif

  return result;
}
#endif
#endif


bool in_free_list(free_list *fl,char *memory)
{
  bool result = FALSE;
  for ( ; fl != NULL && !result ; fl = fl -> next )
    result = ((long)(fl->memory)) == ((long)memory);
  return(result);
}

bool free_list_has_duplicates(free_list *fl)
{
  bool result = FALSE;
  my_printf_stderr("   [does free list have duplicates?] ......\n");
  for ( ; fl != NULL && !result ; fl = fl -> next )
  {
    result = in_free_list(fl->next,fl->memory);
    if (result) my_printf_stderr("Memory %p is duplicated\n",fl->memory);
  }
  my_printf_stderr("   ......finished\n");
  return(result);
}

/*
   If AMFAST, frees memory block of given size
   If not AMFAST, frees the memory block of size "base_size+BYTES_PER_TAG"
*/
void am_extended_free(char *memory, int base_size)
{
  am_memory_state_t *state_ptr = am_get_memory_state();
  int ext_size = compute_extended_size( base_size);
  int nhp2_size;

#ifdef WIN32
  /*EnterCriticalSection(&am_get_internal_state()->win32_am_malloc_mutex);*/
#endif
#ifdef USE_PTHREADS
  pthread_mutex_lock(&am_get_internal_state()->pthread_am_malloc_mutex);
#endif

#ifdef VERY_CAREFUL_CHECKING
    am_check_free_space_sometimes_fatal( Check_free_space_perc, NULL,
                                         "am_extended_free");
#endif

# if !defined(AMFAST) && defined(VERY_CAREFUL_CHECKING)
    am_check_heap_sometimes_fatal( Check_heap_perc, NULL, "am_extended_free");
# endif


#ifndef AMFAST
  {
    int count;
    bool removed;

    /* Checks for the current piece of memory should be executed every
       time, since am_check_heap_sometimes() might not have run. */
    am_check_pre_guard_fatal( memory, "am_free");
    am_check_post_guard_fatal( memory, ext_size, "am_free");

    count = memory_count_ref( memory);
    removed = can_remove_count_from_hash(count,base_size);
    if ( !removed )
    {
      my_printf(0,"SERIOUS PROBLEM. YOU ARE FREEING SOMETHING THAT YOU "
             "PROBABLY NEVER ALLOCATED (OR MAYBE ALREADY FREED)\n");
      my_printf(0,"(cryptic internal count code is %d)\n",count); 
      my_error("am_free");
    }
  }
#endif

  nhp2_size = next_highest_power_of_two(ext_size);

  if ( memory == NULL )
  {
    my_printf_stderr("am_free(NULL,%d)\n",base_size);
    my_error("You can't free NULL");
  }
  if ( !state_ptr->Started )
    my_error("amma.c, am_free(), Not started");

  if ( base_size < 0 )
  {
    my_printf_stderr("am_free(memory,%d) illegal\n",base_size);
    my_error("amma.c, am_free()");
  }
  else if ( nhp2_size >= MAX_AM_MALLOC_SIZE )
  {
    state_ptr->Total_freed += ext_size;
    state_ptr->Current_large_mallocked -= ext_size;
    free(memory);
  }
  else if ( ext_size == 0 )
  {
    if ( state_ptr->Num_zeroes_allocated <= 0 )
      my_error("free size 0? There's nothing of that size mallocked\n");
    state_ptr->Num_zeroes_allocated -= 1;
  }
  else if ( state_ptr->Num_allocated[ext_size] <= 0 )
  {
    basic_am_malloc_report();
    my_printf_stderr(
            "Free something size %d? Nothing am_malloced of that size\n",
            base_size
	    );
    my_printf_stderr("Memory location of this thing is %8lX\n",(long) memory);

    if ( state_ptr->Frees[nhp2_size] == NULL )
      my_printf_stderr(
              "In fact, I never called am_malloc(%d) during entire program\n",
              base_size
	      );
    else
    {
      my_printf_stderr(
              "(Note: you may have previously mallocked & freed things "
              "that size)\n"
	      );
      if ( in_free_list(state_ptr->Frees[nhp2_size],memory) )
        my_printf_stderr(" ---You've freed that exact piece of memory before\n");
      else
      {
        my_printf_stderr(" ---But you've never freed that piece of memory\n");
        my_printf_stderr("    (calling it size %d while freeing) before\n",
                base_size);
      }

      if ( free_list_has_duplicates(state_ptr->Frees[nhp2_size]) )
      {
        my_printf_stderr("There are duplicate entries in the free list for\n");
        my_printf_stderr("things of that size. You must have earlier on\n");
        my_printf_stderr("freed the same piece of memory twice. That\n");
        my_printf_stderr("confused my counters which is why you're getting\n");
        my_printf_stderr(
                "errors now instead of when you did the duplicate free.\n");
      }
      else
      {
        my_printf_stderr("The free list for things this size has no\n");
        my_printf_stderr("duplicates. That means this very am_free call\n");
        my_printf_stderr("must be freeing a never-allocated thing. Or\n");
        my_printf_stderr("something am_mallocked with a different size.\n");
        my_printf_stderr("Or, earlier, you freed something else, wrongly.\n");
      }
    }
  
    my_error("amma.c, am_free()");
  }
  else
  {
    free_list *newv;

#ifdef VERY_CAREFUL_CHECKING
    /* Check if memory already freed. */
    /* added by awm Sep 1 95 */
    if ( state_ptr->Check_already_freed && 
	 in_free_list(state_ptr->Frees[nhp2_size],memory) )
    {
      my_printf_stderr("am_free(memory=%lx,size=%d)\n",(long)memory,base_size);
      my_error("am_free: freeing something already freed");
    }
#endif

#ifndef AMFAST
    {
      /* Write magic bytes into freed memory. */
      int k;
      for ( k = 0 ; k < nhp2_size ; k++ )
        memory[k] = (char) FREE_MAGIC;
    }
#endif

    newv = alloc_free_node();
    newv -> memory = memory;
    newv -> next = state_ptr->Frees[nhp2_size];
    state_ptr->Frees[nhp2_size] = newv;
    state_ptr->Num_allocated[ext_size] -= 1;
  }

#ifdef WIN32
  /*LeaveCriticalSection(&am_get_internal_state()->win32_am_malloc_mutex);*/
#endif
#ifdef USE_PTHREADS
  pthread_mutex_unlock(&am_get_internal_state()->pthread_am_malloc_mutex);
#endif
}

/* If AMFAST frees the memory (macro in amma.h)
   If not AMFAST, frees the memory, and the BYTES_PER_TAG before and
   BYTES_PER_TAG/2 after.
*/
#ifndef AMFAST
#ifdef CLASSIC_AMMA
void _am_free(char *memory, int size)
{
  char *ptr = NULL;

#ifdef TRACE_MEMORY
  if ( TRACE_MEMORY == (long)memory )
    my_printf(0,"am_free(%lX,%d)\n",(long)memory,size);
#endif

  /* We only want to subtract the initial tag area.  The post-memory
     tag is not relevant to the starting address. */
  ptr = memory - BYTES_PER_TAG;
  am_extended_free(ptr,size);
}
#endif
#endif

int free_length(free_list *fr)
{
  int result = 0;
  for ( ; fr != NULL ; fr = fr -> next )
    result += 1;
  return(result);
}

/* Give as much memory as we can find (hopefully all) 
   back to the operating sys via free().
   When working with TALLOC, it will free EVERYTHING that has
   been AM_MALLOCed, otherwise it won't.

   Do not call this function unless you are going to reset AM_MALLOC!
*/

void am_free_to_os(void)
{
  am_memory_state_t *state_ptr = am_get_memory_state();
#ifdef USE_TALLOC
  if (state_ptr->malloc_ctx) {
    talloc_free(state_ptr->malloc_ctx);  /* free everything */
    state_ptr->malloc_ctx = NULL;
  }
#elif defined CLASSIC_AMMA
  /* need TALLOC, or AMMA+HALLOC for this to work */
  if (state_ptr->Started) {
    my_printf(0,"# Warning, going to leak as much as %d bytes\n", 
	   state_ptr->Total_mallocked);
  }
  am_free_freelist_to_os();         /* free not so much */
#endif


#if defined CLASSIC_AMMA && !defined AMFAST
  state_ptr->Free_amnodes = NULL;
#endif

}

/* Give as much freelist memory as we can find back to the operating sys via free().
 * This function is intended to flush all the freelists, without flushing the
 * dynamically allocated memory that belongs to ivecs, dyvs, etc.
 */
void am_free_freelist_to_os(void)
{
  am_memory_state_t *state_ptr = am_get_memory_state();
#ifdef CLASSIC_AMMA
  int i;
  free_list *cur,*tmp;

  cur = state_ptr->Free_frees;
  while(cur)
  {
    tmp = cur;
    cur = cur->next;
    /* a free_list node in Free_frees has no memory to free:
     * that memory is owned by some ivec, dyv, or whatever.
     * Only the free_list nodes in Frees[i] have memory
    free(tmp->memory);
    */
    free(tmp);
    state_ptr->Total_freed += sizeof(free_list)*2;
  }
  state_ptr->Free_frees = NULL;
  for (i=0;i<MAX_AM_MALLOC_SIZE;i++)
    if (state_ptr->Frees[i])
    {
      cur = state_ptr->Frees[i];
      while (cur)
      {
	tmp = cur;
	cur = cur->next;
	free(tmp->memory);
	free(tmp);
	state_ptr->Total_freed += sizeof(free_list) + i;
      }
      state_ptr->Frees[i] = NULL;
    }
#endif
}


bool am_all_freed( void)
{
  am_memory_state_t *state_ptr = am_get_memory_state();
  bool all_freed;
  int size;

  all_freed = TRUE;

  if ( state_ptr->Current_large_mallocked > 0 ) all_freed = FALSE;

  for ( size = 0 ; all_freed && size < MAX_AM_MALLOC_SIZE ; size++ ) {
    all_freed = (state_ptr->Num_allocated[size] == 0);
  }

  return all_freed;
}

void basic_am_malloc_report(void)
{
  am_memory_state_t *state_ptr = am_get_memory_state();
#ifdef CLASSIC_AMMA

  int size;
  bool boring = TRUE;
  bool free_sizes_exist = FALSE;
  bool all_freed = TRUE;
  int free_bytes = 0;

  for ( size = 0 ; size < MAX_AM_MALLOC_SIZE ; size++ )
    free_bytes += ( sizeof(free_list) + size ) * free_length(state_ptr->Frees[size]);

  free_bytes += 2 * sizeof(free_list) * free_length(state_ptr->Free_frees);

  all_freed = am_all_freed();

  my_printf(0,
          "\n#\n# am_malloc report:                       %s is "
          "am_free()'d\n#\n",
          (all_freed) ? "EVERYTHING" : "NOT everything"
	 );

  my_printf(0,"# Num_previous_am_mallocs = %d\n",state_ptr->Num_previous_am_mallocs);

  if ( !all_freed )
  {
#ifdef AMFAST
    my_printf(0,"#   Run the Debug version (AMFAST undefined during compiling)\n");
    my_printf(0,"#   to find out how to track the leak.\n#\n");
#else
    my_printf(0,"#   On am_malloc call number %d you got a piece of memory\n",
           lowest_count_in_hash());
    my_printf(0,"#   that you never freed. To find this memory leak, EITHER\n");
    my_printf(0,"#   add a call to memory_leak_stop(%d) at the start of your\n",
           lowest_count_in_hash());
    my_printf(0,"#    main() program. OR (for general purpose use) make sure\n");
    my_printf(0,"#    there's a call to memory_leak_check_args(argc,argv) at\n");
    my_printf(0,"#    the start of the main() program, and then include\n");
    my_printf(0,"#         memleak %d\n",lowest_count_in_hash());
    my_printf(0,"#    on the command line when you run the program.\n#\n");
#endif
  }

  my_printf(0, "# Max bytes in use during this program = %d\n",
          state_ptr->Max_mallocked);
  my_printf(0, "# Total bytes currently in use         = %d\n",
          state_ptr->Total_mallocked-state_ptr->Total_freed);
  my_printf(0,"# Bytes currently on DAMUT free lists  = %d\n",free_bytes);
  my_printf(0,"#      Free nodes: %3d allocated; %3d are free themselves\n",
          state_ptr->Frees_allocated,
          free_length(state_ptr->Free_frees)
          );
  my_printf(0,"# Num calls to OS-level malloc         = %d\n",state_ptr->Num_mallocs);

  if ( state_ptr->Num_zeroes_allocated > 0 )
    my_printf_stderr(
            "# You currently have %d zero-length byte%s \"allocated\"\n",
            state_ptr->Num_zeroes_allocated,
            (state_ptr->Num_zeroes_allocated == 1) ? "" : "s"
	    );

  for ( size = 0 ; size < MAX_AM_MALLOC_SIZE ; size++ )
  {
    if ( state_ptr->Started && state_ptr->Num_allocated[size] == 0 && 
	 state_ptr->Frees[size] != 0 )
      free_sizes_exist = TRUE;

    if ( state_ptr->Started && state_ptr->Num_allocated[size] > 0 )
    {
      boring = FALSE;
      my_printf(0,"# Size %4d bytes: %4d allocated; %4d on free list\n",
             size,
             state_ptr->Num_allocated[size],
             free_length(state_ptr->Frees[size])
	     );
    }
  }

  if ( free_sizes_exist )
  {
    int num_shown = 0;
    int n = 9;
    my_printf(0,"#\n# Following sizes have nothing allocated right now,\n");
    my_printf(0,"# but have free lists (length shown in parentheses)\n#\n");

    for ( size = 0 ; size < MAX_AM_MALLOC_SIZE ; size++ )
    {

      if ( state_ptr->Started && state_ptr->Frees[size] != NULL && 
	   state_ptr->Num_allocated[size] == 0 )
      {
        num_shown += 1;
        my_printf(0,"%s%d(%d)%s",
               (num_shown % n == 1) ? "# " : "",
               size,free_length(state_ptr->Frees[size]),
               (num_shown % n == 0) ? "\n" : " "
	       );
      }
    }

    if ( num_shown % n != 0 ) my_printf(0,"\n");
  }
  else if ( boring )
    my_printf(0,"# Nothing allocated, nor on any free list\n");
  
  my_printf(0,"#\n");

#elif defined USE_TALLOC
  talloc_report_full(state_ptr->malloc_ctx, stdout);
#endif
}



/***************************************************************************/
/* MEMORY CHECKING FUNCTIONS -- these need global vars from this file.     */
/* See comments in amma_check.c.                                           */
/***************************************************************************/


/****************************/
/* Configuration accessors. */
/****************************/

/* Frequency of sweeps in VERY_CAREFUL_CHECKING mode. */
void am_check_heap_set_perc( double perc)
{
  am_memory_state_t *state_ptr = am_get_memory_state();
#ifdef AMFAST
  my_error( "am_check_heap_set_perc: "
            "THIS FUNCTION SHOULD NOT BE CALLED WHEN AMFAST IS SET.");
#endif
  state_ptr->Check_heap_perc = perc;
  return;
}

void am_check_free_space_set_perc( double perc)
{
  am_memory_state_t *state_ptr = am_get_memory_state();
#ifdef AMFAST
  my_error( "am_check_free_space_set_perc:\n"
            "Do not call this function when AMFAST is defined.  For\n"
            "instance, try building with t=fastcareful or t=verycareful.\n");
#endif
  state_ptr->Check_free_space_perc = perc;
  return;
}

/* Whether to check if memory is being freed a second time when
   VERY_CAREFUL_CHECKING is enabled. */
void am_check_already_freed_set( bool val)
{
  am_memory_state_t *state_ptr = am_get_memory_state();
#ifdef AMFAST
  my_error( "am_check_free_space_set_perc:\n"
            "Do not call this function when AMFAST is defined.  For\n"
            "instance, try building with t=fastcareful or t=verycareful.\n");
#endif
  state_ptr->Check_already_freed = val;
  return;
}



/***********************/
/* Checking functions. */
/***********************/

int am_check_heap( void)
{
  /* Must make conditional since Amnode_buckets is not always defined. */
#ifdef AMFAST
  my_error( "am_check_heap: "
            "THIS FUNCTION SHOULD NOT BE CALLED WHEN AMFAST IS SET.");
  return 1;
#else
  am_memory_state_t *state_ptr = am_get_memory_state();
  return am_check_heap_nonglobal( state_ptr->Amnode_buckets);
#endif
}

int am_check_heap_sometimes( double perc, int *r_ran)
{
  /* Must make conditional since Amnode_buckets is not always defined. */
#ifdef AMFAST
  my_error( "am_check_heap_sometimes:\n"
            "Do not call this function while the AMFAST macro is defined.\n"
            "For instance, try building with t=debug or t=verycareful.\n");
  return 1;
#else
  am_memory_state_t *state_ptr = am_get_memory_state();
  return am_check_heap_sometimes_nonglobal( state_ptr->Amnode_buckets, perc, r_ran);
#endif
}

void am_check_heap_sometimes_fatal( double perc, int *r_ran, const char *msg)
{
  /* Must make conditional since Amnode_buckets is not always defined. */
#ifdef AMFAST
  my_error( "am_check_heap_sometimes:\n"
            "Do not call this function while the AMFAST macro is defined.\n"
            "For instance, try building with t=debug or t=verycareful.\n");
#else
  am_memory_state_t *state_ptr = am_get_memory_state();
  am_check_heap_sometimes_nonglobal_fatal( state_ptr->Amnode_buckets, perc, r_ran, msg);
#endif
}

int am_check_free_space( void)
{
  am_memory_state_t *state_ptr = am_get_memory_state();
  return am_check_free_space_nonglobal( state_ptr->Frees);
}

int am_check_free_space_sometimes( double perc, int *r_ran)
{
  am_memory_state_t *state_ptr = am_get_memory_state();
  return am_check_free_space_sometimes_nonglobal( state_ptr->Frees, perc, r_ran);
}

void am_check_free_space_sometimes_fatal( double perc, int *r_ran,
                                          const char *msg)
{
  am_memory_state_t *state_ptr = am_get_memory_state();
  am_check_free_space_sometimes_nonglobal_fatal( state_ptr->Frees, perc, r_ran, msg);
  return;
}

/* A simplified memory report for the PC.  

   This has been moved back and forth between amma.c and stats.c, supposedly
   to eliminate a dependency on statistics
*/

char *pc_memory_report(void)
{
  int size;
  bool all_freed = TRUE;
  int free_bytes = 0;
  char s[1024];
  char *report;
  am_memory_state_t *state_ptr = am_get_memory_state();

  for ( size = 0 ; size < MAX_AM_MALLOC_SIZE ; size++ )
    free_bytes += ( sizeof(free_list) + size ) * free_length(state_ptr->Frees[size]);

  free_bytes += 2 * sizeof(free_list) * free_length(state_ptr->Free_frees);

  all_freed = am_all_freed();

  snprintf(s, 1024, "All Freed = %s, Total_mallocked (%d) - free_bytes (%d) - Total_freed (%d) = %d", 
	  (all_freed == TRUE) ? "True" : "False", state_ptr->Total_mallocked,
	   free_bytes, state_ptr->Total_freed, 
	   state_ptr->Total_mallocked-(free_bytes + state_ptr->Total_freed));
  report = mk_copy_string(s);
  return(report);
}
