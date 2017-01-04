/*
File:        track.c
Author:      J. Kubica
Created:     Fri Sept 19 10:35:50 EDT 2003
Description: Functions for the "track" data structure.  Tracks are sets of
             astronomical observations and corresponding motion parameters
             in (RA, dec) coordinates.

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

#include "track.h"

#define c_to_i(X)     (X - '0')
#define cc_to_i(X,Y)  (c_to_i(X)*10 + c_to_i(Y))


/* Define a fractional number of days whose span is considered to
be the same night. */
#define SAME_NIGHT_THRESHOLD 0.6


/* -----------------------------------------------------------------------*/
/* -------- Tracks -------------------------------------------------------*/
/* -----------------------------------------------------------------------*/

/* --- Memory functions for the joint observations ---------------------------- */

track* mk_track_single_ind(simple_obs_array* all_obs, int ind) {
  ivec*  inds;
  track* X;

  inds = mk_ivec_1(ind);
  X    = mk_track_from_N_inds(all_obs,inds);
  free_ivec(inds);

  return X;
}


track* mk_track_pair_ind(simple_obs_array* all_obs, int A, int B) {
  ivec*  inds;
  track* X;

  inds = mk_ivec_2(A,B);
  X    = mk_track_from_N_inds(all_obs,inds);
  free_ivec(inds);

  return X;
}


track* mk_track_from_N_inds(simple_obs_array* all_obs, ivec* inds) {
  simple_obs* A;
  track* X = AM_MALLOC(track);
  dyv* temp_RA;
  dyv* temp_DEC;
  dyv* times;
  dyv* RA;
  dyv* DEC;
  dyv* w;
  ivec* t_inds;
  ivec* s_inds;
  double bsum;
  double t0,   tt,   dt;
  double RA0,  RAt,  dRA;
  double DEC0, DECt, dDEC;
  int N = ivec_size(inds);
  int i;

  /* Allocate space for the following operations... */
  times  = mk_dyv(N);
  RA     = mk_dyv(N);
  DEC    = mk_dyv(N);
  w      = mk_dyv(N);
  s_inds = mk_ivec(N);

  /* For the indices by time... */
  for(i=0;i<N;i++) {
    dyv_set(times,i,simple_obs_time(simple_obs_array_ref(all_obs,ivec_ref(inds,i))));
  }
  t_inds = mk_indices_of_sorted_dyv(times);  
  for(i=0;i<N;i++) {
    ivec_set(s_inds,i,ivec_ref(inds,ivec_ref(t_inds,i)));
  }

  /* Set the indices, find the first time,
     and the average brightness */
  A    = simple_obs_array_ref(all_obs,ivec_ref(s_inds,0));
  RA0  = simple_obs_RA(A);
  DEC0 = simple_obs_DEC(A);
  t0   = simple_obs_time(A);
  bsum = 0.0;
  for(i=0;i<N;i++) {
    A     = simple_obs_array_ref(all_obs,ivec_ref(s_inds,i));
    RAt   = simple_obs_RA(A);
    DECt  = simple_obs_DEC(A);
    tt    = simple_obs_time(A);
    bsum += simple_obs_brightness(A);
    
    dt   = tt - t0;
    dRA  = RAt - RA0;
    dDEC = DECt - DEC0;
    if(dRA < -12.0) { dRA += 24.0; }
    if(dRA >  12.0) { dRA -= 24.0; }

    dyv_set(RA,i,dRA);
    dyv_set(DEC,i,dDEC);
    dyv_set(w,i,cos(DECt * DEG_TO_RAD));
    dyv_set(times,i,dt);
  }

  X->simp_ind   = s_inds;
  X->time       = t0;
  X->brightness = (float)(bsum / (double)N);

  temp_RA  = mk_fill_obs_moments(RA, times, w, 3);
  temp_DEC = mk_fill_obs_moments(DEC, times, NULL, 3);
  dyv_set(temp_RA,0,dyv_ref(temp_RA,0)+RA0);
  dyv_set(temp_DEC,0,dyv_ref(temp_DEC,0)+DEC0);
  for(i=0;i<3;i++) {
    X->RA_m[i]  = dyv_ref(temp_RA,i);
    X->DEC_m[i] = dyv_ref(temp_DEC,i);
  }
  free_dyv(temp_RA);
  free_dyv(temp_DEC);
  
  free_ivec(t_inds);
  free_dyv(times);
  free_dyv(RA);
  free_dyv(w);
  free_dyv(DEC);

  return X;
}



track* mk_combined_track(track* A, track* B, simple_obs_array* all_obs) {
  track* C;
  ivec* Av;
  ivec* Cv;
  ivec* Bv;

  Av = track_individs(A);
  Bv = track_individs(B);
  Cv = mk_ivec_union(Av,Bv);
  C  = mk_track_from_N_inds(all_obs, Cv);

  free_ivec(Cv);

  return C;  
}


track* mk_track_add_one(track* old, simple_obs* nu, simple_obs_array* all_obs) {
  track* res;
  ivec* inds;
  ivec* ind;
  
  ind  = mk_ivec_1(simple_obs_id(nu));
  inds = mk_ivec_append(track_individs(old),ind);
  res  = mk_track_from_N_inds(all_obs, inds);

  free_ivec(ind);
  free_ivec(inds);

  return res;
}


track* mk_track_add_one_ind(track* old, int ind, simple_obs_array* all_obs) {
  track* res;
  ivec* inds;
  ivec* ind1;
  
  ind1 = mk_ivec_1(ind);
  inds = mk_ivec_append(track_individs(old),ind1);
  res  = mk_track_from_N_inds(all_obs, inds);

  free_ivec(ind1);
  free_ivec(inds);

  return res;
}


track* mk_copy_track(track* old) {
  track* X = AM_MALLOC(track);
  int i;

  X->simp_ind   = mk_copy_ivec(old->simp_ind);
  X->time       = old->time;
  X->brightness = old->brightness;

  for(i=0;i<3;i++) {
    X->RA_m[i]  = old->RA_m[i];
    X->DEC_m[i] = old->DEC_m[i];
  }

  return X;
}


void free_track(track* old) {
  free_ivec(old->simp_ind);
  AM_FREE(old,track);
}


/* --- Observation Access Functions ------------------------------------ */

int safe_track_num_obs(track* X) {
  return ivec_size(X->simp_ind);
}


date* mk_track_date(track* X) {
  return mk_date_from_daycode((int)X->time);
}

/* returns a global time (accounting for year/month/day/time) */
double safe_track_time(track* X) {
  return X->time;
}


double safe_track_RA(track* X) {
  return X->RA_m[0];
}


double track_indiv_RA(track* X, simple_obs_array* all_obs, int obs_num) {
  simple_obs* T = simple_obs_array_ref(all_obs,ivec_ref(X->simp_ind,obs_num));

  my_assert(T != NULL);
  return simple_obs_RA(T);
}


double safe_track_vRA(track* X) {
  return X->RA_m[1];
}


double safe_track_aRA(track* X) {
  return X->RA_m[2];
}


double safe_track_DEC(track* X) {
  return X->DEC_m[0];
}


double track_indiv_DEC(track* X, simple_obs_array* all_obs, int obs_num) {
  simple_obs* T = simple_obs_array_ref(all_obs,ivec_ref(X->simp_ind,obs_num));

  my_assert(T != NULL);
  return simple_obs_DEC(T);
}


double safe_track_vDEC(track* X) {
  return X->DEC_m[1];
}


double safe_track_aDEC(track* X) {
  return X->DEC_m[2];
}


double safe_track_brightness(track* X) {
  return (double)X->brightness;
}


void track_set_RA(track* X, double nuVal) {
  X->RA_m[0] = nuVal;
}

void track_set_vRA(track* X, double nuVal) {
  X->RA_m[1] = nuVal;
}

void track_set_aRA(track* X, double nuVal) {
  X->RA_m[2] = nuVal;
}

void track_set_DEC(track* X, double nuVal) {
  X->DEC_m[0] = nuVal;
}

void track_set_vDEC(track* X, double nuVal) {
  X->DEC_m[1] = nuVal;
}

void track_set_aDEC(track* X, double nuVal) {
  X->DEC_m[2] = nuVal;
}


simple_obs* safe_track_indiv(track* X, int obs_num, simple_obs_array* all_obs) {
  return simple_obs_array_ref(all_obs,ivec_ref(X->simp_ind,obs_num));
}


simple_obs* safe_track_first(track* X, simple_obs_array* all_obs) {
  return simple_obs_array_ref(all_obs,ivec_ref(X->simp_ind,0));
}


simple_obs* safe_track_last(track* X, simple_obs_array* all_obs) {
  return simple_obs_array_ref(all_obs,ivec_ref(X->simp_ind,ivec_size(X->simp_ind)-1));
}


ivec* safe_track_individs(track* X) {
  return X->simp_ind;
}


double safe_track_first_time(track* X, simple_obs_array* obs) {
  return simple_obs_time(track_first(X,obs));
}


double safe_track_last_time(track* X, simple_obs_array* obs) {
  return simple_obs_time(track_last(X,obs));
}


double safe_track_time_length(track* X, simple_obs_array* obs) {
  return (simple_obs_time(track_last(X,obs)) - simple_obs_time(track_first(X,obs)));
}


/* Track time length rounded up */
double track_num_nights(track* X, simple_obs_array* obs) {
  double L = track_time_length(X,obs);
  double N;

  N = (double)((int)L);
  if(L-N > 1e-20) {
    N += 1.0;
  }

  return N;
}


/* On how many different nights did we see observations. */
/* Effectively counts the number of tracklets making     */
/* up the track.                                         */
int track_num_nights_seen(track* X, simple_obs_array* obs) {
  simple_obs* A;
  ivec* inds = track_individs(X);
  double tlast;
  int count = 0;
  int i, N;

  A = simple_obs_array_ref(obs,ivec_ref(inds,0));
  N = ivec_size(inds);
  tlast = simple_obs_time(A);
  count++;

  for(i=1;i<N;i++) {
    A = simple_obs_array_ref(obs,ivec_ref(inds,i));
    if(simple_obs_time(A)-tlast > SAME_NIGHT_THRESHOLD) {
      tlast = simple_obs_time(A);
      count++;
    }
  }

  return count;
}


/* Return the observation that occurred at t or -1
   if no such observation */
int track_at_t(track* X, double t, simple_obs_array* all_obs) {
  double diff = 1.0;
  int res = -1;
  int i = 0;

  while((i<ivec_size(X->simp_ind))&&(diff > 0.001)) {
    diff = t - simple_obs_time(simple_obs_array_ref(all_obs,ivec_ref(X->simp_ind,i)));
    i++;
  }
  i--;

  if((i<ivec_size(X->simp_ind))&&(fabs(diff) < 0.001)) { 
    res = ivec_ref(X->simp_ind,i);
  }

  return res;
}


bool track_equal(track* A, track* B) {
  ivec* Aind = track_individs(A);
  ivec* Bind = track_individs(B);
  bool eq    = TRUE;
  int i      = 0;

  eq = (ivec_size(Aind) == ivec_size(Bind));
  while(eq && (i<ivec_size(Aind))) {
    eq = eq && (ivec_ref(Aind,i) == ivec_ref(Bind,i));
    i++;
  }

  return eq;
}


