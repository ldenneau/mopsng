/* 
 * File: internal_state.c
 * Autors: Adam Goode, Jeanie Komarek
 * Date: Feb. 7, 2006
 * Description: Internal state structure.  Use instead of creating individual
 * global variables in utils.
 *
 * Copyright Auton Lab, CMU
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
#include "internal_state.h"
#include "ambs.h"

/* --------------------------------------------------------- */

/* 
   g_state_ptr keeps the internal_state for the program.  Some (duplicate)
   state may also be stored as thread specific data if we are using
   threads.
*/
static am_internal_state_t *g_state_ptr = NULL;

/* ------------------------------------------------------------------ */
static am_internal_state_t *_am_mk_internal_state(void);
static am_memory_state_t *_am_mk_memory_state(void);
static void _am_destroy_memory_state();
static void _check_aggressive_abort();
/* ------------------------------------------------------------------ */

/****** CREATE internal_state ***************************************/
am_memory_state_t *_am_mk_memory_state(void)
{
  int i;
  /* Do not use AM_MALLOC here. */
  am_memory_state_t *ret = malloc(sizeof(am_memory_state_t));

  ret->Check_heap_perc = 0.01;
  ret->Check_free_space_perc = 0.01;

#ifndef AMFAST
  ret->Free_amnodes = NULL;
  for (i = 0; i<NUM_COUNT_BUCKETS; i++){
    ret->Amnode_buckets[i]=NULL;
  }
  ret->Num_items_in_buckets = 0;
  ret->Amnode_buckets_started = FALSE;
#endif

  ret->Total_mallocked = 0;
  ret->Total_freed = 0;
  ret->Max_mallocked = 0;
  ret->Current_large_mallocked = 0;
  ret->Num_zeroes_allocated = 0;
  ret->Num_mallocs = 0;
  ret->Dummy_memory = 0;

  for (i=0; i<MAX_AM_MALLOC_SIZE;i++) {
    ret->Frees[i] = NULL;
    ret->Num_allocated[i]=0;
  }
  ret->Started = FALSE;

  ret->generation_number = 0;

  ret->Free_frees = NULL;
  ret->Frees_allocated = 0;

  ret->Check_already_freed = TRUE;

  ret->Num_previous_am_mallocs = 0;
  ret->Stop_at_nth_malloc = -1;
  ret->Stop_at_defined_by_argc_argv = FALSE;
#ifdef AMFAST
  ret->More_than_two_billion_mallocs = FALSE;
#endif

#if defined USE_TALLOC
  ret->malloc_ctx = NULL;
#endif

  ret->String_Arrays_mallocked = 0;
  ret->String_Arrays_freed = 0;

  ret->Dyvs_mallocked = 0;
  ret->Dyvs_freed = 0;

  return ret;
}

am_internal_state_t *_am_mk_internal_state(void)
{
  /* Do not use AM_MALLOC here. */
  am_internal_state_t *ret = malloc(sizeof(am_internal_state_t));

  ret->memory_state = _am_mk_memory_state();

  ret->user_interaction_mode = kUI_Shell;
  ret->Verbosity = 1;
  ret->error_ptr = my_error_default;
  ret->last_error = NULL;
  ret->print_ptr = my_print_default;
  ret->graphics_initialized = FALSE;

  /* Initialize thread related vars */
#ifdef USE_PTHREADS
  (void) pthread_key_create(&(ret->thread_specific_state_pthread_key), NULL);
  pthread_mutex_init(&(ret->pthread_am_malloc_mutex), NULL);
#endif
#ifdef WIN32
 InitializeCriticalSection(&ret->win32_am_malloc_mutex);
#endif

  /* Check if AM_VERBOSITY is set */
  {
    char *verbosity_str = getenv("AM_VERBOSITY");
    if (verbosity_str){
      bool ok = TRUE;
      int verbosity = int_from_string(verbosity_str, &ok);
      if (ok) ret->Verbosity=verbosity;
    }
  }    

  return ret;
}


