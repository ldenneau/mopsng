/*
  Python wrapper for the Auton code (i.e. linkTracklets, findTracklets, 
  fieldProximity, detectionProximity, orbitProximity).
*/
#include "autonmodule.h"

#define true 1
#define false 0



/* Reads in an array of pw_linears where each line contains */
/* TRACK_ID TIME RA DEC where TRACK_ID is an 8 character ID */
/* names - an empty name to store the track id to inds.     */
/*         CAN BE set to NULL (and thus ignored).           */
/* RAdegrees - true off RA is given in degrees.  Otherwise  */
/*             is if given in hours.                        */
pw_linear_array* mk_load_RA_DEC_pw_linear_array_from_list(PyListObject* list, 
                                                          namer* names, 
                                                          bool RAdegrees) {
  pw_linear_array* res = NULL;
  pw_linear*       indiv;
  namer*           ids_to_inds;
  int i, j, ind;
  char *id = NULL;
  dyv *pt;
  bool givennamer;
  PyObject* seq;
  long size;
  
    
  if(names == NULL) {
    givennamer  = FALSE;
    ids_to_inds = mk_empty_namer(FALSE);
  } else {
    my_assert(namer_num_indexes(names)==0);

    givennamer  = TRUE;
    ids_to_inds = names;
  }  
  
  /* Extract data from the input list. */
  seq = PySequence_Fast((PyObject*)list, "expected a sequence");
  size = PySequence_Size((PyObject*)list);

  /* Allocate memory */
  res         = mk_empty_pw_linear_array_sized(10);
  pt          = mk_dyv(2);
    
  for(j=0; j<size; j++) {
    PyObject* orbit = PySequence_Fast(PySequence_Fast_GET_ITEM(seq, j),
                                      "expected a sequence");
    double ra, dec, mjd;
    
    
    /* Extract the ID string. */
    id = PyString_AS_STRING(PySequence_Fast_GET_ITEM(orbit, 0));
    if(!id)
      id = mk_printf("FAKEID");
    ind = namer_name_to_index(ids_to_inds, id);

    /* Extract the coordinates... */
    ra = PyFloat_AS_DOUBLE(PySequence_Fast_GET_ITEM(orbit, 2));
    if(RAdegrees)
      ra /= 15.0;
    dec = PyFloat_AS_DOUBLE(PySequence_Fast_GET_ITEM(orbit, 3));
    dyv_set(pt, 0, ra);
    dyv_set(pt, 1, dec);
    
    /* Extract the MJD */
    mjd = PyFloat_AS_DOUBLE(PySequence_Fast_GET_ITEM(orbit, 1));

    /* It we have seen this track before augment it otherwise create a new 
       track. */
    if(ind > -1) {
      indiv = pw_linear_array_ref(res, ind);
      pw_linear_add(indiv, mjd, pt);
    } else {
      indiv = mk_empty_pw_linear(2);
      pw_linear_add(indiv, mjd, pt);
      
      pw_linear_array_add(res, indiv);
      add_to_namer(ids_to_inds, id);
      
      free_pw_linear(indiv);
    }
    
    /* Free up the allocated memory. */
    Py_DECREF(orbit);
  } /* end for */
  free_dyv(pt);
  
  /* Make sure nothing weird happens at the 24.0->0.0 line. */
  if(res != NULL) {
    for(i=0; i<pw_linear_array_size(res); i++) {
      pw_linear_conv_RADEC(pw_linear_array_ref(res, i));
    }
  }

  /* Free memory and quit. */
  Py_DECREF(seq);
  if(givennamer==FALSE)
    free_namer(ids_to_inds);
  return res;
}


/* RAdegrees is true iff RA is given in degrees.  Otherwise uses */
/* RA in hours.                                                */
rd_plate_array* mk_load_rd_plate_array_from_list(PyListObject* list, 
                                                 bool RAdegrees) {
  rd_plate_array* res = mk_empty_rd_plate_array(10);
  rd_plate* indiv;
  long j;
  PyObject* seq;
  long size;
  
  
  /* Unpack list into the individual detections */
  seq = PySequence_Fast((PyObject*)list, "expected a sequence");
  size = PySequence_Size((PyObject*)list);
  for(j=0; j<size; j++) {
    char id[255];               /* A bit of a waste of space... */
    double mjd;
    double ra;
    double dec;
    double radius;
    PyObject* field;

    
    /* Fetch the field spec. */
    field = PySequence_Fast(PySequence_Fast_GET_ITEM(seq, j),
                            "expected a sequence");
    
    /* Extract id, time, ra, dec and radius. */
    sprintf(id, "%ld", PyInt_AsLong(PySequence_Fast_GET_ITEM(field, 0)));
    mjd = PyFloat_AS_DOUBLE(PySequence_Fast_GET_ITEM(field, 1));
    ra = PyFloat_AS_DOUBLE(PySequence_Fast_GET_ITEM(field, 2));
    if(RAdegrees)
      ra /= 15.0;
    dec = PyFloat_AS_DOUBLE(PySequence_Fast_GET_ITEM(field, 3));
    radius = PyFloat_AS_DOUBLE(PySequence_Fast_GET_ITEM(field, 4)) * DEG_TO_RAD;

    /* Create and add a new rd_plate element. */
    indiv = mk_rd_plate(id, mjd, ra, dec, radius);
    if(indiv != NULL) {
      rd_plate_array_add(res, indiv);
      free_rd_plate(indiv);
    }
  }
  Py_DECREF(seq);
  return(res);
}


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

  return(counts);
}

/* The output of this function is a Python dictionary of the form
   {field_id: [orbit1_id, orbit2_id, ...]}
*/
PyDictObject* dump_fp_results(rd_plate_array* fields, namer* orbits, 
                              ivec_array* mapping) {
  ivec* inds;
  int i, j;
  PyDictObject* res = (PyDictObject*)PyDict_New();
  
  
  if(res == NULL)
    Py_FatalError("Cannot init result dictionary.");
  
  for(i=0; i<ivec_array_size(mapping); i++) {
     PyListObject* list;
     long num_orbits;
     
     
     inds = ivec_array_ref(mapping, i);
     num_orbits = ivec_size(inds);
     if(!num_orbits) {
       continue;
     }
    
    /* Create the orbit_id list. */
    list = (PyListObject*)PyList_New((Py_ssize_t)num_orbits);
    if(list == NULL)
      Py_FatalError("Cannot init orbit list.");
    
    for(j=0; j<num_orbits; j++) {
      /* Add the orbit_id (i.e. object name) to the list. */
      PyList_SET_ITEM(list, 
                      (Py_ssize_t)j, 
                      PyString_FromString(namer_index_to_name(orbits, 
                                                              ivec_ref(inds, 
                                                                       j))));
    } /* end for */
    
    /* Add the entry to the dictionary. */
    if(PyDict_SetItemString((PyObject*)res,
                            rd_plate_id(rd_plate_array_ref(fields, i)),
                            (PyObject*)list) == -1)
      Py_FatalError("Cannot add orbit list to the result dictionary.");
  } /* end for */
  return(res);
}


