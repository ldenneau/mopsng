/*
File:        astroclean.c
Author:      J. Kubica
Created:     Tue, January 17 2005
Description: A collection of functions for cleaning astrodata.

Copyright 2006, The Auton Lab, CMU
                                                                  
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

#include "astroclean.h"
#include "astroclean_api.h"

/* Returns a dyv with [ra_min ra_max dec_min dec_max] */
dyv* ac_compute_obs_bounds2(simple_obs_array* obs) {
  simple_obs* X;
  dyv* res = mk_zero_dyv(4);
  double rMin = 0.0;
  double rMax = 0.0;
  double dMin = 0.0;
  double dMax = 0.0;
  int i;

  /* Go through each point in "inds" */
  for(i=0;i<simple_obs_array_size(obs);i++) {
    X = simple_obs_array_ref(obs,i);
    if((i==0)||(rMin > simple_obs_RA(X))) { rMin = simple_obs_RA(X); }
    if((i==0)||(rMax < simple_obs_RA(X))) { rMax = simple_obs_RA(X); }
    if((i==0)||(dMin > simple_obs_DEC(X))) { dMin = simple_obs_DEC(X); }
    if((i==0)||(dMax < simple_obs_DEC(X))) { dMax = simple_obs_DEC(X); }
  }

  /* Set the results. */
  dyv_set(res,0,rMin);
  dyv_set(res,1,rMax);
  dyv_set(res,2,dMin);
  dyv_set(res,3,dMax);

  return res;
}


/* ----------------------------------------------------------------- */
/* --- Initialization Functions ------------------------------------ */
/* ----------------------------------------------------------------- */

/* Initialize the AstroClean engine and return an opaque handle */
/* that contains state info.                                    */
int AstroClean_Init(AstroCleanStateHandle* state,
  int verbosity,      /* 0 => no output, 1 => normal, 2 => debugging */
  FILE *log_fp        /* use as way to pass file descriptor in for output */
                    ) {
  astroclean_state* st;
  int result = 0;

  if((verbosity > 0)&&(log_fp != NULL)) {
    fprintf(log_fp,"Initializing AstroClean State.\n");
  }

  /* Allocate memory for the state */
  st = AM_MALLOC(astroclean_state);
  if(st != NULL) {
    st->obs = mk_empty_simple_obs_array(128);
    st->clean = mk_ivec(0);
    st->noise = mk_ivec(0);

    /* Set the known values. */
    st->verbosity = verbosity;
    st->log_fp    = log_fp;

    state[0] = (AstroCleanStateHandle)st;
  } else {
    result = 1;
  }

  return result;
}


int AstroClean_AddDetection(AstroCleanStateHandle* st,
                            double ra,         /* Right Ascension (in hours) */
                            double dec,        /* Declination (in degrees)   */
                            double time,       /* Time (in MJD)              */
                            double brightness  /* Brightness                 */
                            ) {
  astroclean_state* state = (astroclean_state*)st;
  int old_maxsize = simple_obs_array_max_size(state->obs);
  int old_size    = simple_obs_array_size(state->obs);
  simple_obs* o;

  o = mk_simple_obs_simplest(old_size,time,ra,dec,brightness);

  /* Add the detection and it's index into clean. */
  simple_obs_array_add(state->obs,o);  
  add_to_ivec(state->clean,old_size);

  /* Check if we doubled the array size. */
  if(old_maxsize < simple_obs_array_max_size(state->obs)) {
    if((state->verbosity > 1)&&(state->log_fp != NULL)) {
      fprintf(state->log_fp,"Ran out of space for detections... doubling array size to %i.\n",
              simple_obs_array_max_size(state->obs));
    }
  }

  free_simple_obs(o);

  return 0;
}


/* Release all data structures used by AstroClean. */
int AstroClean_Free(AstroCleanStateHandle* st) {
  astroclean_state* state = (astroclean_state*)st;
  int result = 0;

  if(st != NULL) {
    if((state->verbosity > 0)&&(state->log_fp != NULL)) {
      fprintf(state->log_fp,"Freeing AstroClean State.\n");
    }
  
    free_simple_obs_array(state->obs);
    free_ivec(state->clean);
    free_ivec(state->noise);

    AM_FREE(state,astroclean_state);
  } else {
    result = 1;
  }

  return result;
}


/* ----------------------------------------------------------------- */
/* --- State Access Functions -------------------------------------- */
/* ----------------------------------------------------------------- */

/* Get the total number of detections. */
int AstroClean_Num_Detections(AstroCleanStateHandle* state) {
  astroclean_state* st = (astroclean_state*)state;  
  return simple_obs_array_size(st->obs);
}

/* Get the number of clean detections. */
int AstroClean_Num_Clean(AstroCleanStateHandle* state) {
  astroclean_state* st = (astroclean_state*)state;  
  return ivec_size(st->clean);
}

/* Get the number of noise detections. */
int AstroClean_Num_Noise(AstroCleanStateHandle* state) {
  astroclean_state* st = (astroclean_state*)state;  
  return ivec_size(st->noise);
}

/* Get the i-th clean detection's index in the data. */
int AstroClean_Clean_Ref(AstroCleanStateHandle* state, int i) {
  astroclean_state* st = (astroclean_state*)state;  
  return ivec_ref(st->clean,i);
}

