/*
File:        main.c
Author:      J. Kubica
Description: The main function for AstroClean.

Copyright 2006, The Auton Lab, CMU
                                                                  
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
#include "astroclean_api.h"
#include "astroclean.h"
#include "obs.h"

#define AC_TRACKLET_VERSION 1
#define AC_TRACKLET_RELEASE 0
#define AC_TRACKLET_UPDATE  0

#define AC_OUTPUT_MPC  0
#define AC_OUTPUT_MITI 1

/* DETECTION_ID EPOCH_MJD RA_DEG DEC_DEG MAG OBSCODE [OBJECT_NAME] */
void fprintf_MITI_simple_obs(FILE* f,simple_obs* S, char* oname) {
  fprintf(f,simple_obs_id_str(S));
  fprintf(f," %18.10f",simple_obs_time(S));
  fprintf(f," %18.12f",simple_obs_RA(S)*15.0);
  fprintf(f," %18.12f",simple_obs_DEC(S));
  fprintf(f," %15.10f",simple_obs_brightness(S));
  fprintf(f," %4i",simple_obs_obs_code(S));
  if(oname != NULL) { fprintf(f," %s",oname); }
  fprintf(f,"\n");
}

/* Compute the bounds of a set of observations:             */
/*  simple_obs_array[inds[i]] for all i in 1 to size(inds). */
/* Used to scale the drawing window.                        */
/* Returns a dyv with [ra_min ra_max dec_min dec_max]       */
dyv* ac_compute_obs_bounds(simple_obs_array* obs, ivec* inds) {
  simple_obs* X;
  dyv* res = mk_zero_dyv(4);
  double rMin = 0.0;
  double rMax = 0.0;
  double dMin = 0.0;
  double dMax = 0.0;
  int i, N;

  /* Go through each point in "inds" */
  N = ivec_size(inds);
  for(i=0;i<N;i++) {
    X = simple_obs_array_ref(obs,ivec_ref(inds,i));
    if((i==0)||(rMin > simple_obs_RA(X)))  { rMin = simple_obs_RA(X);  }
    if((i==0)||(rMax < simple_obs_RA(X)))  { rMax = simple_obs_RA(X);  }
    if((i==0)||(dMin > simple_obs_DEC(X))) { dMin = simple_obs_DEC(X); }
    if((i==0)||(dMax < simple_obs_DEC(X))) { dMax = simple_obs_DEC(X); }
  }

  /* Set the results. */
  dyv_set(res,0,rMin);
  dyv_set(res,1,rMax);
  dyv_set(res,2,dMin);
  dyv_set(res,3,dMax);

  return res;
}


