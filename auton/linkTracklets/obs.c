/*
File:        obs.c
Author:      J. Kubica
Created:     Fri Sept 19 10:35:50 EDT 2003
Description: A simple data structure representing astronomical observations
             and corresponding functions for this data structure.

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

#include "obs.h"
#include "geometry.h"

#define c_to_i(X)     (X - '0')
#define cc_to_i(X,Y)  (c_to_i(X)*10 + c_to_i(Y))

int NUM_FOR_QUAD = 4;

double time_from_MPC_string(char* str) {
  char* temp;
  int month;
  int year;
  double day;
  double time;

  my_assert((int)strlen(str) >= 50);

  year  = cc_to_i(str[15],str[16])*100 + cc_to_i(str[17],str[18]);
  month = cc_to_i(str[20],str[21]);
  temp  = mk_substring(str,23,31);
  day   = atof(temp);

  time  = frac_date_to_MJD(year, month, day);

  free_string(temp);
  
  return time;
}

void bound_RA_DEC(double* RA, double* DEC) {
  if(DEC[0] < -90.0) {
    DEC[0] = -180.0 - DEC[0];
    RA[0] += 12.0;
  }
  if(DEC[0] > 90.0) {
    DEC[0] = 180.0 - DEC[0];
    RA[0] += 12.0;
  }

  if(RA[0] < 0.0)  { RA[0] += 24.0; }
  if(RA[0] > 24.0) { RA[0] -= 24.0; }
}


/* -----------------------------------------------------------------------*/
/* -------- Simple Observations ------------------------------------------*/
/* -----------------------------------------------------------------------*/

/* --- Memory functions for the simple observations ---------------------------- */

simple_obs* mk_simple_obs_simplest(int id, double time, double RA, double DEC, 
                                   double brightness) {
  return mk_simple_obs_time(id, time, RA, DEC, brightness, 'V', 500, NULL);
}


simple_obs* mk_simple_obs_simple(int id, int year, int month, double day, 
                                 double RA, double DEC, double brightness) {
  return mk_simple_obs(id, year, month, day, RA, DEC, brightness, 'V', 500,
                       NULL);
}


/* Use 'V" as the default type
   Use NULL for obs_code and id_str if unknown */
simple_obs* mk_simple_obs(int id, int year, int month, double day,
                          double RA, double DEC, double brightness,
                          char type, int obs_code, char* id_str) {
  simple_obs* X;
  double time;

  time = frac_date_to_MJD(year, month, day);
  X = mk_simple_obs_time(id, time, RA, DEC, brightness, 
                         type, obs_code, id_str);

  return X;
}


simple_obs* mk_simple_obs_time(int id, double time, double RA, double DEC,
                               double brightness, char type, int obs_code,
                               char* id_str) {
  simple_obs* X = AM_MALLOC(simple_obs);
  int i;

  X->time       = time;
  X->RA         = RA;
  X->DEC        = DEC;
  X->id         = id;
  X->brightness = (float)brightness;
  X->type       = type;
  X->obs_code   = obs_code;

  /* Copy the string padding it with spaces if required to */
  /* ensure that it is at least 8 characters.              */
  if(id_str != NULL) {
    if(strlen(id_str) >= 8) {
      X->id_str = mk_copy_string(id_str);
    } else {
      X->id_str = mk_copy_string("        ");
      for(i=0;i<strlen(id_str);i++) { 
        X->id_str[i] = id_str[i]; 
      }
    }
  } else {
    X->id_str = mk_copy_string("0FAKSE00");
  }

  return X;
}


simple_obs* mk_range_random_simple_obs(double time, int id,
                                       double RA_min, double RA_max,
                                       double DEC_min, double DEC_max,
                                       double bright_min, double bright_max) {
  simple_obs* X = AM_MALLOC(simple_obs);

  X->time       = time;
  X->RA         = range_random(RA_min,RA_max);
  X->DEC        = range_random(DEC_min,DEC_max);
  X->id         = id;
  X->brightness = (float)range_random(bright_min,bright_max);
  X->obs_code   = 500;
  X->id_str     = mk_copy_string("RANDOM  ");
  X->type        = 'V';

  return X;
}


simple_obs* mk_random_simple_obs(double time, int id) {
  return mk_range_random_simple_obs(time,id,0.0,24.0,-90.0,90.0,20.0,30.0);
}


simple_obs* mk_simple_obs_from_MPC_string(char* str, int id) {
  simple_obs* X;
  char* temp;
  char* id_s;
  char type;
  int month;
  int year;
  int OC;
  double day;
  double RA;
  double DEC;
  double brightness;

  my_assert((int)strlen(str) >= 79);

  id_s  = mk_substring(str,5,12); 

  /* extract the date information */
  year  = cc_to_i(str[15],str[16])*100 + cc_to_i(str[17],str[18]);
  month = cc_to_i(str[20],str[21]);
  temp = mk_substring(str,23,31);
  day = atof(temp);
  free_string(temp);

  /* extract the RA */
  RA   = (double)(cc_to_i(str[32],str[33]));
  RA  += ((double)(cc_to_i(str[35],str[36]))) / 60.0;
  temp = mk_substring(str,38,44);
  RA += atof(temp)/3600.0;
  free_string(temp);

  /* extract the DEC */
  DEC   = (double)(cc_to_i(str[45],str[46]));
  DEC  += ((double)(cc_to_i(str[48],str[49]))) / 60.0;
  temp = mk_substring(str,51,56);
  DEC += atof(temp)/3600.0;
  free_string(temp);
  if(str[44] == '-') {
    DEC *= -1.0;
  }

  /* extract the brightness */
  temp = mk_substring(str,65,69);
  brightness = atof(temp);
  free_string(temp);

  /* extract the observatory code */
  temp = mk_substring(str,77,80);
  OC   = atoi(temp);
  free_string(temp);

  type = str[70];

  X = mk_simple_obs(id,year,month,day,RA,DEC,brightness,type,OC,id_s);
  free_string(id_s);

  return X;
}


/* Makes an observation from a string with three numbers */
/* time, ra, dec                                         */
simple_obs* mk_simple_obs_from_three_string(char* str, int id) {
  dyv* nums;
  simple_obs* X = NULL;

  nums = mk_dyv_from_string(str,NULL);

  if(dyv_size(nums) == 3) {
    X = mk_simple_obs_simplest(id, dyv_ref(nums,0), dyv_ref(nums,1),
                               dyv_ref(nums,2),0.0);
  }

  free_dyv(nums);

  return X;
}


simple_obs* mk_copy_simple_obs(simple_obs* old) {
  simple_obs* X = AM_MALLOC(simple_obs);

  X->RA         = old->RA;
  X->DEC        = old->DEC;
  X->time       = old->time;
  X->id         = old->id;
  X->type       = old->type;
  X->brightness = old->brightness;
  X->obs_code   = old->obs_code;
  X->id_str     = mk_copy_string(old->id_str);

  return X;
}


void free_simple_obs(simple_obs* old) {
  free_string(old->id_str);
  AM_FREE(old,simple_obs);
}



/* --- Simple Observation Access Functions ------------------------------------ */

int safe_simple_obs_id(simple_obs* X) {
  return X->id;
}

void simple_obs_set_ID(simple_obs* X, int id) {
  X->id = id;
}

date* mk_simple_obs_date(simple_obs* X) {
  return mk_MJD_date(X->time);
}

/* returns a global time (accounting for year/month/day/time) */
double safe_simple_obs_time(simple_obs* X) {
  return X->time;
}

void simple_obs_set_time(simple_obs* X, double time) {
  X->time = time;
}

double safe_simple_obs_RA(simple_obs* X) {
  return X->RA;
}

void simple_obs_set_RA(simple_obs* X, double RA) {
  X->RA = RA;
}

double safe_simple_obs_DEC(simple_obs* X) {
  return X->DEC;
}

void simple_obs_set_DEC(simple_obs* X, double DEC) {
  X->DEC = DEC;
}

/* Return RA/DEC in radians... */
double safe_simple_obs_RA_rad(simple_obs* X)  {
  return (X->RA * 15.0 * DEG_TO_RAD);
}
double safe_simple_obs_DEC_rad(simple_obs* X) {
  return (X->DEC * DEG_TO_RAD);
}

void simple_obs_set_RA_DEC(simple_obs* X, double r, double d) {
  X->RA  = r;
  X->DEC = d;
}

double safe_simple_obs_brightness(simple_obs* X) {
  return (double)X->brightness;
}

void safe_simple_obs_set_brightness(simple_obs* X, double brightness) {
  X->brightness = (float)brightness;
}

int safe_simple_obs_obs_code(simple_obs* X) {
  return X->obs_code;
}

void simple_obs_set_obs_code(simple_obs* X, int code) {
  X->obs_code = code;
}

char* safe_simple_obs_id_str(simple_obs* X) {
  return X->id_str;
}

void simple_obs_set_id_str(simple_obs* X, char* id_str) {
  int i;

  /* Copy the string padding it with spaces if required to */
  /* ensure that it is at least 8 characters.              */
  if(id_str != NULL) {
    if(strlen(id_str) >= 8) {
      X->id_str = mk_copy_string(id_str);
    } else {
      X->id_str = mk_copy_string("        ");
      for(i=0;i<strlen(id_str);i++) { X->id_str[i] = id_str[i]; }
    }
  } else {
    X->id_str = mk_copy_string("0FAKSE00");
  }

}

char safe_simple_obs_type(simple_obs* X) {
  return X->type;
}



/* --- Simple Observation Helper Functions ----------------------------------- */


