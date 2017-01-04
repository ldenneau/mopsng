/*
   File:        geometry.c
   Author:      Jeff Schneider
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

#include "geometry.h"
#include "eq_solvers.h"

/* The first three cases below are obvious.  The last one is not.
   I took it from 
   http://mathworld.wolfram.com/Circle-CircleIntersection.html
*/
double circle_intersection_area(double x1, double y1, double r1,
				double x2, double y2, double r2)
{
  double area = 0.0;
  double d = sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));

  /* they don't intersect */
  if (d >= (r1+r2)) area = 0.0;
  /* circle 2 encloses circle 1 */
  else if (d <= (r2 - r1)) area = PI*r1*r1;
  /* circle 1 encloses circle 2 */
  else if (d <= (r1 - r2)) area = PI*r2*r2;
  /* there is a non-trivial intersection */
  else
  {
    double term1 = r1 * r1 * acos((d*d + r1*r1 - r2*r2)/(2*d*r1));
    double term2 = r2 * r2 * acos((d*d + r2*r2 - r1*r1)/(2*d*r2));
    double term3 = -0.5 * sqrt((-d+r1+r2)*(d+r1-r2)*(d-r1+r2)*(d+r1+r2));
    area = term1 + term2 + term3;
  }
  return area;
}


/* --- Added: 11/13/2003 by Jeremy Kubica ------------------------- */
/* --- Functions for computing the distance between a point and a   */
/*     quadratic curve.                                             */

/* Computes the (weighted) euclidean distance from a point
   to a specific point on a quadratic curve:

   y = a*x^2 + b*x + c

   where the square of the distance is computed as

   ----
   \      [    pt_d - y_d    ]^2
    >     [ ---------------- ]
   /      [     metric_d     ]
   ---- d

   If metric==NULL then metric_d = 1 for all d.
*/
double dist_pt_on_quad_curve_to_point(dyv* a, dyv* b, dyv* c, double x,
                                      dyv* pt, dyv* metric) {
  double dist = 0.0;
  double diff, y, w_sq;
  int D = dyv_size(a);
  int d;

  for(d=0;d<D;d++) {
    w_sq = 1.0;
    if(metric) { w_sq = dyv_ref(metric,d) * dyv_ref(metric,d); }

    y = dyv_ref(a,d)*x*x + dyv_ref(b,d)*x + dyv_ref(c,d);
    diff = (y - dyv_ref(pt,d));

    dist += (diff*diff)/w_sq;
  }

  return sqrt(dist);
}


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
double dist_quad_curve_to_point(dyv* a, dyv* b, dyv* c, dyv* pt, dyv* metric) {
  double A = 0.0;
  double B = 0.0;
  double C = 0.0;
  double D = 0.0;
  double at, bt, ct, ptt;
  double w_sq;
  double best;
  double temp;
  int DSize = dyv_size(a);
  int d;
  int res;
  double t[3];

  for(d=0;d<DSize;d++) {
    w_sq = 1.0;
    if(metric) { w_sq = dyv_ref(metric,d) * dyv_ref(metric,d); }

    at = dyv_ref(a,d);
    bt = dyv_ref(b,d);
    ct = dyv_ref(c,d);
    ptt= dyv_ref(pt,d);

    A += (at*at)/w_sq;
    B += (at*bt)/w_sq;
    C += (2.0*at*ptt-2.0*at*ct-bt*bt)/w_sq;
    D += (ptt*bt-bt*ct)/w_sq;
  }
  A *= -2.0;
  B *= -3.0;

  res = cubic_eq_solve(A,B,C,D,0.0,t);
  best = dist_pt_on_quad_curve_to_point(a,b,c,t[0],pt,metric);

  if(res > 1) {
    temp = dist_pt_on_quad_curve_to_point(a,b,c,t[1],pt,metric);
    if(temp < best) { best = temp; }
  }

  if(res > 2) {
    temp = dist_pt_on_quad_curve_to_point(a,b,c,t[2],pt,metric);
    if(temp < best) { best = temp; }
  }

  return best;
}

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
                                        dyv* pt, dyv* metric) {
  double A = 0.0;
  double B = 0.0;
  double C = 0.0;
  double D = 0.0;
  double at, bt, ct, ptt;
  double w_sq;
  double best;
  double temp;
  int DSize = dyv_size(a);
  int d, i;
  int res;
  double t[3];

  for(d=0;d<DSize;d++) {
    w_sq = 1.0;
    if(metric) { w_sq = dyv_ref(metric,d) * dyv_ref(metric,d); }

    at = dyv_ref(a,d);
    bt = dyv_ref(b,d);
    ct = dyv_ref(c,d);
    ptt= dyv_ref(pt,d);

    A += (at*at)/w_sq;
    B += (at*bt)/w_sq;
    C += (2.0*at*ptt-2.0*at*ct-bt*bt)/w_sq;
    D += (ptt*bt-bt*ct)/w_sq;
  }
  A *= -2.0;
  B *= -3.0;

  best = dist_pt_on_quad_curve_to_point(a,b,c,xmin,pt,metric);
  temp = dist_pt_on_quad_curve_to_point(a,b,c,xmax,pt,metric);
  if(temp < best) { best = temp; }

  res = cubic_eq_solve(A,B,C,D,0.0,t);
  for(i=0;i<res;i++) {
    if((t[i] < xmax)&&(t[i] > xmin)) {
      temp = dist_pt_on_quad_curve_to_point(a,b,c,t[i],pt,metric);
      if(temp < best) { best = temp; }
    }
  }

  return best;
}



