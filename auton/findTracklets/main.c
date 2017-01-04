/*
   File:        main.c
   Author:      J. Kubica
   Description: The main function for FindTracklets.

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
#include "rdt_tree.h"
#include "tracklet_mht.h"
#include "findtrackletsapi.h"
#include "gcf.h"

void output_tracklet_results(simple_obs_array* parr, track_array* tarr,
                             char* pairfile, char* sumfile) {
  FILE* fp;
  ivec* inds;
  int i, j;

  /* Create the summary file first. */
  fp = fopen(sumfile,"w");
  if(fp) {
    fprintf(fp,"TRACKNUM OBSNUM OBSID\n");    

    for(i=0;i<track_array_size(tarr);i++) {
      inds = track_individs(track_array_ref(tarr,i));
      for(j=0;j<ivec_size(inds);j++) {
        fprintf(fp,"%i %i ",i,ivec_ref(inds,j));
        fputs(simple_obs_id_str(simple_obs_array_ref(parr,ivec_ref(inds,j))), fp);
        fprintf(fp,"\n");
      }
    }

    fclose(fp);
  }

  /* Now create the match file... */
  fp = fopen(pairfile,"w");
  if(fp) {
    for(i=0;i<track_array_size(tarr);i++) {
      inds = track_individs(track_array_ref(tarr,i));
      for(j=0;j<ivec_size(inds)-1;j++) {
        fprintf(fp,"%i ",ivec_ref(inds,j));
      }
      fprintf(fp,"%i\n",ivec_ref(inds,ivec_size(inds)-1));
    }
    fclose(fp);
  }
}


double compute_gcr(simple_obs_array* parr, ivec* inds) {
    /* Compute the great circle residual to the detections in the tracklet.  */
    int nobs = ivec_size(inds);
    double t[nobs];
    double c[nobs][2];
    int i;
    for (i = 0; i < nobs; i++) {
        simple_obs* obs = simple_obs_array_ref(parr, ivec_ref(inds, i));
        t[i] = simple_obs_time(obs);
        c[i][0] = M_PI / 180 * simple_obs_RA(obs) * 15;      // RA stored in hours
        c[i][1] = M_PI / 180 * simple_obs_DEC(obs);

//        printf("%s %f %f %f\n", simple_obs_id_str(obs), simple_obs_time(obs), simple_obs_RA(obs) * 15, simple_obs_DEC(obs));
    }
    gcfparam gcf;
    gcf.nObs = nobs;
    double nt[nobs];
    double rs[nobs][2];
    gcf.ntime = nt;
    gcf.rs = rs;
    gcFit(&gcf, t, c);
    
    double res[nobs][2];
    double rms = gcRmsRes(&gcf, res);

//    printf("RMS=%f\n\n", rms);
    return rms;
}


void output_tracklet_ids(simple_obs_array* parr, track_array* tarr, char* idsfile) {
  FILE* fp;
  ivec* inds;
  int i, j;

  /* Just write out lines containing detection IDs.  Each line is a single tracklet: GC_resid_deg detID1 detID2 .... */
  fp = fopen(idsfile,"w");
  if(fp) {
    for(i=0;i<track_array_size(tarr);i++) {
      inds = track_individs(track_array_ref(tarr,i));

      // Compute and emit great circle residual in deg.
//      float gcr_deg = compute_gcr(parr, inds);
//      fprintf(fp,"%6.3f ", gcr_deg);

      for(j=0;j<ivec_size(inds) - 1;j++) {
        fprintf(fp,"%s ", simple_obs_id_str(simple_obs_array_ref(parr,ivec_ref(inds,j))));
      }
      j = ivec_size(inds) - 1;  /* last item */
      fprintf(fp,"%s\n", simple_obs_id_str(simple_obs_array_ref(parr,ivec_ref(inds,j))));
    }

    fclose(fp);
  }
}

