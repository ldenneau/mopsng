/*
  File:        main.c
  Author:      J. Kubica
  Description: The main function for DetectionProximity.

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

#include "detectprox.h"
#include "rdt_tree.h"

#define DETECTPROX_VERSION 1
#define DETECTPROX_RELEASE 0
#define DETECTPROX_UPDATE  0

int main(int argc,char *argv[]) {
  DetectionProximityStateHandle oph;
  char* fnameD = string_from_args("data",argc,argv,NULL);
  char* fnameQ = string_from_args("queries",argc,argv,NULL);
  char* fout1  = string_from_args("matchfile",argc,argv,"matches.txt");
  double d_thresh = double_from_args("d_thresh",argc,argv,1.0);
  double t_thresh = double_from_args("t_thresh",argc,argv,1.0);
  double b_thresh = double_from_args("b_thresh",argc,argv,1.0);
  int    verb     = int_from_args("verbosity",argc,argv,0);
  simple_obs_array* query = NULL;
  simple_obs_array* data  = NULL;
  simple_obs*       o;
  FILE* fp;
  int i, j;

  memory_leak_check_args(argc,argv);

  printf("DETECTIONPROXIMITY VERSION: %i.%i.%i\n",DETECTPROX_VERSION,
         DETECTPROX_RELEASE,DETECTPROX_UPDATE);
  printf("This program comes with ABSOLUTELY NO WARRANTY. This is free "
         "software, and you are welcome to redistribute it under certain "
         " conditions.  See included license for details.\n\n");

  if((fnameD != NULL)&(fnameQ != NULL)) {
    data  = mk_simple_obs_array_from_file(fnameD,0.0,NULL,NULL);
    query = mk_simple_obs_array_from_file(fnameQ,0.0,NULL,NULL);

    if(verb > 0) { 
      printf("Loaded %i data pointss and %i query pointss.\n",
            simple_obs_array_size(data),simple_obs_array_size(query));
    }
    if(verb > 1) { 
      printf("\n-----------------------------\n");
      printf("- Data Points ---------------\n");
      printf("-----------------------------\n");
      fprintf_simple_obs_array(stdout,data);

      printf("\n-----------------------------\n");
      printf("- Query Points --------------\n");
      printf("-----------------------------\n");
      fprintf_simple_obs_array(stdout,query);
    }

    DetectionProximity_Init(&oph,verb,stdout);

    /* Add all of the data orbits. */
    for(i=0;i<simple_obs_array_size(data);i++) {
      o = simple_obs_array_ref(data,i);
      DetectionProximity_AddDataDetection(oph,simple_obs_RA(o),simple_obs_DEC(o),
                                          simple_obs_time(o),simple_obs_brightness(o));
    }

    /* Add all of the query orbits. */
    for(i=0;i<simple_obs_array_size(query);i++) {
      o = simple_obs_array_ref(query,i);
      DetectionProximity_AddQueryDetection(oph,simple_obs_RA(o),simple_obs_DEC(o),
					   simple_obs_time(o),simple_obs_brightness(o),
					   d_thresh,b_thresh,t_thresh);
    }

    DetectionProximity_Run(oph);

    fp = fopen(fout1,"w");
    if(fp != NULL) {
      for(i=0;i<simple_obs_array_size(query);i++) {
        for(j=0;j<DetectionProximity_Num_Matches(oph,i);j++) {
          fprintf(fp,"%i %i\n",i,DetectionProximity_Get_Match(oph,i,j));
	}
      }
      fclose(fp);
    } else {
      printf("ERROR: Unable to open output file [");
      printf(fout1);
      printf("] for writing.\n");
    }

    DetectionProximity_Free(oph);
    free_simple_obs_array(query);
    free_simple_obs_array(data);
  }
  
  am_malloc_report_polite();
  return 0;
}
