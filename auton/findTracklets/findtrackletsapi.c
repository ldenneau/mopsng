/* File:        findtrackletsapi.c
   Author:      J. Kubica
   Created:     Mon, Oct. 10, 2005
   Description: Findtracklets API functions.

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

#include "findtrackletsapi.h"
#include "am_time.h"
#include "rdt_tree.h"
#include "tracklet_mht.h"

typedef struct findtracklets_state {

  /* Data */
  int num_points;
  simple_obs_array* data;        /* All of the data points  */

  /* Other (optional) data */
  namer* truenames;
  ivec*  true_groups;
  dyv*   length;
  dyv*   angle;
  dyv*   exp_time;

  /* Parameters. */
  double athresh;
  double thresh;
  double maxLerr;
  double minv;
  double maxv;
  double maxt;
  double etime;
  int minobs;
  int maxobs;
  bool eval;
  bool greedy;
  bool use_pht;

  /* Results */
  track_array* results;

  /* Running Options: */
  int verbosity;      /* 0 => no output, 1 => normal, 2 => verbose/debugging */
  FILE *log_fp;       /* use as way to pass file descriptor in for debugging output */

} findtracklets_state;


/* Initialize the find tracklets program (allocate memory, etc.) */ 
/* and return an opaque handle that contains state info.         */
/* Everything is created using the default parameter values.     */
int FindTracklets_Init(FindTrackletsStateHandle* ftsh,
                       int verbosity,  /* 0 => no output, 1 => normal, 2 => verbose/debugging */
                       FILE *log_fp    /* file descriptor in for debugging output */
                       ) {
  findtracklets_state* state;

  if((verbosity > 0)&&(log_fp != NULL)) {
    fprintf(log_fp,"Initializing FindTracklets State.\n");
  }

  state = AM_MALLOC(findtracklets_state);

  /* Set known values. */
  state->num_points  = 0;
  state->verbosity   = verbosity;
  state->log_fp      = log_fp;

  /* Set the default parameters. */
  state->athresh = FT_DEF_ATHRESH;
  state->thresh  = FT_DEF_THRESH;
  state->maxLerr = FT_DEF_MAXLERR;
  state->minv    = FT_DEF_MINV;
  state->maxv    = FT_DEF_MAXV;
  state->maxt    = FT_DEF_MAXT;
  state->etime   = FT_DEF_ETIME;
  state->minobs  = FT_DEF_MINOBS;
  state->minobs  = FT_DEF_MAXOBS;
  state->eval    = FALSE;
  state->greedy  = FALSE;
  state->use_pht = FALSE;

  /* Allocate space for the data */
  state->data        = mk_empty_simple_obs_array(128);
  state->true_groups = mk_ivec(0);
  state->truenames   = mk_empty_namer(TRUE);
  state->length      = mk_dyv(0);
  state->angle       = mk_dyv(0);
  state->exp_time    = mk_dyv(0);

  /* Set the results to empty. */
  state->results = NULL;

  ftsh[0] = (FindTrackletsStateHandle)state;

  return 0;
}



/* Set the maximum angular threshold (in degrees). */
int FindTracklets_set_athresh(FindTrackletsStateHandle* state, double nu_val) {
  findtracklets_state* st = (findtracklets_state*)state;

  if((st->verbosity > 1)&&(st->log_fp != NULL)) {
    fprintf(st->log_fp,"Setting the angular threshold as %f\n",nu_val);
  }

  st->athresh = nu_val;

  return 0;
}


/* Set the maximum fit threshold (in degrees). */
int FindTracklets_set_thresh(FindTrackletsStateHandle* state, double nu_val)  {
  findtracklets_state* st = (findtracklets_state*)state;

  if((st->verbosity > 1)&&(st->log_fp != NULL)) {
    fprintf(st->log_fp,"Setting the fit threshold as %f\n",nu_val);
  }

  st->thresh = nu_val;

  return 0;
}


/* Set the maximum length error (in degrees). */
int FindTracklets_set_maxLerr(FindTrackletsStateHandle* state, double nu_val)  {
  findtracklets_state* st = (findtracklets_state*)state;

  if((st->verbosity > 1)&&(st->log_fp != NULL)) {
    fprintf(st->log_fp,"Setting the maximum length error threshold as %f\n",nu_val);
  }

  st->maxLerr = nu_val;

  return 0;
}


