/* File:        detectprox.h
   Author:      J. Kubica
   Created:     Sun, August 7 2005
   Description: Header for the detection proximity interface.

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

#ifndef ASTRO_DETECT_PROX_H
#define ASTRO_DETECT_PROX_H

#include <stdio.h>
#include <stdlib.h>

typedef void *DetectionProximityStateHandle;

/* Initialize the DetectionProximity engine and return an opaque handle
   that contains state info. */
int DetectionProximity_Init(DetectionProximityStateHandle *ophp,
   int verbosity,      /* 0 => no output, 1 => normal, 2 => verbose/debugging */
   FILE *log_fp        /* use as way to pass file descriptor in for debugging output */
);  /* initialize all structures for OP run */



/* Add a data detection to the tree.  Return the internal DetectionProximity */
/* number for that orbit. */
int DetectionProximity_AddDataDetection(DetectionProximityStateHandle fph,
    double ra,            /* Right Ascension (in hours) */
    double dec,           /* Declination (in degrees)   */
    double time,          /* Time (in MJD)              */
    double brightness     /* Brightness                 */
);



/* Add a query detection to the query set.  Return the internal DetectionProximity */
/* number for that orbit. */
int DetectionProximity_AddQueryDetection(DetectionProximityStateHandle fph,
    double ra,            /* Right Ascension (in hours) */
    double dec,           /* Declination (in degrees)   */
    double time,          /* Time (in MJD)              */
    double brightness,    /* Brightness                 */
    double dist_thresh,   /* Query's distance threshold (in degrees) */
    double bright_thresh, /* Query's brightness threshold            */
    double time_thresh    /* Query's time threshold                  */
);



/* Process the tree. */
int DetectionProximity_Run(DetectionProximityStateHandle fph);


/* Fetch a result from processing - determine the number of "nearby" data */
/* orbits for query orbit number "query_num"                              */
/* Note: query_num is the index returned by DetectionProximity_AddQueryDetection  */ 
int DetectionProximity_Num_Matches(DetectionProximityStateHandle fph,
    int query_num
);


/* Fetch a result from processing - return the "match_num"th matching       */
/* orbit for query "query_num".  This function can be called N times        */
/* for each query_num where: N = DetectionProximity_Num_Matches(fph,query_num)  */
/* Returns -1 on an error and the corresponding data orbit index otherwise. */
int DetectionProximity_Get_Match(DetectionProximityStateHandle fph,
    int query_num,
    int match_num
);


/* Release all data structures used by DetectionProximity. */
int DetectionProximity_Free(DetectionProximityStateHandle fph);

#endif
