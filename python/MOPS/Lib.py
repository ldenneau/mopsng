"""
Contains useful MOPS routines, based on Perl PS::MOPS::Lib counterparts.
For more information, see the documentation for the PS::MOPS::Lib
Perl module.
"""
from __future__ import division

__author__ = 'Larry Denneau, Institute for Astronomy, University of Hawaii'
__date__ = '$Date$'
__version__ = '$Revision$'[11:-2]


import os
import math
import time
import slalib
from MOPS.Constants import *


SQRT2_DIV_2 = math.sqrt(2) / 2

def base60_to_decimal(xyz,delimiter=None):
    """Decimal value from numbers in sexagesimal system. 

    The input value can be either a floating point number or a string
    such as "hh mm ss.ss" or "dd mm ss.ss". Delimiters other than " "
    can be specified using the keyword ``delimiter``.
    """
    divisors = [1,60.0,3600.0]
    xyzlist = str(xyz).split(delimiter)
    sign = -1 if xyzlist[0].find("-") != -1 else 1
    xyzlist = [abs(float(x)) for x in xyzlist]
    decimal_value = 0 

    for i,j in zip(xyzlist,divisors): # if xyzlist has <3 values then
                                      # divisors gets clipped.
        decimal_value += i/j

    decimal_value = -decimal_value if sign == -1 else decimal_value
    return decimal_value

def decimal_to_base60(deci,precision=1e-8):
    """Converts decimal number into sexagesimal number parts. 

    ``deci`` is the decimal number to be converted. ``precision`` is how
    close the multiple of 60 and 3600, for example minutes and seconds,
    are to 60.0 before they are rounded to the higher quantity, for
    example hours and minutes.
    """
    sign = "+" # simple putting sign back at end gives errors for small
               # deg. This is because -00 is 00 and hence ``format``,
               # that constructs the delimited string will not add '-'
               # sign. So, carry it as a character.
    if deci < 0:
        deci = abs(deci)
        sign = "-" 

    frac1, num = math.modf(deci)
    num = int(num) # hours/degrees is integer valued but type is float
    frac2, frac1 = math.modf(frac1*60.0)
    frac1 = int(frac1) # minutes is integer valued but type is float
    frac2 *= 60.0 # number of seconds between 0 and 60 

    # Keep seconds and minutes in [0 - 60.0000)
    if abs(frac2 - 60.0) < precision:
        frac2 = 0.0
        frac1 += 1
    if abs(frac1 - 60.0) < precision:
        frac1 = 0.0
        num += 1 

    return (sign,num,frac1,frac2)

def gd2jd(year,month,day,hour=12,minute=0,second=0):
    """
    Converts a Gregorian calendar date to a Julian date.
    """
    year, month, day, hour, minute =\
        int(year),int(month),int(day),int(hour),int(minute)

    if month <= 2:
        month +=12
        year -= 1 

    modf = math.modf
    # Julian calendar on or before 1582 October 4 and Gregorian calendar
    # afterwards.
    if ((10000L*year+100L*month+day) <= 15821004L):
        b = -2 + int(modf((year+4716)/4)[1]) - 1179
    else:
        b = int(modf(year/400)[1])-int(modf(year/100)[1])+\
            int(modf(year/4)[1]) 

    mjdmidnight = 365L*year - 679004L + b + int(30.6001*(month+1)) + day

    fracofday = base60_to_decimal(\
        " ".join([str(hour),str(minute),str(second)])) / 24.0 

    return MJD2JD_OFFSET + mjdmidnight + fracofday
  
def jd2mjd(jd):
    """
    Convert JD to MJD; just subtract offset.
    """
    return(jd - MJD2JD_OFFSET)


def mjd2jd(mjd):
    """
    Convert MJD to JD; just add offset.
    """
    return(mjd + MJD2JD_OFFSET)


def mjd2nn(mjd, ut2local_hours):
    """
    Convert floating-point MJD to local night number.
    """
    return int(mjd - 0.5 + ut2local_hours / 24.0);


def nn2mjd(nn, ut2local_hours):
    """
    Convert integer local night number to MJD.  The range of
    time from MJD to MJD+1 is one local noon to the next.
    """
    return nn + 0.5 - ut2local_hours / 24.0;