void simple_obs_add_gaussian_noise(simple_obs* X, double sig_ra, double sig_dec,
                                   double sig_bright) {
  simple_obs_set_RA(X,simple_obs_RA(X)+gen_gauss()*sig_ra);
  simple_obs_set_DEC(X,simple_obs_DEC(X)+gen_gauss()*sig_dec);
  simple_obs_set_brightness(X,simple_obs_brightness(X)+gen_gauss()*sig_bright);
}


/* gives x coordinate if the observation is projected onto the unit sphere */
double simple_obs_unit_X(simple_obs* X) {
  double ra1, dec1;

  /* Correct the coordinates */
  ra1 = 15.0 * X->RA * DEG_TO_RAD;

  /* We want: dec =   0 to be phi =  90
              dec =  90 to be phi =   0
              and      dec = -90 to be phi = 180 */
  dec1 = (90.0 - X->DEC) * DEG_TO_RAD;

  /* Find the cartesian coordinates of the two vectors */
  return cos(ra1)*sin(dec1);
}


/* gives y coordinate if the observation is projected onto the unit sphere */
double simple_obs_unit_Y(simple_obs* X) {
  double ra1, dec1;

  /* Correct the coordinates */
  ra1 = 15.0 * X->RA * DEG_TO_RAD;

  /* We want: dec =   0 to be phi =  90
              dec =  90 to be phi =   0
              and      dec = -90 to be phi = 180 */
  dec1 = (90.0 - X->DEC) * DEG_TO_RAD;

  /* Find the cartesian coordinates of the two vectors */
  return sin(ra1)*sin(dec1);
}


/* gives z coordinate if the observation is projected onto the unit sphere */
double simple_obs_unit_Z(simple_obs* X) {
  double dec1;

  /* We want: dec =   0 to be phi =  90
              dec =  90 to be phi =   0
              and      dec = -90 to be phi = 180 */
  dec1 = (90.0 - X->DEC) * DEG_TO_RAD;

  /* Find the cartesian coordinates of the two vectors */
  return cos(dec1);
}


void simple_calc_XYZ_from_RADEC(double RA, double DEC, double* X,
                                double* Y, double *Z) {
  double r, d;
  
  /* Correct the coordinates 
     We want: dec =   0 to be phi =  90
              dec =  90 to be phi =   0
              and      dec = -90 to be phi = 180 */
  r = 15.0 * RA * DEG_TO_RAD;
  d = (90.0 - DEC) * DEG_TO_RAD;

  X[0] = cos(r)*sin(d);
  Y[0] = sin(r)*sin(d);
  Z[0] = cos(d);
}


/* --- Simple Observation Other Functions ------------------------------------ */

double simple_obs_euclidean_distance(simple_obs* A, simple_obs* B) {
  double dRA;
  double dDEC;

  dDEC = A->DEC - B->DEC;
  dRA  = A->RA  - B->RA;
  if(fabs(dRA) > 12.0) {
    if(dRA < -12.0) { dRA += 24.0; }
    if(dRA >  12.0) { dRA -= 24.0; }
  }
  dRA = dRA*15.0*cos((B->DEC+A->DEC)/2.0 * DEG_TO_RAD);
 
  return sqrt(dRA*dRA + dDEC*dDEC);
}


double simple_obs_euclidean_dist_given(double RA1, double RA2,
                                       double DEC1, double DEC2) {
  double dRA  = RA1  - RA2;
  double dDEC = DEC1 - DEC2;

  if(fabs(dRA) > 12.0) {
    if(dRA < -12.0) { dRA += 24.0; }
    if(dRA >  12.0) { dRA -= 24.0; }
  }
  dRA = dRA*15.0*cos((DEC2+DEC1)/2.0 * DEG_TO_RAD);
 
  return sqrt(dRA*dRA + dDEC*dDEC);
}


/* Computes the weighted euclidean distance:
  
   d = sqrt[(dRA / w_ra)^2 + (dDEC / w_dec)^2]
*/
double simple_obs_weighted_euclidean_distance(simple_obs* A, simple_obs* B, 
                                              double w_ra, double w_dec) {
  double dRA;
  double dDEC;

  dDEC = A->DEC - B->DEC;
  dRA  = A->RA - B->RA;
  if(fabs(dRA) > 12.0) {
    if(dRA < -12.0) { dRA += 24.0; }
    if(dRA >  12.0) { dRA -= 24.0; }
  }
  dRA = dRA*15.0*cos((A->DEC+B->DEC)/2.0 * DEG_TO_RAD);
 
  dRA  = dRA / w_ra;
  dDEC = dDEC / w_dec;

  return sqrt(dRA*dRA + dDEC*dDEC);
}


double angular_distance_RADEC(double ra1, double ra2, double dec1, double dec2) {
  double d1 = dec1 * DEG_TO_RAD;
  double d2 = dec2 * DEG_TO_RAD;
  double r1 = ra1 * 15.0 * DEG_TO_RAD;
  double r2 = ra2 * 15.0 * DEG_TO_RAD;
  double inside, result, temp1, temp2;

  /*inside = cos(d1)*cos(d2)*cos(r1-r2)+sin(d1)*sin(d2);*/

  /* If the points are really close together use the */
  /* more expensive, but more precise formula...     * /
     if(inside > 0.999) {*/
    temp1 = sin((d1-d2)/2.0);
    temp2 = sin((r1-r2)/2.0);
    inside = temp1*temp1+cos(d1)*cos(d2)*temp2*temp2;
    result = 2.0*asin(sqrt(inside));
    /*} else {
    result = acos(inside);
    }*/

  return result;
}


double simple_obs_cosine_distance(simple_obs* A, simple_obs* B) {
  double ra1, ra2, dec1, dec2;
  double x1,y1,z1;
  double x2,y2,z2;
  double top, bot;

  /* Correct the coordinates */
  ra1 = 15.0 * A->RA * DEG_TO_RAD;
  ra2 = 15.0 * B->RA * DEG_TO_RAD;
  
  /* We want: dec =   0 to be phi =  90
              dec =  90 to be phi =   0
     and      dec = -90 to be phi = 180 */

  dec1 = (90.0 - A->DEC) * DEG_TO_RAD;
  dec2 = (90.0 - B->DEC) * DEG_TO_RAD;
  
  /* Find the cartesian coordinates of the two vectors */
  x1 = cos(ra1)*sin(dec1);
  y1 = sin(ra1)*sin(dec1);
  z1 = cos(dec1);

  x2 = cos(ra2)*sin(dec2);
  y2 = sin(ra2)*sin(dec2);
  z2 = cos(dec2);

  top = (x1*x2+y1*y2+z1*z2);
  bot = sqrt(x2*x2+y2*y2+z2*z2) * sqrt(x1*x1+y1*y1+z1*z1);

 return acos(top/bot);
}



/* --- Output functions ------------------------------------------------------- */

void printf_simple_obs(simple_obs* S) {
  fprintf_simple_obs(stdout,"",S,"\n");
}


void fprintf_simple_obs(FILE* f, char* prefix, simple_obs* S, char* suffix) {
  fprintf(f,prefix);
  fprintf(f,"%10.5f: (%18.13f,%18.13f) - %5.3f",S->time,S->RA,S->DEC,S->brightness);  
  fprintf(f,suffix);
}


void fprintf_MPC_simple_obs(FILE* f,simple_obs* S) {
  char* MPC_s = mk_MPC_string_from_simple_obs(S);

  fprintf(f,MPC_s);
  fprintf(f,"\n");

  free_string(MPC_s);
}


char* mk_MPC_string_from_simple_obs(simple_obs* S) {
  char* res;
  char  signn;
  int year, month;
  int dec_h, dec_m;
  int ra_h, ra_m;
  double day, dec_s, ra_s;
  double RA, DEC;
  
  my_assert(strlen(S->id_str) >= 7);

  MJD_to_date(S->time, &year, &month, &day);

  RA       = S->RA;
  if(RA < 0.0)  { RA += 24.0; }
  if(RA > 24.0) { RA -= 24.0; }

  ra_h     = (int)RA;
  RA       = (RA - (double)ra_h) * 60.0;
  ra_m     = (int)RA;
  ra_s     = (RA - (double)ra_m) * 60.0;

  DEC      = S->DEC;
  if(DEC < 0.0) { 
    signn = '-';
    DEC   = -DEC;
  } else {
    signn = '+';
  }
  dec_h    = (int)DEC;
  DEC      = (DEC - (double)dec_h) * 60.0;
  dec_m    = (int)DEC;
  dec_s    = (DEC - (double)dec_m) * 60.0;

  res = mk_printf("     %c%c%c%c%c%c%c  C%4i %02i %08.5f %02i %02i %06.3f%c%02i %02i %05.2f         %04.1f %c      %3i",
                  S->id_str[0], S->id_str[1], S->id_str[2], S->id_str[3],
                  S->id_str[4], S->id_str[5], S->id_str[6],
                  year, month, day, ra_h, ra_m, ra_s, signn, dec_h,
                  dec_m, dec_s, S->brightness, S->type, S->obs_code);

  return res;
}


char* mk_string_from_simple_obs(simple_obs* S) {
  char* res;

  res = mk_printf("%10.5f: (%10.5f,%10.5f) - %5.3f",
                  S->time, S->RA, S->DEC, S->brightness);

  return res;
}


int dyv_count_num_unique(dyv* X, double thresh) {
  dyv* sorted = mk_dyv_sort(X);
  int count = 1;
  int i;

  for(i=1;i<dyv_size(sorted);i++) {
    if((dyv_ref(sorted,i)-dyv_ref(sorted,i-1)) > thresh) {
      count++;
    }
  }

  free_dyv(sorted);
 
  return count;
}


