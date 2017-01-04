"""
$Id$

Provides abstraction and services for the MOPS alert queue.
The queue is essentially a bin of objects that have mutated and need to
be sent through alert generation.

Note that this module simply provides procedural interfaces to
the database.  A real design would implement a queue object and
peel them off the queue, providing an update() method.
"""

__date__ = '$Date$'
__version__ = '$Revision$'[11:-3]


import os
import os.path
import logging

import MOPS.Constants
import MOPS.Alerts.Alert as Alert
import MOPS.Alerts.Constants
from MOPS.Instance import Instance

class AQueue(object):    
    """
    Placeholder class for results from retrieve().  It would be nice if
    this class could update the DB automagically when its status changed.
    """

    #--------------------------------------------------------------------------   
    # Class variables.
    #--------------------------------------------------------------------------              
    
    # class variable which stores a database handle to the database hosting
    # the aqueue table. Database is specified by the AQUEUE_DATABASE variable
    # in the MOPS.Constants module.
    inst= Instance(MOPS.Alerts.Constants.AQUEUE_DB_NAME)
    dbh = inst.get_dbh()
    cursor = dbh.cursor()

    #--------------------------------------------------------------------------   
    # Class methods.
    #--------------------------------------------------------------------------              
    
    @classmethod
    def save(cls, aObj): 
        """
        Place the specified alert object into the alert queue.
        
        @param aObj: Alert object that is to be added to the aQueue table.
        """
        aObj.save()
    # <-- end save

    @classmethod
    def retrieve(cls, status, objectType=None, includeSyntheticObjects=False):
        """
        Retrieve alerts that match the status and object type given. Include
        alerts for synthetic objects as well if specified.
        
        @param status: Status that retrieved alert queue objects must have.
        @param objectType: Type of alert object to retrieve. Can be either
                           tracklet or derived object.
        @param includeSyntheticObjects: Indicate if alerts for synthetic objects
                                        should also be retrieved.
        @return: A list of AlertObj objects.
        @rtype: AlertObj[] 
        """
        sql = """select alert_id, 
                   status,
                   object_id,
                   object_type, 
                   db_name,
                   classification 
                from aqueue
                %s"""
        
        if(includeSyntheticObjects):
            # retrieve alerts for real and synthetic objects
            if (objectType != None):
                # retrieve alerts that are of the type specified
                where = "where status=%s and object_type=%s"
                cls.cursor.execute(sql % where, (status, objectType))
            else:
                where = "where status=%s"              
                cls.cursor.execute(sql % where, (status))
            # <-- end if
        else:
            # retrieve alerts for real objects only
            if (objectType != None):
                where = "where classification=%s and status=%s and object_type=%s"
                cls.cursor.execute(sql % where, (MOPS.Constants.MOPS_EFF_NONSYNTHETIC, 
                                             status, objectType))
            else:
                where = "where classification=%s and status=%s"
                cls.cursor.execute(sql % where, 
                               (MOPS.Constants.MOPS_EFF_NONSYNTHETIC, status))
            # <-- end if
        # <-- end if
            
        results = cls.cursor.fetchall()
        return [Alert.AlertObj(*row) for row in results]
    # <-- end retrieve

    @classmethod
    def update(cls, alertId, status):
        """
        Update the processing status of the specified alert.
        
        @param alertId: Unique id that identifies the alert object in the
                        aqueue table.
                        
        @param status: The new status of the alert object.
        """
        cursor = cls.dbh.cursor()

        sql = """update aqueue set status=%s where alert_id=%s"""
        n = cls.cursor.execute(sql, (status, alertId))
        return(cls.dbh.commit())
    # <-- end update
    
    @classmethod
    def prepareForProcessing(cls, objectType=None):
        """
        Mark all unprocessed items as ready for processing.
        
        @param objectType: A string specifying an alert object type. If this
                           parameter is provided then only alert objects that 
                           match the object type will be prepared for 
                           processing. If it is not provided then all objects
                           regardless of type will be prepared.
                           
        @return: Status of commit of updates to database.
        """
        

        # Now setup for processing.
        if (objectType == None):
            sql = "update aqueue set status=%s where status=%s"
            n = cls.cursor.execute(sql, (MOPS.Alerts.Constants.AQUEUE_STATUS_READY, 
                                     MOPS.Alerts.Constants.AQUEUE_STATUS_NEW))
        else:
            sql = """update aqueue set status=%s 
                     where status=%s and object_type=%s"""
            n = cls.cursor.execute(sql, (MOPS.Alerts.Constants.AQUEUE_STATUS_READY,
                             MOPS.Alerts.Constants.AQUEUE_STATUS_NEW, objectType))
        # <-- end if            
        return(cls.dbh.commit())
    # <-- end prepareForProcessing    

    @classmethod
    def doneWithProcessing(cls, objectType=None):
        """
        Mark everything with status 'R' as processed.
        
        @param objectType: A string specifying an alert object type. If this
                           parameter is provided then only alert objects that 
                           match the object type will be prepared for 
                           processing. If it is not provided then all objects
                           regardless of type will be prepared.
        """
        cursor = cls.dbh.cursor()
        if (objectType == None):
            sql = """update aqueue set status=%s where status=%s"""
            n = cls.cursor.execute(sql, (MOPS.Alerts.Constants.AQUEUE_STATUS_DONE,
                             MOPS.Alerts.Constants.AQUEUE_STATUS_READY))
        else:
            sql = """update aqueue set status=%s where status=%s and object_type=%s"""
            n = cls.cursor.execute(sql, (MOPS.Alerts.Constants.AQUEUE_STATUS_DONE,
                             MOPS.Alerts.Constants.AQUEUE_STATUS_READY, objectType))
        # <-- end if            
        return(cls.dbh.commit())
    # <-- end doneWithProcessing

    @classmethod
    def resetProcessing(cls, objectType=None):
        """
        Mark all items marked as ready for processing back to new.
        
        @param objectType: A string specifying an alert object type. If this
                           parameter is provided then only alert objects that 
                           match the object type will be prepared for 
                           processing. If it is not provided then all objects
                           regardless of type will be prepared.
        """
        cursor = cls.dbh.cursor()
        if (objectType == None):
            sql = """update aqueue set status=%s where status=%s"""
            n = cls.cursor.execute(sql, (MOPS.Alerts.Constants.AQUEUE_STATUS_NEW,
                             MOPS.Alerts.Constants.AQUEUE_STATUS_READY))
        else:
            sql = """update aqueue set status=%s where status=%s and object_type=%s"""
            n = cls.cursor.execute(sql, (MOPS.Alerts.Constants.AQUEUE_STATUS_NEW,
                             MOPS.Alerts.Constants.AQUEUE_STATUS_READY, objectType))
        # <-- end if
        return(cls.dbh.commit())
    # <-- end resetProcessing
# <-- end class