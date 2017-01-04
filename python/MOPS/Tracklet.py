from __future__ import division
from __future__ import with_statement

"""
$Id$

Provides abstraction and services for the MOPS Fields DB table.

Note that this module simply provides procedural interfaces to
the database.
"""

__author__ = 'LSST'
__date__ = '$Date$'
__version__ = '$Revision$'[11:-2]


import os
import os.path
import logging
import copy

import MOPS.Constants
import MOPS.Utilities
import MOPS.Lib
from MOPS.Submissions import Submission
from MOPS.Detection import Detection
from MOPS.Known import Known
from MOPS.Exceptions import *
from MOPS.Instance import Transaction


class Tracklet(object):
    """
    Placeholder class for results from retrieve().  It would be nice if
    this class could update the DB automagically when its status changed.
    """
    def __init__(self, trackletId, vRa, vDec, vTot, vRaErr, vDecErr, posAng,
                 gcrArcsec, extEpoch, extRa, extDec, extMag, probability, 
                 digest, status, knownId, fieldId, knownName=None):

        if trackletId is None:
            self._id = None
        else:
            self._id = int(trackletId)
        # <-- end if
        
        self.vRa = vRa
        self.vDec = vDec
        self.vTot = vTot
        self.vRaErr = vRaErr
        self.vDecErr = vDecErr
        self.posAng = posAng
        self.gcrArcsec = gcrArcsec
        self.extEpoch = extEpoch
        self.extRa = extRa
        self.extDec = extDec
        self.extMag = extMag
        self.probability = probability
        self.digest = digest
        self.status = status
        self.knownId = knownId
        self.fieldId = fieldId
        self.detections = []
        self.submissions = []
        self.known = None
        self.knownName = knownName
        return
    # <-- end def

    def __str__(self):
        showList = sorted(set(self.__dict__))
        return MOPS.Utilities.toString(self, showList)
    # <-- end def

    def fetchKnown(self, dbh=None):
        """
        Fetch Known information if object is a known object.
        """
        if not self.knownId:
            # Tracklet is not known return.
            return self.known
        # <-- end if
        
        if not self.known:
            if not dbh:
                raise RuntimeError('fetchKnown() called with no dbh')
            # <-- end if

            sql = '''\
select known_id, known_name, q, e, i, node, arg_peri, time_peri, epoch, h_v
from known
where known_id = %d
''' %(self.knownId)

            # Execute the SQL statement.
            cursor = dbh.cursor()
            n = cursor.execute(sql)
            if(not n):
                # Not known , then return
                return self.known
            # <-- end if

            row = cursor.fetchone()
            row = list(row)
            self.known = (Known(*row))
        # <-- end if

        return self.known
    # <-- end def
    
    def fetchSubmissons(self, dbh=None):
        """
        Fetch any submissions that this tracklet is a part of.
        """
        if not self.submissions:
            if not dbh:
                raise RuntimeError('fetchSubmissions() called with no dbh')
            # <-- end if

            sql = '''\
select seq_num, batch_num, fpa_id, epoch_mjd, ra_deg, dec_deg, filter_id, mag,
obscode, desig, digest, spatial_idx, survey_mode, dbname, det_id, tracklet_id,
derivedobject_id, submitter, submitted, disposition, discovery, mpc_desig
from export.mpc_sub
where tracklet_id = %d
order by epoch_mjd
''' %(self._id)

            # Execute the SQL statement.
            cursor = dbh.cursor()
            n = cursor.execute(sql)
            if(not n):
                # No submissions, then return
                return
            # <-- end if

            row = cursor.fetchone()
            while(row):
                row = list(row)
                self.submissions.append(Submission(*row))

                # Next line.
                row = cursor.fetchone()
            # <-- while
        # <-- end if

        return self.submissions
    # <-- end def
    
    def fetchDetections(self, dbh=None):
        """
        Fetch the detections that make up this tracklet and store them in
        self.detections
        """

        if not self.detections:
            if not dbh:
                raise RuntimeError('fetchDetections() called with no dbh')

            self.detections = []        # in case it's None

            sql = '''\
select distinct(d.det_id), d.ra_deg, d.dec_deg, 
d.epoch_mjd, d.mag,
d.ref_mag, d.s2n, d.ra_sigma_deg, d.dec_sigma_deg,
d.mag_sigma, d.orient_deg, d.length_deg, d.status,
d.field_id, d.filter_id, d.obscode, d.object_name
from detections d, tracklet_attrib ta
where ta.tracklet_id = %d and ta.det_id = d.det_id
order by d.epoch_mjd
''' %(self._id)

            # Execute the SQL statement.
            cursor = dbh.cursor()
            n = cursor.execute(sql)
            if(not n):
                # No detections???
                raise RuntimeError('tracklet %d has no detections' % self._id)
            # <-- end if

            row = cursor.fetchone()
            while(row):
                row = list(row)
                self.detections.append(Detection(*row))

                # Next line.
                row = cursor.fetchone()
            # <-- end while
        # <-- end if

        return self.detections
    # <-- end def

    def fetchFieldId(self, dbh):
        sql = '''\
select field_id from tracklets where tracklet_id=%s
'''
        cursor = dbh.cursor()
        cursor.execute(sql, (self._id,))
        return (cursor.fetchall())[0][0]


    @staticmethod
    def retrieve_derivedobject(dbh, derivedobject_id, fetch_detections=False):
        """
        Return a list of tracklets belonging to the specified derived object ID.
        """
        sql = '''\
select t.tracklet_id, v_ra, v_dec, v_tot, v_ra_sigma, v_dec_sigma, pos_ang_deg, 
gcr_arcsec, ext_epoch, ext_ra, ext_dec, ext_mag, probability, digest, status, known_id, 
field_id
from tracklets t, derivedobject_attrib da
where da.derivedobject_id=%s and
t.tracklet_id = da.tracklet_id
'''

        # Execute the SQL.
        cursor = dbh.cursor()
        n = cursor.execute(sql, (derivedobject_id,))
        if not n:
            raise RuntimeError("got empty tracklet list for derived object ID %d" % derivedobject_id)
        # <-- end if

        rows = cursor.fetchall()
        tracklets = [Tracklet(*row) for row in rows]
        if fetch_detections:
            for t in tracklets:
                t.fetchDetections(dbh)
            # <-- end for
        # <-- end if

        return tracklets
    # <-- end def

    @staticmethod
    def retrieve(dbh, tracklet_id, fetch_detections=False):
        """
        Return the tracklet with the specified ID.
        """
        sql = '''\
select t.tracklet_id, v_ra, v_dec, v_tot, v_ra_sigma, v_dec_sigma, pos_ang_deg,
gcr_arcsec, ext_epoch, ext_ra, ext_dec, ext_mag, probability, digest, status, known_id, 
field_id, known_name
from tracklets t left join known using (known_id)
where t.tracklet_id=%s
'''

        # Execute the SQL.
        cursor = dbh.cursor()
        n = cursor.execute(sql, (tracklet_id,))
        if not n:
            raise RuntimeError("Retrieved empty tracklet for ID %d" % tracklet_id)
        # <-- end if

        row = cursor.fetchone()
        tracklet = Tracklet(*row)
        if fetch_detections:
            tracklet.fetchDetections(dbh)
            # <-- end for
        # <-- end if

        return tracklet
    # <-- end def

    @staticmethod
    def new(trackletId, fieldId, detectionList):
        """
        Create an in-memory tracklet, unattached to any particular MOPS database,
        from a specified list of detections.
        """
        import slalib

        if trackletId is not None:
            # Not supported yet
            raise RuntimeError("Tracklet.New() only supported with trackletId=None")

        status = 'U'
        probability = 1.0
        digest = 0.0

        # Copy our detection list and sort by epoch_mjd.  Then get min/max
        # positions at endpoints.
        dets = copy.copy(detectionList)
        dets.sort(lambda det1, det2: cmp(det1.mjd, det2.mjd))
        last_det = dets[-1]
        first_det = dets[0]

        delta_epoch_days = last_det.mjd - first_det.mjd
        if delta_epoch_days != 0:
            delta_ra_deg = MOPS.Lib.dang(last_det.ra, first_det.ra)
            delta_dec_deg = MOPS.Lib.dang(last_det.dec, first_det.dec)
            extEpoch = first_det.mjd + delta_epoch_days / 2
            extRa, extDec = MOPS.Lib.normalizeRADEC(
                first_det.ra + delta_ra_deg / 2,
                first_det.dec + delta_dec_deg / 2,
            )
            vRa = delta_ra_deg / delta_epoch_days
            vDec = delta_dec_deg / delta_epoch_days

            vTot = slalib.sla_dsep(
                first_det.ra / MOPS.Constants.DEG_PER_RAD,
                first_det.dec / MOPS.Constants.DEG_PER_RAD,
                last_det.ra / MOPS.Constants.DEG_PER_RAD,
                last_det.dec / MOPS.Constants.DEG_PER_RAD
            ) * MOPS.Constants.DEG_PER_RAD / delta_epoch_days
            extMag = (first_det.mag + last_det.mag) / 2

            posAng = slalib.sla_dbear(
                first_det.ra / MOPS.Constants.DEG_PER_RAD,
                first_det.dec / MOPS.Constants.DEG_PER_RAD,
                last_det.ra / MOPS.Constants.DEG_PER_RAD,
                last_det.dec / MOPS.Constants.DEG_PER_RAD
            ) * MOPS.Constants.DEG_PER_RAD
        else:
            extEpoch = first_det.mjd
            extRa = first_det.ra
            extDec = first_det.dec
            extMag = first_det.mag
            vRa = 0
            vDec = 0
            posAng = 0
        # <- end if

        # These aren't supported yet.
        vRaErr = 0
        vDecErr = 0
        gcrArcsec = None

        tracklet = Tracklet(
            trackletId=None,
            vRa=vRa,
            vDec=vDec,
            vTot=vTot,
            vRaErr=vRaErr,
            vDecErr=vDecErr,
            posAng=posAng,
            gcrArcsec=gcrArcsec,
            extEpoch=extEpoch,
            extRa=extRa,
            extDec=extDec,
            extMag=extMag,
            probability=probability,
            digest=digest,
            status=status,
            known_id=None,
            fieldId=fieldId
        )

        # Load our detections into the tracklet
        tracklet.detections = detectionList

        return tracklet
    # <-- end def

    def upgrade(self, dbh):
        """
        Upgrade ourselves by inserting any necessary detections into the MOPS
        database, then storing our tracklet data in the DB.
        """
        if self._id is not None:
            return          # already in DB
        # <-- end if

        # Need to suss out efficiency-related SSM ID, classifcation.
        (classification, ssm_id) = MOPS.Lib.classifyDetections(dbh, self.detections)

        # Do DB stuff here.
        with Transaction(dbh):
            for det in self.detections:
                det.upgrade(dbh)
            # <-- end for

            cursor = dbh.cursor()        
            sql = '''\
insert into tracklets (
field_id,
v_ra, v_dec, v_tot, v_ra_sigma, v_dec_sigma, pos_ang_deg, gcr_arcsec
ext_epoch, ext_ra, ext_dec, ext_mag, 
probability, digest, status, classification, ssm_id, known_id
)
values (
%s,
%s, %s, %s, %s, %s, %s, %s,
%s, %s, %s, %s, 
%s, %s, %s, %s, %s, %s
)
'''
            res = cursor.execute(sql, (
                self.fieldId,
                self.vRa,
                self.vDec,
                self.vTot,
                self.vRaErr,
                self.vDecErr,
                self.posAng,
                self.gcrArcsec,
                self.extEpoch,
                self.extRa,
                self.extDec,
                self.extMag,
                self.probability,
                self.digest,
                'A',                    # always attributed (A) on upgrade
                classification,
                ssm_id,
                self.knownId
            ))
            if not res:
                raise RuntimeError('upgrade of tracklet failed')
            # <-- end if
            self._id = dbh.insert_id()

            # Put an entry in tracklet_attrib.
            sql = '''\
insert into tracklet_attrib
(tracklet_id, det_id)
values (%s, %s)
'''
            for det in self.detections:
                cursor.execute(sql, (self._id, det._id))
            # <-- end for
        # <- with
    # <-- end def    
if __name__ == "__main__":
    print 'hi!'
# <-- end if