/* Computes the coefficients for the motion equation
   (WITHOUT too much computation... hopefully):

   X = M_0 + M_1 * t + M_2 * 0.5 * t^2

   will default to M_i = 0.0 if i > floor(log2(# points)) + 1
*/
dyv* mk_fill_obs_moments(dyv* X, dyv* time, dyv* weights, int num_coefficients) {
  double A, B, C, D, E, F, G;
  double a, b, c, t, x;
  double bot, w, sum;
  double dobN, tspread;
  dyv* res;
  int i, N, Nv;

  /* Count the number of observations and the number of virtual
     observations (i.e. the number of distinct time steps) */
  N    = dyv_size(X);
  Nv   = dyv_count_num_unique(time, 1e-6);
  dobN = (double)N;
  tspread = dyv_max(time) - dyv_min(time);

  if(weights != NULL) {
    dobN = 0.0;
    for(i=0;i<N;i++) { 
      dobN += (dyv_ref(weights,i) * dyv_ref(weights,i)); 
    }
  }

  my_assert(N > 0);
  res = mk_zero_dyv(num_coefficients);

  /* There are 4 cases:  
     1) Nv <= 0     -> Return all zeros
     2) Nv == 1     -> Return that point with 0 vel/accel
     3) 1 < Nv < 4  -> Compute POS and VEL
     4) Nv >= 4     -> Compute POS, VELL, and ACCEL
  */

  /* Very quickly filter out the 1 and 2 data points case */
  if(Nv == 1) {
    sum = 0.0;
    bot = 0.0;
    for(i=0;i<N;i++) {
      w = 1.0;
      if(weights != NULL) {
        w = dyv_ref(weights,i);
      }
      sum += (dyv_ref(X,i) * w);
      bot += w;
    }
    dyv_set(res,0,sum/bot);
  } else {
    A = 0.0; B = 0.0; C = 0.0;
    D = 0.0; E = 0.0; F = 0.0;
    G = 0.0;

    for(i=0;i<N;i++) {
      w = 1.0;
      if(weights != NULL) {
        w = dyv_ref(weights,i);
        w = w * w;
      }

      t  = dyv_ref(time,i);
      x  = dyv_ref(X,i);
      A += ((t*t*t*t)*w);
      B += ((t*t*t)*w);
      C += ((t*t)*w);
      D += ((t)*w);
      E += ((x*t*t)*w);
      F += ((x*t)*w);
      G += ((x)*w);
    }

    /* Default to linear for a few points or */
    /* A very short arc (< 2 hours).         */
    if((Nv < NUM_FOR_QUAD)||(tspread < 0.1)) {
      bot = D*D - C*dobN;
     
      a = 0.0;    
      b = (G*D - dobN*F)/bot;
      c = (F*D - C*G)/bot;
    } else {
      bot  = A*D*D - A*dobN*C + dobN*B*B;
      bot += C*C*C - 2.0*C*B*D;

      a  = C*C*G - G*B*D + D*D*E;
      a += B*dobN*F-dobN*E*C - D*F*C;
      a  = 2.0 * (a/bot);

      b  = D*A*G - D*E*C - B*G*C;
      b += B*dobN*E + F*C*C -dobN*A*F;
      b  = (b/bot);
      
      c  = E*C*C - E*B*D - A*G*C + A*D*F;
      c += B*B*G - C*B*F;
      c  = (c/bot);
    }

    dyv_set(res,0,c);
    dyv_set(res,1,b);
    dyv_set(res,2,a);
  }

  return res;
}


dyv* mk_obs_RA_moments(simple_obs_array* obs, int num_coeff) {
  int N = simple_obs_array_size(obs);
  dyv* time = mk_dyv(N);
  dyv* RA   = mk_dyv(N);
  dyv* w    = mk_dyv(N);
  dyv* res;
  double RA0, RAt, DECt, dRA;
  double t0, tt, dt;
  int i;

  RA0 = simple_obs_RA(simple_obs_array_ref(obs,0));
  t0  = simple_obs_time(simple_obs_array_ref(obs,0));
  for(i=0;i<N;i++) {
    DECt = simple_obs_DEC(simple_obs_array_ref(obs,i));
    RAt  = simple_obs_RA(simple_obs_array_ref(obs,i));
    tt   = simple_obs_time(simple_obs_array_ref(obs,i));

    dt  = tt - t0;
    dRA = RAt - RA0;
    if(dRA < -12.0) { dRA += 24.0; }
    if(dRA >  12.0) { dRA -= 24.0; }

    dyv_set(RA,i,dRA);
    dyv_set(time,i,dt);
    dyv_set(w,i,cos(DECt * DEG_TO_RAD));
  }

  res = mk_fill_obs_moments(RA, time, w, num_coeff);
  dyv_set(res,0,dyv_ref(res,0)+RA0);

  free_dyv(w);
  free_dyv(RA);
  free_dyv(time);

  return res;
}


dyv* mk_obs_DEC_moments(simple_obs_array* obs, int num_coeff) {
  int N = simple_obs_array_size(obs);
  dyv* time = mk_dyv(N);
  dyv* DEC  = mk_dyv(N);
  dyv* res;
  double DEC0, DECt, dDEC;
  double t0, tt, dt;
  int i;

  DEC0 = simple_obs_DEC(simple_obs_array_ref(obs,0));
  t0   = simple_obs_time(simple_obs_array_ref(obs,0));
  for(i=0;i<N;i++) {
    DECt = simple_obs_DEC(simple_obs_array_ref(obs,i));
    tt   = simple_obs_time(simple_obs_array_ref(obs,i));

    dt  = tt - t0;
    dDEC = DECt - DEC0;

    dyv_set(DEC,i,dDEC);
    dyv_set(time,i,dt);
  }

  res = mk_fill_obs_moments(DEC, time, NULL, num_coeff);
  dyv_set(res,0,dyv_ref(res,0)+DEC0);

  free_dyv(DEC);
  free_dyv(time);

  return res;
}

/* -----------------------------------------------------------------------*/
/* -------- Arrays Observations ------------------------------------------*/
/* -----------------------------------------------------------------------*/


/* --- Memory functions for the simple observation arrays ------------------- */

simple_obs_array* mk_empty_simple_obs_array(int size) {
  simple_obs_array* res = AM_MALLOC(simple_obs_array);
  int i;

  res->size = 0;
  res->max_size = size;

  res->the_obs = AM_MALLOC_ARRAY(simple_obs*,size);
  for(i=0;i<size;i++) {
    res->the_obs[i] = NULL;
  }

  return res;
}


simple_obs_array* mk_copy_simple_obs_array(simple_obs_array* old) {
  simple_obs_array* res = mk_empty_simple_obs_array(old->max_size);
  int i;

  res->size = old->size;
  for(i=0;i<res->size;i++) {
    if(old->the_obs[i]) {
      res->the_obs[i] = mk_copy_simple_obs(old->the_obs[i]);
    }
  }

  return res;
}


simple_obs_array* mk_simple_obs_array_subset(simple_obs_array* old,
                                             ivec* inds) {
  simple_obs_array* res = mk_empty_simple_obs_array(ivec_size(inds));
  int i, ind;

  for(i=0;i<ivec_size(inds);i++) {
    ind = ivec_ref(inds,i);
    if((ind < old->size)&&(ind >= 0)&&(old->the_obs[ind])) {
      simple_obs_array_add(res,old->the_obs[ind]);
    }
  }

  return res;
}


simple_obs_array* mk_simple_obs_array_concat(simple_obs_array* A,
                                             simple_obs_array* B) {
  simple_obs_array* res = mk_empty_simple_obs_array(A->size + B->size);
  int i;

  for(i=0;i<A->size;i++) {  
    simple_obs_array_add(res,A->the_obs[i]);
  }
  for(i=0;i<B->size;i++) {
    simple_obs_array_add(res,B->the_obs[i]);
  }

  return res;
}


simple_obs_array* mk_simple_obs_array_from_MPC_file(char *filename, int start_id) 
{
  simple_obs_array* res = NULL;
  simple_obs* A;
  FILE *fp = fopen(filename,"r");
  int size = 0; 
  int line_number = 0;
  char *s;
  int i;

  if (!fp) {
    printf("ERROR: Unable to open MPC observation file (");
    printf(filename);
    printf(") for reading.\n");
  } else {
    printf("Loading file...\n");

    /* Read through once to count the number of records */
    while((s = mk_next_interesting_line_string(fp,&line_number))) {
      size++;
      free_string(s);
    }

    fclose(fp);
    fp = fopen(filename,"r");

    printf("Found %i observations to read.  Reading...\n",size);

    /* Read through a second time extracting records */
    i = 0;
    res = mk_empty_simple_obs_array(size);
    while((s = mk_next_interesting_line_string(fp,&line_number))) {
      A = mk_simple_obs_from_MPC_string(s,start_id);
      simple_obs_array_add(res,A);
      start_id++;

      free_simple_obs(A);
      free_string(s);
    }
    
    printf("Done loading.\n");
    fclose(fp);
  }

  return res;
}


