/*
File:        MHT.c
Author:      J. Kubica
Created:     Tue June 15, 2004  
Description: A simple version of Multi-hypothesis tracking.

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

#include "MHT.h"
#include "t_tree.h"
#include "rdvv_tree.h"

extern int rdvv_count;

track_array* mk_order_tracks_by_trust(track_array* arr, simple_obs_array* obs) {
  track_array* res;
  track* X;
  dyv* scores;
  ivec* inds;
  double max;
  int i, N;

  /* Very basic error checking... */
  if(track_array_size(arr) <= 1) {
    res = mk_copy_track_array(arr);
    return res;
  }

  N      = track_array_size(arr);
  scores = mk_zero_dyv(N);

  /* Fill the score count... */
  for(i=0; i < N; i++) {
    X = track_array_ref(arr,i);
    dyv_set(scores,i,mean_sq_track_residual(X,obs));
  }

  /* Augment the scores with the size... */
  max = (double)((int)dyv_max(scores)+1);
  for(i=0;i<N;i++) {
    X = track_array_ref(arr,i);
    dyv_set(scores,i,-(double)track_num_obs(X)*max+dyv_ref(scores,i));
  }

  /* Sort on the new scores... */
  inds = mk_indices_of_sorted_dyv(scores);

  /* Create the new track array */
  res = mk_empty_track_array(N);
  for(i=0;i<N;i++) {
    X = track_array_ref(arr,ivec_ref(inds,i));
    track_array_add(res,X);
  }

  free_dyv(scores);
  free_ivec(inds);

  return res;
}


/* Given an ivec array of observation->tracks it is in  */
/* AND the observations of a new track, determine which */
/* tracks the new track intersects.                     */
ivec* mk_MHT_overlap_query(ivec_array* seen, ivec* pts) {
  ivec* res = mk_ivec(0);
  ivec* occr;
  ivec* temp;
  int i;

  for(i=0;i<ivec_size(pts);i++) {
    occr = ivec_array_ref(seen,ivec_ref(pts,i));
    temp = mk_sivec_union(occr,res);
    
    free_ivec(res);
    res = temp;
  }

  return res;
}


/* Given an ivec array of observation->tracks it is in  */
/* AND the observations of a new track, determine which */
/* tracks the new track is a subset of.                 */
ivec* mk_MHT_subset_query(ivec_array* seen, ivec* pts) {
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
void MHT_add_to_overlap_array(ivec_array* seen, ivec* pts, int index) {
  ivec* occr;
  int i;

  for(i=0;i<ivec_size(pts);i++) {
    occr = ivec_array_ref(seen,ivec_ref(pts,i));
    add_to_sivec(occr,index);
  }
}


track_array* mk_MHT_remove_overlaps(track_array* old, simple_obs_array* obs,
				    bool allow_conflicts,
				    double min_percentage_overlap) {
  track_array* sorted;
  track_array* res;
  ivec_array*  occur;
  ivec* inds;
  track *A, *B, *C;
  int N = track_array_size(old);
  int i, j, a, b, overlap;
  bool valid, found, subset;

  /* Put the track array in "trusted" order and allocate space */
  sorted = mk_order_tracks_by_trust(old, obs);
  occur  = mk_array_of_zero_length_ivecs(simple_obs_array_size(obs));
  res    = mk_empty_track_array(N);

  /* Put in tracks most trusted first and check if there is any overlaps */
  for(i=0;i<N;i++) {
    found  = FALSE;
    subset = FALSE;
    A      = track_array_ref(old,i);
    inds   = mk_MHT_overlap_query(occur,track_individs(A));

    for(j=0;j<ivec_size(inds);j++) {
      B = track_array_ref(res,ivec_ref(inds,j));
      a = track_num_obs(A);
      b = track_num_obs(B);

      overlap = track_overlap_size(A,B,obs);
      valid   = ((double)(2*overlap)/(double)(a+b) >= min_percentage_overlap);
      subset  = subset || track_subset(B,A,obs);

      if((allow_conflicts == FALSE)&&(valid==TRUE)&&(subset==FALSE)) {
	valid = (valid && track_valid_overlap(A,B,obs));
      }
      found = found || valid;

      if((valid==TRUE)&&(subset==FALSE)) {
	C = mk_combined_track(A,B,obs);

        track_array_set(res,ivec_ref(inds,j),C);
	MHT_add_to_overlap_array(occur, track_individs(C), ivec_ref(inds,j));

	free_track(C);
      }
    }

    /* If no valid overlaps where found... add the origional point. */
    if((found == FALSE)&&(subset==FALSE)) {
      track_array_add(res,A);
      MHT_add_to_overlap_array(occur, track_individs(A), track_array_size(res)-1);
    }

    free_ivec(inds);
  }

  free_ivec_array(occur);
  free_track_array(sorted);

  return res;
}


track_array* mk_MHT_remove_subsets(track_array* old, simple_obs_array* obs) {
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
    hit = mk_MHT_subset_query(occur,track_individs(X));
    
    if(ivec_size(hit) == 0) {
      track_array_add(res,X);
      MHT_add_to_overlap_array(occur, track_individs(X), track_array_size(res)-1);
    }

    free_ivec(hit);
  }

  free_ivec_array(occur);
  free_ivec(sizes);
  free_ivec(inds);

  return res;
}