orbit_array* mk_orbit_array_from_list(PyListObject* list, 
                                      string_array** names) {
  orbit_array*   res = NULL;
  orbit*         A;
  int size = 0;
  int j;
  PyObject* seq;
  
  
  if(names != NULL)
    names[0] = mk_string_array(0);
  
  /* Unpack list into the individual detections */
  seq = PySequence_Fast((PyObject*)list, "expected a sequence");
  size = PySequence_Size((PyObject*)list);
  res = mk_empty_orbit_array_sized(size);
  
  for(j=0; j<size; j++) {
    PyObject* orbit;

    
    /* Fetch the field spec. */
    orbit = PySequence_Fast(PySequence_Fast_GET_ITEM(seq, j),
                            "expected a sequence");
    
    /* Create the orbit and add it to the orbit array res.*/
    /* FIXME: Not clear whet they mean by the last member (equinox). */
    A = mk_orbit2(PyFloat_AS_DOUBLE(PySequence_Fast_GET_ITEM(orbit, 5)),
                  PyFloat_AS_DOUBLE(PySequence_Fast_GET_ITEM(orbit, 0)),
                  PyFloat_AS_DOUBLE(PySequence_Fast_GET_ITEM(orbit, 1)),
                  PyFloat_AS_DOUBLE(PySequence_Fast_GET_ITEM(orbit, 
                                                             2)) * DEG_TO_RAD,
                  PyFloat_AS_DOUBLE(PySequence_Fast_GET_ITEM(orbit, 
                                                             3)) * DEG_TO_RAD,
                  PyFloat_AS_DOUBLE(PySequence_Fast_GET_ITEM(orbit, 
                                                             4)) * DEG_TO_RAD,
                  PyFloat_AS_DOUBLE(PySequence_Fast_GET_ITEM(orbit, 7)),
                  PyFloat_AS_DOUBLE(PySequence_Fast_GET_ITEM(orbit, 7)));
    orbit_array_add(res, A);
    free_orbit(A);

    if(names != NULL && PySequence_Size((PyObject*)orbit) >= 9) {
      char* id = id = PyString_AS_STRING(PySequence_Fast_GET_ITEM(orbit, 8));
      if(id)
        add_to_string_array(names[0], id);
      else
        add_to_string_array(names[0], "");
    }
  }
  return(res);
}


PyDictObject* orbitproximity_main(PyListObject* knownOrbits, 
                                  PyListObject* queryOrbits, 
                                  double q_thresh,
                                  double e_thresh,
                                  double i_thresh,
                                  double w_thresh,
                                  double O_thresh,
                                  double t_thresh,
                                  int noise_pts) {
  OrbitProximityStateHandle oph;
  orbit_array* query = NULL;
  orbit_array* data  = NULL;
  orbit*       o;
  orbit*       o2;
  string_array* qnames;
  string_array* dnames;
  int i, j, ind;
  PyDictObject* result;
 

  /* Load the set of data orbits and query orbits from the file names */
  data  = mk_orbit_array_from_list(knownOrbits, &dnames);
  query = mk_orbit_array_from_list(queryOrbits, &qnames);

  /* Generate a series of "noise orbits" (for testing) */
  for(i=0; i<noise_pts; i++) {
    ind = (int)range_random(0.0, (double)orbit_array_size(data) - 0.0001);
    o = orbit_array_ref(data, ind);
    o2 = mk_orbit2(orbit_t0(o) + range_random(-50.0, 50.0),
                   orbit_q(o) + range_random(-0.2, 0.2),
                   orbit_e(o) + range_random(-0.1, 0.1),
                   orbit_i(o) + range_random(-0.01, 0.01),
                   orbit_O(o) + range_random(-0.01, 0.01),
                   orbit_w(o) + range_random(-0.01, 0.01),
                   orbit_epoch(o), orbit_equinox(o));
    orbit_array_add(data, o2);
    free_orbit(o2);
  }

  OrbitProximity_Init(&oph, q_thresh, e_thresh, i_thresh,
                      w_thresh, O_thresh, t_thresh, false, stdout);

  /* Add all of the data orbits. */
  for(i=0; i<orbit_array_size(data); i++) {
    o = orbit_array_ref(data, i);
    OrbitProximity_AddDataOrbit(oph,
                                orbit_q(o),
                                orbit_e(o),
                                orbit_i(o) * RAD_TO_DEG,
                                orbit_w(o) * RAD_TO_DEG,
                                orbit_O(o) * RAD_TO_DEG,
                                orbit_t0(o),
                                orbit_equinox(o));
  }

  /* Add all of the query orbits. */
  for(i=0; i<orbit_array_size(query); i++) {
    o = orbit_array_ref(query,i);
    OrbitProximity_AddQueryOrbit(oph,
                                 orbit_q(o),
                                 orbit_e(o),
                                 orbit_i(o) * RAD_TO_DEG,
                                 orbit_w(o) * RAD_TO_DEG,
                                 orbit_O(o) * RAD_TO_DEG,
                                 orbit_t0(o),
                                 orbit_equinox(o));
  }

  /* Run the actual proximity algorithm */
  OrbitProximity_Run(oph);
  
  /* Create the output list. */
  result = (PyDictObject*)PyDict_New();

  for(i=0;i<orbit_array_size(query);i++) {
    int num_matches = OrbitProximity_Num_Matches(oph,i);
    
    
    if(!num_matches) 
      continue;
    
    /* Create the list of matches. */
    PyListObject* list = (PyListObject*)PyList_New((Py_ssize_t)num_matches);
    for(j=0; j<num_matches; j++) {
      PyList_SET_ITEM(list,  
                      (Py_ssize_t)j, 
                      PyString_FromString(string_array_ref(dnames,
                                           OrbitProximity_Get_Match(oph,i,j))));
    }
    /* Create the dictionary entry. */
    if(PyDict_SetItemString((PyObject*)result,
                            string_array_ref(qnames,i),
                            (PyObject*)list) == -1)
      Py_FatalError("Cannot add orbit list to the result dictionary.");
  }
  OrbitProximity_Free(oph);
  free_orbit_array(query);
  free_orbit_array(data);
  free_string_array(qnames);
  free_string_array(dnames);
  return(result);
}


