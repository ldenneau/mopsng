/*
File:        orbit.h
Author:      J. Kubica
Created:     Fri Oct 3 11:05:39 EDT 2003
Description: Header for the astronomical orbits data structures.

Copyright 2003, The Auton Lab, CMU

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


#ifndef ASTRO_ORBIT_H
#define ASTRO_ORBIT_H

#define ORBIT_ARRAY_SIZE   10

#include "obs.h"
/* #include "astro_util_funs.h" */
#include "track.h"

typedef struct orbit {
  double equinox; /* Equinox of orbits coordinate system.     */

  /* Parameters usually given... */
  double a;       /* semi major axis (in AU)                  */
  double e;       /* Eccentricity                             */
  double i;       /* Inclination (in radians)                 */
  double w;       /* Argument of the perihelion (in radians)  */
  double O;       /* Longitude of ascending node (in radians) */
  double M;       /* mean anomally (in radians)               */
  double H;       /* Absolute magnitude                       */
  double Epoch;   /* Orbit Epoch                              */

  /* Parameters usually calculated... */
  double p;       /* Orbital Parameter:   p = a*(1 - e*e)     */
  double q;       /* Perihelion Distance: q = p / (1+e)       */
                  /*                  or: q = a * (1-e)       */

  double n;       /* Mean motion around the sun: n = sqrt(GM_SUN/(a^3)) */
  double t0;      /* Time of perihelion passage (MJD)
		     t0 = Epoch - M/n                         */

} orbit;


typedef struct orbit_array {
  int max_size;
  int size;

  orbit** arr;
} orbit_array;



/* --- Memory functions for the simple orbits ---------------------------- */

orbit* mk_orbit(double a, double e, double i, double w, double O,
                double M, double H, double Epoch, double equinox);

orbit* mk_orbit2(double t0, double q, double e, double i,
                 double O, double w, double Epoch, double equinox);

orbit* mk_copy_orbit(orbit* old);

orbit* mk_orbit_from_simulated_string(char* str, double equinox);

orbit* mk_orbit_from_gorbit_string(char* str, double equinox);

orbit* mk_orbit_from_neworbit3_string(char* str, double equinox);

void free_orbit(orbit* old);


/* --- Orbit Access Functions -------------------------------------------- */

double safe_orbit_a(orbit* o);
double safe_orbit_semi_major_axis(orbit* o);

double safe_orbit_e(orbit* o);
double safe_orbit_eccentricity(orbit* o);

/* Return orbit inclination (in radians) */
double safe_orbit_i(orbit* o);
double safe_orbit_inclination(orbit* o);

/* Return the argument of the perihelion (in radians) */
double safe_orbit_w(orbit* o);
double safe_orbit_arg_of_perihelion(orbit* o);

/* Return the ascending node (in radians) */
double safe_orbit_O(orbit* o);
double safe_orbit_ascending_node(orbit* o);

/* Return the mean anomally (in radians) */
double safe_orbit_M(orbit* o);
double safe_orbit_mean_anomally(orbit* o);

double safe_orbit_H(orbit* o);
double safe_orbit_abs_magnitude(orbit* o);

double safe_orbit_q(orbit* o);
double safe_orbit_perhelion_dist(orbit* o);

double safe_orbit_p(orbit* o);
double safe_orbit_orbital_param(orbit* o);

double safe_orbit_n(orbit* o);
double safe_orbit_mean_motion(orbit* o);

double safe_orbit_t0(orbit* o);
double safe_orbit_perihelion_time(orbit* o);

double safe_orbit_epoch(orbit* o);

double safe_orbit_equinox(orbit* o);

double safe_orbit_period(orbit* o);

#ifdef AMFAST

#define orbit_a(x)                 (x->a)
#define orbit_semi_major_axis(x)   (x->a)
#define orbit_e(x)                 (x->e)
#define orbit_eccentricity(x)      (x->e)
#define orbit_i(x)                 (x->i)
#define orbit_inclination(x)       (x->i)
#define orbit_w(x)                 (x->w)
#define orbit_arg_of_perihelion(x) (x->w)
#define orbit_O(x)                 (x->O)
#define orbit_ascending_node(x)    (x->O)
#define orbit_M(x)                 (x->M)
#define orbit_mean_anomally(x)     (x->M)
#define orbit_H(x)                 (x->H)
#define orbit_abs_magnitude(x)     (x->H)
#define orbit_q(x)                 (x->q)
#define orbit_perhelion_dist(x)    (x->q)
#define orbit_p(x)                 (x->p)
#define orbit_orbital_param(x)     (x->p)
#define orbit_n(x)                 (x->n)
#define orbit_mean_motion(x)       (x->n)
#define orbit_t0(x)                (x->t0)
#define orbit_perihelion_time(x)   (x->t0)
#define orbit_epoch(x)             (x->Epoch)
#define orbit_equinox(x)           (x->equinox)
#define orbit_period(x)            ((2.0*PI)/x->n)