track_array* mk_linear_matches_recursive(track_array* arr, simple_obs_array* obs,
					 dyv* times, int t, int max_hyp, int indiv_max_hyp,
					 double fit_thresh_rd, track_array* curr_hyp, t_tree* tr,
					 dyv* midpt_thresh, dyv* near_thresh, dyv* accel_thresh,
					 int dir) {
  track_array* next_hyp;
  track_array* temp;
  track_array* res;
  track *A, *B, *C;
  double time, fit_rd;
  ivec* inds;
  ivec *tempinds, *nuinds;
  dyv* scores;
  int i,j,N,M;

  /* The base condition is that we are off the edge of time */
  if((t >= dyv_size(times))||(t < 0)) {
    return mk_copy_track_array(curr_hyp);
  }

  /* Grab the initial information and allocate result space. */
  time     = dyv_ref(times,t);
  N        = track_array_size(curr_hyp);
  next_hyp = mk_copy_track_array(curr_hyp);

  /* For EACH hypothesis, try projecting it ahead. */
  for(i=0;i<N;i++) {

    /* Use different searches for the linear and quadratic projections */
    A  = track_array_ref(curr_hyp,i);

    /* WARNING changed performance!!! */
    if(track_time_length(A,obs) < 0.5) {
      /*if(i == 0) { */
      /*inds  = mk_rdvv_find_midpt(rdvv, arr, A, time-1e-6,time+1e-6,lin_thresh);*/
      inds = mk_t_tree_find_midpt(tr,arr,A,time-1e-6,time+1e-6,midpt_thresh,accel_thresh);
    } else {
      /*inds = mk_rd_near_bounded_track_range(rd,end_pts,A,time-1e-6,time+1e-6,quad_thresh);*/

      track_force_t0(A,time);
      inds = mk_t_tree_near_point(tr,arr,A,near_thresh);
      track_force_t0_first(A,obs);
    }  
    M = ivec_size(inds);

    /* Check if we have too many potential matches for this one individual */
    if(M > indiv_max_hyp) {

      /* Find the scores of those hypothesis... */
      scores = mk_dyv(M);
      for(j=0;j<M;j++) {
	B = track_array_ref(arr,ivec_ref(inds,j));
	if(i>0) {
	  dyv_set(scores,j,mean_sq_second_track_residual(A,B,obs));
	} else {
	  dyv_set(scores,j,track_midpt_distance(A,B));
	}
      }

      /* Sort the scores and copy the best into a new set of indices */
      tempinds = mk_indices_of_sorted_dyv(scores);
      nuinds   = mk_ivec(indiv_max_hyp);

      for(j=0;j<indiv_max_hyp;j++) {
	ivec_set(nuinds,j,ivec_ref(inds,ivec_ref(tempinds,j)));
      }
      
      free_ivec(inds);
      free_ivec(tempinds);
      free_dyv(scores);

      inds = nuinds;
      M    = indiv_max_hyp;
    }


    /* For each initial match, try actually fitting the function */
    for(j=0;j<M;j++) {
      B  = track_array_ref(arr,ivec_ref(inds,j));

      /* Make sure the tracks do not actually overlap! */
      if(track_overlap_in_time(A,B,obs) == FALSE) {
	C  = mk_combined_track(A,B,obs);
	fit_rd = mean_sq_track_residual(C,obs);
	
	if(fit_rd < fit_thresh_rd) {
	  track_array_add(next_hyp,C);
	}

	free_track(C);
      }
    }

    free_ivec(inds);
  }

  /* Check that the maximum number of hypothesis is not exceeded */
  /* and if so, do some pruning.                                 */
  if(track_array_size(next_hyp) > max_hyp) {
    temp = mk_order_tracks_by_trust(next_hyp,obs);
    free_track_array(next_hyp);

    /* Create a new array and add the initial stub. */
    next_hyp = mk_empty_track_array(max_hyp);
    track_array_add(next_hyp,track_array_ref(arr,0));
    
    /* Add the next best max_hyp-1 hypothesis */
    for(j=0;j<max_hyp-1;j++) {
      track_array_add(next_hyp,track_array_ref(temp,j));
    }

    free_track_array(temp);
  }

  /* Recursively continue to look for points. */
  res = mk_linear_matches_recursive(arr, obs, times, t+dir, max_hyp, indiv_max_hyp, 
				    fit_thresh_rd, next_hyp, tr, midpt_thresh, 
				    near_thresh, accel_thresh, dir);

  free_track_array(next_hyp);

  return res;
}



