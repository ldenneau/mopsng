/*
File:        neos_header.h
Author:      J. Kubica
Created:     Mon Sept 29 13:11:26 EDT 2003
Description: #DEFINES that will be used throughout the Auton astro code.

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

#ifndef NEOS_HEADER_H
#define NEOS_HEADER_H

#include "utils.h"

#define ARCSEC         0.000277777777777777777777778   /* 1/3600 */

#ifndef PI
#define PI             3.141592653589793055255613581
#endif
#define PI2            (2.0*PI)

#define JK_EPS         1e-20

#define frac(x)         ((x) - floor(x))
#define modulo(x,y)     ((y) * frac((x)/(y)))

#define DEG_TO_RAD   0.01745329251994
#define RAD_TO_DEG  57.29577951308232
#define RAD_TO_ARC  (3600.0*180.0/PI)

/* Some reference times... */
#define JD_J2000     2451545.0      /* JD of Epoch J2000.0 */
#define JD_J1950     2433282.5      /* JD of Epoch J1950.0 */
#define JD_J1900     2415020.0      /* JD of Epoch J1900.0 */
#define JD_B1950     2433282.423    /* JD of Epoch B1950.0 */
#define MJD_J2000      51544.5      /* MJD of Epoch J2000.0 */
#define MJD_J1950      33282.0      /* MJD of Epoch J1950.0 */
#define MJD_J1900      15020.0      /* MJD of Epoch J1900.0 */
#define MJD_B1950      33281.923    /* MJD of Epoch B1950.0 */

#define JD_TO_MJD(jd)     ((jd)  - 2400000.5)
#define JD_TO_MJC(jd)     (((jd) - JD_J2000)/36525.0)
#define MJD_TO_JD(mjd)    ((mjd) + 2400000.5)
#define MJD_TO_MJC(mjd)   (((mjd) - MJD_J2000)/36525.0)
#define MJC_TO_MJD(mjc)   (((mjc) * 36525.0)+MJD_J2000)

#define JK_MID(min,max)    (((min) + (max))/2.0)
#define JK_RAD(min,max)    ((max) - JK_MID(min,max))
#define JK_SIMPLE_MIN(a,b) (((a) < (b)) ? (a) : (b))
#define JK_SIMPLE_MAX(a,b) (((a) > (b)) ? (a) : (b))

#define JK_FLOOR(a)        ((double)((int)(a)))
#define JK_FRAC(a)         ((a) - JK_FLOOR(a))
#define JK_MOD(a,base)     ((base)*JK_FRAC((a)/(base)))

#define C_LIGHT      173.14             /* Speed of light AU/Day */
#define AU_TO_KM     149597870.0
#define KGAUSS       0.01720209895
#define GM_SUN       (KGAUSS*KGAUSS)
#define LIGHTTIME    0.005775518304

#endif
