/*
   File:         amma_check.c
   Author:       Paul Komarek
   Created:      Mon Nov 29 15:28:36 EST 2004
   Description:  Heap and free space checkers.

   Copyright 2004 Auton Lab

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

#include <stdio.h>
#include <time.h>

#include "amma_type.h"
#include "amma_check.h"
#include "am_time.h"


/*************************************************************************/
/* UTILS FOR CHECKING                                                    */
/*************************************************************************/

/* We need sub-second timers for our *_sometimes() functions.  The
   accuracy isn't very important.  The necessary code is in am_time.c,
   but we cannot risk using something that might make memory allocations.
*/

#ifdef PC_PLATFORM
int amma_check_gettimeofday(struct timeval* tp, struct timezone* UNUSED)
{
  struct _timeb tb;
  _ftime(&tb);
  if(tp != NULL) {
    tp->tv_sec = tb.time;
    tp->tv_usec = tb.millitm * 1000;
  }
  return 0;
}
#endif /* PC_PLATFORM */

static double amma_check_timeval_to_double(struct timeval this)
{
  double returner;
  returner = this.tv_usec + (1000000 * this.tv_sec);
  return returner;
}

static double amma_check_auton_stored_t_time;
static void amma_check_start_wc_timer(void)
{
  struct timeval ttv;
  gettimeofday(&ttv, NULL);
  amma_check_auton_stored_t_time = amma_check_timeval_to_double(ttv);
}

static double amma_check_stop_wc_timer(void)
{
  struct timeval ttv;
  gettimeofday(&ttv, NULL);
  return (amma_check_timeval_to_double(ttv) - amma_check_auton_stored_t_time);
}



/*************************************************************************/
/* MEM CHECKING FUNCTIONS                                                */
/*************************************************************************/

/*************/
/* PRE GUARD */
/*************/

/*
  If not AMFAST, set first four bytes of tag area to a magic value. If
  AMFAST, complain.
*/
void am_set_pre_guard( char *memory)
{
#ifdef AMFAST
  my_error( "am_set_pre_guard: "
            "THIS FUNCTION SHOULD NOT BE CALLED WHEN AMFAST IS SET.");
#endif
  memory[0] = 7;
  memory[1] = 3;
  memory[2] = 1;
  memory[3] = 0;
  return;
}

/*
  If not AMFAST, check first four bytes of tag area to detect
  corruption.  Returns 1 on corruption, 0 otherwise.
*/
int am_check_pre_guard( char *memory)
{
#ifdef AMFAST
  my_error( "am_check_pre_guard: "
            "THIS FUNCTION SHOULD NOT BE CALLED WHEN AMFAST IS SET.");
  return -1;
#else
  int num;

  num = 0;

  if (memory[0] != 7) {
    fprintf( stderr, "Corrupt pre guard: %p, byte +0, obs val (char) %d\n",
             (void *) memory, (char) memory[0]);
    num += 1;
  }

  if (memory[1] != 3) {
    fprintf( stderr, "Corrupt pre guard: %p, byte +1, obs val (char) %d\n",
             (void *) memory, (char) memory[1]);
    num += 1;
  }

  if (memory[2] != 1) {
    fprintf( stderr, "Corrupt pre guard: %p, byte +2, obs val (char) %d\n",
             (void *) memory, (char) memory[2]);
    num += 1;
  }

  if (memory[3] != 0) {
    fprintf( stderr, "Corrupt pre guard: %p, byte +3, obs val (char) %d\n",
             (void *) memory, (char) memory[3]);
    num += 1;
  }

  if (num>0) {
    fprintf( stderr, "am_check_pre_guard: corrupt.   User's mem was %p\n",
             (void *) compute_userptr( memory));
  }

  return num;
#endif
}

void am_check_pre_guard_fatal( char *memory, const char *msg)
{
  if (am_check_pre_guard( memory)) {
    my_errorf( "%s: PRE GUARD IS CORRUPTED.\n"
               "The memory you have just tried to free has been corrupted.\n"
               "Someone has illegally written into the beginning of this\n"
               "memory,at least into the pre-memory tag area used with\n"
               "non-AMFAST programs.  We do not know when this memory was\n"
               "corrupted.  You can call am_check_heap() any time while your\n"
               "program is running to check for signs that corruption has\n"
               "occurred.  Another option is to create a t=verycareful\n"
               "build, which will call am_check_heap() every time memory is\n"
               "freed.\n");
  }
  return;
}



/**************/
/* POST GUARD */
/**************/

