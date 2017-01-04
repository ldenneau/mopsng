#!/usr/bin/env python

raise RuntimeException("This program has been deprecated.")

"""
Compute the efficiency of attribution and precovery.

We define this efficiency to be the fraction of tracklets that were correctly
assigned to a DerivedObject during a single night. The key piece of information
used to perform this efficiency measurement is the SSM (Syntetic Solar System)
ID that MOPS has already assigned to detections, tracklets and derivedobjects.
Another derived important piece of information is each object and tracklet
classification (which derives from detections' ssm_id).

The processing flow is as follows:
1. Get a list of orbits that were to be precovered/attributed (from EON queue).
2. Get the list of observed nights from the DB (or single night if --nn is used)
3. Get the list of tracklets that were used during attribution/precovery (from
   an input file).
4. For each derived object with ssm_id != NULL in the list at 1.:
  4a. Retrieve the list of tracklets that have same ssm_id and have not been
      attributed to that object and that have MJD in nights.
  4b. For each unattributed tracklet:
    4ba. Compute the object-tracklet distance in arcsec.
    4bb. Write an entry in the history_(precoveries|attributions) table.
  4c. For each attributed tracklet:
    4ca. Write an entry in the history_(precoveries|attributions) table.
5. Cleanup amd quit.
"""

import cPickle as pickle
import os
import math
import time

import numpy
import ssd

import MOPS.Constants
import MOPS.Lib
import MOPS.EONQueue
import MOPS.Condor

from MOPS.Field import Field
from MOPS.Orbit import Orbit
from MOPS.Detection import Detection
from MOPS.DerivedObject import DerivedObject
from MOPS.Exceptions import *

from MOPS.Precovery import getPrecoveryJobs, getAllObservedNights, getAllDerivedObjectsAsEONQueues, getSomeDerivedObjectsAsEONQueues



# Constants
DEG_TO_RAD = math.pi / 180.
DEG_TO_SEC = 3600.



def getObjectSSMIds(instance, mode='precovery', forceIds=[]):
    """
    Get the list of ids of the derived object we want to examine to compute
    efficiency. Only return the subset that has a non null ssm_id.

    Return [(derivedobjecId, ssmId), ...]
    """
    
    
    # Get the list of derived objects waiting in the EON queue.
    if(mode == 'precovery'):
        jobs = getPrecoveryJobs(instance.get_dbh())
        if(forceIds):
            jobs += getSomeDerivedObjectsAsEONQueues(instance.get_dbh(), forceIds)
        # <-- end if
    else:
        jobs = getAllDerivedObjectsAsEONQueues(instance.get_dbh())
    # <-- end if
    if(not jobs):
        # Nothing to do: quit
        return([])
    # <-- end if
    
    # Get the list of all derived object ids from the queue list.
    allObjIds = [int(j.derivedobjectId) for j in jobs]

    # Now extract the subset of these object ids that have a solar system object
    # associated to them.
    sql = 'select derivedobject_id, ssm_id from derivedobjects where ' + \
          'derivedobject_id in (%s) and ssm_id is not null and status != "M"' \
          % (','.join([str(x) for x in allObjIds]))
    print sql
    cursor = instance.get_dbh().cursor()
    n = cursor.execute(sql)
    return(cursor.fetchall())

def getNights(instance, mode, nn=None):
    """
    Get a list of MJD for all te observed mights so far. If nn != None, then
    only return nn as the only night to consider.

    nn, if specified, is the MJD of the only night to examine. Of course, if nn
    is None AND mode != attribution, then it is a noop (i.e. attribution with no
    night to process).
    """
    if(nn != None):
        # process single night
        nights = [nn]
    elif(mode == 'precovery'):
        nights = getAllObservedNights(instance.get_dbh())
    else:
        nights = []
    # <-- end if
    return(nights)