/* There are several tuning parameters:                                 */
/*  lin_thresh    - the fit threshold of the linear projection          */
/*  quad_thresh   - the fit threshold of the quadratic projection       */
/*  fit_thresh    - the fit threshold for the fitted curve.             */
/*  max_hyp       - the maximum number of hypothesis after each turn.   */
/*  indiv_max_hyp - the maximum number of matches for each hypothesis   */
/*                  to consider.                                        */
/*  min_obs       - the minimum number of observations for a valid track*/
track_array* mk_MHT_matches(track_array* arr, simple_obs_array* obs, 
			    double fit_rd, double mid_rd, double quad_rd,
			    int max_hyp, int indiv_max_hyp, int min_obs,
			    bool bwpass) {
  t_tree*      tr;
  track_array* single_res;
  track_array* init_hyp;
  track_array* res;
  track* X;
  ivec*        inds;
  dyv* times;
  dyv* weights;
  dyv* midpt_thresh;
  dyv* accel_thresh;
  dyv* nearpt_thresh;
  int i, j, t, N;
  /*char* tmpstr;*/

  N   = track_array_size(arr);
  res = mk_empty_track_array(N);

  weights = mk_t_tree_weights(1.0,1.0,1.0,1.0,1.0,0.0);
  tr      = mk_t_tree(arr,obs,weights,25); 
  /*tr      = mk_t_tree(arr,obs,weights,10000000);*/

  accel_thresh  = mk_t_tree_accel(1.0,0.02,0.3,10.0,10.0,10.0);
  midpt_thresh  = mk_t_tree_RADEC_thresh(mid_rd);
  nearpt_thresh = mk_t_tree_RADEC_thresh(quad_rd);

  times = mk_sort_all_track_times(arr);
  inds  = mk_ivec(1);

  for(i=0;i<N;i++) {
    if(i % 1000 == 0) {
      printf("Tracked %i\n",i);
    }

    ivec_set(inds,0,i);
    init_hyp = mk_track_array_subset(arr,inds);
    t        = find_index_in_dyv(times,track_time(track_array_ref(arr,i)),1e-8);
    single_res = mk_linear_matches_recursive(arr,obs,times,t+1, max_hyp,indiv_max_hyp,
					     fit_rd,init_hyp,tr,midpt_thresh,
					     nearpt_thresh,accel_thresh,1);
    for(j=0;j<track_array_size(single_res);j++) {
      X = track_array_ref(single_res,j);

      if(track_num_obs(X) >= min_obs) {
	track_array_add(res,X);
      }
    }

    /*tmpstr = simple_obs_id_str(track_first(track_array_ref(init_hyp,0),obs));
    if((tmpstr[0]=='S') && (tmpstr[1]=='0') && (tmpstr[2]=='0') && (tmpstr[3]=='0') &&
       (tmpstr[4]=='4') && (tmpstr[5]=='u') && (tmpstr[6]=='g')) { 
      fprintf_track_array_list(stdout,init_hyp);
      fprintf_track_array_mod_MPC(stdout,init_hyp,obs);
      fprintf_track_array(stdout,init_hyp);
      printf("----------\n");
      fprintf_track_array_list(stdout,single_res);
      fprintf_track_array_mod_MPC(stdout,single_res,obs);
      fprintf_track_array(stdout,single_res);
      wait_for_key();  
      }*/

    free_track_array(single_res);
    free_track_array(init_hyp);

    /* Do the same search backward... */
    if(bwpass) {
      init_hyp = mk_track_array_subset(arr,inds);
      single_res = mk_linear_matches_recursive(arr,obs,times,t-1, max_hyp,indiv_max_hyp,
					       fit_rd,init_hyp,tr,
					       midpt_thresh,nearpt_thresh,accel_thresh,-1);
      for(j=0;j<track_array_size(single_res);j++) {
	X = track_array_ref(single_res,j);

	if(track_num_obs(X) >= min_obs) {
	  track_array_add(res,X);
	}
      }
      /*fprintf_track_array_list(stdout,single_res);
	wait_for_key();*/

      /*tmpstr = simple_obs_id_str(track_first(track_array_ref(init_hyp,0),obs));
      if((tmpstr[0]=='S') && (tmpstr[1]=='0') && (tmpstr[2]=='0') && (tmpstr[3]=='0') &&
	 (tmpstr[4]=='4') && (tmpstr[5]=='u') && (tmpstr[6]=='g')) { 
	fprintf_track_array_list(stdout,init_hyp);
	fprintf_track_array_mod_MPC(stdout,init_hyp,obs);
	fprintf_track_array(stdout,init_hyp);
	printf("----------\n");
	fprintf_track_array_list(stdout,single_res);
	fprintf_track_array_mod_MPC(stdout,single_res,obs);
	fprintf_track_array(stdout,single_res);
	wait_for_key();  
	}*/

      free_track_array(single_res);
      free_track_array(init_hyp); 
    }
  }

  free_t_tree(tr);
  free_dyv(times);
  free_dyv(midpt_thresh);
  free_dyv(accel_thresh);
  free_dyv(nearpt_thresh);
  free_dyv(weights);
  free_ivec(inds);

  return res;
}