/* Get the i-th noise detection's index in the data. */
int AstroClean_Noise_Ref(AstroCleanStateHandle* state, int i) {
  astroclean_state* st = (astroclean_state*)state;  
  return ivec_ref(st->noise,i);
}


/* ---------------------------------------------------------------------- */
/* --- Helper Functions ------------------------------------------------- */
/* ---------------------------------------------------------------------- */

/* nu_clean indicates the observations that are still  */
/* considered clean after the latest round.  Use these */
/* to update the list of clean and noise points.       */
void astroclean_state_update_clean(astroclean_state* st, 
                                   ivec* nu_clean) {
  ivec* nu_all_noise;
  ivec* nu_noise;

  my_assert(is_sivec(nu_clean));

  /* Compute the new clean and noise values. */
  nu_noise     = mk_sivec_difference(st->clean,nu_clean);
  nu_all_noise = mk_sivec_union(st->noise,nu_noise);
  free_ivec(nu_noise);
  
  /* Free the previous lists. */
  free_ivec(st->clean);
  free_ivec(st->noise);

  /* Save the new lists. */
  st->clean = mk_copy_ivec(nu_clean);
  st->noise = nu_all_noise;
}


/* For each time in 'times' return all points at that time */
/* that also appear in the 'use' list.                     */
ivec_array* mk_astroclean_time_to_inds(simple_obs_array* obs, 
                                       dyv* times, ivec* use) {
  int T = dyv_size(times);
  int N = ivec_size(use);
  ivec_array* res;
  ivec* count = mk_zero_ivec(T);
  double curr_t;
  int i, tmatch;

  /* Count the number of times each time has occurred */
  for(i=0;i<N;i++) {
    tmatch = 0;
    curr_t = simple_obs_time(simple_obs_array_ref(obs,ivec_ref(use,i)));
    tmatch = find_index_in_dyv(times,curr_t,0.01);
    if(tmatch >= 0) {
      ivec_increment(count,tmatch,1);
    }
  }

  /* create the result vector */
  res = mk_ivec_array_of_given_lengths(count);

  /* Fill the ivecs */
  for(i=N-1;i>=0;i--) {
    tmatch = 0;
    curr_t = simple_obs_time(simple_obs_array_ref(obs,ivec_ref(use,i)));
    tmatch = find_index_in_dyv(times,curr_t,0.01);
    if(tmatch >= 0) {
      ivec_array_ref_set(res,tmatch,ivec_ref(count,tmatch)-1,ivec_ref(use,i));
      ivec_increment(count,tmatch,-1);
    }
  }

  free_ivec(count);

  return res;
}


/* Compute the bounds of a subset of points. */
void astroclean_obs_bounds(simple_obs_array* obs, ivec* use,
                           double* dLO, double* dHI, 
                           double* rLO, double* rHI) {
  simple_obs* X;
  double r0 = 0.0;
  double r, d;
  int i;

  /* Set the initial values. */
  if(ivec_size(use) > 0) {
    X = simple_obs_array_ref(obs,ivec_ref(use,0));

    r0     = simple_obs_RA(X);
    rLO[0] = simple_obs_RA(X);
    rHI[0] = simple_obs_RA(X);

    dLO[0] = simple_obs_DEC(X);
    dHI[0] = simple_obs_DEC(X);
  } else {
    rLO[0] = 0.0;
    rHI[0] = 0.0;
    dLO[0] = 0.0;
    dHI[0] = 0.0;
  }

  /* Set the remaining values (relative to (r0, d0) */
  /* in order to safely handle wrap-around).        */
  for(i=1;i<ivec_size(use);i++) {
    X = simple_obs_array_ref(obs,ivec_ref(use,i));
    r = simple_obs_RA(X);
    d = simple_obs_DEC(X);

    /* Watch out for wrap around in RA */
    while(r - r0 >  12.0) { r -= 24.0; }
    while(r - r0 < -12.0) { r += 24.0; }

    if(rLO[0] > r) { rLO[0] = r; }
    if(rHI[0] < r) { rHI[0] = r; }

    if(dLO[0] > d) { dLO[0] = d; }
    if(dHI[0] < d) { dHI[0] = d; }
  }
}



/* ---------------------------------------------------------------------- */
/* --- Functions for filtering detections on time ----------------------- */
/* ---------------------------------------------------------------------- */


/* Go through all of the detections in "use" and flag any of     */
/* the detections with time in [L,H].  If L < 0 or H < 0         */
/* those bounds are ignored.  If "use" == NULL then all          */
/* detections are used.                                          */
ivec* mk_astroclean_time_filter(simple_obs_array* obs, ivec* use, 
                                double L, double H) {
  ivec* res = mk_ivec(0);
  double t;
  int N;
  int i, ind;

  if(use == NULL) {

    /* Just iterate through all points checking the time bounds. */
    N = simple_obs_array_size(obs);
    for(i=0;i<N;i++) {
      t = simple_obs_time(simple_obs_array_ref(obs,i));
      if(((t <= L)||(L < 0.0))&&((t >= H)||(H < 0.0))) {
        add_to_ivec(res,i);
      }
    }

  } else {

    /* Just iterate through all points checking the time bounds. */
    N = ivec_size(use);
    for(i=0;i<N;i++) {
      ind = ivec_ref(use,i);
      t   = simple_obs_time(simple_obs_array_ref(obs,ind));
      if(((t >= L)||(L < 0.0))&&((t <= H)||(H < 0.0))) {
        add_to_ivec(res,ind);
      }
    }

  }

  return res;
}