def _alreadyUpdated(instance, mode, trackletId, objectId):
    """
    Check and see if the derivedobject_id/tracklet_id combo is already in the
    appropriate history tables. Return True if it is, False otherwise.
    """
    # Get a cursor.
    dbh = instance.get_dbh()
    cursor = dbh.cursor()

    # Compose the SQL query, depending on mode.
    if(mode == 'precovery'):
        eventType = 'P'
        table = 'history_precoveries'
    else:
        eventType = 'A'
        table = 'history_attributions'
    # <-- end if
    sql = 'select distinct(h.event_id) from history h, %s hx ' %(table)
    sql += 'where h.derivedobject_id = %d and ' %(objectId)
    sql += 'h.event_id = hx.event_id and '
    sql += 'hx.tracklet_id = %d' %(trackletId)
    n = cursor.execute(sql)
    return(n >= 1)

def _updateDB(instance, mode, trackletId, mjd, ra, dec, fieldId, objectId, 
              ssmId, obscode, limitingMag, maxSearchRadius, modifiedObject=None,
              failed=False, mixed=False, forceUpdate=False):
    """
    Uodate the database depending on mode and whether or not the attrib/precov
    succeeded.

    modifiedObject if not None, is the full derivedobject as it was passed from
    the precovery code.

    Also compute the ephem distance between object and tracklet.
    """
    logger = instance.getLogger()
    
    # Get a cursor.
    dbh = instance.get_dbh()
    cursor = dbh.cursor()

    # First of all, check and see if the history tables have already been
    # updated. This could happen if the efficiency script is run manually a
    # second time.
    alreadyThere = _alreadyUpdated(instance, mode, trackletId, objectId)
    if(not forceUpdate  and alreadyThere):
        msg = 'effAttrPrecov: obj %d/tracklet %d already in the DB.' \
              %(objectId, trackletId)
        logger.info(msg)
        return
    # <-- end if

    # Do we need to fetch the orbit from the DB or do we have it already in
    # modifiedObject?
    if(modifiedObject):
        q = modifiedObject.orbit.q
        e = modifiedObject.orbit.e
        i = modifiedObject.orbit.i
        node = modifiedObject.orbit.node
        argPeri = modifiedObject.orbit.argPeri
        timePeri = modifiedObject.orbit.timePeri
        epoch = modifiedObject.orbit.epoch
        mag = modifiedObject.orbit.h_v
        src = modifiedObject.orbit.src
        orbitId = modifiedObject.orbit._id
    else:
        # Fetch the new orbit.    
        (q, e, i, node, argPeri, timePeri,
         epoch, mag, src, conv_code, orbitId) = _fetchOrbit(instance, objectId)
    # <-- end if

    # Compute ephemerides at mjd.
    # pos = [ra, dec, mag, mjd, raErr, decErr, smaa, smia, pa]
    # Make also sure that we handle missing SRCs well.
    srcArray = None
    if(src and len(src) == 21 and None not in src):
        srcArray = numpy.array(src)
    # <-- end if
    # RA, Dec, mag, MJD, uncertainties
    pos = ssd.ephemerides(numpy.array([q, e, i, node, argPeri, timePeri]),
                          epoch,
                          numpy.array([mjd, ]),
                          obscode,
                          mag,
                          None,
                          srcArray)[0]

    # Just make sure that the object predicted position is actually visible.
    mag = pos[2]
    magThreshold = limitingMag
    if(mag > magThreshold):
        # Object is not visible, forget about the DB update.
        msg = 'effAttrPrecov: obj %d/tracklet %d too faint: no DB update.' \
              %(objectId, trackletId)
        logger.info(msg)
        return
    # <-- end if

    # Compute positional uncertainties etc.
    uncertainty = pos[6]
    distance = MOPS.Lib.sphericalDistance_arcsec([ra, dec], pos[:2])

    # If the uncertainty is greater than the maxSearchRadius, no need to update
    # the DB either: the search was aborted. A None or negative predicted
    # positional uncertainty means that the search happened with a search radius
    # equal to maxSearchRadius.
    if(uncertainty > maxSearchRadius):
        msg = 'effAttrPrecov: obj %d/tracklet %d uncert too big: no DB update.'\
              %(objectId, trackletId)
        logger.info(msg)
        return
    # <-- end if
    
    # Update both the history table and the history_<mode> table.
    logger.info('effAttrPrecov: updating db for obj %d and tracklet %d.' \
                %(objectId, trackletId))
    sql = 'select max(event_id) from history'
    n = cursor.execute(sql)
    if(n == 0):
        eventId = 0
    else:
        eventId = int(cursor.fetchone()[0]) + 1
    # <-- end if

    if(mode == 'precovery'):
        eventType = 'P'
        table = 'history_precoveries'
    else:
        eventType = 'A'
        table = 'history_attributions'
    # <-- end if
    if(failed):
        classif = 'X'
        orbitType = 'F'
    else:
        if(mixed):
            classif = 'M'
        else:
            classif = 'C'
        # <-- end if
        orbitType = 'Y'
    # <-- end if

    # See if we are in forceUpdate mode and the entry is already in the DB.
    if(forceUpdate  and alreadyThere):
        pass

    # Update the main history table.
    sql = 'insert into history values (%d, ' %(eventId) + \
          '"%s", ' %(eventType) + \
          'sysdate(), ' + \
          '%d, ' %(objectId) + \
          '%d, ' %(orbitId) + \
          '"%s", ' %(orbitType) + \
          'NULL, ' + \
          'NULL, ' + \
          '%d, ' %(fieldId) + \
          '"%s", ' %(classif) + \
          '%d)' %(ssmId)
    cursor.execute(sql)
    
    # Now update the corresponding sub-table.
    sql =  'insert into %s values (%d, ' %(table, eventId) + \
          '%d, ' %(trackletId) + \
          '%f, ' %(distance) + \
          '%f) ' %(uncertainty)
    cursor.execute(sql)
    
    # Commit and quit
    dbh.commit()
    return