/* The panstarrs string is a white space separated string with:               */
/* DETECTION_ID EPOCH_MJD RA_DEG DEC_DEG MAG OBSCODE OBJECT_NAME              */
/* The TRACKLET_NAME is optional so the code can be used with find tracklets. */
/* If true_groups and pairs are not null then they hold the true_group        */
/* information and the tracklet information (both need to be FREED if used).  */
/* True_groups is determined by matching object names.                        */
/* True_pairs is determined by matching detection_ids.                        */
/* LENGTH (column 9) is optional and gives the detection elongation (degrees) */
/*   It is placed in the dyv "length" in RADIANS.                             */
/* ANGLE (column 10) is optional and gived the elongation angle (degrees)     */
/*   It is placed in the dyv "angle" in RADIANS.                              */
/* EXPOSURE (column 11) is optional and gives the exposure time (sec)         */
/*   It is placed in the dyv "exp_time" in DAYS.                              */
simple_obs_array* mk_simple_obs_array_from_PANSTARRS_list(char *filename, 
                                                          int start_id,
                                                          ivec** true_groups,
                                                          ivec** pairs,
                                                          dyv** length,
                                                          dyv** angle,
                                                          dyv** exp_time) {
  simple_obs_array* res = NULL;
  simple_obs*       A;
  string_array*     strarr;
  namer*            nm1;
  namer*            nm2;
  FILE *fp = fopen(filename,"r");
  int size = 0; 
  int line_number = 0;
  char* s;
  char* name;
  double tmp;
  int i, ind;

  if (!fp) {
      printf("ERROR: Unable to open observation file (");
      printf(filename);
      printf(") for reading.\n");
  } else {
    printf("Loading file...\n");

    /* Read through once to count the number of records */
    while((s = mk_next_interesting_line_string(fp,&line_number))) {
      size++;
      free_string(s);
    }

    fclose(fp);
    fp = fopen(filename,"r");

    printf("Found %i observations to read.  Reading...\n",size);

    /* Read through a second time extracting records */
    i = 0;
    res     = mk_empty_simple_obs_array(size);
    nm1     = mk_empty_namer(TRUE);
    nm2     = mk_empty_namer(TRUE);
    if(pairs != NULL)       { pairs[0]       = mk_constant_ivec(size,-1);  }
    if(true_groups != NULL) { true_groups[0] = mk_constant_ivec(size,-1);  }
    if(length != NULL)      { length[0]      = mk_constant_dyv(size,-1.0); }
    if(angle != NULL)       { angle[0]       = mk_zero_dyv(size);          }
    if(exp_time != NULL)    { exp_time[0]    = mk_zero_dyv(size);          }

    while((s = mk_next_interesting_line_string(fp,&line_number))) {
      if((strlen(s) > 2)&&(s[0] != '#')) {
        strarr = mk_broken_string(s);
        if(string_array_size(strarr) >= 6) {

          name = string_array_ref(strarr,0);
          A = mk_simple_obs_time(start_id, atof(string_array_ref(strarr,1)),
                                 atof(string_array_ref(strarr,2))/15.0,
                                 atof(string_array_ref(strarr,3)),
                                 atof(string_array_ref(strarr,4)), 'v',
                                 atoi(string_array_ref(strarr,5)), name);

          /* Load the tracklet name into pairs if nessecary. */        
          if(pairs != NULL) {
            add_to_namer(nm2,name);
            ind = namer_name_to_index(nm2,name);
            ivec_set(pairs[0],start_id,ind);
          }

          /* Load the object name into true_groups if nessecary. */        
          if((true_groups != NULL)&&(string_array_size(strarr) >= 7)) {
            name = string_array_ref(strarr,6);

            /* Get the index of this name */
            if(!eq_string(name,"FALSE") && !eq_string(name, "NS") &&
               !eq_string(name, "NA")) {
              add_to_namer(nm1,name);
              ind = namer_name_to_index(nm1,name);
              ivec_set(true_groups[0],start_id,ind);
            } else {
              ivec_set(true_groups[0],start_id,-1);
            }
          }

          if((length != NULL)&&(string_array_size(strarr) >= 8)) {
            tmp = atof(string_array_ref(strarr,7)) * DEG_TO_RAD;
            dyv_set(length[0],start_id,tmp);
          }
          if((angle != NULL)&&(string_array_size(strarr) >= 9)) {
            tmp = atof(string_array_ref(strarr,8)) * DEG_TO_RAD;
            dyv_set(angle[0],start_id,tmp);
          }
          if((exp_time != NULL)&&(string_array_size(strarr) >= 10)) {
            tmp = atof(string_array_ref(strarr, 9));
            dyv_set(*exp_time, start_id, tmp / (24.0 * 60.0 * 60.0));
          }

          simple_obs_array_add(res,A);
          start_id++;
          free_simple_obs(A);
        }
        free_string_array(strarr);
      }
      free_string(s);
    }

    free_namer(nm1);
    free_namer(nm2);
    fclose(fp);

    printf("Done loading.\n");
  }

  return res;
}


/* The panstarrs string is a white space separated string with:               */
/* DETECTION_ID EPOCH_MJD RA_DEG DEC_DEG MAG OBSCODE OBJECT_NAME              */
/* The TRACKLET_NAME is optional so the code can be used with find tracklets. */
/* If true_groups and pairs are not null then they hold the true_group        */
/* information and the tracklet information (both need to be FREED if used).  */
/* True_groups is determined by matching object names.                        */
/* True_pairs is determined by matching detection_ids.                        */
/* LENGTH (column 9) is optional and gives the detection elongation (degrees) */
/*   It is placed in the dyv "length" in RADIANS.                             */
/* ANGLE (column 10) is optional and gived the elongation angle (degrees)     */
/*   It is placed in the dyv "angle" in RADIANS.                              */
/* EXPOSURE (column 11) is optional and gives the exposure time (sec)         */
/*   It is placed in the dyv "exp_time" in DAYS.                              */
simple_obs_array* mk_simple_obs_array_from_PANSTARRS_file(char *filename,
                                                          int start_id, 
                                                          ivec** true_groups,
                                                          ivec** pairs,
                                                          dyv** length,
                                                          dyv** angle,
                                                          dyv** exp_time) {
  simple_obs_array* res = NULL;
  simple_obs*       A;
  string_array*     strarr;
  namer*            nm1;
  namer*            nm2;
  FILE *fp = fopen(filename,"r");
  int size = 0; 
  int line_number = 0;
  char* s;
  char* name;
  double tmp;
  int i, ind;

  if (!fp) {
    printf("ERROR: Unable to open observation file (");
    printf(filename);
    printf(") for reading.\n");
  } else {
    printf("Loading file...\n");

    /* Read through once to count the number of records */
    while((s = mk_next_interesting_line_string(fp,&line_number))) {
      size++;
      free_string(s);
    }

    fclose(fp);
    fp = fopen(filename,"r");

    printf("Found %i observations to read.  Reading...\n",size);

    /* Read through a second time extracting records */
    i = 0;
    res     = mk_empty_simple_obs_array(size);
    nm1     = mk_empty_namer(TRUE);
    nm2     = mk_empty_namer(TRUE);
    if(pairs != NULL)       { pairs[0]       = mk_constant_ivec(size,-1);   }
    if(true_groups != NULL) { true_groups[0] = mk_constant_ivec(size,-1);   }
    if(length != NULL)      { length[0]      = mk_constant_dyv(size, -1.0); }
    if(angle != NULL)       { angle[0]       = mk_zero_dyv(size);           }
    if(exp_time != NULL)    { exp_time[0]    = mk_zero_dyv(size);           }

    while((s = mk_next_interesting_line_string(fp,&line_number))) {
   
      if((strlen(s) > 2)&&(s[0] != '#')) {

              strarr = mk_broken_string(s);
              if(string_array_size(strarr) >= 6) {
          name = string_array_ref(strarr,0);
          A = mk_simple_obs_time(start_id,atof(string_array_ref(strarr,1)),
          atof(string_array_ref(strarr,2))/15.0,
          atof(string_array_ref(strarr,3)),
          atof(string_array_ref(strarr,4)),'v',
          atoi(string_array_ref(strarr,5)),name);

          /* Load the tracklet name into pairs if nessecary. */        
          if(pairs != NULL) {
            add_to_namer(nm2,name);
            ind = namer_name_to_index(nm2,name);
            ivec_set(pairs[0],start_id,ind);
          }

          /* Load the object name into true_groups if nessecary. */        
          if((true_groups != NULL)&&(string_array_size(strarr) >= 7)) {
            name = string_array_ref(strarr, 6);

            /* Get the index of this name */
            if(!eq_string(name,"FALSE") && !eq_string(name, "NS") &&
               !eq_string(name, "NA")) {
              add_to_namer(nm1,name);
              ind = namer_name_to_index(nm1,name);
              ivec_set(true_groups[0],start_id,ind);
            } else {
              ivec_set(true_groups[0],start_id,-1);
            }
          }

          /* Add elongation information (length, angle, and exposure time)  */
          /* if it is present.                                              */
          if((length != NULL)&&(string_array_size(strarr) >= 8)) {
            tmp = atof(string_array_ref(strarr, 7)) * DEG_TO_RAD;
            dyv_set(length[0], start_id, tmp);
          }
          if((angle != NULL)&&(string_array_size(strarr) >= 9)) {
            tmp = atof(string_array_ref(strarr, 8)) * DEG_TO_RAD;
            dyv_set(angle[0], start_id, tmp);
          }
          if((exp_time != NULL)&&(string_array_size(strarr) >= 10)) {
            tmp = atof(string_array_ref(strarr, 9));
            dyv_set(*exp_time, start_id, tmp / (24.0 * 60.0 * 60.0));
          }

          simple_obs_array_add(res,A);
          start_id++;
          free_simple_obs(A);
        }
        free_string_array(strarr);
      }
      free_string(s);
    }

    free_namer(nm1);
    free_namer(nm2);
    fclose(fp);

    printf("Done loading.\n");
  }

  return res;
}



