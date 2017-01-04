/*
   File:        occ_tree_funs.c
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

#include "occ_tree_funs.h"

int gen_count;

bool pw_linear_hit_rd_plate(pw_linear* T, rd_plate* P, double thresh) {
  double t = rd_plate_time(P);
  double r, d, dist;
 
  r = pw_linear_predict(T, t, 0);
  d = pw_linear_predict(T, t, 1);
  dist = angular_distance_RADEC(r,rd_plate_RA(P),d,rd_plate_DEC(P));
  return ((dist - rd_plate_radius(P) - thresh) < 1e-10);
}

/* ----------------------------------------------------------------- */
/* --- Evaluation Functions ---------------------------------------- */
/* ----------------------------------------------------------------- */

/* For each ivec in A what is the hamming distance */
/* to its corresponding ivec in B.  Returns sum.   */
double calculate_errors(ivec_array* A, ivec_array* B) {
  double c  = 0.0;
  double co = 0.0;
  int NA = ivec_array_size(A);
  int i, j;
  ivec* a;
  ivec* b;

  for(i=0;i<NA;i++) {
    a  = ivec_array_ref(A,i);
    b  = ivec_array_ref(B,i);
    co = c;

    for(j=0;j<ivec_size(a);j++) {
      if(find_index_in_ivec(b,ivec_ref(a,j)) == -1) { c += 1.0; }
    }
    for(j=0;j<ivec_size(b);j++) {
      if(find_index_in_ivec(a,ivec_ref(b,j)) == -1) { c += 1.0; }
    }

    if(c >= co+0.5) {
      printf("ERROR\n");
      fprintf_ivec(stdout,"A ",a,"\n");
      fprintf_ivec(stdout,"B ",b,"\n");
      wait_for_key();
    }
  }

  return c;
}


/* ----------------------------------------------------------------- */
/* --- Exhaustive Search ------------------------------------------- */
/* ----------------------------------------------------------------- */

ivec_array* mk_exhaustive(pw_linear_array* tarr, rd_plate_array* parr,
                          double thresh) {
  ivec_array* res;
  ivec*       part;
  int i, j;

  gen_count = 0;

  res = mk_zero_ivec_array(rd_plate_array_size(parr));
  for(i=0;i<rd_plate_array_size(parr);i++) {
    part = mk_ivec(0);

    for(j=0;j<pw_linear_array_size(tarr);j++) {
      gen_count++;
      if(pw_linear_hit_rd_plate(pw_linear_array_ref(tarr,j),
                                rd_plate_array_ref(parr,i),thresh)) {
        add_to_ivec(part,j);
      } 
    }

    ivec_array_set(res,i,part);
    free_ivec(part);
  }

  return res;
}


/* ----------------------------------------------------------------- */
/* --- PTree Search ------------------------------------------------ */
/* ----------------------------------------------------------------- */

void occ_tree_pw_linear_range(pw_linear* T, double* r, double *d, double *radius) {
  double rlo, rhi;
  double dlo, dhi;
  double dist;
  int i;

  /* Find the bounds of the region */
  rlo = pw_linear_y(T,0,0); rhi = rlo;
  dlo = pw_linear_y(T,0,1); dhi = dlo;
  for(i=1;i<pw_linear_size(T);i++) {
    if(pw_linear_y(T,i,0) < rlo) { rlo = pw_linear_y(T,i,0); }
    if(pw_linear_y(T,i,1) < dlo) { dlo = pw_linear_y(T,i,1); }
    if(pw_linear_y(T,i,0) > rhi) { rhi = pw_linear_y(T,i,0); }
    if(pw_linear_y(T,i,1) > dhi) { dhi = pw_linear_y(T,i,1); }
  }

  /* Find the center of the region */
  r[0] = (rhi-rlo)/2.0;
  d[0] = (dhi-dlo)/2.0;
  radius[0] = 0.0;

  /* Compute the radius of the region */
  for(i=0;i<pw_linear_size(T);i++) {
    dist = angular_distance_RADEC(r[0],pw_linear_y(T,i,0),
                                  d[0],pw_linear_y(T,i,1));
    if(dist > radius[0]) { radius[0] = dist; }
  }
}


