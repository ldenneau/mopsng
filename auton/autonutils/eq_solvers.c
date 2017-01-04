/*
   File:        eq_solvers.h
   Author:      J. Kubica
   Created:     Wed Nov 12 16:12:06 EDT 2003
   Description: Useful functions to solve simple equations
                (quadratic,cubic)

   Copyright 2003, The Auton Lab, CMU

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

#include "eq_solvers.h"


/* Solve the quadratic equation:

     y = a*x + b.

   Takes:
     a, b = The coefficients of the linear
            equation.
     y    = The result of the quadratic.
     x*   = An array of length >= 1 in which
            the solution will be dumped.
   Returns:
     The number of REAL roots (0-1).  Fills the
     array with the solution (i.e. x[0] is the
     first root).  x[i] is UNDEFINED for
     i >= N where N is the integer returned.
*/
int linear_eq_solver(double a, double b,
		     double y, double *x) {
  int res = 0;

  /* Check for division by zero */
  if( (y-b) < (fabs(a) * 1e80) ) {
    res = 1;
    x[0] = (y-b)/a;
  }

  return res;
}


/* Solve the quadratic equation:

     y = a*x^2 + b*x + c. 

   Takes:
     a, b, c = The coefficients of the quadratic
               equation.
     y       = The result of the quadratic.
     x*      = An array of length >= 2 in which
               the solution will be dumped.
   Returns:
     The number of REAL roots (0-2).  Fills the
     array with the solution (i.e. x[0] is the
     first root).  x[i] is UNDEFINED for
     i >= N where N is the integer returned.

     NOTE: Checks for divide by zero using
           1e-80 as a threshold.  That is
           if |a| < 1e-80 then equation is
           treated as linear.
*/     
int quad_eq_solve(double a, double b, double c, 
                  double y, double *x) {
  double inner;
  int res = 0;

  /* First check if we are actually trying to just
     solve a linear equations (also prevents divide 
     by zero). */
  if(fabs(a)*1e80 < 1.0) {
    res = linear_eq_solver(b,c,y,x);
  } else {

    c = c - y;
    inner = b * b - 4.0 * a * c;

    if(inner >= 0.0) {
      if(inner == 0.0) {
        res = 1;
        x[0] = -b / (2.0 * a);
      } else {
        res = 2;
        x[0] = (-b - sqrt(inner)) / (2.0 * a);
        x[1] = (-b + sqrt(inner)) / (2.0 * a);
      }
    }
  }

  return res;
}



/* Solve the cubic equation:

     y = a*x^3 + b*x^2 + c*x + d.

   Takes:
     a, b, c, d = The coefficients of the cubic
                  equation.
     y          = The result of the quadratic.
     x*         = An array of length >= 3 in which
                  the solution will be dumped.
   Returns:
     The number of REAL roots (0-3).  Fills the
     array with the solution (i.e. x[0] is the
     first root).  x[i] is UNDEFINED for
     i >= N where N is the integer returned.


     NOTE: Checks for divide by zero using
           1e-80 as a threshold.  That is
           if |a| < 1e-80 then equation is
           treated like a quadratic.
*/
int cubic_eq_solve(double a, double b, double c, double d,
                   double y, double *x) {
  double an, bn, cn;
  double A,B,Q,R;
  double theta;
  double sgn;
  int res = 0;

  /* First check if we are actually trying to just
     solve a quadratic equation */
  if(fabs(a)*1e80 < 1.0) {  
    res = quad_eq_solve(b,c,d,y,x);
  } else {

    /* Convert the problem into x^3 + an*x^2 + bn*x + cn = 0. */
    an = b/a;
    bn = c/a;
    cn = (d - y)/a;

    Q = (an*an - 3.0*bn) / 9.0;
    R = (2.0*an*an*an - 9.0 * an * bn + 27.0 * cn) / 54.0;

    if( (R*R) < (Q*Q*Q) ) {
      res = 3;
      theta = acos(R/sqrt(Q*Q*Q));
      x[0] = -2.0 * sqrt(Q) * cos(theta/3.0) - an/3.0;
      x[1] = -2.0 * sqrt(Q) * cos((theta+2.0*PI)/3.0) - an/3.0;
      x[2] = -2.0 * sqrt(Q) * cos((theta-2.0*PI)/3.0) - an/3.0;
    } else {
      res = 1;
      sgn = 0.0;

      if(R > 0.0) { sgn = 1.0; }
      if(R < 0.0) { sgn = -1.0; }

      A = -sgn * pow( (fabs(R) + sqrt(R*R-Q*Q*Q)), 1.0/3.0);
      B = 0.0;
      if(fabs(A) > 1e-50) { B = Q/A; }
      x[0] = A + B - an/3.0;
    }
  }
  
  return res;
}