/****** DESTROY internal_state ***************************************/

void _am_destroy_memory_state()
{
  am_memory_state_t *state = 
    am_get_memory_state(); /* might get thread specific memory */

  am_free_to_os(); /* give everything back we can to the os */

  /* Do not use AM_FREE here */
  free(state);  
}

void am_destroy_internal_state(void)
{
  _am_destroy_memory_state();

#ifdef USE_PTHREADS
  pthread_setspecific(g_state_ptr->thread_specific_state_pthread_key, NULL);
  pthread_key_delete(g_state_ptr->thread_specific_state_pthread_key);
  pthread_mutex_destroy(&(g_state_ptr->pthread_am_malloc_mutex));
#endif
#ifdef WIN32
  DeleteCriticalSection(&(g_state_ptr->win32_am_malloc_mutex));
#endif

  /* Do not use AM_FREE here */
  free(g_state_ptr);  
  g_state_ptr = NULL;
}



/******* INIT/ACCESS internal state ***************************/
am_internal_state_t *am_get_internal_state(void) 
{
  /* initialization of g_state_ptr must be done on the first call to
     am_get_internal_state since the auton code is not set up to 
     explicitely initialize the internal_state.
  */     
  if (!g_state_ptr){
    g_state_ptr = _am_mk_internal_state();
    _check_aggressive_abort(); /*call once to remove warning */
  }

#ifdef USE_PTHREADS 
#ifdef ASL_BUILD
  /*don't add function call overhead if not using threads */
  _check_aggressive_abort(); 
#endif
#endif

  return g_state_ptr;
}

/* accessors for data that might  be stored thread specific */
am_memory_state_t *am_get_memory_state(void)
{
  /* use am_get_internal_state() to ensure g_state_ptr is initialized */
  am_memory_state_t *ret = am_get_internal_state()->memory_state;

#ifdef USE_PTHREADS
  am_thread_specific_state_t *thread_state = am_get_thread_specific_state();
  if (thread_state && thread_state->memory_state) 
    ret = thread_state->memory_state;
#endif
  return ret;
}

my_error_ptr_t am_get_my_error_ptr(void)
{
  /* use am_get_internal_state() to ensure g_state_ptr is initialized */
  my_error_ptr_t ret = am_get_internal_state()->error_ptr;

#ifdef USE_PTHREADS
  am_thread_specific_state_t *thread_state = am_get_thread_specific_state();
  if (thread_state && thread_state->error_ptr) ret = thread_state->error_ptr;
#endif
  return ret;
}


/****** SET/REPLACE internal_state pointer *************************/

void am_set_internal_state(am_internal_state_t* new_state_ptr)
{
  /* free the previous data before assigning the new data */
  if (g_state_ptr && g_state_ptr != new_state_ptr)
    am_destroy_internal_state();

  g_state_ptr = new_state_ptr;    
}

void am_set_internal_state_exported(am_internal_state_t* new_state_ptr){
  am_set_internal_state(new_state_ptr);
}

void *am_switch_to_main_thread_memory()
{
  am_memory_state_t *memory_state = NULL;				
  am_thread_specific_state_t *thread_state = am_get_thread_specific_state(); 
  if (thread_state && thread_state->memory_state){
    memory_state = thread_state->memory_state;			    
    thread_state->memory_state = NULL;                              
  }  
  return (void *)memory_state;
}

void am_switch_to_thread_specific_memory(void *thread_mem)
{
  am_thread_specific_state_t *thread_state = am_get_thread_specific_state(); 
  if (thread_mem && thread_state)
    thread_state->memory_state = (am_memory_state_t *)thread_mem;
}

