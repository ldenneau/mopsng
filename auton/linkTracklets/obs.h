/*
File:        obs.h
Author:      J. Kubica
Created:     Fri Sept 19 10:35:50 EDT 2003
Description: Header for the astronomy observations data structure and functions.

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

#ifndef ASTRO_OBS_H
#define ASTRO_OBS_H

#include "neos_header.h"
#include "namer.h"
#include "am_time.h"

/*#define NUM_FOR_QUAD    4 */     /* number of observations to use a
                                      quadratic approximation */

typedef struct simple_obs
{
  int id;                /* A (unique) ID number that can
                            be used to track individual
                            observations */

  double time;           /* Daycode + decimal days */
  double RA;             /* [0.0, 24.0] */
  double DEC;            /* [-90.0, 90.0] */
  float  brightness;

  int    obs_code;       /* 3 digit observation code */
  char   type;      
  char*  id_str;         /* Usually the 7 digit ID code + '\0'  */
                         /* Constrained to be AT LEAST 7 digits */
} simple_obs;


typedef struct simple_obs_array
{
  int size;
  int max_size;

  simple_obs** the_obs;
} simple_obs_array;


double time_from_MPC_string(char* str);

int dyv_count_num_unique(dyv* X, double thresh);

/* -----------------------------------------------------------------------*/
/* -------- Simple Observations ------------------------------------------*/
/* -----------------------------------------------------------------------*/


/* --- Memory functions for the simple observations ---------------------------- */

simple_obs* mk_simple_obs_simplest(int id, double time,
                                   double RA, double DEC, double brightness);

simple_obs* mk_simple_obs_simple(int id, int year, int month, double day,
                                 double RA, double DEC, double brightness);

/* Use 'V" as the default type
   Use NULL for obs_code and id_str if unknown */
simple_obs* mk_simple_obs(int id, int year, int month, double day, double RA, double DEC,
                          double brightness, char type, int obs_code, char* id_str);

simple_obs* mk_simple_obs_time(int id, double time, double RA, double DEC,
                          double brightness, char type, int obs_code, char* id_str);

simple_obs* mk_range_random_simple_obs(double time, int id,
                                       double RA_min, double RA_max,
                                       double DEC_min, double DEC_max,
                                       double bright_min, double bright_max);

simple_obs* mk_random_simple_obs(double time, int id);

simple_obs* mk_simple_obs_from_MPC_string(char* str, int id);

/* Makes an observation from a string with three numbers */
/* time, ra, dec                                         */
simple_obs* mk_simple_obs_from_three_string(char* str, int id);

simple_obs* mk_copy_simple_obs(simple_obs* old);

void free_simple_obs(simple_obs* old);



/* --- Simple Observation Access Functions ------------------------------------ */


int safe_simple_obs_id(simple_obs* X);

date* mk_simple_obs_date(simple_obs* X);

/* returns a global time (accounting for year/month/day/time) */
double safe_simple_obs_time(simple_obs* X);

double safe_simple_obs_RA(simple_obs* X);

double safe_simple_obs_DEC(simple_obs* X);

void simple_obs_set_RA_DEC(simple_obs* X, double r, double d);

double safe_simple_obs_brightness(simple_obs* X);

void safe_simple_obs_set_brightness(simple_obs* X, double brightness);

int safe_simple_obs_obs_code(simple_obs* X);

void simple_obs_set_obs_code(simple_obs* X, int code);

char* safe_simple_obs_id_str(simple_obs* X);

void simple_obs_set_id_str(simple_obs* X, char* id_str);

char safe_simple_obs_type(simple_obs* X);

void simple_obs_set_ID(simple_obs* X, int id);

void simple_obs_set_time(simple_obs* X, double time);

void simple_obs_set_RA(simple_obs* X, double RA);

void simple_obs_set_DEC(simple_obs* X, double DEC);

/* Return RA/DEC in radians... */
double safe_simple_obs_RA_rad(simple_obs* X);
double safe_simple_obs_DEC_rad(simple_obs* X);

/* Allow a few speedups */
#ifdef AMFAST

