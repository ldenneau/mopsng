/* File:        orbprox.c
   Author:      J. Kubica
   Created:     Tue, April 19 2005
   Description: The orbit proximity algorithm.

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

#include "orbprox.h"
#include "orbit.h"
#include "orbit_tree.h"

#define ORBPROX_PTS_PER_LEAF 25

typedef struct orbprox_state {
  int num_orbits;
  int num_queries;

  /* Running Options: */
  int verbosity;      /* 0 => no output, 1 => normal, 2 => verbose/debugging */
  FILE *log_fp;       /* use as way to pass file descriptor in for output */

  /* Actual data arrays */
  orbit_array* data;        /* All of the data orbits  */
  orbit_array* queries;     /* All of the query orbits */

  /* The algorithm data structures */
  dyv* thresholds;          /* The proximity thresholds for each param */

  /* The result array (and tree data structure) */
  ivec_array* results;

} orbprox_state;


/* Initialize the OrbitProximity engine and return an opaque handle
   that contains state info. */
int OrbitProximity_Init(OrbitProximityStateHandle *ophp,
                        double q_thresh,    /* Perihelion dist thresh (AU)  */
                        double e_thresh,    /* Eccentricity threshold       */
                        double i_thresh,    /* Inclination thresh (degrees) */
                        double w_thresh,    /* w threshold (in degrees)     */
                        double O_thresh,    /* O threshold (in degrees)     */
                        double t_thresh,    /* T0 threshold (in days)       */
                        int verbosity,      /* 0 => no output, 1 => normal, */
                                            /* 2 => verbose/debugging       */
                        FILE *log_fp        /* file descriptor for output   */
                        ) {
  orbprox_state* state;

  if((verbosity > 0)&&(log_fp != NULL)) {
    fprintf(log_fp,"Initializing OrbitProximity State.\n");
  }

  state = AM_MALLOC(orbprox_state);
 
  /* Set known values. */
  state->num_orbits  = 0;
  state->num_queries = 0;
  state->verbosity   = verbosity;
  state->log_fp      = log_fp;

  /* Allocate space for the data and query orbits. */
  state->data    = mk_empty_orbit_array_sized(128);
  state->queries = mk_empty_orbit_array_sized(64);
  
  /* Fix the thresholds. */
  state->thresholds = mk_orb_tree_search_thresh(q_thresh, 
                                                e_thresh, 
                                                i_thresh*DEG_TO_RAD,
                                                O_thresh*DEG_TO_RAD, 
                                                w_thresh*DEG_TO_RAD, 
                                                t_thresh);

  if((verbosity > 0)&&(log_fp != NULL)) { 
    fprintf(log_fp,"Setting the following thresholds:\n");
    fprintf(log_fp," q  = %10f (AU)\n",q_thresh); 
    fprintf(log_fp," e  = %10f\n",e_thresh); 
    fprintf(log_fp," i  = %10f (degrees; %10f radians)\n",
            i_thresh,i_thresh*DEG_TO_RAD); 
    fprintf(log_fp," w  = %10f (degrees; %10f radians)\n",
            w_thresh,w_thresh*DEG_TO_RAD); 
    fprintf(log_fp," O  = %10f (degrees; %10f radians)\n",
            O_thresh,O_thresh*DEG_TO_RAD); 
    fprintf(log_fp," t0 = %10f (days)\n",t_thresh); 
  }

  /* Set the results to empty. */
  state->results = NULL;  

  ophp[0] = (OrbitProximityStateHandle)state;

  return 0;
}



/* Add a data orbit to the tree.  Return the internal OrbitProximity */
/* number for that orbit. */
int OrbitProximity_AddDataOrbit(OrbitProximityStateHandle fph,
                                double q,    /* Perihelion Distance (in AU)  */
                                double e,    /* Eccentricity                 */
                                double i,    /* Inclination (in degrees)     */
                                double w,    /* Arg of perihelion (degrees)  */
                                double O,    /* Long. of asc. node (degrees) */
                                double t0,   /* Time of perihelion (MJD)     */
                                double equinox /* Equinox of orbit (in MJD)  */
                                ) {
  orbprox_state* state = (orbprox_state*)fph;
  int old_maxsize = orbit_array_max_size(state->data);
  int old_size    = orbit_array_size(state->data);
  orbit* o;

  o = mk_orbit2(t0, q, e, i*DEG_TO_RAD, O*DEG_TO_RAD, 
                w*DEG_TO_RAD, equinox, equinox);

  state->num_orbits += 1;
  orbit_array_add(state->data,o);

  if(old_maxsize < orbit_array_max_size(state->data)) {
    if((state->verbosity > 1)&&(state->log_fp != NULL)) {
      fprintf(state->log_fp,
              "Data orbits: doubling array size to %i.\n",
              orbit_array_max_size(state->data));
    }
  }

  free_orbit(o);

  return (old_size+1 == orbit_array_size(state->data));
}