void tracklet_main(int argc,char *argv[]) {
  char* fname = string_from_args("file",argc,argv,NULL);
  char* fout1 = string_from_args("pairfile",argc,argv,"pairs.obs");
  char* fout2 = string_from_args("summaryfile",argc,argv,"pairs.sum");
  char* fout3 =  string_from_args("idsfile",argc,argv,NULL);
  char* fout_mpc = string_from_args("mpc_file",argc,argv,NULL);
  double athresh = double_from_args("athresh",argc,argv,FT_DEF_ATHRESH);
  double thresh  = double_from_args("thresh",argc,argv,FT_DEF_THRESH);
  double maxLerr = double_from_args("maxLerr",argc,argv,FT_DEF_MAXLERR);
  double etime   = double_from_args("etime",argc,argv,FT_DEF_ETIME);
  double minv    = double_from_args("minv",argc,argv,FT_DEF_MINV);
  double maxv    = double_from_args("maxv",argc,argv,FT_DEF_MAXV);
  double maxt    = double_from_args("maxt",argc,argv,FT_DEF_MAXT);
  int    minobs  = double_from_args("minobs",argc,argv,FT_DEF_MINOBS);  
  int    maxobs  = double_from_args("maxobs",argc,argv,FT_DEF_MAXOBS);  
  bool   eval          = bool_from_args("eval",argc,argv,FALSE);
  bool   greedy        = bool_from_args("greedy",argc,argv,FALSE);
  bool   removedups    = bool_from_args("remove_subsets",argc,argv,TRUE);
  bool   use_pht       = bool_from_args("use_pht", argc, argv, FALSE);
  simple_obs_array* obs;
  track_array* trcks;
  track_array* cheat;
  ivec_array* obs_to_track;
  dyv* length;
  dyv* angle;
  dyv* exp_time;
  ivec* true_groups;
  ivec* cheat_pairs;
  ivec* roc;
  ivec* inds;
  int i,j;
  double percent_correct = 0.0;
  double percent_found = 0.0;
  int matches_found = 0;

  printf("FIND_TRACKLETS VERSION: %i.%i.%i\n",TRACKLET_VERSION,
         TRACKLET_RELEASE,TRACKLET_UPDATE);
  printf("This program comes with ABSOLUTELY NO WARRANTY. This is free "
         "software, and you are welcome to redistribute it under certain "
         " conditions.  See included license for details.\n\n");

  if(fname) {
    printf("Input file:           "); puts(fname); printf("\n");
  } else {
    printf("Input file:           <NOT GIVEN!>\n");
  }
  printf("Output file (pairs):   "); puts(fout1); printf("\n");
  printf("Output file (summary): "); puts(fout2); printf("\n");

  if(fout3 != NULL) {
    printf("Output file (ids):     "); puts(fout3); printf("\n");
  } else {
    printf("Output file (ids):     [N/A]\n");
  }

  if(fout_mpc != NULL) {
    printf("Output file (MPC):     "); puts(fout_mpc); printf("\n");
  } else {
    printf("Output file (MPC):     [N/A]\n");
  }

  printf("Fit threshold (degrees)  = %12.8f   (default = %f)\n",thresh,FT_DEF_THRESH);
  printf("Angle Thresh (degrees)   = %12.8f   (default = %f)\n",athresh,FT_DEF_ATHRESH);
  printf("maxLerr (degrees)        = %12.8f   (default = %f)\n",maxLerr,FT_DEF_MAXLERR);
  printf("Min. Velocity (deg/day)  = %12.8f   (default = %f)\n",minv,FT_DEF_MINV);
  printf("Max. Velocity (deg/day)  = %12.8f   (default = %f)\n",maxv,FT_DEF_MAXV);
  printf("Max. Spread   (days)     = %12.8f   (default = %f)\n",maxt,FT_DEF_MAXT);
  printf("Exposure Time (sec)      = %12.8f   (default = %f)\n",etime,FT_DEF_ETIME);
  printf("Min. Number of Obs.      = %12i   (default = %i)\n",minobs,FT_DEF_MINOBS);
  printf("Max. Number of Obs.      = %12i   (default = %i)\n",maxobs,FT_DEF_MAXOBS);
  if(greedy) {
    printf("Greedy mode:                 ON\n");
  } else {
    printf("Greedy mode:                 OFF\n");
  }
  if(use_pht) {
    printf("PHT mode:                    ON\n");
  } else {
    printf("PHT mode:                    OFF\n");
  }
  if(eval) {
    printf("Evaluation mode:             ON\n");
  } else {
    printf("Evaluation mode:             OFF\n");
  }
  if(removedups) {
    printf("Remove subsets/duplicates:   ON\n");
  } else {
    printf("Remove subsets/duplicates:   OFF\n");
  }

  minv    = minv * DEG_TO_RAD;
  maxv    = maxv * DEG_TO_RAD;
  thresh  = thresh * DEG_TO_RAD;
  athresh = athresh * DEG_TO_RAD;
  maxLerr = maxLerr * DEG_TO_RAD;

  /* Make sure the exposure time is realistic and in days. */
  if(etime < 0.01) { etime = 0.01; }
  etime = etime / (24.0 * 60.0 * 60.0); 

  printf("\n\n");

  if(fname == NULL) {
    printf("ERROR: No filename given.\n");
  } else {
    obs = mk_simple_obs_array_from_file_elong(fname, maxt, &true_groups, NULL,
                                              &length, &angle, &exp_time);

    if((obs != NULL)&&(simple_obs_array_size(obs) > 0)) {
      trcks = mk_tracklets_MHT(obs, minv, maxv, thresh, maxt, minobs,
                               removedups, angle, length, exp_time,
                               athresh, maxLerr, etime,
                               maxobs, greedy, use_pht);

      printf(">> Dumping tracks to output files.\n");

      if (fout3) {
        output_tracklet_ids(obs,trcks,fout3);
      }
      else {
        /* idsfile not specified, so output usual stuff (sum and pairs) */
        output_tracklet_results(obs,trcks,fout1,fout2);
      }

      if(fout_mpc != NULL) {
        dump_tracks_to_MPC_file(fout_mpc,obs,trcks);
      }

      /* Do the scoring. */
      if(eval == TRUE) {
        printf("\n\nScoring the tracks (");
        puts(curr_time()); 
        printf(").  This may take a little while.\n");

        obs_to_track = mk_simple_obs_pairing_from_true_groups(obs,
                                                              true_groups,
                                                              maxt);
        cheat = mk_track_array_from_matched_simple_obs(obs, obs_to_track,
                                                       minobs);
        free_ivec_array(obs_to_track);
     
        /* Fill cheat_pairs with "observation -> true group number" mapping. */
        cheat_pairs = mk_constant_ivec(simple_obs_array_size(obs), -1);
        for(i = 0; i < track_array_size(cheat); i++) {
          inds = track_individs(track_array_ref(cheat, i));
          for(j = 0; j < ivec_size(inds); j++) {
            ivec_set(cheat_pairs, ivec_ref(inds, j), i);
          }
        }

        roc = mk_track_array_roc_vec(obs, trcks, cheat_pairs, minobs, 1, 0.95);
        printf("There are %i true tracklets in the code.\n",
               ivec_max(cheat_pairs)+1);
        printf("The code returned %i suggested tracklets.\n",
               track_array_size(trcks));
        printf("Full Track: Per. Corr. = %f      Per. Found = %f\n",
               roc_percent_correct(roc), roc_percent_found(roc, cheat_pairs));

        /* Count the number of exact matches. */
        matches_found = compute_exact_matches(trcks, cheat_pairs,
                                              &percent_correct,
                                              &percent_found);
        printf("Exact Match: Number = %i,  Percent Found = %f, "
               "Percent Correct = %f\n", matches_found, percent_found,
               percent_correct);

        free_ivec(roc);
        free_ivec(cheat_pairs);
        free_track_array(cheat);
      }
 
      free_simple_obs_array(obs);
      free_ivec(true_groups);
      free_track_array(trcks);
      if(length != NULL) { free_dyv(length); }
      if(angle != NULL) { free_dyv(angle); }
      if(exp_time != NULL) { free_dyv(exp_time); }
    } else {
      /* Write empty output files. */
      FILE * fp;
      if (fout3) {
        fp = fopen(fout3, "w");
        fclose(fp);
      }
      else {
        /* idsfile not specified, so output usual stuff (sum and pairs) */
        fp = fopen(fout1, "w");
        fclose(fp);
        fp = fopen(fout2, "w");
        fclose(fp);
      }
      if(obs != NULL) { free_simple_obs_array(obs); }
    }
  }
}


int main(int argc,char *argv[]) {

  memory_leak_check_args(argc,argv);
  Verbosity = 0.0;

  tracklet_main(argc,argv);
 
  am_malloc_report_polite();
  return 0;
}
