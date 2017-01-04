/*
  File:        occ_tree_funs.h
  Author:      J. Kubica
  Description: Functions for determining whether plates intersect with
               piecewise linear tracks.

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

#ifndef OBSOCCUR_TREES_H
#define OBSOCCUR_TREES_H

#include "plate_tree.h"
#include "pw_tree.h"

/* These functions test plate/pw_linear intersections */
/* Each returns an ivec_array such that each ivec     */
/* corresponds to a plate and is a list of tracks     */
/* that intersect that plate.                         */

bool pw_linear_hit_rd_plate(pw_linear* T, rd_plate* P, double thresh);

/* ----------------------------------------------------------------- */
/* --- Evaluation Functions ---------------------------------------- */
/* ----------------------------------------------------------------- */

/* For each ivec in A what is the hamming distance */
/* to its corresponding ivec in B.  Returns sum.   */
double calculate_errors(ivec_array* A, ivec_array* B);


/* ----------------------------------------------------------------- */
/* --- Exhaustive Search ------------------------------------------- */
/* ----------------------------------------------------------------- */

ivec_array* mk_exhaustive(pw_linear_array* tarr, rd_plate_array* parr,
                          double thresh);


/* ----------------------------------------------------------------- */
/* --- PTree Search ------------------------------------------------ */
/* ----------------------------------------------------------------- */

ivec_array* mk_plate_tree_int_search(plate_tree* tr, rd_plate_array* parr,
                                     pw_linear_array* tarr, double thresh);


/* ----------------------------------------------------------------- */
/* --- MTTree Search ----------------------------------------------- */
/* ----------------------------------------------------------------- */

ivec_array* mk_pw_tree_search(pw_tree* tr, pw_linear_array* tarr, 
                              rd_plate_array* parr, double thresh);


/* ----------------------------------------------------------------- */
/* --- MTTree/PTree Search ----------------------------------------- */
/* ----------------------------------------------------------------- */

ivec_array* mk_dual_tree_search(pw_tree* ttr, pw_linear_array* tarr,
                                plate_tree* ptr, rd_plate_array* parr,
                                double thresh);

#endif
