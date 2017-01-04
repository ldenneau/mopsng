/*
   File:        main.c
   Author:      J. Kubica
   Description: The main function for OrbitProximity.

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

#include "orbprox.h"
#include "orbit.h"
#include "orbit_tree.h"

#define ORBPROX_VERSION 1
#define ORBPROX_RELEASE 0
#define ORBPROX_UPDATE  3

int main(int argc,char *argv[]) {
  OrbitProximityStateHandle oph;
  char* fnameD = string_from_args("data",argc,argv,NULL);
  char* fnameQ = string_from_args("queries",argc,argv,NULL);
  char* fout1  = string_from_args("matchfile",argc,argv,"matches.txt");
  double q_thresh = double_from_args("q_thresh",argc,argv,0.01);
  double e_thresh = double_from_args("e_thresh",argc,argv,0.01);
  double i_thresh = double_from_args("i_thresh",argc,argv,0.1);
  double w_thresh = double_from_args("w_thresh",argc,argv,1.0);
  double O_thresh = double_from_args("O_thresh",argc,argv,1.0);
  double t_thresh = double_from_args("t_thresh",argc,argv,0.1);
  int    verb     = int_from_args("verbosity",argc,argv,0);
  int    noisepts = int_from_args("noise_pts",argc,argv,0);
  bool use_names = bool_from_args("use_names",argc,argv,FALSE);
  orbit_array* query = NULL;
  orbit_array* data  = NULL;
  orbit*       o;
  orbit*       o2;
  string_array* qnames;
  string_array* dnames;
  char* qname;
  char* dname;
  FILE* fp;
  int i, j, ind;

  memory_leak_check_args(argc,argv);

  printf("ORBITPROXIMITY VERSION: %i.%i.%i\n",
         ORBPROX_VERSION,ORBPROX_RELEASE,ORBPROX_UPDATE);
  printf("This program comes with ABSOLUTELY NO WARRANTY. This is free "
         "software, and you are welcome to redistribute it under certain "
         " conditions.  See included license for details.\n\n");

  if(fnameD != NULL) {
    printf("Data Orbit Files (data) = %s\n",fnameD);
  } else {
    printf("Data Orbit Files (data) = NOT GIVEN\n");
  }
  if(fnameQ != NULL) {
    printf("Query Orbit Files (queries) = %s\n",fnameQ);
  } else {
    printf("Query Orbit Files (queries) = NOT GIVEN\n");
  }

  if((fnameD != NULL)&(fnameQ != NULL)) {

    printf("--- Settings -------------------\n");
    printf("q Threshold (q_thresh) = %15.10f\n",q_thresh);
    printf("e Threshold (q_thresh) = %15.10f\n",e_thresh);
    printf("i Threshold (q_thresh) = %15.10f\n",i_thresh);
    printf("w Threshold (q_thresh) = %15.10f\n",w_thresh);
    printf("O Threshold (q_thresh) = %15.10f\n",O_thresh);
    printf("T Threshold (q_thresh) = %15.10f\n",t_thresh);
    printf("\nVerbosity (verbosity) = %i\n",verb);
    printf("\nNumber of noise points (noise_pts) = %i\n",noisepts);
    printf("--------------------------------\n");
 

    /* ---------------------------------------------------- */
    /* --- Functions for the orbit array ------------------ */
    /* ---------------------------------------------------- */

    /* Load the set of data orbits and query orbits from the file names */
    data  = mk_orbit_array_from_columned_file(fnameD,&dnames);
    query = mk_orbit_array_from_columned_file(fnameQ,&qnames);

    if(verb > 0) { 
      printf("Loaded %i data orbits and %i query orbits.\n",
            orbit_array_size(data),orbit_array_size(query));
    }
    if(verb > 1) { 
      printf("\n-----------------------------\n");
      printf("- Data Orbits ---------------\n");
      printf("-----------------------------\n");
      fprintf_orbit_array3(stdout,data);

      printf("\n-----------------------------\n");
      printf("- Query Orbits --------------\n");
      printf("-----------------------------\n");
      fprintf_orbit_array3(stdout,query);
    }

    /* Generate a series of "noise orbits" (for testing) */
    for(i=0;i<noisepts;i++) {
      ind = (int)range_random(0.0,(double)orbit_array_size(data)-0.0001);
      o   = orbit_array_ref(data,ind);
      o2  = mk_orbit2(orbit_t0(o) + range_random(-50.0,50.0),
		      orbit_q(o) + range_random(-0.2,0.2),
		      orbit_e(o) + range_random(-0.1,0.1),
		      orbit_i(o) + range_random(-0.01,0.01),
		      orbit_O(o) + range_random(-0.01,0.01),
		      orbit_w(o) + range_random(-0.01,0.01),
		      orbit_epoch(o),orbit_equinox(o));
      orbit_array_add(data,o2);
      free_orbit(o2);
    }

    OrbitProximity_Init(&oph,q_thresh,e_thresh,i_thresh,
			w_thresh,O_thresh,t_thresh,verb,stdout);

    /* Add all of the data orbits. */
    for(i=0;i<orbit_array_size(data);i++) {
      o = orbit_array_ref(data,i);
      OrbitProximity_AddDataOrbit(oph,
                                  orbit_q(o),
                                  orbit_e(o),
                                  orbit_i(o)*RAD_TO_DEG,
				  orbit_w(o)*RAD_TO_DEG,
                                  orbit_O(o)*RAD_TO_DEG,
				  orbit_t0(o),
                                  orbit_equinox(o));
    }

    /* Add all of the query orbits. */
    for(i=0;i<orbit_array_size(query);i++) {
      o = orbit_array_ref(query,i);
      OrbitProximity_AddQueryOrbit(oph,
                                   orbit_q(o),
                                   orbit_e(o),
                                   orbit_i(o)*RAD_TO_DEG,
				   orbit_w(o)*RAD_TO_DEG,
                                   orbit_O(o)*RAD_TO_DEG,
				   orbit_t0(o),
                                   orbit_equinox(o));
    }

    /* Run the actual proximity algorithm */
    OrbitProximity_Run(oph);

    fp = fopen(fout1,"w");
    if(fp != NULL) {
      for(i=0;i<orbit_array_size(query);i++) {
        for(j=0;j<OrbitProximity_Num_Matches(oph,i);j++) {

          qname = string_array_ref(qnames,i);
          dname = string_array_ref(dnames,OrbitProximity_Get_Match(oph,i,j));

          /* Output the asteroid names (if possible) */
          /* and the line numbers otherwise.         */
          if((strlen(qname) > 0) && (strlen(dname) > 0) && use_names) {
            fprintf(fp,"%s %s\n",qname,dname);
	  } else {
            fprintf(fp,"%i %i\n",i,OrbitProximity_Get_Match(oph,i,j));
	  }

	}
      }
      fclose(fp);
    } else {
      printf("ERROR: Unable to open output file [");
      printf(fout1);
      printf("] for writing.\n");
    }

    OrbitProximity_Free(oph);
    free_orbit_array(query);
    free_orbit_array(data);

    free_string_array(qnames);
    free_string_array(dnames);
  }
  
  am_malloc_report_polite();
  return 0;
}
