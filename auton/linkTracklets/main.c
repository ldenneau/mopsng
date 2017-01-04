/*
   File:        main.c
   Author:      J. Kubica
   Description: The main function (and helper functions) for LinkTracklets.

   Copyright, The Auton Lab, CMU

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

#include "am_time.h"
#include "track.h"
#include "sb_graph.h"
#include "MHT.h"
#include "rdt_tree.h"
#include "linker.h"

#define NEOS_VERSION 3
#define NEOS_RELEASE 0
#define NEOS_UPDATE  2


ivec* mk_count_tracks_iv_ind(ivec* grps) {
  ivec_array* ivind;
  ivec* counts;
  int max = 0;
  int G = ivec_max(grps)+1;
  int N = ivec_size(grps);
  int i, g;

  ivind = mk_zero_ivec_array(G);

  /* Build the tracks from group labels. */
  for(i=0;i<N;i++) {
    g = ivec_ref(grps,i);
    if(g >= 0) {
      add_to_ivec_array_ref(ivind,g,i);
      if(ivec_array_ref_size(ivind,g) > max) {
        max = ivec_array_ref_size(ivind,g);
      }
    }
  }

  /* Count the sizes of the tracks. */
  counts = mk_zero_ivec(max+1);
  for(i=0;i<G;i++) {
    g = ivec_array_ref_size(ivind,i);
    ivec_set(counts,g,ivec_ref(counts,g)+1);
  }

  free_ivec_array(ivind);

  return counts;  
}



void test_main(int argc,char *argv[]) {
  char* fname  = string_from_args("file",argc,argv,NULL);
  FILE* fout;
  simple_obs_array* obs;  
  ivec* true_groups;
  ivec* true_pairs;
  track_array* t1;
  track_array* t2;
  track*       F;
  track*       S;
  double r,d,vr,vd;
  int grp1, grp2;
  int i, j;

  obs = mk_simple_obs_array_from_file(fname,0.5,&true_groups,&true_pairs);

  t1 = mk_true_tracks(obs,true_pairs);
  t2 = mk_true_tracks(obs,true_groups);

  fout = fopen("res.out","w");

  for(i=0;i<track_array_size(t2);i++) {
    F    = track_array_ref(t2,i);
    grp1 = ivec_ref(true_groups,simple_obs_id(track_first(F,obs)));

    if(i % 100 == 0) { printf("%6i\n",i); }

    for(j=0;j<track_array_size(t1);j++) {
      S    = track_array_ref(t1,j);
      grp2 = ivec_ref(true_groups,simple_obs_id(track_first(S,obs)));
      
      if(grp1 == grp2) {
        if((track_num_obs(S) > 1)&&(track_num_obs(F) >= 6)) {
          fprintf(fout,"%4i %4i ",grp1,simple_obs_id(track_first(S,obs)));

          fprintf(fout,"%10f %10f %10f %10f %10f %10f ",track_RA(F),track_DEC(F),
                  track_vRA(F),track_vDEC(F),track_aRA(F),track_aDEC(F));
          fprintf(fout,"%10f %10f %10f %10f ",track_RA(S),track_DEC(S),
                  track_vRA(S),track_vDEC(S));

          track_RDVV_prediction(F,track_time(S)-track_time(F),&r,&d,&vr,&vd);

          fprintf(fout,"%10f %10f %10f ",r,d,
                  angular_distance_RADEC(track_RA(S),r,
                                         track_DEC(S),d));

          fprintf(fout,"%10f %10f\n",fabs(vr-track_vRA(S)),
                  fabs(vd-track_vDEC(S)));
        }
      }

    }  

  }

  fclose(fout);
}