PyDictObject* detectionproximity_main(PyListObject* dataDets, 
                                      PyListObject* queryDets, 
                                      double d_thresh,
                                      double b_thresh,
                                      double t_thresh) {
  DetectionProximityStateHandle oph;
  simple_obs_array* query = NULL;
  simple_obs_array* data  = NULL;
  simple_obs*       o;
  int i, j;
  long* dataDetIds = NULL;      /* Mapping between DP internal ids and ours. */
  long* queryDetIds = NULL;     /* Mapping between DP internal ids and ours. */
  long nDataDets = 0;
  long nQueryDets = 0;
  
  /* Make sure that we can actually allocate the result dict. */
  PyDictObject* result = (PyDictObject*)PyDict_New();
  if(result == NULL)
    Py_FatalError("Cannot init result dictionary.");
  

  
  /* Create the data and query arrays. */
  data =  mk_simple_obs_array_from_PANSTARRS_list((PyObject*)dataDets, 
                                                  0, 
                                                  NULL, 
                                                  NULL, 
                                                  NULL, 
                                                  NULL, 
                                                  NULL);
  query =  mk_simple_obs_array_from_PANSTARRS_list((PyObject*)queryDets, 
                                                   0, 
                                                   NULL, 
                                                   NULL, 
                                                   NULL, 
                                                   NULL, 
                                                   NULL);

  nDataDets = simple_obs_array_size(data);
  nQueryDets = simple_obs_array_size(query);
  dataDetIds = (long*)malloc(sizeof(long) * nDataDets);
  queryDetIds = (long*)malloc(sizeof(long) * nQueryDets);
  if(dataDetIds == NULL || queryDetIds == NULL)
    Py_FatalError("Cannot init ID mapping dictionaries.");


  /* Init the detection proximity library. */
  DetectionProximity_Init(&oph, 0, stdout);

  /* Add all of the data orbits. */
  for(i=0;i<nDataDets;i++) {
    o = simple_obs_array_ref(data,i);
    DetectionProximity_AddDataDetection(oph,
                                        simple_obs_RA(o),
                                        simple_obs_DEC(o),
                                        simple_obs_time(o),
                                        simple_obs_brightness(o));
    dataDetIds[i] = atol(simple_obs_id_str(o));
  }

  /* Add all of the query orbits. */
  for(i=0;i<nQueryDets;i++) {
    o = simple_obs_array_ref(query,i);
    DetectionProximity_AddQueryDetection(oph,
                                         simple_obs_RA(o),
                                         simple_obs_DEC(o),
                                         simple_obs_time(o),
                                         simple_obs_brightness(o),
                                         d_thresh,
                                         b_thresh,
                                         t_thresh);
    queryDetIds[i] = atol(simple_obs_id_str(o));
  }
  
  /* Run detection proximity. */
  DetectionProximity_Run(oph);

  /* Create the discionary to hold the results. */
  for(i=0;i<simple_obs_array_size(query);i++) {
    PyListObject* list;
    long n;
    
    
    n = DetectionProximity_Num_Matches(oph,i);
    if(!n)
      continue;
    
    /* Create a list of detection matches. */
    list = (PyListObject*)PyList_New((Py_ssize_t)n);
    if(list == NULL)
      Py_FatalError("Cannot init output detection list.");
    
    /* Add each element to the list. */
    for(j=0;j<n;j++) {
      PyList_SET_ITEM(list, 
                      (Py_ssize_t)j, 
                      PyLong_FromLong(dataDetIds[DetectionProximity_Get_Match(oph,i,j)]));
    }
    
    /* Create an entry in the dictionary. */
    if(PyDict_SetItem((PyObject*)result,
                      PyLong_FromLong(queryDetIds[i]),
                      (PyObject*)list) == -1)
      Py_FatalError("Cannot add detection list to the result dictionary.");

    /* Dispose of list. */
    Py_DECREF((PyObject*)list);
  }
  DetectionProximity_Free(oph);
  free_simple_obs_array(query);
  free_simple_obs_array(data);
  free(dataDetIds);
  free(queryDetIds);

  return(result);
}