def _fetchOrbit(instance, objectId):
    # Get a cursor.
    cursor = instance.get_dbh().cursor()

    # Retrieve the orbital params for the derived object.
    sql = 'select o.q, o.e, o.i, o.node, o.arg_peri, o.time_peri, ' + \
          'o.epoch, o.h_v, o.cov_01, o.cov_02, o.cov_03, o.cov_04, ' + \
          'o.cov_05, o.cov_06, o.cov_07, o.cov_08, o.cov_09, o.cov_10, ' + \
          'o.cov_11, o.cov_12, o.cov_13, o.cov_14, o.cov_15, o.cov_16, ' + \
          'o.cov_17, o.cov_18, o.cov_19, o.cov_20, o.cov_21, o.conv_code, ' + \
          'o.orbit_id from orbits o, derivedobjects d where ' + \
          'o.orbit_id = d.orbit_id and d.derivedobject_id = %d' %(objectId)
    n = cursor.execute(sql)
    if(n != 1):
        # Something went wrong!
        raise(ProcessingException('_updateDB touched %d orbits.' %(n)))
    # <-- end if
    orbit = list(cursor.fetchone())
    return(orbit[:8] + [orbit[8:-2]] + orbit[-2:])


def _fetchTrackletDataForSSMId(instance, ssmId, nights, limitingMag):
    """
    Fetch all the tracklets corresponding to a given SSM id and that were
    created in a given set of nights. Only select tracklets that are visible.

    Return an iterator.
    """
    # Grab a cursor to the DB.
    cursor = instance.get_dbh().cursor()
    
    # Prepare and send the SQL query.
    # Note that we want to make sure that we do not select tracklets that are
    # made of detections observed *after* the object was originally create or
    # modified (precovery). We do not want to select tracklets whose detections
    # are too dim either.
    sql = 'select distinct(t.tracklet_id), t.status, t.field_id, t.ext_ra, '+ \
          't.ext_dec, t.ext_epoch ' + \
          'from tracklets t, tracklet_attrib ta, detections d ' + \
          'where t.ssm_id = %d and t.classification="C" and ' %(ssmId) + \
          't.tracklet_id = ta.tracklet_id and d.det_id = ta.det_id '

    # Now, make sure that the tracklets we extract were observed during the
    # nights we are examining.
    sql += 'and ('
    for night in nights:
        sql += 't.ext_epoch between %d and %d or ' %(night, night+1)
    # <-- end for
    sql = sql[:-4] + ') '

    # Now finish off the query by grouping by tracklet_id and restricting to
    # tracklets with at least one visible detection.
    sql += 'group by t.tracklet_id having max(d.mag) < %f' %(limitingMag)

    print sql

    n = cursor.execute(sql)

    # Now return an iterator spitting out a tracklet at the time.
    row = cursor.fetchone()
    while(row):
        yield(row)
        row = cursor.fetchone()
    # <-- end while
    raise(StopIteration())