/* Are all of the track's time in order and unique */
bool track_times_valid(track* A, simple_obs_array* obs) {
  ivec* Aind = track_individs(A);
  bool valid = TRUE;
  double t0, t1;
  int  i;

  if(ivec_size(Aind) >= 1) {
    t0 = simple_obs_time(simple_obs_array_ref(obs,ivec_ref(Aind,0)));  

    for(i=1;(i<ivec_size(Aind)) && valid;i++) {
      t1 = simple_obs_time(simple_obs_array_ref(obs,ivec_ref(Aind,i)));

      valid = (t0 < t1 - 1e-8);
      t0 = t1;
    }
  }

  return valid;
}


/* Do A and B overlap at some point AND not have
   two DIFFERENT observations at any point */
bool track_valid_overlap(track* A, track* B, simple_obs_array* arr) {
  ivec* Aind = track_individs(A);
  ivec* Bind = track_individs(B);
  double aTime, bTime;
  int a   = 0;
  int b   = 0;
  bool eq = TRUE;
  bool overlap = FALSE;

  while(eq && (a < ivec_size(Aind)) && (b < ivec_size(Bind))) {
    aTime = simple_obs_time(simple_obs_array_ref(arr,ivec_ref(Aind,a)));
    bTime = simple_obs_time(simple_obs_array_ref(arr,ivec_ref(Bind,b)));

    if(fabs(aTime - bTime) <  1e-5) {
      overlap = TRUE;
      eq = (ivec_ref(Aind,a) == ivec_ref(Bind,b));
      a++;
      b++;
    } else {
      if(aTime > bTime) { b++; }
      if(aTime < bTime) { a++; }
    }
  }
  
  return (eq && overlap);
}


/* Is B a subset of A? */
bool track_subset(track* A, track* B, simple_obs_array* obs) {
  int overlap;
  bool res;

  overlap = track_overlap_size(A,B,obs);
  res     = (overlap == ivec_size(track_individs(B)));  

  return res;
}


/* Is B a proper subset of A? */
bool track_proper_subset(track* A, track* B, simple_obs_array* obs) {
  ivec* Aind = track_individs(A);
  ivec* Bind = track_individs(B);
  int overlap = track_overlap_size(A,B,obs);
  bool res;

  res = (overlap == ivec_size(Bind));  
  res = res && (overlap < ivec_size(Aind));

  return res;
}


/* Count the number of points at which the two tracks overlap... */
int track_overlap_size(track* A, track* B, simple_obs_array* obs) {
  ivec* Aind = track_individs(A);
  ivec* Bind = track_individs(B);
  double aTime, bTime;
  int count = 0;
  int a     = 0;
  int b     = 0;

  while((a < ivec_size(Aind)) && (b < ivec_size(Bind))) {
    aTime = simple_obs_time(simple_obs_array_ref(obs,ivec_ref(Aind,a)));
    bTime = simple_obs_time(simple_obs_array_ref(obs,ivec_ref(Bind,b)));

    if(fabs(aTime - bTime) <  1e-5) {
      if(ivec_ref(Aind,a) == ivec_ref(Bind,b)) { count++; }
      a++;
      b++;
    } else {
      if(aTime > bTime) { b++; }
      if(aTime < bTime) { a++; }
    }
  }

  return count;
}


int track_overlap_size_slow(track* A, track* B, simple_obs_array* obs) {
  ivec* Aind = track_individs(A);
  ivec* Bind = track_individs(B);
  ivec* intersect = mk_ivec_intersection(Aind,Bind);
  int res;

  res = ivec_size(intersect);

  free_ivec(intersect);

  return res;
}


ivec* mk_track_overlap(track* A, track* B) {
  ivec* Aind = track_individs(A);
  ivec* Bind = track_individs(B);
  ivec* intersect = mk_ivec_intersection(Aind,Bind);
  
  return intersect;
}


bool track_overlap(track* A, track* B, simple_obs_array* obs) {
  return (track_overlap_size(A,B,obs) > 0);
}


bool track_overlap_in_time(track* A, track* B, simple_obs_array* arr) {
  ivec* Aind = track_individs(A);
  ivec* Bind = track_individs(B);
  double aTime, bTime;
  int a   = 0;
  int b   = 0;
  bool overlap = FALSE;

  while((overlap==FALSE) && (a < ivec_size(Aind)) && (b < ivec_size(Bind))) {
    aTime = simple_obs_time(simple_obs_array_ref(arr,ivec_ref(Aind,a)));
    bTime = simple_obs_time(simple_obs_array_ref(arr,ivec_ref(Bind,b)));

    if(fabs(aTime - bTime) <  1e-5) {
      overlap = TRUE;
      a++;
      b++;
    } else {
      if(aTime > bTime) { b++; }
      if(aTime < bTime) { a++; }
    }
  }

  return overlap;
}


/* Do A and B have two DIFFERENT observations at any time */
bool track_collide_in_time(track* A, track* B, simple_obs_array* arr) {
  ivec* Aind = track_individs(A);
  ivec* Bind = track_individs(B);
  double aTime, bTime;
  int a   = 0;
  int b   = 0;
  bool overlap = FALSE;

  while((overlap==FALSE) && (a < ivec_size(Aind)) && (b < ivec_size(Bind))) {
    aTime = simple_obs_time(simple_obs_array_ref(arr,ivec_ref(Aind,a)));
    bTime = simple_obs_time(simple_obs_array_ref(arr,ivec_ref(Bind,b)));

    if(fabs(aTime - bTime) <  1e-10) {
      overlap = (ivec_ref(Aind,a) != ivec_ref(Bind,b));
      a++;
      b++;
    } else {
      if(aTime > bTime) { b++; }
      if(aTime < bTime) { a++; }
    }
  }

  return overlap;
}


/* ---------------------------------------------------------------------- */
/* --- Track Distance Functions ----------------------------------------- */
/* ---------------------------------------------------------------------- */

/* Just compute the distance with RA and DEC */
double track_euclidean_distance(track* A, track* B) {
  double raA  = track_RA(A);
  double raB  = track_RA(B);
  double decA = track_DEC(A);
  double decB = track_DEC(B);

  return simple_obs_euclidean_dist_given(raA, raB, decA, decB);
}


double track_midpt_distance(track* A, track* B) {
  return track_gen_midpt_distance(A,B,0.5);
}


/* Generalized midpoint distance.  Calculates the midpoint distance */
/* using time = A.time + beta * (B.time - A.time).                  */
/* Example: A linear, B linear -> use beta = 0.5                    */
/*          A quad,   B linear -> use beta = 1.0                    */
double track_gen_midpt_distance(track* A, track* B, double beta) {
  double ta, tb, tmid;
  double ar, ad, br, bd;

  /* Calculate the midpoint time. */
  ta   = track_time(A);
  tb   = track_time(B);
  tmid = ta + (tb-ta)*beta;

  /* Compute the predicted positions. */
  track_RA_DEC_prediction(A,tmid-ta,&ar,&ad);
  track_RA_DEC_prediction(B,tmid-tb,&br,&bd);

  /* Return the midpoint distance */
  return simple_obs_euclidean_dist_given(ar,br,ad,bd);
}


double track_obs_euclidean_distance(track* A, simple_obs* B) {
  double ra, dec, dt;

  dt = simple_obs_time(B) - track_time(A);
  track_RA_DEC_prediction(A,dt,&ra,&dec);

  return simple_obs_euclidean_dist_given(ra,  simple_obs_RA(B), 
                                         dec, simple_obs_DEC(B));
}


/* ---------------------------------------------------------------------- */
/* --- Track Prediction Functions --------------------------------------- */
/* ---------------------------------------------------------------------- */

/* Forcibly set t0: Move the RA, vRA, DEC, vDEC, etc. */
void track_force_t0(track* X, double t0) {
  double ra, dec, vra, vdec;
  double dt = t0 - track_time(X);

  track_RDVV_prediction(X,dt,&ra,&dec,&vra,&vdec);

  X->RA_m[0]  =   ra;
  X->RA_m[1]  =  vra;
  X->DEC_m[0] =  dec;
  X->DEC_m[1] = vdec;

  X->time = t0;
}


void track_force_t0_first(track* X, simple_obs_array* all_obs) {
  int ind = ivec_ref(X->simp_ind,0);
  double t_first = simple_obs_time(simple_obs_array_ref(all_obs,ind));

  track_force_t0(X,t_first);
}


void track_force_t0_last(track* X, simple_obs_array* all_obs) {
  int ind = ivec_ref(X->simp_ind,ivec_size(X->simp_ind)-1);
  double t_last = simple_obs_time(simple_obs_array_ref(all_obs,ind));

  track_force_t0(X,t_last);
}


/* Makes a simple linear prediction for RA and DEC only */
/* and stores them in the two given doubles.            */
void track_RA_DEC_prediction_full(track* X, double ellapsed_time, double* pred_RA, 
                                  double* pred_DEC, bool wraparound) {
  double RA, DEC;
  double remainder, change_r, change_d;

  /* Change the DEC... for both the paired and
     simple observations */
  change_r = ellapsed_time * track_vRA(X)
             + 0.5 * ellapsed_time * ellapsed_time * track_aRA(X);
  change_d = ellapsed_time * track_vDEC(X) 
             + 0.5 * ellapsed_time * ellapsed_time * track_aDEC(X);
  DEC = track_DEC(X) + change_d;
  RA  = track_RA(X)  + change_r;
  if((DEC < -90.0)&&(wraparound==TRUE)) {
    remainder = (90.0 + DEC);
    DEC = -90.0 - (change_d + remainder);
    RA += 12.0;
  }
  if((DEC > 90.0)&&(wraparound==TRUE)) {
    remainder = (90.0 - DEC);
    DEC = 90.0 - (change_d - remainder);
    RA += 12.0;
  }

  /* Change the RA... for both the paired and
     simple observations */
  if((RA > 24.0)&&(wraparound==TRUE)) { RA -= 24.0; }
  if((RA <  0.0)&&(wraparound==TRUE)) { RA += 24.0; }

  pred_RA[0]  = RA;  
  pred_DEC[0] = DEC;
}


void track_RA_DEC_prediction(track* X, double ellapsed_time,
                             double* pred_RA, double* pred_DEC) {
  track_RA_DEC_prediction_full(X, ellapsed_time, pred_RA, pred_DEC, TRUE);
}


/* Returns an estimated angular velocity of a linear track. */
/* In radians per day.                                      */
double linear_track_estimated_angular_vel(track* X) {
  double r, d;
  double dist;
  
  track_RA_DEC_prediction(X,1.0,&r,&d);
  dist = angular_distance_RADEC(track_RA(X),r,track_DEC(X),d);

  return dist;
}


/* Makes a simple linear prediction for (RA, DEC, vRA, vDEC) and 
   stores them in the two given doubles. */
void track_RDVV_prediction(track* X, double ellapsed_time,
                           double* pred_RA, double* pred_DEC,
                           double* pred_vRA, double* pred_vDEC) {
  track_RA_DEC_prediction(X,ellapsed_time,pred_RA,pred_DEC);
  pred_vRA[0]  = track_vRA(X) + ellapsed_time * track_aRA(X);
  pred_vDEC[0] = track_vDEC(X) + ellapsed_time * track_aDEC(X);
}


/* Creates a predicted point from the current point
   and an ellapsed time.  Note the MPC strings for the
   simple observations WILL BE INCORRECT! */
simple_obs* mk_simple_obs_track_prediction(track* X, double ellapsed_time) {
  double ra, dec;
  simple_obs* res;

  track_RA_DEC_prediction(X,ellapsed_time,&ra,&dec);
  res = mk_simple_obs_simplest(-1, track_time(X)+ellapsed_time, ra, dec,
                               track_brightness(X));

  return res;  
}


