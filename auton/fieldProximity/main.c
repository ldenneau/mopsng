/*
   File:        main.c
   Author:      J. Kubica
   Description: The main function for FieldProximity.

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

#include "occ_tree_funs.h"

#define OBSOCCUR_VERSION  1
#define OBSOCCUR_UPDATE   0
#define OBSOCCUR_RELEASE  3

extern int gen_count;

void dump_results(char* fout1, rd_plate_array* parr, namer* tnames,
                  ivec_array* res) {
  FILE* fp;
  ivec* inds;
  int i, j;

  fp = fopen(fout1,"w");
  
  if(fp) {
    for(i=0;i<ivec_array_size(res);i++) {
      inds = ivec_array_ref(res,i);

      for(j=0;j<ivec_size(inds);j++) {
        fprintf(fp,rd_plate_id(rd_plate_array_ref(parr,i)));
        fprintf(fp," ");
        fprintf(fp,"%s\n", namer_index_to_name(tnames,ivec_ref(inds,j)));
//        fprintf(fp," %8i %8i\n",i,ivec_ref(inds,j));      // these are line numbers in input files
      }
    }
    fclose(fp);
  } else {
    printf("ERROR: unable to open output file (");
    printf(fout1);
    printf(") for writing.\n");
  }
}


int fieldprox_count_chars(char* str, char c) {
  int L = strlen(str);
  int count = 0;
  int i;

  for(i=0;i<L;i++) {
    if(c == str[i]) { count++; }
  }

  return count;
}

pw_linear_array* mk_load_multiple_track_files(char* fname_orb, namer* id_to_ind, bool usedeg) {
  pw_linear_array* res = NULL;
  string_array*   fnames;
  int num_files = fieldprox_count_chars(fname_orb,',')+1;
  int i;

  if(num_files == 1) {
    res = mk_load_RA_DEC_pw_linear_array(fname_orb,id_to_ind,usedeg); 
  } else {
    fnames = mk_split_string(fname_orb,",");

    for(i=0;i<string_array_size(fnames);i++) {
      printf("Loading tracks from file %s...\n",string_array_ref(fnames,i));

      if(res == NULL) {
        res = mk_load_RA_DEC_pw_linear_array(string_array_ref(fnames,i),id_to_ind,usedeg);
      } else {
        add_to_RA_DEC_pw_linear_array(res,string_array_ref(fnames,i),id_to_ind,usedeg);
      }

    }

    free_string_array(fnames);
  }

  return res;
}


rd_plate_array* mk_load_multiple_plate_files(char* fname_obs, bool usedeg) {
  rd_plate_array* res    = NULL;
  rd_plate_array* sub;
  string_array*   fnames;
  int num_files = fieldprox_count_chars(fname_obs,',')+1;
  int i, j;

  if(num_files == 1) {
    res = mk_load_rd_plate_array(fname_obs,usedeg);
  } else {
    fnames = mk_split_string(fname_obs,",");

    for(i=0;i<string_array_size(fnames);i++) {
      printf("Loading fields from file %s...\n",string_array_ref(fnames,i));

      if(res == NULL) {
        res = mk_load_rd_plate_array(string_array_ref(fnames,i),usedeg);
      } else {
        sub = mk_load_rd_plate_array(string_array_ref(fnames,i),usedeg);
        if(sub != NULL) {
          for(j=0;j<rd_plate_array_size(sub);j++) {
            rd_plate_array_add(res,rd_plate_array_ref(sub,j));
          }
          free_rd_plate_array(sub);
        }
      }

    }

    free_string_array(fnames);
  }

  return res;
}


void orboccur_main(int argc,char *argv[]) {
  char* fname_obs = string_from_args("fieldsfile",argc,argv,NULL);
  char* fname_orb = string_from_args("tracksfile",argc,argv,NULL);
  char* fout1     = string_from_args("outfile",argc,argv,"result.txt");
  double thresh   = double_from_args("thresh",argc,argv,0.0001);
  int method      = int_from_args("method",argc,argv,0);
  int pleaf       = int_from_args("fleaf",argc,argv,10);
  int tleaf       = int_from_args("tleaf",argc,argv,10);
  bool  split_all = bool_from_args("split_all",argc,argv,FALSE); 
  ivec_array*      res  = NULL;
  pw_linear_array* tarr = NULL;
  rd_plate_array*  parr = NULL;
  plate_tree* ptr;
  pw_linear*  T;
  pw_linear*  A;
  pw_linear*  B;
  pw_tree*    ttr;
  namer* track_id_to_ind;
  double ts, te, t, pts, pte;
  bool failed = FALSE;
  int count = 0;
  int i, j;

  printf("Field Proximity VERSION: %i.%i.%i\n",OBSOCCUR_VERSION,
         OBSOCCUR_UPDATE,OBSOCCUR_RELEASE);
  printf("This program comes with ABSOLUTELY NO WARRANTY. This is free "
         "software, and you are welcome to redistribute it under certain "
         " conditions.  See included license for details.\n\n");

  printf("-------- PARAMETERS ---------------------------------- \n");
  if(fname_orb) {
    printf("Track/Position file:  "); printf(fname_orb); printf("\n");
  } else {
    printf("Track/Position file:  <NOT GIVEN!>\n");
  }
  if(fname_obs) {
    printf("Field file:           "); printf(fname_obs); printf("\n");
  } else {
    printf("Field file:           <NOT GIVEN!>\n");
  }
  if(fout1) {
    printf("Result file:          "); printf(fout1); printf("\n");
  } else {
    printf("Result file:          <NOT GIVEN!>\n");
  }
  printf("Matching Method = %2i   (0 = Exhaustive)\n",method);  
  printf("                       (1 = Plate Tree)\n");  
  printf("                       (2 = Track Tree)\n");  
  printf("                       (3 = Dual Plate/Track Tree)\n");  
  printf("Threshold (RD)   = %12.8f   (default = 0.0001)\n",thresh);
  if((method == 1)||(method == 3)) {
    printf("fleaf            = %i   (default 10)\n",pleaf);
  }
  if((method == 2)||(method == 3)) {
    printf("tleaf            = %i   (default 10)\n",tleaf);
    if(split_all) {
      printf("ttree split_all  = TRUE    (default FALSE)\n");
    } else {
      printf("ttree split_all  = FALSE   (default FALSE)\n");
    }
  }

  thresh *= DEG_TO_RAD;

  /* Load the data sets. */
  track_id_to_ind = mk_empty_namer(TRUE);
  if(fname_obs) { 
    parr = mk_load_multiple_plate_files(fname_obs,TRUE);
    /*parr = mk_load_rd_plate_array(fname_obs,TRUE); */
  }
  if(fname_orb) { 
    tarr = mk_load_multiple_track_files(fname_orb,track_id_to_ind,TRUE);
    /*tarr = mk_load_RA_DEC_pw_linear_array(fname_orb,track_id_to_ind,TRUE); */
  }  

  if((parr != NULL)&&(tarr != NULL)) {
   
    /* Find the start and end time of the plates. */
    if(rd_plate_array_size(parr) > 0) {
      pts = rd_plate_time(rd_plate_array_ref(parr,0));
      pte = rd_plate_time(rd_plate_array_ref(parr,0));
      for(i=1;i<rd_plate_array_size(parr);i++) {
        t = rd_plate_time(rd_plate_array_ref(parr,i));
        if(t < pts) { pts = t; }
        if(t > pte) { pte = t; }
      }
      printf("%i fields loaded with t=[%f,%f]\n",rd_plate_array_size(parr),pts,pte);
    } else {
      printf("You loaded an empty plate file.\n");
      failed = TRUE;
      pts = 0; pte = 0;
    }

    /* Find the start and end time of the segments. */
    if(failed == FALSE) {
      if(pw_linear_array_size(tarr) > 0) {
        T  = pw_linear_array_ref(tarr,0);
        ts = pw_linear_x(T,0);
        te = pw_linear_x(T,pw_linear_size(T)-1);
        printf("%i tracks loaded with t=[%f,%f]\n",pw_linear_array_size(tarr),ts,te);

        if((te < pte)||(ts > pts)) {
          printf("ERROR: track estimate time range does not cover ALL fields.\n");
          failed = TRUE;
        }
      } else {
        printf("You loaded an empty field file.\n");
        failed = TRUE;
        ts = 0; te = 0;
      }
    }

    /* Check that the tracks all line up in time... */
    if(failed == FALSE) {
      A = pw_linear_array_ref(tarr,0);
      for(i=1;(i<pw_linear_array_size(tarr))&&(failed==FALSE);i++) {
        B = pw_linear_array_ref(tarr,i);
        if(pw_linear_size(A) != pw_linear_size(B)) {
          printf("ERROR: Two orbits/tracks (");
          printf(namer_index_to_name(track_id_to_ind,0));
          printf(" and ");
          printf(namer_index_to_name(track_id_to_ind,i));
          printf(") are different sizes (%i vs %i)\n",pw_linear_size(A),
                 pw_linear_size(B));
          failed = TRUE;
        }

        for(j=0;(j<pw_linear_size(A))&&(failed==FALSE);j++) {
          if(fabs(pw_linear_x(A,j)-pw_linear_x(B,j)) > 1e-10) {
            printf("ERROR: Two orbits/tracks (");
            printf(namer_index_to_name(track_id_to_ind,0));
            printf(" and ");
            printf(namer_index_to_name(track_id_to_ind,i));
            printf(") are not aligned!\n");
            failed = TRUE;
          }
        }
      }
    }

    if(failed==FALSE) {
      printf("\nDoing the Matching...\n");

      /* Do the actual search */
      switch(method) {
      case 0: 
        res = mk_exhaustive(tarr,parr,thresh);
        break;
      case 1:
        printf("Building field tree "); printf(curr_time()); fflush(stdout);
        ptr = mk_plate_tree(parr,1.0,1.0,1.0,pleaf);
        printf("->"); printf(curr_time()); printf("\n");

        res = mk_plate_tree_int_search(ptr,parr,tarr,thresh);

        free_plate_tree(ptr);
        break;
      case 2:
        printf("Building track tree "); printf(curr_time()); fflush(stdout);
        ttr = mk_pw_tree(tarr,0.0,10.0,tleaf,split_all);
        printf("->"); printf(curr_time()); printf("\n");

        res = mk_pw_tree_search(ttr, tarr, parr, thresh);

        free_pw_tree(ttr);
        break;
      case 3:
        printf("Building field tree "); printf(curr_time()); fflush(stdout);
        ptr = mk_plate_tree(parr,1.0,1.0,1.0,pleaf);
        printf("->"); printf(curr_time()); printf("\n");

        printf("Building track tree "); printf(curr_time()); fflush(stdout);
        ttr = mk_pw_tree(tarr,0.0,10.0,tleaf,split_all);
        printf("->"); printf(curr_time()); printf("\n");

        res = mk_dual_tree_search(ttr,tarr,ptr,parr,thresh);

        free_pw_tree(ttr);
        free_plate_tree(ptr);
        break;
      default:
        printf("%i is not a valid matching option\n",method);
      }
    }

    /* Dump everything to output files (even if */
    /* the input files were bad).               */
    if(failed == TRUE) {
      res = mk_zero_ivec_array(0);
    }
    for(i=0;i<ivec_array_size(res);i++) {
      count += ivec_size(ivec_array_ref(res,i));
    } 
    printf("Done.  Found %i matches.\n",count);
    printf("Dumping to file.\n");
 
    dump_results(fout1,parr,track_id_to_ind,res);
    free_ivec_array(res);
  } else {
    printf("ERROR: Unable to load one or more data sets.\n");
    printf("       Please check file names.             \n");
  }

  /* Free the data sets. */
  free_namer(track_id_to_ind);
  if(parr != NULL) { free_rd_plate_array(parr);  }
  if(tarr != NULL) { free_pw_linear_array(tarr); }
}


