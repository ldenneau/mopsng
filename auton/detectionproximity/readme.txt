Pan-STARRS MOPS DetectionProximity Interface Specification
Author: Jeremy Kubica and Larry Denneau, Jr.
Version: 1.0.0


OVERVIEW

This document describes the the interface by which the MOPS software
system will interact with Jeremy Kubica's software for determining
which data detections are "close" to which query detections.  The
distance is measure using individual thresholds for each query
detection.


DESCRIPTION

DetectionsProximity accepts as input two lists of inputs: data
detections and query detections.  While reading input it builds
KD-trees for the input data, which are then traversed when computing
the result.

The DetectionProximity library shall be provided as a
statically-linkedlibrary compatible with Linux kernel version 2.4 and
gcc 3.2.3.  Thefollowing entry points shall be provided:

  DetectionProximity_Init 
  DetectionProximity_AddDataDetection
  DetectionProximity_AddQueryDetection
  DetectionProximity_Run 
  DetectionProximity_Num_Matches
  DetectionProximity_Get_Match
  DetectionProximity_Free


CONVENTIONS

All C functions shall return zero upon success, and non-zero to
indicatean error, except where otherwise explicitly defined.


DETECTION DEFINITIONS

The detections are specified by 4 parameters:

ra         - Right Ascension (in degrees 0-360)
dec        - Declination     (in degrees -90 to +90)
time       - The time of detection (in MJD)
brightness - The detection's brightness


DECLARATIONS

typedef void *DetectionProximityStateHandle;  // opaque pointer to struct containing state info


FUNCTIONS

/* Initialize the DetectionProximity engine and return an opaque handle */
/* that contains state info. */
int DetectionProximity_Init(DetectionProximityStateHandle *ophp,
    int verbosity,      /* 0 => no output, 1 => normal, 2 => verbose/debugging */
    FILE *log_fp        /* use as way to pass file descriptor in for debugging output */
);  /* initialize all structures for OP run */


/* Add a data detection to the tree.  Return the internal DetectionProximity */
/* number for that orbit. */
int DetectionProximity_AddDataDetection(DetectionProximityStateHandle fph,
    double ra,            /* Right Ascension (in hours) */
    double dec,           /* Declination (in degrees)   */
    double time,          /* Time (in MJD)              */
    double brightness     /* Brightness                 */
);


/* Add a query detection to the query set.  Return the internal DetectionProximity */
/* number for that orbit. */
int DetectionProximity_AddQueryDetection(DetectionProximityStateHandle fph,
    double ra,            /* Right Ascension (in hours) */
    double dec,           /* Declination (in degrees)   */
    double time,          /* Time (in MJD)              */
    double brightness,    /* Brightness                 */
    double dist_thresh,   /* Query's distance threshold (in degrees) */
    double bright_thresh, /* Query's brightness threshold            */
    double time_thresh    /* Query's time threshold                  */
);


/* Process the tree. */
int DetectionProximity_Run(DetectionProximityStateHandle fph);


/* Fetch a result from processing - determine the number of "nearby" data */
/* orbits for query orbit number "query_num"                              */
/* Note: query_num is the index returned by DetectionProximity_AddQueryDetection  */
int DetectionProximity_Num_Matches(DetectionProximityStateHandle fph, 
    int query_num
);


/* Fetch a result from processing - return the "match_num"th matching       */
/* orbit for query "query_num".  This function can be called N times        */
/* for each query_num where: N = DetectionProximity_Num_Matches(fph,query_num)  */
/* Returns -1 on an error and the corresponding data orbit index otherwise. */
int DetectionProximity_Get_Match(DetectionProximityStateHandle fph,
    int query_num,
    int match_num
);

/* Release all data structures used by DetectionProximity. */
int DetectionProximity_Free(DetectionProximityStateHandle fph);




EXECUTABLE MODE:

Detectionproximity can also be run as an executable from the command line.
The program is called as

./detectionproximity data DATAFILENAME queries QUERYFILENAME [parameters]


EXECUTABLE MODE - PARAMETERS:

data - Filename of the data points.

queries - Filename of the query pointss.

matchfile - The name of the match file.

d_thresh - The distance threshold (in degrees)
           default = 1.00

b_thresh - Brightness threshold
           default = 1.00

t_thresh - Time threshold (in days)
           default = 1.00

verbosity - Verbosity level 0 => no output, 1 => normal, 
                            2 => verbose/debugging
            default = 0


EXECUTABLE MODE - DATA:

In executable mode, detectionproximity can read in both data points
and query points (MITI or MPC format).


EXECUTABLE MODE - OUTPUT:

The match file contains the raw matching information.  Each line
corresponds to one match and contains the (white space separated)
query detection number and data detection number.  The observation
numbers are the line of the input file on which the observation
appears (zero indexed).


LICENSE INFORMATION:

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