/* Set the length of time of exposure (for elongation) in seconds */
int FindTracklets_set_etime(FindTrackletsStateHandle* state, double nu_val)  {
  findtracklets_state* st = (findtracklets_state*)state;

  if((st->verbosity > 1)&&(st->log_fp != NULL)) {
    fprintf(st->log_fp,"Setting the exposure time as %f\n",nu_val);
  }

  st->etime = nu_val;

  return 0;
}


/* Set the maximum tracklet velocity (in deg/day) */
int FindTracklets_set_minv(FindTrackletsStateHandle* state, double nu_val)  {
  findtracklets_state* st = (findtracklets_state*)state;

  if((st->verbosity > 1)&&(st->log_fp != NULL)) {
    fprintf(st->log_fp,"Setting the maximum velocity as %f\n",nu_val);
  }

  st->minv = nu_val;
  return 0;
}


/* Set the maximum tracklet velocity (in deg/day) */
int FindTracklets_set_maxv(FindTrackletsStateHandle* state, double nu_val)  {
  findtracklets_state* st = (findtracklets_state*)state;

  if((st->verbosity > 1)&&(st->log_fp != NULL)) {
    fprintf(st->log_fp,"Setting the maximum velocity as %f\n",nu_val);
  }

  st->maxv = nu_val;
  return 0;
}


/* Set the maximum tracklet time span (in days) */
int FindTracklets_set_maxt(FindTrackletsStateHandle* state, double nu_val)  {
  findtracklets_state* st = (findtracklets_state*)state;

  if((st->verbosity > 1)&&(st->log_fp != NULL)) {
    fprintf(st->log_fp,"Setting the maximum time span as %f\n",nu_val);
  }

  st->maxt = nu_val;

  return 0;
}


/* Set the minimum number of required detections. */
int FindTracklets_set_minobs(FindTrackletsStateHandle* state, int nu_val)  {
  findtracklets_state* st = (findtracklets_state*)state;

  if((st->verbosity > 1)&&(st->log_fp != NULL)) {
    fprintf(st->log_fp,"Setting the minimum number of observations as %i\n",nu_val);
  }

  st->minobs = nu_val;

  return 0;
}


/* Set the maximum number of detections. */
int FindTracklets_set_maxobs(FindTrackletsStateHandle* state, int nu_val)  {
  findtracklets_state* st = (findtracklets_state*)state;

  if((st->verbosity > 1)&&(st->log_fp != NULL)) {
    fprintf(st->log_fp,"Setting the maximum number of observations as %i\n",nu_val);
  }

  st->maxobs = nu_val;

  return 0;
}


/* Use evaluation mode? (0=NO, 1=YES) */
int FindTracklets_set_eval(FindTrackletsStateHandle* state, int val)  {
  findtracklets_state* st = (findtracklets_state*)state;

  if((st->verbosity > 1)&&(st->log_fp != NULL)) {
    if(val == 0) { fprintf(st->log_fp,"Turned OFF evaluations mode.\n"); }
    if(val == 1) { fprintf(st->log_fp,"Turned ON evaluations mode.\n"); }
  }

  if(val == 1) { st->eval = TRUE; }
  if(val == 0) { st->eval = FALSE; }

  return 0;
}


/* Use greedy mode? (0=NO, 1=YES) */
int FindTracklets_set_greedy(FindTrackletsStateHandle* state, int val)  {
  findtracklets_state* st = (findtracklets_state*)state;

  if((st->verbosity > 1)&&(st->log_fp != NULL)) {
    if(val == 0) { fprintf(st->log_fp,"Turned OFF greedy mode.\n"); }
    if(val == 1) { fprintf(st->log_fp,"Turned ON greedy mode.\n"); }
  }

  if(val == 1) { st->greedy = TRUE; }
  if(val == 0) { st->greedy = FALSE; }

  return 0;
}

int FindTracklets_set_use_pht(FindTrackletsStateHandle* state, int val) {
  findtracklets_state* st = (findtracklets_state*)state;

  if((st->verbosity > 1)&&(st->log_fp != NULL)) {
    if(val == 0) { fprintf(st->log_fp,"Turned OFF PHT mode.\n"); }
    if(val == 1) { fprintf(st->log_fp,"Turned ON PHT mode.\n"); }
  }

  if(val == 1) { st->use_pht = TRUE; }
  if(val == 0) { st->use_pht = FALSE; }

  return 0;
}