/* Go through all of the "clean" detections and filter on */
/* the detections with time in [L,H].  If L < 0 or H < 0  */
/* those bounds are ignored.                              */
int AstroClean_Time_Filter(AstroCleanStateHandle* state, double L, double H) {
  astroclean_state* st = (astroclean_state*)state;
  ivec* nu_clean;

  if((st->verbosity > 0)&&(st->log_fp != NULL)) {
    fprintf(st->log_fp,"Filtering detections on time [%f,%f].\n",L,H);
    fprintf(st->log_fp,"  Had %i detections\n",ivec_size(st->clean));
  }

  /* Compute the new clean and noise values. */
  nu_clean = mk_astroclean_time_filter(st->obs,st->clean,L,H);
  astroclean_state_update_clean(st,nu_clean); 
  free_ivec(nu_clean);

  if((st->verbosity > 0)&&(st->log_fp != NULL)) {
    fprintf(st->log_fp,"  Now have %i detections.\n",ivec_size(st->clean));
  }

  return 0;
}


/* ---------------------------------------------------------------------- */
/* --- Functions for filtering detections on brightness ----------------- */
/* ---------------------------------------------------------------------- */

/* Go through all of the detections in "use" and flag any of     */
/* the detections with brightness in [L,H].  If L < 0 or H < 0   */
/* those bounds are ignored.  If "use" == NULL then all          */
/* detections are used.                                          */
ivec* mk_astroclean_brightness_filter(simple_obs_array* obs, ivec* use, 
                                      double L, double H) {
  ivec* res = mk_ivec(0);
  double b;
  int N;
  int i, ind;

  if(use == NULL) {

    /* Just iterate through all points checking the brightness bounds. */
    N = simple_obs_array_size(obs);
    for(i=0;i<N;i++) {
      b = simple_obs_brightness(simple_obs_array_ref(obs,i));
      if(((b <= L)||(L < 0.0))&&((b >= H)||(H < 0.0))) {
        add_to_ivec(res,i);
      }
    }

  } else {

    /* Just iterate through all points checking the brightness bounds. */
    N = ivec_size(use);
    for(i=0;i<N;i++) {
      ind = ivec_ref(use,i);
      b   = simple_obs_brightness(simple_obs_array_ref(obs,ind));
      if(((b >= L)||(L < 0.0))&&((b <= H)||(H < 0.0))) {
        add_to_ivec(res,ind);
      }
    }

  }

  return res;
}


/* Go through all of the detections in "use" and flag any of     */
/* the detections with brightness in [L,H].  If L < 0 or H < 0   */
/* those bounds are ignored.  If "use" == NULL then all          */
/* detections are used.                                          */
int AstroClean_Brightness_Filter(AstroCleanStateHandle* state, 
                                 double L, double H) {
  astroclean_state* st = (astroclean_state*)state;
  ivec* nu_clean;

  if((st->verbosity > 0)&&(st->log_fp != NULL)) {
    fprintf(st->log_fp,"Filtering detections on brightness [%f,%f].\n",L,H);
    fprintf(st->log_fp,"  Had %i detections.\n",ivec_size(st->clean));
  }

  /* Compute the new clean and noise values. */
  nu_clean = mk_astroclean_brightness_filter(st->obs,st->clean,L,H);
  astroclean_state_update_clean(st,nu_clean); 
  free_ivec(nu_clean);

  if((st->verbosity > 0)&&(st->log_fp != NULL)) {
    fprintf(st->log_fp,"  Now have %i detections.\n",ivec_size(st->clean));
  }

  return 0;
}



/* ---------------------------------------------------------------------- */
/* --- Functions for filtering detections on position - ----------------- */
/* ---------------------------------------------------------------------- */

/* Go through all of the detections in "use" and flag any of the  */
/* detections with position ra in [rL,rH] and dec in [dL, dH]     */
/* If L < 0 or H < 0  those bounds are ignored.  If "use" == NULL */
/* then all detections are used.                                  */
ivec* mk_astroclean_absregion_filter(simple_obs_array* obs, ivec* use, 
                                     double rL, double rH,
                                     double dL, double dH) {
  ivec* res = mk_ivec(0);
  double r, d;
  int N;
  int i, ind;

  if(use == NULL) {

    /* Just iterate through all points checking the bounds. */
    N = simple_obs_array_size(obs);
    for(i=0;i<N;i++) {
      r = simple_obs_RA(simple_obs_array_ref(obs,i));
      d = simple_obs_DEC(simple_obs_array_ref(obs,i));
      if(((r >= rL)||(rL < 0.0))&&((r <= rH)||(rH < 0.0))) {
        if(((d >= dL)||(dL < -100.0))&&((d <= dH)||(dH < -100.0))) {
          add_to_ivec(res,i);
        }
      }
    }

  } else {

    /* Just iterate through all points checking the bounds. */
    N = ivec_size(use);
    for(i=0;i<N;i++) {
      ind = ivec_ref(use,i);
      r   = simple_obs_RA(simple_obs_array_ref(obs,ind));
      d   = simple_obs_DEC(simple_obs_array_ref(obs,ind));
      if(((r >= rL)||(rL < 0.0))&&((r <= rH)||(rH < 0.0))) {
        if(((d >= dL)||(dL < -100.0))&&((d <= dH)||(dH < -100.0))) {
          add_to_ivec(res,ind);
        }
      }
    }

  }

  return res;
}