/*
  If not AMFAST, set all four bytes of post-memory tag area to a magic
  value.  If AMFAST, complain.  The ext_size parameter is the total
  size allocated, including both tag areas.
*/
void am_set_post_guard( char *memory, int ext_size)
{
#ifdef AMFAST
  my_error( "am_set_post_guard: "
            "THIS FUNCTION SHOULD NOT BE CALLED WHEN AMFAST IS SET.");
#endif
  memory[ext_size-4] = 8;
  memory[ext_size-3] = 12;
  memory[ext_size-2] = 14;
  memory[ext_size-1] = 15;
  return;
}

/*
  If not AMFAST, check all four bytes of post-memory tag area to
  detect corruption.  Returns 1 on corruption, 0 otherwise.
*/
int am_check_post_guard( char *memory, int ext_size)
{
#ifdef AMFAST
  my_error( "am_check_post_guard: "
            "THIS FUNCTION SHOULD NOT BE CALLED WHEN AMFAST IS SET.");
  return -1;
#else
  char *pg;
  int num;

  pg = memory + ext_size - 4;
  num = 0;

  if (pg[0] != 8) {
    fprintf( stderr, "Corrupt post guard: %p, byte +0, obs val (char) %d\n",
             (void *) pg, (char) pg[0]);
    num += 1;
  }

  if (pg[1] != 12) {
    fprintf( stderr, "Corrupt post guard: %p, byte +1, obs val (char) %d\n",
             (void *) pg, (char) pg[1]);
    num += 1;
  }

  if (pg[2] != 14) {
    fprintf( stderr, "Corrupt post guard: %p, byte +2, obs val (char) %d\n",
             (void *) pg, (char) pg[2]);
    num += 1;
  }

  if (pg[3] != 15) {
    fprintf( stderr, "Corrupt post guard: %p, byte +3, obs val (char) %d\n",
             (void *) pg, (char) pg[3]);
    num += 1;
  }

  if (num>0) {
    fprintf( stderr, "am_check_post_guard: corrupt.   User's mem was %p\n",
             (void *) compute_userptr( memory));
  }

  return num;
#endif
}

void am_check_post_guard_fatal( char *memory, int ext_size, const char *msg)
{
  if (am_check_post_guard( memory, ext_size)) {
    my_errorf( "%s: POST GUARD IS CORRUPTED.\n"
               "The memory you have just tried to free has been corrupted.\n"
               "Someone has illegally written into the end of this memory,\n"
               "at least into the post-memory tag area used with non-AMFAST\n"
               "programs.  We do not know when this memory was corrupted.\n"
               "You can call am_check_heap() any time while your program is\n"
               "running to check for signs that corruption has occurred.\n"
               "Another option is to create a t=verycareful build, which\n"
               "will call am_check_heap() every time memory is freed.\n",
               msg);
  }
  return;
}



/***************************/
/* ALLOCATED MEMORY CHECKS */
/***************************/

/*
  This function will walk through the heap, checking all allocated
  memory for signs of corruption.  The number of corrupted amnodes
  found is returned.  Not all types of corruption can be found.  If
  corruption is found, we cannot tell you when it occurred or who was
  responsible.  However, this function can be used as part of a
  divide-and-conquer approach to localizing memory corruption.  This
  function can only be called when AMFAST is *not* set.
*/
int am_check_heap_nonglobal( amnode *amnode_buckets[])
{
#ifdef AMFAST
  my_error( "am_check_heap_nonglobal: "
            "THIS FUNCTION SHOULD NOT BE CALLED WHEN AMFAST IS SET.");
  return 1;
#else
  int bucketidx, pre, post, count, numpre, numpost, numcount, ext_size;
  char *s, *memory, *postarea;
  amnode *amn;

  s = "HEAP ERROR";
  numpre = 0;
  numpost = 0;
  numcount = 0;

  /* For each bucket in amnode_buckets, traverse the linked list of
     amnodes.  For each amnode, check the pre-guard and possibly the
     post-guard. */
  for (bucketidx=0; bucketidx < NUM_COUNT_BUCKETS; ++bucketidx) {
    amn = amnode_buckets[bucketidx];
    while (amn != NULL) {
      /* Check memory for node. */
      memory = amn->memory;
      ext_size = compute_extended_size( amn->size);

      pre = am_check_pre_guard( memory);
      post = am_check_post_guard( memory, ext_size);
      count = amn->count != memory_count_ref( memory);

      /* Corruption detected. */
      if (pre || post) {
        if (pre) {
          fprintf( stderr,
                   "%s: amnode %p pre-guard is corrupted (%d%d%d%d).\n",
                   s, (void *) amn, memory[0], memory[1], memory[2],
                   memory[3]);
        }
        if (post) {
          postarea = memory + compute_extended_size( amn->size);
          fprintf( stderr,
                   "%s: amnode %p post-guard is corrupted (%d%d%d%d).\n",
                   s, (void *) amn, memory[ext_size-4], memory[ext_size-3],
                   memory[ext_size-2], memory[ext_size-1]);
        }
      }

      /* No corruption detected, check count. */
      else if (count) {
        fprintf( stderr,
                 "%s: amnode %p has count %d, but memory has count %d.\n",
                 s, (void *) amn, amn->count, memory_count_ref( memory));
      }

      /* Tally errors. */
      if (pre || post || count) {
        fprintf( stderr, "\n");
        numpre += pre;
        numpost += post;
        numcount += count;
      }

      /* Advance to next amnode in linked list. */
      amn = amn->next;
    }
  }

  /* Print error summary. */
  if (numpre || numpost || numcount) {
    fprintf( stderr, "Found %d corruptions of a pre-guard.\n", numpre);
    fprintf( stderr, "Found %d corruptions of a post-guard.\n", numpost);
    fprintf( stderr, "Found %d corruptions of a count.\n", numcount);
    fprintf( stderr, "\n");
  }

  return int_max( int_max(numpre,numpost), numcount);
#endif
}

