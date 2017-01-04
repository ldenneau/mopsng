Findtracklets is a program to do intra-night linking from observation
files.  The program looks for sets of observations that fit a roughly
linear motion model with a bounded velocity.  In other words, all
observations must be within a given velocity range of each other
|delta_x|/|delta_t| < v_max AND they must fit a simple motion model.

Also included is an evaluation mode.  If evaluation mode is run the
program assumes that the original observation ID fully specifies which
track it belongs to (i.e. the linking could be solved by solely
looking at the ID string).  In this mode the linking is done using the
program and checked using the ID string.  The % correct and % found
are returned for the data set.  This mode should be used to choose
parameters that are correct for a given data set.

---- License Information -----------------------------

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

----- Updates:

Version 2.0.5 (released 3/1/09)
- Small bug fix in PHT math.
- Added the ability to use per detection exposure time.

Version 2.0.4 (released 7/9/08)
- Added the PHT mode (described below).

Version 2.0.3 (released 6/2/07)
- Added "greedy" and "maxobs" command line args.

Version 2.0.2 (released 11/09/05)
- Modified how the use of streaking info works (see below)

Version 2.0.1 (released 10/20/05)
- Fixed a bug where the program would crash on empty
  input file.
- Made memory statement polite

Version 2.0.0 (released 10/11/05)
- Added support for elongation information
- Added a direct software interface

Version 1.0.4 (released 7/8/05)
- Fixed a bug with the maximum velocities scaling.

Version 1.0.3 (released 5/27/05)
- Changed some default values

Version 1.0.2
- New input format supported (see below)
- Changed output files to be white-space separated

Version 1.0.1 
 - Changed input parameters to degrees (from radians)
 - Added outputs for total number of found, true, and matched 
   tracks in eval mode.

----- How to run:

Findtracklets runs from the command line:

./findtracklets file OBS_FILENAME [optional parameters]

For example to run using default parameters type:

./findtracklets file ./fake_small2.txt


The optional parameters are:

thresh - The threshold for a good fit (in degrees).  This is how close
           the points must come to the best fitting line to be
           considered linear (i.e. mean squared error from the fit
           line).  NOTE: This value is given in degrees.  Default =
           0.006.

athresh - The angular threshold for a good fit (in degrees).  How 
            well does the angle of the track elongation have to 
            match the estimated tracklet's line.  This depends on 
            the elongation error model.  NOTE: This value is given 
            in degrees.  Default = 180.0 (i.e. not at all).

maxLerr - The maximum error on the elongation length (in degrees).
            If the length is 0.01 and the maxLerr is 0.005, we
            are claiming the true length must lie between 0.005
            and 0.015.  Default = 0.00015

maxt - The maximum time range for a tracklet (in fractional days).
           This prevents the code from linking two observations from
           different nights, etc.  Default = 0.5

etime - The exposure time (i.e. how much time elapsed during the
            observed elongation) in seconds.  Can be overridden
            in the data file (see below).  Default = 30.0

minv - The minimum angular velocity of a tracklet (in degrees per day).
       NOTE: This value is given in degrees.  Default = 0.0.

maxv - The maximum angular velocity of a tracklet (in degrees per day) if
       no elongation information is provided.  If the observation has an
       elongation then the maximum velocity of the tracklet is the
       maximum of this value and the one derived from the elongation.
       NOTE: This value is given in degrees.  Default = 0.06.

minobs - The minimum number of observations for a valid tracklet.
           Default = 2.

maxobs - The maximum number of observations for a valid tracklet.
         This is useful to prevent the search from branching forever.
         Default = 100.

pairfile - The name of the output pairs file (see below).  Default =
           "pairs.obs"

summaryfile - The name of the output summary file (see below).
           Default = "pairs.sum"

mpc_file - The name of the MPC output file (see below).  If no name
           is provided then no MPC output file is created.
          Default = NULL

greedy - A boolean that indicates whether or not to run in greedy mode.
         If this value is true then the search will *not* branch
         on tracks with >3 observations.  Default = FALSE.

eval - A boolean that indicates whether or not to run in evaluation
           mode.  Default = FALSE.