/* Go through all of the detections in "use" and flag any of the  */
/* detections with position ra in [rL,rH] and dec in [dL, dH]     */
/* If L < 0 or H < 0  those bounds are ignored.                   */
int AstroClean_Extract_AbsRegion(AstroCleanStateHandle* state, 
                                 double rL, double rH,
                                 double dL, double dH) {
  astroclean_state* st = (astroclean_state*)state;
  ivec* nu_clean;

  if((st->verbosity > 0)&&(st->log_fp != NULL)) {
    fprintf(st->log_fp,"Filtering detections on position [%f,%f], [%f,%f].\n",
            rL,rH,dL,dH);
    fprintf(st->log_fp,"  Had %i detections.\n",ivec_size(st->clean));
  }

  /* Compute the new clean and noise values. */
  nu_clean = mk_astroclean_absregion_filter(st->obs,st->clean,rL,rH,dL,dH);
  astroclean_state_update_clean(st,nu_clean); 
  free_ivec(nu_clean);

  if((st->verbosity > 0)&&(st->log_fp != NULL)) {
    fprintf(st->log_fp,"  Now have %i detections.\n",ivec_size(st->clean));
  }

  return 0;
}




/* ---------------------------------------------------------------------- */
/* --- Functions for filtering detections on field density -------------- */
/* ---------------------------------------------------------------------- */


/* Remove all of the detections from all of the fields with  */
/* density above "dense" (in detections per square degree).  */
/* If progressive==true then remove the brightest ones until */
/* we reach the desired density.                             */
int AstroClean_Field_Density_Filter_Full(AstroCleanStateHandle* state, 
                                         double dense, 
                                         bool progressive) {
  astroclean_state* st = (astroclean_state*)state;  
  simple_obs* X;
  ivec_array* t_to_inds;
  ivec* valid = mk_ivec(0);
  ivec* svalid;
  ivec* sinds;
  dyv* bright;
  dyv* times;
  double rLO = 0.0;
  double rHI = 0.0;
  double dLO = 0.0;
  double dHI = 0.0;
  double area = 0.0;
  double N;
  int t, i, Ni;

  if((st->verbosity > 0)&&(st->log_fp != NULL)) {
    fprintf(st->log_fp,"Filtering detections on density %f (relative=%i).\n",
            dense,progressive);
    fprintf(st->log_fp,"  Had %i detections.\n",ivec_size(st->clean));
  }

  /* Get the detection times. */
  times     = mk_simple_obs_plate_times(st->obs,1e-10);
  t_to_inds = mk_astroclean_time_to_inds(st->obs,times,st->clean);

  /* Check each field (separately) */
  for(t=0;t<ivec_array_size(t_to_inds);t++) {
    astroclean_obs_bounds(st->obs, ivec_array_ref(t_to_inds,t), &dLO,
                          &dHI, &rLO, &rHI);

    /* Compute the approximate area of the field. */    
    area = (dHI - dLO) * (15.0*(rHI-rLO)*cos(0.5*(dLO+dHI)*DEG_TO_RAD));
    if(area < 1e-50) { area = 1e-50; }
    N = (double)(ivec_array_ref_size(t_to_inds, t));

    if((st->verbosity > 1)&&(st->log_fp != NULL)) {
      fprintf(st->log_fp,
              "Field %i (t=%f) has area=%f, N=%i, and density=%f vs %f\n",
              t, dyv_ref(times, t), area, (int)N, N/area,dense);
    }

    if((N < 5.0)||(N/area < dense)) {
      append_to_ivec(valid,ivec_array_ref(t_to_inds,t));
    } else {
      if(progressive) {

        /* Log all of the brightnesses. */
        bright = mk_dyv(ivec_array_ref_size(t_to_inds,t));
        for(i=0;i<dyv_size(bright);i++) {
          X = simple_obs_array_ref(st->obs,ivec_array_ref_ref(t_to_inds,t,i));
          dyv_set(bright,i,simple_obs_brightness(X));
        }

        /* Sort the brightness */
        sinds = mk_ivec_sorted_dyv_indices(bright);        

        /* Compute the number to add... */
        Ni = (int)(dense * area) + 1;
        for(i=0;i<Ni;i++) {
          add_to_ivec(valid,ivec_array_ref_ref(t_to_inds,t,ivec_ref(sinds,i)));
        }

        if((st->verbosity > 1)&&(st->log_fp != NULL)) {
          fprintf(st->log_fp,"  Took the best %i\n",Ni);
        }

        /* Free the used memory. */
        free_ivec(sinds);
        free_dyv(bright);
      }
    }

  }

  /* Sort the list of valid detections. */
  svalid = mk_sivec_from_ivec(valid);
  
  /* Update the list of clean and noise points. */
  astroclean_state_update_clean(st,svalid);

  if((st->verbosity > 0)&&(st->log_fp != NULL)) {
    fprintf(st->log_fp,"  Now have %i detections.\n",ivec_size(st->clean));
  }

  /* Free the remaining memory. */
  free_ivec_array(t_to_inds);
  free_ivec(svalid);
  free_ivec(valid);
  free_dyv(times);

  return 0;
}