/*
  This function will call am_check_heap() if enough time has passed
  since the previous invocation.  The first time it is called,
  am_check_heap() is run and the time required is recorded.  On
  subsequent invocations, the previous runs duration is compared to
  the amount of time since the previous run started.  If s is the time
  of our last call to am_check_free_space(), and d is the duration of
  that run, then we will run if

    duration < (now-s)*perc

  where perc is the "percentage of runtime" passed to this function.
*/
int am_check_heap_sometimes_nonglobal( amnode *amnode_buckets[], double perc,
                                       int *r_ran)
{
  static time_t start = 0;
  static double duration = -1.0;
  static unsigned long between = 0;
  int run, numbytes;
  double runtime, oldperc;

#ifdef AMFAST
  my_error( "am_check_heap_sometimes_nonglobal:\n"
            "Do not call this function while the AMFAST macro is defined.\n"
            "For instance, try building with t=debug or t=verycareful.\n");
#endif

  /* Check runtime. */
  run = 0;
  oldperc = 0.0;
  if (start > 0) {
    runtime = difftime( time(NULL), start);
    oldperc = duration / runtime;
    if (oldperc < perc) run = 1;
    else between += 1;
  }
  else run = 1;

  /* Run if needed. */
  if (run) {
    start = time(NULL);
    amma_check_start_wc_timer();
    numbytes = am_check_heap_nonglobal( amnode_buckets);
    duration = amma_check_stop_wc_timer() / 1000000;
    printf( "checked heap(%ld), oldp=%.g, skip=%ld, perc=%g, dur=%f\n",
            (long) start, oldperc, between, perc, duration);
    between = 0;
  }
  else numbytes = 0;

  /* Done. */
  if (r_ran != NULL) *r_ran = run;
  return numbytes;
}

void am_check_heap_sometimes_nonglobal_fatal( amnode *amnode_buckets[],
                                              double perc, int *r_ran,
                                              const char *msg)
{
  if (am_check_heap_sometimes_nonglobal( amnode_buckets, perc, r_ran)) {
    my_errorf( "%s: Corruption has been detected in allocated memory.\n"
               "You can call am_check_heap() manually to isolate the first\n"
               "corruption of your heap.  AMFAST must not be defined\n"
               "when calling this function.  For instance, t=debug or\n"
               "t=verycareful builds meet this criterion.\n",
               msg);
  }
  return;
}



/*****************************/
/* DEALLOCATED MEMORY CHECKS */
/*****************************/

/*
  This function will check a single free_list for corruption.  This can
  only be done when AMFAST is not set.
*/
int am_check_free_list( free_list *fl, int size)
{
  int i, numbytes;
  char *mem, *s;

#ifdef AMFAST
  my_error( "am_check_free_list:\n"
            "Do not call this function when AMFAST is defined.\n"
            "For instance, try building with t=debug or t=verycareful.\n");
#endif

  s = "FREE_LIST ERROR";
  numbytes = 0;

  /* Check for corruption of freed memory. */
  mem = fl->memory;
  for (i=0; i<size; ++i) {
    if (mem[i] != (char) FREE_MAGIC) {
      fprintf( stderr,
               "%s: mem %p, size %d, offset %d: (char) %d != "
               "magic (char) %d.\n",
               s, (void *) mem, size, i, (char) mem[i], (char) FREE_MAGIC);
      numbytes += 1;
    }
  }

  return numbytes;
}