/* Add a data detection to the data set.    */
/* Return the internal FindTracklets number */
/* for that detections.                     */
int FindTracklets_AddDataDetection(FindTrackletsStateHandle fph,
                                  double ra,            /* Right Ascension (in hours) */
                                  double dec,           /* Declination (in degrees)   */
                                  double time,          /* Time (in MJD)              */
                                  double brightness     /* Brightness                 */
                                   ) {
  findtracklets_state* state = (findtracklets_state*)fph;
  int old_maxsize = simple_obs_array_max_size(state->data);
  int old_size    = simple_obs_array_size(state->data);
  simple_obs* o;

  o = mk_simple_obs_simplest(old_size,time,ra,dec,brightness);

  state->num_points += 1;
  simple_obs_array_add(state->data,o);
  
  /* Add the optional information. */
  add_to_ivec(state->true_groups, -1);
  add_to_dyv(state->length, -1.0);
  add_to_dyv(state->angle, 0.0);
  add_to_dyv(state->exp_time, 0.0);

  /* Check if we doubled the array size. */
  if(old_maxsize < simple_obs_array_max_size(state->data)) {
    if((state->verbosity > 1)&&(state->log_fp != NULL)) {
      fprintf(state->log_fp,"Ran out of space for data detections... doubling array size to %i.\n",
              simple_obs_array_max_size(state->data));
    }
  }

  free_simple_obs(o);

  return (old_size+1 == simple_obs_array_size(state->data));
}


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
          ) {
  findtracklets_state* state = (findtracklets_state*)fph;
  int old_maxsize = simple_obs_array_max_size(state->data);
  int old_size    = simple_obs_array_size(state->data);
  simple_obs* o;

  o = mk_simple_obs_simplest(old_size,time,ra,dec,brightness);

  state->num_points += 1;
  simple_obs_array_add(state->data,o);
  
  /* Add the optional information. */
  add_to_ivec(state->true_groups, -1);
  add_to_dyv(state->length, length*DEG_TO_RAD);
  add_to_dyv(state->angle, angle*DEG_TO_RAD);
  add_to_dyv(state->exp_time, exp_time/(24.0*60.0*60.0));
  
  /* Check if we doubled the array size. */
  if(old_maxsize < simple_obs_array_max_size(state->data)) {
    if((state->verbosity > 1)&&(state->log_fp != NULL)) {
      fprintf(state->log_fp,"Ran out of space for data detections... doubling array size to %i.\n",
              simple_obs_array_max_size(state->data));
    }
  }

  free_simple_obs(o);

  return (old_size+1 == simple_obs_array_size(state->data));
}


/* Add the ground truth tracklet to a detection.  Used for evaluation. */
/* Note the detection is indexed by its internal ID.                   */
int FindTracklets_AddTruth(FindTrackletsStateHandle fph,
                           int detection_index,
                           int tracklet_num) {
  findtracklets_state* state = (findtracklets_state*)fph;
  int res = 0;

  if((detection_index >= 0)&&(detection_index < ivec_size(state->true_groups))) {

    if((state->verbosity > 1)&&(state->log_fp != NULL)) {
      fprintf(state->log_fp,"Assigning detection %i to group %i\n",
              detection_index,tracklet_num);
    }

    ivec_set(state->true_groups,detection_index,tracklet_num);

  } else {

    if((state->verbosity > 0)&&(state->log_fp != NULL)) {
      fprintf(state->log_fp,"ERROR: Tried to assign a name to an invalid detection index (%i).\n",
              detection_index);      
    }

    res = 1;
  }

  return res;
}


