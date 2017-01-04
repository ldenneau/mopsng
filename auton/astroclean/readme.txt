AstroClean is a program that applies a set of simple heuristic filters
to a data set.  The program filters are described below.  By default
all of these filters are turned OFF.

The program filters the detections based on the given criteria and
returns a list of "clean" detections.  Optionally the program can plot
the detections and return a list of "noise" detections.

It is important to note that AstroClean's filters are largely
heuristic.  Further, the implementations are not yet optimized.

Copyright 2006, The Auton Lab, CMU


------------------------------------------------------
---- License Information -----------------------------
------------------------------------------------------

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


------------------------------------------------------ 
---- History/Updates ---------------------------------
------------------------------------------------------

Version 1.0.0 (Released )


------------------------------------------------------
---- How to Run --------------------------------------
------------------------------------------------------

AstroClean runs from the command line:

./astrocleam file OBS_FILENAME [optional parameters]


------------------------------------------------------
---- Filters and Parameters --------------------------
------------------------------------------------------

AstroClean has a variety of different filters, each with associated
parameters.  The parameters and filters are described below.

--- General Parameters -------------------------------

AstroClean has several general parameters:

file - The input file string

clean_file - The name of the output file for "clean" detections.
  Default: "output.txt"

noise_file - The name of the output file for "noise" detections.  
  If no name is given then no noise output file is produced.  
  Default: <none>

outtype - The file type for output (default 0):
  0 = MPC
  1 = MITI


--- Brightness Filter --------------------------------

Remove all detections with brightness outside the given range [bLO,
bHI].  These bounds are given on the numerical value of brightness.
So bLO < bHI.  If either bLO or bHI are less than 0.0 then that bound
is ignored, allowing you to place only a minimum or maximum filter.
If both values are less than 0.0, the filter is ignored.

Parameters:

bLO - The low bound on the numeric value of the brightness.  
  Default: -1.0

bHI - The high bound on the numeric value of the brightness.  
  Default: -1.0


--- Time Filter --------------------------------------

Remove all detection outside of a given time range [tLO, tHI].  If
either tLO or tHI are less than 0.0 then that bound is ignored,
allowing you to place only a minimum or maximum filter.  If both
values are less than 0.0, the filter is ignored.

Parameters:

tLO - The low bound on the time (MJD).  
  Default: -1.0

tHI - The high bound on the time (MJD).
  Default: -1.0


--- Region Extraction --------------------------------

There are two ways to extract regions in AstroClean.  First, you can
specify a minimum and maximum RA/DEC.  Second, you can specify a point
and radius (extracting all detections within the radius of the given
point).  AstroClean allows you to do either, both, or none.

Point-based: Extract all points within "regionrad" of the point
("regionra", "regiondec").  If regionrad < 0.0 then this filter is
ignored. Parameters:

regionrad - The region radius (in degrees).  Default: -1.0

regionra - The RA coordinate of the center of the region (in hours).
  Default: 0.0

regiondec - The DEC coordinate of the center of the region (in
  degrees).  Default: 0.0

For example, to find all detections within 1 degree of RA=12.4 and
  DEC=33.56 use:

./astroclean file FILENAME regionrad 1.0 regionra 12.4 regiondec 33.56


Coordinate-based: Extract all coordinates with RA in [ra_min, ra_max]
and DEC in [dec_min, dec_max].  If either of the RA bounds are less
than 0.0, it is ignored.  If either of the dec_bounds < -90.0, it is
ignored.  This filter does NOT handle wrap around, so min <= max.
Parameters:

ra_min - The minimum RA (in hours).  Default: -1.0

ra_max - The maximum RA (in hours).  Default: -1.0

dec_min - The minimum DEC (in degrees).  Default: -1.0

dec_max - The maximum DEC (in degrees).  Default: -1.0

For example, to find all detections with RA > 12.4 and -30.0 < DEC <
30.0 use:

./astroclean file FILENAME ra_min 12.4 dec_min -30.0 dec_max 30.0


--- Proximity Filtering ------------------------------

If two points are within "proxrad" of each other remove the dimmer
one.  In case of ties, remove the one with the higher index.  If
proxrad < 0.0 ignore this filter.  Parameters:

proxrad - The proximity radius (in degrees).  Default: -1.0


--- Density Filtering --------------------------------

There are four ways to filter on density in AstroClean.  All of them
check for densities > "density".  If density < 0.0 then NO density
filter is applied.

Point-based Boolean: Determine whether to remove each point by
checking the density of its neighbors.  Count the number of detections
within radius "Dradius".  If there are too many neighbors (>
"density), remove the point.  Completely clears out high density
localized regions.

Point-based Relative: Determine whether to remove each point by
checking the density of its neighbors.  Count the number of detections
within radius "Dradius".  If there are too many neighbors (>
"density), determine the correct number N we should see under the
given density.  If the current detection's brightness is in the top N
of its neighbors, keep it.  Adaptively clears out the high density
localized regions (keeping the brightest detections in each region).

Field-based Boolean: Compute the number of detections and area of each
field and compare with the target density ("density").  If the compute
density is higher than the given density, remove all detections in
this field.

Field-based Relative: Compute the number of detections and area of
each field and compare with the target density ("density").  Determine
the correct number N we should see under the given density and keep
the brightest N.

Parameters:

density - The target density (in detections per square degree).  
  If < 0.0 no filtering is done.  Default: -1.0

Dradius - The search radius (in degrees) for point-based filtering.  
  If < 0.0 field-based filtering is used.  Default: -1.0

Drelative - Use relative filtering (otherwise use Boolean).  
  Default: true

For example, to use point-base relative filtering at 100.0 detections
per square degree and a search radius of 1 degree:

./astroclean file FILENAME density 100.0 Dradius 1.0

and to do field-based Boolean filtering at the same density:

./astroclean file FILENAME density 100.0 Drelative false


--- Linear  Filtering --------------------------------

AstroClean can filter "linear" noise in the data.  It uses a heuristic
that first finds all detections within a radius of "linerad" of the
current detection and computes the angle to those neighbors.  If more
than "linesupport" different detections have angles within "lineang"
of each other, the point is assumed to belong to a line and is
removed.  If linesupport < 1, this filter is ignored.

Parameters:

linesupport - The minimum number of detections needed to support a
line.  Default: 0

linerad - The search radius (in degrees) for neighboring points.
Default: 0.1

lineang - The angle threshold (in degrees).  All supporting points
must fall within lineang of each other.  Default: 2.0



------------------------------------------------------
--- INPUT FILES --------------------------------------
------------------------------------------------------

AstroClean supports two different input file formats (note that the
program is "smart" and determines the format of the given file from
its structure):

1) Standard MPC format

2) PANSTARRS input format


--------- PANSTARRS Input File Format (from Larry Denneau's Spec):

Each line consists of one detection with the following (white space
separated) information:

ID EPOCH_MJD RA_DEG DEC_DEG MAG OBSCODE OBJECT_NAME LENGTH ANGLE

The ID takes the place of the detection ID.  OBJECT_NAME is optional
and provides the ground truth information for the evaluation code
(i.e. all detections of the same asteroid should have the same
OBJECT_NAME).

LENGTH and ANGLE are optional elongation information.  If

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

AstroClean will output between 1 and 5 files.

1) A file of "clean" detections in either MPC or Pan-STARRS/MITI
   format (see above)

2) A file of "noise" detections in either MPC or Pan-STARRS/MITI
   format (see above).  This file is optional.


------------------------------------------------------
--- SOFTWARE INTERFACE -------------------------------
------------------------------------------------------

AstroClean also provides a direct API interface.  This interface
includes a separate function for each filter.  The details are
provided in "astroclean_api.h"
