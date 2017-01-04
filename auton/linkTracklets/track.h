/*
File:        track.h
Author:      J. Kubica
Created:     Mon Dec 22 16:36:39 EDT 2003
Description: Header for functions on the "track" data structure.
             A tracks is a set of astronomical observations and corresponding
             motion parameters in (RA, dec) coordinates.

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

#ifndef TRACK_H
#define TRACK_H

#define TRACK_TIME_DIFF   1e-5

#include "neos_header.h"
#include "am_time.h"
#include "obs.h"
#include "sb_graph.h"
#include "eq_solvers.h"

typedef struct track
{
  ivec* simp_ind;

  double time;        /* First time */

  /* Motion coeff's for (RA,DEC) */
  double RA_m[3];
  double DEC_m[3];

  float  brightness;  /* Average brightness */
} track;


typedef struct track_array
{
  int size;
  int max_size;
  track** the_obs;
} track_array;




/* -----------------------------------------------------------------------*/
/* -------- Tracks -------------------------------------------------------*/
/* -----------------------------------------------------------------------*/


/* --- Memory functions for the joint observations ---------------------- */

track* mk_track_single_ind(simple_obs_array* all_obs, int ind);

track* mk_track_pair_ind(simple_obs_array* all_obs, int A, int B);

track* mk_copy_track(track* old);

track* mk_track_from_N_inds(simple_obs_array* all_obs, ivec* inds);

track* mk_combined_track(track* A, track* B, simple_obs_array* all_obs);

track* mk_track_add_one(track* old, simple_obs* nu, simple_obs_array* all_obs);

track* mk_track_add_one_ind(track* old, int ind, simple_obs_array* all_obs);

void free_track(track* old);


/* --- Observation Access Functions ------------------------------------ */

int safe_track_num_obs(track* X);

date* mk_track_date(track* X);

/* returns a global time (accounting for year/month/day/time) */
double safe_track_time(track* X);

double safe_track_RA(track* X);
double safe_track_vRA(track* X);          /* change in RA per time unit */
double safe_track_aRA(track* X);          /* returns 0.0 if num_obs < 3 */
double track_indiv_RA(track* X, simple_obs_array* all_obs, int obs_num);

double safe_track_DEC(track* X);
double safe_track_vDEC(track* X);          /* change in DEC per time unit */
double safe_track_aDEC(track* X);          /* returns 0.0 if num_obs < 3 */
double track_indiv_DEC(track* X, simple_obs_array* all_obs, int obs_num);

double safe_track_brightness(track* X);

void track_set_RA(track* X, double nuVal);
void track_set_vRA(track* X, double nuVal);
void track_set_aRA(track* X, double nuVal);

void track_set_DEC(track* X, double nuVal);
void track_set_vDEC(track* X, double nuVal);
void track_set_aDEC(track* X, double nuVal);

simple_obs* safe_track_indiv(track* X, int obs_num, simple_obs_array* all_obs);

simple_obs* safe_track_first(track* X, simple_obs_array* all_obs);

simple_obs* safe_track_last(track* X, simple_obs_array* all_obs);

double safe_track_first_time(track* X, simple_obs_array* obs);

double safe_track_last_time(track* X, simple_obs_array* obs);

double safe_track_time_length(track* X, simple_obs_array* obs);

/* Track time length rounded up */
double track_num_nights(track* X, simple_obs_array* obs);

/* On how many different nights did we see observations. */
/* Effectively counts the number of tracklets making     */
/* up the track.                                         */
int track_num_nights_seen(track* X, simple_obs_array* obs);

ivec* safe_track_individs(track* X);

void incr_track_name2(char* nme);

/* Allow a few speedups */
#ifdef AMFAST

