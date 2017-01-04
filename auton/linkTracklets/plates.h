/*
   File:        plates.h
   Author:      J. Kubica
   Created:     Thu Nov 18, 2004
   Description: Plates (regions in RA/DEC)

   Copyright 2004, The Auton Lab, CMU

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

#ifndef PLATES_H
#define PLATES_H

#include "obs.h"

/* A simple set of bounds in RA/DEC */
typedef struct rd_plate {
  char* id;

  double time;
  double ra;
  double dec;
  double radius;   /* IN RADIANS */

} rd_plate;  


typedef struct rd_plate_array {
  int size;
  int max_size;

  rd_plate** rd_plates;
} rd_plate_array;


/* ------------------------------------------------------------------- */
/* -------- Rd_Plate Functions ------------------------------------------ */
/* ------------------------------------------------------------------- */

/* --- Memory Functions ---------------------------------------------- */

rd_plate* mk_empty_rd_plate(void);

/* RA is given in hours, dec in degreesm and radius in radians */
rd_plate* mk_rd_plate(char* id, double time, double ra, double dec, double radius);

/* RA is given in hours, dec in degreesm and radius in radians */
rd_plate* mk_rd_plate_simple(double time, double ra, double dec, double radius);

/* RA is given in hours, dec in degreesm and radius in radians */
rd_plate* mk_random_rd_plate(double t_min, double t_max,
                             double r_min, double r_max,
                             double d_min, double d_max,
                             double radius);

/* Takes a string with "ID TIME RA DEC RADIUS" */
/* Time is given in decimal MJD.               */
/* Radius is given in degrees.                 */
/* RAdegrees is true iff RA is given in degrees.  Otherwise uses */
/* RA in hours.                                                */
rd_plate* mk_rd_plate_from_string(char* str, bool RAdegrees);

rd_plate* mk_copy_rd_plate(rd_plate* old);

void free_rd_plate(rd_plate* old);


/* --- Getter/Setter Functions --------------------------------------- */

char* safe_rd_plate_id(rd_plate* p);

double safe_rd_plate_time(rd_plate* p);
double safe_rd_plate_RA(rd_plate* p);
double safe_rd_plate_DEC(rd_plate* p);
double safe_rd_plate_radius(rd_plate* p);

/* Allow a few speedups */
#ifdef AMFAST

#define rd_plate_id(p)            (p->id)
#define rd_plate_time(p)          (p->time)
#define rd_plate_RA(p)            (p->ra)
#define rd_plate_DEC(p)           (p->dec)
#define rd_plate_radius(p)        (p->radius)    /* RADIUS in radians */
#define rd_plate_deg_radius(p)    (p->radius * RAD_TO_DEG)

#else

#define rd_plate_id(p)            (safe_rd_plate_id(p))
#define rd_plate_time(p)          (safe_rd_plate_time(p))
#define rd_plate_RA(p)            (safe_rd_plate_RA(p))
#define rd_plate_DEC(p)           (safe_rd_plate_DEC(p))
#define rd_plate_radius(p)        (safe_rd_plate_radius(p))
#define rd_plate_deg_radius(p)    (safe_rd_plate_radius(p) * RAD_TO_DEG)

#endif


/* --- I/O Functions ------------------------------------------------- */

void fprintf_plate(FILE* f, char* pre, rd_plate* X, char* post);


/* ------------------------------------------------------------------- */
/* -------- Rd_Plate Array Functions ------------------------------------ */
/* ------------------------------------------------------------------- */

rd_plate_array* mk_empty_rd_plate_array(int size);

rd_plate_array* mk_copy_rd_plate_array(rd_plate_array* old);

rd_plate_array* mk_rd_plate_array_subset(rd_plate_array* old, ivec* inds);

rd_plate_array* mk_rd_plate_array_concat(rd_plate_array* A, rd_plate_array* B);

/* RAdegrees is true iff RA is given in degrees.  Otherwise uses */
/* RA in hours.                                                */
rd_plate_array* mk_load_rd_plate_array(char* filename, bool RAdegrees);

rd_plate_array* mk_random_rd_plate_array(int N, double t_min, double t_max,
					 double r_min, double r_max,
					 double d_min, double d_max,
					 double radius);

void free_rd_plate_array(rd_plate_array* old);

rd_plate* safe_rd_plate_array_ref(rd_plate_array* X, int index);

rd_plate* safe_rd_plate_array_first(rd_plate_array* X);

rd_plate* safe_rd_plate_array_last(rd_plate_array* X);

void rd_plate_array_set(rd_plate_array* X, int index, rd_plate* A);

void rd_plate_array_add(rd_plate_array* X, rd_plate* A);

int safe_rd_plate_array_size(rd_plate_array* X);

int safe_rd_plate_array_max_size(rd_plate_array* X);

int rd_plate_array_number_nonnull(rd_plate_array* X);

/* Allow a few speedups */
#ifdef AMFAST

#define rd_plate_array_size(X)     (X->size)
#define rd_plate_array_max_size(X) (X->max_size)
#define rd_plate_array_ref(X,i)    (X->rd_plates[i])
#define rd_plate_array_first(X)    (X->rd_plates[0])
#define rd_plate_array_last(X)     (X->rd_plates[X->size-1])

#else

#define rd_plate_array_size(X)     (safe_rd_plate_array_size(X))
#define rd_plate_array_max_size(X) (safe_rd_plate_array_max_size(X))
#define rd_plate_array_ref(X,i)    (safe_rd_plate_array_ref(X,i))
#define rd_plate_array_first(X)    (safe_rd_plate_array_first(X))
#define rd_plate_array_last(X)     (safe_rd_plate_array_last(X))

#endif

/* --- I/O Functions ------------------------------------------------- */

void fprintf_rd_plate_array(FILE* f, rd_plate_array* tarr);

#endif