/* Add a query orbit to the query set.  Return the internal OrbitProximity */
/* number for that orbit. */
int OrbitProximity_AddQueryOrbit(OrbitProximityStateHandle fph,
                                double q,    /* Perihelion Distance (in AU)  */
                                double e,    /* Eccentricity                 */
                                double i,    /* Inclination (in degrees)     */
                                double w,    /* Arg of perihelion (degrees)  */
                                double O,    /* Long. of asc. node (degrees) */
                                double t0,   /* Time of perihelion (MJD)     */
                                double equinox  /* Equinox of orbit (in MJD) */
                                ) {
  orbprox_state* state = (orbprox_state*)fph;
  int old_maxsize = orbit_array_max_size(state->queries);
  int old_size    = orbit_array_size(state->queries);
  orbit* o;

  o = mk_orbit2(t0, q, e, i*DEG_TO_RAD, O*DEG_TO_RAD, 
                w*DEG_TO_RAD, equinox, equinox);

  state->num_queries += 1;
  orbit_array_add(state->queries,o);

  if(old_maxsize < orbit_array_max_size(state->queries)) {
    if((state->verbosity > 1)&&(state->log_fp != NULL)) {
      fprintf(state->log_fp,"Query orbits: doubling array size to %i.\n",
              orbit_array_max_size(state->queries));
    }
  }

  free_orbit(o);

  return (old_size+1 == orbit_array_size(state->queries));
}



/* Process all of the query orbits against all of the data orbits. */
int OrbitProximity_Run(OrbitProximityStateHandle fph) {
  orbprox_state* state   = (orbprox_state*)fph;
  orb_tree*      tr;
  ivec*          subres;
  dyv*           weights;
  int            i;

  /* If the results have already been run... remove them. */
  if(state->results != NULL) {
    free_ivec_array(state->results);
    state->results = NULL;
  }

  /* Create the orbit tree. Do not split on t0, leave it to */
  /* the exhaustive search.                                 */
  if((state->verbosity > 0)&&(state->log_fp != NULL)) {
    fprintf(state->log_fp,"Building the orbit tree from %i data orbits.\n",
            state->num_orbits);
  }
  weights = mk_orb_tree_weights(1.0,1.0,1.0,1.0,1.0,0.0);
  tr      = mk_orb_tree(state->data,weights,ORBPROX_PTS_PER_LEAF);
  
  /* Actually compute the results. */
  if((state->verbosity > 0)&&(state->log_fp != NULL)) {
    fprintf(state->log_fp,"Testing each of the %i query orbits.\n",
            state->num_queries);
  }
  state->results = mk_zero_ivec_array(state->num_queries);

  /* Run a tree search for each query */
  for(i=0;i<state->num_queries;i++) {
    subres = mk_orb_tree_range_search(orbit_array_ref(state->queries,i),tr,
                                      state->data,state->thresholds);
    ivec_array_set(state->results,i,subres);

    if((state->verbosity > 1)&&(state->log_fp != NULL)) {
      fprintf(state->log_fp,"Found %i matches for query %i:\n",
              ivec_size(subres),i);
      fprintf_ivec(state->log_fp,"  ",subres,"\n");
    }

    free_ivec(subres);
  }

  /* Free the allocated space (tree and weights) */
  if((state->verbosity > 1)&&(state->log_fp != NULL)) {
    fprintf(state->log_fp,"Freeing the orbit data structures.\n");
  }
  free_orb_tree(tr);
  free_dyv(weights);  

  return 0;
}


/* Fetch a result from processing - determine the number of "nearby" data */
/* orbits for query orbit number "query_num"                              */
/* Note: query_num is the index returned by OrbitProximity_AddQueryOrbit  */ 
int OrbitProximity_Num_Matches(OrbitProximityStateHandle fph,
                               int query_num) {
  orbprox_state* state = (orbprox_state*)fph;
  int res = -1;

  if(state->results != NULL) {
    if((query_num < ivec_array_size(state->results))&&(query_num >= 0)) {
      res = ivec_array_ref_size(state->results,query_num);
    }
  }

  return res;
}


/* Fetch a result from processing - return the "match_num"th matching       */
/* orbit for query "query_num".  This function can be called N times        */
/* for each query_num where: N = OrbitProximity_Num_Matches(fph,query_num)  */
/* Returns -1 on an error and the corresponding data orbit index otherwise. */
int OrbitProximity_Get_Match(OrbitProximityStateHandle fph,
                             int query_num,
                             int match_num
                             ) {
  orbprox_state* state = (orbprox_state*)fph;
  int res = -1;

  if(state->results != NULL) {
    if((query_num < ivec_array_size(state->results))&&(query_num >= 0)) {
      if((match_num >= 0) && 
         (match_num < ivec_array_ref_size(state->results,query_num))) {
        res = ivec_array_ref_ref(state->results,query_num,match_num);
      }
    }
  }

  return res;
}


/* Release all data structures used by OrbitProximity. */
int OrbitProximity_Free(OrbitProximityStateHandle fph) {
  orbprox_state* state = (orbprox_state*)fph;   
  int res = 1;

  if(state != NULL) {
    free_dyv(state->thresholds);
    free_orbit_array(state->data);
    free_orbit_array(state->queries);

    if(state->results != NULL) {
      free_ivec_array(state->results);
    }

    AM_FREE(state,orbprox_state);
    res = 0;
  }

  return res;
}