/* ------------------------------------------------------------------- */
/* --- Dual tree based MHT ------------------------------------------- */
/* ------------------------------------------------------------------- */

void check_leaves_given(track_array* arr, simple_obs_array* obs, track* query,  
			rdvv_tree** trees, int num_trees, double thresh,
		        int K, int Kmax, track_array* res) {
  track* X;
  ivec* inds;
  ivec* inds_o;
  double t0;
  int i, j, trck;
  bool err = FALSE;

  if((num_trees >= K)&&(num_trees <= Kmax)) {
    inds_o = track_individs(query);
    inds   = mk_copy_ivec(inds_o);
    t0     = track_time(query);

    for(i=0;i<num_trees;i++) {
      trck   = ivec_ref(rdvv_tracks(trees[i]),0);
      inds_o = track_individs(track_array_ref(arr,trck));

      err = err || (track_time(track_array_ref(arr,trck)) <= t0);
  
      for(j=0;j<ivec_size(inds_o);j++) {
	add_to_ivec(inds,ivec_ref(inds_o,j));
      }
    }

    if(err==FALSE) {
      X = mk_track_from_N_inds(obs,inds);
      /*fprintf_track_list(stdout,"Checking ",X,"");
	printf(" = %15.12f (%15.12f)\n",mean_sq_track_residual(X,obs),max_sq_track_residual(X,obs));*/

      if(mean_sq_track_residual(X,obs) < thresh) {
        /*fprintf_track_list(stdout,"",X,"");*/

	track_array_add(res,X);
      }

      free_track(X);
    }

    free_ivec(inds);

  }
}


bool check_rdvv_midpt(rdvv_tree* tr, track* X, double thresh, double max_acc, int verb) {
  bool prune     = FALSE;
  double r_min, r_max, r_mid, r_rad; 
  double d_min, d_max, d_mid, d_rad;
  double aminR, amaxR, aminD, amaxD;
  double ts, te, tq;
  double a,b,c,d;
  double dist;
  
  /* Decide whether or not to prune: check time */
  tq = track_time(X);
  ts = RDVV_SIMPLE_MAX(tq + 1e-5, rdvv_lo_time(tr));
  te = rdvv_hi_time(tr);
  prune = (ts > te);

  /* Decide to bound velocity IFF the track is not */
  /* INSIDE the query box. */
  if(prune == FALSE) {

    /* Place bounds on the RA accel */
    a = (rdvv_lo_vra(tr) - track_vRA(X))/(ts - tq);
    b = (rdvv_hi_vra(tr) - track_vRA(X))/(ts - tq);
    c = (rdvv_lo_vra(tr) - track_vRA(X))/(te - tq);
    d = (rdvv_hi_vra(tr) - track_vRA(X))/(te - tq);
    aminR = RDVV_SIMPLE_MIN(RDVV_SIMPLE_MIN(a,b),RDVV_SIMPLE_MIN(c,d));
    amaxR = RDVV_SIMPLE_MAX(RDVV_SIMPLE_MAX(a,b),RDVV_SIMPLE_MAX(c,d));

    aminR = RDVV_SIMPLE_MAX(aminR,-max_acc/15.0);
    amaxR = RDVV_SIMPLE_MIN(amaxR, max_acc/15.0);

    /* Place bounds on the RA positions that could be hit... */
    a = track_RA(X) + (ts-tq)*track_vRA(X) + 0.5*(ts-tq)*(ts-tq)*aminR;
    b = track_RA(X) + (ts-tq)*track_vRA(X) + 0.5*(ts-tq)*(ts-tq)*amaxR;
    c = track_RA(X) + (te-tq)*track_vRA(X) + 0.5*(te-tq)*(te-tq)*aminR;
    d = track_RA(X) + (te-tq)*track_vRA(X) + 0.5*(te-tq)*(te-tq)*amaxR;
    r_min = RDVV_SIMPLE_MIN(RDVV_SIMPLE_MIN(a,b),RDVV_SIMPLE_MIN(c,d));
    r_max = RDVV_SIMPLE_MAX(RDVV_SIMPLE_MAX(a,b),RDVV_SIMPLE_MAX(c,d));
    r_mid = RDVV_MID(r_min,r_max);
    r_rad = RDVV_RAD(r_min,r_max);

    dist = fabs(r_mid - rdvv_mid_ra(tr));
    while(dist > 12.0) { dist = fabs(dist - 24.0); }
    prune = (dist - r_rad - rdvv_rad_ra(tr) > thresh/15.0);
  }

  /* Decide to bound velocity IFF the track is not */
  /* INSIDE the query box. */
  if(prune == FALSE) {

    /* Place bounds on the DEC accel */
    a = (rdvv_lo_vdec(tr) - track_vDEC(X))/(ts - tq);
    b = (rdvv_hi_vdec(tr) - track_vDEC(X))/(ts - tq);
    c = (rdvv_lo_vdec(tr) - track_vDEC(X))/(te - tq);
    d = (rdvv_hi_vdec(tr) - track_vDEC(X))/(te - tq);
    aminD = RDVV_SIMPLE_MIN(RDVV_SIMPLE_MIN(a,b),RDVV_SIMPLE_MIN(c,d));
    amaxD = RDVV_SIMPLE_MAX(RDVV_SIMPLE_MAX(a,b),RDVV_SIMPLE_MAX(c,d));

    aminD = RDVV_SIMPLE_MAX(aminD,-max_acc);
    amaxD = RDVV_SIMPLE_MIN(amaxD,max_acc);

    /* Place bounds on the DEC positions that could be hit... */
    a = track_DEC(X) + (ts-tq)*track_vDEC(X) + 0.5*(ts-tq)*(ts-tq)*aminD;
    b = track_DEC(X) + (ts-tq)*track_vDEC(X) + 0.5*(ts-tq)*(ts-tq)*amaxD;
    c = track_DEC(X) + (te-tq)*track_vDEC(X) + 0.5*(te-tq)*(te-tq)*aminD;
    d = track_DEC(X) + (te-tq)*track_vDEC(X) + 0.5*(te-tq)*(te-tq)*amaxD;
    d_min = RDVV_SIMPLE_MIN(RDVV_SIMPLE_MIN(a,b),RDVV_SIMPLE_MIN(c,d));
    d_max = RDVV_SIMPLE_MAX(RDVV_SIMPLE_MAX(a,b),RDVV_SIMPLE_MAX(c,d));
    d_mid = RDVV_MID(d_min,d_max);
    d_rad = RDVV_RAD(d_min,d_max);

    dist = fabs(d_mid - rdvv_mid_dec(tr));
    prune = (dist - d_rad - rdvv_rad_dec(tr) > thresh);
  }

  return (prune==FALSE);
}



