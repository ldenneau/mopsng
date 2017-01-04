#!/usr/bin/env python
"""
precov_test.py

Introduction
Gven a failed precovery MOPS database, identify SSM objects, tracklets,
detections which should have been linked into DerivedObject orbits but where
not.


Observing Strategy
Setup a (no noise) simulation with fields from only one month (say Nov. 2006).
Add a single night from a previous month (say Oct. 2006).
Choose a single region on the sky (e.g. opposition high).

In my case the setup was as follows:
+-------------------------------------------+
|OC Num | Epoch (MJD) | Date       | Fields |
|83     | 54024       | 2006-10-16 | 660    |
|       |             |            |        |
|84     | 54053       | 2006-11-14 | 660    |
|84     | 54060       | 2006-11-21 | 660    |
|84     | 54064       | 2006-11-25 | 660    |
+-------------------------------------------+

Run
    shell> dtctl --finish
    shell> lodctl --finish
To build tracklets, tracks and orbits from the data in the database. What will
happen is that the three nights from the second month (Nov. 2006 in the example
above) will produce DerivedObjects. On the other hand, data from the single
night (in the first month) will produce tracklets only (and not DerivedObjects).

Now run
    shel> precov --finish
To try and connect tracklets from the one night from the first month with orbits
derived from the three nights of the second month.


Efficiency Measurement
Once PS-MOPS is done with the precovery stage (i.e. precov --finish), extract
from the database the list of detections which
    1. Belong to the first night (i.e. MJD 54024 in the example above).
    2. Have magnitude <= limiting_mag (as defined in master.cf).
    3. Belong to a tracklet.
    4. Are associated to a SSM object for which we have a DerivedObject (from
       the second month).
and whose tracklets (which belong to the first night)
    1. Are not associated to any DerivedObject.

For each of those detections, compute their distance from the ephemeris of the
corresponding DerivedOject orbit (i.e. same SSM) at the time of detection. Plot
those distances in a histogram.
"""
import math
import socket

import MySQLdb



# Constants
DB_USER = 'mops'
DB_PASSWD = 'mops'
DB_HOST = 'po12.ifa.hawaii.edu'
LIMITING_MAG = 24.

# Ephemd server.
EPHEMD_HOST = 'mopsdc.ifa.hawaii.edu'
EPHEMD_PORT = 60666
# FIXME: Get the bscode from the command line.
OBSCODE = 568



# Classes
# TODO: Use decorators like in efficiency.py
class Detection(object):
    def __init__(self,
                 det_id,
                 field_id,
                 tracklet_id,
                 ssm_id,
                 ra_deg,
                 dec_deg,
                 epoch_mjd,
                 mag):
        self.det_id = int(det_id)
        self.field_id = int(field_id)
        self.tracklet_id = int(tracklet_id)
        self.ssm_id = int(ssm_id)
        self.ra_deg = float(ra_deg)
        self.dec_deg = float(dec_deg)
        self.epoch_mjd = float(epoch_mjd)
        self.mag = float(mag)
        return

class Orbit(object):
    def __init__(self,
                 orbit_id,
                 object_name,
                 ssm_id,
                 q,
                 e,
                 i,
                 node,
                 arg_peri,
                 time_peri,
                 epoch,
                 h_v,
                 residuals):
        self.orbit_id = int(orbit_id)
        self.object_name = object_name
        self.ssm_id = int(ssm_id)
        self.q = float(q)
        self.e = float(e)
        self.i = float(i)
        self.node = float(node)
        self.arg_peri = float(arg_peri)
        self.time_peri = float(time_peri)
        self.epoch = int(epoch)
        self.h_v = float(h_v)
        self.residuals = float(residuals)
        return

class Distance(object):
    """
    Describe the Orbit-Detection distance and hold information about the Orbit
    and the Detection the distance measure refers to.
    """
    def __init__(self, detection, orbit, distance):
        # TODO: make this class a bit less trivial!
        self.detection = detection
        self.orbit = orbit
        self.distance = distance
        return

    def __repr__(self):
        return('%.04f  %d  %s  %d  %d  %d' % (self.distance,
                                              self.orbit.ssm_id,
                                              self.orbit.object_name,
                                              self.detection.det_id,
                                              self.detection.tracklet_id,
                                              self.detection.field_id))


