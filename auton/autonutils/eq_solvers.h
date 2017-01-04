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

#ifndef EQ_SOLVERS_H
#define EQ_SOLVERS_H

#include <math.h>
#include <stdlib.h>

#ifndef PI
#define PI 3.14159265358979
#endif

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
                     double y, double *x);



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
                  double y, double *x);



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
                   double y, double *x);


#endif