def jd2ocnum(jd):
    """
    Convert a JD to a MOPS Observing Cycle number.
    """
    return(round((((int(jd - 0.5) + 0.5) - OC_TREF_0HUT_JD) / \
                  OC_SYNODIC_PERIOD) - 0.49999999999))


def ocnum2jd(ocnum):
    """
    Convert an integer OC number to the JD at which the OC begins. Note that OC
    number is defined to change at UT=0h.
    """
    ocnum = int(ocnum)     # only accept integer OC nums
    
    # time of full moon
    full_moon_jd = OC_TREF_0HUT_JD + OC_SYNODIC_PERIOD * ocnum
    return(int(full_moon_jd))


def ocnum2mjd(ocnum):
    return(jd2mjd(ocnum2jd(ocnum)))


def mjd2ocnum(mjd):
    """
    Convert an MJD to a MOPS Observing Cycle number.
    """
    return(jd2ocnum(mjd2jd(mjd)))


def getOCNum(mjd):
    """
    Return the OC number of the date specified.
    """
    return(mjd2ocnum(mjd))


# Filter magnitude corrections. Reference: email from Spahr.
_v2filt = {
    'g' :  0.01, 
    'r' :  0.23,
    'i' :  1.22,
    'z' :  0.70,           
    'U' :  1.10,
    'J' :  1.10,
    ' ' : -0.80, 
    'B' : -0.80, 
    'R' :  0.40,
    'I' :  0.80,
    'C' :  0.40,
}

# Corrections from PS1/Sloan AB mags to Johnson.
_AB2filt = {
    'u' : 0.981,
    'g' : -0.093,
    'r' : 0.166,
    'i' : 0.397,
    'z' : 0.572,
    'y' : 0.0,
}

# We now have prelimiary filter corrections from PS1 mags to V based on
# work from Price, Grav, Granvik and Fitzsimmons.   E.g., given a reported 
# g mag, the corresponding V is g - 0.5.
_ps1v2filt = {
    'g' : -0.5,
    'r' : -0.1,
    'i' : +0.3,
    'z' : -0.1,
    'y' : 0.0,
    'w' : -0.5,        # same as g for now
}


def v2filt(mag, filt, cvt=None):
    """
    Convert specified mag to V-band.
    """
    if cvt is None:
        cvt = _ps1v2filt
    return mag - cvt.get(filt, 0.0)

def filt2v(mag, filt, cvt=None):
    """
    Correct specified V-band mag for filter.
    """
    if cvt is None:
        cvt = _ps1v2filt
    return mag + cvt.get(filt, 0.0)

def classifyNames(names):
    """ 
    Walk the list and update a dict with keys as follows:
    {
      object_name_A : count
      object_name_B : count
      NS : count
    }

    Then: 

    1. If there is only NS, then return MOPS_EFF_NONSYNTHETIC
    2. If there is NS and other, return MOPS_EFF_BAD.
    3. If there are multiple other, return MOPS_EFF_MIXED
    4. Return MOPS_EFF_CLEAN.
    """
    
    have_synth = False
    have_nonsynth = False
    tabulationizer = {}
    for name in names:
        if name in (None, '', 'NS'):
            have_nonsynth = True
        else:
            have_synth = True
            tabulationizer[name] = 1
        # <-- if
    # <-- for

    if have_nonsynth and not have_synth:
        return MOPS_EFF_NONSYNTHETIC
    elif not have_nonsynth and have_synth:
        if len(tabulationizer.keys()) > 1:
            return MOPS_EFF_MIXED
        else:
            return MOPS_EFF_CLEAN
    else:
        return MOPS_EFF_BAD     # some mixture of synth+nonsynth
    # <-- if


def sphericalDistance_arcsec(pos1, pos2):
    """
    XXX appears OK to retire this routine.

    Compute the spherical distance in arcsec between pos1 and pos2.

    posN=[ra, dec]
    ra, dec in decimal degrees.

    This is approximate!!!  
    XXX LD We should really use slalib, but it's a PITA.
    """
    d = math.sqrt((pos1[1] - pos2[1])**2 +
                  ((pos1[0] - pos2[0]) * math.cos(pos1[1] / DEG_PER_RAD))**2)
    return d * ARCSECONDS_PER_DEG