remove_subsets - A boolean that indicates whether to remove an
           tracklet if its observations are the subset of another
           tracklet.  Set to FALSE to keep subset tracklet.
           Default = TRUE.

use_pht - Switch to PHT from MHT tracking.  Default = FALSE.

Note: The default parameters were chosen because the empirically perform
      well on the simulated data.

--------- Example command line:

./findtracklets file ./fake_small2.txt eval true thresh 0.02 maxt 0.001


------------------------------------------------------
--- MHT Search ---------------------------------------
------------------------------------------------------

The algorithm works by performing an exhaustive multiple hypothesis
search from a given starting observation (each observation is used as
a possible starting point to generate the full list of tracklets).
First a set of neighbors is generated by finding all *future*
observations that are reachable from the starting observation.  Then
all possible paths through these neighbors are considered, building a
list of *all* valid paths.

For example, imagine that our query observation Q had 5 neighbors (x1,
x2, x3, x4, x5).  The search would start off with a list of paths
containing one element, Q:

[Q]

Next the algorithm considers adding the first neighbor (sorted by
time) to each path on the list, providing a new set of candidates:

[Q], [Q, x1]

These paths are tested as to their fit to the motion model with bad
paths being rejected.  Next we consider adding the next neighbor to
each path:

[Q], [Q, x1], [Q, x2], [Q, x1, x2]

Note that we keep old paths, allowing the algorithm to search paths
that skip time steps.  As the algorithm progresses the number of paths
grows quickly (up to 2^# Neighors).  Hopefully most of the paths will
be poor fits and can be pruned.

There are several solutions to preventing the search from going out of
control.  Setting the maximum number of observations will bound the
size of the paths we can add, giving a running time of O(2^maxobs).
Setting the search to greedy will replace paths of length 3+ when we
add an observation.  So if x4 is a good fit for [Q, x1, x2, x3], we
only save [Q, x1, x2, x3, x4] and delete the previous track.  This
allows "deep" searches without a high cost.

A linear motion model is used for 1-3 observations and a quadratic for
>3 observations.  The fit is measured by mean square error to the
track.  The good matchings are dumped to files described below.


------------------------------------------------------
--- PHT Search ---------------------------------------
------------------------------------------------------

A Partial Hough Transform search is used from each possible starting
observation (each observation is used as a possible starting point to
generate the full list of tracklets).  It only searches for linear
tracklets.

First, a set of neighbors is generated by finding all *future*
observations that are reachable from the starting observation.  These
matches are then converted into circular "feasible regions" of
velocity defined by an estimated velocity (vRA, vDEC) and an
associated radius (uncertainty).  Further each region is associated
with the time of the later detection.

Tracklets are constructed by taking each of these regions and forming
a tracklet from of all EARLIER detections whose regions overlaps the
current one.  If two overlapping regions occur at the same time, then
only the closer one is added.

For example if a detection Q matches 4 later detections X1, X2, X3,
and X4 (at times 1, 2, 3, and 4 respectively) and their corresponding
regions have the following overlaps:

(X1, X2), (X1, X3), (X1, X4), (X2, X4)

then we will return the following tracklets (allowing subsets):

{Q, X1, X2, X4}    // Using X4 as the query region
{Q, X1, X3}        // Using X3 as the query region
{Q, X2}            // Using X2 as the query region
{Q, X1}            // Using X1 as the query region

Note that this formulation means that we are not guaranteed to be able
to fit a line that passes within the threshold of all the points.
Rather the threshold is only used to do pairwise compatibility
checking.

PHT also has a strick greedy mode, where only the longest tracklet for
each starting detection is returned.  Thus we will return at most N
tracklets given N detections.


------------------------------------------------------
--- ELONGATION INFORATION ----------------------------
------------------------------------------------------

If elongation information is provided, the algorithm
uses this information to help prune the tracks.  First,
it prunes an detections from a track whose angle of
elongation is more than athresh degrees from the estimated
tracklet's angle.  Second, it limits the search by

If length/etime > maxv the algorithm uses: 

  min_v = MAX[(length-maxLerr)/etime, 0] 
  max_v = (length+maxLerr)/etime

otherwise the algorithm does just uses the given bounds:

  min_v = 0.0
  max_v = maxv

------------------------------------------------------
--- INPUT FILES --------------------------------------
------------------------------------------------------

FindTracklets supports two different input file formats (note that the
program is "smart" and determines the format of the given file from
its structure):

1) Standard MPC format