PyDictObject* fieldproximity_main(PyListObject* fieldList,
                                  PyListObject* orbitList,
                                  double thresh,
                                  int method,
                                  int pleaf,
                                  int tleaf,
                                  bool split_all) {
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
  int i, j;


  /* Turn the threashold value into radians. */
  thresh *= DEG_TO_RAD;

  /* Load the data sets. */
  track_id_to_ind = mk_empty_namer(TRUE);
  if(fieldList)
    parr = mk_load_rd_plate_array_from_list(fieldList, true);
  if(orbitList) 
    tarr = mk_load_RA_DEC_pw_linear_array_from_list(orbitList,
                                                    track_id_to_ind,
                                                    true);
  if((parr == NULL) || (tarr == NULL)) {
    Py_FatalError("Error in loading input data sets.");
    Py_INCREF(Py_None);
    return((PyDictObject*)Py_None);
  }
    
  /* Find the start and end time of the plates. */
  if(rd_plate_array_size(parr) <= 0) {
    Py_FatalError("Error in finding plate boundaries.");
    Py_INCREF(Py_None);
    return((PyDictObject*)Py_None);
  }
  pts = rd_plate_time(rd_plate_array_ref(parr, 0));
  pte = rd_plate_time(rd_plate_array_ref(parr, 0));
  for(i=1; i<rd_plate_array_size(parr); i++) {
    t = rd_plate_time(rd_plate_array_ref(parr, i));
    if(t < pts)
      pts = t;
    if(t > pte)
      pte = t;
  }

  /* Find the start and end time of the segments. */
  if(pw_linear_array_size(tarr) <= 0) {
    Py_FatalError("Error in finding start and end time of segments.");
    Py_INCREF(Py_None);
    return((PyDictObject*)Py_None);
  }
  T  = pw_linear_array_ref(tarr, 0);
  ts = pw_linear_x(T, 0);
  te = pw_linear_x(T, pw_linear_size(T) - 1);
  
  if((te < pte)||(ts > pts)) {
    Py_FatalError("Error in time boundaries.");
    Py_INCREF(Py_None);
    return((PyDictObject*)Py_None);
  }
    
  /* Check that the tracks all line up in time... */
  A = pw_linear_array_ref(tarr, 0);
  for(i=1; i<pw_linear_array_size(tarr); i++) {
    B = pw_linear_array_ref(tarr, i);
    if(pw_linear_size(A) != pw_linear_size(B)) {
      Py_FatalError("Tracks do not line up in time.");
      Py_INCREF(Py_None);
      return((PyDictObject*)Py_None);
    }
    
    for(j=0; j<pw_linear_size(A); j++) {
      if(fabs(pw_linear_x(A,j)-pw_linear_x(B,j)) > 1e-10) {
        Py_FatalError("Tracks do not line up in time (2).");
        Py_INCREF(Py_None);
        return((PyDictObject*)Py_None);
      }
    } /* end for */
  }   /* end for */
  
  /* Do the actual search */
  switch(method) {
  case 0: 
    res = mk_exhaustive(tarr, parr, thresh);
    break;
  case 1:
    ptr = mk_plate_tree(parr,1.0,1.0,1.0,pleaf);
    res = mk_plate_tree_int_search(ptr,parr,tarr,thresh);
    free_plate_tree(ptr);
    break;
  case 2:
    ttr = mk_pw_tree(tarr,0.0,10.0,tleaf,split_all);
    res = mk_pw_tree_search(ttr, tarr, parr, thresh);
    free_pw_tree(ttr);
    break;
  case 3:
    ptr = mk_plate_tree(parr,1.0,1.0,1.0,pleaf);
    ttr = mk_pw_tree(tarr,0.0,10.0,tleaf,split_all);
    res = mk_dual_tree_search(ttr,tarr,ptr,parr,thresh);
    free_pw_tree(ttr);
    free_plate_tree(ptr);
    break;
  default:
    Py_FatalError("Not a valid matching option\n");
    Py_INCREF(Py_None);
    return((PyDictObject*)Py_None);
  }

  int count = 0;
  for(i=0;i<ivec_array_size(res);i++) {
    count += ivec_size(ivec_array_ref(res,i));
  } 

  /* Dump everything to the output dictionary. */
  PyDictObject* result = dump_fp_results(parr, track_id_to_ind, res);
  // free_ivec_array(res);
    
  /* Free the data sets. */
  free_namer(track_id_to_ind);
  if(parr != NULL)
    free_rd_plate_array(parr);
  if(tarr != NULL)
    free_pw_linear_array(tarr);
  return(result);
}


PyListObject* linktracklets_main(ivec* true_groups,
                                 ivec* true_pairs,
                                 simple_obs_array* obs,
                                 double fit_thresh,
                                 double lin_thresh,
                                 double quad_thresh,
                                 double pred_thresh,
                                 double vtree_thresh,
                                 double min_overlap,
                                 double plate_width,
                                 double roc_thresh,
                                 double maxv,
                                 double minv,
                                 double sigma,
                                 double acc_r,
                                 double acc_d,
                                 int    seed,
                                 int    min_sup,
                                 int    max_hyp,
                                 int    max_match,
                                 int    min_obs,
                                 bool   bwpass,
                                 bool   endpts,
                                 bool   removedups,
                                 bool   rem_overlap,
                                 bool   allow_conflicts,
                                 double start_t_range,
                                 double end_t_range) {
  double r_lo_n, r_hi_n, d_lo_n, d_hi_n, t_lo_n, t_hi_n;
  double r_lo, r_hi, d_lo, d_hi, t_lo, t_hi;
  ivec* track_sizes;
  dyv* org_times;
  dyv* times;
  simple_obs* A;
  simple_obs* B;
  track_array* t1;
  track_array* t2;
  track_array* t3;
  double dist, dtime;
  int tcount = 0;
  int i;
  int search_type = 0;
  char *s = "linkTracklets";
  double last_start_obs_time = -1.0;
  double first_end_obs_time = -1.0;

  /* Set the random seed and the search mode */
  /* FIXME: I guess this way of selecting the search type is wrong. */
  if(seed) am_srand(seed);
  if( eq_string(s,"vtree") )    { search_type = 0; }
  if( eq_string(s,"seq") )      { search_type = 1; }
  if( eq_string(s,"seqaccel") ) { search_type = 2; }


  /* Convert things into useful units (radians). */
  vtree_thresh *= DEG_TO_RAD;
  acc_r        *= DEG_TO_RAD;
  acc_d        *= DEG_TO_RAD;

  if(obs == NULL) {
    Py_INCREF(Py_None);
    return((PyListObject*)Py_None);
  }

  /* Start by recentering the tracks. */
  simple_obs_array_compute_bounds(obs,NULL,&r_lo,&r_hi,&d_lo,
                                  &d_hi,&t_lo,&t_hi);
  recenter_simple_obs_array(obs,NULL,(r_hi+r_lo)/2.0,12.0,
                            (d_lo+d_hi)/2.0,0.0,t_lo,0.0);
  simple_obs_array_compute_bounds(obs,NULL,&r_lo_n,&r_hi_n,
                                  &d_lo_n,&d_hi_n,&t_lo_n,&t_hi_n);
  
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
    
  /* Provide the option to add gaussian noise. */
  if(sigma > 1e-10) {
    simple_obs_array_add_gaussian_noise(obs, sigma/15.0, sigma, 0.0);
  }
    
  t1 = mk_true_tracks(obs,true_pairs);
    
  track_sizes = mk_count_tracks_iv_ind(true_groups);
  free_ivec(track_sizes);

  /* Save the original times and flatten the obs. */
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
    
  if((maxv < 999.0)||(minv > 1e-10)) {
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
  }
    
  /* Do the actual searching... */
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
    t2 = mk_sequential_accel_only_tracks(obs,t1,vtree_thresh,acc_r,acc_d,
                                         min_sup, fit_thresh,pred_thresh,
                                         plate_width,
                                         last_start_obs_time, 
                                         first_end_obs_time);
    break;
  default:
    t2 = NULL;
  }
    
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
    t3 = mk_MHT_remove_subsets(t2,obs);
    free_track_array(t2);
    t2 = t3;
  }
    
  if(rem_overlap) {
    t3 = mk_MHT_remove_overlaps(t2,obs,FALSE,0.5);
    free_track_array(t2);
    t2 = t3;
  }
  t3 = mk_order_tracks_by_trust(t2, obs);
  free_track_array(t2);
  t2 = t3;

  track_array_unflatten_to_plates(t1, obs, org_times);

  /* Put the bounds back they way they were. */
  if(search_type < 5) {
    recenter_simple_obs_array(obs,NULL,12.0,(r_hi+r_lo)/2.0,0.0,
                              (d_lo+d_hi)/2.0,0.0,t_lo);
    simple_obs_array_compute_bounds(obs,NULL,&r_lo,&r_hi,&d_lo,&d_hi,&t_lo,
                                    &t_hi);
  }
    
  /* Spit out the tracklets */
  PyListObject* tracklets = (PyListObject*)dump_tracks(obs, t2);

  free_simple_obs_array(obs);
  free_track_array(t2);
  free_track_array(t1);
  free_ivec(true_groups);
  free_ivec(true_pairs);
  free_dyv(org_times);
    
  return(tracklets);
}

