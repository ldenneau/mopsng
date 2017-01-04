/*
   File:        geometry.h
   Author:      Jeff Schneider, Jeremy Kubica
   Created:     Sat Mar 22 15:41:57 EST 2003
   Description: miscellanous geometric computations

   Copyright 2003, Carnegie Mellon University
   
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

#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "amut.h"
#include "amdmex.h"

double circle_intersection_area(double x1, double y1, double r1,
				double x2, double y2, double r2);


/* --- Added: 11/13/2003 by Jeremy Kubica ------------------------- */
/* --- Functions for computing the distance between a point and a   */
/*     quadratic curve. */

/* Computes the (weighted) euclidean distance from a point
   a quadratic curve:

   y = a*x^2 + b*x + c

   where the square of the distance is computed as

   ----
   \      [    pt_d - y_d    ]^2
    >     [ ---------------- ]
   /      [     metric_d     ]
   ---- d

   If metric==NULL then metric_d = 1 for all d.
*/
double dist_quad_curve_to_point(dyv* a, dyv* b, dyv* c, dyv* pt, dyv* metric);


/* Computes the (weighted) euclidean distance from a point
   a bounded quadratic curve:

   y = a*x^2 + b*x + c   where xmin <= x <= xmax

   where the square of the distance is computed as

   ----
   \      [    pt_d - y_d    ]^2
    >     [ ---------------- ]
   /      [     metric_d     ]
   ---- d

   If metric==NULL then metric_d = 1 for all d.
*/
double dist_bounded_quad_curve_to_point(dyv* a, dyv* b, dyv* c,
                                        double xmin, double xmax,
                                        dyv* pt, dyv* metric);



/* ----------------------------------------------------------------------- */
/* --- Functions for doing standard rotations to 3 dimensional vectors --- */
/* ----------------------------------------------------------------------- */

/* Create the matrix for 3D rotation about the X axis */
/*   Note: theta is angle in radians                  */
dym* mk_3d_rotation_mat_X(double theta);

/* Create the matrix for 3D rotation about the Y axis */
/*   Note: theta is angle in radians                  */
dym* mk_3d_rotation_mat_Y(double theta);

/* Create the matrix for 3D rotation about the Z axis */
/*   Note: theta is angle in radians                  */
dym* mk_3d_rotation_mat_Z(double theta);

/* Perform a rotation of V around the X axis and store the result in res */
/*   Note: theta is angle in radians                  */
void rotate_X_3d(dyv* V, double theta, dyv* res);

/* Perform a rotation of V around the Y axis and store the result in res */
/*   Note: theta is angle in radians                  */
void rotate_Y_3d(dyv* V, double theta, dyv* res);

/* Perform a rotation of V around the Z axis and store the result in res */
/*   Note: theta is angle in radians                  */
void rotate_Z_3d(dyv* V, double theta, dyv* res);

/* Perform a rotation of V around the X axis and return the result */
/*   Note: theta is angle in radians                  */
dyv* mk_3d_rotation_X(dyv* V, double theta);

/* Perform a rotation of V around the Y axis and return the result */
/*   Note: theta is angle in radians                  */
dyv* mk_3d_rotation_Y(dyv* V, double theta);

/* Perform a rotation of V around the Z axis and return the result */
/*   Note: theta is angle in radians                  */
dyv* mk_3d_rotation_Z(dyv* V, double theta);


#endif /* #ifdef GAUSSIAN_H */