def  waitForFiles(fileNames, polling_time=0.1, timeout=60):
    """
    Wait for fileNames and check to see if they are there every polling_time 
    seconds. If the files do not show up within timeout seconds, then give up.
    """
    t0 = time.time()
    while(fileNames):
        fileNames = [f for f in fileNames if not os.path.exists(f)]
        time.sleep(polling_time)
        if(time.time() - t0 > timeout):
            break
        # <-- end if
    # <-- end if
    return


def _fold(ang_deg):
    """
    Fold into [0,360)
    """
    if ang_deg > 180:
        ang_deg -= (int(ang_deg / 360.0 + 0.5)) * 360
    elif ang_deg < -180:
        ang_deg += (int(-ang_deg / 360.0 + 0.5)) * 360
    return ang_deg


def dang(a1_deg, a2_deg):
    """
    Calculate the difference ($a1 - $a2) between two angles, handling
    wraparound at 0/360.  Inputs and outputs are degrees.  Angles are folded
    into (-180, 180) before calculating difference.  If the difference is
    larger than 180 degrees, 360 is subtracted from it.  If the difference is
    less than -180 degrees, 360 is added to it.  
    """

    # Fold out-of-range values.
    a1_deg, a2_deg = map(_fold, [a1_deg, a2_deg]);
    delta_deg = a1_deg - a2_deg

    # Get difference between values and re-fold.
    if delta_deg > 180:
        delta_deg -= 360
    elif delta_deg < -180:
        delta_deg += 360

    return delta_deg


def _fmod(a, b):
    """
    Return the remainder of a divided by b.  If a is less than
    zero, return the negative of the remainder of -a divided by b.
    """
    if (a < 0):
        div = (-a - int(-a / b) * b)
        if div != 0:
            return b - div
        else:
            return 0
    else:
        return a - int(a / b) * b


def normalizeRADEC(ra_deg, dec_deg):
    """
    Normalize a given RA/DEC by wrapping the RA in [0, 360) and the DEC in
    [-90, 90].  If the DEC exceeds 90 or -90 then correct the RA
    appropriately.
    """

    # First fold DEC into (-180, 180).  If the dec is gerater than +90
    #or less than -90 the RA changes by 180.
    dec_deg = _fmod(dec_deg + 180, 360) - 180;
    if dec_deg > 90:
        dec_deg = 180 - dec_deg
        ra_deg += 180
    elif dec_deg < -90:
        dec_deg = -180 - dec_deg
        ra_deg += 180

    return (_fmod(ra_deg, 360), dec_deg)


def classifyDetections(dbh, det_list):
    """
    Given some detections that originated from an LSD tracklet search,
    return a MOPS efficiency classification for the set of detections,
    including an SSM ID if possible.  This routine will normally be
    used in the trackelt upgrade procedure, where we need to assign
    a MOPS efficiency classification to a new tracklet; this is usually
    done in Perl-land when tracklets are created.

    These detections have partial information, since they are represented
    skeletally in the LSD archives.  So we actually need to query the DB
    regarding object name and such.  All we have are field_id, det_id (maybe)
    and det_num.

    We know det_num => nonsynthetic, so the only cases we have to
    worry about are

    * det_num and !det_num => nonsynth + synth => MOPS_EFF_BAD
    * all have det_num => MOPS_EFF_NONSYNTHETIC
    * all do NOT have det_num => MOPS_EFF_CLEAN or MOPS_EFF_MIXED; have to
      fetch and examine ssm_id
    """

    classification = None
    ssm_id = None

    # Get SSM IDs if possible.
    try:
        cursor = dbh.cursor()
        sql = 'select s.ssm_id, d.object_name from detections d join ssm s using (object_name) where det_id=%s'
        for det in det_list:
            if det._id:
                cursor.execute(sql, (det._id,))
                res = cursor.fetchone()
                if res:
                    det.ssmId = int(res[0])         # populate ssm_id
                    det.objectName = res[1]         # populate ssm_id
                # <- if res
            # <- if det._id
        # <- for det
    except Exception, e:
        print "classifyDetections fail: ", [d._id for d in det_list]
        raise(e)
    # <- try/except

    # Now scan the list and determine our classification.
    have_synth = False
    have_nonsynth = False
    ssm_id_map = {}
    for det in det_list:
        if det.detNum:
            have_nonsynth = True
        else:
            have_synth = True
            if det.ssmId is not None:
                ssm_id_map[det.ssmId] = 1
            else:
                # OK, the only way we can putatively get here is if there is a data error, e.g.
                # a det with no detNum but also without an ssmID.  Unfortunately, blind synthetic
                # detections behave this way.  So we have a tradeoff of allowing blind synth
                # testing, or checking for errors.  For now we will allow this condition to occur
                # and assume blind synths.
                have_nonsynth = True

                # raise RuntimeError('got synth detection but no ssm ID')
            # <- if det.ssmId
        # <- if det.detNum
    # <- for det
    if have_nonsynth and not have_synth:
        classification = MOPS_EFF_NONSYNTHETIC
    elif not have_nonsynth and have_synth:
        # All synth
        if len(ssm_id_map.keys()) > 1:
            classification = MOPS_EFF_MIXED
        else:
            classification = MOPS_EFF_CLEAN
            ssm_id = (ssm_id_map.keys())[0]
    else:
        # Have both, must be bad
        classification = MOPS_EFF_BAD
    # <- if have_nonsynth

    # Check
    if classification is None:
        raise RuntimeError("didn't get a classification, weird")

    return classification, ssm_id