bool pw_linear_seg_hit_plate_tree(plate_tree* tr, double r_s, double r_e,
                                  double d_s, double d_e, double thresh) { 
  double xs, xe, xp, ys, ye, yp, zs, ze, zp;
  double d_p = plate_tree_DEC(tr);
  double r_p = plate_tree_RA(tr);
  double t, top, bot, r, d;
  double dist;
  bool hit = FALSE;

  xs = cos(r_s*15.0*DEG_TO_RAD)*cos(d_s*DEG_TO_RAD);
  ys = sin(r_s*15.0*DEG_TO_RAD)*cos(d_s*DEG_TO_RAD);
  zs = sin(d_s*DEG_TO_RAD);

  xe = cos(r_e*15.0*DEG_TO_RAD)*cos(d_e*DEG_TO_RAD);
  ye = sin(r_e*15.0*DEG_TO_RAD)*cos(d_e*DEG_TO_RAD);
  ze = sin(d_e*DEG_TO_RAD);

  xp = cos(r_p*15.0*DEG_TO_RAD)*cos(d_p*DEG_TO_RAD);
  yp = sin(r_p*15.0*DEG_TO_RAD)*cos(d_p*DEG_TO_RAD);
  zp = sin(d_p*DEG_TO_RAD);

  /* Find the closest point on the line to the plate's center */
  top = (xp-xs)*(xe-xs) + (yp-ys)*(ye-ys) + (zp-zs)*(ze-zs);
  bot = (xe-xs)*(xe-xs) + (ye-ys)*(ye-ys) + (ze-zs)*(ze-zs);
  if((bot < 1e-10)&&(bot > -1e-10)) {
    t = 0.0;
  } else {
    t = top/bot;
  }

  if(t < 0.0) { t = 0.0; }
  if(t > 1.0) { t = 1.0; }

  r = (1.0-t)*r_s + t*r_e;
  d = (1.0-t)*d_s + t*d_e;

  dist = angular_distance_RADEC(r,r_p,d,d_p);
  hit  = (dist - thresh - plate_tree_radius(tr) < 1e-10);

  return hit;
}



bool pw_linear_hit_plate_tree(plate_tree* tr, pw_linear* T, double thresh) {
  bool hit = FALSE;
  double p_ts, p_te, ts, te;
  double r1, r2, d1, d2;
  int s, N;

  /* Get the time bounds of the plate tree node */
  N    = pw_linear_size(T);
  p_ts = plate_tree_lo_time(tr);
  p_te = plate_tree_hi_time(tr);

  /* Check that the bounds are valid. */
  my_assert((pw_linear_x(T,0) < p_ts)&&(pw_linear_x(T,N-1) > p_te));

  s  = pw_linear_first_larger_x(T,p_ts);
  if(s > 0) { s--; }

  while((s < N-1)&&(pw_linear_x(T,s) < p_te)&&(hit==FALSE)) {

    if(p_ts > pw_linear_x(T,s) + 1e-3) {
      ts = p_ts;
      r1 = pw_linear_predict(T, p_ts, 0);
      d1 = pw_linear_predict(T, p_ts, 1);
    } else {
      ts = pw_linear_x(T,s);
      r1 = pw_linear_y(T,s,0);
      d1 = pw_linear_y(T,s,1);
    }

    if(p_te < pw_linear_x(T,s+1) - 1e-3) {
      te = p_te;
      r2 = pw_linear_predict(T, p_te, 0);
      d2 = pw_linear_predict(T, p_te, 1);
    } else {
      te = pw_linear_x(T,s+1);
      r2 = pw_linear_y(T,s+1,0);
      d2 = pw_linear_y(T,s+1,1);
    }

    hit = pw_linear_seg_hit_plate_tree(tr,r1,r2,d1,d2,thresh);

    s++;
  }

  return hit;
}