/* Remove all of the detections from all of the fields with */
/* density above "dense" (in detections per square degree). */
int AstroClean_Field_Density_Filter(AstroCleanStateHandle* state,
                                    double dense) {
  return AstroClean_Field_Density_Filter_Full(state,dense,FALSE);
}


/* Go through each field keeping only the brightest detections. */
/* Keep enough detections so "dense" = the new field density    */
/* in detections per degree.                                    */
int AstroClean_Field_Relative_Density_Filter(AstroCleanStateHandle* state, 
                                             double dense) {
  return AstroClean_Field_Density_Filter_Full(state,dense,TRUE);
}


/* ---------------------------------------------------------------------- */
/* --- Functions for filtering linear over-densities -------------------- */
/* ---------------------------------------------------------------------- */

/* Remove all detections that could lie along a line.       */
/* radius    = How far do we look (in degrees)              */
/* ang_tresh = How well do they need to fit a line (in deg) */ 
/* support   = The minimum level of support needed.         */
int AstroClean_Linear_Filter(AstroCleanStateHandle* state, double radius,
                             double ang_thresh, int support) {
  astroclean_state* st = (astroclean_state*)state;
  rdt_tree*         tr;
  simple_obs*       X;
  simple_obs*       Y;
  ivec_array* t_to_inds;
  ivec* valid = mk_ivec(0);
  ivec* svalid;
  ivec* neighs;
  ivec* atT;
  ivec* sinds;
  dyv* times;
  dyv* sangs;
  dyv* angs;
  dyv* dists;
  double dr, dd, ang, dist;
  double mindist = 0.0;
  double maxdist = 0.0;
  int t, i, j;
  int s, e;
  bool isgood;

  if((st->verbosity > 0)&&(st->log_fp != NULL)) {
    fprintf(st->log_fp,
            "Filtering linear noise (ang=%f, rad=%f, support=%i).\n",
            ang_thresh, radius, support);
    fprintf(st->log_fp, "  Had %i detections.\n", ivec_size(st->clean));
  }

  ang_thresh *= DEG_TO_RAD;

  /* Get the detection times. */
  times     = mk_simple_obs_plate_times(st->obs, 1e-10);
  t_to_inds = mk_astroclean_time_to_inds(st->obs, times, st->clean);
 
  /* Check each field (separately) */
  for(t=0; t<ivec_array_size(t_to_inds); t++) {
    atT = ivec_array_ref(t_to_inds, t);
    if(ivec_size(atT) > 0) {

      if((st->verbosity > 1)&&(st->log_fp != NULL)) {
        fprintf(st->log_fp,
                "  Constructing a tree (%i detections at time %f = step %i)\n",
                ivec_size(atT), dyv_ref(times,t), t);
      }
      tr = mk_rdt_tree(st->obs, atT, FALSE, 10);

      /* For each point at the given time, find all of its */
      /* neighbors (within radius and at the same times).  */
      for(i=0;i<ivec_size(atT);i++) {

        /* Grab the point and compute its (x,y,z) position */
        X  = simple_obs_array_ref(st->obs,ivec_ref(atT,i));
        isgood = TRUE;

        /* Get all of the neighbors (these will be the points */
        /* we check are along a line).                        */
        neighs = mk_rdt_tree_range_search(tr, st->obs, X,
                                          simple_obs_time(X)-1e-8,
                                          simple_obs_time(X)+1e-8,
                                          radius*DEG_TO_RAD);

        if(ivec_size(neighs) >= 0) {

          /* Compute the angle to each neighbor. */
          angs  = mk_empty_dyv(2*ivec_size(neighs));
          dists = mk_empty_dyv(2*ivec_size(neighs));
          for(j=0;j<ivec_size(neighs);j++) {
            Y  = simple_obs_array_ref(st->obs,ivec_ref(neighs,j));
            dist = angular_distance_RADEC(simple_obs_RA(X),simple_obs_RA(Y),
                                          simple_obs_DEC(X),simple_obs_DEC(Y));
 
            /* Compute the approximate angle, using unit sphere. */
            /* Ignore really close points (within 10.0 arcsecs)  */
            /* because they may have inaccurate angles.          */
            if(dist >= 5.0e-5) {
              dr  = 15.0*(simple_obs_RA(Y)-simple_obs_RA(X));
              dr  = dr*cos(0.5*(simple_obs_DEC(Y)+simple_obs_DEC(X))*DEG_TO_RAD);

              dd  = (simple_obs_DEC(Y)-simple_obs_DEC(X));
              ang = atan2(dd,dr);

              if(ang < 0.0) { ang += PI; }
            } else {
              /* Ignore REALLY close points. */
              ang = 1000.0 + 100.0 * (double)j;
            }
            
            /* Only add things to the dyv if needed.       */
            /* Don't bother adding and sorting FAR points. */
            if(ang < 10.0) {
              add_to_dyv(angs,ang);
              add_to_dyv(dists,dist);
 
              if(ang < 0.25*PI+ang_thresh) {
                add_to_dyv(angs,ang+PI);
                add_to_dyv(dists,dist);
              }
            }

          }

          /* Sort the angles and go through them looking for any */
          /* group of support or more points within ang_thresh   */
          sinds = mk_ivec_sorted_dyv_indices(angs);
          sangs = mk_dyv_sort(angs);

          e = 0;
          for(s=0;(s<dyv_size(sangs))&&(isgood);s++) {
            while((e<dyv_size(sangs))&&
                  (dyv_ref(sangs,s)+ang_thresh >= dyv_ref(sangs,e))) {
              e++;
            }
            e--;

            /* If we have enough support, mark the point as invalid. */
            if(support <= e-s+1) { 

              /* Find the min and max dists for points on this interval */
              for(j=s;j<=e;j++) { 
                dist = dyv_ref(dists,ivec_ref(sinds,j));
                if((j==s)||(dist < mindist)) { mindist = dist; }
                if((j==s)||(dist > maxdist)) { maxdist = dist; }
              }     

              /* Only consider it a line if things are reasonably */
              /* spread out on it (don't treat a nearby clump as  */
              /* evidence of a line).                             */
              if(mindist < 1.5*maxdist) {
                isgood=FALSE;

                printf("%i Neighbors\n",ivec_size(neighs));
                printf("%i) s=(%i,%f) e=(%i,%f)\n",
                       ivec_ref(atT,i),s,dyv_ref(sangs,s),
                       e,dyv_ref(sangs,e));

                if((st->verbosity > 1)&&(st->log_fp != NULL)) {
                  fprintf(st->log_fp, "  Detection %i (%f,%f)",
                          ivec_ref(atT,i),simple_obs_RA(X),
                          simple_obs_DEC(X));
                  fprintf(st->log_fp, " has support of %i (vs %i).\n",
                          e-s+1,support);
                }
              }
            }
          }

          /* Free all of the temp vectors for this point. */
          free_ivec(sinds);
          free_dyv(dists);
          free_dyv(sangs);
          free_dyv(angs);
        }

        /* If the point is still good, add it to the results */
        if(isgood) { add_to_ivec(valid, ivec_ref(atT,i)); }
 
        free_ivec(neighs);
      }

      free_rdt_tree(tr);
    }
  }

  /* Sort the list of valid detections. */
  svalid = mk_sivec_from_ivec(valid);

  /* Update the list of clean and noise points. */
  astroclean_state_update_clean(st,svalid);

  if((st->verbosity > 0)&&(st->log_fp != NULL)) {
    fprintf(st->log_fp,"  Now have %i detections.\n",ivec_size(st->clean));
  }

  /* Free the remaining memory. */
  free_ivec_array(t_to_inds);
  free_ivec(svalid);
  free_ivec(valid);
  free_dyv(times);

  return 0;
}



