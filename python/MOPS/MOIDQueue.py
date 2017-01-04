"""
$Id$

Provides abstraction and services for the MOPS end-of-night MOID
queue.  The queue is essentially a bin of orbits that need to
be batch-processed by MOID software.

For now we just support the submit() class method, as everything else
is handled in the moid Perl program.
"""

__author__ = 'Larry Denneau, Institute for Astronomy, University of Hawaii'
__date__ = '$Date$'
__version__ = '$Revision$'[11:-2]


import os
import os.path
import logging

import MOPS.Constants


def submit(dbh, orbitId, eventId):
    """
    Place the specified orbit into the MOID queue.
    """

    cursor = dbh.cursor()

    # First see if this object is already there.
    existing = cursor.execute("select orbit_id from moid_queue where orbit_id=%s", (orbitId,))
    if (existing):
        # Yep, delete it.
        cursor.execute("delete from moid_queue where orbit_id=%s", (orbitId,))

    # Insert the object.
    sql = """insert into moid_queue (orbit_id, event_id) values (%s, %s)"""
    return cursor.execute(sql, (orbitId, eventId))