#define simple_obs_id(X)               (X->id)
#define simple_obs_time(X)             (X->time)
#define simple_obs_RA(X)               (X->RA)
#define simple_obs_DEC(X)              (X->DEC)
#define simple_obs_RA_rad(X)           (X->RA * 15.0 * DEG_TO_RAD)
#define simple_obs_DEC_rad(X)          (X->DEC * DEG_TO_RAD)
#define simple_obs_brightness(X)       (X->brightness)
#define simple_obs_set_brightness(X,b) (X->brightness = b)
#define simple_obs_obs_code(X)         (X->obs_code)
#define simple_obs_id_str(X)           (X->id_str)
#define simple_obs_type(X)             (X->type)

#else

#define simple_obs_id(X)               (safe_simple_obs_id(X))
#define simple_obs_time(X)             (safe_simple_obs_time(X))
#define simple_obs_RA(X)               (safe_simple_obs_RA(X))
#define simple_obs_DEC(X)              (safe_simple_obs_DEC(X))
#define simple_obs_RA_rad(X)           (safe_simple_obs_RA_rad(X))
#define simple_obs_DEC_rad(X)          (safe_simple_obs_DEC_rad(X))
#define simple_obs_brightness(X)       (safe_simple_obs_brightness(X))
#define simple_obs_set_brightness(X,b) (safe_simple_obs_set_brightness(X,b))
#define simple_obs_obs_code(X)         (safe_simple_obs_obs_code(X))
#define simple_obs_id_str(X)           (safe_simple_obs_id_str(X))
#define simple_obs_type(X)             (safe_simple_obs_type(X))

#endif


/* --- Simple Observation Helper Functions ------------------------------------ */

void simple_obs_add_gaussian_noise(simple_obs* X, double sig_ra, double sig_dec,
                                   double sig_bright);

/* gives x coordinate if the observation is projected onto the unit sphere */
double simple_obs_unit_X(simple_obs* X);

/* gives y coordinate if the observation is projected onto the unit sphere */
double simple_obs_unit_Y(simple_obs* X);

/* gives z coordinate if the observation is projected onto the unit sphere */
double simple_obs_unit_Z(simple_obs* X);

void simple_calc_XYZ_from_RADEC(double RA, double DEC, double* X,
                                double* Y, double *Z);

/* --- Simple Observation Other Functions ------------------------------------ */

double simple_obs_euclidean_distance(simple_obs* A, simple_obs* B);

double simple_obs_euclidean_dist_given(double RA1, double RA2,
                                       double DEC1, double DEC2);

/* Computes the weighted euclidean distance:
  
   d = sqrt[(dRA / w_ra)^2 + (dDEC / w_dec)^2]
*/
double simple_obs_weighted_euclidean_distance(simple_obs* A, simple_obs* B,
                                              double w_ra, double w_dec);

double angular_distance_RADEC(double ra1, double ra2, double dec1, double dec2);

double simple_obs_cosine_distance(simple_obs* A, simple_obs* B);


/* --- Output functions ------------------------------------------------------- */

void printf_simple_obs(simple_obs* S);

void fprintf_simple_obs(FILE* f, char* prefix, simple_obs* S, char* suffix);

void fprintf_MPC_simple_obs(FILE* f,simple_obs* S);

char* mk_MPC_string_from_simple_obs(simple_obs* S);

char* mk_string_from_simple_obs(simple_obs* S);


/* Computes the coefficients for the motion equation:

   X = M_0 + M_1 * t + M_2 * 0.5 * t^2

   will default to M_i = 0.0 if i > floor(log2(# points)) + 1
*/
dyv* mk_fill_obs_moments(dyv* X, dyv* time, dyv* weights, int num_coefficients);

dyv* mk_obs_RA_moments(simple_obs_array* obs, int num_coeff);

dyv* mk_obs_DEC_moments(simple_obs_array* obs, int num_coeff);


/* -----------------------------------------------------------------------*/
/* -------- Arrays Observations ------------------------------------------*/
/* -----------------------------------------------------------------------*/

simple_obs_array* mk_empty_simple_obs_array(int size);

simple_obs_array* mk_copy_simple_obs_array(simple_obs_array* old);

simple_obs_array* mk_simple_obs_array_subset(simple_obs_array* old, ivec* inds);

simple_obs_array* mk_simple_obs_array_concat(simple_obs_array* A, simple_obs_array* B);

void free_simple_obs_array(simple_obs_array* old);

simple_obs_array* mk_simple_obs_array_from_MPC_file(char *filename, int start_id);