/* Modifies the observations in the track so the fit on the plates */
void track_flatten_to_plates(track* A, simple_obs_array* arr, dyv* plates,
                            double plate_width) {
  simple_obs* X;
  ivec* inds;
  double time;
  double RA = 0.0;
  double DEC = 0.0;
  int i, ind;

  inds = track_individs(A);
  for(i=0;i<ivec_size(inds);i++) {
    X = simple_obs_array_ref(arr,ivec_ref(inds,i));

    time = simple_obs_time(X);
    ind  = find_index_in_dyv(plates,time,plate_width);
 
    if(ind == -1) {
      printf("ERROR: In Pull to Plates\n");
    } else {
      if(dyv_ref(plates,ind) > time) {
        ind--;
      }

      /* Make sure you actually need to move it to a plate... */
      if(fabs(dyv_ref(plates,ind)-simple_obs_time(X)) > 1e-10) {
        track_RA_DEC_prediction(A, dyv_ref(plates,ind)-track_time(A),
                                              &RA, &DEC);

        simple_obs_set_RA(X,RA);
        simple_obs_set_DEC(X,DEC);
        simple_obs_set_time(X,dyv_ref(plates,ind));
      }
    }
  }

  /* Finally adjust the front of the track to
     the new first observation time */
  track_force_t0_first(A, arr);
}


/* Modifies the observations in the track so they are at their original times */ 
void track_unflatten_to_plates(track* A, simple_obs_array* arr, dyv* org_times) {
  simple_obs* X;
  ivec* inds;
  double time;
  double RA = 0.0;
  double DEC = 0.0;
  double otime;
  int i;

  inds = track_individs(A);
  for(i=0;i<ivec_size(inds);i++) {
    X     = simple_obs_array_ref(arr,ivec_ref(inds,i));
    otime = dyv_ref(org_times,ivec_ref(inds,i)); 
    time  = simple_obs_time(X);

    /* Make sure you actually need to move it to a plate... */
    if(fabs(otime-simple_obs_time(X)) > 1e-10) {
      track_RA_DEC_prediction(A,otime-track_time(A),&RA, &DEC);
      simple_obs_set_RA(X,RA);
      simple_obs_set_DEC(X,DEC);
      simple_obs_set_time(X,otime);
    }
  }

  /* Finally adjust the front of the track to
     the new first observation time */
  track_force_t0_first(A, arr);
}


/* ---------------------------------------------------------------------- */
/* --- Track Residual Functions ----------------------------------------- */
/* ---------------------------------------------------------------------- */

/* Returns the residuals from the track fit. */
dyv* mk_track_residuals(track* X, simple_obs_array* arr) {
  dyv* res;
  simple_obs* actu;
  double dist, tdif;
  double RA  = 0.0;
  double DEC = 0.0;
  int N = track_num_obs(X);
  int i;
  
  res = mk_dyv(N);
  for(i=0;i<N;i++) {
    actu = track_indiv(X,i,arr);
    tdif = simple_obs_time(actu) - track_time(X);
    track_RA_DEC_prediction(X, tdif, &RA, &DEC);    
    dist = simple_obs_euclidean_dist_given(RA, simple_obs_RA(actu),
                                           DEC,simple_obs_DEC(actu)); 
    dyv_set(res,i,dist);
  }

  return res;
}


dyv* mk_track_RA_residuals(track* X, simple_obs_array* arr) {
  dyv* res;
  simple_obs* actu;
  double tdif;
  double RA  = 0.0;
  double DEC = 0.0;
  double diff;
  int N = track_num_obs(X);
  int i;
  
  res = mk_dyv(N);
  for(i=0;i<N;i++) {
    actu = track_indiv(X,i,arr);
    tdif = simple_obs_time(actu) - track_time(X);
    track_RA_DEC_prediction(X, tdif, &RA, &DEC);    

    diff = simple_obs_RA(actu)-RA;
    if(fabs(diff) > 12.0) {
      if(diff < -12.0) { diff += 24.0; }
      if(diff >  12.0) { diff -= 24.0; }
    }

    dyv_set(res,i,diff);
  }

  return res;
}


dyv* mk_track_DEC_residuals(track* X, simple_obs_array* arr) {
  dyv* res;
  simple_obs* actu;
  double tdif;
  double RA  = 0.0;
  double DEC = 0.0;
  int N = track_num_obs(X);
  int i;
  
  res = mk_dyv(N);
  for(i=0;i<N;i++) {
    actu = track_indiv(X,i,arr);
    tdif = simple_obs_time(actu) - track_time(X);
    track_RA_DEC_prediction(X, tdif, &RA, &DEC);    
    dyv_set(res,i,simple_obs_DEC(actu)-DEC);
  }

  return res;
}


/* Returns the cumulative residuals of a given dimension in a given form. */
/* dim: 1=ra, 2=dec           */
/* res: 1=mean, 2=sum, 3=max  */
double track_given_residual(track* X, simple_obs_array* arr, int dim, 
                            int res, bool sq) {
  simple_obs* actu;
  double tdif, diff;
  double tval, oval, temp;
  double sum = 0.0;
  double max = 0.0;
  double result;
  int N = track_num_obs(X);
  int i;

  for(i=0;i<N;i++) {
    actu = track_indiv(X,i,arr);
    tdif = simple_obs_time(actu) - track_time(X);

    oval = 0.0;
    tval = 0.0;

    switch(dim) {
    case 1:
      track_RA_DEC_prediction(X,tdif,&tval,&temp);
      oval = simple_obs_RA(actu);
      break;
    case 2:
      track_RA_DEC_prediction(X,tdif,&temp,&tval);
      oval = simple_obs_DEC(actu);
      break;
    }

    /* Compute the actual distance... */
    diff = fabs(tval - oval);
    if(dim==1) {
      while(diff > 12.0) { diff = fabs(diff-24.0); }
    }

    /* Handle squaring if required... */
    if(sq) { diff = diff * diff; }

    /* Update the sum and the max */
    sum += diff;
    if((i==0)||(diff > max)) { max = diff; }
  }

  result = sum;
  if(res==1) { result = sum/((double)N); }
  if(res==3) { result = max; }

  return result;
}


double mean_track_residual(track* X,simple_obs_array* arr) {
  simple_obs* actu;
  double mean = 0.0;
  double dist, tdif;
  double RA  = 0.0;
  double DEC = 0.0;
  int N = track_num_obs(X);
  int i;

  for(i=0;i<N;i++) {
    actu = track_indiv(X,i,arr);
    tdif = simple_obs_time(actu) - track_time(X);
    track_RA_DEC_prediction(X, tdif, &RA, &DEC);
    dist = simple_obs_euclidean_dist_given(RA, simple_obs_RA(actu),
                                           DEC,simple_obs_DEC(actu));
    mean += (dist / (double)N);
  }

  return mean;
}


double mean_track_residual_angle(track* X, simple_obs_array* arr) {
  simple_obs* actu;
  double mean = 0.0;
  double dist, tdif;
  double RA  = 0.0;
  double DEC = 0.0;
  int N = track_num_obs(X);
  int i;

  for(i=0;i<N;i++) {
    actu = track_indiv(X,i,arr);
    tdif = simple_obs_time(actu) - track_time(X);
    track_RA_DEC_prediction(X, tdif, &RA, &DEC);
    dist = angular_distance_RADEC(RA, simple_obs_RA(actu),
                                  DEC,simple_obs_DEC(actu));
    mean += (dist / (double)N);
  }

  return mean;
}


double max_track_residual_angle(track* X, simple_obs_array* arr) {
  simple_obs* actu;
  double max = 0.0;
  double dist, tdif;
  double RA  = 0.0;
  double DEC = 0.0;
  int N = track_num_obs(X);
  int i;

  for(i=0;i<N;i++) {
    actu = track_indiv(X,i,arr);
    tdif = simple_obs_time(actu) - track_time(X);
    track_RA_DEC_prediction(X, tdif, &RA, &DEC);
    dist = angular_distance_RADEC(RA, simple_obs_RA(actu),
                                  DEC,simple_obs_DEC(actu));

    if(dist > max) { max = dist; }
  }

  return max;
}


double mean_sq_track_residual(track* X,simple_obs_array* arr) {
  simple_obs* actu;
  double mean = 0.0;
  double dist, tdif;
  double RA  = 0.0;
  double DEC = 0.0;
  int N = track_num_obs(X);
  int i;

  for(i=0;i<N;i++) {
    actu = track_indiv(X,i,arr);
    tdif = simple_obs_time(actu) - track_time(X);
    track_RA_DEC_prediction(X, tdif, &RA, &DEC);
    dist = simple_obs_euclidean_dist_given(RA, simple_obs_RA(actu),
                                           DEC,simple_obs_DEC(actu));
    mean += (dist * dist);
  }

  return mean/((double)N);
}


double max_sq_track_residual(track* X,simple_obs_array* arr) {
  double max = 0.0;
  dyv* resid;
  int N;
  int i;

  resid = mk_track_residuals(X, arr);
  N     = dyv_size(resid);
  for(i=0;i<N;i++) {

    if((i==0)||(max < dyv_ref(resid,i))) {
      max = dyv_ref(resid,i);
    }
  }
  free_dyv(resid);

  return max * max;
}


/* Compute the mean residual of track B in the eyes of track A */
double mean_sq_second_track_residual(track* A, track* B, simple_obs_array* arr) {
  simple_obs* actu;
  double mean = 0.0;
  double dist, tdif;
  double RA  = 0.0;
  double DEC = 0.0;
  int N = track_num_obs(B);
  int i;

  for(i=0;i<N;i++) {
    actu = track_indiv(B,i,arr);
    tdif = simple_obs_time(actu) - track_time(A);
    track_RA_DEC_prediction(A, tdif, &RA, &DEC);
    dist = simple_obs_euclidean_dist_given(RA, simple_obs_RA(actu),
                                           DEC,simple_obs_DEC(actu));
    mean += (dist * dist);
  }

  return mean/(double)N;
}


double single_track_residual(track* X, simple_obs* actu) {
  double dist, tdif;
  double RA  = 0.0;
  double DEC = 0.0;
  
  tdif = simple_obs_time(actu) - track_time(X);
  track_RA_DEC_prediction(X, tdif, &RA, &DEC);
  dist = simple_obs_euclidean_dist_given(RA, simple_obs_RA(actu),
                                         DEC,simple_obs_DEC(actu));

 return dist;
}


/* --- Output functions ------------------------------------------------------- */

void printf_track(track* S) {
  fprintf_track(stdout,"",S,"\n");
}


void fprintf_track(FILE* f, char* prefix, track* S, char* suffix) {
  fprintf(f,prefix);
  fprintf(f,"%10.5f; x=(%10.5f,%10.5f) v=(%10.5f,%10.5f) a=(%10.5f,%10.5f) - %5.3f",
          track_time(S),track_RA(S),track_DEC(S),
          track_vRA(S), track_vDEC(S),
          track_aRA(S), track_aDEC(S),
          track_brightness(S));  
  fprintf(f,suffix);
}


void fprintf_track_list(FILE* f, char* prefix, track* S, char* suffix) {
  int N = ivec_size(S->simp_ind);
  int i;

  fprintf(f,prefix);
  for(i=0;i<N;i++) {
    fprintf(f,"%i ",ivec_ref(S->simp_ind,i));
  }
  fprintf(f,suffix);
}


