/*
   File:        plate_tree.h
   Author(s):   Kubica
   Created:     Mon June 8 2004
   Description: Data struture for a tree structure to hold RA/DEC points 
                with velocities.  For use in occur/attribution queries.
   Copyright (c) Carnegie Mellon University

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

#ifndef PLATE_TREE_H
#define PLATE_TREE_H

#include "plates.h"

typedef struct plate_tree {
  int num_points;

  struct plate_tree* left;
  struct plate_tree* right;

  double t_hi;     /* Time bounds */
  double t_lo;

  double ra;
  double dec;
  double radius;   /* RADIUS IN RADIANS */

  ivec *plates;
} plate_tree;


/* --------------------------------------------------------------------- */
/* --- Tree Memory Functions ------------------------------------------- */
/* --------------------------------------------------------------------- */

plate_tree* mk_empty_plate_tree(void);

/* wt, wr, wd are the relative weights of time, RA, and DEC  */
/* a high value means pay a lot of attention a value of zero */
/* means that attribute will be ignored for splitting.       */
plate_tree* mk_plate_tree(rd_plate_array* arr, double wt,
                          double wr, double wd, int max_leaf_pts);

void free_plate_tree(plate_tree* old);


/* --------------------------------------------------------------------- */
/* --- Getter/Setter Functions ----------------------------------------- */
/* --------------------------------------------------------------------- */

int safe_plate_tree_N(plate_tree* tr);

int safe_plate_tree_num_points(plate_tree* tr);

bool safe_plate_tree_is_leaf(plate_tree* tr);

ivec* safe_plate_tree_rd_plates(plate_tree* tr);

plate_tree* safe_plate_tree_right_child(plate_tree* tr);
plate_tree* safe_plate_tree_left_child(plate_tree* tr);

double safe_plate_tree_RA(plate_tree* p);
double safe_plate_tree_DEC(plate_tree* p);
double safe_plate_tree_radius(plate_tree* p);

double safe_plate_tree_lo_time(plate_tree* p);
double safe_plate_tree_hi_time(plate_tree* p);

/* Allow a few speedups */
#ifdef AMFAST

#define plate_tree_N(X)               (X->num_points)
#define plate_tree_num_points(X)      (X->num_points)
#define plate_tree_is_leaf(X)         (X->left == NULL)
#define plate_tree_rd_plates(X)       (X->plates)
#define plate_tree_right_child(X)     (X->right)
#define plate_tree_left_child(X)      (X->left)

#define plate_tree_lo_time(X)         (X->t_lo)
#define plate_tree_hi_time(X)         (X->t_hi)
#define plate_tree_RA(X)              (X->ra)
#define plate_tree_DEC(X)             (X->dec)
#define plate_tree_radius(X)          (X->radius)

#else

#define plate_tree_N(X)               (safe_plate_tree_num_points(X))
#define plate_tree_num_points(X)      (safe_plate_tree_num_points(X))
#define plate_tree_is_leaf(X)         (safe_plate_tree_is_leaf(X))
#define plate_tree_rd_plates(X)       (safe_plate_tree_rd_plates(X))
#define plate_tree_right_child(X)     (safe_plate_tree_right_child(X))
#define plate_tree_left_child(X)      (safe_plate_tree_left_child(X))

#define plate_tree_lo_time(X)         (safe_plate_tree_lo_time(X))
#define plate_tree_hi_time(X)         (safe_plate_tree_hi_time(X))

#define plate_tree_RA(X)              (safe_plate_tree_RA(X))
#define plate_tree_DEC(X)             (safe_plate_tree_DEC(X))
#define plate_tree_radius(X)          (safe_plate_tree_radius(X))

#endif


/* --- Simple I/O Functions ------------------------------------ */

void fprintf_plate_tree_pts(FILE* f, plate_tree* tr);

void fprintf_plate_tree(FILE* f, plate_tree* tr);

void fprintf_plate_tree_node(FILE* f, char* pre, plate_tree* tr, char* post);

#endif
