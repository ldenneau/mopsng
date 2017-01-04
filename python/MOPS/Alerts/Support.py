#!/usr/bin/env python

"""
Some derivedobject support for alerts.  This code used to be in Precovery.py,
but that module has been retired.
"""

from __future__ import with_statement
from __future__ import division


from MOPS.Exceptions import ProcessingException
from MOPS.Orbit import Orbit
from MOPS.DerivedObject import DerivedObject


def fetchDerivedObject(dbh, derivedObjectId, fetchTracklets=True):
    """
    Given a database connection handle and a derivedobject id, fetch the
    relevant data from the database and return a DerivedObject instance.

    @param dbh: Database conection object
    @param derivedObjectId: MOPS.DerivedObject instance id
    @param fetchTracklets: boolean to decide whether or not to fetch the
           MOPS.DerivedObject instance Tracklets as well.
    """
    # Get two cursors from the DB connection.
    cursor = dbh.cursor()

    # Compose the SQL statement.
    sql = '''\
select distinct(do.derivedobject_id), do.object_name, 
do.status, do.stable_pass,
o.orbit_id,
o.q, o.e, o.i, o.node, o.arg_peri, o.time_peri, o.h_v,
o.epoch,
o.cov_01, o.cov_02, o.cov_03, o.cov_04, o.cov_05, o.cov_06,
o.cov_07, o.cov_08, o.cov_09, o.cov_10, o.cov_11, o.cov_12,
o.cov_13, o.cov_14, o.cov_15, o.cov_16, o.cov_17, o.cov_18,
o.cov_19, o.cov_20, o.cov_21,
o.residual, o.chi_squared, o.moid_1, o.conv_code, o.moid_2
from derivedobjects do, orbits o where
do.derivedobject_id=%s and
do.orbit_id = o.orbit_id
'''

    # Execute it!
    n = cursor.execute(sql, (derivedObjectId,))
    if not n:
        raise ProcessingException("Got empty derived object back for DO ID %d" \
                                  % (int(derivedObjectId)))

    row = list(cursor.fetchone())

    # Create the Orbit instance.  Note [row[13:34]] is a list of the SRC entries.
    o = Orbit(*(row[4:13] + [row[13:34]] + row[34:]))

    # Create a bare-bone DerivedObject.
    dobj = DerivedObject(row[0], row[1], row[2], row[3], o)
    
    # Fetch the detections and the field.
    if fetchTracklets:
        dobj.fetchTracklets(dbh)
    # <-- end if
    return dobj

def fetchAllDerivedObjects(dbh, fetchTracklets=True):
    """
    Given a database connection handle fetch the relevant data from the database
    and return an iterator for all returned derived objects.

    @param dbh: Database conection object
    @param fetchTracklets: boolean to decide whether or not to fetch the
           MOPS.DerivedObject instance Tracklets as well.
    """
    # Get two cursors from the DB connection.
    cursor = dbh.cursor()

    # Compose the SQL statement (we exclude merged objects).
    sql = '''\
select distinct(do.derivedobject_id), do.object_name, o.orbit_id,
o.q, o.e, o.i, o.node, o.arg_peri, o.time_peri, o.h_v,
o.epoch,
o.cov_01, o.cov_02, o.cov_03, o.cov_04, o.cov_05, o.cov_06,
o.cov_07, o.cov_08, o.cov_09, o.cov_10, o.cov_11, o.cov_12,
o.cov_13, o.cov_14, o.cov_15, o.cov_16, o.cov_17, o.cov_18,
o.cov_19, o.cov_20, o.cov_21,
o.residual, o.chi_squared, o.moid_1, o.conv_code, o.moid_2,
do.status, do.stable_pass
from derivedobjects do, orbits o where
do.orbit_id = o.orbit_id and do.status != "M"
'''
    # Execute it!
    n = cursor.execute(sql)
    if not n:
        raise ProcessingException("Got empty derived object back for DO ID %d" \
                                  % (int(derivedObjectId)))

    row = list(cursor.fetchone())
    while(row):
        # Create the Orbit instance.
        o = Orbit(*(row[2:11] + [row[11:32]] + row[32:37]))

        # Create a bare-bone DerivedObject.
        dobj = DerivedObject(row[0], row[1], row[37], row[38], o)
    
        # Fetch the detections and the field.
        if fetchTracklets:
            dobj.fetchTracklets(dbh)
        # <-- end if

        # Yield the newly create DO instance.
        yield dobj

        # Go to the next row,
        row = cursor.fetchone()
        if row:
            row = list(row)
    # <-- end if
    return