void fprintf_MPC_track(FILE* f, track* S, simple_obs_array* all_obs) {
  int N = ivec_size(S->simp_ind);
  int i;

  for(i=0;i<N;i++) {
    fprintf_MPC_simple_obs(f,simple_obs_array_ref(all_obs,ivec_ref(S->simp_ind,i)));
  }
}


/* Display the track as a white space separated list of observation IDs. */
void fprintf_track_ID_list(FILE* f, char* pre, track* S, char* post, 
                           simple_obs_array* all_obs) {
  int N = ivec_size(S->simp_ind);
  int i;

  fprintf(f,pre);
  for(i=0;i<N-1;i++) {
    fprintf(f,simple_obs_id_str(simple_obs_array_ref(all_obs,ivec_ref(S->simp_ind,i))));
    fprintf(f," ");
  }
  fprintf(f,simple_obs_id_str(simple_obs_array_ref(all_obs,ivec_ref(S->simp_ind,N-1))));
  fprintf(f,post);
}

char* mk_string_from_track(track* S) {
  char* res;

  res = mk_printf("%10.5f; x=(%10.5f,%10.5f) v=(%10.5f,%10.5f) a=(%10.5f,%10.5f) - %5.3f",
                  track_time(S),track_RA(S),track_DEC(S),
                  track_vRA(S), track_vDEC(S),
                  track_aRA(S), track_aDEC(S),
                  track_brightness(S));
  return res;
}


/* -----------------------------------------------------------------------*/
/* -------- Arrays Observations ------------------------------------------*/
/* -----------------------------------------------------------------------*/


/* --- Memory functions for the joint observation arrays ------------------- */

track_array* mk_empty_track_array(int max_size) {
  track_array* res = AM_MALLOC(track_array);
  int i;

  res->size = 0;
  res->max_size = max_size;
  res->the_obs = AM_MALLOC_ARRAY(track*,max_size);
  for(i=0;i<max_size;i++) {
    res->the_obs[i] = NULL;
  }

  return res;
}


track_array* mk_copy_track_array(track_array* old) {
  track_array* res = mk_empty_track_array(old->max_size);
  int i;

  for(i=0;i<old->size;i++) {
    if(old->the_obs[i]) {
      res->the_obs[i] = mk_copy_track(old->the_obs[i]);
    }
  }
  res->size = old->size;

  return res;
}


/* Copy the track array, but remove any NULL tracks. */
track_array* mk_filter_copy_track_array(track_array* old) {
  track_array* res = mk_empty_track_array(old->max_size);
  int j = 0;
  int i;

  for(i=0;i<old->size;i++) {
    if(old->the_obs[i]) {
      res->the_obs[j] = mk_copy_track(old->the_obs[i]);
      j++;
    }
  }
  res->size = j;

  return res;
}


track_array* mk_track_array_subset(track_array* old, ivec* inds) {
  track_array* res = mk_empty_track_array(ivec_size(inds));
  track* A;
  int i;

  for(i=0;i<ivec_size(inds);i++) {
    A = track_array_ref(old,ivec_ref(inds,i));
    if(A != NULL) {
      track_array_add(res,A);
    }
  }

  return res;
}


track_array* mk_merged_track_array(track_array* A, track_array* B) {
  int Na = track_array_size(A);
  int Nb = track_array_size(B);
  track_array* res = mk_empty_track_array(Na + Nb);
  track* X;
  int i;

  for(i=0;i<Na;i++) {
    X = track_array_ref(A,i);
    if(X != NULL) { 
      track_array_add(res, X);
    }
  }

  for(i=0;i<Nb;i++) {
    X = track_array_ref(B,i);
    if(X != NULL) { 
      track_array_add(res, X);
    }
  }

  return res;
}


track_array* mk_track_array_from_simple_obs_array(simple_obs_array* old) {
  track_array* res;
  track* X;
  int N = simple_obs_array_size(old);
  int i;

  res = mk_empty_track_array(N);
  for(i=0;i<N;i++) {
    if(simple_obs_array_ref(old,i)) {
      X = mk_track_single_ind(old,i);
      track_array_add(res,X);
      free_track(X);
    } else {
      track_array_add(res,NULL);
    }
  }

  return res;
}


track_array* mk_track_array_from_simple_obs_array_subset(simple_obs_array* old, ivec* ids) {
  simple_obs* A;
  track_array* res;
  track* X;
  int N = ivec_size(ids);
  int i, id;

  res = mk_empty_track_array(N);
  for(i=0;i<N;i++) {
        id = ivec_ref(ids,i);
    A  = simple_obs_array_ref(old,id);

    if(A != NULL) {
      X = mk_track_single_ind(old,id);
      track_array_add(res,X);
      free_track(X);
    } else {
      track_array_add(res,NULL);
    }
  }

  return res;
}


/* Make a simple obs array of the starting positions of the tracks. */
/* If inds == NULL then uses all tracks.                            */
simple_obs_array* mk_simple_obs_array_from_track_array(track_array* old, ivec* inds) {
  simple_obs_array* res;
  simple_obs*       X;
  track*            A;
  int i, ind, N;

  N   = track_array_size(old);
  res = mk_empty_simple_obs_array(N);

  for(i=0;i<N;i++) {

    if(inds == NULL) {
      ind = i;
    } else {
      ind = ivec_ref(inds,i);
    }
    A   = track_array_ref(old,ind);

    if(A != NULL) {
      X = mk_simple_obs_simplest(ind, track_time(A), track_RA(A), track_DEC(A),
                                 track_brightness(A));
      simple_obs_array_add(res,X);
      free_simple_obs(X);
    }
  }

  return res;
}


track_array* mk_track_array_from_matched_simple_obs(simple_obs_array* obs, ivec_array* matches,
                                                    int min_obs) {
  track_array* res;
  track*       X;
  ivec*        inds;
  int i, N;

  N   = ivec_array_size(matches);
  res = mk_empty_track_array(N);

  for(i=0;i<N;i++) {
    inds = ivec_array_ref(matches,i);

    if(ivec_size(inds) >= min_obs) {
      X    = mk_track_from_N_inds(obs,inds);
      track_array_add(res,X);
      free_track(X);
    }
  }

  return res;
}


track_array* mk_true_tracks(simple_obs_array* obs, ivec* true_groups) {
  track_array* res;
  track* trck;
  ivec* s_inds;
  ivec* inds;
  int s, e, gs;
  int count = 0;
  int N, i;

  s_inds = mk_indices_of_sorted_ivec(true_groups);
  N      = ivec_size(s_inds);
  
  /* Count the number of unique groups */
  s = 0;
  while((s < N)&&(ivec_ref(true_groups,ivec_ref(s_inds,s)) < 0)) { s++; }
  while(s < N) {
    e  = s;
    gs = ivec_ref(true_groups,ivec_ref(s_inds,s));
    while((e < N)&&(gs == ivec_ref(true_groups,ivec_ref(s_inds,e)))) {
      e++;
    }

    count++;
    s = e;
  }

  res = mk_empty_track_array(count);

  /* Form tracks from the groups */
  s = 0;
  while((s < N)&&(ivec_ref(true_groups,ivec_ref(s_inds,s)) < 0)) { s++; }
  while(s < N) {
    e  = s;
    gs = ivec_ref(true_groups,ivec_ref(s_inds,s));
    while((e < N)&&(gs == ivec_ref(true_groups,ivec_ref(s_inds,e)))) {
      e++;
    }

    inds = mk_ivec(e-s);
    for(i=s;i<e;i++) {
      ivec_set(inds,i-s,ivec_ref(s_inds,i));
    }
    trck = mk_track_from_N_inds(obs,inds);
    track_array_add(res,trck);

    free_ivec(inds);
    free_track(trck);

    s = e;
  }
  
  free_ivec(s_inds);  

  return res;
}


/* Makes a track array where all tracklets are part of true tracks */
/* of a given size = number of observations (used for testing only) */
track_array* mk_track_array_filter_on_true_track_size(track_array* old, ivec* true_groups,
                                                      int size, bool ORLESS) {
  track_array* res;
  ivec* used;
  ivec* obs_inds;
  ivec* trk_inds;
  bool good;
  int Nt, Ng, i, j, ind;

  Nt   = track_array_size(old);
  Ng   = ivec_max(true_groups);
  used = mk_zero_ivec(Nt);
  res  = mk_empty_track_array(track_array_size(old));

  /* For each group... */
  for(i=0;i<=Ng;i++) {
    obs_inds = mk_indices_in_ivec(true_groups,i);

    /* Does the group have the correct number of observations */
    good = (ivec_size(obs_inds) == size);
    if(ORLESS) {
      good = good || (ivec_size(obs_inds) <= size);
    }

    if(good) {

      /* Make sure the track corresponding to each observation 
         is in the resulting array. */
      for(j=0;j<ivec_size(obs_inds);j++) {
        trk_inds = mk_find_obs_in_tracks(old,ivec_ref(obs_inds,j));

        if(ivec_size(trk_inds) > 0) {
          ind = ivec_ref(trk_inds,0);
          
          if(ivec_ref(used,ind)==0) {
            ivec_set(used,ind,1);
            track_array_add(res,track_array_ref(old,ind));
          }
        }
        
        free_ivec(trk_inds);
      }
    }

    free_ivec(obs_inds);
  }

  free_ivec(used);

  return res;
}
 

/* Makes a track array where all tracklets are part of true tracks */
/* of a given size = number of observations (used for testing only) */
ivec* mk_groups_filter_on_true_track_size(ivec* true_groups, int size, bool ORLESS) {
  ivec* obs_inds;
  ivec* nu_groups; 
  bool good;
  int Ng, i, j;

  Ng        = ivec_max(true_groups);
  nu_groups = mk_constant_ivec(ivec_size(true_groups),-1);

  /* For each group... */
  for(i=0;i<=Ng;i++) {
    obs_inds = mk_indices_in_ivec(true_groups,i);

    /* Does the group have the correct number of observations */
    good = (ivec_size(obs_inds) == size);
    if(ORLESS) {
      good = good || (ivec_size(obs_inds) <= size);
    }

    if(good) {

      for(j=0;j<ivec_size(obs_inds);j++) {
        ivec_set(nu_groups,ivec_ref(obs_inds,j),i);
      }
    }

    free_ivec(obs_inds);
  }

  return nu_groups;
}


void free_track_array(track_array* old) {
  int i;

  for(i=0;i<old->size;i++) {
    if(old->the_obs[i]) { free_track(old->the_obs[i]); }
  }

  AM_FREE_ARRAY(old->the_obs,track*,old->max_size);
  AM_FREE(old,track_array); 
}


void track_array_double_size(track_array* old) {
  track** nu_arr;
  int i;

  nu_arr = AM_MALLOC_ARRAY(track*,2*old->max_size);
  for(i=0;i<old->size;i++) {
    nu_arr[i] = old->the_obs[i];
  }
  for(i=old->size;i<2*old->max_size;i++) {
    nu_arr[i] = NULL;
  }

  AM_FREE_ARRAY(old->the_obs,track*,old->max_size);

  old->the_obs  = nu_arr;
  old->max_size = 2*old->max_size;
}



track* safe_track_array_ref(track_array* X, int index) {
  my_assert((X->size > index)&&(index >= 0));
  return X->the_obs[index];
}


/* Return the current size of the array (the index of the
   last element +1) */
int safe_track_array_size(track_array* X) {
  return X->size;
}