void MHT_tree_feasiable(track* X, simple_obs_array* obs, rdvv_tree* tree, 
			double thresh, double lin_thresh, double max_acc,
		        double* a_min_d, double* a_max_d,
		        double* a_min_r, double* a_max_r, bool verb) {
  double a_min_x = 0.0;
  double a_max_x = 0.0;
  double a_min_v = 0.0;
  double a_max_v = 0.0;
  double xb, tb, xq, vq, vb;
  double ts_b, te_b, ts_q, te_q, t_dt;
  double diff, val;
  int xb_i, tb_i, xq_i, vq_i, vb_i;

  /* Set the default values... */
  a_min_d[0] = 1.0;
  a_max_d[0] = 0.0;
  a_min_r[0] = 1.0;
  a_max_r[0] = 0.0;

  /* Find the times and make sure that the box starts */
  /* at least 1 second (1e-5 days) after the query.   */
  ts_b = rdvv_lo_time(tree);
  te_b = rdvv_hi_time(tree);
  ts_q = track_first_time(X,obs);
  te_q = track_last_time(X,obs);
  t_dt = te_q - ts_q; 
  if(ts_b < te_q + 1e-5) { ts_b = te_q + 1e-5; }
  if(t_dt < 1e-5)        { t_dt = 1e-5; }
  
  
  /* Check that the points satisfy midpoint constraints... */
  if(check_rdvv_midpt(tree, X, lin_thresh, max_acc, 0)==FALSE) {
	ts_b = te_b + 1.0;
  }

  /* We can prune immediately if the entire box occurs BEFORE     */
  /* the query, otherwise we need to look at acceleration ranges. */
  if(ts_b <= te_b) {

    /* Given an allowable error of thresh, what accels would   */
    /* be required to get the velocity into the correct bounds */
    /* AND the position into the correct bounds.               */
    for(tb_i = 0; tb_i < 2; tb_i++) {
      tb = (tb_i == 0) ? (ts_b-ts_q) : (te_b-ts_q);
       
      for(vq_i = 0; vq_i < 2; vq_i++) {
        vq = (vq_i == 0) ? (-2.0*thresh/t_dt) : (2.0*thresh/t_dt);
        vq = vq + track_vDEC(X);

	/* Check against the velocity bounds... */
	for(vb_i = 0; vb_i < 2; vb_i++) {
	  vb = (vb_i == 0) ? (rdvv_lo_vdec(tree)-2.0*thresh/t_dt) : 
	    (rdvv_hi_vdec(tree)+2.0*thresh/t_dt);

          val = (vb - vq)/tb;

	  if((vb_i+tb_i+vq_i == 0)||(val > a_max_v)) { a_max_v = val; }
	  if((vb_i+tb_i+vq_i == 0)||(val < a_min_v)) { a_min_v = val; }
	  if(verb) { printf("DEC v: %12.8f -> [%12.8f, %12.8f]\n",val,a_min_v,a_max_v); }
	}

	/* Check against the position bounds... */
        for(xb_i = 0; xb_i < 2; xb_i++) {
          for(xq_i = 0; xq_i < 2; xq_i++) {
	    xb  = (xb_i == 0) ? (rdvv_lo_dec(tree)-thresh) : (rdvv_hi_dec(tree)+thresh);
	    xq  = (xq_i == 0) ? (track_DEC(X)-thresh) : (track_DEC(X)+thresh);

	    val = 2.0*(xb - xq - vq*tb)/(tb*tb);

	    if((tb_i+vq_i+xb_i+xq_i==0)||(val > a_max_x)) { a_max_x = val; }
  	    if((tb_i+vq_i+xb_i+xq_i==0)||(val < a_min_x)) { a_min_x = val; }
  	    if(verb) { printf("DEC x: %12.8f -> [%12.8f, %12.8f]\n",val,a_min_x,a_max_x); }
	  }
	}

      }
    }

    a_min_d[0] = RDVV_SIMPLE_MAX(RDVV_SIMPLE_MAX(a_min_v,a_min_x),-max_acc);
    a_max_d[0] = RDVV_SIMPLE_MIN(RDVV_SIMPLE_MIN(a_max_v,a_max_x),max_acc);
    if(verb) { printf("DEC: [%12.8f, %12.8f]\n",a_min_d[0],a_max_d[0]); }
  }
 

  if(a_min_d[0] <= a_max_d[0]) {

    /* Given an allowable error of thresh, what accels would   */
    /* be required to get the velocity into the correct bounds */
    /* AND the position into the correct bounds.               */
    for(tb_i = 0; tb_i < 2; tb_i++) {
      tb = (tb_i == 0) ? (ts_b-ts_q) : (te_b-ts_q);
       
      for(vq_i = 0; vq_i < 2; vq_i++) {
        vq = (vq_i == 0) ? (-2.0*(thresh/15.0)/t_dt) : (2.0*(thresh/15.0)/t_dt);
        vq = vq + track_vRA(X);

	/* Check against the velocity bounds... */
	for(vb_i = 0; vb_i < 2; vb_i++) {
	  vb = (vb_i == 0) ? (rdvv_lo_vra(tree)-2.0*(thresh/15.0)/t_dt) : 
	    (rdvv_hi_vra(tree)+2.0*(thresh/15.0)/t_dt);

          val = (vb - vq)/tb;

	  if((vb_i+tb_i+vq_i == 0)||(val > a_max_v)) { a_max_v = val; }
	  if((vb_i+tb_i+vq_i == 0)||(val < a_min_v)) { a_min_v = val; }
	  if(verb) { printf("RA v:  %12.8f -> [%12.8f, %12.8f]\n",val,a_min_v,a_max_v); }
	}

	/* Check against the position bounds... */
        for(xb_i = 0; xb_i < 2; xb_i++) {
          for(xq_i = 0; xq_i < 2; xq_i++) {
	    xb  = (xb_i == 0) ? (rdvv_lo_ra(tree)-thresh/15.0) : (rdvv_hi_ra(tree)+thresh/15.0);
	    xq  = (xq_i == 0) ? (track_RA(X)-thresh/15.0) : (track_RA(X)+thresh/15.0);

	    diff = xb - xq - vq*tb;
	    val = 2.0*diff/(tb*tb);

	    if((tb_i+vq_i+xb_i+xq_i==0)||(val > a_max_x)) { a_max_x = val; }
  	    if((tb_i+vq_i+xb_i+xq_i==0)||(val < a_min_x)) { a_min_x = val; }
  	    if(verb) { printf("RA x:  %12.8f -> [%12.8f, %12.8f]\n",val,a_min_x,a_max_x); }
	  }
	}

      }
    }

    a_min_r[0] = RDVV_SIMPLE_MAX(RDVV_SIMPLE_MAX(a_min_v,a_min_x),-max_acc/15.0);
    a_max_r[0] = RDVV_SIMPLE_MIN(RDVV_SIMPLE_MIN(a_max_v,a_max_x),max_acc/15.0);
    if(verb) { printf("RA: [%12.8f, %12.8f]\n",a_min_r[0],a_max_r[0]); }
  }

  /*if((verb==FALSE)&&((a_min_r[0] > a_max_r[0])||(a_max_d[0] < a_min_d[0]))) {
    MHT_tree_feasiable(X,obs,tree,thresh,max_acc,a_min_d,a_max_d,a_min_r,a_max_r,TRUE);
    wait_for_key();
    }*/
}



