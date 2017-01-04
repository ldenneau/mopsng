/*
File:        MHT.h
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
along with this program.  If not, see <http://www.lsstcorp.org/LegalNotices/>.
*/

#ifndef TRACKLET_MHT_H
#define TRACKLET_MHT_H

#include "track.h"
#include "rdt_tree.h"

track_array* mk_tracklets_MHT(simple_obs_array* arr, double minv, double maxv,
                              double thresh, double maxt, int min_size,
                              bool remove_subsets,
                              dyv* angle, dyv* length, dyv* exp_time,
                              double athresh, double maxLerr, double etime,
                              int max_obs, bool greedy, bool pht);


/* --- Functions for removing overlaps ----------------------------- */

track_array* mk_tracklet_remove_subsets(track_array* old,
                                        simple_obs_array* obs);

/* Given an ivec array of observation->tracks it is in  */
/* AND the observations of a new track, determine which */
/* tracks the new track is a subset of.                 */
ivec* mk_tracklet_subset_query(ivec_array* seen, ivec* pts);

/* Given an ivec array of observation->tracks it is in  */
/* AND a new track, add the new track to the array.     */
void tracklet_add_to_overlap_array(ivec_array* seen, ivec* pts, int index);

#endif
