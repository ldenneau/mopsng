/* File:        detectprox.c
   Author:      J. Kubica
   Created:     Sun, August 7 2005
   Description: The detection proximity interface.

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

#include "detectprox.h"
#include "rdt_tree.h"

#define DETECTPROX_PTS_PER_LEAF 25

typedef struct detectprox_state {
  int num_points;
  int num_queries;

  /* Actual data arrays */
  simple_obs_array* data;        /* All of the data points  */
  simple_obs_array* queries;     /* All of the query points */

  /* The algorithm data structures */
  dyv* d_thresh;
  dyv* t_thresh;
  dyv* b_thresh;

  /* The result array (and tree data structure) */
  ivec_array* results;

  /* Running Options: */
  int verbosity;      /* 0 => no output, 1 => normal, 2 => verbose/debugging */
  FILE *log_fp;       /* use as way to pass file descriptor in for debugging output */

} detectprox_state;


/* Initialize the DetectionProximity engine and return an opaque handle
   that contains state info. */
int DetectionProximity_Init(DetectionProximityStateHandle *ophp,
                            int verbosity,   /* 0 => no output, 1 => normal,  */
                                             /* 2 => verbose/debugging        */
                            FILE *log_fp  /* file descriptor for debugging output */
                            ) {
  detectprox_state* state;

  if((verbosity > 0)&&(log_fp != NULL)) {
    fprintf(log_fp,"Initializing DetectionProximity State.\n");
  }

  state = AM_MALLOC(detectprox_state);
 
  /* Set known values. */
  state->num_points  = 0;
  state->num_queries = 0;
  state->verbosity   = verbosity;
  state->log_fp      = log_fp;

  /* Allocate space for the data and query orbits. */
  state->data    = mk_empty_simple_obs_array(128);
  state->queries = mk_empty_simple_obs_array(64);
  
  /* Fix the thresholds. */
  state->d_thresh = mk_dyv(0);
  state->b_thresh = mk_dyv(0);
  state->t_thresh = mk_dyv(0);

  /* Set the results to empty. */
  state->results = NULL;  

  ophp[0] = (DetectionProximityStateHandle)state;

  return 0;
}



/* Add a data detection to the tree.  Return the internal DetectionProximity */
/* number for that orbit. */
int DetectionProximity_AddDataDetection(DetectionProximityStateHandle fph,
                                        double ra,        /* Right Ascension (in hours) */
                                        double dec,       /* Declination (in degrees)   */
                                        double time,      /* Time (in MJD)              */
                                        double brightness /* Brightness                 */
                                        ) {
  detectprox_state* state = (detectprox_state*)fph;
  int old_maxsize = simple_obs_array_max_size(state->data);
  int old_size    = simple_obs_array_size(state->data);
  simple_obs* o;

  o = mk_simple_obs_simplest(old_size,time,ra,dec,brightness);

  state->num_points += 1;
  simple_obs_array_add(state->data,o);

  if(old_maxsize < simple_obs_array_max_size(state->data)) {
    if((state->verbosity > 1)&&(state->log_fp != NULL)) {
      fprintf(state->log_fp,"Ran out of space for data orbits... doubling array size to %i.\n",
              simple_obs_array_max_size(state->data));
    }
  }

  free_simple_obs(o);

  return (old_size+1 == simple_obs_array_size(state->data));
}


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
                                         ) {
  detectprox_state* state = (detectprox_state*)fph;
  int old_maxsize = simple_obs_array_max_size(state->queries);
  int old_size    = simple_obs_array_size(state->queries);
  simple_obs* o;

  o = mk_simple_obs_simplest(old_size,time,ra,dec,brightness);

  state->num_queries += 1;
  simple_obs_array_add(state->queries,o);
  add_to_dyv(state->d_thresh,dist_thresh*DEG_TO_RAD);
  add_to_dyv(state->t_thresh,time_thresh);
  add_to_dyv(state->b_thresh,bright_thresh);

  if(old_maxsize < simple_obs_array_max_size(state->queries)) {
    if((state->verbosity > 1)&&(state->log_fp != NULL)) {
      fprintf(state->log_fp,"Ran out of space for query orbits... doubling array size to %i.\n",
              simple_obs_array_max_size(state->queries));
    }
  }

  free_simple_obs(o);

  return (old_size+1 == simple_obs_array_size(state->queries));
}