/* The panstarrs string is a white space separated string with:               */
/* DETECTION_ID EPOCH_MJD RA_DEG DEC_DEG MAG OBSCODE OBJECT_NAME              */
/* The TRACKLET_NAME is optional so the code can be used with find tracklets. */
/* If true_groups and pairs are not null then they hold the true_group        */
/* information and the tracklet information (both need to be FREED if used).  */
/* True_groups is determined by matching object names.                        */
/* True_pairs is determined by matching detection_ids.                        */
/* LENGTH (column 9) is optional and gives the detection elongation (degrees) */
/*   It is placed in the dyv "length" in RADIANS.                             */
/* ANGLE (column 10) is optional and gives the elongation angle (degrees)     */
/*   It is placed in the dyv "angle" in RADIANS.                              */
/* EXPOSURE (column 11) is optional and gives the exposure time (sec)         */
/*   It is placed in the dyv "exp_time" in DAYS.                              */
simple_obs_array* mk_simple_obs_array_from_PANSTARRS_file(char *filename,
                                                          int start_id,
                                                          ivec** true_groups,
                                                          ivec** pairs,
                                                          dyv** length,
                                                          dyv** angle,
                                                          dyv** exp_time);

/* DES format contains
 0  ID
 1  TIME (MJD)
 2  OBS_TYPE (O usually)
 3  RA (deg)
 4  DEC (deg)
 5  APPMAG (mag)
 6  FILTER (grizy etc.)
 7  OBSERVATORY (obscode)
 8  RMS_RA (arcsec)
 9  RMS_DEC (arcsec)
10  RMS_MAG (mag)
11  S2N
12  Secret_name
*/
simple_obs_array* mk_simple_obs_array_from_DES_file(char *filename, int start_id, 
                                                          ivec** true_groups, ivec** pairs,
                                                          dyv** length, dyv** angle);

/* The megacam string is a white space separated string with:          */
/* RA (in degrees)   Dec (in degrees)   Flux   Sigma  Timestamp  Error */
/* The error column is optional.  If it is there and error!=NULL, then */
/* the errors are appended to the end of the error dyv.                */
/* min_sigma - the smallest simga to consider.                         */
simple_obs_array* mk_simple_obs_array_from_megacam_file(char *filename, double min_sigma,
                                                        int start_id, dyv* error);

/* The megacam string is a white space separated string with:          */
/* X  Y  Flux   Sigma  Timestamp  Error                                */
/* The error column is optional.  If it is there and error!=NULL, then */
/* the errors are appended to the end of the error dyv.                */
/* The function returns a dym with:                                    */
/* Timestamp, X, Y, Flux, Sigma                                        */
/* min_sigma - the smallest simga to consider.                         */
dym* mk_xy_obs_from_megacam_file(char *filename, double min_sigma,
                                 dyv* error);


/* Load the detections file from either MPC of PanSTARRS format.              */
/* Automatically decides which format the file is in (hopefully).             */
/* If true_groups and pairs are not null then they hold the true_group        */
/* information and the tracklet information (both need to be FREED if used).  */
simple_obs_array* mk_simple_obs_array_from_file_elong(char *filename,
                                                      double max_t_dist,
                                                      ivec** true_groups,
                                                      ivec** pairs,
                                                      dyv** length,
                                                      dyv** angle,
                                                      dyv** exp_time);

simple_obs_array* mk_simple_obs_array_from_file(char *filename, double max_t_dist,
                                                ivec** true_groups, ivec** pairs);


simple_obs_array* mk_simple_obs_array_from_three_file(char *filename, int start_id);

simple_obs* safe_simple_obs_array_ref(simple_obs_array* X, int index);

simple_obs* safe_simple_obs_array_first(simple_obs_array* X);

simple_obs* safe_simple_obs_array_last(simple_obs_array* X);

void simple_obs_array_set(simple_obs_array* X, int index, simple_obs* A);

void simple_obs_array_add(simple_obs_array* X, simple_obs* A);

int safe_simple_obs_array_size(simple_obs_array* X);

int safe_simple_obs_array_max_size(simple_obs_array* X);

int simple_obs_array_number_nonnull(simple_obs_array* X);

void simple_obs_array_sort_by_time(simple_obs_array* X);

