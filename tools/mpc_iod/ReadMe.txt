A README.TXT FILE
-----------------


INTRODUCTION
------------

Since you're reading this, you've already unpacked the
ZIP file.

You should have two executables:

IOD
SplitUp

and one text file:

IOD.txt

IOD is the orbit calculation program.
SplitUp splits up large observation batches for submission to multiple
CPUs using the Sun Grid Engine queueing system.
IOD.txt is the help file for IOD.

The executables should run on Intel and AMD processors.


CONFIGURING IOD
---------------

IOD needs to know where to find the DE405 file and the help file.

You can tell IOD where these two files are located using one of the
methods below:

1) Via enviroment variables MPCIOD_DATA and MPCIOD_HELP.  E.g.:
    
setenv MPCIOD_DATA "/home/graff/OrbFit/lib/jpleph"
setenv MPCIOD_HELP "/home/graff/PanSTARRS/IOD.txt"

2) Via command line flags.  E.g.:

IOD -defile=/home/graff/OrbFit/lib/jpleph -helpfile=/home/graff/PanSTARRS/IOD.txt ...

3) Via a configuration file.  E.g.:

IOD -config=IOD.config ...

where IOD.config contains:

DEFILE=/home/graff/OrbFit/lib/jpleph
HELPFILE=/home/graff/PanSTARRS/IOD.txt

It is recommended that you use the environment variable method.


USING IOD
---------

IOD takes in a stream of observations via standard input and writes out
a stream of orbits to standard output.

It is recommended you examine the help file before attempting to run IOD:

IOD -help

A minimal run of IOD can be achieved as follows (assuming IOD has been
configured using environment variables):

IOD < obs.txt > orb.txt

where obs.txt is a file of observations and orb.txt receives the generated
orbits.

You can specify how hard IOD will try to get an orbits using -quitpass.
If you omit -quitpass, IOD will try very hard and it will take a long
time to run.

It is STRONGLY recommended that you use:

IOD -quitpass=3 < obs.txt > orb.txt

when dealing with realistic observation files containing a mix of
correctly-linked, mislinked and false-detection objects.  -quitpass=3
is a good trade-off between a high success rate for correctly-linked
real objects and a low false-postive rate for incorrectly-linked objects.

If you are running only correctly-linked real objects, you can use
-quitpass=4 or omit quitpass altogether for testing purposes.


USING SplitUp
-------------

SplitUp allows you to split up an observation file into many smaller
observation files.  It has flags to enable you to be running many
observation batches from the same directory.  It reads an observation
stream from standard input and writes out a series of observation files
with a name chosen by the user:

Splitup -root=TS -numfiles=30 < obs.txt

will split obs.txt into 30 files named TS0001-TS0030, as well as
creating a shell-script file Submit.sh which can be run to submit
the observation batches for processing (there will be files named
Submit_TS0001.sh through Submit_TS0030.sh).

If you have a large number of CPUs available (say, more than 50), you
are advised to use the -numobs flag to tell SplitUp how many observations
it is to expect.  This avoids a problem with having too many output files
open at the same time.

Splitup -root=BigBatch -numobs=5134534 -numfiles=500 < obs.txt

would splitup obs.txt into 500 files, each containing ~10000 observations
(BigBatch_0001.obs to BigBatch_0500.obs).


PROBLEMS?
---------

E-mail full details of any problems to gwilliams@cfa.harvard.edu.