def validOrbit(orbit):
    """
    Perform a sanity check on the orbit, return a true value if the
    orbit passes muster.  We use this routine to reject orbits that are
    non-physical or lie outside what can be handled in the MOPS phase space
    (usually very-high eccentricity orbits).
    """

    if orbit.q <= 0 or orbit.e < 0 \
            or orbit.i > 180 or orbit.i < 0 \
            or (orbit.e > 10 and orbit.e / orbit.q > 1e7) \
            or orbit.node < 0 or orbit.node > 360 \
            or orbit.argPeri < 0 or orbit.argPeri > 360:
        return False
    return True


def formatTimingMsg(subsystem, nn, time_sec, subsubsystem=None, comment=None):
    # Format a timing message for inclusion in the log file using a standard format:
    #   SUBSYS/SUBSUBSYS TIME_SEC NN # COMMENT
    # where SUBSUBSYS and COMMENT are optional
    if not nn:
        nn = 0

    msg = 'TIMING ' + subsystem
    if subsubsystem:
        msg += '/' + subsubsystem
    msg += ' ' + ("%.3f" % time_sec) + ' ' + str(nn)
    if comment:
        msg += ' # ' + comment
    return msg


def computeXYIdx(field, ra_deg, dec_deg, fov_deg=None):
    """
    Given a field and detection, compute an integer index describing the X/Y
    numbered position of a rectangle on the field's tangent-plane projection
    where the detection can be found.  The rectangle will always be oriented
    toward the north pole.  If the field center is exactly on the north pole,
    orient the field so that "up" is along the 12h RA line, and along the
    0h line for the south pole.
    """

    if fov_deg:
        fov_rad = fov_deg * RAD_PER_DEG
    else:
        fov_rad = field.FOV_deg * RAD_PER_DEG

    half_fov_rad = fov_rad / 2
    size = field.xyidxSize

    xi_rad, eta_rad, j = slalib.sla_ds2tp(
        ra_deg * RAD_PER_DEG,
        dec_deg * RAD_PER_DEG,
        field.ra * RAD_PER_DEG,
        field.dec * RAD_PER_DEG
    )

    if j != 0:
        return int(size * size / 2)     # center bin, on field center

    xb = int((xi_rad + half_fov_rad) / fov_rad * size)
    yb = int((eta_rad + half_fov_rad) / fov_rad * size)
    if xb >= size:
        xb = size - 1
    elif xb < 0:
        xb = 0
    if yb >= size:
        yb = size - 1
    elif yb < 0:
        yb = 0

    return yb * size + xb


