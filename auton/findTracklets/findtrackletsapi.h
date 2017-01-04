/* File:        findtrackletsapi.h
   Author:      J. Kubica
   Created:     Mon, Oct. 10, 2005
   Description: Header for the findtracklets.

   Copyright 2005, The Auton Lab, CMU                             

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

#ifndef FINDTRACKLETS_API_H
#define FINDTRACKLETS_API_H

#include <stdio.h>
#include <stdlib.h>

#define TRACKLET_VERSION 2
#define TRACKLET_RELEASE 0
#define TRACKLET_UPDATE  5

#define FT_DEF_THRESH       0.0003
#define FT_DEF_ATHRESH      180.0
#define FT_DEF_MAXLERR      0.00015
#define FT_DEF_MINV         0.0
#define FT_DEF_MAXV         0.5
#define FT_DEF_MAXT         0.5
#define FT_DEF_ETIME        30.0
#define FT_DEF_MINOBS       2
#define FT_DEF_MAXOBS       100

typedef void *FindTrackletsStateHandle;

/* Initialize the find tracklets program (allocate memory, etc.) */
/* and return an opaque handle that contains state info.         */
/* Everything is created using the default parameter values.     */
int FindTracklets_Init(FindTrackletsStateHandle* state,
                       int verbosity,  /* 0 => no output, 1 => normal, */
                                       /* 2 => verbose/debugging */
                       FILE *log_fp    /* file descriptor in for debugging output */
                      );


/* Set the maximum angular threshold (in degrees). */
int FindTracklets_set_athresh(FindTrackletsStateHandle* state, double nu_val);

/* Set the maximum fit threshold (in degrees). */
int FindTracklets_set_thresh(FindTrackletsStateHandle* state, double nu_val);

/* Set the maximum length error (in degrees). */
int FindTracklets_set_maxLerr(FindTrackletsStateHandle* state, double nu_val);

/* Set the length of time of exposure (for elongation) in seconds */
int FindTracklets_set_etime(FindTrackletsStateHandle* state, double nu_val);

/* Set the maximum tracklet velocity (in deg/day) */
int FindTracklets_set_minv(FindTrackletsStateHandle* state, double nu_val);

/* Set the maximum tracklet velocity (in deg/day) */
int FindTracklets_set_maxv(FindTrackletsStateHandle* state, double nu_val);

/* Set the maximum tracklet time span (in days) */
int FindTracklets_set_maxt(FindTrackletsStateHandle* state, double nu_val);

/* Set the minimum number of required detections. */
int FindTracklets_set_minobs(FindTrackletsStateHandle* state, int nu_val);

/* Set the maximum number of detections. */
int FindTracklets_set_maxobs(FindTrackletsStateHandle* state, int nu_val);

/* Use evaluation mode? (0=NO, 1=YES) */
int FindTracklets_set_eval(FindTrackletsStateHandle* state, int val);

/* Use greedy mode? (0=NO, 1=YES) */
int FindTracklets_set_greedy(FindTrackletsStateHandle* state, int val);

/* Use PHT (Partial Hough Transform)? (0=NO, 1=YES) */
int FindTracklets_set_use_pht(FindTrackletsStateHandle* state, int val);

/* Add a data detection to the data set.    */
/* Return the internal FindTracklets number */
/* for that detections.                     */
int FindTracklets_AddDataDetection(FindTrackletsStateHandle fph,
          double ra,            /* Right Ascension (in hours) */
          double dec,           /* Declination (in degrees)   */
          double time,          /* Time (in MJD)              */
          double brightness     /* Brightness                 */
          );


/* Add a data detection to the data set (with elongation information). */
/* Return the internal FindTracklets number for that detections.       */
int FindTracklets_AddDataDetection_elong(FindTrackletsStateHandle fph,
          double ra,          /* Right Ascension (in hours) */
          double dec,         /* Declination (in degrees)   */
          double time,        /* Time (in MJD)              */
          double brightness,  /* Brightness                 */
          double angle,       /* Elongation Angle (in deg)  */
          double length,      /* Elongation Length (in deg) */
          double exp_time     /* Exposure time (in sec)     */
          );


/* Add the ground truth tracklet to a detection.  Used for evaluation. */
/* Note the detection is indexed by its internal ID.                   */
int FindTracklets_AddTruth(FindTrackletsStateHandle fph, 
                                 int detection_index,
                                 int tracklet_num
                                 );


/* Add the ground truth name to a detection.  Used for evaluation. */
/* Note the detection is indexed by its internal ID.               */
int FindTracklets_AddTrackletName(FindTrackletsStateHandle fph, 
                                 int detection_index,
                                 char* name
                                 );


/* Process the tree. */
int FindTracklets_Run(FindTrackletsStateHandle fph);


/* Fetch the total number of result tracklets. */
int FindTracklets_Num_Tracklets(FindTrackletsStateHandle fph);


/* Fetch the number of detections in a given tracklet. */
int FindTracklets_Num_Detections(FindTrackletsStateHandle fph,
                                 int tracklet_number
                                 );


/* Fetch a given detection id from a given tracklet.            */
/* This function can be called N times for each tracklet where: */
/*    N = FindTracklets_Num_Detections(fph,tracklet_num)        */
/* Returns -1 on an error and the corresponding FindTracklets   */
/* detection ID otherwise.                                      */
int FindTracklets_Get_Match(FindTrackletsStateHandle fph,
                            int tracklet_number,
                            int match_num
                            );


/* Release all data structures used by DetectionProximity. */
int FindTracklets_Free(FindTrackletsStateHandle fph);

#endif
