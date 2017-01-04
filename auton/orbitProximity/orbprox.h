/* File:        orbprox.h
   Author:      J. Kubica
   Created:     Tue, April 19 2005
   Description: Header for the orbit proximity interface.

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

#ifndef ASTRO_ORBIT_PROX_H
#define ASTRO_ORBIT_PROX_H

#include <stdio.h>
#include <stdlib.h>

typedef void *OrbitProximityStateHandle;

/* Initialize the OrbitProximity engine and return an opaque handle
   that contains state info. */
int OrbitProximity_Init(OrbitProximityStateHandle *ophp,
   double q_thresh,    /* Perihelion Distance threshold (in AU) */
   double e_thresh,    /* Eccentricity threshold */
   double i_thresh,    /* Inclination threshold (in degrees) */
   double w_thresh,    /* Argument of the perihelion threshold (in degrees) */
   double O_thresh,    /* Longitude of ascending node threshold (in degrees) */
   double t_thresh,    /* Time of perihelion passage threshold (in days) */
   int verbosity,      /* 0 => no output, 1 => normal, 2 => debugging */
   FILE *log_fp        /* use as way to pass file descriptor in for output */
);  /* initialize all structures for OP run */



/* Add a data orbit to the tree.  Return the internal OrbitProximity */
/* number for that orbit. */
int OrbitProximity_AddDataOrbit(OrbitProximityStateHandle fph,
    double q,             /* Perihelion Distance (in AU)              */
    double e,             /* Eccentricity                             */
    double i,             /* Inclination (in degrees)                 */
    double w,             /* Argument of the perihelion (in degrees)  */
    double O,             /* Longitude of ascending node (in degrees) */
    double t0,            /* Time of perihelion passage (MJD)         */
    double equinox        /* Equinox of orbit (in MJD)                */
);



/* Add a query orbit to the query set.  Return the internal OrbitProximity */
/* number for that orbit. */
int OrbitProximity_AddQueryOrbit(OrbitProximityStateHandle fph,
     double q,             /* Perihelion Distance (in AU)              */
     double e,             /* Eccentricity                             */
     double i,             /* Inclination (in degrees)                 */
     double w,             /* Argument of the perihelion (in degrees)  */
     double O,             /* Longitude of ascending node (in degrees) */
     double t0,            /* Time of perihelion passage (MJD)         */
     double equinox        /* Equinox of orbit (in MJD)                */
);




/* Process the tree. */
int OrbitProximity_Run(OrbitProximityStateHandle fph);


/* Fetch a result from processing - determine the number of "nearby" data */
/* orbits for query orbit number "query_num"                              */
/* Note: query_num is the index returned by OrbitProximity_AddQueryOrbit  */ 
int OrbitProximity_Num_Matches(OrbitProximityStateHandle fph,
    int query_num
);


/* Fetch a result from processing - return the "match_num"th matching       */
/* orbit for query "query_num".  This function can be called N times        */
/* for each query_num where: N = OrbitProximity_Num_Matches(fph,query_num)  */
/* Returns -1 on an error and the corresponding data orbit index otherwise. */
int OrbitProximity_Get_Match(OrbitProximityStateHandle fph,
    int query_num,
    int match_num
);


/* Release all data structures used by OrbitProximity. */
int OrbitProximity_Free(OrbitProximityStateHandle fph);

#endif
