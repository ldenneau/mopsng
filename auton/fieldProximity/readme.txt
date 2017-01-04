Fieldproximity is a program to quickly determine whether an orbit/track
appears near a field/observation.  The program takes two input files
(described below) that indicate the given fields and orbit/track
positions.  The fields are given by a circular region (center point
and radius).  The tracks/orbits are given by a piecewise linear
approximation (sets of (time,RA,DEC) tuples).  The orbit/region pairs
are checked for "closeness" and the resulting matches returned in a
file.

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

--- Updates ------------------------------------------

Version 1.0.3 (Released 8/7/2005)
 - Program more robust to empty input files.

Version 1.0.2
 - Program now more robust to reading in "bad" input lines.
 - Program now allows multiple field files.
 - Program now allows multiple track files.

Version 1.0.1
 - RA now given in terms of degrees
   (instead of hours).
 - Changed name to field proximity instead of orboccur

--- HOW TO RUN ---------------------------------------

Fieldproximity runs from the command line:

./fieldproximity fieldsfile FIELD_FILENAME tracksfile TRACK_FILENAME [optional parameters]

For example to run using default parameters and the included
sample file, type:

./fieldproximity fieldsfile ./test.fields tracksfile ./test.tracks 

The optional parameters are:

outfile   - The filename for the program output.  
            (default = "result.txt")

thresh    - The threshold for what a "close" field is.  
            This value can effectively be used to increase 
            the width of fields by allowing near misses 
            to count as intersections. This value is given
            in degrees. (default = 0.0001).

method    - Search method (described below).  
            (default = 0) 

fleaf     - Maximum number of fields in a leaf node.  
            (default = 10) Only used for field trees (method 1).

tleaf     - Maximum number of tracks in a leaf node.
            (default = 10) Only used for track trees (method 2).

split_all - The track tree measures distances between two tracks as
            the greatest distance between two tracks.  To accelerate
            tree construction you can instead consider the distance
            between two tracks on a given segment (say the widest
            one).  The code does this by default, but can be forced to
            consider the distance over all segments using split_all.
            Setting to true will allow for tighter bounds, but
            significantly slow down ttree construction. This is used
            only for track trees (METHOD 2).  (default = false)

A note on pleaf and tleaf: This value effectively controls the size of
the tree.  The tree construction algorithms will build the tree by
recursively splitting the set of fields/tracks and creating two
children nodes.  This value indicates a which point it will stop and
consider the node a leaf.  A value of 10 indicates that a leaf node
can contain UP TO 10 children.  The fewer objects in a leaf the more
pruning opportunities there are above it, but the more tests required
if the leaf does not get pruned.

--- EXAMPLE COMMAND LINE ------------------------------------

./fieldproximity fieldsfile ./test.fields tracksfile ./test.tracks method 2 


--- SEARCH METHOD -------------------------------------------

The program has three different search modes (0-2).  All of the search
modes are exact and will return every intersection.  They vary in
their use of data structures, which may allow the program to not
test impossible pairs.

0) Exhaustive - This approach does not build any data structures.
   Instead it does a brute force search over EVERY track/field pair
   testing for an intersection.  This approach is preferably when
   there are only a few fields (<1000) and a few tracks (<1000).

1) Field Tree - This approach uses a KD-tree of fields.  It first
   builds a KD-tree on the fields and then tests all of the tracks for
   intersection using the KD-tree to prune impossible sets of fields.
   This approach is preferable when there are many fields OR when
   there are only a few tracks.

2) Track Tree - This approach uses a ball-tree of tracks/orbits.  It
   first builds a ball-tree on the orbits and then tests all of the
   fields for intersection using the ball-tree to prune impossible
   sets of tracks. This approach is preferable when there are many
   tracks OR when there are only a few fields.  However, it is
   important to note that the construction of the track tree itself
   can be computationally expensive.

Additional factors to consider when choosing a methods is: 

- Number of segments in the track approximation.  The field tree
method's cost is roughly linear in the number of segments in the
approximation.  Thus if the orbits are approximated by >1,000
segments, it may be preferable to use the track tree.  

- Coherence of data.  The track tree is most effective when there are
underlying coherent bundles of tracks that the tree can exploit.



--- INPUT FILES ---------------------------------------------

The input consists of two files:

FIELDS FILE: 
The fields file contains all information on the field
data structures.  It contains EXACTLY one field entry per row of the
form:

FIELDID TIME RA DEC RADIUS

The items are space separated values.  The field id is an arbitrary
length string of letters and numbers.  The remaining values are
arbitrary precession decimal numbers.  RA is given in degrees
[0.0,360.0], DEC is given in degrees [-90.0,90.0] and RADIUS is given
in degrees.

You can use multiple field files in a single run by placing their
names in a comma separated list.  NOTE: The parser stops when it hits
the first white space, so the list must contain NO spaces.  For
example: "fieldfiles test.A,test.B,test.C" will use those three files,
but "fieldfiles test.A, test.B, test.C" will ONLY use the first one.



TRACKS FILE: 
The tracks file contains position samples for all tracks.
These position samples are used to form a linear approximation of the
orbit over the given nights.  Each line consists of a single track
position sample of the form:

TRACK_ID TIME RA DEC

The items are space separated values.  The track id is an arbitrary
length string of letters and numbers.  The remaining values are
arbitrary precession decimal numbers.  RA is given in degrees
[0.0,360.0] and DEC is given in degrees [-90.0,90.0].

The entries for each object do not need to be on adjacent lines (they
are resolved by their ID tags) and do not need to be in temporal
order.  The interpolator assumes "slow" moving tracks, so transitions
of >180.0 in RA are assumed to cross the 0 line.

The one constraint is each object MUST have the same number of samples
at the SAME times.  This greatly simplifies and accelerates tree
construction.

You can use multiple track files in a single run by placing their
names in a comma separated list.  NOTE: The parser stops when it hits
the first white space, so the list must contain NO spaces.  For
example: "trackfiles test.A,test.B,test.C" will use those three files,
but "trackfiles test.A, test.B, test.C" will ONLY use the first one.

The program is "smart" about using multiple files.  It will track IDs
between files allowing us to place a single track in multiple files.
This is important if you want to generate positions all tracks in one
file for each night.  You can then load the multiple files and get all
of the tracks over the nights of interest.



--- OUTPUT FILES ---------------------------------------------

The program outputs a single file.  Each line contains one
intersection and is of the form:

FIELD_ID TRACK_ID FIELD_NUM TRACK_NUM

The FIELD_ID and TRACK_ID are the ids given in the input files.  The
FIELD_NUM is the line of the field input file on which this field
occurs (zero indexed).  The TRACK_NUM indicates that this track was
the TRACK_NUM'th track seen in the track file (zero indexed).
