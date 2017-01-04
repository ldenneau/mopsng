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

import MOPS.Constants
import MOPS.Utilities
from MOPS.Tracklet import Tracklet



class Field(object):
    """
    Placeholder class for results from retrieve().  It would be nice if
    this class could update the DB automagically when its status changed.
    """
    def __init__(self,
                 fieldId,
                 mjd,                   # MJD of mid esposure
                 ra,
                 dec,
                 surveyMode='DD',
                 mjdStart=None,
                 mjdEnd=None,
                 filterName=None,
                 limitingMag=None,
                 raErr=0.,
                 decErr=0.,
                 obscode=None,
                 de=[0., ] * 10,         # detection efficiency array.
                 ocnum=None,
                 status=MOPS.Constants.FIELD_STATUS_READY,
                 parentId=None,
                 xyidxSize=None,
                 FOV_deg=None):
        self._id = fieldId
        self.mjd = mjd
        self.ra = ra
        self.dec = dec
        self.surveyMode = surveyMode
        self.mjdStart = mjdStart
        self.mjdEnd = mjdEnd
        self.filterName = filterName
        self.limitingMag = limitingMag
        self.raErr = raErr
        self.decErr = decErr
        self.obscode = obscode
        self.de = de
        self.ocnum = ocnum or MOPS.Utilities.mjd2ocnum(mjd)
        self.status = status
        self.parentId = parentId
        self.tracklets = []

        self.xyidxSize = xyidxSize
        self.FOV_deg = FOV_deg
        return

    @classmethod
    def new(cls, mjd, ra, dec, expTime, filterName, obscode, limitingMag):
        """
        Given MJD of mid exposure, RA, Dec (deg) of center, expTime in seconds
        and filter name, return a Field instance.
        """
        halfExpTimeDays = expTime / (2. * 86400.)
        mjdStart = mjd - halfExpTimeDays
        mjdEnd = mjd + halfExpTimeDays
        return(cls(None,
                   mjd=mjd,
                   ra=ra,
                   dec=dec,
                   mjdStart=mjdStart,
                   mjdEnd=mjdEnd,
                   filterName=filterName,
                   obscode=obscode,
                   limitingMag=limitingMag))

    @staticmethod
    def fetch(dbh, field_id):
        sql = """
select
    field_id,
    epoch_mjd, ra_deg, dec_deg, survey_mode, time_stop, time_start, status, 
    filter_id, limiting_mag, ra_sigma, dec_sigma, obscode,
    de1, de2, de3, de4, de5, de6, de7, de8, de9, de10,
    ocnum, nn, parent_id, fpa_id, xyidx_size, fov_deg, se_deg, b_deg
from `fields`
where field_id=%s
"""

        cursor = dbh.cursor()
        n = cursor.execute(sql, (field_id,))
        if not n:
            return None
        else:
            row = cursor.fetchone()
            cols = [d[0] for d in cursor.description]
            row_dict = dict(zip(cols, row))
            de = [row_dict["de%d" % (n + 1)] for n in range(10)]
            return Field(
                field_id, 
                row_dict['epoch_mjd'],
                row_dict['ra_deg'],
                row_dict['dec_deg'],
                row_dict['survey_mode'],
                row_dict['time_start'],
                row_dict['time_stop'],
                row_dict['filter_id'],
                row_dict['limiting_mag'],
                row_dict['ra_sigma'],
                row_dict['dec_sigma'],
                row_dict['obscode'],
                de,
                row_dict['ocnum'],
                row_dict['status'],
                row_dict['parent_id'],
                row_dict['xyidx_size'],
                row_dict['fov_deg'],
            )
        # <- if

    def insert(self, dbh):
        """
        Given a MOPS DB instance, add an entry to the fields table.
        """
        parent = str(self.parentId)
        if parent == 'None':
            parent = 'NULL'
        # <-- end if
        
        sql = """
insert into `fields` (
    epoch_mjd, ra_deg, dec_deg, survey_mode, time_start, time_stop, 
    status, filter_id, limiting_mag, ra_sigma, dec_sigma, obscode, parent_id, 
    de1, de2, de3, de4, de5, de6, de7, de8, de9, de10, 
    ocnum, xyidx_size, fov_deg
) 
values (
    %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, 
    %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s,
    %s, %s
)
"""

        cursor = dbh.cursor()
        return cursor.execute(sql, (
            self.mjd, self.ra, self.dec, self.surveyMode, self.mjdStart, self.mjdEnd, self.status,
            self.filterName, self.limitingMag, self.raErr, self.decErr, self.obscode, parent,
            self.de[0], self.de[1], self.de[2], self.de[3], self.de[4], self.de[5], 
            self.de[6], self.de[7], self.de[8], self.de[9],
            self.ocnum, self.xyidxSize, self.FOV_deg
        ))

    def fetchTracklets(self, dbh, trackletStatus='any'):
        """
        Fetch the tracklets and store them in self.tracklets

        If trackletStatus is specified, only the tracklets with that status are
        are selected. If trackletStatus='any' then all the tracklets are
        selected.
        """
        sql = '''\
select distinct(tracklet_id), v_ra, v_dec, v_tot,
v_ra_sigma, v_dec_sigma, pos_ang_deg, gcr_arcsec,  
ext_epoch, ext_ra, ext_dec, ext_mag,
probability, digest, status, known_id, field_id
from tracklets where field_id=%(fieldId)d
%(extraWhere)s
'''
        if not trackletStatus or trackletStatus == 'any':
            whereSQL = ''
        else:
            whereSQL = ' and status = "%s"' % (trackletStatus)
        # <-- end if

        # Execute the SQL.
        cursor = dbh.cursor()
        n = cursor.execute(sql % {'fieldId': self._id, 'extraWhere': whereSQL})
        if not n:
            self.tracklets = []
            return
        # <-- end if

        row = cursor.fetchone()
        while row:
            row = list(row)
            t = Tracklet(*row)
            t.fetchDetections(dbh)
            self.tracklets.append(t)

            # Next line.
            row = cursor.fetchone()
        # <-- end while
        return
    

    def fetchTrackletsByDetIndex(self, dbh, pos, search_radius_deg, zone_list=None, min_probability=None, minv_degperday=0):
        """
        Return a list of tracklet objects for each detection that
        appears in the indexed positions (zone list) for this field
        and is within the search radius from the position specified in pos.

        Note that the zone list is really just a shortcut for select detections
        within the field so that we have a tiny list to evaluate against the
        search radius.
        """

        search_radius_rad = search_radius_deg * MOPS.Constants.RAD_PER_DEG

        # construct SQL 'in' clause for zone list
        if zone_list:
            zone_str = 'and d.xyidx in (' + ','.join([str(x) for x in zone_list]) + ')'
        else:
            zone_str = ''

        # construct SQL min_probability clause
        if min_probability:
            prob_str = 'and t.probability > %f' % min_probability
        else:
            prob_str = ''

        minv_str = 'and t.v_tot > %f' % minv_degperday

        sql = """\
select  
    t.tracklet_id, t.v_ra, t.v_dec, t.v_tot, t.v_ra_sigma, t.v_dec_sigma, 
    t.pos_ang_deg, t.gcr_arcsec, t.ext_epoch, t.ext_ra, t.ext_dec, t.ext_mag, 
    t.probability, t.digest, t.status, t.known_id, t.field_id
from 
    detections d 
        join tracklet_attrib ta using (det_id)
        join tracklets t using (tracklet_id)
where 
    d.field_id=%s 
""" + ' '.join([zone_str, prob_str, minv_str]) + """
    and t.status=%s
    and 
        /* spherical distance between fields */
        acos(least(1.0, /* handle slight roundoff error causing arg > 1.0 */
            sin(radians(d.dec_deg)) * sin(%s)
            + cos(radians(d.dec_deg)) * cos(%s) * cos(radians(d.ra_deg) - %s)
        )) < %s
"""

        # Execute the SQL.
        cursor = dbh.cursor()
        n = cursor.execute(sql, (
            self._id, 
            MOPS.Constants.TRACKLET_STATUS_UNATTRIBUTED,
            pos.dec_deg * MOPS.Constants.RAD_PER_DEG,
            pos.dec_deg * MOPS.Constants.RAD_PER_DEG,
            pos.ra_deg * MOPS.Constants.RAD_PER_DEG,
            search_radius_rad,
        ))
        if not n:
            return []
        # <-- end if

        tracklets = []
        row = cursor.fetchone()
        while row:
            t = Tracklet(*list(row))
            t.fetchDetections(dbh)
            tracklets.append(t)

            # Next line.
            row = cursor.fetchone()
        # <-- end while

        return tracklets
    