#define track_num_obs(X)     (ivec_size(X->simp_ind))
#define track_size(X)        (ivec_size(X->simp_ind))
#define track_time(X)        (X->time)
#define track_RA(X)          (X->RA_m[0])
#define track_vRA(X)         (X->RA_m[1])
#define track_aRA(X)         (X->RA_m[2])
#define track_DEC(X)         (X->DEC_m[0])
#define track_vDEC(X)        (X->DEC_m[1])
#define track_aDEC(X)        (X->DEC_m[2])
#define track_brightness(X)  ((double)X->brightness)
#define track_indiv(X,i,all) (simple_obs_array_ref(all,ivec_ref(X->simp_ind,i)))
#define track_first(X,all)   (simple_obs_array_ref(all,ivec_ref(X->simp_ind,0)))
#define track_last(X,all)    (simple_obs_array_ref(all,ivec_ref(X->simp_ind,ivec_size(X->simp_ind)-1)))
#define track_individs(X)    (X->simp_ind)
#define track_first_time(X,all)  (simple_obs_time(track_first(X,all)))
#define track_last_time(X,all)   (simple_obs_time(track_last(X,all)))
#define track_time_length(X,all) (simple_obs_time(track_last(X,all)) - simple_obs_time(track_first(X,all)))

#else

#define track_num_obs(X)     (safe_track_num_obs(X))
#define track_size(X)        (safe_track_num_obs(X))
#define track_time(X)        (safe_track_time(X))
#define track_RA(X)          (safe_track_RA(X))
#define track_vRA(X)         (safe_track_vRA(X))
#define track_aRA(X)         (safe_track_aRA(X))
#define track_DEC(X)         (safe_track_DEC(X))
#define track_vDEC(X)        (safe_track_vDEC(X))
#define track_aDEC(X)        (safe_track_aDEC(X))
#define track_brightness(X)  (safe_track_brightness(X))
#define track_indiv(X,i,all) (safe_track_indiv(X,i,all))
#define track_first(X,all)   (safe_track_first(X,all))
#define track_last(X,all)    (safe_track_last(X,all))
#define track_individs(X)    (safe_track_individs(X))
#define track_first_time(X,all)  (safe_track_first_time(X,all))
#define track_last_time(X,all)   (safe_track_last_time(X,all))
#define track_time_length(X,all) (safe_track_time_length(X,all))

#endif


/* Return the observation that occurred at t or -1
   if no such observation */
int track_at_t(track* X, double t, simple_obs_array* all_obs);

bool track_equal(track* A, track* B);

/* Do A and B overlap at some point AND not have
   two DIFFERENT observations at any point */
bool track_valid_overlap(track* A, track* B, simple_obs_array* arr);

/* Is B a subset of A? */
bool track_subset(track* A, track* B, simple_obs_array* obs);

/* Is B a proper subset of A? */
bool track_proper_subset(track* A, track* B, simple_obs_array* obs);

int track_overlap_size(track* A, track* B, simple_obs_array* obs);

ivec* mk_track_overlap(track* A, track* B);

/* Are all of the track's time in order and unique */
bool track_times_valid(track* A, simple_obs_array* obs);

bool track_overlap(track* A, track* B, simple_obs_array* obs);

bool track_overlap_in_time(track* A, track* B, simple_obs_array* arr);

/* Do A and B have two DIFFERENT observations at any time */
bool track_collide_in_time(track* A, track* B, simple_obs_array* arr);


/* ---------------------------------------------------------------------- */
/* --- Track Distance Functions ----------------------------------------- */
/* ---------------------------------------------------------------------- */

/* Just compute the distance with RA and DEC */
double track_euclidean_distance(track* A, track* B);

double track_midpt_distance(track* A, track* B);

/* Generalized midpoint distance.  Calculates the midpoint distance */
/* using time = A.time + beta * (B.time - A.time).                  */
/* Example: A linear, B linear -> use beta = 0.5                    */
/*          A quad,   B linear -> use beta = 1.0                    */
double track_gen_midpt_distance(track* A, track* B, double beta);

double track_obs_euclidean_distance(track* A, simple_obs* B);


/* ---------------------------------------------------------------------- */
/* --- Track Prediction Functions --------------------------------------- */
/* ---------------------------------------------------------------------- */