/* DES format contains
 0  ID
 1  TIME (MJD)
 2  OBS_TYPE (O usually)
 3  RA (deg)
 4  DEC (deg)
 5  APPMAG (mag)
 6  FILTER (grizy etc.)
 7  OBSERVATORY (obscode)
 8  RMS_RA (arcsec)
 9  RMS_DEC (arcsec)
10  RMS_MAG (mag)
11  S2N
12  Secret_name
*/
simple_obs_array* mk_simple_obs_array_from_DES_file(char *filename, int start_id, 
                                                    ivec** true_groups, ivec** pairs,
                                                    dyv** length, dyv** angle) {
  simple_obs_array* res = NULL;
  simple_obs*       A;
  string_array*     strarr;
  namer*            nm1;
  namer*            nm2;
  FILE *fp = fopen(filename,"r");
  int size = 0; 
  int line_number = 0;
  char* s;
  char* name;
  double tmp;
  int i, ind;

  if (!fp) {
    printf("ERROR: Unable to open observation file (");
    printf(filename);
    printf(") for reading.\n");
  } else {
    printf("Scanning file...\n");

    /* Read through once to count the number of records */
    while((s = mk_next_interesting_line_string(fp,&line_number))) {
      size++;
      free_string(s);
    }

    fclose(fp);
    fp = fopen(filename,"r");

    printf("Found %i observations to read.  Reading...\n",size);

    /* Read through a second time extracting records */
    i = 0;
    res = mk_empty_simple_obs_array(size);
    nm1 = mk_empty_namer(TRUE);
    nm2 = mk_empty_namer(TRUE);
    if(pairs != NULL)       { pairs[0]       = mk_constant_ivec(size,-1);  }
    if(true_groups != NULL) { true_groups[0] = mk_constant_ivec(size,-1);  }
    if(length != NULL)      { length[0]      = mk_constant_dyv(size,-1.0); }
    if(angle != NULL)       { angle[0]       = mk_zero_dyv(size);          }

    while((s = mk_next_interesting_line_string(fp,&line_number))) {
   
      if((strlen(s) > 2)&&(s[0] != '#') && !(s[0] == '!' && s[1] == '!')) {
        strarr = mk_broken_string(s);
        if(string_array_size(strarr) >= 6) {
          name = string_array_ref(strarr,0);
          A = mk_simple_obs_time(start_id,atof(string_array_ref(strarr,1)),
          atof(string_array_ref(strarr,3))/15.0,
          atof(string_array_ref(strarr,4)),
          atof(string_array_ref(strarr,5)),'v',
          atoi(string_array_ref(strarr,7)),name);

          /* Load the tracklet name into pairs if nessecary. */        
          if(pairs != NULL) {
            add_to_namer(nm2,name);
            ind = namer_name_to_index(nm2,name);
            ivec_set(pairs[0],start_id,ind);
          }

          /* Load the object name into true_groups if nessecary. */        
          if((true_groups != NULL)&&(string_array_size(strarr) >= 12)) {
            name = string_array_ref(strarr, 12);

            /* Get the index of this name */
            if(!eq_string(name,"FALSE") && !eq_string(name, "NS") && !eq_string(name, "NA")) {
              add_to_namer(nm1,name);
              ind = namer_name_to_index(nm1,name);
              ivec_set(true_groups[0],start_id,ind);
            } else {
              ivec_set(true_groups[0],start_id,-1);
            }
          }

          if((length != NULL)&&(string_array_size(strarr) >= 8)) {
            tmp = atof(string_array_ref(strarr,7)) * DEG_TO_RAD;
            dyv_set(length[0],start_id,tmp);
          }
          if((angle != NULL)&&(string_array_size(strarr) >= 9)) {
            tmp = atof(string_array_ref(strarr,8)) * DEG_TO_RAD;
            dyv_set(angle[0],start_id,tmp);
          }

          simple_obs_array_add(res,A);
          start_id++;
          free_simple_obs(A);
        }

        free_string_array(strarr);
      }
      free_string(s);
    }

    free_namer(nm1);
    free_namer(nm2);

    printf("Done loading.\n");

    fclose(fp);
  }

  return res;
}


/* The megacam string is a white space separated string with:          */
/* RA (in degrees)   Dec (in degrees)   Flux   Sigma  Timestamp  Error */
/* The error column is optional.  If it is there and error!=NULL, then */
/* the errors are appended to the end of the error dyv.                */
/* min_sigma - the smallest simga to consider.                         */
simple_obs_array* mk_simple_obs_array_from_megacam_file(char *filename,
                                                        double min_sigma,
                                                        int start_id,
                                                        dyv* error) {
  simple_obs_array* res = NULL;
  simple_obs*       A;
  string_array*     strarr;
  double ra, dec, brt, time, sig;
  FILE *fp = fopen(filename,"r");
  int size = 0; 
  int line_number = 0;
  char* s;
  int i;

  if (!fp) {
    printf("ERROR: Unable to open observation file (");
    printf(filename);
    printf(") for reading.\n");
  } else {
    /* Read through once to count the number of records */
    while((s = mk_next_interesting_line_string(fp,&line_number))) {
      size++;
      free_string(s);
    }

    fclose(fp);
    fp = fopen(filename,"r");

    /* Read through a second time extracting records */
    i = 0;
    res     = mk_empty_simple_obs_array(size);
    while((s = mk_next_interesting_line_string(fp,&line_number))) {
      if((strlen(s) > 2)&&(s[0] != '#')) {
        strarr = mk_broken_string(s);
        if(string_array_size(strarr) >= 5) {
          ra   = atof(string_array_ref(strarr,0))/15.0;
          dec  = atof(string_array_ref(strarr,1));
          brt  = atof(string_array_ref(strarr,2));
          sig  = atof(string_array_ref(strarr,3));
          time = atof(string_array_ref(strarr,4)); 

          if(sig >= min_sigma) {
            while(ra <   0.0) { ra += 24.0; }
            while(ra >= 24.0) { ra -= 24.0; }

            A = mk_simple_obs_simplest(start_id,time,ra,dec,brt);
            simple_obs_array_add(res,A);
            free_simple_obs(A);

            /* Add error if it is there. */
            if((string_array_size(strarr) >= 6)&&(error != NULL)) {
              add_to_dyv(error,atof(string_array_ref(strarr,5)));
            }

            start_id++;
          }
        }
        free_string_array(strarr);
      }
      free_string(s);
    }
    fclose(fp);
  }

  return res;
}


/* The megacam string is a white space separated string with:          */
/* X  Y  Flux   Sigma  Timestamp  Error                                */
/* The error column is optional.  If it is there and error!=NULL, then */
/* the errors are appended to the end of the error dyv.                */
/* The function returns a dym with:                                    */
/* Timestamp, X, Y, Flux, Sigma                                        */
/* min_sigma - the smallest simga to consider.                         */
dym* mk_xy_obs_from_megacam_file(char *filename, double min_sigma,
                                 dyv* error) {
  dym* res = NULL;
  string_array*  strarr;
  double x, y, brt, time, sig;
  FILE *fp = fopen(filename,"r");
  int line_number = 0;
  char* s;
  int i;

  if (!fp) {
    printf("ERROR: Unable to open observation file (");
    printf(filename);
    printf(") for reading.\n");
  } else {

    /* Read through a second time extracting records */
    i = 0;
    res = mk_zero_dym(0,5);
    while((s = mk_next_interesting_line_string(fp,&line_number))) {
      if((strlen(s) > 2)&&(s[0] != '#')) {

        strarr = mk_broken_string(s);
        if(string_array_size(strarr) >= 5) {
          x    = atof(string_array_ref(strarr,0));
          y    = atof(string_array_ref(strarr,1));
          brt  = atof(string_array_ref(strarr,2));
          sig  = atof(string_array_ref(strarr,3));
          time = atof(string_array_ref(strarr,4)); 

          if(sig >= min_sigma) {
            add_row(res);
            dym_set(res,i,0,time);
            dym_set(res,i,1,x);
            dym_set(res,i,2,y);
            dym_set(res,i,3,brt);
            dym_set(res,i,4,sig);

            /* Add error if it is there. */
            if((string_array_size(strarr) >= 6)&&(error != NULL)) {
              add_to_dyv(error,atof(string_array_ref(strarr,5)));
            }

            i++;
          }
        }

        free_string_array(strarr);
      }
      free_string(s);
    }
    fclose(fp);
  }
  return res;
}


