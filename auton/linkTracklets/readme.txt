LinkTracklets is a program to do inter-night linking from observation
files.  The program is currently set up to use pre-specified
intra-night linkages.  This means that observations with the same ID
on the same night will be assumed to correspond to the same object.
The program takes the intra-night linked observations, and forms
tracks (linkings of 2 or more observations).  The tracks are then
pruned by fitting an orbit to the observations in the track and
removing the track if the mean-squared- residual is above some
threshold.  The remaining good tracks are then dumped to files
(described below).

Also included is an evaluation mode.  If evaluation mode is run the
program assumes that the original observation ID fully specifies which
track it belongs to (i.e. the linking could be solved by solely
looking at the ID string).  In this mode the linking is done using the
program and checked using the ID string.  The % correct and % found
are returned for the data set.  This mode should be used to choose
parameters that are correct for a given data set.

What is new in version 3.0.2:
- Fixed a bug in the determination of the number of
  nights on which a track is seen.
- Added the ability to only find tracks with their first/last
  observation in a given window.

What is new in version 3.0.1:
- Fixed a bug with outputting the linked observations 
  with the flattened times and positions.
- The program no longer crashes on an invalid
  file name.

What is new in version 3.0.0 (released 10/5/2005):
- A new variable trees linkage code 
  NOTE: this means all new command line args!

What is new in version 2.1.11:
- Fixed a bug in the calculation of the true
  track size distribution.

What is new in version 2.1.10:
- Changed the evaluation functions.  The full track evaluation
  functions now only consider tracks with >= min_obs AND that have
  observations on >= mindays DIFFERENT nights.

What is new in version 2.1.9:
- Fixed a bug that would use a linear estimate, but quadratic
  prediction if the tracklet had a very short arc.

What is new in version 2.1.8 (released 5/10/05):
- Removed TI coordinates (to save space and time)
- Fixed a bug where overlapping tracklets could be merged.
- Added an option to output the internal track scores

What is new in version 2.1.7:
- Fixed a minor memory leak in data set loads
- Now prevent estimation of quadratic tracks
  from very short arcs.

What is new in version 2.1.6:
- Option to filter out "slow" tracklets
- Option to filter out tracks observed on
  only a few distinct nights.

What is new in version 2.1.5:
- Option to turn on/off bwpass
- Option to filterout the "fast" tracklets
- Fixed a bug with track IDs in the obs result file.
- Changed the plate width default
- New input format supported (see below)
- New output format supported (see below)

What is new in version 2.1.4:
- New score matrices
- Removed unneeded output (dashed lines, memory check, etc.)

What is new in version 2.1.3:
- Added a backward pass to the linkage algorithm.
  This should allow linkTracklets to pickup tracks with
  early gaps.

What is new in version 2.1.2:
- Fixed a bug where program output would not use
  most lowercase letters (a-y).
- Added a bunch of new evaluation statistics.

What is new in version 2.1.1:
- Version information
- Ability to output individual result files without orbit fitting
- Ability to turn off TI coordinate calculations
- Increased compatibility with MPC format (7 digit ids, etc.)

What is new in version 2.1:
- Program NEVER calls gorbit if fit_orbs==FALSE.
- Program now produces a tracking summary file.

What is new in version 2:
- New tracker (full MHT).
- New track merger.
  - More accurate merging
  - Faster Merging
- Support for use of (T,I) coordinates.
- Faster evaluation functions


---- How to install: Create a directory "linkTracklets" and un-tar the
included file into the directory.

----- How to run:

LinkTracklets runs from the command line:

./linkTracklets RUNTYPE file OBS_FILENAME [optional parameters]

The run type specifies the type of algorithm to use (Sequential or vtrees):

- vtree - The new fast vtrees code (default).
- seq   - The previous sequential vtrees code.

The sequential search works by projecting the current track estimate
forward in time, associating it with new tracklets, and refining the
estimate.  The first prediction is done using a linear estimate and
tracklets within "lin_thresh" of the predicted position are included
as matches.  Subsequent predictions are done with the estimated
quadratic and "quad_thresh" is used as the distance threshold.
Finally, tracks are filtered if their fit (i.e. the fit of their
member detections to the estimated track) is worse than "fit_thresh".