/* Forcibly set t0: Move the RA, vRA, DEC, vDEC, etc.
   estimates to correspond with shifting the initial
   time to t0.  This change is nonlasting in that
   if the paired observation is used in a combine or
   add later it will vanish.  This can also be reversed
   with "track_unforce_t0" */
void track_force_t0(track* X, double t0);

void track_force_t0_first(track* X, simple_obs_array* all_obs);

void track_force_t0_last(track* X, simple_obs_array* all_obs);

/* Creates a predicted point from the current point
   and an ellapsed time.  Note the MPC strings for the
   simple observations WILL BE INCORRECT! */
track* mk_track_prediction(track* X, double ellapsed_time);

/* Creates a predicted point from the current point
   and an ellapsed time.  Note the MPC strings for the
   simple observations WILL BE INCORRECT! */
simple_obs* mk_simple_obs_track_prediction(track* X, double ellapsed_time);

/* Makes a simple linear prediction for RA and DEC only and
   stores them in the two given doubles. */
void track_RA_DEC_prediction(track* X, double ellapsed_time,
                             double* pred_RA, double* pred_DEC);


/* Makes a simple linear prediction for (RA, DEC, vRA, vDEC) and */
/* stores them in the given doubles. */
void track_RDVV_prediction(track* X, double ellapsed_time,
                           double* pred_RA, double* pred_DEC,
                           double* pred_vRA, double* pred_vDEC) ;

/* Returns an estimated angular velocity of a linear track. */
/* In radians per day.                                      */
double linear_track_estimated_angular_vel(track* X);

/* Modifies the observations in the track so the fit on the plates */
void track_flatten_to_plates(track* A, simple_obs_array* arr, 
                             dyv* plates, double plate_width);

/* Modifies the observations in the track so they are at their original times */
void track_unflatten_to_plates(track* A, simple_obs_array* arr, dyv* org_times);


/* ---------------------------------------------------------------------- */
/* --- Track Residual Functions ----------------------------------------- */
/* ---------------------------------------------------------------------- */

/* Returns the residuals from the track fit. */
dyv* mk_track_residuals(track* X, simple_obs_array* arr);

dyv* mk_track_RA_residuals(track* X, simple_obs_array* arr);

dyv* mk_track_DEC_residuals(track* X, simple_obs_array* arr);

/* Returns the cumulative residuals of a given dimension in a given form. */
/* dim: 1=ra, 2=dec, 3=T, 4=I */
/* res: 1=mean, 2=sum, 3=max  */
double track_given_residual(track* X, simple_obs_array* arr, int dim,
                            int res, bool sq);

double mean_track_residual(track* X, simple_obs_array* arr);

double mean_track_residual_angle(track* X, simple_obs_array* arr);

double max_track_residual_angle(track* X, simple_obs_array* arr);

double mean_sq_track_residual(track* X, simple_obs_array* arr);

double max_sq_track_residual(track* X, simple_obs_array* arr);

/* Compute the mean residual of track B in the eyes of track A */
double mean_sq_second_track_residual(track* A, track* B, simple_obs_array* arr);

double single_track_residual(track* X, simple_obs* actu);


/* --- Output functions ------------------------------------------------------- */

void printf_track(track* S);

void fprintf_track(FILE* f, char* prefix, track* S, char* suffix);

void fprintf_track_list(FILE* f, char* prefix, track* S, char* suffix);

void fprintf_MPC_track(FILE* f, track* S, simple_obs_array* all_obs);

/* Display the track as a white space separated list of observation IDs. */
void fprintf_track_ID_list(FILE* f, char* pre, track* S, char* post,
                           simple_obs_array* all_obs);

char* mk_string_from_track(track* S);


/* --- Memory functions for the joint observation arrays ------------------- */

track_array* mk_empty_track_array(int size);

track_array* mk_copy_track_array(track_array* old);

/* Copy the track array, but remove any NULL tracks. */
track_array* mk_filter_copy_track_array(track_array* old);