/* ---------------------------------------------------------------------- */
/* --- Functions for filtering duplicates ------------------------------- */
/* ---------------------------------------------------------------------- */

/* Check for all "nearby" detections (within "radius"), remove a point */
/* if there is a point with a higher brightness or a point with the    */
/* same brightness and a lower index.  Radius is given in degrees.     */
int AstroClean_Duplicate_Filter(AstroCleanStateHandle* state, double radius) {
  astroclean_state* st = (astroclean_state*)state;
  rdt_tree*         tr;
  simple_obs*       X;
  simple_obs*       Y;
  ivec_array* t_to_inds;
  ivec* valid = mk_ivec(0);
  ivec* svalid;
  ivec* neighs;
  ivec* atT;
  dyv* times;
  double b0, b;
  int t, i, j;
  bool isgood;

  if((st->verbosity > 0)&&(st->log_fp != NULL)) {
    fprintf(st->log_fp,"Filtering on proximity (radius=%f).\n",radius);
    fprintf(st->log_fp,"  Had %i detections.\n",ivec_size(st->clean));
  }

  radius *= DEG_TO_RAD;

  /* Get the detection times. */
  times     = mk_simple_obs_plate_times(st->obs,1e-10);
  t_to_inds = mk_astroclean_time_to_inds(st->obs,times,st->clean);

  /* Check each field (separately) */
  for(t=0;t<ivec_array_size(t_to_inds);t++) {
    atT = ivec_array_ref(t_to_inds,t);
    if(ivec_size(atT) > 0) {

      if((st->verbosity > 1)&&(st->log_fp != NULL)) {
        fprintf(st->log_fp,"  Building a tree (%i pts) at time %f (step %i)\n",
                ivec_size(atT),dyv_ref(times,t),t);
      }
      tr = mk_rdt_tree(st->obs,atT,FALSE,10);

      /* For each point at the given time, find all of its */
      /* neighbors (within radius and at the same times).  */
      for(i=0;i<ivec_size(atT);i++) {
        X  = simple_obs_array_ref(st->obs,ivec_ref(atT,i));
        b0 = simple_obs_brightness(X);

        neighs = mk_rdt_tree_range_search(tr,st->obs,X,simple_obs_time(X)-1e-8,
                                          simple_obs_time(X)+1e-8,radius);
        isgood = TRUE;

        /* Test against each neighbor. */
        for(j=0;j<ivec_size(neighs);j++) {
          Y = simple_obs_array_ref(st->obs,ivec_ref(neighs,j));
          b = simple_obs_brightness(Y);

          /* Filter if the neighbor is brighter (b < b0) */
          /* or the same brightness with a lower index.  */
          if((b < b0-1e-20) || 
             ((fabs(b-b0)<1e-20)&&(ivec_ref(neighs,j) < ivec_ref(atT,i)))) {
            isgood = FALSE;
          }
        }

        /* If the point is still good, add it to the results */
        if(isgood) { add_to_ivec(valid, ivec_ref(atT,i)); }

        free_ivec(neighs);
      }

      free_rdt_tree(tr);
    }
  }

  /* Sort the list of valid detections. */
  svalid = mk_sivec_from_ivec(valid);

  /* Update the list of clean and noise points. */
  astroclean_state_update_clean(st,svalid);

  if((st->verbosity > 0)&&(st->log_fp != NULL)) {
    fprintf(st->log_fp,"  Now have %i detections.\n",ivec_size(st->clean));
  }

  /* Free the remaining memory. */
  free_ivec_array(t_to_inds);
  free_ivec(svalid);
  free_ivec(valid);
  free_dyv(times);

  return 0;
}