void segs_trees_recurse_given(track_array* arr, track* query, simple_obs_array* obs, 
			      rdvv_tree** trees, int num_trees, double thresh1, double thresh2, double lin_thresh,
			      int K, int Kmax, track_array* res, double max_acc,
			      double old_min_r, double old_max_r, 
			      double old_min_d, double old_max_d) {
  rdvv_tree** next_trees;
  rdvv_tree*  curr;
  bool valid   = TRUE;
  bool leaves  = TRUE;
  bool no_time = TRUE;
  bool leaf;
  double max_r, min_r, max_d, min_d;
  double r_max_r, r_min_r, r_max_d, r_min_d;
  double l_max_r, l_min_r, l_max_d, l_min_d;
  int tot_pts = 0;
  int num_pts = 0;
  int max_pts = 0;
  int max_ind = -1;
  int i, size;

  /* Check if there are any non-leaf nodes. */
  /* If so, find the largest to split on.   */
  for(i=0;i<num_trees;i++) {
    leaf     = rdvv_is_leaf(trees[i]);
    num_pts  = rdvv_num_points(trees[i]);
    no_time  = no_time && (rdvv_split_dim(trees[i])!=RDVV_T);

    tot_pts += num_pts;
    leaves   = leaves && leaf;
    if((leaf==FALSE)&&(num_pts > max_pts)) {
      max_pts = num_pts;
      max_ind = i;
    }
  }

  /* If everything is a leaf... then see if it forms a valid track. */
  if(leaves && (tot_pts >= K)) {
    check_leaves_given(arr,obs,query,trees,num_trees,thresh2,K,Kmax,res);
  } else {
    size = track_array_size(res);

    /* Check if this set of trees is feasiable... */
    valid = (max_ind >= 0);
    valid = valid && (tot_pts >= K);
    valid = valid && ((no_time==FALSE)||(num_trees >= K));

    if(valid) {
      curr = trees[max_ind];

      MHT_tree_feasiable(query, obs, rdvv_right_child(curr), thresh1, lin_thresh,
			   max_acc, &r_min_d, &r_max_d, &r_min_r, &r_max_r, FALSE);
      MHT_tree_feasiable(query, obs, rdvv_left_child(curr), thresh1, lin_thresh,
			   max_acc, &l_min_d, &l_max_d, &l_min_r, &l_max_r, FALSE);

      min_r = RDVV_SIMPLE_MAX(RDVV_SIMPLE_MAX(old_min_r,r_min_r),l_min_r);
      max_r = RDVV_SIMPLE_MIN(RDVV_SIMPLE_MIN(old_max_r,r_max_r),l_max_r);
      min_d = RDVV_SIMPLE_MAX(RDVV_SIMPLE_MAX(old_min_d,r_min_d),l_min_d);
      max_d = RDVV_SIMPLE_MIN(RDVV_SIMPLE_MIN(old_max_d,r_max_d),l_max_d);

      /* If the branching factor allows, try both... */
      if((num_trees < Kmax)&&(rdvv_split_dim(trees[max_ind])==RDVV_T)&&
	 (min_r <= max_r)&&(min_d <= max_d)) {
	/*if((num_trees < Kmax)&&(min_r <= max_r)&&(min_d <= max_d)) {*/
        next_trees = AM_MALLOC_ARRAY(rdvv_tree*,num_trees+1);

        for(i=0;i<num_trees;i++) { next_trees[i] = trees[i]; }

        next_trees[max_ind]   = rdvv_right_child(curr);
        next_trees[num_trees] = rdvv_left_child(curr);

        segs_trees_recurse_given(arr,query,obs,next_trees,num_trees+1,
				 thresh1,thresh2,lin_thresh,K,Kmax,res,max_acc,min_r,max_r,min_d,max_d);

        AM_FREE_ARRAY(next_trees,rdvv_tree*,num_trees+1);
      } 

      /* Try the right sub-tree... */
      min_r = RDVV_SIMPLE_MAX(old_min_r,r_min_r);
      max_r = RDVV_SIMPLE_MIN(old_max_r,r_max_r);
      min_d = RDVV_SIMPLE_MAX(old_min_d,r_min_d);
      max_d = RDVV_SIMPLE_MIN(old_max_d,r_max_d);
      if((min_r <= max_r)&&(min_d <= max_d)) {
	trees[max_ind] = rdvv_right_child(curr);
	segs_trees_recurse_given(arr,query,obs,trees,num_trees,thresh1,thresh2,lin_thresh,
				 K,Kmax,res,max_acc,min_r,max_r,min_d,max_d);
      } 

      /* Try the left sub-tree... */
      min_r = RDVV_SIMPLE_MAX(old_min_r,l_min_r);
      max_r = RDVV_SIMPLE_MIN(old_max_r,l_max_r);
      min_d = RDVV_SIMPLE_MAX(old_min_d,l_min_d);
      max_d = RDVV_SIMPLE_MIN(old_max_d,l_max_d);
      if((min_r <= max_r)&&(min_d <= max_d)) {
        trees[max_ind] = rdvv_left_child(curr);
        segs_trees_recurse_given(arr,query,obs,trees,num_trees,thresh1,thresh2,lin_thresh,
	  		         K,Kmax,res,max_acc,min_r,max_r,min_d,max_d);
      } 

      /* Reset the lines row. */
      trees[max_ind] = curr;

      /*if((MHT_trees_feasiable_given(query, obs, trees, num_trees, thresh, max_acc,FALSE) == FALSE)&&(size!=track_array_size(res))) { 
	MHT_trees_feasiable_given(query, obs, trees, num_trees, thresh, max_acc,TRUE);
	}*/

    }/* else {
      printf("Pruned %i (%i trees)\n",tot_pts,num_trees);
      }*/
  }
}