PyListObject* dump_tracks(simple_obs_array* obs, track_array* trcks) {
  track* A;
  ivec* inds;
  int maxobj = track_array_size(trcks);
  int i, j;
  char  nme[7];
  PyListObject* tracks = (PyListObject*)PyList_New((Py_ssize_t)0);
  if(tracks == NULL) 
    Py_FatalError("Cannot init tracks list.");


  for(i=0;i<6;i++) {
    nme[i] = '0';
  }
  nme[6] = 0;

  /* In reality, what the input data calls observation/detection IDs are really
     tracklet IDs and are used to group detections together. The implication is
     that if we output the "detection IDs" for each tracklet in a track, we end
     up with the same tracklet ID repeated n times, where n is the number of 
     detections in that tracklet. We therefore, simply output the first of these
     IDs per tracklet. */
  for(i=0; i<maxobj; i++) {
    /* Fetch the i-th track. */
    A = track_array_ref(trcks, i);
    if((A != NULL) && (simple_obs_time(track_last(A, obs)) - 
                       simple_obs_time(track_first(A, obs)) > 0.8)) {
      /* Extract the individual tracklets. */
      inds = track_individs(A);

      /* Create an array for the tracklet ids. */
      long num_dets = ivec_size(inds);
      
      /* Create an empty Python list to hold tracklet IDs. */
      PyListObject* tracklets = (PyListObject*)PyList_New((Py_ssize_t)0);
      if(tracklets == NULL)
        Py_FatalError("Cannot init tracklets list.");
      
      /* Only fetch unique IDs in inds and add them to tracklets. */
      long long ref_id = -1;
      for(j=0; j<num_dets; j++) {
        long long cur_id;
        
        cur_id = atoll(simple_obs_id_str(simple_obs_array_ref(obs, 
                       ivec_ref(inds, j))));
        if(j == 0) {
          /* This is the first time we enter the loop. */
          ref_id = cur_id;
          
          /* Add this tracklet ID to tracklets. */
          PyList_Append((PyObject*)tracklets, 
                        PyLong_FromLong(cur_id));
        } else if(cur_id != ref_id) {
          /* We have a new tracklet! Add it tracklet ID to tracklets. */
          PyList_Append((PyObject*)tracklets, PyLong_FromLong(cur_id));
          
          /* Reset ref_id */
          ref_id = cur_id;
        }
      }
      
      /* Add the tracklet list to the track. */
      PyList_Append((PyObject*)tracks, (PyObject*)tracklets);
      Py_DECREF(tracklets);
    }
    incr_track_name2(nme);
  }
  // Py_DECREF(tracklets);
  return(tracks);
}