/* Process the tree. */
int DetectionProximity_Run(DetectionProximityStateHandle fph) {
  detectprox_state* state   = (detectprox_state*)fph;
  simple_obs*    q;
  simple_obs*    x;
  rdt_tree*      tr;
  ivec*          subres;
  ivec*          temp;
  double         t_q;
  int            i,j;

  /* If the results have already been run... remove them. */
  if(state->results != NULL) {
    free_ivec_array(state->results);
    state->results = NULL;
  }

  /* Create the tree.  */
  if((state->verbosity > 0)&&(state->log_fp != NULL)) {
    fprintf(state->log_fp,"Building the tree data structure from %i data points.\n",
            state->num_points);
  }
  tr = mk_rdt_tree(state->data,NULL,FALSE,DETECTPROX_PTS_PER_LEAF);

  /* Actually compute the results. */
  if((state->verbosity > 0)&&(state->log_fp != NULL)) {
    fprintf(state->log_fp,"Testing each of the %i query points.\n",
            state->num_queries);
  }
  state->results = mk_zero_ivec_array(state->num_queries);
  for(i=0;i<state->num_queries;i++) {
    q      = simple_obs_array_ref(state->queries,i);
    t_q    = simple_obs_time(q);
    subres = mk_rdt_tree_range_search(tr,state->data,q,t_q-dyv_ref(state->t_thresh,i),
                                      t_q+dyv_ref(state->t_thresh,i),
                                      dyv_ref(state->d_thresh,i));
    
    /* Do a post filtering on brightness. */
    if(dyv_ref(state->b_thresh,i) > -1e-20) {
      temp = mk_ivec(0);
      for(j=0;j<ivec_size(subres);j++) {
        x = simple_obs_array_ref(state->data,ivec_ref(subres,j));
        if(fabs(simple_obs_brightness(q)-simple_obs_brightness(x)) < dyv_ref(state->b_thresh,i)) {
          add_to_ivec(temp,ivec_ref(subres,j));
        }
      }
      free_ivec(subres);
      subres = temp;
    }

    ivec_array_set(state->results,i,subres);

    if((state->verbosity > 1)&&(state->log_fp != NULL)) {
      fprintf(state->log_fp,"Found matches for query %i:\n",i);
      fprintf_ivec(state->log_fp,"  ",subres,"\n");
    }

    free_ivec(subres);
  }

  /* Free the allocated space */
  if((state->verbosity > 1)&&(state->log_fp != NULL)) {
    fprintf(state->log_fp,"Freeing the orbit data structures.\n");
  }
  free_rdt_tree(tr);

  return 0;
}


/* Fetch a result from processing - determine the number of "nearby" data   */
/* orbits for query orbit number "query_num."  Note: query_num is the index */
/* returned by DetectionProximity_AddQueryDetection.  */ 
int DetectionProximity_Num_Matches(DetectionProximityStateHandle fph,
                                   int query_num
                                   ) {
  detectprox_state* state = (detectprox_state*)fph;
  int res = -1;

  if(state->results != NULL) {
    if((query_num < ivec_array_size(state->results))&&(query_num >= 0)) {
      res = ivec_array_ref_size(state->results,query_num);
    }
  }

  return res;
}


/* Fetch a result from processing - return the "match_num"th matching       */
/* orbit for query "query_num".  This function can be called N times for    */
/* each query_num where: N = DetectionProximity_Num_Matches(fph,query_num)  */
/* Returns -1 on an error and the corresponding data orbit index otherwise. */
int DetectionProximity_Get_Match(DetectionProximityStateHandle fph,
                                 int query_num,
                                 int match_num
                                 ) {
  detectprox_state* state = (detectprox_state*)fph;
  int res = -1;

  if(state->results != NULL) {
    if((query_num < ivec_array_size(state->results))&&(query_num >= 0)) {
      if((match_num >= 0)&&(match_num < ivec_array_ref_size(state->results,query_num))) {
        res = ivec_array_ref_ref(state->results,query_num,match_num);
      }
    }
  }

  return res;
}


/* Release all data structures used by DetectionProximity. */
int DetectionProximity_Free(DetectionProximityStateHandle fph) {
  detectprox_state* state = (detectprox_state*)fph;   
  int res = 1;

  if(state != NULL) {
    free_dyv(state->t_thresh);
    free_dyv(state->b_thresh);
    free_dyv(state->d_thresh);
    free_simple_obs_array(state->data);
    free_simple_obs_array(state->queries);

    if(state->results != NULL) {
      free_ivec_array(state->results);
    }

    AM_FREE(state,detectprox_state);
    res = 0;
  }

  return res;
}