/* Returns a vector with the same indices as in old_inds */
/* but re-ordered to by in ascending order with time.    */
ivec* mk_sort_simple_obs_array_indices(simple_obs_array* obs, ivec* old_inds);

/* Get the time of the earliest observation */
double simple_obs_array_first_time(simple_obs_array* X);

/* Get the time of the latest observation */
double simple_obs_array_last_time(simple_obs_array* X);

/* Allow a few speedups */
#ifdef AMFAST

#define simple_obs_array_size(X) (X->size)
#define simple_obs_array_max_size(X) (X->max_size)
#define simple_obs_array_ref(X,i) (X->the_obs[i])
#define simple_obs_array_first(X) (X->the_obs[0])
#define simple_obs_array_last(X)  (X->the_obs[X->size-1])

#else

#define simple_obs_array_size(X) (safe_simple_obs_array_size(X))
#define simple_obs_array_max_size(X) (safe_simple_obs_array_max_size(X))
#define simple_obs_array_ref(X,i) (safe_simple_obs_array_ref(X,i))
#define simple_obs_array_first(X) (safe_simple_obs_array_first(X))
#define simple_obs_array_last(X)  (safe_simple_obs_array_last(X))

#endif


void simple_obs_array_add_gaussian_noise(simple_obs_array* obs, double sig_ra, 
                                         double sig_dec, double sig_bright);

/* Break the observations up by which night the occur */
ivec_array* mk_break_up_by_night(simple_obs_array* obs, double gap);

void simple_obs_array_flatten_to_plates(simple_obs_array* obs, double plate_width);

dyv* mk_simple_obs_plate_times(simple_obs_array* obs, double plate_width);

dyv* mk_sorted_simple_obs_array_times(simple_obs_array* obs);

dyv* mk_simple_obs_array_times(simple_obs_array* obs);


/*
  obs    - the observations
  times  - a vector of the times you want indexed
*/
ivec_array* mk_time_to_simple_obs_ind(simple_obs_array* obs, dyv* times);


void fprintf_simple_obs_array(FILE* f, simple_obs_array* X);

void fprintf_MPC_simple_obs_array(FILE* f, simple_obs_array* X);

/* Print the obs array as: */
/* [time, RA, DEC]         */
void fprintf_simple_obs_array_as_RD_dym(FILE* f, simple_obs_array* X);

/* Print the obs array as: */
/* [time, RA, DEC]         */
void fprintf_simple_obs_array_RDT(FILE* f, simple_obs_array* X);


/* Create the true groups directly from the observation labels. */
/* Conv_ids - Indicates whether to change data from spacewatch  */
/*            format into standard format.                      */
ivec* mk_simple_obs_groups_from_labels(simple_obs_array* pts, bool conv_ids);

/* Make the intra-night linkages assuming: the true groups are known */
/* AND everything that occurred within max_t_dist of each other      */
/* should be linked together.                                        */
ivec_array* mk_simple_obs_pairing_from_true_groups(simple_obs_array* obs, ivec* groups,
                                                   double max_t_dist);


/* --- Bounds functions on a simple obs array --------------------------- */

/* Use inds=NULL to compute the information on all the observations. */
void simple_obs_array_compute_bounds(simple_obs_array* arr, ivec* inds,
                                     double *r_lo, double* r_hi,
                                     double *d_lo, double* d_hi,
                                     double *t_lo, double* t_hi);

/* Rotate all of the observations such that IF they were centered */
/* on (r_old, d_old) they are now centered on (r_new, d_new) AND  */
/* since it is a rotation all pairwise distances are maintained.  */
/* Use inds=NULL to compute the information on all the observations. */
void recenter_simple_obs_array(simple_obs_array* arr, ivec* inds,
                               double r_old, double r_new,
                               double d_old, double d_new,
                               double t_old, double t_new);

/* -----------------------------------------------------------------------*/
/* -------- Simple Bounds ------------------------------------------------*/
/* -----------------------------------------------------------------------*/

/* Compute the mid RA from a subset (or full set of inds == NULL) */
/* of RA values. */
double mid_RA(dyv* RA, ivec* inds);

/* Compute the width in RA from a subset */
/* (or full set of inds == NULL) of RA values. */
double width_RA(dyv* RA, ivec* inds);

double mid_DEC(dyv* DEC, ivec* inds);

#endif