simple_obs_array* mk_simple_obs_array_from_PANSTARRS_list(PyObject* list, 
                                                          int start_id, 
                                                          ivec** groups, 
                                                          ivec** pairs,
                                                          dyv** length, 
                                                          dyv** angle,  
                                                          dyv** exp_time) {
  PyObject* seq = NULL;
  int j, ind;
  simple_obs_array* res = NULL;
  simple_obs*       A;
  namer*            nm1;
  namer*            nm2;
  int size = 0; 
  double tmp;
  long id;
  
  
  /* Unpack list into the individual detections */
  seq = PySequence_Fast(list, "expected a sequence");
  size = PySequence_Size(list);

/*   printf("Got %d detections\n", size); */

  res = mk_empty_simple_obs_array(size);
  nm1 = mk_empty_namer(TRUE);
  nm2 = mk_empty_namer(TRUE);
  if(pairs != NULL)
    pairs[0] = mk_constant_ivec(size, -1);
  if(groups != NULL) 
    groups[0] = mk_constant_ivec(size, -1);
  if(length != NULL) 
    length[0] = mk_constant_dyv(size, -1.0);
  if(angle != NULL)  
    angle[0] = mk_zero_dyv(size);
  if(exp_time != NULL)  
    exp_time[0] = mk_zero_dyv(size);
  char* detName = (char*)malloc(sizeof(char) * 255);
  
  for(j=0; j<size; j++) {
    /* Unpack detection into its fields */
    PyObject* detSeq = NULL;
    double mjd, ra, dec, mag;
    long obsid;
    int detLen;                 /* In reality we do not need to compute 
                                   detLen every time since it is always the 
                                   same. */
    
    detSeq = PySequence_Fast(PySequence_Fast_GET_ITEM(seq, j), 
                             "expected a sequence");
    detLen = PySequence_Size(detSeq); /* We do this here ONLY to support 
                                         multiformat input files. */
    
/*     printf("Detection %d: %d elements\n", j, detLen); */
    
    id = PyInt_AS_LONG(PySequence_Fast_GET_ITEM(detSeq, 0));
    sprintf(detName, "%ld", id);
    
    mjd = PyFloat_AS_DOUBLE(PySequence_Fast_GET_ITEM(detSeq, 1));
    ra = PyFloat_AS_DOUBLE(PySequence_Fast_GET_ITEM(detSeq, 2));
    dec = PyFloat_AS_DOUBLE(PySequence_Fast_GET_ITEM(detSeq, 3));
    mag = PyFloat_AS_DOUBLE(PySequence_Fast_GET_ITEM(detSeq, 4));
    obsid = PyInt_AS_LONG(PySequence_Fast_GET_ITEM(detSeq, 5));
    A = mk_simple_obs_time(start_id,
                           mjd,
                           ra/15.0,
                           dec,
                           mag,
                           'V',
                           (int)obsid,
                           detName);
    
    /* Load the tracklet name into pairs if nessecary. */        
    if(pairs != NULL) {
      add_to_namer(nm2, detName);
      ind = namer_name_to_index(nm2, detName);
      ivec_set(pairs[0], start_id, ind);
    }

    /* Load the object name into groups if nessecary. */        
    if((groups != NULL) && (detLen >= 7)) {
      char* name = NULL;

      
      name = PyString_AS_STRING(PySequence_Fast_GET_ITEM(detSeq, 6));
      
      /* Get the index of this name */
      if(!eq_string(name, "FALSE") && !eq_string(name, "NA") && !eq_string(name, "NS")) {
        add_to_namer(nm1, name);
        ind = namer_name_to_index(nm1, name);
        ivec_set(groups[0], start_id, ind);
      } else {
        ivec_set(groups[0], start_id, -1);
      }

      /*free(name);     don't do this with PyString_AS_STRING(); see http://docs.python.org/api/stringObjects.html */
    }
    
    if((length != NULL) && (detLen >= 8)) {
      tmp = PyFloat_AS_DOUBLE(PySequence_Fast_GET_ITEM(detSeq, 7)) * DEG_TO_RAD;
      dyv_set(length[0], start_id, tmp);
    }
    
    if((angle != NULL) && (detLen >= 9)) {
      tmp = PyFloat_AS_DOUBLE(PySequence_Fast_GET_ITEM(detSeq, 8)) * DEG_TO_RAD;
      dyv_set(angle[0], start_id, tmp);
    }
    
    if((exp_time != NULL) && (detLen >= 10)) {
      tmp = PyFloat_AS_DOUBLE(PySequence_Fast_GET_ITEM(detSeq, 9));
      dyv_set(*exp_time, start_id, tmp / (24.0 * 60.0 * 60.0));
    }
    
    simple_obs_array_add(res, A);
    start_id++;
    
    free_simple_obs(A);
    Py_DECREF(detSeq);

  }
  free(detName);
  free_namer(nm1);
  free_namer(nm2);
  Py_DECREF(seq);
  return(res);
}


/* Wrapper code and Python doc. */
/* LinkTracklets */
static char linktracklets_main__doc__[] = "Main entry point for LinkTracklets.";
PyListObject* wrap_linktracklets_main(PyObject *self, 
                                      PyObject *args, 
                                      PyObject *kw){
  /* Define the keyword names */
  static char *kwlist[] = {"detections",
                           "fit_thresh",
                           "lin_thresh",
                           "quad_thresh",
                           "pred_thresh",
                           "vtree_thresh",
                           "min_overlap",
                           "plate_width",
                           "roc_thresh",
                           "maxv",
                           "minv",
                           "sigma",
                           "acc_r",
                           "acc_d",
                           "seed",
                           "min_sup",
                           "max_hyp",
                           "max_match",
                           "min_obs",
                           "bwpass",
                           "endpts",
                           "remove_subsets",
                           "remove_overlaps",
                           "allow_conflicts",
                           "start_t_range",
                           "end_t_range",
                           NULL};
  /* Define all the vars and give optional keywords default values */
  PyObject* detectionList;
  double fit_thresh = 0.0001;
  double lin_thresh = 0.05;
  double quad_thresh = 0.02;
  double pred_thresh = 0.0005;
  double vtree_thresh = 0.0002;
  double min_overlap = 0.5;
  double plate_width = 0.001;
  double roc_thresh = 1.0;
  double maxv = 1000.0;
  double minv = 0.0;
  double sigma = 0.0;
  double acc_r = 0.02;
  double acc_d = 0.02;
  int seed = 1;
  int min_sup = 3;
  int max_hyp = 500;
  int max_match = 500;
  int min_obs = 6;
  bool bwpass = true;
  bool endpts = true;
  bool remove_subsets = true;
  bool remove_overlaps = false;
  bool allow_conflicts = false;
  double start_t_range = -1.0;
  double end_t_range = -1.0;

  /* Support variables */
  ivec* groups;
  ivec* pairs;
    
    
  /* Parse args and keywords */
  if(!PyArg_ParseTupleAndKeywords(args, 
                                  kw, 
                                  "O|dddddddddddddiiiiibbbbbdd", 
                                  kwlist, 
                                  &detectionList,
                                  &fit_thresh,
                                  &lin_thresh,
                                  &quad_thresh,
                                  &pred_thresh,
                                  &vtree_thresh,
                                  &min_overlap,
                                  &plate_width,
                                  &roc_thresh,
                                  &maxv,
                                  &minv,
                                  &sigma,
                                  &acc_r,
                                  &acc_d,
                                  &seed,
                                  &min_sup,
                                  &max_hyp,
                                  &max_match,
                                  &min_obs,
                                  &bwpass,
                                  &endpts,
                                  &remove_subsets,
                                  &remove_overlaps,
                                  &allow_conflicts,
                                  &start_t_range,
                                  &end_t_range))
    return NULL;
    
  /* TODO: Support detection elongation and angle information. */
  simple_obs_array* res;
  res = mk_simple_obs_array_from_PANSTARRS_list(detectionList,
                                                0,
                                                &groups,
                                                &pairs,
                                                NULL,      /* length */
                                                NULL,      /* angle */
                                                NULL);     /* exp_time */
    
  /* Now invoke linktracklets_main */
  PyListObject* tracklets;
  tracklets = linktracklets_main(groups,
                                 pairs,
                                 res,
                                 fit_thresh,
                                 lin_thresh,
                                 quad_thresh,
                                 pred_thresh,
                                 vtree_thresh,
                                 min_overlap,
                                 plate_width,
                                 roc_thresh,
                                 maxv,
                                 minv,
                                 sigma,
                                 acc_r,
                                 acc_d,
                                 seed,
                                 min_sup,
                                 max_hyp,
                                 max_match,
                                 min_obs,
                                 bwpass,
                                 endpts,
                                 remove_subsets,
                                 remove_overlaps,
                                 allow_conflicts,
                                 start_t_range,
                                 end_t_range);
  return(tracklets);
};