/* Add the ground truth name to a detection.  Used for evaluation. */
/* Note the detection is indexed by its internal ID.               */
int FindTracklets_AddTrackletName(FindTrackletsStateHandle fph, 
                                 int detection_index,
                                 char* name
                                  ) {
  findtracklets_state* state = (findtracklets_state*)fph;
  int res = 0;
  int ind;


  if((detection_index >= 0)&&(detection_index < ivec_size(state->true_groups))) {

    /* Get the index of this name */
    if(!eq_string(name,"FALSE")) {
      add_to_namer(state->truenames,name);
      ind = namer_name_to_index(state->truenames,name);

      if((state->verbosity > 1)&&(state->log_fp != NULL)) {
        fprintf(state->log_fp,"Assigning detection %i to group %i\n",
                detection_index,ind);
      }

      ivec_set(state->true_groups,detection_index,ind);
    } else {
      ivec_set(state->true_groups,detection_index,-1);
    }

  } else {

    if((state->verbosity > 0)&&(state->log_fp != NULL)) {
      fprintf(state->log_fp,"ERROR: Tried to assign a name to an invalid detection index (%i).\n",
              detection_index);      
    }

    res = 1;
  }

  return res;
}


/* Process the tree. */
int FindTracklets_Run(FindTrackletsStateHandle fph) {
  findtracklets_state* state = (findtracklets_state*)fph;
  track_array* cheat;
  ivec_array* obs_to_track;
  ivec* cheat_pairs;
  ivec* roc;
  ivec* inds;
  int i,j;

  if((state->verbosity > 0)&&(state->log_fp != NULL)) {
    fprintf(state->log_fp,"FIND_TRACKLETS VERSION: %i.%i.%i\n\n",TRACKLET_VERSION,
            TRACKLET_RELEASE,TRACKLET_UPDATE);

    fprintf(state->log_fp,"Fit threshold (degrees)  = %12.8f   (default = %f)\n",
            state->thresh,FT_DEF_THRESH);
    fprintf(state->log_fp,"Angle Thresh (degrees)   = %12.8f   (default = %f)\n",
            state->athresh,FT_DEF_ATHRESH);
    fprintf(state->log_fp,"maxLerr (degrees)        = %12.8f   (default = %f)\n",
            state->maxLerr,FT_DEF_MAXLERR);
    fprintf(state->log_fp,"Min. Velocity (deg/day)  = %12.8f   (default = %f)\n",
            state->minv,FT_DEF_MINV);
    fprintf(state->log_fp,"Max. Velocity (deg/day)  = %12.8f   (default = %f)\n",
            state->maxv,FT_DEF_MAXV);
    fprintf(state->log_fp,"Max. Spread   (days)     = %12.8f   (default = %f)\n",
            state->maxt,FT_DEF_MAXT);
    fprintf(state->log_fp,"Exposure Time (sec)      = %12.8f   (default = %f)\n",
            state->etime,FT_DEF_ETIME);
    fprintf(state->log_fp,"Max. Number of Obs.      = %12i   (defaul = %i)\n",
            state->maxobs,FT_DEF_MAXOBS);
    fprintf(state->log_fp,"Min. Number of Obs.      = %12i   (defaul = %i)\n",
            state->minobs,FT_DEF_MINOBS);
    if(state->greedy) {
      fprintf(state->log_fp,"Greedy mode:                 ON\n");
    } else {
      fprintf(state->log_fp,"Greedy mode:                 OFF\n");
    }
    if(state->use_pht) {
      fprintf(state->log_fp,"PHT mode:                    ON\n");
    } else {
      fprintf(state->log_fp,"PHT mode:                    OFF\n");
    }
    if(state->eval) {
      fprintf(state->log_fp,"Evaluation mode:             ON\n");
    } else {
      fprintf(state->log_fp,"Evaluation mode:             OFF\n");
    }
    fprintf(state->log_fp,"Remove subsets/duplicates:   ON\n");
 
    fprintf(state->log_fp,"\n\n\nRun started (");
    fputs(curr_time(), state->log_fp);
    fprintf(state->log_fp,")\n");
  }

  state->results = mk_tracklets_MHT(state->data,
                                    state->minv*DEG_TO_RAD,
                                    state->maxv*DEG_TO_RAD,
                                    state->thresh*DEG_TO_RAD,
                                    state->maxt,
                                    state->minobs,
                                    TRUE,
                                    state->angle,
                                    state->length,
                                    state->exp_time,
                                    state->athresh*DEG_TO_RAD,
                                    state->maxLerr*DEG_TO_RAD,
                                    state->etime/(24.0*60.0*60.0),
                                    state->maxobs,
                                    state->greedy,
                                    state->use_pht);  

  /* Do the scoring. */
  if(state->eval == TRUE) {

    if((state->verbosity > 0)&&(state->log_fp != NULL)) {    
      fprintf(state->log_fp,"\n\nScoring the tracks (");
      fputs(curr_time(), state->log_fp);
      fprintf(state->log_fp,").  This may take a little while.\n");
    }

    obs_to_track = mk_simple_obs_pairing_from_true_groups(state->data,
                                                          state->true_groups,
                                                          state->maxt);
    cheat = mk_track_array_from_matched_simple_obs(state->data,
                                                   obs_to_track,
                                                   state->minobs);
    free_ivec_array(obs_to_track);

    cheat_pairs = mk_constant_ivec(simple_obs_array_size(state->data),-1);
    for(i=0;i<track_array_size(cheat);i++) {
      inds = track_individs(track_array_ref(cheat,i));
      for(j=0;j<ivec_size(inds);j++) {
        ivec_set(cheat_pairs,ivec_ref(inds,j),i);
      }
    }

    roc = mk_track_array_roc_vec(state->data,state->results,cheat_pairs,state->minobs,1,0.95);
    
    if((state->verbosity > 0)&&(state->log_fp != NULL)) {
      fprintf(state->log_fp,"There are %i true tracklets in the code.\n",
              ivec_max(cheat_pairs)+1);
      fprintf(state->log_fp,"The code returned %i suggested tracklets.\n",
              track_array_size(state->results));
      fprintf(state->log_fp,"%i of the return tracklets EXACTLY matched true tracklets.\n",
              ivec_sum(roc));

      fprintf(state->log_fp,"Full Track: Per. Corr. = %f      Per. Found = %f\n",
             roc_percent_correct(roc),roc_percent_found(roc,cheat_pairs));
    }

    free_ivec(roc);
    free_ivec(cheat_pairs);
    free_track_array(cheat);
  }

  return 0;
}