/* Return the maximum size of the array  */
int safe_track_array_max_size(track_array* X) {
  return X->max_size;
}


int track_array_longest_track_length(track_array* X) {
  track* A;
  int longest = 0;
  int i;

  for(i=0;i<track_array_size(X);i++) {
    A = track_array_ref(X,i);
    if(A != NULL) {
      if(longest < track_num_obs(A)) {
        longest = track_num_obs(A);
      }
    }
  }

  return longest;
}


void track_array_set(track_array* X, int index, track* newobs) {
  my_assert((X->max_size > index)&&(index >= 0));

  if(X->the_obs[index] != NULL) {
    free_track(X->the_obs[index]);
    X->the_obs[index] = NULL;
  }

  if(newobs != NULL) {  
    X->the_obs[index] = mk_copy_track(newobs);
    if(index >= X->size) { X->size = index+1; }
  } else {
    X->the_obs[index] = NULL;
  }
}


void track_array_remove(track_array* X, int index) {
  my_assert((X->max_size > index)&&(index >= 0));

  if(X->the_obs[index] != NULL) {
    free_track(X->the_obs[index]);
  }
  X->the_obs[index] = NULL;
}


void track_array_add(track_array* X, track* A) {

  /* If the array is full, double the size */
  if(X->max_size <= X->size) {
    track_array_double_size(X);
  }

  track_array_set(X,X->size,A);
}


void track_array_add_all(track_array* X, track_array* toadd) {
  track* A;
  int i;

  for(i=0;i<track_array_size(toadd);i++) {
    A = track_array_ref(toadd,i);
    if(A != NULL) {
      track_array_add(X,A);
    }
  }
}


/* Find all occurrences of observation "obs_num" in a track. */
ivec* mk_find_obs_in_tracks(track_array* X, int obs_num) {
  track* A;
  ivec* res;
  ivec* inds;
  int i, j, N;

  N   = track_array_size(X);
  res = mk_ivec(0);

  /* Check each track... */
  for(i=0;i<N;i++) {

    A = track_array_ref(X,i);
    if(A !=  NULL) {

      /* Check all of the observations in the track */
      inds = track_individs(A);
      for(j=0;j<ivec_size(inds);j++) {
        if(obs_num == ivec_ref(inds,j)) {
          add_to_ivec(res,i);
        }
      }

    }

  }

  return res;
}


/* If the true groups are not needed then it can be set to NULL */
void track_array_add_random(track_array* arr, simple_obs_array* obs,
                            int N, ivec* true_groups) {
  simple_obs* A;
  track*      old;
  track*      nu;
  ivec*       inds;
  double      t0, t1;
  int ind, tot, num;
  int i, j;

  tot = track_array_size(arr);
  for(i=0;i<N;i++) {
    ind  = (int)(range_random(0.0,(double)tot - 1e-20));
    old  = track_array_ref(arr,ind);
    nu   = mk_copy_track(old);

    inds = track_individs(nu);
    num  = ivec_size(inds);
    t0   = track_time(old);

    /* Give the track a kick in RA and DEC */
    track_set_RA(nu,  track_RA(old)+gen_gauss()*0.02);
    track_set_vRA(nu, track_vRA(old)+gen_gauss()*0.002);

    track_set_DEC(nu,  track_DEC(old)+gen_gauss()*0.2);
    track_set_vDEC(nu, track_vDEC(old)+gen_gauss()*0.02);

    /* Create new observations to support this track. */
    for(j=0;j<num;j++) {
      ind = ivec_ref(inds,j);
      t1  = simple_obs_time(simple_obs_array_ref(obs,ind));

      A = mk_simple_obs_track_prediction(nu, t1 - t0);
      simple_obs_array_add(obs,A);
      free_simple_obs(A);

      ivec_set(inds,j,simple_obs_array_size(obs)-1);

      if(true_groups) { add_to_ivec(true_groups,-1); }
    }

    /* Add the new track to the array and groups. */
    track_array_add(arr,nu);
    free_track(nu);
  }
}



/*
  obs    - the observations
  times  - a vector of the times you want indexed
*/
ivec_array* mk_time_to_track_ind(track_array* obs, dyv* times) {
  int T = dyv_size(times);
  int N = track_array_size(obs);
  ivec_array* res;
  ivec* count = mk_zero_ivec(T);
  double curr_t;
  int i, tmatch;

  /* Count the number of times each time has occurred */
  for(i=0;i<N;i++) {
    tmatch = 0;
    curr_t = track_time(track_array_ref(obs,i));
    tmatch = find_index_in_dyv(times,curr_t,TRACK_TIME_DIFF);
    if(tmatch >= 0) {
      ivec_increment(count,tmatch,1);
    } else {
      printf("Time %f occurred in observations, but not dyv!\n",curr_t);
    }
  }

  /* create the result vector */
  res = mk_ivec_array_of_given_lengths(count);

  /* Fill the ivecs */
  for(i=N-1;i>=0;i--) {
    tmatch = 0;
    curr_t = track_time(track_array_ref(obs,i));
    tmatch = find_index_in_dyv(times,curr_t,0.01);
    if(tmatch >= 0) {
      ivec_array_ref_set(res,tmatch,ivec_ref(count,tmatch)-1,i);
      ivec_increment(count,tmatch,-1);
    }
  }

  free_ivec(count);

  return res;
}


ivec_array* mk_end_times_to_track_ind(track_array* obs, simple_obs_array* arr, dyv* end_times) {
  int T = dyv_size(end_times);
  int N = track_array_size(obs);
  ivec_array* res;
  ivec* count = mk_zero_ivec(T);
  double curr_t;
  int i, tmatch;

  /* Count the number of times each time has occurred */
  for(i=0;i<N;i++) {
    tmatch = 0;
    curr_t = track_last_time(track_array_ref(obs,i),arr);
    tmatch = find_index_in_dyv(end_times,curr_t,TRACK_TIME_DIFF);
    if(tmatch >= 0) {
      ivec_increment(count,tmatch,1);
    } else {
      printf("Time %f occurred in observations, but not dyv!\n",curr_t);
    }
  }

  /* create the result vector */
  res = mk_ivec_array_of_given_lengths(count);

  /* Fill the ivecs */
  for(i=N-1;i>=0;i--) {
    tmatch = 0;
    curr_t = track_last_time(track_array_ref(obs,i),arr);
    tmatch = find_index_in_dyv(end_times,curr_t,TRACK_TIME_DIFF);
    if(tmatch >= 0) {
      ivec_array_ref_set(res,tmatch,ivec_ref(count,tmatch)-1,i);
      ivec_increment(count,tmatch,-1);
    }
  }

  free_ivec(count);

  return res;
}


void track_array_force_t0(track_array* X, double t0) {
  int i;

  for(i=0;i<X->size;i++) {
    if(X->the_obs[i]) {
      track_force_t0(X->the_obs[i],t0);
    }
  }
}


void fprintf_track_array(FILE* f, track_array* X) {
  int i;

  fprintf(f,"Observation array (%i of %i):\n",X->size,X->max_size);
  for(i=0;i<X->size;i++) {
    fprintf(f,"%6i) ",i);
    if(X->the_obs[i]) { 
      fprintf_track(f,"",X->the_obs[i],"\n");
    } else {
      fprintf(f,"EMPTY!\n");
    }
  }
}


void fprintf_track_array_list(FILE* f, track_array* X) {
  int i;
  
  for(i=0;i<X->size;i++) { 
    fprintf(f,"%6i: ",i);
    if(X->the_obs[i] != NULL) {
      fprintf_track_list(f,"",X->the_obs[i],"\n");
    } else {
      fprintf(f,"EMPTY!\n");
    }
  }
}


/* Display the track array as a white space separated list of observation IDs. */
void fprintf_track_array_ID_list(FILE* f, track_array* X, simple_obs_array* all_obs) {
  int i;

  for(i=0;i<X->size;i++) {
    if(X->the_obs[i] != NULL) {
      fprintf_track_ID_list(f,"",X->the_obs[i],"\n",all_obs);
    }
  }
}


/* Print the track array as:               */
/* [a_RA, v_RA, x_RA, a_DEC, v_DEC, x_DEC] */
void fprintf_track_array_as_RD_dym(FILE* f, track_array* X) {
  track *Y;
  double r, d, vr, vd;
  double t0, t;
  dyv* times;
  dym* M;
  int N = track_array_size(X);
  int i;

  M     = mk_zero_dym(N,6);
  times = mk_sort_all_track_times(X);
  t0    = dyv_ref(times,0);

  for(i=0;i<N;i++) {
    Y = track_array_ref(X,i);
    t = track_time(Y);

    track_RDVV_prediction(Y,(t0-t),&r,&d,&vr,&vd);

    dym_set(M,i,0,track_aRA(Y));
    dym_set(M,i,1,vr);
    dym_set(M,i,2,r);
    dym_set(M,i,3,track_aDEC(Y));
    dym_set(M,i,4,vd);
    dym_set(M,i,5,d);
  }

  save_dym(f,M);

  free_dym(M);
  free_dyv(times);
}


void incr_track_name(char* nme) {
  bool roll = TRUE;
  int i = 5;

  while((roll==TRUE)&&(i >= 0)) {
    roll = FALSE;
    nme[i] += 1;

    if((nme[i] > '9')&&(nme[i] < 'A')) { nme[i] = 'A'; }
    if((nme[i] > 'Z')&&(nme[i] < 'a')) { nme[i] = 'a'; }
    if(nme[i] > 'z') { 
      roll   = TRUE;
      nme[i] = '0';
      i--;
    }
  }
}


/* Displays the track list as a series of MPS strings
   each string is modified so that the label
   corresponds to the track (i.e. observations may
   appear multiple times with different labels) */
void fprintf_track_array_mod_MPC(FILE* f, track_array* X, simple_obs_array* obs) {
  track* A;
  ivec* inds;
  char* str;
  char  nme[7];
  int i, j, k;

  for(i=0;i<6;i++) {
    nme[i] = '0';
  }
  nme[6] = 0;

  for(i=0;i<track_array_size(X);i++) {
    A = track_array_ref(X,i);
    if(A != NULL) {
      inds = track_individs(A);
      for(j=0;j<ivec_size(inds);j++) {
        str = mk_MPC_string_from_simple_obs(simple_obs_array_ref(obs,ivec_ref(inds,j)));
        for(k=0;k<6;k++) { str[k+5] = nme[k]; }
        str[k+5] = ' ';

        fprintf(f,str);
        fprintf(f,"\n");
        free_string(str);
      }
      incr_track_name(nme);
    }
  }
}


void incr_track_name2(char* nme) {
  bool roll = TRUE;
  int i = 5;

  while((roll==TRUE)&&(i >= 0)) {
    roll = FALSE;
    nme[i] += 1;

    if((nme[i] > '9')&&(nme[i] < 'A')) { nme[i] = 'A'; }
    if((nme[i] > 'Z')&&(nme[i] < 'a')) { nme[i] = 'a'; }
    if(nme[i] > 'z') {
      roll   = TRUE;
      nme[i] = '0';
      i--;
    }
  }
}


int _sort_names(const void *a, const void *b) {
    int ai, bi;
    ai = atoi(*(const char **) a);
    bi = atoi(*(const char **) b);
    if (ai < bi) {
        return -1;
    }
    else if (ai > bi) {
        return 1;
    }
    return 0;   /* equal */
}