/* FindTracklets */
static char findtracklets_main__doc__[] = "Main entry point for FindTracklets.";
PyListObject* wrap_findtracklets_main(PyObject *self, 
                                      PyObject *args, 
                                      PyObject *kw){
  /* Define the keyword names */
  static char *kwlist[] = {"detections",
                           "athresh",
                           "thresh",
                           "maxLerr",
                           "etime",
                           "minv",
                           "maxv",
                           "maxt",
                           "minobs",
                           "maxobs",
                           "remove_subsets",
                           "greedy",
			   "pht",
                           NULL};
  /* Define all the vars and give optional keywords default values */
  double athresh = 180.0;
  double thresh = 0.0003;
  double maxLerr = 0.00015;
  double etime = 30.0;
  double minv = 0.0;
  double maxv = 0.5;
  double maxt = 0.5;
  int minobs = 2;
  int maxobs = 100;
  bool removedups = true;
  bool greedy = false;
  bool pht = false;

  /* Support variables */
  simple_obs_array* obs;
  track_array* trcks;
  dyv*  length;
  dyv*  angle;
  dyv*  exp_time;
  ivec* groups;
  ivec* inds;
  int i,j;
  long num_trks;
  PyObject* detectionList;
  PyListObject* tracklets;
  
  
  /* Parse args and keywords */
  if(!PyArg_ParseTupleAndKeywords(args, 
                                  kw, 
                                  "O|dddddddiibbb",
                                  kwlist,
                                  &detectionList,
                                  &athresh, 
                                  &thresh, 
                                  &maxLerr, 
                                  &etime,
                                  &minv, 
                                  &maxv, 
                                  &maxt, 
                                  &minobs, 
                                  &maxobs,
                                  &removedups,
                                  &greedy,
                                  &pht)) {
    Py_INCREF(Py_None);
    return((PyListObject*)Py_None);
  }
  minv *= DEG_TO_RAD;
  maxv *= DEG_TO_RAD;
  thresh *= DEG_TO_RAD;
  athresh *= DEG_TO_RAD;
  maxLerr *= DEG_TO_RAD;
        
  /* Make sure the exposure time is realistic and in days. */
  if(etime < 0.01)
    etime = 0.01;
  etime /= (24.0 * 60.0 * 60.0);
  
/*   printf("parsed input args.\n"); */

  /* Parse the detectionList and build a list of observation structures. */
  obs = mk_simple_obs_array_from_PANSTARRS_list(detectionList,
                                                maxt,
                                                &groups,
                                                NULL,
                                                &length,
                                                &angle, 
                                                &exp_time);
  if((obs == NULL) || (simple_obs_array_size(obs) <= 0)) {
    Py_INCREF(Py_None);
    printf("Ops: empty imput list.\n");
    return((PyListObject*)Py_None);
  }
//  Py_DECREF(detectionList);
  
/*   printf("got %d detections\n", n); */

  /* Create the tracklets */
  trcks = mk_tracklets_MHT(obs,
                           minv,
                           maxv,
                           thresh,
                           maxt,
                           minobs,
                           removedups,
                           angle,
                           length,
                           exp_time,
                           athresh,
                           maxLerr,
                           etime,
                           maxobs,
                           greedy,
                           pht);
   /*printf("got %d tracklets\n", track_array_size(trcks)); */

  /* Convert the tracklets to a Python list */
  num_trks = track_array_size(trcks);
  tracklets = (PyListObject*)PyList_New((Py_ssize_t)num_trks);
  if(tracklets == NULL) 
    Py_FatalError("Cannot init tracklets list.");

  for(i=0; i<num_trks; i++) {
    long num_dets;
    PyListObject* detections;
        
    inds = track_individs(track_array_ref(trcks, i));
    num_dets = ivec_size(inds);
    
/*     printf("Tracklet %d: %ld detections\n", i, num_dets); */
        
    /* Create a Python list to hold the detections. */
    detections = (PyListObject*)PyList_New((Py_ssize_t)num_dets);
    if(detections == NULL)
      Py_FatalError("Cannot init detections list.");

    for(j=0; j<num_dets; j++) {
      /* Insert the ID into detections */
      PyList_SET_ITEM(detections, 
                      (Py_ssize_t)j, 
                      PyLong_FromString(simple_obs_id_str(simple_obs_array_ref(obs, ivec_ref(inds, j))), NULL, 10));
    }
    /* Add the detection list to the tracklet. */
    PyList_SET_ITEM((PyObject*)tracklets, 
                    (Py_ssize_t)i,
                    (PyObject*)detections);
    //Py_DECREF(detections);
  }
    
  /* Cleanup and quit */
  free_simple_obs_array(obs);
  free_ivec(groups);
  // Do not free trcks: Python still needs it.... maybe.
  // free_track_array(trcks);
  if(length != NULL)
    free_dyv(length);
  if(angle != NULL)
    free_dyv(angle);
  if(exp_time != NULL)
    free_dyv(exp_time);
  return(tracklets);
};

/* FieldProximity */
static char fieldproximity_main__doc__[] = "\
fieldproximity(fieldList, orbitList, thresh, method, \
                  fleaf, tleaf, split_all)\n \
\
    fieldList is a list of the form [(fieldID, RA, Dec, MJD, R), ] \
    orbitList is a list of the form [(objName, MJD, RA, Dec, mag), ] \
    thresh in degrees (default 0.0001) \
    method is the search method (default 0) \
    ... \
    REMEMBER: objName has to be a string!!!!!!!! \
    The return object is a dictionary of the form \
    {field_id: [object1_name, object2_name, ...]} \
Note: RA and Dec are assumed to be in decimal degrees.\
      fieldID is an integer, objName is a string.";