bool pw_linear_hit_plate_tree2(plate_tree* tr, pw_linear* T, double thresh) {
  bool hit = FALSE;
  double p_ts, p_te, tps, tpe;
  int s, N;
  double ts, rs, ds, xs, ys, zs;
  double te, re, de, xe, ye, ze;
  double dp, rp, xp, yp, zp;
  double t, top, bot, r, d;
  double dist;

  /* Get the time bounds of the plate tree node */
  N    = pw_linear_size(T);
  p_ts = plate_tree_lo_time(tr);
  p_te = plate_tree_hi_time(tr);

  /* Find the FIRST segment to try. */
  s  = pw_linear_first_larger_x(T,p_ts);
  if(s > 0) { s--; }
  if(s >= N-1) { s = N-2; }

  /* Find all of the plate information ONCE */
  dp = plate_tree_DEC(tr);
  rp = plate_tree_RA(tr);
  xp = cos(rp*15.0*DEG_TO_RAD)*cos(dp*DEG_TO_RAD);
  yp = sin(rp*15.0*DEG_TO_RAD)*cos(dp*DEG_TO_RAD);
  zp = sin(dp*DEG_TO_RAD);

  /* Find the information for the first segment */
  re = pw_linear_y(T,s,0);
  de = pw_linear_y(T,s,1);
  xe = cos(re*15.0*DEG_TO_RAD)*cos(de*DEG_TO_RAD);
  ye = sin(re*15.0*DEG_TO_RAD)*cos(de*DEG_TO_RAD);
  ze = sin(de*DEG_TO_RAD);  

  /* Try each viable segment, looking for a match. */
  while((s < N-1)&&(pw_linear_x(T,s) < p_te)&&(hit==FALSE)) {
    gen_count++;

    /* Update the segment information. */
    rs = re; ds = de;
    xs = xe; ys = ye; zs = ze;

    re = pw_linear_y(T,s+1,0);
    de = pw_linear_y(T,s+1,1);
    xe = cos(re*15.0*DEG_TO_RAD)*cos(de*DEG_TO_RAD);
    ye = sin(re*15.0*DEG_TO_RAD)*cos(de*DEG_TO_RAD);
    ze = sin(de*DEG_TO_RAD);

    /* Find the time bounds of the box in relation to the line */
    ts  = pw_linear_x(T,s);
    te  = pw_linear_x(T,s+1);
    tps = (plate_tree_lo_time(tr) - ts)/(te - ts);
    tpe = (plate_tree_hi_time(tr) - ts)/(te - ts);
    if(tps < 0.0) { tps = 0.0; }
    if(tpe > 1.0) { tpe = 1.0; }

    /* Check that the time bounds are OK */
    if(tps < tpe + 1e-10) {
      /* Find the closest point on the line to the plate's center */
      top = (xp-xs)*(xe-xs) + (yp-ys)*(ye-ys) + (zp-zs)*(ze-zs);
      bot = (xe-xs)*(xe-xs) + (ye-ys)*(ye-ys) + (ze-zs)*(ze-zs);
      if((bot < 1e-10)&&(bot > -1e-10)) {
        t = 0.0;
      } else {
        t = top/bot;
      }

      if(t < tps) { t = tps; }
      if(t > tpe) { t = tpe; }

      r = (1.0-t)*rs + t*re;
      d = (1.0-t)*ds + t*de;

      dist = angular_distance_RADEC(r,rp,d,dp);
      hit  = (dist - thresh - plate_tree_radius(tr) < 1e-10);
    }
    s++;
  }

  return hit;
}


