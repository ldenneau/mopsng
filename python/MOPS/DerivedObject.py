"""
MOPS Python object-relational mapper: DerivedObject class.
"""

from operator import add

import MOPS.Constants
import MOPS.Utilities as Utilities
import MOPS.SSM
from MOPS.Submissions import Submission
from MOPS.Tracklet import Tracklet
from MOPS.Orbit import Orbit



class DerivedObject(object):
    def __init__(self, doId, objectName, status, stablePass, orbit):
        self._id = int(doId)
        self.orbit = orbit
        self.objectName = objectName
        self.tracklets = []
        self.submissions = []

        # Efficiency-related
        self.classification = None          # MOPS classification; shouldn't really ever be None, but we're not playing that game
        self.ssmId = None

        # Status
        self.status = status                # DO merged status
        self.stablePass = stablePass        # DO stable precovery pass completed
        return

    def __str__(self):
        showList = sorted(set(self.__dict__))
        return Utilities.toString(self, showList)
    # <-- end __str__
    
    def fetchSubmissons(self, dbh=None):
        """
        Fetch any submissions that this derived object is a part of.
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
where derivedobject_id = %d
order by epoch_mjd
''' %(self._id)

            # Execute the SQL statement.
            cursor = dbh.cursor()
            n = cursor.execute(sql)
            if(not n):
                # No submissions, then return
                return
            # <-- if

            row = cursor.fetchone()
            while(row):
                row = list(row)
                self.submissions.append(Submission(*row))

                # Next line.
                row = cursor.fetchone()
            # <-- while
        # <-- if
        return self.submissions
    # <-- end def

    def fetchTracklets(self, dbh=None):
        """
        Fetch the tracklets that make up this derived object and store them in
        self.tracklets
        """

        if not self.tracklets:
            if not dbh:
                raise RuntimeError('fetchTracklets() called with no dbh provided')

            self.tracklets = Tracklet.retrieve_derivedobject(dbh, self._id, fetch_detections=True)
        return self.tracklets

    def arcLength(self, dbh=None):
        """
        Compute the arc length in days of the detections that make up this
        derived object.

        If the tracklets have not been loaded, raise an exception.
        """
        if not self.tracklets:
            self.tracklets = self.fetchTracklets(dbh)
        # <-- end if

        # Derive the minimum and maximum epoch of the detections in
        # self.tracklets.
        dets = reduce(lambda x, y: x+y, [t.detections for t in self.tracklets])
        dets.sort(cmp=lambda d1, d2: cmp(d1.mjd, d2.mjd))
        return(abs(dets[-1].mjd - dets[0].mjd))


    def _attribute(self, cursor, trk):
        """Low-level attribute tracklet to derived object."""


    def classify(self, dbh):
        """Walk this derived object's tracklet list and determine the MOPS 
        efficiency classification for it based on its detections' object_names.
        """

        # Build det list.
        detections = reduce(add, [t.fetchDetections() for t in self.tracklets])

        # Set classification and SSM ID.
        self.classification = MOPS.Lib.classifyNames([d.objectName for d in detections])
        if self.classification == MOPS.Constants.MOPS_EFF_CLEAN:
            self.ssmId = MOPS.SSM.objectName2ssmId(dbh, detections[-1].objectName)
        else:
            self.ssmId = None

        return self.classification


    def attributeTracklets(self, dbh, orbit, tracklet, tracklets=None):
        """
        Associate the specified tracklet(s) with our derived object and
        replace our derived object's orbit with the specified orbit.  The 
        orbit provided is presumably the orbit computed after adding the
        tracklets.
        """

        if tracklets is None:
            tracklets = [tracklet]      # create a list of one

        # DB prep.
        cursor = dbh.cursor()

        # Add the tracklets.
        self.tracklets += tracklets
        for tracklet in tracklets:
            # Now add tracklet._id to derivedobject_attrib where
            # derivedobject_id = self._id
            cursor.execute("insert into derivedobject_attrib (derivedobject_id, tracklet_id) values (%s, %s)", (self._id, tracklet._id))
            cursor.execute('update tracklets set status=%s where tracklet_id=%s', (MOPS.Constants.TRACKLET_STATUS_ATTRIBUTED, tracklet._id))
        # <-- end for

        # Update the orbit. The orbit ID is None and will get assigned by the DB
        # and updated by self.orbit.save()
        self.orbit = orbit
        self.orbit.save(dbh)

        # Re-classify based on new constituent detections.
        self.classify(dbh)
        
        # Update our member columns.
        cursor.execute('update derivedobjects set updated=current_timestamp(), orbit_id=%s, classification=%s, ssm_id=%s where derivedobject_id=%s',
            (self.orbit._id, self.classification, self.ssmId, self._id))

        return

    @staticmethod
    def fetch(dbh, derivedObjectId=None, derivedObjectName=None, fetchTracklets=True):
        """
        Given a database connection handle and a derived object id, fetch the
        relevant data from the database and return a DerivedObject instance.

        @param dbh: Database conection object
        @param derivedObjectId: MOPS.DerivedObject instance id
        @param fetchTracklets: boolean to decide whether or not to fetch the
               MOPS.DerivedObject instance Tracklets as well.
        """
        where = None
        if derivedObjectId:
            where = ' where do.derivedobject_id=%s and do.orbit_id = o.orbit_id'
            args = (derivedObjectId,)
        elif derivedObjectName:
            where = 'where do.object_name=%s and do.orbit_id = o.orbit_id'
            args = (derivedObjectName,)
        else:
            raise RuntimeError('no derived object selector provided')
            
        # Get two cursors from the DB connection.
        cursor = dbh.cursor()

        # Compose the SQL statement.
        sql = '''\
select do.derivedobject_id, do.object_name, 
do.status, do.stable_pass,
o.orbit_id,
o.q, o.e, o.i, o.node, o.arg_peri, o.time_peri, o.h_v,
o.epoch,
o.cov_01, o.cov_02, o.cov_03, o.cov_04, o.cov_05, o.cov_06,
o.cov_07, o.cov_08, o.cov_09, o.cov_10, o.cov_11, o.cov_12,
o.cov_13, o.cov_14, o.cov_15, o.cov_16, o.cov_17, o.cov_18,
o.cov_19, o.cov_20, o.cov_21,
o.residual, o.chi_squared, o.moid_1, o.conv_code, o.moid_2
from derivedobjects do, orbits o %s
''' 

        # Execute it!
        n = cursor.execute(sql % where, args)
        if not n:
            raise RuntimeError("Got empty derived object back for " + (where % args))

        row = list(cursor.fetchone())

        # Create the Orbit instance.
        if row[13] is None:
            sqrt_cov = None
        else:
            sqrt_cov = row[13:34]
        o = Orbit(*(row[4:13] + [sqrt_cov] + row[34:]))

        # Create a bare-bone DerivedObject.
        dobj = DerivedObject(row[0], row[1], row[2], row[3], o)
        
        # Fetch the detections and the field.
        if fetchTracklets:
            dobj.fetchTracklets(dbh)
        # <-- end if
        return dobj