#else 

#define orbit_a(x)                 safe_orbit_a(x)
#define orbit_semi_major_axis(x)   safe_orbit_semi_major_axis(x)
#define orbit_e(x)                 safe_orbit_e(x)
#define orbit_eccentricity(x)      safe_orbit_eccentricity(x)
#define orbit_i(x)                 safe_orbit_i(x)
#define orbit_inclination(x)       safe_orbit_inclination(x)
#define orbit_w(x)                 safe_orbit_w(x)
#define orbit_arg_of_perihelion(x) safe_orbit_arg_of_perihelion(x)
#define orbit_O(x)                 safe_orbit_O(x)
#define orbit_ascending_node(x)    safe_orbit_ascending_node(x)
#define orbit_M(x)                 safe_orbit_M(x)
#define orbit_mean_anomally(x)     safe_orbit_mean_anomally(x)
#define orbit_H(x)                 save_orbit_H(x)
#define orbit_abs_magnitude(x)     safe_orbit_abs_magnitude(x)
#define orbit_q(x)                 safe_orbit_q(x)
#define orbit_perhelion_dist(x)    safe_orbit_perhelion_dist(x)
#define orbit_p(x)                 safe_orbit_p(x)
#define orbit_orbital_param(x)     safe_orbit_orbital_param(x)
#define orbit_n(x)                 safe_orbit_n(x)
#define orbit_mean_motion(x)       safe_orbit_mean_motion(x)
#define orbit_t0(x)                safe_orbit_t0(x)
#define orbit_perihelion_time(x)   safe_orbit_perihelion_time(x)
#define orbit_epoch(x)             safe_orbit_epoch(x)
#define orbit_equinox(x)           safe_orbit_equinox(x)
#define orbit_period(x)            safe_orbit_period(x)

#endif

/* --- Output functions ------------------------------------------ */

void printf_orbit(orbit* o);

void fprintf_orbit(FILE* f, char* prefix, orbit* o, char* suffix);
void fprintf_orbit2(FILE* f, char* prefix, orbit* o, char* suffix);
void fprintf_orbit3(FILE* f, char* prefix, orbit* o, char* suffix);


/* ---------------------------------------------------- */
/* --- Functions for the orbit array ------------------ */
/* ---------------------------------------------------- */

orbit_array* mk_empty_orbit_array(void);

orbit_array* mk_empty_orbit_array_sized(int size);

orbit_array* mk_copy_orbit_array(orbit_array* old);

orbit_array* mk_orbit_array_subset(orbit_array* old, ivec* inds);

orbit_array* mk_orbit_array_concat(orbit_array* A, orbit_array* B);

/* Loads in a series of orbits from a file.  Each line */
/* corresponds to a single orbit and has the form:     */
/* q e i O w t0 equinox                                */
/* q is given in AU; i,O,w are in degrees, and         */
/* t0,equinox are in MJD.                              */
/* If names != NULL and there is a 8th (optional) name */
/* column, it fills in the name array.                 */
orbit_array* mk_orbit_array_from_columned_file(char* filename, 
                                               string_array** names);

void free_orbit_array(orbit_array* old);

orbit* safe_orbit_array_ref(orbit_array* X, int index);

orbit* safe_orbit_array_first(orbit_array* X);

orbit* safe_orbit_array_last(orbit_array* X);

void orbit_array_set(orbit_array* X, int index, orbit* A);

void orbit_array_add(orbit_array* X, orbit* A);

void orbit_array_add_many(orbit_array* X, orbit_array* nu);

int safe_orbit_array_size(orbit_array* X);

int safe_orbit_array_max_size(orbit_array* X);

int orbit_array_number_nonnull(orbit_array* X);

/* Allow a few speedups */
#ifdef AMFAST

#define orbit_array_size(X) (X->size)
#define orbit_array_max_size(X) (X->max_size)
#define orbit_array_ref(X,i) (X->arr[i])
#define orbit_array_first(X) (X->arr[0])
#define orbit_array_last(X)  (X->arr[X->size-1])

#else

#define orbit_array_size(X) (safe_orbit_array_size(X))
#define orbit_array_max_size(X) (safe_orbit_array_max_size(X))
#define orbit_array_ref(X,i) (safe_orbit_array_ref(X,i))
#define orbit_array_first(X) (safe_orbit_array_first(X))
#define orbit_array_last(X)  (safe_orbit_array_last(X))

#endif

void fprintf_orbit_array(FILE* f, orbit_array* X);
void fprintf_orbit_array2(FILE* f, orbit_array* X);
void fprintf_orbit_array3(FILE* f, orbit_array* X);


#endif


