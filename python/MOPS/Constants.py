"""
Contains useful MOPS constants, which should be exactly the same as their
Perl PS::MOPS::Constants counterparts.  For more information, see the documentation
for the PS::MOPS::Constants Perl module.
"""

__author__ = 'Larry Denneau, Institute for Astronomy, University of Hawaii'
__date__ = '$Date$'
__version__ = '$Revision$'[11:-2]


import math


# Default obscode
DEF_OBSCODE = 807                       # CTIO


# Mathmatical constants.
PI = math.pi
TWOPI = math.pi * 2
DEG_PER_RAD = 180. / PI
RAD_PER_DEG = PI / 180.
ARCSECONDS_PER_DEG = 3600.

# Date handling.
SECONDS_PER_DAY = 86400
MJD2JD_OFFSET = 2400000.5               # convert MJD to JD



# Astrodynamic parameters.  See http://ssd.jpl.nasa.gov/astro_constants.html.
METERS_PER_AU = 1.49597870691e11        # m
GM = 1.32712440018e20                   # m3/s2

# MOPS Observation Cycle.
OC_TREF_0HUT_JD = 2451564.5             # nearest 0h UT prior to 21 JAN 2000 full moon
OC_SYNODIC_PERIOD = 29.53058867



# MOPS parameters/constants.
B62_CHARS = '0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ'
MOPS_NONSYNTHETIC_OBJECT_NAME = 'NS'    # all nonsynth detections use this object name


# Various efficiency-related statuses.
DETECTION_STATUS_UNFOUND = 'X'
DETECTION_STATUS_FOUND = 'F'

DETECTION_STATUS_DESCS = {
    DETECTION_STATUS_UNFOUND : 'UNFOUND',
    DETECTION_STATUS_FOUND : 'FOUND',
}


# Fields.
FIELD_STATUS_INGESTED = 'I'             # inserted into DB
FIELD_STATUS_READY = 'N'                # ready for processing (siblings inserted)
FIELD_STATUS_NEW = FIELD_STATUS_READY   # synonym
FIELD_STATUS_SYNTH = 'D'              # detections (real, ssm) inserted
FIELD_STATUS_TRACKLETS_DONE = 'T'     # ingested, tracklets done
FIELD_STATUS_TRACKLET = FIELD_STATUS_TRACKLETS_DONE    # synonym
FIELD_STATUS_POSTTRACKLET = 'U'       # post-tracklet stamps & digest (PS1 only)
FIELD_STATUS_ATTRIBUTIONS = 'A'       # derived orbits attributed
FIELD_STATUS_LINK1 = 'K'              # slow link pass completed
FIELD_STATUS_LINKDONE = 'L'           # all linking completed
FIELD_STATUS_READYTOLINK = FIELD_STATUS_ATTRIBUTIONS # generalize in case it changes


#TRACKLET_STATUS_UNFOUND = 'X'        # deprecated
TRACKLET_STATUS_UNATTRIBUTED = 'U'
TRACKLET_STATUS_ATTRIBUTED = 'A'
TRACKLET_STATUS_KILLED = 'K'
TRACKLET_STATUS_KNOWN = 'W'           # externally attributed to known object

TRACKLET_STATUS_DESCS = {
    TRACKLET_STATUS_UNATTRIBUTED : 'UNATTRIBUTED',
    TRACKLET_STATUS_ATTRIBUTED : 'ATTRIBUTED',
    TRACKLET_STATUS_KILLED : 'KILLED',
    TRACKLET_STATUS_KNOWN : 'KNOWN',
}


DERIVEDOBJECT_STATUS_NEW = 'I'      # DO processed against orbit identification
DERIVEDOBJECT_STATUS_MERGED = 'M'   # DO was merged with another DO
DERIVEDOBJECT_STATUS_KILLED = 'K'   # DO has been marked as being invalid

DERIVEDOBJECT_STATUS_DESCS = {
    DERIVEDOBJECT_STATUS_NEW : 'NEW',
    DERIVEDOBJECT_STATUS_MERGED : 'MERGED',
    DERIVEDOBJECT_STATUS_KILLED : 'KILLED',
}