/* Fetch the total number of result tracklets. */
int FindTracklets_Num_Tracklets(FindTrackletsStateHandle fph) {
  findtracklets_state* state = (findtracklets_state*)fph;
  int res = -1;

  if(state->results != NULL) {
    res = track_array_size(state->results);
  }

  return res;
}


/* Fetch the number of detections in a given tracklet. */
int FindTracklets_Num_Detections(FindTrackletsStateHandle fph,
                                 int tracklet_number
                                 ) {
  findtracklets_state* state = (findtracklets_state*)fph;
  int res = -1;

  if(state->results != NULL) {
    if((tracklet_number < track_array_size(state->results))&&(tracklet_number >= 0)) {
      res = track_num_obs(track_array_ref(state->results,tracklet_number));
    }
  }

  return res;
}


/* Fetch a given detection id from a given tracklet.            */
/* This function can be called N times for each tracklet where: */
/*    N = FindTracklets_Num_Detections(fph,tracklet_num)        */
/* Returns -1 on an error and the corresponding FindTracklets   */
/* detection ID otherwise.                                      */
int FindTracklets_Get_Match(FindTrackletsStateHandle fph,
                            int tracklet_number,
                            int match_num
                            ) {
  findtracklets_state* state = (findtracklets_state*)fph;
  int res = -1;

  if(state->results != NULL) {
    if((tracklet_number < track_array_size(state->results))&&(tracklet_number >= 0)) {
      if((match_num >= 0)&&(match_num < track_num_obs(track_array_ref(state->results,tracklet_number)))) {
        res = ivec_ref(track_individs(track_array_ref(state->results,tracklet_number)),match_num);
      }
    }
  }

  return res;
}


/* Release all data structures used by FindTracklets. */
int FindTracklets_Free(FindTrackletsStateHandle fph) {
  findtracklets_state* st = (findtracklets_state*)fph;
  int res = 1;

  if((st->verbosity > 0)&&(st->log_fp != NULL)) {
    fprintf(st->log_fp,"Freeing FindTracklets State.\n");
  }  

  if(st != NULL) {
    free_simple_obs_array(st->data);
    free_dyv(st->length); 
    free_dyv(st->angle);
    free_dyv(st->exp_time);
    free_ivec(st->true_groups);
    free_namer(st->truenames);

    if(st->results != NULL) {
      free_track_array(st->results);
    }

    AM_FREE(st,findtracklets_state);
    res = 0;
  }

  return res;
}