track_array* mk_track_array_subset(track_array* old, ivec* inds);

track_array* mk_merged_track_array(track_array* A, track_array* B);

track_array* mk_track_array_from_simple_obs_array(simple_obs_array* old);

track_array* mk_track_array_from_simple_obs_array_subset(simple_obs_array* old, ivec* ids);

/* Make a simple obs array of the starting positions of the tracks. */
/* If inds == NULL then uses all tracks.                            */
simple_obs_array* mk_simple_obs_array_from_track_array(track_array* old, ivec* inds);

track_array* mk_track_array_from_matched_simple_obs(simple_obs_array* obs, ivec_array* matches,
                                                    int min_obs);

track_array* mk_true_tracks(simple_obs_array* obs, ivec* true_groups);

/* Makes a track array where all tracklets are part of true tracks */
/* of a given size = number of observations (used for testing only) */
track_array* mk_track_array_filter_on_true_track_size(track_array* old, ivec* true_groups,
                                                      int size, bool ORLESS);

/* Makes a track array where all tracklets are part of true tracks */
/* of a given size = number of observations (used for testing only) */
ivec* mk_groups_filter_on_true_track_size(ivec* true_groups, int size, bool ORLESS);

void track_array_double_size(track_array* old);

void free_track_array(track_array* old);

track* safe_track_array_ref(track_array* X, int index);

/* Return the current size of the array (the index of the
   last element +1) */
int safe_track_array_size(track_array* X);

/* Return the maximum size of the array  */
int safe_track_array_max_size(track_array* X);


/* Allow a few speedups */
#ifdef AMFAST

#define track_array_size(X)     (X->size)
#define track_array_max_size(X) (X->max_size)
#define track_array_ref(X,i)    (X->the_obs[i])

#else

#define track_array_size(X)     (safe_track_array_size(X))
#define track_array_max_size(X) (safe_track_array_max_size(X))
#define track_array_ref(X,i)    (safe_track_array_ref(X,i))

#endif

int track_array_longest_track_length(track_array* X);

void track_array_set(track_array* X, int index, track* newobs);

void track_array_remove(track_array* X, int index);

void track_array_add(track_array* X, track* A);

void track_array_add_all(track_array* X, track_array* toadd);

/* Find all occurrences of observation "obs_num" in a track. */
ivec* mk_find_obs_in_tracks(track_array* X, int obs_num);


/* If the true groups are not needed then it can be set to NULL */
void track_array_add_random(track_array* arr, simple_obs_array* obs,
                            int N, ivec* true_groups);

/*
  obs    - the observations
  times  - a vector of the times you want indexed
*/
ivec_array* mk_time_to_track_ind(track_array* obs, dyv* times);

ivec_array* mk_end_times_to_track_ind(track_array* obs, simple_obs_array* arr, dyv* end_times);

void track_array_force_t0(track_array* X, double t0);

void fprintf_track_array(FILE* f, track_array* X);

void fprintf_track_array_list(FILE* f, track_array* X);

/* Print the track array as:               */
/* [a_RA, v_RA, x_RA, a_DEC, v_DEC, x_DEC] */
void fprintf_track_array_as_RD_dym(FILE* f, track_array* X);

/* Displays the track list as a series of MPS strings
   each string is modified so that the label
   corresponds to the track (i.e. observations may
   appear multiple times with different labels) */
void fprintf_track_array_mod_MPC(FILE* f, track_array* X, simple_obs_array* obs);

void dump_trackids_to_file(FILE* fp, simple_obs_array* obs, track_array* trcks);

void dump_tracks_to_file(FILE* f_obs, FILE* f_summary, char* score_file,
                         bool indiv_files, simple_obs_array* obs, 
                         track_array* trcks);

void dump_tracks_to_MPC_file(char* file_name, simple_obs_array* obs, track_array* trcks);

/* Display the track array as a white space separated list of observation IDs. */
void fprintf_track_array_ID_list(FILE* f, track_array* X, simple_obs_array* all_obs);