void clean_main(int argc,char *argv[]) {
  AstroCleanStateHandle acsh;
  char* fname = string_from_args("file",argc,argv,NULL);
  char* fout  = string_from_args("clean_file",argc,argv,"output.txt");
  char* fout2 = string_from_args("noise_file",argc,argv,NULL);

  /* Parameters for brightness thresholding. */
  double bLO  = double_from_args("bLO",argc,argv,-1.0);
  double bHI  = double_from_args("bHI",argc,argv,-1.0);

  /* Parameters for time thresholding. */
  double tLO  = double_from_args("tLO",argc,argv,-1.0);
  double tHI  = double_from_args("tHI",argc,argv,-1.0);
   
  /* Parameters for density thresholding. */
  double density= double_from_args("density",argc,argv,-1.0);
  double Dradius= double_from_args("Dradius",argc,argv,-1.0);
  bool   adaptD = bool_from_args("Drelative",argc,argv,TRUE);

  /* Parameters for line extraction. */
  int linesupport = int_from_args("linesupport",argc,argv,0);
  double linerad = double_from_args("linerad",argc,argv,0.1);
  double lineang = double_from_args("lineang",argc,argv,2.0);

  /* Parameters for duplicate removal. */
  double proxrad  = double_from_args("proxrad",argc,argv,-1.0);
  double stationrad  = double_from_args("stationrad",argc,argv,-1.0);

  /* Parameters for region extraction. */
  double ra_min  = double_from_args("ra_min",argc,argv,-1.0);
  double ra_max  = double_from_args("ra_max",argc,argv,-1.0);
  double dec_min = double_from_args("dec_min",argc,argv,-200.0);
  double dec_max = double_from_args("dec_max",argc,argv,-200.0);

  /* Parameters for region extraction. */
  double regionrad = double_from_args("regionrad",argc,argv,-1.0);
  double regionra  = double_from_args("regionra",argc,argv,0.0);
  double regiondec = double_from_args("regiondec",argc,argv,0.0);

  int outtype     = int_from_args("outtype",argc,argv,0);
  int verbosity   = int_from_args("verbosity",argc,argv,1);
  simple_obs_array* obs;
  simple_obs*       X;
  dyv*              bnds;
  ivec*             inds;
  int i, N;
  FILE* fp;

  printf("ASTROCLEAN VERSION: %i.%i.%i\n", AC_TRACKLET_VERSION,
	       AC_TRACKLET_RELEASE, AC_TRACKLET_UPDATE);
  printf("Copyright 2006, The Auton Lab, CMU\n");
  printf("This program comes with ABSOLUTELY NO WARRANTY. This is free "
         "software, and you are welcome to redistribute it under certain "
         " conditions.  See included license for details.\n\n");

  if(fname) {
    printf("Input file:           "); printf(fname); printf("\n");
  } else {
    printf("Input file:           <NOT GIVEN!>\n");
  }
  printf("Output file (clean):  "); printf(fout); printf("\n");
  if(fout2) {
    printf("Output file (noise):  "); printf(fout2); printf("\n");
  } else {
    printf("Output file (noise):  <NONE GIVEN> - NO NOISE OUTPUT USED\n");
  }

  printf("Output format:        ");
  switch(outtype) {
  case AC_OUTPUT_MPC:  printf("MPC\n");  break;
  case AC_OUTPUT_MITI: printf("MITI\n"); break;
  default:
    outtype = AC_OUTPUT_MPC;
    printf("Unknown (using MPC)\n");
  }

  printf("\n\n--- FILTERS -----------------------------------\n");

  printf("\nTIME [tLO,tHI]: ");
  if((tLO >= 0.0)||(tHI >= 0.0)) {
    if(tLO >= 0.0) { printf("%f <= ",tLO); }
    printf("time");
    if(tHI >= 0.0) { printf(" <= %f",tHI); }
  } else {
    printf("<NO FILTERS>");
  }
  printf("\n");

  printf("\nBRIGHTNESS [bLO,bHI]: ");
  if((bLO >= 0.0)||(bHI >= 0.0)) {
    if(bLO >= 0.0) { printf("%f <= ",bLO); }
    printf("brght");
    if(bHI >= 0.0) { printf(" <= %f",bHI); }
  } else {
    printf("<NO FILTERS>");
  }
  printf("\n");

  printf("\nABS REGION EXTRACTION [ra_min, ra_max, dec_min, dec_max]: ");
  if((ra_min >= 0.0)||(ra_max >= 0.0)||(dec_min >= -100.0)||(dec_max >= -100.0)) {
    printf("\n");
    if(ra_min >= 0.0) { printf("  RA  >= %f\n",ra_min); }
    if(ra_max >= 0.0) { printf("  RA  <= %f\n",ra_max); }
    if(dec_min >= -100.0) { printf("  DEC >= %f\n",dec_min); }
    if(dec_max >= -100.0) { printf("  DEC <= %f\n",dec_max); }
  } else {
    printf("<NO FILTERS>\n");
  }

  printf("\nREGION EXTRACTION [regionrad]: ");
  if(regionrad > 0.0) {
    printf("Points within %f degrees of (%f,%f)",regionrad,regionra,regiondec);
  } else {
    printf("<NO FILTERS>");
  }
  printf("\n");

  printf("\nPROXIMITY [proxrad]: ");
  if(proxrad > 0.0) {
    printf("proximity <= %f (degrees)\n",proxrad);
  } else {
    printf("<NO FILTERS>");
  }
  printf("\n");

  printf("\nSTATIONARY [stationrad]: ");
  if(stationrad > 0.0) {
    printf("proximity <= %f (degrees)\n",stationrad);
  } else {
    printf("<NO FILTERS>");
  }
  printf("\n");

  printf("\nDENSITY [dense]: ");
  if(density >= 0.0) { 
    printf("Density <= %f (detections/sq degree) ",density);
    
    if(Dradius > 0.0) {
      printf(" (pointwise, radius=%f)  ",Dradius);
    } else {
      printf(" (fieldwise)  ");
    }
    if(adaptD) {
      printf(" [relative]");
    } else {
      printf(" [Boolean]");
    }
  } else {
    printf("<NO FILTERS>");
  }
  printf("\n");
  
  printf("\nLINES [linesupport]: ");
  if(linesupport > 0) {
    printf("\n  Min. Support [linesupport] = %i",linesupport);
    printf("\n  Angular Thresh [lineang]   = %f (degrees)",lineang);
    printf("\n  Neighbor radius [linerad]  = %f (degrees)",linerad);
  } else {
    printf("<NO FILTERS>");
  }
  printf("\n");

  printf("\n-----------------------------------------------\n\n");

  if(fname == NULL) {
    printf("ERROR: No filename given.\n");
  } else {
    if(verbosity > 0) {
      printf(">> Loading the input data.\n");
    }
    obs = mk_simple_obs_array_from_file(fname,0.0,NULL,NULL);
    if(verbosity > 0) {
      printf("   Loaded %i detections.\n",simple_obs_array_size(obs));
    }

    if(verbosity > 0) {
      printf("\n--------- Data Stats -----------------------------\n");
      printf("  Number of detections: %i\n",simple_obs_array_size(obs));

      inds = mk_sequence_ivec(0,simple_obs_array_size(obs));
      bnds = ac_compute_obs_bounds(obs,inds);
      printf("  RA bounds:  [%f,%f]\n",dyv_ref(bnds,0),dyv_ref(bnds,1));
      printf("  DEC bounds: [%f,%f]\n",dyv_ref(bnds,2),dyv_ref(bnds,3));
      free_ivec(inds);
      free_dyv(bnds);

      bnds = mk_simple_obs_plate_times(obs,1e-10);
      printf("  Time Steps: %i\n",dyv_size(bnds));
      free_dyv(bnds);
    }

    printf("--------------------------------------------------\n\n");

    /* Create the astroclean data structure. */
    AstroClean_Init(&acsh,verbosity,stdout);
    for(i=0; i<simple_obs_array_size(obs); i++) {
      X = simple_obs_array_ref(obs,i);
      AstroClean_AddDetection(acsh,simple_obs_RA(X),simple_obs_DEC(X),
                              simple_obs_time(X),simple_obs_brightness(X));
    }

    /* Filter on time */
    if((tLO >= 0.0)||(tHI >= 0.0)) {
      AstroClean_Time_Filter(acsh,tLO,tHI);
    }

    /* Filter on brightness */
    if((bLO >= 0.0)||(bHI >= 0.0)) {
      AstroClean_Brightness_Filter(acsh,bLO,bHI);
    }

    /* Filter on region 1 */
    if((ra_min >= 0.0)||(ra_max >= 0.0)||(dec_min >= -100.0)||(dec_max >= -100.0)) {
      AstroClean_Extract_AbsRegion(acsh,ra_min,ra_max,dec_min,dec_max);
    }

    /* Filter on region 2 */
    if(regionrad > 0.0) {
      AstroClean_Extract_Region(acsh,regionrad,regionra,regiondec);
    }

    /* Filter out duplicates based on immediate proximity */
    if(proxrad > 1e-20) {
      AstroClean_Duplicate_Filter(acsh,proxrad);
    }

    /* Filter out stationaries based on immediate proximity */
    if(stationrad > 1e-20) {
      AstroClean_Stationary_Filter(acsh,stationrad);
    }

    /* Filter on lines */
    if(linesupport > 0) {
      AstroClean_Linear_Filter(acsh, linerad, lineang, linesupport);
    }

    /* Filter on density. */
    if(density >= 0.0) {
      if(Dradius <= 0.0) {
        if(adaptD) {
          AstroClean_Field_Relative_Density_Filter(acsh,density);
        } else {
          AstroClean_Field_Density_Filter(acsh,density);
        }
      } else {
        if(adaptD) {
          AstroClean_Pointwise_Relative_DensityFilter(acsh,density,Dradius);
	} else {
          AstroClean_Pointwise_DensityFilter(acsh,density,Dradius);
	}
      }
    }
  
    /* Dump the clean detections. */
    printf(">> Dumping clean detections to output file.\n");
    fp = fopen(fout,"w");
    if(fp) {
      N = AstroClean_Num_Clean(acsh);
      for(i=0;i<N;i++) { 
        X = simple_obs_array_ref(obs,AstroClean_Clean_Ref(acsh,i));
	switch(outtype) {
	case AC_OUTPUT_MPC:  fprintf_MPC_simple_obs(fp,X); break;
	case AC_OUTPUT_MITI: fprintf_MITI_simple_obs(fp,X,NULL); break;
	}
      }
      fclose(fp);
    }

    /* Dump the noise detections (if wanted). */
    if(fout2) {
      printf(">> Dumping noise detections to output file.\n");
      fp = fopen(fout2,"w");
      if(fp) {
	N = AstroClean_Num_Noise(acsh);
	for(i=0;i<N;i++) { 
	  X = simple_obs_array_ref(obs,AstroClean_Noise_Ref(acsh,i));
	  switch(outtype) {
	  case AC_OUTPUT_MPC:  fprintf_MPC_simple_obs(fp,X); break;
	  case AC_OUTPUT_MITI: fprintf_MITI_simple_obs(fp,X,NULL); break;
	  }
	}
	fclose(fp);
      } 
    }
    
    /* Free the used memory. */
    AstroClean_Free(acsh); 
    free_simple_obs_array(obs);
  }
}


int main(int argc,char *argv[]) {
  memory_leak_check_args(argc,argv);
  Verbosity = 0.0;

  clean_main(argc,argv);

  am_malloc_report_polite();
  return 0;
}
