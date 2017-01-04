/*
   File:        backtrace.h
   Author:      Paul Komarek
   Created:     Sun Oct 20 22:57:35 EDT 2002
   Description: Facilities to produce backtraces from running code.

   Copyright 2002, AUTON Lab, Carnegie Mellon University.

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


/* As of Sun Oct 20 22:57:35 EDT 2002, the backtrace functionality for
   GNU CC combined with GLIBC only returns function names from shared
   objects (you can get addresses from statically linked stuff).
   I have added a btdebug compile type to Make.common which does the right
   stuff.  To get the most from these backtrace utiltities (at least
   under GNU CC and GLIBC), compile with "gmake -DBTDEBUG".  This is
   currently done automatically by Make.common.
*/

#ifndef __BACKTRACE_H__
#define __BACKTRACE_H__

#include <stdlib.h>
#include "standard.h"

typedef struct bt_string_array_struct {
  int size;
  char **arr;
} bt_string_array;

/* Structure to hold function names and other stuff. */
typedef struct btinfo_struct {
  int maxdepth;      /* Maximum number of stack frames to print. */
  bt_string_array *funcnames;
} btinfo;

/* Public API. */
btinfo *mk_btinfo( int maxdepth);
void fprintf_btinfo( FILE *f, char *preline, btinfo *bt, char *postline);
void free_btinfo( btinfo *bt);

/* Easy ways to print backtrace. */
void fprintf_backtrace( FILE *f, int numlevels, char *pre);
void printf_backtrace( void);

/* Helpers for catching SIGSEGV (segfault). */
extern void backtrace_sigsegv( int sig);
void backtrace_install_sigsegv_handler( void);
extern void backtrace_sigusr2( int sig);
void backtrace_install_sigusr2_handler( void);


/*************************************************************************/
/* Stuff below is for implementors.                                      */
/*************************************************************************/

/* To add a new backtrace implementation, do the following things:

   1) Add a function for your fill_btinfo_funcnames_<platform> function
      below.  #include any necessary header files.  Wrap the whole mess
      in #ifdefs so other platforms don't look for these header files.

   2) Add a fill_btinfo_funcnames_<platform> function to backtrace.c.
      Wrap your function in the same #ifdefs that you use below.

   3) Add a call to your fill_btinfo_funcnames_<platform> function
      to fill_btinfo_funcnames() in backtrace.c.  Wrap that call in the
      same #ifdefs that you use below.

   For anyone who has a better way of doing this, please consider the
   following before overhauling the system:

   a) We have conditionally included headers.
   b) We want prototypes for these fill_btinfo_funcnames_<platform>
      function so we can rearrange callers and callees in backtrace.c.
   c) Part or all of the fill_btinfo_funcnames_<platform> functions will
      be enclosed in #ifdefs.

   If we sacrifice b, we can eliminate one of the three places authors
   need to put #ifdef statements, but we lose flexibility.

   That said, feel free to improve this setup.
*/

void fill_btinfo_funcnames( btinfo *bt);

/* For gcc, use "echo | cpp -dM" to get a list of predefined symbols like
   __i386__, etc.  The echo and pipe are there to create an "empty .h" file.
*/

/* GCC + GLIBC */
#ifdef __GNUC__
#ifdef __GLIBC__
#if __i386__ || __x86_64__
#include <execinfo.h>
void fill_btinfo_funcnames_gcc_glibc_i386( btinfo *bt);
#endif
#endif
#endif

#endif /* __BACKTRACE_H__ */