# Routines
def getFirstNightMJD(dbc):
    """
    Return the lowest MJD from the fields table rounded to the nearest int.
    """
    mjd = None

    sql = 'select min(epoch_mjd) from `fields`'
    n = dbc.execute(sql)
    if(n):
        mjd = int(round(float(dbc.fetchone()[0])))
    return(mjd)

def getOrphanDetections(dbc, limitingMag):
    """
    Query @param database for detections which
      1. Have magnitude <= limitingMag.
      2. Belong to a tracklet.
      3. Are associated to a SSM object for which we have a DerivedObject.
      4. Belong to the first night (See FIXME below).
   and whose tracklets
      1. Are not associated to any DerivedObject.
    """
    detections = []

    # FIXME: Generalize to the case where the single night is not the first one.
    firstNight = getFirstNightMJD(dbc)

    # Identify orphan detections.
    sql = """select det.det_id,
                    det.field_id,
                    ta.tracklet_id,
                    s.ssm_id,
                    det.ra_deg,
                    det.dec_deg,
                    det.epoch_mjd,
                    det.mag
             from detections det,
                  tracklet_attrib ta,
                  derivedobjects d,
                  ssm s
             where  det.mag <= %f and
                    det.epoch_mjd between %d and %d and
                    det.object_name = s.object_name and
                    s.ssm_id = d.ssm_id and
                    det.det_id = ta.det_id and
                    ta.tracklet_id not in
                        (select tracklet_id from derivedobject_attrib)"""
    n = dbc.execute(sql % (limitingMag, firstNight - 1, firstNight + 1))
    if(n):
        res = dbc.fetchall()
        for row in res:
            try:
                detections.append(Detection(*row))
            except:
                print('------')
                print(row)
                print('------')


    for d in detections:
        if(d.tracklet_id == 73):
            print((d.ra_deg, d.dec_deg, d.epoch_mjd))

    return(detections)

def getRefDetections(dbc, limitingMag):
    """
    Query @param database for detections which
      1. Have magnitude <= limitingMag.
      2. Belong to a tracklet.
      3. Are associated to a SSM object for which we have a DerivedObject.
      4. Belong to the first night (See FIXME below).
   and whose tracklets
      1. *Are* associated to a DerivedObject.
    """
    detections = []

    # FIXME: Generalize to the case where the single night is not the first one.
    firstNight = getFirstNightMJD(dbc)

    # Identify orphan detections.
    sql = """select det.det_id,
                    det.field_id,
                    ta.tracklet_id,
                    s.ssm_id,
                    det.ra_deg,
                    det.dec_deg,
                    det.epoch_mjd,
                    det.mag
             from detections det,
                  tracklet_attrib ta,
                  derivedobjects d,
                  ssm s
             where  det.mag <= %f and
                    det.epoch_mjd between %d and %d and
                    det.object_name = s.object_name and
                    s.ssm_id = d.ssm_id and
                    det.det_id = ta.det_id and
                    ta.tracklet_id in
                        (select tracklet_id from derivedobject_attrib)"""
    n = dbc.execute(sql % (limitingMag, firstNight - 1, firstNight + 1))
    if(n):
        res = dbc.fetchall()
        for row in res:
            try:
                detections.append(Detection(*row))
            except:
                print('------')
                print(row)
                print('------')
    return(detections)

def getAllAvaiableDetections(dbc, limitingMag):
    """
    Query @param database for detections which
      1. Have magnitude <= limitingMag.
      2. Belong to a tracklet.
      3. Are associated to a SSM object for which we have a DerivedObject.
      4. Belong to the first night (See FIXME below).
    """
    detections = []

    # FIXME: Generalize to the case where the single night is not the first one.
    firstNight = getFirstNightMJD(dbc)

    # Identify orphan detections.
    sql = """select det.det_id,
                    det.field_id,
                    s.ssm_id,
                    det.ra_deg,
                    det.dec_deg,
                    det.epoch_mjd,
                    det.mag
             from detections det,
                  tracklet_attrib ta,
                  derivedobjects d,
                  ssm s
             where  det.mag <= %f and
                    det.epoch_mjd between %d and %d and
                    det.object_name = s.object_name and
                    s.ssm_id = d.ssm_id and
                    det.det_id = ta.det_id"""
    n = dbc.execute(sql % (limitingMag, firstNight - 1, firstNight + 1))
    if(n):
        res = dbc.fetchall()
        for row in res:
            try:
                detections.append(Detection(*row))
            except:
                print('------')
                print(row)
                print('------')
    return(detections)