/* ----------------------------------------------------------------------- */
/* --- Functions for doing standard rotations to 3 dimensional vectors --- */
/* ----------------------------------------------------------------------- */


/* Create the 3D rotation matrix for rotation about the X axis */
dym* mk_3d_rotation_mat_X(double theta) {
  dym* res = mk_zero_dym(3,3);

  dym_set(res,0,0,1.0);
  dym_set(res,1,1,cos(theta));
  dym_set(res,1,2,sin(theta));
  dym_set(res,2,1,-sin(theta));
  dym_set(res,2,2,cos(theta));

  return res;
}


/* Perform a rotation of V around the X axis and store the result in res */
void rotate_X_3d(dyv* V, double theta, dyv* res) {
  dyv_set(res,0,dyv_ref(V,0));
  dyv_set(res,1,dyv_ref(V,1)*cos(theta)+dyv_ref(V,2)*sin(theta));
  dyv_set(res,2,-dyv_ref(V,1)*sin(theta)+dyv_ref(V,2)*cos(theta));
}


/* Perform a rotation of V around the X axis and return the result */
dyv* mk_3d_rotation_X(dyv* V, double theta) {
  dyv* res = mk_dyv(3);

  rotate_X_3d(V,theta,res);

  return res;
}


/* Create the 3D rotation matrix for rotation about the Y axis */
dym* mk_3d_rotation_mat_Y(double theta) {
  dym* res = mk_zero_dym(3,3);

  dym_set(res,1,1,1.0);
  dym_set(res,0,0,cos(theta));
  dym_set(res,2,0,sin(theta));
  dym_set(res,0,2,-sin(theta));
  dym_set(res,2,2,cos(theta));

  return res;
}


/* Perform a rotation of V around the Y axis and store the result in res */
void rotate_Y_3d(dyv* V, double theta, dyv* res) {
  dyv_set(res,0,dyv_ref(V,0)*cos(theta)-dyv_ref(V,2)*sin(theta));
  dyv_set(res,1,dyv_ref(V,1));
  dyv_set(res,2,dyv_ref(V,0)*sin(theta)+dyv_ref(V,2)*cos(theta));
}


/* Perform a rotation of V around the Y axis and return the result */
dyv* mk_3d_rotation_Y(dyv* V, double theta) {
  dyv* res = mk_dyv(3);

  rotate_Y_3d(V,theta,res);

  return res;
}


/* Create the 3D rotation matrix for rotation about the Z axis */
dym* mk_3d_rotation_mat_Z(double theta) {
  dym* res = mk_zero_dym(3,3);

  dym_set(res,2,2,1.0);
  dym_set(res,0,0,cos(theta));
  dym_set(res,0,1,sin(theta));
  dym_set(res,1,0,-sin(theta));
  dym_set(res,1,1,cos(theta));

  return res;
}


/* Perform a rotation of V around the Z axis and store the result in res */
void rotate_Z_3d(dyv* V, double theta, dyv* res) {
  dyv_set(res,0,dyv_ref(V,0)*cos(theta)+dyv_ref(V,1)*sin(theta));
  dyv_set(res,1,-dyv_ref(V,0)*sin(theta)+dyv_ref(V,1)*cos(theta));
  dyv_set(res,2,dyv_ref(V,2));
}


/* Perform a rotation of V around the Z axis and return the result */
dyv* mk_3d_rotation_Z(dyv* V, double theta) {
  dyv* res = mk_dyv(3);

  rotate_Z_3d(V,theta,res);

  return res;
}
