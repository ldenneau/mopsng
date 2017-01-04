"""
$Id$

Provides abstraction and services for the MOPS alert queue.
The queue is essentially a bin of derived objects that have mutated and need to
be sent through alert generation.

Note that this module simply provides procedural interfaces to
the database.  A real design would implement a queue object and
peel them off the queue, providing an update() method.
"""

__date__ = '$Date$'
__version__ = '$Revision$'[11:-2]


import os
import os.path
import logging

import MOPS.Constants


class AQueueItem(object):
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
    Place the specified derived object into the alert queue.
    """

    cursor = dbh.cursor()

    # First see if this object is already there.
    existing = cursor.execute("select derivedobject_id from aqueue where derivedobject_id=%s", (derivedobjectId,))
    if (existing):
        # Yep, delete it.
        cursor.execute("delete from aqueue where derivedobject_id=%s", (derivedobjectId,))

    # Insert the object.
    sql = """insert into aqueue (derivedobject_id, event_id, status) values (%s, %s, %s)"""
    return cursor.execute(sql, (derivedobjectId, eventId, MOPS.Constants.AQUEUE_STATUS_NEW))


def retrieve(dbh, status, includeSyntheticObjects=True):
    """
    Retrieve objects with the specified status from the queue.
    """
    cursor = dbh.cursor()
    if(not includeSyntheticObjects):
        return(retrieveNonSyntheticObjects(dbh, status))
    # <-- end if
    
    sql = """\
select distinct(aq.derivedobject_id), 
       aq.event_id, 
       aq.status, 
       h.field_id 
from aqueue aq, 
     history h 
where aq.event_id=h.event_id and 
      aq.status=%s
"""
    cursor.execute(sql, (status,))
    results = cursor.fetchall()
    return [AQueueItem(*row) for row in results]


def retrieveNonSyntheticObjects(dbh, status):
    """
    Retrieve non synthetic objects with the specified status from the queue.
    """
    cursor = dbh.cursor()

    sql = """\
select distinct(aq.derivedobject_id), 
       aq.event_id, 
       aq.status, 
       h.field_id 
from aqueue aq, 
     history h, 
     derivedobjects do 
where aq.derivedobject_id=do.derivedobject_id and
      do.classification=%s and 
      aq.event_id=h.event_id and 
      aq.status=%s
"""
    cursor.execute(sql, (MOPS.Constants.MOPS_EFF_NONSYNTHETIC, status))
    results = cursor.fetchall()
    return [AQueueItem(*row) for row in results]

   
def update(dbh, derivedobjectId, status):
    """
    Update the processing status of the specified object.
    """

    cursor = dbh.cursor()
    sql = """update aqueue set status=%s where derivedobject_id=%s"""
    n = cursor.execute(sql, (status, derivedobjectId))
    return(dbh.commit())

def prepareForProcessing(dbh):
    """
    Mark all unprocessed items as ready for processing.
    """
    cursor = dbh.cursor()

    # Now setup for processing.
    sql = """update aqueue set status=%s where status=%s"""
    n = cursor.execute(sql, (MOPS.Constants.AQUEUE_STATUS_READY,
                             MOPS.Constants.AQUEUE_STATUS_NEW))
    return(dbh.commit())
    

def doneWithProcessing(dbh):
    """
    Mark everything with status 'R' as processed.
    """
    cursor = dbh.cursor()
    sql = """update aqueue set status=%s where status=%s"""
    n = cursor.execute(sql, (MOPS.Constants.AQUEUE_STATUS_DONE,
                             MOPS.Constants.AQUEUE_STATUS_READY))
    return(dbh.commit())


def resetProcessing(dbh):
    """
    Mark all items marked as ready for processing back to new.
    """
    cursor = dbh.cursor()
    sql = """update aqueue set status=%s where status=%s"""
    n = cursor.execute(sql, (MOPS.Constants.AQUEUE_STATUS_NEW,
                             MOPS.Constants.AQUEUE_STATUS_READY))
    return(dbh.commit())



