/*
File:        astroclean.h
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

#ifndef ASTRO_CLEAN_H
#define ASTRO_CLEAN_H

#include "obs.h"
#include "rdt_tree.h"

typedef struct astroclean_state {

  /* The data/state */
  simple_obs_array*   obs;     /* All of the detections.               */
  ivec*               clean;   /* The indices of the clean detections. */
  ivec*               noise;   /* The indices of the noise detections. */

  /* Running Options: */
  int verbosity;      /* 0 => no output, 1 => normal, 2 => verbose/debugging */
  FILE *log_fp;       /* use as way to pass file descriptor in for output */

} astroclean_state;


/* ---------------------------------------------------------------------- */
/* --- Helper Functions ------------------------------------------------- */
/* ---------------------------------------------------------------------- */

void astroclean_state_update_clean(astroclean_state* st, ivec* nu_clean);

ivec_array* mk_astroclean_time_to_inds(simple_obs_array* obs, dyv* times, 
                                       ivec* use);

void astroclean_obs_bounds(simple_obs_array* obs, ivec* use,
                           double* dLO, double* dHI,
                           double* rLO, double* rHI);

/* ---------------------------------------------------------------------- */
/* --- Functions for filtering detections on time ----------------------- */
/* ---------------------------------------------------------------------- */


/* Go through all of the detections in "use" and flag any of     */
/* the detections with time in [L,H].  If L < 0 or H < 0         */
/* those bounds are ignored.  If "use" == NULL then all          */
/* detections are used.                                          */
ivec* mk_astroclean_time_filter(simple_obs_array* obs, ivec* use,
                                double L, double H);

/* ---------------------------------------------------------------------- */
/* --- Functions for filtering detections on brightness ----------------- */
/* ---------------------------------------------------------------------- */

/* Go through all of the detections in "use" and flag any of     */
/* the detections with brightness in [L,H].  If L < 0 or H < 0   */
/* those bounds are ignored.  If "use" == NULL then all          */
/* detections are used.                                          */
ivec* mk_astroclean_brightness_filter(simple_obs_array* obs, ivec* use,
                                      double L, double H);


#endif