2) PANSTARRS input format

--------- PANSTARRS Input File Format (from Larry Denneau's Spec):

Each line consists of one detection with the following (white space
separated) information:

ID EPOCH_MJD RA_DEG DEC_DEG MAG OBSCODE OBJECT_NAME LENGTH
ANGLE EXPOSURE_TIME

The ID takes the place of the detection ID.  OBJECT_NAME is optional
and provides the ground truth information for the evaluation code
(i.e. all detections of the same asteroid should have the same
OBJECT_NAME).

LENGTH, ANGLE, and EXPOSURE_TIME are optional elongation information.
If length and angle are present, then this information is used to
re-define the bounds of minimum and maximum velocity.  If
EXPOSURE_TIME is also present then it overrides the etime command-line
parameter.  Note that LENGTH, ANGLE, and EXPOSURE_TIME must be present
for all observations or no observations.


Notes:

* Values will be whitespace-separated (ASCII space or tab characters)

* Ignore lines beginning with '#'

* Fields are never fixed-width.  Some floating-point values may have many digits.

* All time/epoch values should be specified in MJD and have millisecond
precision (1E-8 days).

* All RA and DEC values should be specified in degrees and have a
precision of 1 milliarcsecond (~1E-9 deg).



------------------------------------------------------
--- OUTPUT FILES -------------------------------------
------------------------------------------------------


MATCH FILE -------------------------------

The match file contains the raw matching information.  Each line
corresponds to one tracklet and contains the (white space separated)
numbers of the observations in that tracklet.  The observation numbers
are the line of the input file on which the observation appears (zero
indexed).


SUMMARY FILE -----------------------------

The summary file provides a overview of the matchings.  Each line
contains information about one track/observation assignment in the
form:

track_number observation_number observation_id

The observation_number is that observation's position in the input
file (zero indexed).  The observation_id is the ID string given in the
MPC file.  The track_number indicates to which tracklet the current
assignment applies.


MPC FILE --------------------------------

The MPC file provides the output in the same format as the input.
Each line is a single observation in MPC format.  The observation ID
is changed so that each tracklet has a single unique ID for all of its
observations.  It is important to note that as a result of the linking
process the same observation may appear in multiple tracklets and thus
may appear on multiple output lines with DIFFERENT IDs.

This output file is formated so as to serve as valid input for other
pieces of the MOPS pipeline (such as LinkTracklets).

This file is only generated if a file name is provided with the
mpc_file command-line argument.


------------------------------------------------------
--- SOFTWARE INTERFACE -------------------------------
------------------------------------------------------

FindTracklets also provides a direct API.  FindTracklets takes as
input a list of detections.  The program then links these detections
into proposed tracklets.  Each tracklet can be recovered (one
detection at a time) through the given interface functions.

The API acts in exactly the same way as the above algorithm
description.  All parameters are assumed to be the given default
values unless changes with an explicit setter function
(e.g. FindTracklets_set_thresh).

The FindTracklets library is provided as astatically-linked library
compatible with Linux kernel version 2.4 and gcc 3.2.3.  Thefollowing
entry points are provided:

  FindTracklets_Init
  FindTracklets_set_thresh
  FindTracklets_set_athresh
  FindTracklets_set_maxLerr 
  FindTracklets_set_etime
  FindTracklets_set_maxv
  FindTracklets_set_maxt
  FindTracklets_set_minobs
  FindTracklets_set_eval
  FindTracklets_AddDataDetection
  FindTracklets_AddDataDetection_elong
  FindTracklets_AddTruth
  FindTracklets_AddTrackletName
  FindTracklets_Run
  FindTracklets_Num_Tracklets
  FindTracklets_Num_Detections
  FindTracklets_Get_Match
  FindTracklets_Free


CONVENTIONS

All C functions shall return zero upon success, and non-zero to
indicatean error, except where otherwise explicitly defined.


DECLARATIONS

typedef void *FindTrackletsStateHandle;


FUNCTIONS

/* Initialize the find tracklets program (allocate memory, etc.) */
/* and return an opaque handle that contains state info.         */
/* Everything is created using the default parameter values.     */
int FindTracklets_Init(FindTrackletsStateHandle* state,
                       int verbosity,  /* 0 => no output, 1 => normal, 2 => verbose/debugging */
                       FILE *log_fp    /* file descriptor in for debugging output */
                       );


/* Set the maximum angular threshold (in degrees). */
int FindTracklets_set_athresh(FindTrackletsStateHandle* state, double nu_val);

/* Set the maximum fit threshold (in degrees). */
int FindTracklets_set_thresh(FindTrackletsStateHandle* state, double nu_val);

/* Set the maximum length error (in degrees). */
int FindTracklets_set_maxLerr(FindTrackletsStateHandle* state, double nu_val);

/* Set the length of time of exposure (for elongation) in seconds */
int FindTracklets_set_etime(FindTrackletsStateHandle* state, double nu_val);

/* Set the maximum tracklet velocity (in deg/day) */
int FindTracklets_set_maxv(FindTrackletsStateHandle* state, double nu_val);

/* Set the maximum tracklet time span (in days) */
int FindTracklets_set_maxt(FindTrackletsStateHandle* state, double nu_val);

/* Set the minimum number of required detections. */
int FindTracklets_set_minobs(FindTrackletsStateHandle* state, int nu_val);

int FindTracklets_set_minobs(FindTrackletsStateHandle* state, int nu_val);

/* Use evaluation mode? (0=NO, 1=YES) */
int FindTracklets_set_eval(FindTrackletsStateHandle* state, int val);

/* Use PHT mode? (0=NO, 1=YES) */
int FindTracklets_set_use_pht(FindTrackletsStateHandle* state, int val);

/* Add a data detection to the data set.    */
/* Return the internal FindTracklets number */
/* for that detections.                     */
int FindTracklets_AddDataDetection(FindTrackletsStateHandle fph,
                                  double ra,            /* Right Ascension (in hours) */
                                  double dec,           /* Declination (in degrees)   */
                                  double time,          /* Time (in MJD)              */
                                  double brightness     /* Brightness                 */
                                  );


/* Add a data detection to the data set (with elongation information). */
/* Return the internal FindTracklets number for that detections.       */
int FindTracklets_AddDataDetection_elong(FindTrackletsStateHandle fph,
                                        double ra,          /* Right Ascension (in hours) */
                                        double dec,         /* Declination (in degrees)   */
                                        double time,        /* Time (in MJD)              */
                                        double brightness,  /* Brightness                 */
                                        double angle,       /* Elongation Angle (in deg)  */
                                        double length       /* Elongation Length (in deg) */
                                        );


/* Add the ground truth tracklet to a detection.  Used for evaluation. */
/* Note the detection is indexed by its internal ID.                   */
int FindTracklets_AddTruth(FindTrackletsStateHandle fph,
                                 int detection_index,
                                 int tracklet_num
                                 );


/* Add the ground truth name to a detection.  Used for evaluation. */
/* Note the detection is indexed by its internal ID.               */
int FindTracklets_AddTrackletName(FindTrackletsStateHandle fph,
                                 int detection_index,
                                 char* name
                                 );


/* Process the tree. */
int FindTracklets_Run(FindTrackletsStateHandle fph);


/* Fetch the total number of result tracklets. */
int FindTracklets_Num_Tracklets(FindTrackletsStateHandle fph);


/* Fetch the number of detections in a given tracklet. */
int FindTracklets_Num_Detections(FindTrackletsStateHandle fph,
                                 int tracklet_number
                                 );

/* Fetch a given detection id from a given tracklet.            */
/* This function can be called N times for each tracklet where: */
/*    N = FindTracklets_Num_Detections(fph,tracklet_num)        */
/* Returns -1 on an error and the corresponding FindTracklets   */
/* detection ID otherwise.                                      */
int FindTracklets_Get_Match(FindTrackletsStateHandle fph,
                            int tracklet_number,
                            int match_num
                            );


/* Release all data structures used by DetectionProximity. */
int FindTracklets_Free(FindTrackletsStateHandle fph);