void orboccur_test(int argc,char *argv[]) {
  double thresh    = double_from_args("thresh",argc,argv,0.0001);
  bool   split_all = bool_from_args("split_all",argc,argv,FALSE); 
  int NP          = int_from_args("NP",argc,argv,100);
  int NT          = int_from_args("NT",argc,argv,100);
  int knots       = int_from_args("knots",argc,argv,100);
  int pleaf       = int_from_args("pleaf",argc,argv,10);
  int tleaf       = int_from_args("tleaf",argc,argv,10);
  ivec_array*       res1 = NULL;
  ivec_array*       res2 = NULL;
  ivec_array*       res3 = NULL;
  ivec_array*       res4 = NULL;
  pw_linear_array* tarr = NULL;
  rd_plate_array*  parr = NULL;
  plate_tree* ptr;
  pw_tree*    ttr;
  dyv *times, *Lbnd, *Ubnd, *sigv, *sigiv;
  ivec *keepbnds;
  int t;

  printf("OBSOCCUR VERSION: %i.%i.%i\n\n",OBSOCCUR_VERSION,
         OBSOCCUR_UPDATE,OBSOCCUR_RELEASE);

  printf("-------- PARAMETERS ---------------------------------- \n");
  printf("Threshold (RD)   = %12.8f   (default = 0.0001)\n",thresh);
  printf("pleaf            = %i   (default 10)\n",pleaf);
  printf("tleaf            = %i   (default 10)\n",tleaf);

  /* Load the data sets. */
  parr = mk_random_rd_plate_array(NP,0.1,9.9,0.0,24.0,-60.0,60.0,0.175);
  
  times = mk_zero_dyv(knots);
  for(t=0;t<knots;t++) { dyv_set(times,t,10.0*((double)(t))/((double)(knots-1))); }
  Lbnd = mk_dyv_2(0.0,-60.0);
  Ubnd = mk_dyv_2(24.0,+60.0);
  sigiv = mk_dyv_2(0.05,0.5);
  sigv = mk_dyv_2(0.01,0.1);
  keepbnds = mk_ivec_2(0,1);
  tarr = mk_random_pw_linear_array(NT,times,Lbnd,Ubnd,sigiv,sigv,keepbnds);
  free_ivec(keepbnds);
  free_dyv(times);
  free_dyv(sigiv);
  free_dyv(sigv);
  free_dyv(Lbnd);
  free_dyv(Ubnd);

  printf("%i plates loaded\n",rd_plate_array_size(parr));
  printf("%i tracks loaded\n",pw_linear_array_size(tarr));

  printf("Building ptree "); printf(curr_time()); fflush(stdout);
  ptr = mk_plate_tree(parr,1.0,1.0,1.0,pleaf);
  printf("->"); printf(curr_time()); printf("\n");

  printf("Building ttree "); printf(curr_time()); fflush(stdout);
  ttr = mk_pw_tree(tarr,0.0,10.0,tleaf,split_all);
  printf("->"); printf(curr_time()); printf("\n");

  printf("Matching EXH : "); printf(curr_time()); fflush(stdout);
  res1 = mk_exhaustive(tarr,parr,thresh);
  printf("->"); printf(curr_time()); printf(" (%i)\n",gen_count); 

  printf("Matching PTR : "); printf(curr_time()); fflush(stdout);
  res2 = mk_plate_tree_int_search(ptr,parr,tarr,thresh);
  printf("->"); printf(curr_time()); printf(" (%i)\n",gen_count);

  printf("Matching TTR : "); printf(curr_time()); fflush(stdout);
  res3 = mk_pw_tree_search(ttr, tarr, parr, thresh);
  printf("->"); printf(curr_time()); printf(" (%i)\n",gen_count);

  printf("Matching DUAL: "); printf(curr_time()); fflush(stdout);
  res4 = mk_dual_tree_search(ttr,tarr,ptr,parr,thresh);
  printf("->"); printf(curr_time()); printf(" (%i)\n",gen_count);

  printf("E vs P = %f\n",calculate_errors(res1,res2));
  printf("E vs T = %f\n",calculate_errors(res1,res3));
  printf("E vs D = %f\n",calculate_errors(res1,res4));
  printf("P vs T = %f\n",calculate_errors(res2,res3));
  printf("P vs D = %f\n",calculate_errors(res2,res4));
  printf("T vs D = %f\n",calculate_errors(res3,res4));

  free_plate_tree(ptr);
  free_pw_tree(ttr);
  free_ivec_array(res1);
  free_ivec_array(res2);
  free_ivec_array(res3);
  free_ivec_array(res4);

  /* Free the data sets. */
  if(parr != NULL) { free_rd_plate_array(parr);  }
  if(tarr != NULL) { free_pw_linear_array(tarr); }
}


int main(int argc,char *argv[]) {
  memory_leak_check_args(argc,argv);

  orboccur_main(argc,argv);

  am_malloc_report_polite();
  return 0;
}