/* ---------------------------------------------------------------------- */
/* --- Functions for removing stationaries ------------------------------ */
/* ---------------------------------------------------------------------- */

/* Remove all detections that are "close" to one another, e.g. stationaries.
 * Radius is given in degrees.     */
int AstroClean_Stationary_Filter(AstroCleanStateHandle* state, double radius) {
  astroclean_state* st = (astroclean_state*)state;
  rdt_tree*         tr;
  simple_obs*       X;
  simple_obs*       Y;
  ivec_array* t_to_inds;
  ivec* valid = mk_ivec(0);
  ivec* svalid;
  ivec* neighs;
  ivec* atT;
  dyv* times;
  double b0, b;
  int t, i, j;

  if((st->verbosity > 0)&&(st->log_fp != NULL)) {
    fprintf(st->log_fp,"Filtering on proximity (radius=%f).\n",radius);
    fprintf(st->log_fp,"  Had %i detections.\n",ivec_size(st->clean));
  }

  radius *= DEG_TO_RAD;

  /* Get the detection times. */
  times     = mk_simple_obs_plate_times(st->obs,1e-10);
  t_to_inds = mk_astroclean_time_to_inds(st->obs,times,st->clean);

  /* Check each field (separately) */
  for(t=0;t<ivec_array_size(t_to_inds);t++) {
    atT = ivec_array_ref(t_to_inds,t);
    if(ivec_size(atT) > 0) {

      if((st->verbosity > 1)&&(st->log_fp != NULL)) {
        fprintf(st->log_fp,"  Building a tree (%i pts) at time %f (step %i)\n",
                ivec_size(atT),dyv_ref(times,t),t);
      }
      tr = mk_rdt_tree(st->obs,atT,FALSE,10);

      /* For each point at the given time, find all of its */
      /* neighbors (within radius and at the same times).  */
      for(i=0;i<ivec_size(atT);i++) {
        X = simple_obs_array_ref(st->obs,ivec_ref(atT,i));

        neighs = mk_rdt_tree_range_search(tr,st->obs,X,simple_obs_time(X)-1e-8,
                                          simple_obs_time(X)+1e-8,radius);

        /* If there are no neighbors, then we accept the detection. */
        if (1 == ivec_size(neighs)) {
            add_to_ivec(valid, ivec_ref(atT,i));
        }

        free_ivec(neighs);
      }

      free_rdt_tree(tr);
    }
  }

  /* Sort the list of valid detections. */
  svalid = mk_sivec_from_ivec(valid);

  /* Update the list of clean and noise points. */
  astroclean_state_update_clean(st,svalid);

  if((st->verbosity > 0)&&(st->log_fp != NULL)) {
    fprintf(st->log_fp,"  Now have %i detections.\n",ivec_size(st->clean));
  }

  /* Free the remaining memory. */
  free_ivec_array(t_to_inds);
  free_ivec(svalid);
  free_ivec(valid);
  free_dyv(times);

  return 0;
}


/* ---------------------------------------------------------------------- */
/* --- Punch out a region of the sky ------------------------------------ */
/* ---------------------------------------------------------------------- */

/* Only keep detections within "radius" of the point ("ra", "dec") */
/* "radius", "ra", "dec", are all given in degrees.                */
int AstroClean_Extract_Region(AstroCleanStateHandle* state, double radius,
                              double ra, double dec) {
  astroclean_state* st = (astroclean_state*)state;
  simple_obs*       X;
  ivec*             valid = mk_ivec(0);
  double dist;
  int    i;

  if((st->verbosity > 0)&&(st->log_fp != NULL)) {
    fprintf(st->log_fp,"Extracting a region of %f degrees around (%f,%f).\n",
            radius,ra,dec);
  }

  /* Scale things correctly. */
  radius = radius * DEG_TO_RAD + 1e-20;
  ra /= 15.0;

  /* Go through each of the clean points and see if */
  /* they fall in the given region.                 */
  for(i=0;i<ivec_size(st->clean);i++) {
    X = simple_obs_array_ref(st->obs,ivec_ref(st->clean,i));
    dist = angular_distance_RADEC(simple_obs_RA(X),ra,
                                  simple_obs_DEC(X),dec);
    if(dist <= radius) {
      add_to_ivec(valid,ivec_ref(st->clean,i));
    }
  }

  /* Update the list of clean and noise points. */
  astroclean_state_update_clean(st,valid);

  if((st->verbosity > 0)&&(st->log_fp != NULL)) {
    fprintf(st->log_fp,"  Now have %i detections.\n",ivec_size(st->clean));
  }

  free_ivec(valid);

  return 0;
}