void dump_trackids_to_file(FILE* fp, simple_obs_array* obs, track_array* trcks) {
  track* A;
  ivec* inds;
  int maxobj = track_array_size(trcks);
  int i, j, k;
  char *foo;
  int max_dets;
  int num_dets;
  int num_obs;
  int num_unique_names;
  int max_name_len;
  char **sorted_names;
  char *name_buf;


  /* First count the number of observations in each track and find the maximum, so
  that we can allocate a buffer for sorting the detection IDs for each track. */
  max_dets = -1;
  for (i = 0; i < maxobj; i++) {
    A = track_array_ref(trcks,i);
    if (NULL != A) {
      inds = track_individs(A);
      num_dets = ivec_size(inds);
      if (num_dets > max_dets) {
        max_dets = num_dets;    /* save max */
      }
    }
  }

  if (-1 == max_dets) {
    return; /* empty list; nothing to do */
  }

  /* Now re-scan the data, fetch the det IDs and sort them.  Then walk the sorted
  list and emit Milani-style track IDs. */
  sorted_names = AM_MALLOC_ARRAY(char*,max_dets);
  for(i=0;i<maxobj;i++) {
    A = track_array_ref(trcks,i);
    if((A != NULL) && (simple_obs_time(track_last(A,obs)) - simple_obs_time(track_first(A,obs)) > 0.8)) {

      inds = track_individs(A);
      num_obs = ivec_size(inds);
      for(j=0;j<num_obs;j++) {
        foo = simple_obs_id_str(simple_obs_array_ref(obs,ivec_ref(inds,j)));
        sorted_names[j] = foo;      /* save name */
      }

      /* Sort the names. */
      qsort(sorted_names, num_obs, sizeof(char *), _sort_names);

      /* Compress the list of sorted names, removing duplicates. */
      num_unique_names = 0;
      for (j = 0; j < num_obs; j++) {
        /* Compare name in uncompressed list with last in compressed list. */
        if (0 == j || strcmp(sorted_names[num_unique_names - 1], sorted_names[j]) != 0) {
            /* Different, so add a new name. */
            sorted_names[num_unique_names] = sorted_names[j];
            num_unique_names++;

        }
      }

      /* Find the max length of a name for this track out the compressed list of names. */
      max_name_len = -1;
      for (j = 0; j < num_unique_names; j++) {
        k = strlen(sorted_names[j]);
        if (k > max_name_len) {
          max_name_len = k;
        }
      }
      name_buf = (char *) malloc(max_name_len + 1);

      /* Emit the track ID. */
      for (j = 0; j < num_unique_names; j++) {
        /* Copy the string and terminate at the padding. */
        k = 0;
        while (sorted_names[j][k] != 0 && sorted_names[j][k] != ' ') {
            name_buf[k] = sorted_names[j][k];
            k++;
        }
        name_buf[k] = 0;    /* terminate */

        if (j < num_unique_names - 1) {
          fprintf(fp, "%s=", name_buf);
        }
        else {
          fprintf(fp, "%s\n", name_buf);
        }
      }

      free(name_buf);
    }
  }
  AM_FREE_ARRAY(sorted_names, char *, max_dets);
}



void dump_tracks_to_file(FILE* f_obs, FILE* f_summary, char* score_file,
                         bool indiv_files,
                         simple_obs_array* obs, track_array* trcks) {
  FILE* f_indiv;
  FILE* f_score = NULL;
  track* A;
  ivec* inds;
  int maxobj = track_array_size(trcks);
  int i,j,k;
  char* str_indiv;
  char* str;
  char  nme[7];
  char *foo;

  if(strlen(score_file) > 2) {
    f_score = fopen(score_file,"w");
  }

  if(indiv_files) {
    system("rm -rf neos_indiv_tracks");
    system("mkdir neos_indiv_tracks");
  }

  for(i=0;i<6;i++) {
    nme[i] = '0';
  }
  nme[6] = 0;

  for(i=0;i<maxobj;i++) {
    A = track_array_ref(trcks,i);
    if((A != NULL)&&(simple_obs_time(track_last(A,obs)) - simple_obs_time(track_first(A,obs)) > 0.8\
                     )) {

      /* Dump the observations to a temporary file... */
      inds = track_individs(A);

      /* Write the observations out to a file */
      for(j=0;j<ivec_size(inds);j++) {
        str = mk_MPC_string_from_simple_obs(simple_obs_array_ref(obs,ivec_ref(inds,j)));
        for(k=0;k<6;k++) { str[k+5] = nme[k]; }
        str[11] = ' ';

        fprintf(f_obs,str);
        fprintf(f_obs,"\n");
        free_string(str);

        /* Write to the summary file */
        fprintf(f_summary,nme);
        fprintf(f_summary," ");
        foo = simple_obs_id_str(simple_obs_array_ref(obs,ivec_ref(inds,j)));
        fprintf(f_summary,foo);
        fprintf(f_summary," %8i\n",ivec_ref(inds,j));
      }

      if(f_score != NULL) {
        fprintf(f_score,nme);
        fprintf(f_score," %20g\n",mean_sq_track_residual(A,obs));
      }

      if(indiv_files) {
        str_indiv = mk_printf("./neos_indiv_tracks/%c%c%c%c%c%c.track",
                              nme[0],nme[1],nme[2],nme[3],nme[4],nme[5]);
        f_indiv = fopen(str_indiv,"w");
        for(j=0;j<ivec_size(inds);j++) {
          fprintf_MPC_simple_obs(f_indiv,simple_obs_array_ref(obs,ivec_ref(inds,j)));
        }
        fclose(f_indiv);
        free_string(str_indiv);
      }
    }

    incr_track_name2(nme);
  }

  if(f_score != NULL) {
    fclose(f_score);
  }

}


void dump_tracks_to_MPC_file(char* file_name, simple_obs_array* obs, track_array* trcks) {
  FILE* f_obs;
  track* A;
  ivec* inds;
  int maxobj = track_array_size(trcks);
  int i,j,k;
  char* str;
  char  nme[7];

  f_obs = fopen(file_name,"w");
  if(f_obs) {

    /* Create the initial name for the tracklets */
    for(i=0;i<6;i++) {
      nme[i] = '0';
    }
    nme[6] = 0;

    for(i=0;i<maxobj;i++) {
      A = track_array_ref(trcks,i);
      if((A != NULL)&&(track_time_length(A,obs) > 1e-10)) {
        inds = track_individs(A);

        /* Write the observations out to a file */
        for(j=0;j<ivec_size(inds);j++) {
          str = mk_MPC_string_from_simple_obs(simple_obs_array_ref(obs,ivec_ref(inds,j)));
          for(k=0;k<6;k++) { str[k+5] = nme[k]; }
          str[11] = ' ';

          fprintf(f_obs,str);
          fprintf(f_obs,"\n");
          free_string(str);
        }
      }
      incr_track_name2(nme);
    }

    fclose(f_obs);
  } else {
    printf("ERROR: Failed to open %s for writing.\n",file_name);
  }

}


dyv* mk_sort_all_track_times(track_array* X) {
  track* A;
  double time;
  dyv*   times;
  dyv*   s_times;
  int    N = track_array_size(X);
  int    i, ind;

  times = mk_dyv(0);
  for(i=0;i<N;i++) {
    A = track_array_ref(X,i);
    if(A != NULL) {
      time = track_time(A);
      ind  = find_index_in_dyv(times,time,TRACK_TIME_DIFF);
      if(ind == -1) {
        add_to_dyv(times,time);
      }
    }
  }
  s_times = mk_sorted_dyv(times);
  free_dyv(times);

  return s_times;
}


dyv* mk_sort_all_track_end_times(track_array* X, simple_obs_array* obs) {
  track* A;
  double time;
  dyv*   times;
  dyv*   s_times;
  int    N = track_array_size(X);
  int    i, ind;

  times = mk_dyv(0);
  for(i=0;i<N;i++) {
    A = track_array_ref(X,i);
    if(A != NULL) {
      time = track_last_time(A,obs);
      ind  = find_index_in_dyv(times,time,TRACK_TIME_DIFF);
      if(ind == -1) {
        add_to_dyv(times,time);
      }
    }
  }
  s_times = mk_sorted_dyv(times);
  free_dyv(times);

  return s_times;
}


void track_array_flatten_to_plates(track_array* arr, simple_obs_array* obs, 
                                  double plate_width) {
  track* A;
  dyv* times;
  int N = track_array_size(arr);
  int i;

  times = mk_simple_obs_plate_times(obs,plate_width);

  for(i=0;i<N;i++) {
    A = track_array_ref(arr,i);
    if(A != NULL) {
      track_flatten_to_plates(A,obs,times,plate_width);
    }
  }

  free_dyv(times);
}


void track_array_unflatten_to_plates(track_array* arr, simple_obs_array* obs, 
                                     dyv* org_times) {
  track* A;
  int N = track_array_size(arr);
  int i;

  for(i=0;i<N;i++) {
    A = track_array_ref(arr,i);
    if(A != NULL) { track_unflatten_to_plates(A,obs,org_times); }
  }

}


/* --- Track evaluation functions --------------------------- */

/* Reduce the list of true groups to only with 'min_obs' */
/* or more observations. Also renormalizes the group     */
/* numbers so max(result) == # of tracks with min_obs-1  */
ivec* mk_tracks_of_size_from_list(ivec* true_tracks, int min_obs) {
  ivec* trcks;
  ivec* s_inds;
  int T;
  int count = 0;
  int s = 0;
  int e = 0;
  int i, t;

  /* Sort the group ivecs so we can easily determine length. */
  trcks  = mk_copy_ivec(true_tracks);
  s_inds = mk_indices_of_sorted_ivec(trcks);
  T      = ivec_size(s_inds);

  /* For each group find it's length and remove it is needed. */
  while(s < T) {
    e = s + 1;
    t = ivec_ref(trcks,ivec_ref(s_inds,s));
    
    while((e < T)&&(ivec_ref(trcks,ivec_ref(s_inds,e))==t)) {
      e++;
    }
    e--;

    /* If the track is too short ignore it. */
    if(e-s+1 < min_obs) {
      for(i=s;i<=e;i++) { ivec_set(trcks,ivec_ref(s_inds,i),-1); }
    } else {
      for(i=s;i<=e;i++) { ivec_set(trcks,ivec_ref(s_inds,i),count); }
      count++;
    }
   
    s = e + 1;
  }

  free_ivec(s_inds);

  return trcks;
}


/* Reduce the list of true groups to only with 'min_obs' */
/* or more observations. Also renormalizes the group     */
/* numbers so max(result) == # of tracks with min_obs-1  */
/* Also filter on the number of nights covered.          */
ivec* mk_tracks_of_length_from_list(simple_obs_array* obs, ivec* true_tracks, 
                                    int min_obs, int min_nights) {
  track* X;
  ivec* trcks;
  ivec* s_inds;
  ivec* inds;
  int T;
  int count = 0;
  int s = 0;
  int e = 0;
  int i, t;
  bool valid;

  /* Sort the group ivecs so we can easily determine length. */
  trcks  = mk_copy_ivec(true_tracks);
  s_inds = mk_indices_of_sorted_ivec(trcks);
  T      = ivec_size(s_inds);

  /* For each group find it's length and remove it is needed. */
  while(s < T) {
    e = s + 1;
    t = ivec_ref(trcks,ivec_ref(s_inds,s));
    
    while((e < T)&&(ivec_ref(trcks,ivec_ref(s_inds,e))==t)) {
      e++;
    }
    e--;

    /* If the track is too short ignore it. */
    valid = (e-s+1 >= min_obs);

    /* Test the duration in nights... */
    if(valid) {
      inds = mk_ivec(e-s+1);
      for(i=s;i<=e;i++) { ivec_set(inds,i-s,ivec_ref(s_inds,i)); }
      X = mk_track_from_N_inds(obs,inds);

      valid = (track_num_nights_seen(X,obs) >= min_nights);
      
      free_track(X);
      free_ivec(inds);
    }

    /* If the track is not valid, remove it. */
    if(valid==FALSE) {
      for(i=s;i<=e;i++) { ivec_set(trcks,ivec_ref(s_inds,i),-1); }
    } else {
      for(i=s;i<=e;i++) { ivec_set(trcks,ivec_ref(s_inds,i),count); }
      count++;
    }
   
    s = e + 1;
  }

  free_ivec(s_inds);

  return trcks;
}