The vtrees search works by first finding 2 compatible tracklets from
which to estimate the endpoints of the track.  These two tracklets are
referred to as the "model" tracklets.  The found tracks are then
confirmed by finding additional "support" tracklets.  The initial
search pruning is done with respect to a maximum error bound
"vtree_thresh" (how much the detections are allowed to vary) and
testing for mutual compatibility of a set of points.  This is done
using simplified constraint satisfaction logic.  The support tracklets
are also tested for compatibility by their position relative to the
predicted position of the track estimated from the model tracklets.
They must fall within "pred_thresh" of the predicted track position.
Finally, tracks are filtered if their fit (i.e. the fit of their
member detections to the estimated track) is worse than "fit_thresh".

For example to run using default parameters and the included
sample file, type:

./linkTracklets file ./fake_small2.txt


The optional parameters are:

trackfile  - The filename of the resulting track observation 
             (default = "tracks.obs")

orbitfile  - The filename of the resulting track orbits.
             This file is only created if fit_orbs==TRUE.
             (default = "tracks.orb")

summaryfile  - The filename of the resulting summary file.
               (default = "tracks.sum")

scoresfile   - The filename of the resulting track scores file.
               (default = "" -> do not output)

lin_thresh   - The threshold for saying 2 observations match using a
               linear projection in RA/DEC (default = 0.05).
               NOTE: This command line argument is for runtype = seq ONLY.

quad_thresh   - The threshold for saying 2 observations match using a
                quadratic projection in RA/DEC (default = 0.02).
                NOTE: This command line argument is for runtype = seq ONLY.

fit_thresh   - The threshold for saying 2 observations match using a fit
               quadratic in RA/DEC (default = 0.0001).

pred_thresh  - The threshold for the goodness of fit for the support
               tracklets to the model estimated from the 2 model tracklets.
               For more information see below. (default = 0.0005).
               NOTE: This command line argument is for runtype = vtree ONLY.

vtree_thresh - The MAXIMUM allowed detection position error under a vtree
               search.  (default = 0.0002).
               NOTE: This command line argument is for runtype = vtree ONLY.

maxv         - The maximum speed of a tracklet.  The code ignores any
               tracklets faster than maxv.  The speed is measured in
               degrees per day (default = 1000.0).

minv         - The minimum speed of a tracklet.  The code ignores any
               tracklets slower than minv.  The speed is measured in
               degrees per day (default = 0.0).

acc_r        - The maximum magnitude of acceleration (scalar) in the
               RA direction.  The acceleration is measure in degrees
               per day per day (default = 0.02).

acc_d        - The maximum magnitude of acceleration (scalar) in the
               DEC direction.  The acceleration is measure in degrees
               per day per day (default = 0.02).

start_t_range - If this is set to >0 then it constrains to search to
                only find tracks with their first observation in the
                first 'start_t_range' of the file's time range
                (measured from the earliest observation in the file).
                This is measured in days.  Thus start_t_range=2.0 will
                require that the first observation in a track occurs
                no later than 2 days after the first observation in
                the file.  (default = -1.0).

end_t_range - If this is set to >0 then it constrains to search to
               only find tracks with their last observation in the
               last 'end_t_range' of the file's time range (measured
               from the last observation in the file).  This is
               measured in days.  Thus end_t_range=1.0 will require
               that the last observation in a track occurs no earlier
               than 1 day after the last observation in the file.
               (default = -1.0).



min_sup      - The minimum number of distinct tracklets/nights on which 
               we must observe and object for it to be considered valid.  
               (default = 3)
               NOTE: This argument was previously "mindays" in version 
                     <= 2.1.11

max_hyp      - Maximum number of hypothesis for each observation in 
               the MHT search (default 500).

max_match    - Maximum number of matches for each tracklet at each time
	       during the MHT search (default 500).

min_obs      - Minimum track size to be considered a valid track 
               (default 6).

eval        - A boolean that indicates whether or not to run in evaluation
              mode (default = FALSE).

bwpass      - A boolean that indicates whether or not to do a backward
              tracking pass (default = TRUE).

roc_thresh  - The min percentage of matching observations for a full
              track to be considered correct in the full track scoring
              functions (default = 1.0)

remove_subsets  - A boolean that indicates whether to remove an orbit
                  if its observations are the subset of another orbit.
                  Set to FALSE to keep subset orbits (default = TRUE).