/* ---------------------------------------------------------------------- */
/* --- Functions for filtering local over-densities --------------------- */
/* ---------------------------------------------------------------------- */

/* Remove all detections that have >= "density" points */
/* in their localized regions (defined by "radius").   */
/* density -> Given in points per square degree        */
/* radius  -> Given in degrees                         */
int AstroClean_Pointwise_DensityFilter_Full(AstroCleanStateHandle* state, 
                                            double density, double radius,
                                            bool relative) {
  astroclean_state* st = (astroclean_state*)state;
  rdt_tree*         tr;
  simple_obs*       X;
  simple_obs*       Y;
  ivec_array* t_to_inds;
  ivec* valid = mk_ivec(0);
  ivec* svalid;
  ivec* sinds;
  ivec* atT;
  ivec* neighs;
  dyv* bright;
  dyv* times;
  double area;
  int t, i, j, N;

  if((st->verbosity > 0)&&(st->log_fp != NULL)) {
    fprintf(st->log_fp,"Filtering on local density (density=%f, radius=%f, relative=%i).\n",
            density,radius,relative);
    fprintf(st->log_fp,"  Had %i detections.\n",ivec_size(st->clean));
  }

  area = PI * radius * radius;
  radius *= DEG_TO_RAD;

  /* Get the detection times. */
  times     = mk_simple_obs_plate_times(st->obs,1e-10);
  t_to_inds = mk_astroclean_time_to_inds(st->obs,times,st->clean);
 
  /* Check each field (separately) */
  for(t=0;t<ivec_array_size(t_to_inds);t++) {
    atT = ivec_array_ref(t_to_inds,t);
    if(ivec_size(atT) > 0) {

      if((st->verbosity > 1)&&(st->log_fp != NULL)) {
        fprintf(st->log_fp,
               "  Building tree of the %i detections at time %f (step %i)\n",
                ivec_size(atT),dyv_ref(times,t),t);
      }
      tr = mk_rdt_tree(st->obs,atT,FALSE,10);

      /* For each point at the given time, find all of its */
      /* neighbors (within radius and at the same times).  */
      for(i=0;i<ivec_size(atT);i++) {

        /* Get all of the neighbors */
        X  = simple_obs_array_ref(st->obs,ivec_ref(atT,i));
        neighs = mk_rdt_tree_range_search(tr,st->obs,X,simple_obs_time(X)-1e-8,
                                          simple_obs_time(X)+1e-8,radius);
        N = ivec_size(neighs);

        /* Check the density (which is automatically fine */        
        /* if we only have the point itself).             */
        if( (((double)N)/area <= density) || (N==1) ) {
          
          /* If the density is good, add it straight-up. */
          add_to_ivec(valid,ivec_ref(atT,i));
        } else {
          if(relative) {

            /* Log all of the brightnesses. */
            bright = mk_dyv(N);
            for(j=0;j<N;j++) {
              Y = simple_obs_array_ref(st->obs,ivec_ref(neighs,j));
              dyv_set(bright,j,simple_obs_brightness(Y));
            }

            /* Sort the brightness */
            sinds = mk_ivec_sorted_dyv_indices(bright);

            /* Compute the number to add and check if the current  */
            /* point is bright enough (compared to its neighbors). */
            N = (int)(density * area)+1;
            if(N < dyv_size(bright)) {
              if(dyv_ref(bright,ivec_ref(sinds,N)) > simple_obs_brightness(X))
              {
                add_to_ivec(valid,ivec_ref(atT,i));
              }
            }

            free_ivec(sinds);
            free_dyv(bright);
          }
        }

        free_ivec(neighs);
      }

      free_rdt_tree(tr);
    }
  }

  /* Sort the list of valid detections. */
  svalid = mk_sivec_from_ivec(valid);

  /* Update the list of clean and noise points. */
  astroclean_state_update_clean(st,svalid);

  if((st->verbosity > 0)&&(st->log_fp != NULL)) {
    fprintf(st->log_fp,"  Now have %i detections.\n",ivec_size(st->clean));
  }

  /* Free the remaining memory. */
  free_ivec_array(t_to_inds);
  free_ivec(svalid);
  free_ivec(valid);
  free_dyv(times);

  return 0;
}


/* Remove all detections that have >= "density" points */
/* in their localized regions (defined by "radius").   */
/* density -> Given in points per square degree        */
/* radius  -> Given in degrees                         */
int AstroClean_Pointwise_DensityFilter(AstroCleanStateHandle* state,
                                       double density, double radius) {
  return AstroClean_Pointwise_DensityFilter_Full(state,density,radius,FALSE);
}

/* Go through each field keeping only the brightest detections. */
/* Keep enough detections so "dense" = the new local density    */
/* in detections per degree.                                    */
int AstroClean_Pointwise_Relative_DensityFilter(AstroCleanStateHandle* state,
                                                double density, double radius)
{
  return AstroClean_Pointwise_DensityFilter_Full(state,density,radius,TRUE);
}
