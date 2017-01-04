/*
  File:        auton_sf.c
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

#include "auton_sf.h"

auton_sf *mk_auton_sf(unsigned long overflows, 
		      unsigned long size)
{
  auton_sf *x = AM_MALLOC(auton_sf);

  x -> overflows = overflows;
  x -> size = size;

  return x;
}

auton_sf *mk_copy_auton_sf(auton_sf *old)
{
  auton_sf *new = AM_MALLOC(auton_sf);

  new -> overflows = old -> overflows;
  new -> size = old -> size;

  return new;
}
 
void free_auton_sf(auton_sf *x)
{
  AM_FREE(x,auton_sf);
}

void fprintf_auton_sf(FILE *s, char *m1, auton_sf *x, char *m2)
{
  char *buff;

  buff = mk_printf("%s -> overflows",m1);
  fprintf_ulong(s,buff,x->overflows,m2);
  free_string(buff);

  buff = mk_printf("%s -> size",m1);
  fprintf_ulong(s,buff,x->size,m2);
  free_string(buff);
}

void pauton_sf(auton_sf *x)
{
  fprintf_auton_sf(stdout, "auton_sf", x, "\n");
}

void add_to_auton_sf(auton_sf *x, unsigned long value)
{
  if ( ( ULONG_MAX - x->size ) < value )
    x->overflows++;

  x->size += value;
}