/******** INIT/DESTROY thread specific data ***************************/
void am_init_thread_specific_state()
{
#ifdef USE_PTHREADS
  /* Do not use AM_MALLOC here. */
  am_thread_specific_state_t *state=malloc(sizeof(am_thread_specific_state_t));
  state->memory_state= _am_mk_memory_state();
  state->error_ptr=g_state_ptr->error_ptr; /* gets initialized separately
					      if thread specific error is
					      really desired*/
  state->in_aggressive_abort=FALSE;
  state->graphics_initialized=FALSE;
  pthread_setspecific(g_state_ptr->thread_specific_state_pthread_key,
		      state);
#endif
}

void am_destroy_thread_specific_state()
{
#ifdef USE_PTHREADS
  am_thread_specific_state_t *thread_state = am_get_thread_specific_state();
  if (thread_state && thread_state->memory_state){
    _am_destroy_memory_state(); /* will know to destroy thread specific state ptr*/
    pthread_setspecific(g_state_ptr->thread_specific_state_pthread_key, NULL);
  }     
#endif
}

am_thread_specific_state_t *am_get_thread_specific_state()
{
  am_thread_specific_state_t *state = NULL;
#ifdef USE_PTHREADS
  state = pthread_getspecific(g_state_ptr->thread_specific_state_pthread_key);
#endif
  return state;
}

/********* over ride printing, error behavior *****************/
void set_my_error_ptr(my_error_ptr_t my_error_new)
{
  /* use am_get_internal_state() to ensure g_state_ptr is initialized */
  am_internal_state_t *state_ptr = am_get_internal_state();

#ifdef USE_PTHREADS
  am_thread_specific_state_t *thread_state = am_get_thread_specific_state();
  if (thread_state)
    thread_state->error_ptr = my_error_new;
  else
    state_ptr->error_ptr = my_error_new;
#else
  state_ptr->error_ptr = my_error_new;
#endif
}

void set_my_print_ptr(my_print_ptr_t my_print_new)
{
  /* use am_get_internal_state() to ensure g_state_ptr is initialized */
  am_get_internal_state()->print_ptr = my_print_new;
}

bool am_get_graphics_initialized(void)
{
  /* use am_get_internal_state() to ensure g_state_ptr is initialized */
  am_internal_state_t *state_ptr = am_get_internal_state();

#ifdef USE_PTHREADS
  am_thread_specific_state_t *thread_state = am_get_thread_specific_state();
  if (thread_state)
    return thread_state->graphics_initialized;
  else
    return state_ptr->graphics_initialized;
#else
    return state_ptr->graphics_initialized;
#endif
}

void am_set_graphics_initialized(bool is_initialized)
{
  /* use am_get_internal_state() to ensure g_state_ptr is initialized */
  am_internal_state_t *state_ptr = am_get_internal_state();

#ifdef USE_PTHREADS
  am_thread_specific_state_t *thread_state = am_get_thread_specific_state();
  if (thread_state)
    thread_state->graphics_initialized = is_initialized;
  else
    state_ptr->graphics_initialized = is_initialized;
#else
  state_ptr->graphics_initialized = is_initialized;
#endif
}

/* ----------- Internal helper funcitons ---------------------------- */

void _check_aggressive_abort()
{
#ifdef USE_PTHREADS
  /* aggressive abort means we my_error to jump to the
     thread's cleanup function.  We can only do this if 
     we are using thread specific state - otherwise we have
     no way to clean up the thread's memory.
  */
  am_thread_specific_state_t *thread_state = am_get_thread_specific_state();
  if (thread_state && 
      !thread_state->in_aggressive_abort)
  {
    thread_state->in_aggressive_abort = TRUE; /* avoid recursion */

    /*make sure we don't leave locks on since after my_error is called,
     the thread will be terminated */

#ifdef WIN32
    /*LeaveCriticalSection(&g_state_ptr->win32_am_malloc_mutex);*/
#endif
#ifdef USE_PTHREADS
  pthread_mutex_unlock(&g_state_ptr->pthread_am_malloc_mutex);
#endif
    /* use my_error as a callback signaling the code aborted */
    my_error("User Abort");
  }

  /* will never reach this point if the calling program has terminated
   the thread.*/
#endif
}


/* ------------------------------------------------------------------ */