def inField(field, field_shape, ra_deg, dec_deg, pos_radius_deg):
    """
    Return True if the speicifed RA/Dec position falls inside a circle
    circumscribing the specified field whose radius is the nominal field
    radius + pos_radius_deg (which is normally the uncertainty radius
    for the detection).
    """

    if field_shape == 'circle':
        dist = slalib.sla_dsep(
            field.ra * RAD_PER_DEG,
            field.dec * RAD_PER_DEG,
            ra_deg * RAD_PER_DEG,
            dec_deg * RAD_PER_DEG,
        ) 
        return dist < (field.FOV_deg / 2 + pos_radius_deg) * RAD_PER_DEG 
    elif field_shape == 'square':
        return True
    else:
        raise RuntimeError("unknown field shape: " + field_shape)

    return True


def calculateDCriterion(orb1, orb2):
    """
    Calculate 3- and 4-element D-critera from two input orbits.  Reference:
    Jedicke's D-criterion email, Southworth and Hawkins (1963), Drummond
    (2000).
    """

    i1_rad = orb1.i * RAD_PER_DEG
    i2_rad = orb2.i * RAD_PER_DEG
    omega1_rad = orb1.node * RAD_PER_DEG
    omega2_rad = orb2.node * RAD_PER_DEG

    d1 = (orb1.q - orb2.q)
    d1 *= d1

    d2 = (orb1.e - orb2.e)
    d2 *= d2

    # Handle roundoff errors that can cause the acos() arg to be > 1 or < -1.
    arg = math.cos(i1_rad) * math.cos(i2_rad) + \
        math.sin(i1_rad) * math.sin(i2_rad) * math.cos(omega1_rad - omega2_rad)
    if arg > 1:
        arg = 1
    elif arg < -1:
        arg = -1
    I_rad = math.acos(arg)
    # <- if

    II_rad = (orb1.argPeri - orb2.argPeri) * RAD_PER_DEG
    arg = math.cos((i1_rad + i2_rad) / 2) \
            * math.sin((omega1_rad - omega2_rad) / 2) \
            * (1 / math.cos(I_rad / 2))
    if arg > 1:
        arg = 1
    elif arg < -1:
        arg = -1
    II_rad = (orb1.argPeri - orb2.argPeri) * RAD_PER_DEG \
            + 2 * math.asin(arg)

    d3 = 2 * math.sin(I_rad / 2)
    d3 *= d3

    d4 = (orb1.e + orb2.e) * math.sin(II_rad / 2)
    d4 *= d4

    D3 = math.sqrt(d1 + d2 + d3)
    D4 = math.sqrt(d1 + d2 + d3 + d4)

    return D3, D4


# Support for point-near-un certainty ellipse code.
def _make_transform(ra, dec, bear):
    R = slalib.sla_deuler(
        'zyx', 
        ra,
        -dec,
        math.pi / 2 - bear,
    ) 
    return R


def _transform(R, ra, dec):
    v = slalib.sla_dcs2c(ra, dec)               # cvt to cart
    rot_v = slalib.sla_dmxv(R, v)               # rot
    new_ra, new_dec = slalib.sla_dcc2s(rot_v)   # back to sph
    return new_ra, new_dec


class UncertaintyTester(object):
    def __init__(self, ra_deg, dec_deg, smaa_deg, smia_deg, bear_deg):
        self.ra_deg = ra_deg
        self.dec_deg = dec_deg
        self.smaa_deg = smaa_deg
        self.smia_deg = smia_deg
        self.bear_deg = bear_deg
        self.R = _make_transform(self.ra_deg / DEG_PER_RAD, self.dec_deg / DEG_PER_RAD, self.bear_deg / DEG_PER_RAD)

    def PointInEllipse(self, ra_deg, dec_deg, padding_deg=0):
        if self.smaa_deg == 0 or self.smia_deg == 0:
            return False

        new_ra, new_dec = _transform(self.R, ra_deg / DEG_PER_RAD, dec_deg / DEG_PER_RAD)
        new_ra = new_ra * DEG_PER_RAD
        new_dec = new_dec * DEG_PER_RAD
        a2 = (self.smaa_deg + padding_deg) ** 2
        b2 = (self.smia_deg + padding_deg) ** 2
        dist2 = new_ra * new_ra / a2 + new_dec * new_dec / b2
        return dist2 <= 1
