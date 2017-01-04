/*
   File:        MHT.c
   Author:      J. Kubica
   Created:     Tue June 15, 2004
   Description: Tracking and linking functions for finding astronomical
                tracklets.

   Copyright 2004, The Auton Lab, CMU

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

#include "tracklet_mht.h"

/* --------------------------------------------------------------------- */
/* --- Partial Hough Transform Approach -------------------------------- */
/* --------------------------------------------------------------------- */

/* A PairedVelocity is a circular region in velocity space computed from a */
/* pair of observations and the uncertainty on those observations. */
typedef struct PairedVelocity {
  double vRA;        /* Signed vRA in radians / day */
  double vDEC;       /* Signed vRA in radians / day */
  double dt;         /* Time length (in days) */
  double radius_sq;  /* Uncertainty radius (squared) */
} PairedVelocity;


PairedVelocity* mk_PairedVelocity(simple_obs* first, simple_obs* last,
                                  double error_thresh) {
  PairedVelocity* result = AM_MALLOC(PairedVelocity);

  /* Make sure the observations are correctly ordered. */
  if (simple_obs_time(first) > simple_obs_time(last)) {
    simple_obs* tmp = first;
    first = last;
    last = tmp;
  }

  double dt = simple_obs_time(last) - simple_obs_time(first);
  double dDEC = simple_obs_DEC_rad(last) - simple_obs_DEC_rad(first);
  double dRA = simple_obs_RA_rad(last) - simple_obs_RA_rad(first);
  if (dRA > PI)
    dRA -= 2.0*PI;
  if (dRA < -PI)
    dRA += 2.0*PI;

  /* Compute the velocities, accounting for the error threshold. */
  const double inv_dt = 1.0 / dt;
  result->vRA = dRA * inv_dt;
  result->vDEC = dDEC * inv_dt;
  result->radius_sq = 4.0 * error_thresh * error_thresh * inv_dt * inv_dt;
  result->dt = dt;

  return result;
}

/* Is A a subset of B?  Assumes that the elements of A and B have an */
/* underlying ordering (not necessarily the sorted ordering). */
bool OrderedIvecSubset(ivec* A, ivec* B) {
  if (ivec_size(A) > ivec_size(B))
    return FALSE;
  
  int curr_a_ind = 0;
  int curr_b_ind = 0;
  while ((curr_a_ind < ivec_size(A)) && (curr_b_ind < ivec_size(B))) {
    if (ivec_ref(A, curr_a_ind) == ivec_ref(B, curr_b_ind)) {
      curr_a_ind++;
      curr_b_ind++;
    } else {
      curr_b_ind++;  /* Move along until we find a match. */
    }
  }

  /* A is a subset if every element in A had a match in B. */
  return (curr_a_ind == ivec_size(A));
}


