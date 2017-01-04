/*
   File:         amma_check.h
   Author:       Paul Komarek
   Created:      Mon Nov 29 15:29:42 EST 2004
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

#ifndef AMMA_CHECK_H
#define AMMA_CHECK_H



#include <stdio.h>
#include <time.h>

#include "amma_type.h"



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
void am_set_pre_guard( char *memory);

/*
  If not AMFAST, check first four bytes of tag area to detect
  corruption.  Returns 1 on corruption, 0 otherwise.
*/
int am_check_pre_guard( char *memory);
void am_check_pre_guard_fatal( char *memory, const char *msg);



/**************/
/* POST GUARD */
/**************/

/*
  If not AMFAST, set all four bytes of post-memory tag area to a magic
  value.  If AMFAST, complain.  The ext_size parameter is the total
  size allocated, including both tag areas.
*/
void am_set_post_guard( char *memory, int ext_size);

/*
  If not AMFAST, check all four bytes of post-memory tag area to
  detect corruption.  Returns 1 on corruption, 0 otherwise.
*/
int am_check_post_guard( char *memory, int ext_size);
void am_check_post_guard_fatal( char *memory, int ext_size, const char *msg);



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
int am_check_heap_nonglobal( amnode *amnode_buckets[]);

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
                                       int *r_ran);
void am_check_heap_sometimes_nonglobal_fatal( amnode *amnode_buckets[],
                                              double perc, int *r_ran,
                                              const char *msg);



/*****************************/
/* DEALLOCATED MEMORY CHECKS */
/*****************************/

/*
  This function will check a single free_list for corruption.  This can
  only be done when VERY_CAREFUL_CHECKING is enabled.
*/
int am_check_free_list( free_list *fl, int size);
void am_check_free_list_fatal( free_list *fl, int size, const char *msg);

/*
  This function will walk through freed blocks, checking that it has
  not been written to since it was freed.  The number of corrupted
  bytes is returned.  If corruption is found, we cannot tell you when
  it occurred or who was responsible.  However, this function can be
  used as part of a divide-and-conquer approach to localizing memory
  corruption.
*/
int am_check_free_space_nonglobal( free_list *frees[]);

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
                                             int *r_ran);
void am_check_free_space_sometimes_nonglobal_fatal( free_list *frees[],
                                                    double perc, int *r_ran,
                                                    const char *msg);



#endif /* ifndef AMMA_CHECK_H */