PyDictObject* wrap_fieldproximity_main(PyObject* self, 
                                       PyObject* args, 
                                       PyObject* kw) {
  /* Define the keyword names */
  static char *kwlist[] = {"fields",
                           "orbits",
                           "thresh",
                           "method",
                           "fleaf",
                           "tleaf",
                           "split_all",
                           NULL};
  /* Define all the vars and give optional keywords default values */
  PyObject* fieldList;
  PyObject* orbitList;
  double thresh = 0.0001;
  int method = 0;
  int fleaf = 10;
  int tleaf = 10;
  bool split_all = false;

  /* Support variables */
  PyDictObject* res;
    
    
  /* Parse args and keywords */
  if(!PyArg_ParseTupleAndKeywords(args, 
                                  kw, 
                                  "OO|diiib", 
                                  kwlist, 
                                  &fieldList,
                                  &orbitList,
                                  &thresh,
                                  &method,
                                  &fleaf,
                                  &tleaf,
                                  &split_all))
    return NULL;
  
  /* Invoke the main routine. */
  res = fieldproximity_main((PyListObject*)fieldList, (PyListObject*)orbitList, 
                            thresh, method, fleaf, tleaf, split_all);
  return(res);
};


/* OrbitProximity */
static char orbitproximity_main__doc__[] = "\
orbitproximity(knownOrbits, queryOrbits, q_thresh, e_thresh, i_thresh, \n \
               w_thresh, O_thresh, t_thresh, noise_pts) \n \
\
knownOrbits: [(q, e, i, node, arg_peri, time_peri, h_v, epoch, objName), ] \
queryOrbits: [(q, e, i, node, arg_peri, time_peri, h_v, epoch, objName), ] \
q in AU, e, i, node, arg_peri in decimal degrees, time_peri, epoch in MJD, \
h_v in magnitudes, objName is a string.\n \
The result is a dictionary of the form {queryObjName: [knownObjName, ...]}";
PyDictObject* wrap_orbitproximity_main(PyObject* self, 
                                       PyObject* args, 
                                       PyObject* kw) {
  /* Define the keyword names */
  static char *kwlist[] = {"knownOrbits",
                           "queryOrbits",
                           "q_thresh",
                           "e_thresh",
                           "i_thresh",
                           "w_thresh",
                           "O_thresh",
                           "t_thresh",
                           "noise_pts",
                           NULL};
  /* Define all the vars and give optional keywords default values */
  PyObject* knownOrbits;
  PyObject* queryOrbits;
  double q_thresh = 0.01;
  double e_thresh = 0.01;
  double i_thresh = 0.1;
  double w_thresh = 1.0;
  double O_thresh = 1.0;
  double t_thresh = 0.1;
  int noise_pts = 0;

  /* Support variables */
  PyDictObject* res;
  
    
  /* Parse args and keywords */
  if(!PyArg_ParseTupleAndKeywords(args, 
                                  kw, 
                                  "OO|ddddddi", 
                                  kwlist, 
                                  &knownOrbits,
                                  &queryOrbits,
                                  &q_thresh,
                                  &e_thresh,
                                  &i_thresh,
                                  &w_thresh,
                                  &O_thresh,
                                  &t_thresh,
                                  &noise_pts))
    return NULL;
  
  /* Invoke the main routine. */
  res = orbitproximity_main((PyListObject*)knownOrbits, 
                            (PyListObject*)queryOrbits, 
                            q_thresh,
                            e_thresh,
                            i_thresh,
                            w_thresh,
                            O_thresh,
                            t_thresh,
                            noise_pts);
  return(res);
};


/* DetectionProximity */
static char detectionproximity_main__doc__[] = "\
detectionproximity(dataDets, queryDets, d_thresh, b_thresh, t_thresh) \n \
\
dataDets: [(det_id, mjd, ra, dec, mag, obscode), ] \
queryDets: [(det_id, mjd, ra, dec, mag, obscode), ] \
The result is a dictionary of the form {queryDet: [dataDet, ...]}";
PyDictObject* wrap_detectionproximity_main(PyObject* self, 
                                           PyObject* args, 
                                           PyObject* kw) {
  /* Define the keyword names */
  static char *kwlist[] = {"dataDets",
                           "queryDets",
                           "d_thresh",
                           "b_thresh",
                           "t_thresh",
                           NULL};
  /* Define all the vars and give optional keywords default values */
  PyObject* dataDets;
  PyObject* queryDets;
  double d_thresh = 1.0;        /* degrees */
  double b_thresh = 1.0;        /* mag */
  double t_thresh = 1.0;        /* days */

  /* Support variables */
  PyDictObject* res;
  
    
  /* Parse args and keywords */
  if(!PyArg_ParseTupleAndKeywords(args, 
                                  kw, 
                                  "OO|ddd", 
                                  kwlist, 
                                  &dataDets,
                                  &queryDets,
                                  &d_thresh,
                                  &b_thresh,
                                  &t_thresh))
    return NULL;
  
  /* Invoke the main routine. */
  res = detectionproximity_main((PyListObject*)dataDets, 
                                (PyListObject*)queryDets, 
                                d_thresh,
                                b_thresh,
                                t_thresh);
  return(res);
};



/* 
   List the methods exposed by the module (and their doc strings).
*/
static PyMethodDef auton_methods[] = {
  {"linktracklets", 
   (PyCFunction)wrap_linktracklets_main, 
   METH_KEYWORDS, 
   linktracklets_main__doc__},
  {"findtracklets", 
   (PyCFunction)wrap_findtracklets_main, 
   METH_KEYWORDS, 
   findtracklets_main__doc__},
  {"fieldproximity", 
   (PyCFunction)wrap_fieldproximity_main, 
   METH_KEYWORDS, 
   fieldproximity_main__doc__},
  {"orbitproximity", 
   (PyCFunction)wrap_orbitproximity_main, 
   METH_KEYWORDS, 
   orbitproximity_main__doc__},
  {"detectionproximity", 
   (PyCFunction)wrap_detectionproximity_main, 
   METH_KEYWORDS, 
   detectionproximity_main__doc__},
  {NULL, NULL}                                            // Sentinel element.
};


/*
  Module doc string
*/
static char auton_module_documentation[] = 
  "Auton Module Docs go here.";

/*
  Define the module and its methods.
*/
void initauton(void) {
  PyObject* m;
  
  /* Create the module and add the functions */
  m = Py_InitModule4("auton", 
                     auton_methods,
                     auton_module_documentation,
                     (PyObject *)NULL, 
                     PYTHON_API_VERSION);

  /* Check for errors */
  if(PyErr_Occurred())
    Py_FatalError("can't initialize auton module.");
}