track_array* mk_tracklets_single_query_PHT(simple_obs_array* arr, int Xind,
                                           rdt_tree* tr, double minv, double maxv,
                                           double thresh, double maxt,
                                           dyv* angle, dyv* length,
                                           dyv* exp_time, double athresh,
                                           double maxLerr, double etime,
                                           bool remove_subsets,
                                           int max_obs, int min_obs,
                                           bool greedy) {
  int i, j;

  /* Use what we know about the elongation to adjust maxv. */
  /* Only mess with v bounds if we have a fast mover.     */
//  double estMinV = 0.0;
  double estMinV = minv;
  double estMaxV = maxv;
  double curr_etime = etime;
  if ((exp_time != NULL) && (dyv_size(exp_time) > Xind) &&
      (dyv_ref(exp_time, Xind) > 0.0)) {
    curr_etime = dyv_ref(exp_time, Xind); 
  }
  if((length != NULL) && (dyv_ref(length, Xind) >= -1e-10)) {
    if(dyv_ref(length, Xind) / curr_etime > maxv) {
      estMinV = (dyv_ref(length, Xind) - maxLerr) / curr_etime;
      estMaxV = (dyv_ref(length, Xind) + maxLerr) / curr_etime;
      if(estMinV < 0.0) { estMinV = 0.0; }
    }
  }

  /* Find all feasible second endpoints. */
  simple_obs* X = simple_obs_array_ref(arr, Xind);
  ivec* pairs = mk_rdt_tree_moving_pt_query(tr, arr, X,
                                            simple_obs_time(X)+1e-5,
                                            simple_obs_time(X)+maxt,
                                            estMinV, estMaxV, thresh);
  int N = ivec_size(pairs);

  /* Find the times and sort them. */
  dyv* times = mk_zero_dyv(N);
  for(i = 0; i < N; i++) {
    simple_obs* Y = simple_obs_array_ref(arr, ivec_ref(pairs, i));
    dyv_set(times, i, simple_obs_time(Y));
  }
  ivec* order = mk_ivec_sorted_dyv_indices(times);
  ivec* ordered_pairs = mk_zero_ivec(N);
  for(i = 0; i < N; i++) {
    ivec_set(ordered_pairs, i, ivec_ref(pairs, ivec_ref(order, i)));
  }
  free_ivec(order);
  free_dyv(times);

  /* Build an array of the pairwise velocities from the starting detection */
  /* to each candidate. */
  PairedVelocity** bounds_array = AM_MALLOC_ARRAY(PairedVelocity*, N);
  for (i = 0; i < N; i++) {
    simple_obs* Y = simple_obs_array_ref(arr, ivec_ref(ordered_pairs, i));
    bounds_array[i] = mk_PairedVelocity(X, Y, thresh);
  }

  /* For each possible end point, create a tracklet from all all other */
  /* compatible detections (defined by overlap in velocity space).  We */
  /* could use a tree here to speed things up if this becomes a bottleneck.*/
  track_array* res = mk_empty_track_array(1);
  ivec_array* res_inds = mk_ivec_array(0);
  int length_longest = 0;
  for (i = N-1; i >= 0; --i) {
    ivec* inds = mk_ivec_1(Xind);
    PairedVelocity* end_pt = bounds_array[i];

    /* Greedily build the rest of the tracklet, adding the closest */
    /* detection (in velocity space) at each time. */
    double t_last = -1.0;
    double d_last = 0.0;
    for (j = 0; j <= i; ++j) {
      PairedVelocity* curr_pt = bounds_array[j];

      double dRA = (end_pt->vRA - curr_pt->vRA) * cos(end_pt->vDEC);
      double dDEC = (end_pt->vDEC - curr_pt->vDEC);
      double dist = (dRA*dRA + dDEC*dDEC);
      if (dist <= end_pt->radius_sq + curr_pt->radius_sq) {
        /* Check that we are either adding a detection at a new time or */
        /* a closer detection at the current time. */
        if (t_last + 1e-10 > curr_pt->dt) { 
          if (dist < d_last) {
            ivec_set(inds, ivec_size(inds) - 1, ivec_ref(ordered_pairs, j));
            t_last = curr_pt->dt;
            d_last = dist;
          }
        } else {
          add_to_ivec(inds, ivec_ref(ordered_pairs, j));
          t_last = curr_pt->dt;
          d_last = dist;
        }
      }
    }

    /* If the tracklet is long enough (but not too long) AND is not a subset */
    /* of something that we have already found: add it to the results. */
    if ((ivec_size(inds) >= min_obs) && (ivec_size(inds) <= max_obs)) {
      bool is_subset = FALSE;
      if (remove_subsets) {
        /* Brute force: We can make this more efficient if needed. */
        for (j = 0; (j < ivec_array_size(res_inds)) && !is_subset; ++j) {
          is_subset = OrderedIvecSubset(inds, ivec_array_ref(res_inds, j));
        }
      }
      if (!is_subset) {
        track* trk = mk_track_from_N_inds(arr, inds);
        track_array_add(res, trk);
        add_to_ivec_array(res_inds, inds);
        free_track(trk);

        if (ivec_size(inds) > length_longest) {
          length_longest = ivec_size(inds);
        }
      }
    }

    free_ivec(inds);
  }

  /* If we are in greedy mode, only take the longest tracks.  */
  if (greedy) {
    track_array* greed_res = mk_empty_track_array(1);
    for (i = 0; i < track_array_size(res); ++i) {
      if (track_size(track_array_ref(res, i)) == length_longest) {
        track_array_add(greed_res, track_array_ref(res, i));
      }
    }

    free_track_array(res);
    res = greed_res;    
  }

  /* Free the used memory. */
  for (i = 0; i < N; i++) {
    AM_FREE(bounds_array[i], PairedVelocity);
  }
  AM_FREE_ARRAY(bounds_array, PairedVelocity*, N);
  free_ivec_array(res_inds);
  free_ivec(ordered_pairs);
  free_ivec(pairs);

  return res;
}


