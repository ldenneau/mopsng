/*
File:        MHT.h
Author:      J. Kubica
Created:     Tue June 15, 2004
Description: Header for the C adaptation of MHT

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

#ifndef MHT_H
#define MHT_H

#include "track.h"

track_array* mk_order_tracks_by_trust(track_array* arr, simple_obs_array* obs);

ivec* mk_MHT_overlap_query(ivec_array* seen, ivec* pts);

ivec* mk_MHT_subset_query(ivec_array* seen, ivec* pts);

void MHT_add_to_overlap_array(ivec_array* seen, ivec* pts, int index);

track_array* mk_MHT_remove_overlaps(track_array* old, simple_obs_array* obs,
                                    bool allow_conflicts,
                                    double min_percentage_overlap);

track_array* mk_MHT_remove_subsets(track_array* old, simple_obs_array* obs);


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
			    bool bwpass);

#endif
