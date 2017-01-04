/*
  File:        auton_sf.h
  Author(s):   Sabhnani
  Created:     Mon Sep  5 10:45:35 EDT 2005
  Description: structure to help compute memory usage by a structure
  Copyright (c) Carnegie Mellon University

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

#ifndef auton_sf_H
#define auton_sf_H

/* This structure stores the memory utilization by a structure.

   A new function called auton_sf <struct name>_sizeof(<struct name> x)
   needs to written as mk_<struct name> and mk_copy_<struct name>.

   vec_size in amiv.h/c and pivecs_sizeof in pivecs.h/c 
   provide sample code.
*/

typedef struct auton_sf
{
  unsigned long overflows; /* number of overflows occured while 
			      computing the size */
  unsigned long size;      /* size of memory in bytes */
} auton_sf;

#include "amma.h"
#include "am_string.h"

auton_sf *mk_auton_sf(unsigned long overflows, unsigned long size);
   
auton_sf *mk_copy_auton_sf(auton_sf *x);

void free_auton_sf(auton_sf *x);

void fprintf_auton_sf(FILE *s, char *m1, auton_sf *x, char *m2);

void pauton_sf(auton_sf *x);

void add_to_auton_sf(auton_sf *x, unsigned long value);

#endif /* auton_sf_H */