void plate_tree_search_int_recurse(plate_tree* tr, rd_plate_array* parr,
                                   pw_linear* T, double thresh, ivec* res) {
  int i, ind;
  bool recurse;

  if(plate_tree_is_leaf(tr)) {
    for(i=0;i<ivec_size(plate_tree_rd_plates(tr));i++) {
      ind = ivec_ref(plate_tree_rd_plates(tr),i);
      gen_count++;

      if(pw_linear_hit_rd_plate(T,rd_plate_array_ref(parr,ind),thresh)) {
        add_to_ivec(res,ind);
      }
    }
  } else {

    recurse = pw_linear_hit_plate_tree2(tr,T,thresh);

    if(recurse) {
      plate_tree_search_int_recurse(plate_tree_right_child(tr),parr,T,thresh,res);
      plate_tree_search_int_recurse(plate_tree_left_child(tr),parr,T,thresh,res);
    }
  }
}


ivec_array* mk_plate_tree_int_search(plate_tree* tr, rd_plate_array* parr,
                                     pw_linear_array* tarr, double thresh) {
  ivec_array* res;
  ivec*       subres;
  int N = rd_plate_array_size(parr);
  int i, j;

  gen_count = 0;

  /* For each pw_linear find which rd_plates it hits. */
  res = mk_zero_ivec_array(N);
  for(i=0;i<pw_linear_array_size(tarr);i++) {
    subres = mk_ivec(0);
    plate_tree_search_int_recurse(tr,parr,pw_linear_array_ref(tarr,i),thresh,subres);

    for(j=0;j<ivec_size(subres);j++) {
      add_to_ivec_array_ref(res,ivec_ref(subres,j),i);
    }

    free_ivec(subres);
  }

  return res;
}


/* ----------------------------------------------------------------- */
/* --- MTTree Search ----------------------------------------------- */
/* ----------------------------------------------------------------- */

bool plate_hit_pw_tree(pw_tree* tr, rd_plate* P, double thresh) {
  pw_linear* trck;
  double t, r, d, dist, rad;
  int s, e;

  t    = rd_plate_time(P);
  trck = pw_tree_anchor(tr);

  /* Find the prediction and boundry points... */
  r = pw_linear_predict_full(trck,t,0,&s,&e);
  d = pw_linear_predict_full(trck,t,1,&s,&e);
  dist = angular_distance_RADEC(r,rd_plate_RA(P),d,rd_plate_DEC(P));

  /* Find the radius for this segment. */  
  rad = pw_tree_pw_radius(tr,s);
  if(rad < pw_tree_pw_radius(tr,e)) {
    rad = pw_tree_pw_radius(tr,e);
  }

  dist = dist - rad - rd_plate_radius(P);

  return (dist < thresh + 1e-8);
}


void pw_tree_search_recurse(pw_tree* tr, pw_linear_array* tarr,
                            rd_plate* X, double thresh, ivec* res) {
  bool recurse;
  ivec* inds;
  int i, ind, old;

  if(pw_tree_is_leaf(tr)) {

    inds = pw_tree_tracks(tr);
    for(i=0;i<ivec_size(inds);i++) {
      ind = ivec_ref(inds,i);
      gen_count++;

      if(pw_linear_hit_rd_plate(pw_linear_array_ref(tarr,ind), X, thresh)) {
        add_to_ivec(res,ind);
      }
    }

  } else {
    gen_count++;
    recurse = plate_hit_pw_tree(tr,X,thresh);
    old = ivec_size(res);

    if(recurse) {
      pw_tree_search_recurse(pw_tree_right_child(tr),tarr,X,thresh,res);
      pw_tree_search_recurse(pw_tree_left_child(tr),tarr,X,thresh,res);
    }
  }

}