def objectEfficiency(instance, objectId, ssmId, nights, mode, attribTracklets,
                     modifiedObject, obscode, limitingMag, maxSearchRadius,
                     forceUpdate=False):
    """
    Given a derived object id and a list of MJDs, retrieve all the clean
    tracklets that are associated (via ssm_id) to it. Look at their status and
    determine whether the previous attribution/precovery attempt was successful.
    Look for extra information into the list of tracklets that the previous
    attribution/precovery ended up using.

    Of course, the efficiency computation can only be done for objects derived
    from the solar system model.

    modifiedObject if not None, is the full derivedobject as it was passed from
    the precovery code.

    We need to think about how to get the percent of mixed tracklets.

    If forceUpdate=True, then we update the database no matter what. If a DB
    entry was already there then we overwrite it.
    """
    cursor = instance.get_dbh().cursor()
    
    # Extract the tracklet id from attribTracklets
    refTrackletIds = [t._id for t in attribTracklets]

    # Fetch all the tracklets that should be associated with objectID (via its
    # SSM link).
    trackletFetcher = _fetchTrackletDataForSSMId(instance, ssmId,
                                                 nights, limitingMag)

    try:
        data = trackletFetcher.next()
        while(data):
            trackletId, status, fieldId, ra, dec, mjd = data

            # We have the following cases now:
            # 1. status=U: failed.
            # 2. status=A and objectId == reObjectId and \
            #    trackletId in refTrackletIds: OK.
            # 3. else: old association.
            if(status == 'U'):
                _updateDB(instance, mode, trackletId, mjd, ra, dec, fieldId,
                          objectId, ssmId, obscode, limitingMag,
                          maxSearchRadius, modifiedObject,
                          failed=True, mixed=False, forceUpdate=forceUpdate)
            elif(trackletId in refTrackletIds):
                _updateDB(instance, mode, trackletId, mjd, ra, dec, fieldId,
                          objectId, ssmId, obscode, limitingMag,
                          maxSearchRadius, modifiedObject,
                          failed=False, mixed=False, forceUpdate=forceUpdate)
            else:
                # Nothing to see here.
                pass
            # <-- end if
        
            # Go to the next line.
            data = trackletFetcher.next()
        # <-- end while
    except StopIteration:
        # Nothing else to do.
        pass

    # Now do the same thing for mixed tracklets.
    return


def efficiency(instance,
               mode,
               attribTracklets,
               modifiedObjects,
               obscode,
               limitingMag,
               maxSearchRadius,
               nn=None,
               forceIds=[],
               forceUpdate=False):
    """
    Efficiency computation for attribution/precovery.

    nn is used for attribution.
    """
    # Get a logger
    logger = instance.getLogger()

    # Get the list of derived object ids to be examined.
    # What we get is [(derivedobjectId, ssmId), ...]
    objectData = getObjectSSMIds(instance, mode, forceIds)
    if(mode == 'precovery'):
        msg = 'found %d %s jobs.' %(len(objectData), mode)
    else:
        msg = 'found %d derived objects.' %(len(objectData))
    # <-- end if
    logger.info('effAttrPrecov: %s' %(msg))
    if(not objectData):
        return(0)
    # <-- end if

    # Get a lookup table of modifiedObjectIds and modifiedObjets.
    modObjLookup = dict([(do._id, do) for do in modifiedObjects])

    # Now get all available (i.e. observed) nights.
    # What we get is [MJD1, MJD2, ...]
    nights = getNights(instance, mode, nn)
    logger.info('effAttrPrecov: found %d nights to examine.' %(len(nights)))
    if(not nights):
        return
    # <-- end if

    # Loop through all the objects.
    for (objectId, ssmId) in objectData:
        # Copute efficiency for that object. This also updates the database.
        logger.info('Computing efficiency for SSM %d' %(ssmId))

        # Id the objectId part od modifiedObjects?
        obj = modObjLookup.get(objectId, None)
        objectEfficiency(instance,
                         objectId,
                         ssmId,
                         nights,
                         mode,
                         attribTracklets,
                         obj,
                         obscode,
                         limitingMag,
                         maxSearchRadius,
                         forceUpdate)
    # <-- end for
    return