/* Load the detections file from either MPC of PanSTARRS format.              */
/* Automatically decides which format the file is in (hopefully).             */
/* If true_groups and pairs are not null then they hold the true_group        */
/* information and the tracklet information (both need to be FREED if used).  */
simple_obs_array* mk_simple_obs_array_from_file_elong(char *filename,
                                                      double max_t_dist,
                                                      ivec** true_groups,
                                                      ivec** pairs,
                                                      dyv** length,
                                                      dyv** angle,
                                                      dyv** exp_time) {
  simple_obs_array* obs;
  FILE *fp = fopen(filename,"r");
  int line_number = 0;
  bool emptyfile = FALSE;
  bool MPC = FALSE;
  ivec_array* tarr;
  ivec* pair;
  char* s;
  int i, j;

  /* Determine if the file is MPC format. */
  if (!fp) {
    printf("ERROR: Unable to open observation file (");
    printf(filename);
    printf(") for reading.\n");
  } else {
    s = mk_next_interesting_line_string(fp,&line_number);

    if(s != NULL) {
      MPC = ((int)strlen(s) >= 79);
      MPC = MPC && ((s[0]==' ')&&(s[1]==' '));
      MPC = MPC && ((s[2]==' ')&&(s[3]==' '));
      MPC = MPC && ((s[20]=='0')||(s[20]=='1')||(s[20]==' '));
      MPC = MPC && ((s[22]==' ')&&(s[64]==' ')&&(s[19]==' '));        
      MPC = MPC && ((s[34]==' ')&&(s[37]==' ')&&(s[47]==' '));
      MPC = MPC && (s[36]!=' ');
      MPC = MPC && ((s[52]!=' ')&&(s[33]!=' ')&&(s[7]!=' '));
      MPC = MPC && ((s[50]==' ')&&(s[58]==' ')&&(s[59]==' '));
      MPC = MPC && ((s[60]==' ')&&(s[61]==' ')&&(s[62]==' '));
      MPC = MPC && ((s[63]==' ')&&(s[64]==' ')&&(s[71]==' '));
      MPC = MPC && ((s[72]==' ')&&(s[73]==' ')&&(s[74]==' '));
      MPC = MPC && ((s[77]!=' ')&&(s[78]!=' ')&&(s[79]!=' '));
      MPC = MPC && ((s[75]==' ')&&(s[76]==' '));
    
      free_string(s);
    } else {
      emptyfile = TRUE;
    }

    fclose(fp);
  }

  if(emptyfile == FALSE) {
    if(MPC) {
      printf("Loading detections as MPC format.\n");
      obs = mk_simple_obs_array_from_MPC_file(filename,0);
    
      if((true_groups != NULL)&&(obs != NULL)) {
        true_groups[0] = mk_simple_obs_groups_from_labels(obs, TRUE);

        if(pairs != NULL) {
          pairs[0] = mk_constant_ivec(ivec_size(true_groups[0]), -1);
          tarr = mk_simple_obs_pairing_from_true_groups(obs, true_groups[0],
                                                        max_t_dist);
          for(i=0; i<ivec_array_size(tarr); i++) {
            pair = ivec_array_ref(tarr,i);
            for(j=0; j<ivec_size(pair); j++) {
              ivec_set(pairs[0], ivec_ref(pair,j), i);
            }
          }
          free_ivec_array(tarr);
        }
      }

      if(length != NULL) { length[0] = NULL; }
      if(angle != NULL) { angle[0]  = NULL; }
      if(exp_time != NULL) { exp_time[0]  = NULL; }
    } else {
      printf("Loading detections as PanSTARRS format.\n");
      obs = mk_simple_obs_array_from_PANSTARRS_file(filename, 0, true_groups,
                                                    pairs, length, angle,
                                                    exp_time);
    }
  } else {
    obs = mk_empty_simple_obs_array(0);
  }
  
  return obs;
}


simple_obs_array* mk_simple_obs_array_from_file(char *filename,
                                                double max_t_dist,
                                                ivec** true_groups,
                                                ivec** pairs) {
  return mk_simple_obs_array_from_file_elong(filename, max_t_dist, true_groups, 
                                             pairs, NULL, NULL, NULL);
}


simple_obs_array* mk_simple_obs_array_from_three_file(char *filename, int start_id) 
{
  simple_obs_array* res = NULL;
  simple_obs* A;
  FILE *fp = fopen(filename,"r");
  int size = 0; 
  int line_number = 0;
  char *s;
  int i;

  if (!fp) {
      printf("ERROR: Unable to open observation file (");
      printf(filename);
      printf(") for reading.\n");
  } else {
    printf("Loading file...\n");

    /* Read through once to count the number of records */
    while((s = mk_next_interesting_line_string(fp,&line_number))) {
      size++;
      free_string(s);
    }

    fclose(fp);
    fp = fopen(filename,"r");

    printf("Found %i observations to read.  Reading...\n",size);

    /* Read through a second time extracting records */
    i = 0;
    res = mk_empty_simple_obs_array(size);
    while((s = mk_next_interesting_line_string(fp,&line_number))) {
      A = mk_simple_obs_from_three_string(s,start_id);
      if(A != NULL) {
        simple_obs_array_add(res,A);
        start_id++;
        free_simple_obs(A);
      }
      free_string(s);
    }
    
    printf("Done loading.\n");

    fclose(fp);
  }

  return res;
}


void free_simple_obs_array(simple_obs_array* old) {
  int i;

  for(i=0;i<old->max_size;i++) {
    if(old->the_obs[i]) { free_simple_obs(old->the_obs[i]); }
  }

  AM_FREE_ARRAY(old->the_obs,simple_obs*,old->max_size);
  AM_FREE(old,simple_obs_array); 
}


simple_obs* safe_simple_obs_array_ref(simple_obs_array* X, int index) {
  my_assert((X->max_size > index)&&(index >= 0));
  return X->the_obs[index];
}


simple_obs* safe_simple_obs_array_first(simple_obs_array* X) {
  return simple_obs_array_ref(X,0);
}


simple_obs* safe_simple_obs_array_last(simple_obs_array* X) {
  return simple_obs_array_ref(X,X->size-1);
}


void simple_obs_array_double_size(simple_obs_array* old) {
  simple_obs** nu_arr;
  int i;

  nu_arr = AM_MALLOC_ARRAY(simple_obs*,2*old->max_size+1);
  for(i=0;i<old->size;i++) {
    nu_arr[i] = old->the_obs[i];
  }
  for(i=old->size;i<2*old->max_size+1;i++) {
    nu_arr[i] = NULL;
  }

  AM_FREE_ARRAY(old->the_obs,simple_obs*,old->max_size);

  old->the_obs  = nu_arr;
  old->max_size = 2*old->max_size+1;
}


void simple_obs_array_set(simple_obs_array* X, int index, simple_obs* A) {
  my_assert(index >= 0);

  while(index >= X->max_size) { simple_obs_array_double_size(X); }
  
  if(X->the_obs[index]) { free_simple_obs(X->the_obs[index]); }
  if(index >= X->size) { X->size = index+1; }

  if(A != NULL) {
    X->the_obs[index] = mk_copy_simple_obs(A);
  } else {
    X->the_obs[index] = NULL;
  }
}


void simple_obs_array_add(simple_obs_array* X, simple_obs* A) {
  simple_obs_array_set(X,X->size,A);
}


int safe_simple_obs_array_size(simple_obs_array* X) {
  return X->size;
}


int safe_simple_obs_array_max_size(simple_obs_array* X) {
  return X->max_size;
}


int simple_obs_array_number_nonnull(simple_obs_array* X) {
  int count = 0;
  int i;

  for(i=0;i<simple_obs_array_size(X);i++) {
    if(simple_obs_array_ref(X,i) != NULL) {
      count++;
    }
  }

  return count;
}


void simple_obs_array_sort_by_time(simple_obs_array* X) {
  simple_obs** temp;
  dyv* times;
  ivec* inds;
  double maxT = -1.0;
  int nonnullcount = 0;
  int i, ind;

  temp = AM_MALLOC_ARRAY(simple_obs*,X->max_size);
  for(i=0;i<X->max_size;i++) { temp[i] = NULL; }

  /* Put the times in a dyv accounting for the fact there
     might be gaps (NULL obs).  In that case uses the maxTime
     plus delta to move them towards the end. */
  times = mk_zero_dyv(X->size);
  for(i=0;i<X->size;i++) {
    if(X->the_obs[i] != NULL) {
      nonnullcount++;
      if(maxT < simple_obs_time(X->the_obs[i])) {
        maxT = simple_obs_time(X->the_obs[i]);
      }
    }
  }
  for(i=0;i<X->size;i++) {
    if(X->the_obs[i] != NULL) {
      dyv_set(times,i,simple_obs_time(X->the_obs[i]));
    } else {
      dyv_set(times,i,maxT+1.0);
    }
  }
   
  inds = mk_ivec_sorted_dyv_indices(times);

  for(i=0;i<X->size;i++) {
    ind = ivec_ref(inds,i);
 
    if((ind >= 0)&&(ind < X->max_size)) {
      temp[i] = X->the_obs[ind];
    }
  }

  AM_FREE_ARRAY(X->the_obs,simple_obs*,X->max_size);
  X->the_obs = temp;
  X->size = nonnullcount;

  free_ivec(inds);
  free_dyv(times);
}


/* Returns a vector with the same indices as in old_inds */
/* but re-ordered to by in ascending order with time.    */
ivec* mk_sort_simple_obs_array_indices(simple_obs_array* obs, ivec* old_inds) {
  simple_obs* X;
  dyv*        times;
  ivec*       order;
  ivec*       nu_inds;
  int N = ivec_size(old_inds);
  int i, ind;

  /* Create the list of times. */
  times = mk_zero_dyv(N);
  for(i=0;i<N;i++) {
    ind = ivec_ref(old_inds,i);
    X   = simple_obs_array_ref(obs,ind);

    if(X != NULL) {
      dyv_set(times,i,simple_obs_time(X));
    }
  }
  order = mk_ivec_sorted_dyv_indices(times);

  /* Put the new indices in the correct order. */
  nu_inds = mk_ivec(N);
  for(i=0;i<N;i++) {
    ivec_set(nu_inds,i,ivec_ref(old_inds,ivec_ref(order,i)));
  }

  /* Free other allocated space. */
  free_ivec(order);
  free_dyv(times);

  return nu_inds;
}


/* Get the time of the earliest observation */
double simple_obs_array_first_time(simple_obs_array* X) {
  simple_obs* A;
  double ts, t;
  int N = simple_obs_array_size(X);
  int i;

  ts = -1.0;
  for(i=0;i<N;i++) {
    A = simple_obs_array_ref(X,i);
    if(A != NULL) {
      t = simple_obs_time(A);

      if((t < ts)||(ts < 0.0)) {
        ts = t;
      }
    }
  }

  return ts;
}


/* Get the time of the latest observation */
double simple_obs_array_last_time(simple_obs_array* X) {
  simple_obs* A;
  double te, t;
  int N = simple_obs_array_size(X);
  int i;

  te = -1.0;
  for(i=0;i<N;i++) {
    A = simple_obs_array_ref(X,i);
    if(A != NULL) {
      t = simple_obs_time(A);

      if((t > te)||(te < 0.0)) {
        te = t;
      }
    }
  }

  return te;
}