def getOrbit(dbc, ssm_id):
    """
    Query the orbits table (joined with derivedobjects) to retrieve the orbital
    parameters of the orbit associated with the SSM object ssm_id.
    """
    orbit = None

    sql = """select o.orbit_id,
                    d.object_name,
                    d.ssm_id,
                    o.q,
                    o.e,
                    o.i,
                    o.node,
                    o.arg_peri,
                    o.time_peri,
                    o.epoch,
                    o.h_v,
                    o.residual
             from orbits o,
                  derivedobjects d
             where d.ssm_id = %d and
                   d.orbit_id = o.orbit_id"""
    n = dbc.execute(sql % (ssm_id))
    if(n):
        # TODO: Handle the case where we have >1 orbits.
        res = dbc.fetchone()
        orbit = Orbit(*res)
    else:
        print('No orbit for SSM(%d).' % (ssm_id))
    return(orbit)

def computeDistance(detection, orbit):
    """
    Given a Detection and an Orbit, compute the distance between the ephemeris
    for the orbit at the time of Detection and the Detection itself.
    """
    (ra, dec) = getOrbitPosition(orbit, detection.epoch_mjd)
    d = sphericalDistance((detection.ra_deg, detection.dec_deg),
                          (ra, dec))
    return(d)

def computeDistances(detections, orbits):
    """
    Just like distance() above, but in batch mode. The only caveat is that it
    could happen that the orbit corresponding to the same SSM onject as one
    detection was not found. In that (RARE) case, ornits has a None.

    Return a list of Detection instances.
    """
    # Prune bad orbits and the corresponding detections.
    dets = []
    orbs = []
    for i in range(len(detections)):
        if(detections[i] != None and orbits[i] != None):
            dets.append(detections[i])
            orbs.append(orbits[i])
    positions = getOrbitPositions(orbs, dets)
    return(sphericalDistances(dets, positions, orbits))


def getOrbitPosition(orbit, mjd):
    """
    Compute ephemeis for @param orbit at the time @param mjd.

    The cumputation of ephemeris uses the ephemd server, which is assumed to be
    up and running on EPHEMD_HOST on port EPHEMD_PORT.

    Return RA, Dec in decimal degrees.
    """
    ra = None
    dec = None

    # TODO: refactor this into a class (using connection pooling).
    # Connect to the ephem server and send the request.
    request = '%f %f %f %f %f %f %f %f %f %d %d\n' % (orbit.q,
                                                      orbit.e,
                                                      orbit.i,
                                                      orbit.node,
                                                      orbit.arg_peri,
                                                      orbit.time_peri,
                                                      orbit.h_v,
                                                      orbit.epoch,
                                                      mjd,
                                                      OBSCODE,
                                                      orbit.orbit_id)
    skt = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    skt.connect((EPHEMD_HOST, EPHEMD_PORT))
    skt.send(request)
    res = skt.recv(4096)
    skt.close()

    # Parse the response from the server (error messages start with 'E').
    if(res[0] == 'E'):
        print('Warning: ephemeris computation for Orbit(%d) failed.' \
              % (orbit.orbit_id))
        retunr(None, None)
    try:
        # Fetch the first line only.
        res = res.split('\n')[0]
        (orbit_id, epoch, ra, dec, mag, orientation, length) = res.split()
    except:
        print('Warning: parsing of results for Orbit(%d) failed.' \
              % (orbit.orbit_id))
        print('         got: "%s"' % (res))
    return(float(ra), float(dec))