/* --------------------------------------------------------------------- */
/* --- Multiple Hypothesis Tracking Approach --------------------------- */
/* --------------------------------------------------------------------- */

track_array* mk_tracklets_single_query(simple_obs_array* arr, int Xind,
                                       rdt_tree* tr,  double minv, double maxv,
                                       double thresh, double maxt,
                                       dyv* angle, dyv* length, dyv* exp_time,
                                       double athresh, double maxLerr,
                                       double etime, bool remove_subsets,
                                       int max_obs, bool greedy) {
  track_array* res = mk_empty_track_array(10);
  track_array* res2;
  track* A;
  track* B;
  simple_obs* X = simple_obs_array_ref(arr,Xind);
  simple_obs* Y;
  ivec* ord_pairs;
  ivec* pairs;
  ivec* order;
  ivec* inds;
  dyv*  times;
  bool  valid;
  double ang1, ang2;
  double dD, dR, diff, vel;
  double estMinV = minv;        /* 0.0 */
  double estMaxV = maxv;
  int N, Nlast;
  int i, j;
  int Yind;

  /* Use what we know about the elongation to adjust maxv */
  /* Only mess with v bounds if we have a fast mover.     */
  double curr_etime = etime;
  if ((exp_time != NULL) && (dyv_size(exp_time) > Xind) &&
      (dyv_ref(exp_time, Xind) > 0.0)) {
    curr_etime = dyv_ref(exp_time, Xind); 
  }
  if((length != NULL) && (dyv_ref(length,Xind) >= -1e-10)) {
    if(dyv_ref(length, Xind) / curr_etime > maxv) {            /* New test. */
      estMinV = (dyv_ref(length, Xind) - maxLerr) / curr_etime;
      estMaxV = (dyv_ref(length, Xind) + maxLerr) / curr_etime;

      if(estMinV < 0.0) { estMinV = 0.0; }
    }
  }

  /* Find all feasible second endpoints. */
  pairs = mk_rdt_tree_moving_pt_query(tr,arr,X,simple_obs_time(X)+1e-5,
                                      simple_obs_time(X)+maxt,estMinV,
                                      estMaxV,thresh);
  N     = ivec_size(pairs);

  /* Find the times and sort them... */
  times = mk_zero_dyv(N);
  for(i=0;i<N;i++) {
    dyv_set(times,i,simple_obs_time(simple_obs_array_ref(arr,ivec_ref(pairs,i))));
  }
  order     = mk_ivec_sorted_dyv_indices(times);
  ord_pairs = mk_zero_ivec(N);
  for(i=0;i<N;i++) {
    ivec_set(ord_pairs,i,ivec_ref(pairs,ivec_ref(order,i)));
  }

  /* Start the tracklet at the origional observation */
  B = mk_track_single_ind(arr,Xind);
  track_array_add(res,B);
  free_track(B);

  /* Try a large and messy MHT */
  for(i=0;i<N;i++) {
    Y = simple_obs_array_ref(arr,ivec_ref(ord_pairs,i));

    Nlast = track_array_size(res);
    for(j=0;j<Nlast;j++) {
      A = track_array_ref(res,j);

      /* Check that the new obs is after the last obs and  */
      /* not too far from the first observation (in time). */
      /* Also allow a max depth branching factor.          */
      valid = (simple_obs_time(Y) - track_first_time(A,arr) < maxt);
      valid = valid && (simple_obs_time(Y) - track_last_time(A,arr) > 1e-5);
      valid = valid && (track_num_obs(A) < max_obs);

      if(valid) {

        /* Actually try the track and make sure it is a good enough fit. */
        /* Also check the restrictions on max velocity.                  */
        B = mk_track_add_one_ind(A,ivec_ref(ord_pairs,i),arr);
        if ((track_num_obs(B) <= 2) ||
            (mean_track_residual_angle(B,arr) < thresh)) {

          /* No greedy replacement for length 1-2 tracks. */
          if ((track_num_obs(B) <= 3) || !greedy) {
            track_array_add(res,B);
          } else {
            track_array_set(res,j,B); /* Replace A with B */
            A = B;                    /* For safety */
          }
        }
        free_track(B);
      }

    }
  }

  /* Post filter all of the tracklets based on */
  /* the elongation length and angle.          */
  if((length != NULL)&&(angle != NULL)) {
    res2 = mk_empty_track_array(track_array_size(res));

    for(i=0;i<track_array_size(res);i++) {
      A     = track_array_ref(res,i);
      vel   = linear_track_estimated_angular_vel(A);
      inds  = track_individs(A);
      Y     = track_last(A,arr);
      valid = (track_size(A) > 1);

      /* Check each detection for compatible length. */
      for(j=0;(j<track_size(A))&&(valid);j++) {
        Yind = ivec_ref(inds,j);
        if(dyv_ref(length,Yind) >= 0.0) {
          estMinV = (dyv_ref(length,Yind) - maxLerr) / curr_etime;
          estMaxV = (dyv_ref(length,Yind) + maxLerr) / curr_etime;
          valid   = (estMinV <= vel)&&(estMaxV >= vel);
        }
      }

      /* Check each detection for compatible angle. */
      if(valid) {
        dR = 15.0*(simple_obs_RA(Y)  - simple_obs_RA(X));
        if(dR >  180.0) { dR -= 360.0; }
        if(dR < -180.0) { dR += 360.0; }
        dD = simple_obs_DEC(Y) - simple_obs_DEC(X);
        ang1 = atan2(dR,dD);
        if(ang1 < 0.0) { ang1 += 2.0*PI; }

        /* Only check the angle if the detection          */
        /* is long enough that the angle means something! */
        for(j=0;(j<track_size(A))&&(valid);j++) {
          Yind = ivec_ref(inds,j);
          if(dyv_ref(length,Yind)-2.0*maxLerr > 0.0) {
            ang2 = dyv_ref(angle,Yind);
            if(ang2 < 0.0) { ang2 += 2.0 * PI; }

            diff = fabs(ang1 - ang2);
            if(diff > 3.0*PI/2.0) { diff = fabs(diff - 2.0*PI); }
            if(diff > PI/2.0)     { diff = fabs(diff - PI);     }
            valid = (diff < athresh);
          }
        }
      }

      if(valid) { track_array_add(res2,A); } 
    }

    free_track_array(res);
    res = res2;
  }

  free_dyv(times);
  free_ivec(pairs);
  free_ivec(order);
  free_ivec(ord_pairs);

  return res;
}