void simple_obs_array_add_gaussian_noise(simple_obs_array* obs, double sig_ra,
                                         double sig_dec, double sig_bright) {
  simple_obs* X;
  int i;
  
  for(i=0;i<simple_obs_array_size(obs);i++) {
    X = simple_obs_array_ref(obs,i);
    if(X != NULL) {
      simple_obs_add_gaussian_noise(X,sig_ra,sig_dec,sig_bright);
    }
  }
}


/* Break the observations up by which night the occur */
ivec_array* mk_break_up_by_night(simple_obs_array* obs, double gap) {
  ivec_array* res;
  ivec*       sinds;
  ivec*       tmp;
  dyv*        times;
  double curr;
  double last;
  int i, T;

  /* Grab and sort the times. */
  times  = mk_simple_obs_array_times(obs);
  sinds  = mk_ivec_sorted_dyv_indices(times);
  T      = dyv_size(times);

  /* Allocate space for the result. */
  res = mk_ivec_array(0);
  tmp = mk_ivec(0);

  last = dyv_ref(times,ivec_ref(sinds,0));
  for(i=0;i<T;i++) {
    curr = dyv_ref(times,ivec_ref(sinds,i));

    /* Either treat the observation as part of the night */
    /* or as the start of a new night.                   */
    if(curr-last < gap) {
      add_to_ivec(tmp,ivec_ref(sinds,i));
    } else {
      add_to_ivec_array(res,tmp);

      free_ivec(tmp);
      tmp = mk_ivec_1(ivec_ref(sinds,i));
    }

    last = curr;
  }

  /* Add on the last nights worth of data. */
  if(ivec_size(tmp) > 0) {
    add_to_ivec_array(res,tmp);
  }

  free_ivec(sinds);
  free_ivec(tmp);
  free_dyv(times);

  return res;
}


void simple_obs_array_flatten_to_plates(simple_obs_array* obs, double plate_width) {
  simple_obs* X;
  double time;
  dyv* times;
  int ind;
  int N = simple_obs_array_size(obs);
  int i;

  times = mk_simple_obs_plate_times(obs,plate_width);

  for(i=0;i<N;i++) {
    X = simple_obs_array_ref(obs,i);
    if(X != NULL) {
      time = simple_obs_time(X);
      ind  = find_index_in_dyv(times,time,plate_width);

      if(ind == -1) {
        printf("ERROR: In Pull to Plates\n");
      } else {
        if(dyv_ref(times,ind) > time) {
          ind--;
        }

        simple_obs_set_time(X,dyv_ref(times,ind));
      }
    }
  }

  free_dyv(times);
}


dyv* mk_simple_obs_plate_times(simple_obs_array* obs, double plate_width) {
  dyv* all_times;
  dyv* times;
  int s,e,T;

  all_times = mk_sorted_simple_obs_array_times(obs);
  times     = mk_dyv(0);
  s         = 0;
  T         = dyv_size(all_times);

  while(s < T) {
    e = s;
    add_to_dyv(times,dyv_ref(all_times,s));

    while((e < T)&&(dyv_ref(all_times,e)-dyv_ref(all_times,s) <= plate_width)) {
      e++;
    }
    s = e;
  }

  free_dyv(all_times);
  return times;
}


dyv* mk_simple_obs_array_times(simple_obs_array* obs) {
  dyv* times;
  int N  = simple_obs_array_size(obs);
  int i;

  times = mk_zero_dyv(N);
  for(i=0;i<N;i++) {
    if(simple_obs_array_ref(obs,i) != NULL) {
      dyv_set(times,i,simple_obs_time(simple_obs_array_ref(obs,i)));
    }
  }

  return times;
}


dyv* mk_sorted_simple_obs_array_times(simple_obs_array* obs) {
  dyv* times;
  dyv* s_times;
  int N  = simple_obs_array_size(obs);
  int Nn = simple_obs_array_number_nonnull(obs);
  int j  = 0;
  int i;

  times = mk_dyv(Nn);
  for(i=0;i<N;i++) {
    if(simple_obs_array_ref(obs,i) != NULL) {
      dyv_set(times,j,simple_obs_time(simple_obs_array_ref(obs,i)));
      j++;
    }
  }

  s_times = mk_sorted_dyv(times);
  free_dyv(times);

  return s_times;
}



/*
  obs    - the observations
  times  - a vector of the times you want indexed 
*/
ivec_array* mk_time_to_simple_obs_ind(simple_obs_array* obs, dyv* times) {
  int T = dyv_size(times);
  int N = simple_obs_array_size(obs);
  ivec_array* res;
  ivec* count = mk_zero_ivec(T);
  double curr_t;
  int i, tmatch;

  /* Count the number of times each time has occurred */
  for(i=0;i<N;i++) {
    tmatch = 0;
    curr_t = simple_obs_time(simple_obs_array_ref(obs,i));
    tmatch = find_index_in_dyv(times,curr_t,0.01);
    if(tmatch >= 0) {
     ivec_increment(count,tmatch,1);
    }
  }

  /* create the result vector */
  res = mk_ivec_array_of_given_lengths(count);

  /* Fill the ivecs */
  for(i=N-1;i>=0;i--) {
    tmatch = 0;
    curr_t = simple_obs_time(simple_obs_array_ref(obs,i));
    tmatch = find_index_in_dyv(times,curr_t,0.01);
    if(tmatch >= 0) {
      ivec_array_ref_set(res,tmatch,ivec_ref(count,tmatch)-1,i);
      ivec_increment(count,tmatch,-1);
    }
  }

  free_ivec(count);

  return res;
}


void fprintf_simple_obs_array(FILE* f, simple_obs_array* X) {
  int i;

  for(i=0;i<X->size;i++) {
    fprintf(f,"%6i) ",i);
    if(X->the_obs[i]) { 
      fprintf_simple_obs(f,"",X->the_obs[i],"\n");
    } else {
      fprintf(f,"EMPTY!\n");
    }
  }
}


void fprintf_MPC_simple_obs_array(FILE* f, simple_obs_array* X) {
  int i;

  for(i=0;i<X->size;i++) {
    if(X->the_obs[i]) { 
      fprintf_MPC_simple_obs(f,X->the_obs[i]);
    }
  }
}


/* Print the obs array as: */
/* [time, RA, DEC]         */
void fprintf_simple_obs_array_as_RD_dym(FILE* f, simple_obs_array* X) {
  simple_obs *Y;
  dym* M;
  dyv* times;
  double t0;
  int N = simple_obs_array_size(X);
  int i;

  M     = mk_zero_dym(N,3);
  times = mk_sorted_simple_obs_array_times(X);
  t0    = dyv_ref(times,0);

  for(i=0;i<N;i++) {
    Y = simple_obs_array_ref(X,i);

    dym_set(M,i,0,simple_obs_time(Y)-t0);
    dym_set(M,i,1,simple_obs_RA(Y));
    dym_set(M,i,2,simple_obs_DEC(Y));
  }

  save_dym(f,M);

  free_dym(M);
  free_dyv(times);
}


/* Print the obs array as: */
/* [time, RA, DEC]         */
void fprintf_simple_obs_array_RDT(FILE* f, simple_obs_array* X) {
  simple_obs *Y;
  int N = simple_obs_array_size(X);
  int i;

  for(i=0;i<N;i++) {
    Y = simple_obs_array_ref(X,i);
    fprintf(f,"%25.15g %25.15g %25.15g\n",simple_obs_time(Y),
            simple_obs_RA(Y),simple_obs_DEC(Y));
  }
}


/* Create the true groups directly from the observation labels. */
/* Conv_ids - Indicates whether to change data from spacewatch  */
/*            format into standard format.                      */
ivec* mk_simple_obs_groups_from_labels(simple_obs_array* pts, bool conv_ids) {
  simple_obs* X;
  namer* nm;
  ivec* res;
  bool sw_conv_message = FALSE;
  char* name;
  int i, ind, N;

  /* Allocate space for the results */
  N   = simple_obs_array_size(pts);
  nm  = mk_empty_namer(TRUE);
  res = mk_zero_ivec(N);

  /* Go through each name and place it in the namer */
  for(i=0;i<N;i++) {

    X = simple_obs_array_ref(pts,i);
    if(X != NULL) {
      name = mk_copy_string(simple_obs_id_str(X));

      /* Check if the string is in spacewatch format */
      if(conv_ids && name && (name[4] == '.')) {
        if(sw_conv_message == FALSE) {
          printf("Warning: Found observation label [");
          printf(name);
          printf("] in the simulated\n");
          printf("space watch format (XXXX.#).  This and all future labels of this type\n");
          printf("will be converted to XXXX00 format for true group identification.\n");
          sw_conv_message = TRUE;
        }
        name[4] = '0';
        name[5] = '0';
      }

      /* Get the index of this name */
      add_to_namer(nm,name);
      ind = namer_name_to_index(nm,name);
      ivec_set(res,i,ind);

      free_string(name);
    } else {
      ivec_set(res,i,-1);
    }
  }

  free_namer(nm);

  return res;
}