def getOrbitPositions(orbits, detections):
    """
    Like getOrbitPosition, but in batch mode.
    """
    ra = None
    dec = None
    results = []

    skt = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    skt.connect((EPHEMD_HOST, EPHEMD_PORT))
    for i in range(len(orbits)):
        # TODO: refactor this into a class (using connection pooling).
        # Connect to the ephem server and send the request.
        mjd = detections[i].epoch_mjd
        request = '%f %f %f %f %f %f %f %f %f %d %d\n' % (orbits[i].q,
                                                          orbits[i].e,
                                                          orbits[i].i,
                                                          orbits[i].node,
                                                          orbits[i].arg_peri,
                                                          orbits[i].time_peri,
                                                          orbits[i].h_v,
                                                          orbits[i].epoch,
                                                          mjd,
                                                          OBSCODE,
                                                          orbits[i].orbit_id)
        # Send the request.
        skt.send(request)
        # Fetch the answer.
        results.append(skt.recv(4096))
        if(orbits[i].object_name == 'L0000P8'):
            print(request)
            print(results[-1])
    # <-- end for
    skt.close()

    # Parse the response from the server (error messages start with 'E').
    positions = []
    for res in results:
        if(res[0] == 'E'):
            print('Warning: ephemeris computation for Orbit(%d) failed.' \
                      % (orbit.orbit_id))
            return(None, None)
        try:
            # Fetch the first line only.
            res = res.split('\n')[0]
            (orbit_id, epoch, ra, dec, mag, orientation, length) = res.split()
        except:
            print('Warning: parsing of results for Orbit(%d) failed.' \
                      % (orbit.orbit_id))
            print('         got: "%s"' % (res))
        positions.append((float(ra), float(dec)))
    return(positions)

def sphericalDistance(pos1, pos2):
    """
    Compte the spherical distance between two points on the sky (@param pos1 and
    @param pos2). Points are given as (RA, Dec) tuples; RA and Dec both in
    decimal degrees.

    Return the distance in arcseconds.
    """
    d = -1
    # TODO: Turn positions into structures.
    # FIXME: Use slalib for this as the formula used here is fairly lame.
    delatRA = (pos1[0] - pos2[0]) * math.cos(pos1[1] * math.pi / 180.)
    deltaDec = pos1[1] - pos2[1]
    d = math.sqrt(delatRA**2 + deltaDec**2)

    # Convert d from degrees to arcsec
    d *= 3600.
    return(d)

def sphericalDistances(detections, positions, orbits):
    """
    Like sphericalDistance but in batch mode.

    Return a list of Distance instances.
    """
    distances = []

    for i in range(len(detections)):
        pos1 = (detections[i].ra_deg, detections[i].dec_deg)
        pos2 = positions[i]
        distances.append(Distance(detections[i],
                                  orbits[i],
                                  sphericalDistance(pos1, pos2)))
    return(distances)

def getOrphanOrbitDistances(dbc):
    """
    Retrieve the list of orphan detections and the list of orbits whch should
    have been associated to the same orphan detections (but are not). Compute
    the orbit-detection distance.

    Return list of Distance instances.
    """
    orbits = []
    detections = getOrphanDetections(dbc, limitingMag=LIMITING_MAG)

    for det in detections:
        orbit = getOrbit(dbc, det.ssm_id)
        if(not orbit):
            print('Warning: could not find orbit for SSM(%d).' % (det.ssm_id))
            orbits.append(None)
            continue
        orbits.append(orbit)
    # <-- end for
    return(computeDistances(detections, orbits))

def getRefOrbitDistances(dbc):
    """
    Compute the orbit-detection distances for all detections which
      1. Have magnitude <= limitingMag.
      2. Belong to a tracklet.
      3. Are associated to a SSM object for which we have a DerivedObject.
      4. Belong to the first night (See FIXME below).
    and whose tracklets
      1. *Are* associated to a DerivedObject.

    Return list of Distance instances.
    """
    # TODO: refactor getRefOrbitDistances and getOrphanOrbitDistances into one.
    orbits = []
    detections = getRefDetections(dbc, LIMITING_MAG)

    for det in detections:
        orbit = getOrbit(dbc, det.ssm_id)
        if(not orbit):
            print('Warning: could not find orbit for SSM(%d).' % (det.ssm_id))
            orbits.append(None)
            continue
        orbits.append(orbit)
    # <-- end for
    return(computeDistances(detections, orbits))