# EON queue.  The status name indicates what has *been done* to some object,
# and not what the object is "ready for".
EONQUEUE_STATUS_NEW = 'N'           # DO yet to be run against orbit identification
EONQUEUE_STATUS_IDENTIFIED = 'I'    # DO processed against orbit identification

# Revert when we support reporting.
EONQUEUE_STATUS_PRECOVERED = 'P'    # DO precovery completed
EONQUEUE_STATUS_POSTFIT = 'X'       # postfit resids processed

EONQUEUE_STATUS_REPORTED = 'X'      # DO reported to outside world
EONQUEUE_STATUS_RETIRED = 'X'       # DO was retired from queue mid-processing (e.g., merged with another DO)

# Status progression.
EONQUEUE_STATUS_NEXT = {
    EONQUEUE_STATUS_NEW : EONQUEUE_STATUS_IDENTIFIED,
    EONQUEUE_STATUS_IDENTIFIED : EONQUEUE_STATUS_PRECOVERED,
    EONQUEUE_STATUS_PRECOVERED : EONQUEUE_STATUS_POSTFIT,
    EONQUEUE_STATUS_POSTFIT : EONQUEUE_STATUS_REPORTED,
    EONQUEUE_STATUS_REPORTED : EONQUEUE_STATUS_RETIRED,
    EONQUEUE_STATUS_RETIRED : None
}
# Add when we support reporting.
#    EONQUEUE_STATUS_PRECOVERED : EONQUEUE_STATUS_RETIRED,

EONQUEUE_STATUS_DESCS = {
    EONQUEUE_STATUS_NEW : 'NEW',
    EONQUEUE_STATUS_IDENTIFIED : 'IDENTIFIED',
    EONQUEUE_STATUS_PRECOVERED : 'PRECOVERED',
    EONQUEUE_STATUS_POSTFIT : 'POSTFIT',
    EONQUEUE_STATUS_REPORTED : 'REPORTED',
    EONQUEUE_STATUS_RETIRED : 'RETIRED',
}

# MOPS efficiency values.
MOPS_EFF_UNFOUND = 'X'             # unfound object
MOPS_EFF_CLEAN = 'C'               # all same synthetic object
MOPS_EFF_MIXED = 'M'               # all synthetic, but different objs
MOPS_EFF_BAD = 'B'                 # synth + nonsynth tracklet
MOPS_EFF_NONSYNTHETIC = 'N'        # all nonsynth

MOPS_EFF_DESCS = {
    MOPS_EFF_UNFOUND : 'UNFOUND',
    MOPS_EFF_CLEAN : 'CLEAN',
    MOPS_EFF_MIXED : 'MIXED',
    MOPS_EFF_BAD : 'BAD',
    MOPS_EFF_NONSYNTHETIC : 'NONSYNTHETIC',
}


# Orbit codes.
MOPS_EFF_ORBIT_IODFAIL = 'I'       # orbit failed IOD
MOPS_EFF_ORBIT_DIFFAIL = 'D'       # orbit passed IOD but failed diffcor
MOPS_EFF_ORBIT_FAIL = 'F'          # other failure
MOPS_EFF_ORBIT_REJECTED = 'J'      # good orbit, but rejected by tracklet mgt
MOPS_EFF_ORBIT_RESIDUALS = 'R'     # good orbit, but rejected by tracklet resids analysis
MOPS_EFF_ORBIT_OK = 'Y'            # orbit passed IOD and diffcor

MOPS_EFF_ORBIT_DESCS = {
    MOPS_EFF_ORBIT_IODFAIL : 'IOD FAILED',
    MOPS_EFF_ORBIT_DIFFAIL : 'DIFF FAILED',
    MOPS_EFF_ORBIT_FAIL : 'FAILED',
    MOPS_EFF_ORBIT_REJECTED : 'TRACKLET MGT FAILED',
    MOPS_EFF_ORBIT_RESIDUALS : 'RESIDUALS ANALYSIS FAILED',
    MOPS_EFF_ORBIT_OK : 'OK',
}
