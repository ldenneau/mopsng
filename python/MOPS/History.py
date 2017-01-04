"""
MOPS Python object-relational mapper: History class and subclasses.
"""

from __future__ import with_statement

from MOPS.Lib import calculateDCriterion
from MOPS.Instance import Transaction
from MOPS.Tracklet import Tracklet
from MOPS.Orbit import Orbit
from MOPS.SSM import SSM
from MOPS.Constants import *
from MOPS.Exceptions import *


EVENT_TYPE_DERIVATION = 'D'
EVENT_TYPE_IDENTIFICATION = 'I'
EVENT_TYPE_ATTRIBUTION = 'A'
EVENT_TYPE_PRECOVERY = 'P'
EVENT_TYPE_REMOVAL = 'R'


class HistoryBase(object):
    def __init__(self, event_type, derivedobject_id, field_id, orbit_id, orbit_code, classification, ssm_id, 
            event_id=None, event_time=None, is_lsd=None, d3=None, d4=None):
        self.event_id = event_id
        self.event_type = event_type
        self.derivedobject_id = derivedobject_id
        self.field_id = field_id
        self.orbit_id = orbit_id
        self.orbit_code = orbit_code
        self.classification = classification
        self.ssm_id = ssm_id
        self.is_lsd = is_lsd or 'N'
        self.d3 = None
        self.d4 = None
        return

    def record(self, dbh):
        """Record the event in the database."""

        # Populate the D3 and D4 parameters.  We should do this at
        # creation, but unlike the Perl version we do not have
        # inst/dbh at object creation, only when recording.
        if self.ssm_id is not None and (self.d3 is None or self.d4 is None):
            if not self.orbit_id:
                raise RuntimeError("ssm_id but not orbit_id specified")

            my_orb = Orbit.retrieve(dbh, self.orbit_id)
            ssm_orb = SSM.retrieve(dbh, self.ssm_id)

            if my_orb is None or ssm_orb is None:
                raise RuntimeError("my_orb or ssm_orb is None")

            self.d3, self.d4 = calculateDCriterion(ssm_orb, my_orb)
        # <- if ssm_id ...

        cursor = dbh.cursor()

        # Retrieve the last event that modified this object so that we can more easily
        # restore our paper trail of orbit modifications.
        sql = '''\
select event_id 
from history h 
where h.derivedobject_id=%s
order by event_id desc 
limit 1
'''
        res = cursor.execute(sql, (self.derivedobject_id,))
        if not res:
            raise ProcessingException('derivedobject lookup failed: %s: %s' % (sql, str(self.derivedobject_id)))
        last_event_id = int(cursor.fetchall()[0][0])

        sql = '''\
insert into history (
event_type, event_time, derivedobject_id, field_id, orbit_id, orbit_code, classification, ssm_id, d3, d4, is_lsd, last_event_id
)
values (
%s, sysdate(), %s, %s, %s, %s, %s, %s, %s, %s, %s, %s
)
'''
        values = [
            self.event_type, 
            self.derivedobject_id, 
            self.field_id, 
            self.orbit_id, 
            self.orbit_code,
            self.classification, 
            self.ssm_id,
            self.d3, 
            self.d4,
            self.is_lsd,
            last_event_id,
        ]

        res = cursor.execute(sql, values)     # caller must commit
        if not res:
            raise ProcessingException('insert failed: %s' % sql)
        self.event_id = int(dbh.insert_id())
        self.last_event_id = last_event_id


class Attribution(HistoryBase):
    def __init__(self, derivedobject_id, field_id, orbit_id, orbit_code, classification, ssm_id,
        tracklet_id, ephemeris_distance_arcsec, ephemeris_uncertainty_arcsec, is_lsd):
        
        HistoryBase.__init__(self, EVENT_TYPE_ATTRIBUTION, derivedobject_id, 
            field_id, orbit_id, orbit_code, classification, ssm_id, is_lsd=is_lsd)

        self.tracklet_id = tracklet_id
        self.ephemeris_distance_arcsec = ephemeris_distance_arcsec
        self.ephemeris_uncertainty_arcsec = ephemeris_uncertainty_arcsec

    def record(self, dbh):
        HistoryBase.record(self, dbh)

        sql = '''\
insert into history_attributions (
event_id, tracklet_id, ephemeris_distance, ephemeris_uncertainty 
)
values (
%s, %s, %s, %s
)
'''
        values = [
            self.event_id,
            self.tracklet_id,
            self.ephemeris_distance_arcsec,
            self.ephemeris_uncertainty_arcsec
        ]

        cursor = dbh.cursor()
        res = cursor.execute(sql, values)     # caller must commit
        if not res:
            raise ProcessingException('insert failed: %s' % sql)


class Precovery(HistoryBase):
    def __init__(self, derivedobject_id, field_id, orbit_id, orbit_code, classification, ssm_id,
        tracklet_id, ephemeris_distance_arcsec, ephemeris_uncertainty_arcsec, is_lsd):
        
        HistoryBase.__init__(self, EVENT_TYPE_PRECOVERY, derivedobject_id, 
            field_id, orbit_id, orbit_code, classification, ssm_id, is_lsd=is_lsd)

        self.tracklet_id = tracklet_id
        self.ephemeris_distance_arcsec = ephemeris_distance_arcsec
        self.ephemeris_uncertainty_arcsec = ephemeris_uncertainty_arcsec

    def record(self, dbh):
        HistoryBase.record(self, dbh)

        sql = '''\
insert into history_precoveries (
event_id, tracklet_id, ephemeris_distance, ephemeris_uncertainty 
)
values (
%s, %s, %s, %s
)
'''
        values = [
            self.event_id,
            self.tracklet_id,
            self.ephemeris_distance_arcsec,
            self.ephemeris_uncertainty_arcsec
        ]

        cursor = dbh.cursor()
        res = cursor.execute(sql, values)     # caller must commit
        if not res:
            raise ProcessingException('insert failed: %s' % sql)