/* Update the pairwise associations seen with this new track. */
/* The variables 'nu' and 'cor' indicate the number of new    */
/* assocations and new correct associations in this track.    */
void track_array_update_seen_matches(track* X, ivec* true_tracks, sb_graph* seen,
                                     int* nu, int* cor) {
  ivec* inds;
  int i,j,a,b;
  
  /* Reset the counts */
  nu[0]  = 0;
  cor[0] = 0;

  /* Go through all the pairwise associations in this track. */
  inds = track_individs(X);
  for(i=0;i<ivec_size(inds)-1;i++) {
    for(j=i+1;j<ivec_size(inds);j++) {
      a = ivec_ref(inds,i);
      b = ivec_ref(inds,j);

      /* Check if this association is new. */
      if(sb_graph_edge_exists(seen,a,b) == FALSE) {
        nu[0] = nu[0] + 1;

        /* Check if this association is valid. */
        if((ivec_ref(true_tracks,a) == ivec_ref(true_tracks,b))&&(ivec_ref(true_tracks,a) != -1)) {
          cor[0] = cor[0] + 1;
        }

        /* Mark association as old. */
        sb_graph_edge_set(seen,a,b);    
      }
    }
  }
}


/* Count the percentage of the adjacent matches that are correct */
double track_array_percent_correct(track_array* mtch, ivec* true_tracks, int min_obs) {
  sb_graph* grph;
  ivec* trcks;
  track* X;
  double count_total   = 0.0;
  double count_correct = 0.0;
  int N = track_array_size(mtch);
  int i,nu,cor;

  /* Remove all groups smaller than a given threshold */
  trcks = mk_tracks_of_size_from_list(true_tracks,min_obs);

  /* Count the number of correct and total associations. */
  grph = mk_empty_sb_graph(ivec_size(true_tracks));

  for(i=0;i<N;i++) {
    X = track_array_ref(mtch,i);

    if(X != NULL) {
      track_array_update_seen_matches(X,trcks,grph,&nu,&cor);
      
      count_total   += (double)nu;
      count_correct += (double)cor;
    }
  }

  free_sb_graph(grph);
  free_ivec(trcks);

  return (count_correct/count_total);
}


/* Count the percentage of the adjacent matches that are correct */
double track_array_adj_percent_correct(track_array* mtch, ivec* true_tracks) {
  sb_graph* grph;
  track* X;
  ivec*  inds;
  double count_total   = 0.0;
  double count_correct = 0.0;
  int N = track_array_size(mtch);
  int i,j;
  int a,b;

  grph = mk_empty_sb_graph(ivec_size(true_tracks));

  for(i=0;i<N;i++) {
    X = track_array_ref(mtch,i);

    if(X != NULL) {
      inds = track_individs(X);
      for(j=0;j<ivec_size(inds)-1;j++) {
        a = ivec_ref(inds,j);
        b = ivec_ref(inds,j+1);

        if(sb_graph_edge_exists(grph,a,b) == FALSE) {
          count_total += 1.0;
 
          /* If both points are from the same group...
             AND a has never seen another point before, count it
             as a fair match. */
          if((ivec_ref(true_tracks,a) == ivec_ref(true_tracks,b))&&
             (ivec_ref(true_tracks,a) != -1)) {

            if(sb_graph_num_edges(grph,a) == 0) {
              count_correct += 1.0;
            }
            sb_graph_edge_set(grph,a,b);
          }
        }
      }
    }
  }

  free_sb_graph(grph);

  return (count_correct/count_total);
}



double track_array_percent_found(track_array* mtchs, ivec* true_tracks, int min_obs) {
  sb_graph* grph;
  track* X;
  ivec* trcks;
  ivec* count;
  double count_total   = 0.0;
  double count_correct = 0.0;
  double num;
  int nu, cor;
  int i, grp;

  /* Remove all groups smaller than a given threshold */
  trcks = mk_tracks_of_size_from_list(true_tracks,min_obs);

  /* Count the total number of associations we should see... */
  count = mk_zero_ivec(ivec_max(trcks)+1);
  for(i=0;i<ivec_size(trcks);i++) {
    grp = ivec_ref(trcks,i);

    if(grp >= 0) {
      ivec_set(count,grp,ivec_ref(count,grp)+1);
    }
  }

  for(i=0;i<ivec_size(count);i++) {
    num = (double)ivec_ref(count,i);
    count_total += 0.5 * num * (num - 1.0);
  }

  /* Count the number of correctly found tracks */
  grph = mk_empty_sb_graph(ivec_size(true_tracks));
  for(i=0;i<track_array_size(mtchs);i++) {
    X = track_array_ref(mtchs,i);

    if(X != NULL) {
      track_array_update_seen_matches(X,trcks,grph,&nu,&cor);
      count_correct += (double)cor;
    }
  }

  free_sb_graph(grph);
  free_ivec(count);
  free_ivec(trcks);

  return (count_correct/count_total);
}


/* Returns the percentage of the true adjacent matches that were actually found. */
double track_array_adj_percent_found(track_array* mtchs, ivec* true_tracks) {
  ivec* sorted_tracks;
  sb_graph* grph;
  track* X;
  ivec*  inds;
  double count_total   = 0.0;
  double count_correct = 0.0;
  int N = track_array_size(mtchs);
  int num_tracks;
  int i,j;
  int a,b;

  /* Count the true number of adjacent matches (ignore if
     the true track is -1) */
  sorted_tracks = mk_sivec_from_ivec(true_tracks);
  num_tracks    = ivec_size(sorted_tracks);
  for(i=0;i<num_tracks;i++) {
    inds = mk_indices_in_ivec(true_tracks,ivec_ref(sorted_tracks,i));
    count_total += (double)(ivec_size(inds) - 1);
    free_ivec(inds);
  }

  grph = mk_empty_sb_graph(ivec_size(true_tracks));

  /* Count the number of correctly found adjacent pairs */
  for(i=0;i<N;i++) {
    X = track_array_ref(mtchs,i);

    if(X != NULL) {
      inds = track_individs(X);
      for(j=0;j<ivec_size(inds)-1;j++) {
        a = ivec_ref(inds,j);
        b = ivec_ref(inds,j+1);

        if(sb_graph_edge_exists(grph,a,b) == FALSE) {
 
          /* If both points are from the same group...
             AND a has never seen another point before, count it
             as a fair match. */
          if((ivec_ref(true_tracks,a) == ivec_ref(true_tracks,b))&&
             (ivec_ref(true_tracks,a) != -1)) {

            if(sb_graph_num_edges(grph,a) == 0) {
              count_correct += 1.0;
            }
            sb_graph_edge_set(grph,a,b);
          }
        }
      }
    }
  }

  free_ivec(sorted_tracks);
  free_sb_graph(grph);

  return (count_correct/count_total);
}


/* Makes a 2 column dym.  The first column is the number */
/* of TRUE positives and the second column is the number */
/* of FALSE positives.  Tracks are assumed to be in      */
/* sorted order.                                         */
/* The last row contains: [total_true_assocations, 0]    */
dym* mk_track_array_ROC_curve(track_array* mtchs, ivec* true_tracks, int min_obs) {
  dym* res;
  sb_graph* grph;
  track* X;
  ivec* trcks;
  ivec* count;
  double n_true = 0.0;      /* # of true associations */
  double n_seen = 0.0;      /* # of associations seen */
  double n_corr = 0.0;      /* # of correct associations seen */
  double num;
  int nu, cor;
  int i, grp, N;

  /* Allocate space for a result. */
  N   = track_array_size(mtchs);
  res = mk_zero_dym(N+1,2);

  /* Remove all groups smaller than a given threshold */
  trcks = mk_tracks_of_size_from_list(true_tracks,min_obs);

  /* Count the total number of associations we should see... */
  count = mk_zero_ivec(ivec_max(trcks)+1);
  for(i=0;i<ivec_size(trcks);i++) {
    grp = ivec_ref(trcks,i);
    if(grp >= 0) { ivec_set(count,grp,ivec_ref(count,grp)+1); }
  }
  for(i=0;i<ivec_size(count);i++) {
    num = (double)ivec_ref(count,i);
    n_true += 0.5 * num * (num - 1.0);
  }
  dym_set(res,N,0,n_true);

  /* Count the number of correctly found tracks */
  grph = mk_empty_sb_graph(ivec_size(true_tracks));
  for(i=0;i<N;i++) {
    X = track_array_ref(mtchs,i);

    if(X != NULL) {
      track_array_update_seen_matches(X,trcks,grph,&nu,&cor);
      n_seen += (double)nu;
      n_corr += (double)cor;
    }
    
    dym_set(res,i,0,n_corr);
    dym_set(res,i,1,n_seen-n_corr);
  }

  free_sb_graph(grph);
  free_ivec(count);
  free_ivec(trcks);

  return res;
}


/* Compares track arrays A and B and returns true iff they are the  */
/* same (up to a change in order).  Also "returns" number of points */
/* in A but not B and the number of points in B but not A.          */
bool track_array_compare_slow(track_array* A, track_array* B, int* aOnly, int* bOnly) {
  track *At, *Bt;
  int Na, Nb;
  int i, j;
  bool found;

  Na = track_array_size(A);
  Nb = track_array_size(B);

  aOnly[0] = 0;
  bOnly[0] = 0;

  for(i=0;i<Na;i++) {
    found = FALSE;
    At = track_array_ref(A,i);
    
    for(j=0;(j<Nb)&(found==FALSE);j++) {
      Bt = track_array_ref(B,j);
      found = track_equal(At, Bt);
    }

    if(found == FALSE) {
      aOnly[0] = aOnly[0] + 1;
    }
  }

  for(i=0;i<Nb;i++) {
    found = FALSE;
    Bt = track_array_ref(B,i);
    
    for(j=0;(j<Na)&(found==FALSE);j++) {
      At = track_array_ref(A,j);
      found = track_equal(At, Bt);
    }

    if(found == FALSE) {
      bOnly[0] = bOnly[0] + 1;
    }
  }


  return (aOnly[0]==0)&&(bOnly[0]==0);
}


/* Returns the highest percentage of observation labels that */
/* match and are not -1.  best_type indicates the most       */
/* common non-negative label.                                */
double track_percent_same(ivec* obs, ivec* true_groups, int* best_type) {
  int best = 0;
  int curr = 0;
  int i,j,gi,gj;

  best_type[0] = -1;

  for(i=0;i<ivec_size(obs);i++) {
    gi   = ivec_ref(true_groups,ivec_ref(obs,i));
    curr = 0;

    if(gi > -1) {
      for(j=0;j<ivec_size(obs);j++) {
        gj = ivec_ref(true_groups,ivec_ref(obs,j));
        if(gi == gj) { curr++; }
      }

      if(curr > best) {
        best_type[0] = gi;
        best = curr; 
      }
    }
  }

  return (double)best/(double)ivec_size(obs);
}