track_array* mk_tracklets_MHT(simple_obs_array* arr, double minv, double maxv,
                              double thresh, double maxt, int min_size,
                              bool remove_subsets,
                              dyv* angle, dyv* length, dyv* exp_time,
                              double athresh, double maxLerr, double etime,
                              int max_obs, bool greedy, bool use_pht) {
  track_array* res = mk_empty_track_array(10);
  track_array* subres;
  track*       T;
  rdt_tree* tr;
  int i, j;

  /* Create the RDT tree */
  tr = mk_rdt_tree(arr,NULL,FALSE,RDT_MAX_LEAF_NODES);

  for(i=0;i<simple_obs_array_size(arr);i++) {
    if (!use_pht) {
      subres = mk_tracklets_single_query(arr, i, tr, minv, maxv, thresh, maxt,
                                         angle, length, exp_time, athresh,
                                         maxLerr, etime, remove_subsets,
                                         max_obs, greedy);
    } else {
      subres = mk_tracklets_single_query_PHT(arr, i, tr,  minv, maxv, thresh,
                                             maxt, angle, length, exp_time,
                                             athresh, maxLerr, etime,
                                             remove_subsets, max_obs, min_size,
                                             greedy);
    }

    for(j=0;j<track_array_size(subres);j++) {
      T = track_array_ref(subres,j);
      if(track_num_obs(T) >= min_size) {
        track_array_add(res,T);
      }
    }
   
    free_track_array(subres);
  }

  if(remove_subsets) {
    subres = mk_tracklet_remove_subsets(res,arr);
    free_track_array(res);
    res = subres;
  }

  free_rdt_tree(tr);
  
  return res;
}