track_array* mk_MHT_all_matches(track_array* arr, simple_obs_array* obs, double thresh1,
				double thresh2, double lin_thresh, int K, int Kmax, double max_acc) {
  double max_r, min_r, max_d, min_d;
  rdvv_tree*   trees[1];
  rdvv_tree*   rdvv;
  track_array* single_res;
  track_array* res;
  int i, N;

  N    = track_array_size(arr);
  res  = mk_empty_track_array(N);
  rdvv = mk_rdvv_tree(arr,obs,1,TRUE);
  trees[0] = rdvv;

  for(i=0;i<N;i++) {
    if(i % 1000 == 0) {
      printf("Tracked %i\n",i);
    }

    MHT_tree_feasiable(track_array_ref(arr,i), obs, rdvv, thresh1, lin_thresh,
		       max_acc, &min_d, &max_d, &min_r, &max_r, FALSE);

    single_res = mk_empty_track_array(2);
    segs_trees_recurse_given(arr,track_array_ref(arr,i),obs,trees,1,thresh1,
			     thresh2,lin_thresh,K,Kmax,single_res,max_acc,min_r,max_r,min_d,max_d);


    /*printf("TRACKED --------------------------- %i\n",i);*/

    track_array_add_all(res,single_res);
    free_track_array(single_res);
  }

  free_rdvv_tree(rdvv);

  return res;
}