indiv_files     - A boolean that indicates whether or not to dump each
		  of the tracks to individual files.  WARNING:  If this
		  option is selected it will dump the files to the
		  directory ./linkTracklets_indiv_tracks and remove and files
		  that were previously in this directory! (default = FALSE).

remove_overlaps - A boolean that indicates whether to merge two orbits
                  if they overlap at some observations (default = TRUE).

allow_conflicts - A boolean that indicates whether to allow conflicted
		  merges when removing overlaps.  A conflicted merge
		  is any merge where the tracks have two different observations
		  occurring at the same time.  (default = FALSE).
		  Note: if remove_overlaps = FALSE this parameter has no effect.

min_overlap - A real valued parameter [0,1] that indicates the degree
	      of overlap required to merge two tracks.  For example,
	      "min_overlap 0.75" will merge two tracks of length four
	      if and only if they overlap on at least 3 observations.
	      (default = 0.5) Note: if remove_overlaps = FALSE this
	      parameter has no effect.

plate_width - The width in time (days) of a single observation plate.  All
	      observations occurring within plate_width of each other
	      will be flatten to the same time. (default = 0.001)


Note: The default parameters were chosen because the empirically perform 
      well on the spacewatch data.

--------- Example command line:

./linkTracklets vtree file ./fake_small2.txt eval true quad_thresh 0.05 fit_thresh 0.02


--------- Input:

LinkTracklets supports two different input file formats (note that the
program is "smart" and determines the format of the given file from
its structure):

1) Standard MPC format

2) PANSTARRS input format


--------- PANSTARRS Input File Format (from Larry Denneau's Spec):

Each line consists of one detection with the following (white space
separated) information:

ID EPOCH_MJD RA_DEG DEC_DEG MAG OBSCODE OBJECT_NAME

The ID takes the place of the detection ID and is used to form
tracklets (each tracklet is a set of all detections with matching
IDs).  OBJECT_NAME is optional and provides the ground truth
information for the evaluation code (i.e. all detections of the same
asteroid should have the same OBJECT_NAME).

Notes:

* Values will be whitespace-separated (ASCII space or tab characters)

* Ignore lines beginning with '#'

* Fields are never fixed-width.  Some floating-point values may have many digits.

* All time/epoch values should be specified in MJD and have millisecond
precision (1E-8 days).

* All RA and DEC values should be specified in degrees and have a
precision of 1 milliarcsecond (~1E-9 deg).


--------- Output:

linkTracklets produces (upto) 4 output files.

track file - A file of proposed linkages.  Each line is a single
             observation.  The given ID is the unique ID for that
             orbit.

orbit file - A file of proposed orbits.  Each line contains one
             orbit (with the unique orbit ID).  This file is only
             produced if fit_orbs==TRUE.

summary file - A file of proposed linkages.  Each line contains
               ORBIT_ID OBS_ID OBS_NUM.  Where the orbit ID is
               assigned unique ID, the obs ID is the original
               observation ID, and the obs number indicates
               which observation (line of input file) this is.

ids file     - A file of proposed linkages.  Each line contains
               white space separated IDS for the linked objects
               of a single track.

scores file  - A file of proposed track scores.  Each line contains
               one orbit (with the unique orbit ID) and its
	       associated score.  By default this file is not
               output.  You can output this file using
	       scoresfile FILENAME in the arguments.


---------- Score Matrices

The code returns two score matrices.  These matrices are generated by
finding the best matching "learned" track for each true track.  The
score of a match is simple the size of the overlap.  Thus for each
true track we find the single learned track with the highest overlap.

The first matrix shows the percentage of tracks with a given length of
the best matching learned track for a given length of the true track.
Thus M_ij shows the percentage of tracks of length j that matched
tracks of length i.  This score does not say anything about how many
of the observations actually match, just that it is the highest
overlap.

The second matrix shows the percentage of tracks with a given length
of overlap for a given length of the true track.  Thus M_ij shows the
percentage of tracks of length j that matched tracks with an overlap
of size i.  This score does not say anything about how long the actual
matched tracks are.


---------- Files include in the tar:

linkTracklets   - Inter-night linkage software

gorbit          - Compiled orbit fiting software

ominusc2        - Compiled orbit evaluation software

fake_small2.txt - Sample observation file

obsrvtry.dat    - A data file used by gorbit/ominusc2

obsrvtry.xrf    - A data file used by gorbit/ominusc2

readme.txt      - This readme file


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