/* --- Functions for removing overlaps ----------------------------- */

track_array* mk_tracklet_remove_subsets(track_array* old, simple_obs_array* obs) {
  track_array* res;
  track *X;
  ivec_array*  occur;
  ivec* sizes;
  ivec* inds;
  ivec* hit;
  int N = track_array_size(old);
  int i;

  /* Allocate space */
  occur = mk_array_of_zero_length_ivecs(simple_obs_array_size(obs));
  res   = mk_empty_track_array(N);
  sizes = mk_zero_ivec(N);

  /* Compute the sizes for all of the tracks. */
  for(i=0;i<N;i++) {
    ivec_set(sizes,i,track_num_obs(track_array_ref(old,i)));
  }
  inds = mk_indices_of_sorted_ivec(sizes);

  /* Put tracks in the result largest to smallest */
  for(i=0;i<N;i++) {
    X   = track_array_ref(old,ivec_ref(inds,N-i-1));
    hit = mk_tracklet_subset_query(occur,track_individs(X));

    if(ivec_size(hit) == 0) {
      track_array_add(res,X);
      tracklet_add_to_overlap_array(occur, track_individs(X), track_array_size(res)-1);
    }

    free_ivec(hit);
  }

  free_ivec_array(occur);
  free_ivec(sizes);
  free_ivec(inds);

  return res;
}


/* Given an ivec array of observation->tracks it is in  */
/* AND the observations of a new track, determine which */
/* tracks the new track is a subset of.                 */
ivec* mk_tracklet_subset_query(ivec_array* seen, ivec* pts) {
  ivec* res;
  ivec* occr;
  ivec* temp;
  int i;

  occr = ivec_array_ref(seen,ivec_ref(pts,0));
  res  = mk_copy_ivec(occr);
  for(i=1;i<ivec_size(pts);i++) {
    occr = ivec_array_ref(seen,ivec_ref(pts,i));
    temp = mk_sivec_intersection(occr,res);

    free_ivec(res);
    res = temp;
  }

  return res;
}


/* Given an ivec array of observation->tracks it is in  */
/* AND a new track, add the new track to the array.     */
void tracklet_add_to_overlap_array(ivec_array* seen, ivec* pts, int index) {
  ivec* occr;
  int i;

  for(i=0;i<ivec_size(pts);i++) {
    occr = ivec_array_ref(seen,ivec_ref(pts,i));
    add_to_sivec(occr,index);
  }
}