def numberOfBins(distribution, method='FreedmanDiaconi'):
    """
    Compute the optimal number of histogram bins for distribution. Use either
    the method proposed by Scott, 79 or the one proposed by Freedman & Diaconi.
    distribution is a numarray 1-D array with the data points.

    Scott:
    W = 3.49 stddev * N**(1/3)
    W: bin width.
    stddev: standard deviation of the data points.
    N: number of data points.

    Freedman & Diaconi:
    W = 2 * IQR * N**(1/3)
    W: bin width.
    IQR: interquartile range (http://en.wikipedia.org/wiki/Interquartile_range).
    N: number of data points.
    """
    N = len(distribution)

    if(method == 'Scott'):
        stddev = distribution.stddev()
        W = 3.49 * stddev * N**(-1./3.)
    elif(method == 'FreedmanDiaconi'):
        sorted = numarray.sort(distribution)
        IQR = abs(sorted[int(N * 0.75)] - sorted[int(N * 0.25)])
        W = 2 * IQR * N**(-1./3.)
    else:
        print('Error: method %s is not supported.' % (method))
        print('  Available methods are Scott or FreedmanDiaconi.')
        raise(NotImplementedError())
    nBins = int(math.ceil(abs(distribution.max() - distribution.min()) / W))
    if(nBins <= 2):
        print('Warning: only got %d bins.' % (nBins))
    return(nBins)

def main(database, plotFileName=None):
    """
    Main entry point.
    """
    # Establish a connection to the DB.
    # TODO: conect once and reuse the connection.
    dbc = MySQLdb.connect(user=DB_USER,
                          passwd=DB_PASSWD,
                          host=DB_HOST,
                          db=database).cursor()

    # Compute the orphan detection-orbit distances.
    distances = getOrphanOrbitDistances(dbc)

    # Just for comparison, compute the detection-orbit distance for those
    # detections which have been precovered.
    refDistances = getRefOrbitDistances(dbc)

    # Output the raw data.
    print('Precovered:')
    print('Distance (")  SSM ID  Object  Detection ID  Tracklet ID  Field ID')
    for d in refDistances:
        print(d)
    print('Non-Precovered:')
    print('Distance (")  SSM ID  Object  Detection ID  Tracklet ID  Field ID')
    for d in distances:
        print(d)

    # Should we produce a plot?
    if(plotFileName):
        import numarray
        import pylab


        # Convert distances into a numarray array.
        distances = numarray.array([d.distance for d in distances])
        refDistances = numarray.array([d.distance for d in refDistances])

        # First sub-plot: detection-orbit distances for precovered detection.
        if(len(refDistances)):
            pylab.subplot(211)

            # Compute the bumber of bins.
            numBins = numberOfBins(refDistances)

            # Create the histogram.
            pylab.hist(refDistances, bins=int(numBins))
            if(not len(distances)):
                pylab.xlabel('Distance (")')
            pylab.ylabel('N Detections')
            pylab.title('Detection-Orbit Distance (Precovered)')
        if(len(distances)):
            pylab.subplot(212)

            # Compute the bumber of bins.
            numBins = numberOfBins(distances)

            # Create the histogram.
            pylab.hist(distances, bins=int(numBins))
            pylab.xlabel('Distance (")')
            pylab.ylabel('N Detections')
            pylab.title('Detection-Orbit Distance (Non Precovered)')
        pylab.savefig(plotFileName)
    # <-- end if

    # Summary
    available = len(refDistances) + len(distances)
    found = len(distances)
    percent = 100. * float(found) / float(available)
    print('Found %d/%d (%.02f%%) orphan detections.' % (found,
                                                        available,
                                                        percent))
    return




if(__name__ == '__main__'):
    # TODO: Parse command line options.
    import sys

    try:
        database = sys.argv[1]
    except:
        print('usage: precov_test.py database [plot file name]')
        sys.exit(1)
    try:
        plot = sys.argv[2]
    except:
        plot = None
    sys.exit(main(database, plot))