ivec_array* mk_pw_tree_search(pw_tree* tr, pw_linear_array* tarr,
                              rd_plate_array* parr, double thresh) {
  ivec_array* res;
  ivec*       subres;
  int N = rd_plate_array_size(parr);
  int i;

  gen_count = 0;

  res = mk_zero_ivec_array(N);
  for(i=0;i<N;i++) {
    subres = mk_ivec(0);
    pw_tree_search_recurse(tr, tarr, rd_plate_array_ref(parr,i), thresh, subres);
    ivec_array_set(res,i,subres);
    free_ivec(subres);
  }

  return res;
}



/* ----------------------------------------------------------------- */
/* --- MTTree/PTree Search ----------------------------------------- */
/* ----------------------------------------------------------------- */


bool old_pw_linear_seg_hit_plate_tree(plate_tree* tr, double r_s, double r_e,
                                  double d_s, double d_e, double thresh, bool verb) { 
  double xs, xe, xp, ys, ye, yp, zs, ze, zp;
  double d_p = plate_tree_DEC(tr);
  double r_p = plate_tree_RA(tr);
  double t, top, bot, r, d;
  double dist;
  bool hit = FALSE;

  dist = angular_distance_RADEC(r_s,r_p,d_s,d_p);
  hit  = (dist - thresh - plate_tree_radius(tr) < 1e-10);

  if(hit==FALSE) {
    dist = angular_distance_RADEC(r_e,r_p,d_e,d_p);
    hit  = (dist - thresh - plate_tree_radius(tr) < 1e-10);
  }

  if(hit==FALSE) {
    xs = cos(r_s*15.0*DEG_TO_RAD)*cos(d_s*DEG_TO_RAD);
    ys = sin(r_s*15.0*DEG_TO_RAD)*cos(d_s*DEG_TO_RAD);
    zs = sin(d_s*DEG_TO_RAD);

    xe = cos(r_e*15.0*DEG_TO_RAD)*cos(d_e*DEG_TO_RAD);
    ye = sin(r_e*15.0*DEG_TO_RAD)*cos(d_e*DEG_TO_RAD);
    ze = sin(d_e*DEG_TO_RAD);

    xp = cos(r_p*15.0*DEG_TO_RAD)*cos(d_p*DEG_TO_RAD);
    yp = sin(r_p*15.0*DEG_TO_RAD)*cos(d_p*DEG_TO_RAD);
    zp = sin(d_p*DEG_TO_RAD);

    /* Find the closest point on the line to the plate's center */
    top = (xp-xs)*(xe-xs) + (yp-ys)*(ye-ys) + (zp-zs)*(ze-zs);
    bot = (xe-xs)*(xe-xs) + (ye-ys)*(ye-ys) + (ze-zs)*(ze-zs);
    if((bot < 1e-10)&&(bot > -1e-10)) {
      t = 0.0;
    } else {
      t = top/bot;
    }

    if((t > -1e-10)&&(t < 1.0+1e-10)) {
      r = (1.0-t)*r_s + t*r_e;
      d = (1.0-t)*d_s + t*d_e;

     dist = angular_distance_RADEC(r,r_p,d,d_p);
      hit  = (dist - thresh - plate_tree_radius(tr) < 1e-10);
    }
  }

  return hit;
}