if(__name__ == '__main__'):
    import optparse
    import sys
    import MOPS.Instance

    
    # Constants
    USAGE = '''\
Usage: effAttrPrecov.py options

Options:
  --instance INSTANCE_NAME
  --nn NIGHT_NUMBER
  --tracklets TRK_FILE 
  --object OBJ_FILE
  --force_objects OBJECT_IDS
  --force_update
  --help
'''
    
    # Get user input (tracks file) and make sure that it exists.
    parser = optparse.OptionParser(USAGE)
    parser.add_option('--instance',
                      dest='instance_name',
                      type='string',
                      help="MOPS DB instance name")
    parser.add_option('--nn',
                      dest='nn',
                      type='int',
                      help="specify night number to process")
    parser.add_option('--tracklets',
                      dest='trackletFileName',
                      help="name of the tracklets file")
    parser.add_option('--objects',
                      dest='objectFileName',
                      help="name of the derived objects file")
    parser.add_option('--mode',
                      dest='mode',
                      help="mode: precovery or attribution")
    parser.add_option('--force_objects',
                      dest='forceObjects',
                      default='',
                      help="force_objects: force examining these object ids" + \
                      " (comma separated values)")
    parser.add_option("--force_update",
                      action="store_true",
                      dest="forceUpdate",
                      default=False)
    (options, args) = parser.parse_args()
    
    # Make sure that we are not missing any needed info.
    if(not options.mode or
       not options.trackletFileName or
       not os.path.exists(options.trackletFileName) or
       not options.objectFileName or
       not os.path.exists(options.objectFileName)):
        parser.error('incorrect number of arguments')
    # <-- end if
    
    # Determine the name of MOPS DB instance.
    mops_instance = MOPS.Instance.Instance(dbname=options.instance_name \
                                           or os.environ['MOPS_DBINSTANCE'])

    # Extract some parameters from the instance configuration.
    config = mops_instance.getConfig()
    obscode = config['site']['obscode']
    limitingMagCorrection = config['site'].get('limiting_mag_correction', 0)
    limitingMag = config['site']['limiting_mag'] + limitingMagCorrection
    maxSearchRadius = config['dtctl']['attribution_proximity_threshold_arcsec']

    mops_logger = mops_instance.getLogger()
    mops_logger.info('effAttrPrecov: starting.')

    # Parse the input files.
    f = file(options.trackletFileName, 'rb')
    tracklets = pickle.load(f)
    f.close()
    f = file(options.objectFileName, 'rb')
    objects = pickle.load(f)
    f.close()

    # Handle the force_objects option.
    forceObjects = []
    if(options.forceObjects):
        try:
            forceObjects = [int(_id) for _id in options.forceObjects.split(',')]
        except:
            mops_logger.info('effAttrPrecov: invalid force_objects syntax.')
            pass
    # <-- end if
    
    # Compute the efficiency.
    rc = efficiency(mops_instance,
                    options.mode,
                    tracklets,
                    objects,
                    obscode,
                    limitingMag,
                    maxSearchRadius,
                    nn=options.nn,
                    forceIds=forceObjects,
                    forceUpdate=options.forceUpdate)
    mops_logger.info('effAttrPrecov: done.')
    sys.exit(rc)

    