void test_MHT_main(int argc,char *argv[]) {
  dym* score;
  FILE* fp;
  char* fname = string_from_args("filename",argc,argv,NULL);
  double fit_thresh    = double_from_args("fit_thresh",argc,argv,0.0001);
  double lin_thresh    = double_from_args("lin_thresh",argc,argv,0.15);
  double quad_thresh   = double_from_args("quad_thresh",argc,argv,0.10);
  double sigma_pos     = double_from_args("sigma_pos",argc,argv,0.0);
  bool   eval          = bool_from_args("eval",argc,argv,TRUE);
  int    max_hyp       = int_from_args("max_hyp",argc,argv,500);
  int    max_match     = int_from_args("max_match",argc,argv,500);
  int    min_obs       = int_from_args("min_obs",argc,argv,7);
  int    num_rand      = int_from_args("num_rand",argc,argv,0);
  simple_obs_array* obs;
  ivec* true_groups;
  ivec_array* obs_to_track;
  track_array* t1;
  track_array* t2;
  track_array* t3;
  bool bwpass = TRUE;

  if(fname == NULL) {
    printf("ERROR: No filename given.\n");
  } else {

    obs         = mk_simple_obs_array_from_MPC_file(fname,0);
    true_groups = mk_simple_obs_groups_from_labels(obs, TRUE);

    if(sigma_pos > 1e-10) {
      printf(">> Adding noise (sigma = %f)...\n",sigma_pos);
      simple_obs_array_add_gaussian_noise(obs, sigma_pos/15.0, sigma_pos, 0.0);
    }

    printf(">> Doing initial same night pairing... cheating.\n");
    obs_to_track = mk_simple_obs_pairing_from_true_groups(obs,true_groups,0.5);
    t1 = mk_track_array_from_matched_simple_obs(obs,obs_to_track,2);
    free_ivec_array(obs_to_track);


    if(num_rand > 0) { track_array_add_random(t1,obs,num_rand,true_groups); }
    printf("   Found %i tracklets to match.\n",track_array_size(t1));

    printf(">> Doing the tracking...\n");
    t2 = mk_MHT_matches(t1,obs,fit_thresh,lin_thresh,quad_thresh,
			max_hyp,max_match,min_obs,bwpass);
    printf("   Found %i potential tracks of %i or more observations.\n",track_array_size(t2),
	   min_obs);

    printf(">> Removing subset/duplicate orbits...\n");
    t3 = mk_MHT_remove_subsets(t2,obs);
    free_track_array(t2);
    t2 = t3;
    printf("   Found %i unique tracks.\n",track_array_size(t2));

    printf(">> Removing Overlapping Orbits...\n");
    t3 = mk_MHT_remove_overlaps(t2,obs,FALSE,0.5);
    free_track_array(t2);
    t2 = t3;
    printf("   After merging there are %i tracks.\n",track_array_size(t2));

    printf(">> Putting Tracks in Trust Order...\n");
    t3 = mk_order_tracks_by_trust(t2, obs);
    free_track_array(t2);
    t2 = t3;

    if(eval == TRUE) {
      printf("\n\nPairwise: Per. Corr. = %f  Per. Found = %f  Per. Found2 = %f\n",
	     track_array_percent_correct(t2,true_groups,1),
	     track_array_percent_found(t2,true_groups,1),
	     track_array_percent_found(t2,true_groups,min_obs));

      /* Generate and save the ROC curve. */
      score = mk_track_array_ROC_curve(t2,true_groups,min_obs);
      fp = fopen("ROC.out","w");
      fprintf_dym(fp,"",score,"\n");
      fclose(fp);
      free_dym(score);
    }

    free_track_array(t2);
    free_track_array(t1);
    free_ivec(true_groups);
    free_simple_obs_array(obs);
  }
}
