"""
$Id$

Provides abstraction and services for the MOPS end-of-night EON
queue.  The queue is essentially a bin of derived objects that
have mutated and need to be sent through orbit identification and 
then precovery.

Note that this module simply provides procedural interfaces to
the database.  A real design would implement a queue object and
peel them off the queue, providing an update() method.
"""

__author__ = 'Larry Denneau, Institute for Astronomy, University of Hawaii'
__date__ = '$Date$'
__version__ = '$Revision$'[11:-2]


import os
import os.path
import logging

import MOPS.Constants


class EONQueueItem(object):
    """
    Placeholder class for results from retrieve().  It would be nice if
    this class could update the DB automagically when its status changed.
    """
    def __init__(self, derivedobjectId, eventId, status, fieldId=None):
        self.derivedobjectId = derivedobjectId
        self.eventId = eventId
        self.status = status
        self.fieldId = fieldId      # only available when retrieved from DB


def submit(dbh, derivedobjectId, eventId):
    """
    Place the specified derived object into the EON queue.
    """

    cursor = dbh.cursor()

    # First see if this object is already there.
    existing = cursor.execute("select derivedobject_id from eon_queue where derivedobject_id=%s", (derivedobjectId,))
    if (existing):
        # Yep, delete it.
        cursor.execute("delete from eon_queue where derivedobject_id=%s", (derivedobjectId,))

    # Insert the object.
    sql = """insert into eon_queue (derivedobject_id, event_id, status) values (%s, %s, %s)"""
    return cursor.execute(sql, (derivedobjectId, eventId, MOPS.Constants.EONQUEUE_STATUS_NEW))


def retrieve(dbh, status): 
    """
    Retrieve objects with the specified status from the queue.
    """
    cursor = dbh.cursor()
    sql = """\
select eonq.derivedobject_id, eonq.event_id, eonq.status, h.field_id 
from eon_queue eonq, history h where eonq.event_id=h.event_id and status=%s
"""
    cursor.execute(sql, (status,))
    results = cursor.fetchall()
    return [EONQueueItem(*row) for row in results]

   
def update(dbh, derivedobjectId, status):
    """
    Update the processing status of the specified object.
    """

    cursor = dbh.cursor()
    sql = """update eon_queue set status=%s where derivedobject_id=%s"""
    return cursor.execute(sql, (status, derivedobjectId))


def flush(dbh):
    """
    Rid our queue of pesky retired queue objects.
    """
    cursor = dbh.cursor()
    sql = """delete from eon_queue where status=%s"""
    return cursor.execute(sql, (MOPS.Constants.EONQUEUE_STATUS_RETIRED,))