/* Make the intra-night linkages assuming: the true groups are known */
/* AND everything that occurred within max_t_dist of each other      */
/* should be linked together.                                        */
ivec_array* mk_simple_obs_pairing_from_true_groups(simple_obs_array* obs, ivec* groups,
                                                   double max_t_dist) {
  ivec_array* res;
  ivec*       inds;
  ivec*       used;
  ivec*       mtch;
  double t0, t_curr;
  int s, e, N, g0, indE;

  /* Set up the structures to find the pairs */
  N    = ivec_size(groups);
  s    = 0;
  inds = mk_indices_of_sorted_ivec(groups);
  used = mk_zero_ivec(N);
  res  = mk_empty_ivec_array();

  /* Skip all the -1 entries */
  while((ivec_ref(groups,ivec_ref(inds,s)) < 0)&&(s < N)) { s++; }

  /* Pair up all later entries */
  while(s < N) {
    mtch = mk_ivec(0);
    t0   = simple_obs_time(simple_obs_array_ref(obs,ivec_ref(inds,s)));
    g0   = ivec_ref(groups,ivec_ref(inds,s));
    e    = s;
    
    /* Check all points of the same group to */
    /* see if they fall in the range.        */
    while((e < N)&&(g0 == ivec_ref(groups,ivec_ref(inds,e)))) {
      indE   = ivec_ref(inds,e);
      t_curr = simple_obs_time(simple_obs_array_ref(obs,indE));

      if((ivec_ref(used,indE)==0)&&(fabs(t_curr-t0)<max_t_dist)) {
        add_to_ivec(mtch,indE);
        ivec_set(used,indE,1);
      }
      e++;
    }

    if(ivec_size(mtch) > 0) {
      add_to_ivec_array(res,mtch);
    }
    free_ivec(mtch);

    s = s + 1;
  }

  free_ivec(inds);
  free_ivec(used);

  return res;
}


/* --- Bounds functions on a simple obs array --------------------------- */

/* Use inds=NULL to compute the information on all the observations. */
void simple_obs_array_compute_bounds(simple_obs_array* arr, ivec* inds,
                                     double *r_lo, double* r_hi,
                                     double *d_lo, double* d_hi,
                                     double *t_lo, double* t_hi) {
  simple_obs* X;
  int N = simple_obs_array_size(arr);
  int i, ind;
  double r0, val;

  /* Make sure we are not trying to compute the bounds */
  /* of an empty array.                                */
  if((arr != NULL)&&(simple_obs_array_size(arr) > 0)) {

    if(inds != NULL) { N = ivec_size(inds); }

    /* Start at the first index. */
    if(inds != NULL) { ind = ivec_ref(inds,0); } else { ind = 0; }
    X       = simple_obs_array_ref(arr,ind);
    r0      = simple_obs_RA(X);
    r_lo[0] = simple_obs_RA(X);
    r_hi[0] = simple_obs_RA(X);
    d_lo[0] = simple_obs_DEC(X);
    d_hi[0] = simple_obs_DEC(X);
    t_lo[0] = simple_obs_time(X);
    t_hi[0] = simple_obs_time(X);

    /* Check all of the other observations for bounds. */
    for(i=1;i<N;i++) {
      ind = i;
      if(inds != NULL) { ind = ivec_ref(inds,i); }
      X = simple_obs_array_ref(arr,ind);
   
      if(d_lo[0] > simple_obs_DEC(X)) { d_lo[0] = simple_obs_DEC(X); }
      if(d_hi[0] < simple_obs_DEC(X)) { d_hi[0] = simple_obs_DEC(X); }

      if(t_lo[0] > simple_obs_time(X)) { t_lo[0] = simple_obs_time(X); }
      if(t_hi[0] < simple_obs_time(X)) { t_hi[0] = simple_obs_time(X); }

      val = simple_obs_RA(X);
      while(val - r0 < -12.0) { val += 24.0; }
      while(val - r0 >  12.0) { val -= 24.0; }
      if(r_lo[0] > val) { r_lo[0] = val; }
      if(r_hi[0] < val) { r_hi[0] = val; }
    }
  } else {
    r_lo[0] =   0.0;
    r_hi[0] =  24.0;
    d_lo[0] = -90.0;
    d_hi[0] =  90.0;
    t_lo[0] =   0.0;
    t_hi[0] =   0.0;
  }
}


/* Use inds=NULL to compute the information on all the observations. */
void recenter_simple_obs_array(simple_obs_array* arr, ivec* inds,
                               double r_old, double r_new,
                               double d_old, double d_new,
                               double t_old, double t_new) {
  simple_obs* X;
  int N = simple_obs_array_size(arr);
  int i, ind;
  dym *temp1, *temp2, *temp3, *temp4, *rot;
  dyv* org = mk_zero_dyv(3);
  dyv* res = mk_zero_dyv(3);
  double r, d, x, y, z;
  
  if(inds != NULL) { N = ivec_size(inds); }

  /* Compute the rotation matrix! ----------------- */  
  temp1 = mk_3d_rotation_mat_Z(r_old*15.0*DEG_TO_RAD);
  temp2 = mk_3d_rotation_mat_Y((d_new-d_old)*DEG_TO_RAD); 
  temp3 = mk_3d_rotation_mat_Z(-r_new*15.0*DEG_TO_RAD);
  temp4 = mk_dym_mult(temp3,temp2);
  rot   = mk_dym_mult(temp4,temp1);
  free_dym(temp1);
  free_dym(temp2);
  free_dym(temp3);
  free_dym(temp4);

  /* Check all of the other observations for bounds. */
  for(i=0;i<N;i++) {

    /* Grab the observation. */
   if(inds != NULL) { ind = ivec_ref(inds,i); } else { ind = i; }
    X = simple_obs_array_ref(arr,ind);
    dyv_set(org,0,simple_obs_unit_X(X));
    dyv_set(org,1,simple_obs_unit_Y(X));
    dyv_set(org,2,simple_obs_unit_Z(X));

    /* Compute the rotation. */
    dym_times_dyv(rot,org,res);
    x = dyv_ref(res,0);
    y = dyv_ref(res,1);
    z = dyv_ref(res,2);

    /* Compute the RA/DEC coordinates. */
    d = atan2(z, sqrt(x*x + y*y)) * RAD_TO_DEG;
    r = atan2(y,x) * RAD_TO_DEG / 15.0;
    while(r > 24.0) { r -= 24.0; }
    while(r <  0.0) { r += 24.0; }

    /* Change the actual observation. */
    simple_obs_set_RA(X,r);
    simple_obs_set_DEC(X,d);
    simple_obs_set_time(X,t_new + (simple_obs_time(X)-t_old));
  }

  free_dym(rot);
  free_dyv(org);
  free_dyv(res);
}    


/* -----------------------------------------------------------------------*/
/* -------- Simple Bounds ------------------------------------------------*/
/* -----------------------------------------------------------------------*/

/* Compute the mid RA from a subset (or full set of inds == NULL) */
/* of RA values. */
double mid_RA(dyv* RA, ivec* inds) {
  dyv* all_RA;
  dyv* RA_sort;
  double diff;
  double max_diff_RA  = 0.0;
  double mean_RA;
  int ind_RA  = 0;
  int N;
  int i;

  if(inds == NULL) {
    all_RA = mk_copy_dyv(RA);
  } else {
    all_RA = mk_dyv_subset(RA,inds);
  }
  N        = dyv_size(all_RA);
  RA_sort  = mk_sorted_dyv(all_RA);

  /* Calculate the largest gap, including wrap around */
  max_diff_RA = 0.0;
  for(i=0;i<(N-1);i++) {
    diff = dyv_ref(RA_sort,i+1) - dyv_ref(RA_sort,i);
    if(diff > max_diff_RA) {
      max_diff_RA = diff;
      ind_RA      = i;
    }
  }
  diff = (dyv_ref(RA_sort,0) + 24.0) - dyv_ref(RA_sort,N-1);
  if(diff > max_diff_RA) {
    max_diff_RA = diff;
    ind_RA      = N-1;
  }

  if(ind_RA == N-1) {
    mean_RA = (dyv_ref(RA_sort,N-1) + dyv_ref(RA_sort,0)) / 2.0;
  } else {
    mean_RA = (dyv_ref(RA_sort,ind_RA) + dyv_ref(RA_sort,ind_RA+1) - 24.0) / 2.0;
  }

  free_dyv(all_RA);
  free_dyv(RA_sort);

  if(mean_RA < 0.0)  { mean_RA += 24.0; }
  if(mean_RA > 24.0) { mean_RA -= 24.0; }

  return mean_RA;
}


/* Compute the width in RA from a subset */
/* (or full set of inds == NULL) of RA values. */
double width_RA(dyv* RA, ivec* inds) {
  dyv* all_RA;
  dyv* RA_sort;
  double diff;
  double max_diff_RA  = 0.0;
  double width_RA;
  int ind_RA  = 0;
  int N;
  int i;

  if(inds == NULL) {
    all_RA = mk_copy_dyv(RA);
  } else {
    all_RA = mk_dyv_subset(RA,inds);
  }
  N        = dyv_size(all_RA);
  RA_sort  = mk_sorted_dyv(all_RA);

  /* Calculate the largest gap, including wrap around */
  max_diff_RA = 0.0;
  for(i=0;i<(N-1);i++) {
    diff = dyv_ref(RA_sort,i+1) - dyv_ref(RA_sort,i);
    if(diff > max_diff_RA) {
      max_diff_RA = diff;
      ind_RA      = i;
    }
  }
  diff = (dyv_ref(RA_sort,0) + 24.0) - dyv_ref(RA_sort,N-1);
  if(diff > max_diff_RA) {
    max_diff_RA = diff;
    ind_RA      = N-1;
  }

  if(ind_RA == N-1) {
    width_RA = (dyv_ref(RA_sort,N-1) - dyv_ref(RA_sort,0));
  } else {
    width_RA = (dyv_ref(RA_sort,ind_RA) + (24.0 - dyv_ref(RA_sort,ind_RA+1)));
  }

  free_dyv(all_RA);
  free_dyv(RA_sort);

  return width_RA;
}


double mid_DEC(dyv* DEC, ivec* inds) {
  double min =  500.0;
  double max = -500.0;
  double val;
  int N = ivec_size(inds);
  int i;

  for(i=0;i<N;i++) {
    val = dyv_ref(DEC,ivec_ref(inds,i));
    if((i==0)||(val < min)) {
      min = val;
    }
    if((i==0)||(val > max)) {
      max = val;
    }
  }

  return (min + max)/2.0;
}
