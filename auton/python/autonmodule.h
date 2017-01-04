/*
  Definitions for auton module wrapper
*/
#include "am_time.h"
#include "track.h"
#include "sb_graph.h"
#include "MHT.h"
#include "rdt_tree.h"
#include "linker.h"
#include "plates.h"
#import "pw_linear.h"
#include "occ_tree_funs.h"
#include "orbit.h"
#include "orbprox.h"
#include "tracklet_mht.h"
#include "detectprox.h"

#include <Python.h>



PyListObject* dump_tracks(simple_obs_array*, track_array*);
void incr_track_name2(char*);
void initauton(void);
PyListObject* linktracklets_main(ivec*, ivec*, simple_obs_array*, double, 
                                 double, double, double, double,  double,
                                 double, double, double, double,  double,
                                 double, double, int, int,  int, int, int,
                                 bool, bool, bool, bool,  bool,
                                 double, double);
PyDictObject* fieldproximity_main(PyListObject*, PyListObject*, double, int, 
                                  int, int, bool);
ivec* mk_count_tracks_iv_ind(ivec*);
pw_linear_array* mk_load_RA_DEC_pw_linear_array_from_list(PyListObject*, 
                                                          namer*,  bool);
rd_plate_array* mk_load_rd_plate_array_from_list(PyListObject*, bool);
orbit_array* mk_orbit_array_from_list(PyListObject*, string_array**);
PyDictObject* orbitproximity_main(PyListObject*,  PyListObject*, double,
                                  double, double, double, double, double, int);
simple_obs_array* mk_simple_obs_array_from_PANSTARRS_list(PyObject*, int, 
                                                          ivec**,  ivec**,
                                                          dyv**,  dyv**, dyv**);
PyDictObject* wrap_fieldproximity_main(PyObject*, PyObject*, PyObject*);
PyListObject* wrap_findtracklets_main(PyObject*, PyObject*, PyObject*);
PyListObject* wrap_linktracklets_main(PyObject*, PyObject*, PyObject*);
PyDictObject* wrap_orbitproximity_main(PyObject*, PyObject*, PyObject*);
PyDictObject* dump_fp_results(rd_plate_array*, namer*, ivec_array*);
PyDictObject* detectionproximity_main(PyListObject*, 
                                      PyListObject*, 
                                      double,
                                      double,
                                      double);
PyDictObject* wrap_detectionproximity_main(PyObject*, 
                                           PyObject*, 
                                           PyObject*);