dyv* mk_sort_all_track_times(track_array* X);

dyv* mk_sort_all_track_end_times(track_array* X, simple_obs_array* obs);

void track_array_flatten_to_plates(track_array* arr, simple_obs_array* obs,
                                   double plate_width);

void track_array_unflatten_to_plates(track_array* arr, simple_obs_array* obs,
                                     dyv* org_times);


/* --- Track evaluation functions --------------------------- */

/* Reduce the list of true groups to only with 'min_obs' */
/* or more observations. Also renormalizes the group     */
/* numbers so max(result) == # of tracks with min_obs-1  */
ivec* mk_tracks_of_size_from_list(ivec* true_tracks, int min_obs);

/* Reduce the list of true groups to only with 'min_obs' */
/* or more observations. Also renormalizes the group     */
/* numbers so max(result) == # of tracks with min_obs-1  */
/* Also filter on the number of nights covered.          */
ivec* mk_tracks_of_length_from_list(simple_obs_array* obs, ivec* true_tracks,
                                    int min_obs, int min_nights);

/* Count the percentage of the associations that are correct */
double track_array_percent_correct(track_array* mtch, ivec* true_tracks, int min_obs);

/* Count the percentage of the adjacent matches that are correct */
double track_array_adj_percent_correct(track_array* mtch, ivec* true_tracks);

double track_array_percent_found(track_array* mtchs, ivec* true_tracks, int min_obs);

/* Returns the percentage of the true adjacent matches that were actually found. */
double track_array_adj_percent_found(track_array* mtchs, ivec* true_tracks);

/* Makes a 2 column dym.  The first column is the number */
/* of TRUE positives and the second column is the number */
/* of FALSE positives.  Tracks are assumed to be in      */
/* sorted order.                                         */
/* The last row contains: [total_true_assocations, 0]    */
dym* mk_track_array_ROC_curve(track_array* mtchs, ivec* true_tracks, int min_obs);

/* Compares track arrays A and B and returns true iff they are the  */
/* same (up to a change in order).  Also "returns" number of points */
/* in A but not B and the number of points in B but not A.          */
bool track_array_compare_slow(track_array* A, track_array* B, int* aOnly, int* bOnly);

/* Takes a sorted track array and returns an ivec such that    */
/* each entry corresponds with a track and is 1 iff >= percent */
/* of the track's observations have matching labels.           */
ivec* mk_track_array_roc_vec(simple_obs_array* obs, track_array* res, ivec* true_groups,
                             int min_obs, int min_sup, double percent);

double roc_percent_found(ivec* roc, ivec* true_groups);

double roc_percent_correct(ivec* roc);

double roc_aac(ivec* roc, int stop);

/* Count the found tracks that are exact matches of the true_groups. */
int compute_exact_matches(track_array* res, ivec* true_groups,
                          double* percent_correct, double* percent_found);

/* Returns an imat such that each row corresponds to a track */
/* length and the columns indicate how many of the entries   */
/* correspond to the SAME object.                            */
/* Mij says that there are Mij tracks of length i with j     */
/* matching entries.                                         */
imat* mk_track_array_consistency_mat(track_array* res, ivec* true_groups);

/* Returns an imat such that each row corresponds to a true  */
/* track length: Mij says that for all tracks of length i    */
/* our results contained Mij entries that had j entries that */
/* matched the track.  Only the best match is tracked.       */
imat* mk_track_array_inv_consistency_mat(track_array* res, ivec* true_groups);


/* Returns a score dym.  The columns indicate the size of the */
/* true tracks and the rows indicate the rows of the best     */
/* matching track that was found.  M_ij indicates the         */
/* percentage of true tracks of length j whose best match was */
/* of length i.  If overlap = true then this becomes whose    */
/* best match had overlap of length i.                        */
dym* mk_track_array_RJ_score_mat(track_array* res, ivec* true_groups,
                                 bool overlap);

#endif
