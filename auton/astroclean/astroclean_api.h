/*
File:        astroclean_api.h
Author:      J. Kubica
Created:     Tue, January 17, 2006
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

#ifndef ASTRO_CLEAN_API_H
#define ASTRO_CLEAN_API_H

#include <stdio.h>
#include <stdlib.h>

typedef void *AstroCleanStateHandle;

/* ----------------------------------------------------------------- */
/* --- Initialization Functions ------------------------------------ */
/* ----------------------------------------------------------------- */

/* Initialize the AstroClean engine and return an opaque handle */
/* that contains state info.                                    */ 
int AstroClean_Init(AstroCleanStateHandle* state,
  int verbosity,      /* 0 => no output, 1 => normal, 2 => verbose/debugging */
  FILE *log_fp        /* use as way to pass file descriptor in for output */
);


int AstroClean_AddDetection(AstroCleanStateHandle* st,
  double ra,            /* Right Ascension (in hours) */
  double dec,           /* Declination (in degrees)   */
  double time,          /* Time (in MJD)              */
  double brightness     /* Brightness                 */
);


/* Release all data structures used by AstroClean. */
int AstroClean_Free(AstroCleanStateHandle* st);


/* ----------------------------------------------------------------- */
/* --- State Access Functions -------------------------------------- */
/* ----------------------------------------------------------------- */

/* Get the total number of detections. */
int AstroClean_Num_Detections(AstroCleanStateHandle* state);

/* Get the number of clean detections. */
int AstroClean_Num_Clean(AstroCleanStateHandle* st);

/* Get the number of noise detections. */
int AstroClean_Num_Noise(AstroCleanStateHandle* state);

/* Get the i-th clean detection's index in the data. */
int AstroClean_Clean_Ref(AstroCleanStateHandle* state, int i);

/* Get the i-th noise detection's index in the data. */
int AstroClean_Noise_Ref(AstroCleanStateHandle* state, int i);


/* ----------------------------------------------------------------- */
/* --- Filtering  Functions ---------------------------------------- */
/* ----------------------------------------------------------------- */


/* Go through all of the "clean" detections and filter on */
/* the detections with time in [L,H].  If L < 0 or H < 0  */
/* those bounds are ignored.                              */
int AstroClean_Time_Filter(AstroCleanStateHandle* state, 
  double L,      /* Time low bound (or -1.0)  */ 
  double H       /* Time high bound (or -1.0) */
);


/* Go through all of the "clean" detections and filter on */
/* brightness in [L,H].  If L < 0 or H < 0 those bounds   */
/* are ignored.                                           */
int AstroClean_Brightness_Filter(AstroCleanStateHandle* state, 
  double L,         /* Brightness low bound (or -1.0)  */
  double H          /* Brightness high bound (or -1.0) */
);


/* Go through all of the detections in "use" and flag any of the  */
/* detections with position ra in [rL,rH] and dec in [dL, dH]     */
/* If L < 0 or H < 0  those bounds are ignored (-100 for DEC).    */
int AstroClean_Extract_AbsRegion(AstroCleanStateHandle* state,
                                 double rL, double rH,
                                 double dL, double dH);


/* Remove all of the detections from all of the fields with */
/* density above "dense" (in detections per square degree). */
int AstroClean_Field_Density_Filter(AstroCleanStateHandle* state,
                                    double dense);

/* Go through each field keeping only the brightest detections. */
/* Keep enough detections so "dense" = the new field density    */
/* in detections per degree.                                    */
int AstroClean_Field_Relative_Density_Filter(AstroCleanStateHandle* state, 
                                             double dense);


/* Remove all detections that have >= "density" points */
/* in their localized regions (defined by "radius").   */
/* density -> Given in points per square degree        */
/* radius  -> Given in degrees                         */
int AstroClean_Pointwise_DensityFilter(AstroCleanStateHandle* state,
                                       double density, double radius);

/* Go through each field keeping only the brightest detections. */
/* Keep enough detections so "dense" = the new local density    */
/* in detections per degree.                                    */
int AstroClean_Pointwise_Relative_DensityFilter(AstroCleanStateHandle* state,
                                                double density, double radius);

/* Remove all detections that could lie along a line.       */
/* radius    = How far do we look (in degrees)              */
/* ang_tresh = How well do they need to fit a line (in deg) */
/* support   = The minimum level of support needed.         */
int AstroClean_Linear_Filter(AstroCleanStateHandle* state, double radius,
                             double ang_thresh, int support);


/* Check for all "nearby" detections (within "radius"), remove a point */
/* if there is a point with a higher brightness or a point with the    */
/* same brightness and a lower index.  Radius is given in degrees.     */
int AstroClean_Duplicate_Filter(AstroCleanStateHandle* state, double radius);


/* Remove all detections that are "close" to one another, e.g. stationaries.
 * Radius is given in degrees.     */
int AstroClean_Stationary_Filter(AstroCleanStateHandle* state, double radius);


/* Only keep detections within "radius" of the point ("ra", "dec") */
/* "radius", "ra", "dec", are all given in degrees.                */
int AstroClean_Extract_Region(AstroCleanStateHandle* state, double radius,
                              double ra, double dec);

/* ----------------------------------------------------------------- */
/* --- Clustering Functions ---------------------------------------- */
/* ----------------------------------------------------------------- */

int AstroClean_Cluster_Clean(AstroCleanStateHandle* state,
                             double prox_thresh,
                             int cluster_size_thresh);

#endif