bool pw_tree_hit_plate_tree(plate_tree* ptr, pw_tree* ttr, double thresh, bool verb) {
  bool hit = FALSE;
  pw_linear* T;
  double p_ts, p_te, ts, te;
  double r1, r2, d1, d2;
  double dp = plate_tree_DEC(ptr);
  double rp = plate_tree_RA(ptr);
  double radius;
  int s, N;

  /* Get the time bounds of the plate tree node */
  T    = pw_tree_anchor(ttr);
  N    = pw_linear_size(T);
  p_ts = plate_tree_lo_time(ptr);
  p_te = plate_tree_hi_time(ptr);

  /* Check that the bounds are valid. */
  my_assert((pw_linear_x(T,0) < p_ts)&&(pw_linear_x(T,N-1) > p_te));

  s  = pw_linear_first_larger_x(T,p_ts);
  if(s > 0) { s--; }

  if(verb) {
    printf("PTREE PRUNING\n");
    printf("Node = [%f,%f] (%f,%f)=%f\n",p_ts,p_te,rp,dp,plate_tree_radius(ptr));
    printf("Starting at link %i of %i\n",s,N);
  }

  while((s < N-1)&&(pw_linear_x(T,s) < p_te)&&(hit==FALSE)) {

    if(p_ts > pw_linear_x(T,s)) {
      ts = p_ts;
      r1 = pw_linear_predict(T, p_ts, 0);
      d1 = pw_linear_predict(T, p_ts, 1);
    } else {
      ts = pw_linear_x(T,s);
      r1 = pw_linear_y(T,s,0);
      d1 = pw_linear_y(T,s,1);
    }

    if(p_te < pw_linear_x(T,s+1)) {
      te = p_te;
      r2 = pw_linear_predict(T, p_te, 0);
      d2 = pw_linear_predict(T, p_te, 1);
    } else {
      te = pw_linear_x(T,s+1);
      r2 = pw_linear_y(T,s+1,0);
      d2 = pw_linear_y(T,s+1,1);
    }

    radius = pw_tree_pw_radius(ttr,s);
    if(radius < pw_tree_pw_radius(ttr,s+1)) { radius = pw_tree_pw_radius(ttr,s+1); }

    hit = old_pw_linear_seg_hit_plate_tree(ptr,r1,r2,d1,d2,thresh+radius,verb);
     
    s++;
  }

  return hit;
}


void dual_tree_search_recurse(pw_tree* ttr, pw_linear_array* tarr,
                              plate_tree* ptr, rd_plate_array* parr,
                              double thresh, ivec_array* res) {
  rd_plate*  P;
  pw_linear* T;
  ivec* pinds;
  ivec* tinds;
  double pnum, tnum;
  int i,j;
  bool recurse;

  if(pw_tree_is_leaf(ttr) && plate_tree_is_leaf(ptr)) {

    pinds = plate_tree_rd_plates(ptr);
    tinds = pw_tree_tracks(ttr);

    for(i=0;i<ivec_size(pinds);i++) {
      for(j=0;j<ivec_size(tinds);j++) {
        P = rd_plate_array_ref(parr,ivec_ref(pinds,i));
        T = pw_linear_array_ref(tarr,ivec_ref(tinds,j));

        gen_count++;
        if(pw_linear_hit_rd_plate(T,P,thresh)) {
          add_to_ivec_array_ref(res,ivec_ref(pinds,i),ivec_ref(tinds,j));
        }
      }
    }

  } else {

    recurse = pw_tree_hit_plate_tree(ptr,ttr,thresh,FALSE);
    gen_count++;

    if(recurse) {
      pnum = plate_tree_radius(ptr)*1.5;
      tnum = pw_tree_radius(ttr);

      if(((pw_tree_is_leaf(ttr)==FALSE) && (pnum < tnum)) || plate_tree_is_leaf(ptr)) {
        dual_tree_search_recurse(pw_tree_right_child(ttr),tarr,ptr,parr,thresh,res);
        dual_tree_search_recurse(pw_tree_left_child(ttr),tarr,ptr,parr,thresh,res);
      } else {
        dual_tree_search_recurse(ttr,tarr,plate_tree_right_child(ptr),parr,thresh,res);
        dual_tree_search_recurse(ttr,tarr,plate_tree_left_child(ptr),parr,thresh,res);
      }
    }

  }  
}


ivec_array* mk_dual_tree_search(pw_tree* ttr, pw_linear_array* tarr,
                                plate_tree* ptr, rd_plate_array* parr,
                                double thresh) {
  ivec_array* res = mk_zero_ivec_array(rd_plate_array_size(parr));

  gen_count = 0;

  dual_tree_search_recurse(ttr,tarr,ptr,parr,thresh,res);
  return res;
}