void tracker_main(int argc,char *argv[]) {
  FILE* f1;
  FILE* f3;
  char* fname  = string_from_args("file",argc,argv,NULL);
  char* desfname  = string_from_args("desfile",argc,argv,NULL);
  char* fout1  = string_from_args("trackfile",argc,argv,"tracks.obs");
  char* fout3  = string_from_args("summaryfile",argc,argv,"tracks.sum");
  char* fout4  = string_from_args("idsfile",argc,argv,"tracks.ids");
  char* fout5  = string_from_args("scoresfile",argc,argv,"");
  char* trackids_filename = string_from_args("trackidsfile",argc,argv,"");
  double fit_thresh    = double_from_args("fit_thresh",argc,argv,0.0001);
  double lin_thresh    = double_from_args("lin_thresh",argc,argv,0.05);
  double quad_thresh   = double_from_args("quad_thresh",argc,argv,0.02);
  double pred_thresh   = double_from_args("pred_thresh",argc,argv,0.0005);
  double vtree_thresh  = double_from_args("vtree_thresh",argc,argv,0.0002);
  double min_overlap   = double_from_args("min_overlap",argc,argv,0.50);
  double plate_width   = double_from_args("plate_width",argc,argv,0.001);
  double roc_thresh    = double_from_args("roc_thresh",argc,argv,1.0);
  double maxv          = double_from_args("maxv",argc,argv,1000.0);
  double minv          = double_from_args("minv",argc,argv,0.0);
  double sigma         = double_from_args("sigma",argc,argv,0.0);
  double acc_r         = double_from_args("acc_r",argc,argv,0.02);
  double acc_d         = double_from_args("acc_d",argc,argv,0.02);
  double end_t_range   = double_from_args("end_t_range",argc,argv,-1.0);
  double start_t_range = double_from_args("start_t_range",argc,argv,-1.0);
  int    seed          = int_from_args("seed",argc,argv,0);
  int    min_sup       = int_from_args("min_sup",argc,argv,3);
  int    max_hyp       = int_from_args("max_hyp",argc,argv,500);
  int    max_match     = int_from_args("max_match",argc,argv,500);
  int    min_obs       = int_from_args("min_obs",argc,argv,6);
  bool   bwpass        = bool_from_args("bwpass",argc,argv,TRUE);
  bool   endpts        = bool_from_args("endpts",argc,argv,TRUE);
  bool   eval          = bool_from_args("eval",argc,argv,FALSE);
  bool   removedups    = bool_from_args("remove_subsets",argc,argv,TRUE);
  bool   rem_overlap   = bool_from_args("remove_overlaps",argc,argv,FALSE);
  bool   allow_conflicts = bool_from_args("allow_conflicts",argc,argv,FALSE);
  bool   fileout       = bool_from_args("fileout",argc,argv,TRUE);
  double r_lo_n, r_hi_n, d_lo_n, d_hi_n, t_lo_n, t_hi_n;
  double r_lo, r_hi, d_lo, d_hi, t_lo, t_hi;
  double last_start_obs_time = -1.0;
  double first_end_obs_time  = -1.0;
  ivec* filtered_true_groups;
  ivec* true_groups;
  ivec* true_pairs;
  ivec* track_sizes;
  ivec* roc;
  dyv* org_times;
  dyv* times;
  simple_obs_array* obs;
  simple_obs* A;
  simple_obs* B;
  track_array* t1;
  track_array* t2;
  track_array* t3;
  double dist, dtime;
  int tcount = 0;
  int i;
  int search_type = 0;
  double percent_correct = 0.0;
  double percent_found = 0.0;
  int matches_found = 0;
  char *s = (argc < 2) ? "help" : argv[1];

  /* Set the random seed and the search mode */
  if(seed) am_srand(seed);
  if( eq_string(s,"vtree") )    { search_type = 0; }
  if( eq_string(s,"seq") )      { search_type = 1; }
  if( eq_string(s,"seqaccel") ) { search_type = 2; }

  printf("--------------------------------------------- \n");
  printf("NEOS VERSION: %i.%i.%i\n",NEOS_VERSION,NEOS_RELEASE,NEOS_UPDATE);
  printf("This program comes with ABSOLUTELY NO WARRANTY. This is free "
         "software, and you are welcome to redistribute it under certain "
         " conditions.  See included license for details.\n");
  printf("--------------------------------------------- \n\n");

  if(search_type == 0) { printf("Using VTREE search.\n"); } 
  if(search_type == 1) { printf("Using SEQUENTIAL search.\n"); }
  if(search_type == 2) { printf("Using SEQUENTIAL ACCEL search.\n"); }

  if(fname) {
    printf("Input file:           "); printf(fname); printf("\n");
  } else {
    printf("Input file:           <NOT GIVEN!>\n");
  }
  printf("Output file (tracks):  "); printf(fout1); printf("\n");
  printf("Output file (summary): "); printf(fout3); printf("\n");
  printf("Output file (ids):     "); printf(fout4); printf("\n");
  if((search_type == 0) || (search_type == 2)) {
    printf("VTREES Threshhold (RD)   = %12.8f   (default = 0.0002)\n",vtree_thresh);
    printf("Prediction Threshold (RD)= %12.8f   (default = 0.0005)\n",pred_thresh);
  } else {
    printf("Linear threshold (RD)    = %12.8f   (default = 0.05)\n",lin_thresh);
    printf("Quadratic threshold (RD) = %12.8f   (default = 0.02)\n",quad_thresh);
  }
  printf("Fit threshold (RD)       = %12.8f   (default = 0.0001)\n",fit_thresh);
  printf("Plate Width              = %12.8f   (default = 0.001)\n",plate_width);
  if (start_t_range > 0.0) {
    printf("Start Time Range (day)   = %12.8f   (default = ignored)\n",start_t_range);
  } else {
        printf("Start Time Range (day)   =   ignored      (default = ignored)\n");
  }
  if (end_t_range > 0.0) {
    printf("End Time Range (day)     = %12.8f   (default = ignored)\n",end_t_range);
  } else {
        printf("End Time Range (day)     =   ignored      (default = ignored)\n");
  }
  printf("Max Vel. (deg/day)       = %12.8f   (default = 1000.0)\n",maxv);
  printf("Min Vel. (deg/day)       = %12.8f   (default = 0.0)\n",minv);
  if((search_type == 0) || (search_type == 2)) {
    printf("Max Accel. (RA)          = %12.8f   (default = 0.02)\n",acc_r);
    printf("Max Accel. (DEC)         = %12.8f   (default = 0.02)\n",acc_d);
  }
  if(rem_overlap) {
    printf("Min Overlap              = %12.8f   (default = 0.5)\n",min_overlap);
  }
  if(eval) {
    printf("Evaluation mode:             ON\n");
    printf("ROC Thresh              %7.4f\n",roc_thresh);
  } else {
    printf("Evaluation mode:             OFF\n");
  }
  if(search_type == 1) {
    if(bwpass) {
      printf("Backward Tracking Pass:      ON\n");
    } else {
      printf("Backward Tracking Pass:      OFF\n");
    }
  }
  if(search_type == 0) {
    if(endpts) {
      printf("Endpoint Only Search:        ON\n");
    } else {
      printf("Endpoint Only Search:        OFF\n");
    }
  }
  if(removedups) {
    printf("Remove subsets/duplicates:   ON\n");
  } else {
    printf("Remove subsets/duplicates:   OFF\n");
  }
  if(rem_overlap) {
    printf("Remove Overlaps:             ON\n");
    if(allow_conflicts) {
      printf("Allow conflicts:             ON\n");
    } else {
      printf("Allow conflicts:             OFF\n");
    }
  } else {
    printf("Remove Overlaps:             OFF\n");
  }
  if(removedups) {
    printf("Produce Output Files         ON\n");
  } else {
    printf("Produce Output Files         OFF\n");
  }
  if(search_type == 1) {
    printf("Maximum Hypothesis   = %4i  (default 500)\n",max_hyp);
    printf("Maximum Matches      = %4i  (default 500)\n",max_match);
  }
  printf("Minimum Observations = %4i  (default   6)\n",min_obs);
  printf("Min Tracklets/Days   = %4i  (default   3)\n",min_sup);
  printf("\n\n");

  /* Convert things into useful units (radians). */
  vtree_thresh *= DEG_TO_RAD;
  acc_r        *= DEG_TO_RAD;
  acc_d        *= DEG_TO_RAD;

  if(fname == NULL && desfname == NULL) {
    printf("ERROR: No filename given.\n");
  } else {

    if (desfname) {
      printf("Loading detections in DES format from %s.\n", desfname);
      obs = mk_simple_obs_array_from_DES_file(desfname, 0.5, &true_groups,
                                              &true_pairs, NULL, NULL);
    } else {
      obs = mk_simple_obs_array_from_file(fname,0.5,&true_groups,&true_pairs);
    }

    if (obs != NULL && obs->size > 0) {

      /* Start by recentering the tracks. */
      printf(">> Shifting the observation bounds. ("); printf(curr_time()); printf(")\n");
      simple_obs_array_compute_bounds(obs,NULL,&r_lo,&r_hi,&d_lo,&d_hi,&t_lo,&t_hi);
      printf("   Bounds were R=[%12.8f,%12.8f], D=[%12.8f,%12.8f], T=[%12.8f,%12.8f]\n",
             r_lo, r_hi, d_lo, d_hi, t_lo, t_hi);
      recenter_simple_obs_array(obs,NULL,(r_hi+r_lo)/2.0,12.0,(d_lo+d_hi)/2.0,0.0,t_lo,0.0);
      simple_obs_array_compute_bounds(obs,NULL,&r_lo_n,&r_hi_n,&d_lo_n,&d_hi_n,&t_lo_n,&t_hi_n);
      printf("   Bounds are  R=[%12.8f,%12.8f], D=[%12.8f,%12.8f], T=[%12.8f,%12.8f]\n",
             r_lo_n, r_hi_n, d_lo_n, d_hi_n, t_lo_n, t_hi_n);

      /* Adjust the starting/ending bounds for the location of the first/last obs. */
      if (start_t_range > 0.0) {
        last_start_obs_time = t_lo_n + start_t_range;
      } else {
        last_start_obs_time = t_hi_n;
      }
      if (end_t_range > 0.0) {
        first_end_obs_time = t_hi_n - end_t_range;
      } else {
        first_end_obs_time = t_lo_n;
      }
      printf("   Setting upper bound of first obs = %12.8f\n", last_start_obs_time);
      printf("   Setting lower bound of last obs = %12.8f\n", first_end_obs_time);

      /* Provide the option to add gaussian noise. */
      if(sigma > 1e-10) {
        printf(">> Adding noise (sigma = %f degrees)...\n",sigma);
        simple_obs_array_add_gaussian_noise(obs, sigma/15.0, sigma, 0.0);
      }

      t1 = mk_true_tracks(obs,true_pairs);

      track_sizes = mk_count_tracks_iv_ind(true_groups);
      printf("Loaded %i observations from %i different tracklets\n",
             simple_obs_array_size(obs),track_array_size(t1));
      printf("Loaded %i observations from %i different tracks\n",
             simple_obs_array_size(obs),ivec_max(true_groups));
      fprintf_ivec(stdout,"Distribution of track sizes: ",track_sizes,"\n");
      free_ivec(track_sizes);

      /* Save the original times and flatten the obs. */
      printf(">> Flattening the Tracks to Plates "); printf(curr_time()); printf("\n");
      org_times = mk_simple_obs_array_times(obs);
      track_array_flatten_to_plates(t1, obs, plate_width);

      /* Count the number of unique time steps. */
      tcount = 1;
      times  = mk_sorted_simple_obs_array_times(obs);
      for(i=1;i<dyv_size(times);i++) {
        if(dyv_ref(times,i) > dyv_ref(times,i-1)+1e-10) {
          tcount++;
        }
      }
      free_dyv(times);
      printf("The flattened data contains %i unique times.\n",tcount);

      if((maxv < 999.0)||(minv > 1e-10)) {
        printf(">> Filtering Out 'Fast' and 'Slow' Tracklets...\n");
        t2 = mk_empty_track_array(track_array_size(t1));

        for(i=0;i<track_array_size(t1);i++) {
          A = track_first(track_array_ref(t1,i),obs);
          B = track_last(track_array_ref(t1,i),obs);

          dtime = (simple_obs_time(B)-simple_obs_time(A));
          if(dtime > 1e-10) {
            dist = angular_distance_RADEC(simple_obs_RA(A),simple_obs_RA(B),
                                          simple_obs_DEC(A),simple_obs_DEC(B));
            dist = dist * RAD_TO_DEG;
            if((dist/dtime <= maxv)&&(dist/dtime >= minv)) {
              track_array_add(t2,track_array_ref(t1,i));
            }
          }
        }

        free_track_array(t1);
        t1 = t2;
        printf("   %i Tracklets escaped the cut.\n",track_array_size(t1));
      }

      /* Do the actual searching... */
      printf(">> Doing the tracking "); printf(curr_time()); printf("\n");
      switch(search_type) {
      case 0:
        t2 = mk_vtrees_tracks(obs,t1,vtree_thresh,acc_r,acc_d,min_sup,2,
                              fit_thresh,pred_thresh,endpts,plate_width,
                              last_start_obs_time, first_end_obs_time);
        break;
      case 1:
        t2 = mk_MHT_matches(t1,obs,fit_thresh,lin_thresh,quad_thresh,
                            max_hyp,max_match,min_obs,bwpass);
        break;
      case 2:
        t2 = mk_sequential_accel_only_tracks(obs,t1,vtree_thresh,acc_r,acc_d,min_sup,
                                             fit_thresh,pred_thresh,plate_width,
                                             last_start_obs_time, first_end_obs_time);
        break;
      default:
        t2 = NULL;
      }

      printf("   Found %i potential tracks (",track_array_size(t2));
      printf(curr_time()); printf(").\n");

      printf(">> Removing 'short' tracks (< %i nights, < %i obs)...\n",min_sup,min_obs);
      t3 = mk_empty_track_array(track_array_size(t2));
      for(i=0;i<track_array_size(t2);i++) {
        if(track_num_nights_seen(track_array_ref(t2,i),obs) >= min_sup) {
          if(track_num_obs(track_array_ref(t2,i)) >= min_obs) {
            track_array_add(t3,track_array_ref(t2,i));
          }
        }
      }
      free_track_array(t2);
      t2 = t3;

      if(removedups) {
        printf(">> Removing subset/duplicate orbits "); printf(curr_time()); printf("\n");

        t3 = mk_MHT_remove_subsets(t2,obs);
        free_track_array(t2);
        t2 = t3;
        printf("   Found %i unique tracks.\n",track_array_size(t2));
      }

      if(rem_overlap) {
        printf(">> Removing overlapping orbits "); printf(curr_time()); printf("\n");
        t3 = mk_MHT_remove_overlaps(t2,obs,FALSE,0.5);
        free_track_array(t2);
        t2 = t3;
        printf("   After merging there are %i tracks.\n",track_array_size(t2));
      }

      printf(">> Putting Tracks in Trust Order "); printf(curr_time()); printf("\n");
      t3 = mk_order_tracks_by_trust(t2, obs);
      free_track_array(t2);
      t2 = t3;

      printf(">> Unflattening the Tracks to Plates "); printf(curr_time()); printf("\n");
      track_array_unflatten_to_plates(t1, obs, org_times);

      /* Put the bounds back they way they were. */
      if(search_type < 5) {
        printf(">> Shifting the observation bounds, back ("); printf(curr_time()); printf(")\n");
        recenter_simple_obs_array(obs,NULL,12.0,(r_hi+r_lo)/2.0,0.0,(d_lo+d_hi)/2.0,0.0,t_lo);
        simple_obs_array_compute_bounds(obs,NULL,&r_lo,&r_hi,&d_lo,&d_hi,&t_lo,&t_hi);
        printf("   Bounds are  R=[%12.8f,%12.8f], D=[%12.8f,%12.8f], T=[%12.8f,%12.8f]\n",
               r_lo, r_hi, d_lo, d_hi, t_lo, t_hi);
      }

      if(fileout) {
        printf(">> Dumping tracks to output files "); printf(curr_time()); printf("\n");
        f1 = fopen(fout1,"w");
        f3 = fopen(fout3,"w");
        dump_tracks_to_file(f1,f3,fout5,FALSE,obs,t2);
        fclose(f1);
        fclose(f3);
        f1 = fopen(fout4,"w");
        fprintf_track_array_ID_list(f1,t2,obs);
        fclose(f1);
      }

      if (trackids_filename) {
        printf(">> Dumping track IDs to %s.\n", trackids_filename);
        f1 = fopen(trackids_filename, "w");
        if (f1) {
            dump_trackids_to_file(f1,obs,t2);
            fclose(f1);
        }
      }

      if(eval == TRUE) {
        printf("\n\nScoring the tracks (");
        printf(curr_time()); 
        printf(").  This may take a little while.\n");

        printf("\n\nPairwise: Per. Corr. = %f  Per. Found = %f  Per. Found2 = %f\n",
               track_array_percent_correct(t2,true_groups,1),
               track_array_percent_found(t2,true_groups,1),
               track_array_percent_found(t2,true_groups,min_obs));

        filtered_true_groups = mk_tracks_of_length_from_list(obs, true_groups,
                                                             min_obs, min_sup);

        roc = mk_track_array_roc_vec(obs, t2, filtered_true_groups, min_obs,
                                     min_sup, roc_thresh);
        printf("Full Track: There are %i true tracks with %i or more obs on %i"
               " or more days.\n", ivec_max(filtered_true_groups)+1,
               min_obs, min_sup);
        printf("Full Track: We found %i tracks with %i or more obs on %i"
               " or more days.\n", track_array_size(t2), min_obs, min_sup);
        printf("Full Track: Per. Corr. = %f      Per. Found = %f\n",
               roc_percent_correct(roc),
               roc_percent_found(roc, filtered_true_groups));
        printf("Full Track: AAC (at 0.90) = %f\n",roc_aac(roc,
                  (int)((double)ivec_max(filtered_true_groups)*0.90)));
        printf("Full Track: AAC (at 0.75) = %f\n",roc_aac(roc,
                  (int)((double)ivec_max(filtered_true_groups)*0.75)));
        printf("Full Track Sum: %f %f %f %f\n",roc_percent_correct(roc),
               roc_percent_found(roc,filtered_true_groups),
               roc_aac(roc,(int)((double)ivec_max(filtered_true_groups)*0.90)),
               roc_aac(roc,(int)((double)ivec_max(filtered_true_groups)*0.75)));
        free_ivec(roc);

        /* Count the number of exact matches. */
        matches_found = compute_exact_matches(t2, filtered_true_groups,
                                              &percent_correct,
                                              &percent_found);
        printf("Exact Match: Number = %i,  Percent Found = %f, "
               "Percent Correct = %f\n", matches_found, percent_found,
               percent_correct);

        free_ivec(filtered_true_groups); 
      }

      free_track_array(t2);
      free_track_array(t1);
      free_ivec(true_groups);
      free_ivec(true_pairs);
      free_dyv(org_times);
    }

    if (NULL != obs) {
      free_simple_obs_array(obs);
    }
  }
}


int main(int argc,char *argv[]) {
  int seed = int_from_args("seed",argc,argv,0);

  if (seed) am_srand(seed);
  memory_leak_check_args(argc,argv);

  Verbosity = 0.0;

  tracker_main(argc,argv);

  am_malloc_report_polite();
  return 0; 
}