/* Takes a sorted track array and returns an ivec such that    */
/* each entry corresponds with a track and is 1 iff >= percent */
/* of the track's observations have matching labels.           */
ivec* mk_track_array_roc_vec(simple_obs_array* oarr, track_array* res, ivec* true_groups, 
                             int min_obs, int min_sup, double percent) {
  track* X;
  ivec* seen;
  ivec* roc;
  ivec* obs;
  double score;
  int best;
  int G = ivec_max(true_groups);
  int N = track_array_size(res);
  int i;

  seen = mk_zero_ivec(G+1);
  roc  = mk_zero_ivec(N); 

  for(i=0;i<N;i++) {
    X   = track_array_ref(res,i);
    obs = track_individs(X);

    if((ivec_size(obs) >= min_obs)&&(track_num_nights_seen(X,oarr) >= min_sup)) {
      score = track_percent_same(obs, true_groups, &best);

      if((score >= percent)&&(best >= -1)&&(ivec_ref(seen,best) == 0)) {
        ivec_set(seen,best,1);
        ivec_set(roc,i,1);
      }
    }
  }

  free_ivec(seen);

  return roc;
}


/* Returns an imat such that each row corresponds to a track */
/* length and the columns indicate how many of the entries   */
/* correspond to the SAME object.                            */
/* Mij says that there are Mij tracks of length i with j     */
/* matching entries.                                         */
imat* mk_track_array_consistency_mat(track_array* res, ivec* true_groups) {
  imat* themat;
  ivec* obs;
  double score;
  int best;
  int longest = 0;
  int N = track_array_size(res);
  int i, r, c;

  /* Find the longest track... */
  for(i=0;i<N;i++) {
    obs   = track_individs(track_array_ref(res,i));
    if(ivec_size(obs) > longest) {
      longest = ivec_size(obs);
    }
  }

  /* For each track compute the most number of matches... */
  themat = mk_zero_imat(longest+1,longest+1);
  for(i=0;i<N;i++) {
    obs   = track_individs(track_array_ref(res,i));
    score = track_percent_same(obs, true_groups, &best);

    r = ivec_size(obs);
    c = (int)(track_percent_same(obs,true_groups,&best)*(double)(r)+1e-5);

    imat_set(themat,r,c,imat_ref(themat,r,c)+1);
  }

  return themat;
}


/* Returns an imat such that each row corresponds to a true  */
/* track length: Mij says that for all tracks of length i    */
/* our results contained Mij entries that had j entries that */
/* matched the track.  Only the best match is tracked.       */
imat* mk_track_array_inv_consistency_mat(track_array* res, ivec* true_groups) {
  ivec* true_sizes;
  ivec* found_sizes;
  imat* themat;
  ivec* obs;
  double score;
  int longest = 0;
  int G = ivec_max(true_groups)+1;
  int N = track_array_size(res);
  int i, r, c, ind, size;

  /* Find the true sizes of the tracks... */
  true_sizes  = mk_zero_ivec(G);
  found_sizes = mk_zero_ivec(G); 
  for(i=0;i<ivec_size(true_groups);i++) {
    ind = ivec_ref(true_groups,i);
    if(ind >= 0) {
      ivec_set(true_sizes,ind,ivec_ref(true_sizes,ind)+1);
    }
  }
  longest = ivec_max(true_sizes);

  /* For each track compute the most number of matches... */
  for(i=0;i<N;i++) {
    obs   = track_individs(track_array_ref(res,i));
    score = track_percent_same(obs, true_groups, &ind);
    size  = (int)(score*(double)ivec_size(obs)+1e-5);

    if(ivec_ref(found_sizes,ind) < size) {
      ivec_set(found_sizes,ind,size);
    }
  }

  /* Compute the results... */
  themat = mk_zero_imat(longest+1,longest+1);   
  for(i=0;i<G;i++) {
    r = ivec_ref(true_sizes,i);
    c = ivec_ref(found_sizes,i);
    imat_set(themat,r,c,imat_ref(themat,r,c)+1);
  }

  free_ivec(true_sizes);
  free_ivec(found_sizes);

  return themat;
}


int track_RJ_compute_overlaps_recurse(ivec* obs, ivec* true_groups, int i, int G) {
  int curr = 0;

  if(i < ivec_size(obs)) { 
    curr = track_RJ_compute_overlaps_recurse(obs,true_groups,i+1,G);
    if(G == ivec_ref(true_groups,ivec_ref(obs,i))) { curr++; }
  }
  
  return curr;
}

void track_RJ_compute_overlaps(ivec* obs, ivec* true_groups, ivec* overlap_sizes,
                               ivec* best_ind, int learned_ind) {
  int count, i, G;

  for(i=0;i<ivec_size(obs);i++) {
    G = ivec_ref(true_groups,ivec_ref(obs,i));

    if(G >= 0) {
      count = 1 + track_RJ_compute_overlaps_recurse(obs,true_groups,i+1,G);

      if(count > ivec_ref(overlap_sizes,G)) {
        ivec_set(overlap_sizes,G,count);
        ivec_set(best_ind,G,learned_ind);
      }
    }
  }
}


/* Returns a score dym.  The columns indicate the size of the */
/* true tracks and the rows indicate the rows of the best     */
/* matching track that was found.  M_ij indicates the         */
/* percentage of true tracks of length j whose best match was */
/* of length i.  If overlap = true then this becomes whose    */
/* best match had overlap of length i.                        */
dym* mk_track_array_RJ_score_mat(track_array* res, ivec* true_groups, 
                                 bool overlap) {
  ivec_array* true_tracks;
  ivec* overlap_sizes;
  ivec* best_ind;
  dym*  themat;
  ivec* obs;
  double count;
  int L_tru = 0;
  int L_fnd = 0;
  int GT = ivec_max(true_groups)+1;
  int GL = track_array_size(res);
  int i, j, r, c, ind;

  /* Create the true tracks. */
  true_tracks = mk_zero_ivec_array(GT);
  for(i=0;i<ivec_size(true_groups);i++) {
    ind = ivec_ref(true_groups,i);
    if(ind >= 0) {
      add_to_ivec_array_ref(true_tracks,ind,i);
    }
  }

  /* Find the longest true track. */
  for(i=0;i<GT;i++) {
    if(ivec_array_ref_size(true_tracks,i) > L_tru) {
      L_tru = ivec_array_ref_size(true_tracks,i);
    }
  }

  /* Find the longest returned track. */
  for(i=0;i<GL;i++) {
    if(track_size(track_array_ref(res,i)) > L_fnd) {
      L_fnd = track_size(track_array_ref(res,i));
    }
  }

  /* For each learned track how much does it overlap each */
  /* of the true tracks.  Only record it if this is a new */
  /* best overlap size.                                   */
  overlap_sizes = mk_zero_ivec(GT);
  best_ind      = mk_zero_ivec(GT);
  for(i=0;i<GL;i++) {
    obs   = track_individs(track_array_ref(res,i));
    track_RJ_compute_overlaps(obs,true_groups,overlap_sizes,best_ind,i);
  }

  /* Compute the results... */
  themat = mk_zero_dym(L_fnd+1,L_tru+1);   
  for(i=0;i<GT;i++) {
    c = ivec_array_ref_size(true_tracks,i);
    r = 0;
    if(ivec_ref(overlap_sizes,i) > 0) {
      if(overlap) {
        r = ivec_ref(overlap_sizes,i);
      } else {
        r = track_size(track_array_ref(res,ivec_ref(best_ind,i)));
      }
    }

    dym_set(themat,r,c,dym_ref(themat,r,c)+1.0);
  }

  /* Normalize the matrix columns. */
  for(j=0;j<dym_cols(themat);j++) {
    count = 0.0;

    for(i=0;i<dym_rows(themat);i++) { count += dym_ref(themat,i,j); }
    if(count < 1.0) { count = 1.0; }
    for(i=0;i<dym_rows(themat);i++) { dym_set(themat,i,j,dym_ref(themat,i,j)/count); }
  }

  free_ivec_array(true_tracks);
  free_ivec(overlap_sizes);
  free_ivec(best_ind);

  return themat;
}



double roc_percent_found(ivec* roc, ivec* true_groups) {
  int G = ivec_max(true_groups)+1;
  return ((double)ivec_sum(roc)) / ((double)G);
}


double roc_percent_correct(ivec* roc) {
  return ((double)ivec_sum(roc)) / ((double)ivec_size(roc));
}


double roc_aac(ivec* roc, int stop) {
  double sum       = 0.0;
  double num_wrong = 0.0;
  int num_correct  = 0;
  int i;

  for(i=0;(i<ivec_size(roc))&&(num_correct < stop);i++) {
    if(ivec_ref(roc,i) == 1) {
      num_correct++;
    } else {
      num_wrong += 1.0;
    }
    sum += num_wrong;
  }

  return sum;
}


/* Compute the number of found tracks that are exact matches of the */
/* true_groups. */
int compute_exact_matches(track_array* res, ivec* true_groups,
                          double* percent_correct, double* percent_found) {
  int num_groups = ivec_max(true_groups) + 1;
  int num_obs = ivec_size(true_groups);
  int num_tracks = track_array_size(res);
  int num_matches = 0;
  int i, j, k;

  /* First compute the true group sizes, skipping the "noise group". */
  ivec* true_track_sizes = mk_zero_ivec(num_groups);
  for (i = 0; i < num_obs; i++) {
    int group = ivec_ref(true_groups, i);
    if (group >= 0) {
      ivec_increment(true_track_sizes, group, 1);
    }
  }

  /* Now compute which groups we saw. */
  ivec* group_seen = mk_zero_ivec(num_groups+1);
  for (i = 0; i < num_tracks; i++) {
    ivec* current_track_obs = track_individs(track_array_ref(res, i));
    int current_size = ivec_size(current_track_obs);

    int group = ivec_ref(true_groups, ivec_ref(current_track_obs, 0));
    if (group < 0) {
      continue;      /* Noise group: Already NOT an exact match. */
    }

    /* Confirm that this track is from a single group and has no repeat obs. */
    /* We do the double FOR loop for now, but can speed this up if needed.   */
    bool clean = TRUE;
    for (j = 0; clean && j < current_size; j++) {
      bool found_double = FALSE;
      for (k = j+1; (k < current_size) && !found_double; k++) {
        found_double = (ivec_ref(current_track_obs, j) ==
                        ivec_ref(current_track_obs, k));
      }

      int second_group = ivec_ref(true_groups, ivec_ref(current_track_obs, j));
      clean = (second_group == group) && !found_double;
    }
    bool exact_match = clean && 
                       (current_size == ivec_ref(true_track_sizes, group));

    /* Mark the group as matched. */
    if (exact_match && (ivec_ref(group_seen, group) == 0)) {
      ivec_set(group_seen, group, 1);
      num_matches++;
    }
  }

  /* Compute the percentage statistics. */
  if (percent_correct != NULL) {
    *percent_correct = ((double)num_matches) / ((double)num_tracks);
  }
  if (percent_found != NULL) {
    *percent_found = ((double)num_matches) / ((double)num_groups);
  }

  /* Free the used memory. */
  free_ivec(true_track_sizes);
  free_ivec(group_seen);

  return num_matches;
}