void am_check_free_list_fatal( free_list *fl, int size, const char *msg)
{
  if (am_check_free_list( fl, size)) {
    my_errorf( "%s: am_check_free_list() found corruption\n"
               "in memory you are about to receive, which is memory you\n"
               "have previously freed.  You can call am_check_free_space()\n"
               "manually to isolate the first corruption of your heap.\n"
               "Note that you need to build without AMFAST defined in order\n"
               "to call am_check_free_space().  For instance, building with\n"
               "t=debug or t=verycareful satisfies this condition.\n",
               msg);
  }
  return;
}

/*
  This function will walk through freed blocks, checking that it has
  not been written to since it was freed.  The number of corrupted
  bytes is returned.  If corruption is found, we cannot tell you when
  it occurred or who was responsible.  However, this function can be
  used as part of a divide-and-conquer approach to localizing memory
  corruption.
*/
int am_check_free_space_nonglobal( free_list *frees[])
{
  int freesize, numbytes;
  free_list *fl;

#ifdef AMFAST
  my_error( "am_check_free_space:\n"
            "Do not call this function when AMFAST is defined.\n"
            "For instance, try building with t=debug or t=verycareful.\n");
#endif


  /* Iterate over Frees, where each bucket starts a linked list of free_list
     structs, and each idx is the number of bytes allocated to the free_list
     structs' memory pointers.  Because we only ever assign to frees indices
     that are powers of two, we only check those as well.  */
  numbytes = 0;
  freesize = 1;
  while(1) {
    if (freesize >= MAX_AM_MALLOC_SIZE) break;
    fl = frees[freesize];

    /* Traverse all free_list structures for freesize. */
    while (fl != NULL) {
      numbytes += am_check_free_list( fl, freesize);
      fl = fl->next;
    }

    freesize <<= 1;
  }

  /* Print error summary. */
  if (numbytes) {
    fprintf( stderr, "Found %d corrupted bytes in freed memory.\n", numbytes);
    fprintf( stderr, "\n");
  }

  return numbytes;
}

/*
  This function will call am_check_free_space() if enough time has
  passed since the previous invocation.  The first time it is called,
  am_check_free_space() is run and the time required is recorded.  On
  subsequent invocations, the previous runs duration is compared to
  the amount of time since the previous run started.  If s is the time
  of our last call to am_check_free_space(), and d is the duration of
  that run, then we will run if

    duration < (now-s)*perc

  where perc is the "percentage of runtime" passed to this function.
*/
int am_check_free_space_sometimes_nonglobal( free_list *frees[], double perc,
                                             int *r_ran)
{
  static time_t start = 0;
  static double duration = -1.0;
  static unsigned long between = 0;
  int run, numbytes;
  double runtime, oldperc;

#ifdef AMFAST
  my_error( "am_check_free_space:\n"
            "Do not call this function when AMFAST is defined.\n"
            "For instance, try building with t=debug or t=verycareful.\n");
#endif


  /* Check runtime. */
  run = 0;
  oldperc = 0.0;
  if (start > 0) {
    runtime = difftime( time(NULL), start);
    oldperc = duration / runtime;
    if (oldperc < perc) run = 1;
    else between += 1;
  }
  else run = 1;

  /* Run if needed. */
  if (run) {
    start = time(NULL);
    amma_check_start_wc_timer();
    numbytes = am_check_free_space_nonglobal( frees);
    duration = amma_check_stop_wc_timer() / 1000000;
    printf( "checked free(%ld), oldp=%.g, skip=%ld, perc=%g, dur=%f\n",
            (long) start, oldperc, between, perc, duration);
    between = 0;
  }
  else numbytes = 0;

  /* Done. */
  if (r_ran != NULL) *r_ran = run;
  return numbytes;
}


void am_check_free_space_sometimes_nonglobal_fatal( free_list *frees[],
                                                    double perc, int *r_ran,
                                                    const char *msg)
{
  if (am_check_free_space_sometimes_nonglobal( frees, perc, r_ran)) {
    my_errorf( "%s: am_check_free_space() found corruption in memory you\n"
               "have previously freed.  You can call am_check_free_space()\n"
               "manually to isolate the first corruption of your heap.\n"
               "Note that you need to build without AMFAST defined in order\n"
               "to call am_check_free_space().  For instance, building with\n"
               "t=debug or t=verycareful satisfies this condition.\n",
               msg);
  }
  return;
}
