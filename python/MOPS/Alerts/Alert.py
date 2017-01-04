"""
$Id$

Provides abstraction and services for the MOPS Alert DB table.

Note that this module simply provides procedural interfaces to
the database.
"""
import logging
import MOPS.Constants
import MOPS.Alerts.Constants
import MOPS.Alerts.Exceptions
import MOPS.Utilities as Utilities
import pprint

from MOPS.DerivedObject import DerivedObject
from MOPS.Tracklet import Tracklet
from MOPS.Instance import Instance

__all__ = ['TrackletAlert', 'DerivedObjAlert', 'Alert']

#--------------------------------------------------------------------------   
# Global variables.
#--------------------------------------------------------------------------              

# Get logger and set logging level. Typically set level to info.
# Add null handler to logger to avoid the No handlers could be found
# for logger XXX error if a handler is not found in a higher level logger
logger = logging.getLogger('MOPS.Alert')
h = Utilities.NullHandler()
logger.addHandler(h)

class AlertObj(object):
    """
    This class is a representation of a record stored in the aQueue database
    table. 
    """
    #--------------------------------------------------------------------------   
    # Class variables.
    #--------------------------------------------------------------------------              

    #TODO: add code here that verifies that a database handle was 
    #successfully retrieved.
    inst = Instance(MOPS.Alerts.Constants.AQUEUE_DB_NAME)
    dbh = inst.get_dbh()

    #--------------------------------------------------------------------------   
    # Instance methods.
    #--------------------------------------------------------------------------              
    def __init__(self, alertId, status, objId, objType, dbname, 
                 classification=MOPS.Constants.MOPS_EFF_NONSYNTHETIC, msg=None,
                 subject=None):
        self._alertId = alertId
        self._alertStatus = status
        self._objId = objId
        self._objType = objType
        self._dbname = dbname
        self._classification = classification
        self._msg = msg
        self._subject = subject
    # <-- end __init__

    def __str__(self):
        showList = sorted(set(self.__dict__))
        return Utilities.toString(self, showList)
    # <-- end __str__

    def save():
        """
        Place the alert object into the aqueue table.
        
        @return: Returns 1 if the alert object was successfully saved to the
                 aqueue table.
                 
        @rtype: integer
        """      
        cursor = AlertObj.dbh.cursor()

        # First see if this object is already there. It it is then remove it.
        existing = cursor.execute(
            "select alert_id from aqueue where alert_id=%s", 
            (self._alertId))
        if (existing):
            cursor.execute("delete from aqueue where alert_id=%s", 
                           (self._alertId,))
        # <-- end if
        # Insert the object.
        sql = """insert into aqueue 
            (alert_id, status, object_id, object_type, 
            dbname, classification) values (%s, %s, %s, %s, %s, %s, %s)"""
        results = cursor.execute(sql, (self._alertId, \
                                    MOPS.Alerts.Constants.AQUEUE_STATUS_NEW, \
                                    self._objId, self._objType, \
                                    self._database, self._classification))
        AlertObj.dbh.commit()
        cursor.close()
        return (results)
    # <-- end save
    
    #--------------------------------------------------------------------------   
    # Getter/Setter methods.
    #--------------------------------------------------------------------------   
    def getAlertId(self):
        """ 
        Unique integer that identifies the alert.
        """        
        return self._alertId
    # <-- end getAlertID
         
    def setAlertId(self, value):
            self._alertId = value
    # <-- end setAlertID
    alertId = property(getAlertId, setAlertId)
        
    def getStatus(self):
        """
        Status of alert. Allowed values are defined in MOPS.Alerts.Constants
        """
        return self._alertStatus
    # <-- end getStatus
     
    def setStatus(self, value):
        if (value in MOPS.Alerts.Constants.AQUEUE_ALLOWED_STATUS):
            self._alertStatus = value
        else:
            raise MOPS.Alerts.Exceptions.AlertException("%s is not a valid alert status" % value)
        # <-- end if
    # <-- end setStatus
    status = property(getStatus, setStatus)
    
    def getObjId(self):
        """
        Unique integer that identifies actual subject of the alert.
        Current possible subjects are Derived Objects and Tracklets
        """
        return self._objId
    # <-- end getObjId

    def setObjId(self, value):
        self._objId = value
    # <-- end setObjId
    objId = property(getObjId, setObjId)
    
    def getObjType(self):
        """
        Single character that specifies the type of the alert.
        Current possible types are defined in MOPS.Alerts.Constants
        """
        return self._objType
    # <-- end getObjType

    def setObjType(self, value):
        if (value in MOPS.Alerts.Constants.AQUEUE_ALLOWED_TYPE):
            self._objType = value
        else:
            raise MOPS.Alerts.Exceptions.AlertException("%s is not a valid alert type" % value)
        # <-- end if
    # <-- end setObjType
    objType = property(getObjType, setObjType)
    
    def getDbName(self):
        """
        Name of the database schema that contains the alert.
        """
        return self._dbname
    # <-- end getDbName

    def setDbName(self, value):
        self._dbname = value
    # <-- end setDbName
    dbname = property(getDbName, setDbName)
    
    def getClassification(self):
        """
        Single character that indicates if the alert is for a 
        synthetic or real object. Allowed values are defined in 
        MOPS.Constants.
        """     
        return self._classification
    # <-- end getClassification

    def setClassification(self, value):
        self._classification = value
    # <-- end setClassification
    classification = property(getClassification, setClassification)
    
    def getMessage(self):
        """
        Text to be included in the published alert.
        """
        return self._msg
    # <-- end getMessage

    def setMessage(self, value):
        self._msg = value
    # <-- end setMessage
    message = property(getMessage, setMessage)

    def getSubject(self):
        """
        Subject to be included in the published alert.
        """
        return self._subject
    # <-- end getSubject

    def setSubject(self, value):
        self._subject = value
    # <-- end setSubject
    subject = property(getSubject, setSubject)

# <-- end class 


class TrackletAlert(AlertObj):
    def __init__(self, a):
        super(TrackletAlert, self).__init__(a.alertId, a.status, 
                                            a.objId, a.objType, a.dbname, 
                                            a.classification, a.message, 
                                            a.subject)
        # TODO: Enclose following code in a try catch block that will throw
        # an exception if the objType is not a tracklet object.
        self._tracklet = None
    # <-- end __init__
    
    def __str__(self):
        showList = sorted(set(self.__dict__))
        return Utilities.toString(self, showList)
    # <-- end __str__

    @property
    def alertObj(self):
        """
        Tracklet that is the subject of the alert.
        """  
        if (self._tracklet == None):
            # Set the default(current) database on the connection to 
            # that containing the subject of the alert. Once done 
            # restore the database.
            AlertObj.dbh.select_db(self._dbname)
            self._tracklet = Tracklet.retrieve(AlertObj.dbh, 
                                               self._objId, False)
            AlertObj.dbh.select_db(MOPS.Alerts.Constants.AQUEUE_DB_NAME)
        # <-- end if
        return self._tracklet
    # <-- end alertObj
# <-- end TrackletAlert


class DerivedObjAlert(AlertObj):
    def __init__(self, a):
        super(DerivedObjAlert, self).__init__(a.alertId, a.status, 
                                              a.objId, a.objType, a.dbname, 
                                              a.classification, a.message)
        # TODO: Enclose following code in a try catch block that will throw
        # an exception if the objType is not a derived object.
        self._derivedObj = None
    # <-- end __init__

    def __str__(self):
        showList = sorted(set(self.__dict__))
        return Utilities.toString(self, showList)
    # <-- end __str__
    
    @property
    def alertObj(self): 
        """
        Derived Object that is the subject of the alert.
        """
        if (self._derivedObj == None):
            # Set the default(current) database on the connection to 
            # that containing the subject of the alert. Once done 
            # restore the database.
            AlertObj.dbh.select_db(self._dbname)
            self._derivedObj = DerivedObject.fetch(AlertObj.dbh, 
                                                   self._objId, 
                                                   fetchTracklets=True)
            AlertObj.dbh.select_db(MOPS.Alerts.Constants.AQUEUE_DB_NAME)
        # <-- end if
        return self._derivedObj
    # <-- end alertObj
# <-- end DerivedObjAlert
        

class Alert(object):
    """
    This class acts as an interface with the Alerts table in the Alert database.
    
    Using this class you can retrieve data from the alerts view, and insert
    data into the alert table which underlies the alerts view.
    """
    #--------------------------------------------------------------------------   
    # Class variables.
    #--------------------------------------------------------------------------              
    
    #TODO: add code here that verifies that a database handle was 
    #successfully retrieved.
    inst = Instance(MOPS.Alerts.Constants.AQUEUE_DB_NAME)
    dbh = inst.get_dbh()

    #--------------------------------------------------------------------------   
    # Instance methods.
    #--------------------------------------------------------------------------                  
    def __init__(self, alertId, rule, channel, status, objId, objType,
                 dbname, classification, msg=None, subject=None, voEvent=None):
        self._alertId = alertId
        self._rule = rule
        self._channel = channel
        self._status = status
        self._objId = objId
        self._objType = objType
        self._dbname = dbname
        self._classification = classification
        self._voEvent = voEvent
        self._msg = msg
        self._subject = subject
    # <-- end __init__

    def __str__(self):
        showList = sorted(set(self.__dict__))
        return Utilities.toString(self, showList)
    # <-- end __str__

    def save(self):
        """
        Insert the alert into the Alerts table.
        
        @return: Returns 1 if the alert object was successfully saved to the
                 aqueue table.                 
        @rtype: integer
        """
        if (self._alertId == None or self._rule == None or 
            self._channel == None or self._status == None):
            raise MOPS.Alerts.Exceptions.AlertException(
                "When saving an alert values for the alert_id, \
                rule, channel, and status must be specified.")
        # <-- end if
        cursor = Alert.dbh.cursor()

        # First see if this object is already there. If it is delete it.
        existing = cursor.execute(
            "select alert_id from alerts where alert_id=%s and rule=%s and channel=%s", 
            (self._alertId, self._rule.lower(), self._channel.lower()))
        if (existing):
            cursor.execute("delete from alerts where alert_id=%s", 
                           (self._alertId,))
        # <-- end if
        # Insert the object.
        sql = """insert into alerts 
            (alert_id, rule, channel, status, vo_event, message, subject) 
            values (%s, %s, %s, %s, %s, %s, %s)"""
        result = cursor.execute(sql, (self._alertId, self._rule.lower(), 
                                      self._channel.lower(), self._status, 
                                      self._voEvent, self._msg, self._subject))
        Alert.dbh.commit()
        cursor.close()
        return result           
    # <-- end save
    
    def updateStatus(self, newStatus):
        """
        Set the status of the alert to the value given by newStatus.
        
        @param newStatus: A string containing the new status for the alert.
        
        @return: Result of the status update. 
        """
        if (newStatus not in MOPS.Alerts.Constants.AQUEUE_ALLOWED_STATUS):
            raise MOPS.Alerts.Exceptions.AlertException("%s is not a valid alert status" % newStatus)
        # <-- end if
        
        cursor = Alert.dbh.cursor()
        if (newStatus == MOPS.Alerts.Constants.AQUEUE_STATUS_DONE):
            # Set pub_timestamp if status is being set to done.
            sql = "update alerts set status=%s, pub_timestamp = NOW() where alert_id=%s"
        else:
            sql = "update alerts set status=%s where alert_id=%s"
        # <-- end if
        
        n = cursor.execute(sql, (newStatus, self._alertId))
        cursor.close()
        return(Alert.dbh.commit())
    # <-- end updateStatus

    def updateMessage(self, msg):
        """
        Set the text of the alert to the value provided.
        
        @param msg: A string containing text to be included in the alert  
                    generated.
        
        @return: Result of the message update. 
        """        
        cursor = Alert.dbh.cursor()
        sql = "update alerts set message=%s where alert_id=%s"
        
        n = cursor.execute(sql, (msg, self._alertId))
        cursor.close()
        return(Alert.dbh.commit())
    # <-- end updateMessage

    def updateVoEvent(self, voEvent):
        """
        Set the voEvent of the alert to the value provided.
        
        @param voEvent: A string containing the xml representation of the 
                        voEvent.
        
        @return: Result of the voEvent update. 
        """        
        cursor = Alert.dbh.cursor()
        sql = "update alerts set vo_event=%s where alert_id=%s"
        
        n = cursor.execute(sql, (voEvent, self._alertId))
        cursor.close()
        return(Alert.dbh.commit())
    # <-- end updateVoEvent

    def updateSubject(self, subj):
        """
        Set the subject of the alert to the value provided.
        
        @param subj: A string containing subject to be included in the alert  
                    generated.
        
        @return: Result of the subject update. 
        """        
        cursor = Alert.dbh.cursor()
        sql = "update alerts set subject=%s where alert_id=%s"
        
        n = cursor.execute(sql, (subj, self._alertId))
        cursor.close()
        return(Alert.dbh.commit())
    # <-- end updateSubject
      
    #--------------------------------------------------------------------------   
    # Getter/Setter methods.
    #--------------------------------------------------------------------------   
    def getAlertId(self):
        """ Unique integer that identifies the alert."""        
        return self._alertId
    # <-- end getAlertID
         
    def setAlertId(self, value):
            self._alertId = value
    # <-- end setAlertID
    alertId = property(getAlertId, setAlertId)
    
    def getRule(self):
        """Name of rule that generated the alert."""
        return self._rule
    # <-- end getRule
        
    def setRule(self, value):
        self._rule = value
    # <-- end setRule
    rule = property(getRule, setRule)
    
    def getChannel(self):
        """Name of channel to use when publishing the alert."""
        return self._channel
    # <-- end getChannel

    def setChannel(self, value):
        self._channel = value
    # <-- end setChannel
    channel = property(getChannel, setChannel)
    
    def getStatus(self):
        """Status of alert. Allowed values are defined in MOPS.Alerts.Constants"""
        return self._status
    # <-- end getStatus
     
    def setStatus(self, value):
        if (value in MOPS.Alerts.Constants.AQUEUE_ALLOWED_STATUS):
            self._status = value
        else:
            raise MOPS.Alerts.Exceptions.AlertException("%s is not a valid alert status" % value)
        # <-- end if 
    # <-- end setStatus
    status = property(getStatus, setStatus)
    
    def getObjId(self):
        """
        Unique integer that identifies actual subject of the alert.
        Current possible subjects are Derived Objects and Tracklets
        """
        return self._objId
    # <-- end getObjId

    def setObjId(self, value):
        self._objId = value
    # <-- end setObjId
    objId = property(getObjId, setObjId)

    def getObjType(self):
        """ 
        Single character that specifies the type of the alert.
        Current possible types are defined in MOPS.Alerts.Constants
        """
        return self._objType
    # <-- end getObjType

    def setObjType(self, value):
        if (value in MOPS.Alerts.Constants.AQUEUE_ALLOWED_TYPE):
            self._objType = value
        else:
            raise MOPS.Alerts.Exceptions.AlertException("%s is not a valid alert type" % value)
        # <-- end if
    # <-- end setObjType
    objType = property(getObjType, setObjType)

    def getDbname(self):
        """
        Name of the database schema that contains the alert.
        """
        return self._dbname
    # <-- end getDbname

    def setDbname(self, value):
        self._dbname = value
    # <-- end setDbname
    dbname = property(getDbname, setDbname)

    def getClassification(self):
        """
        Single character that indicates if the alert is for a 
        synthetic or real object. Allowed values are defined in 
        MOPS.Constants.
        """     
        return self._classification
    # <-- end getClassification

    def setClassification(self, value):
        self._classification = value
    # <-- end setClassification
    classification = property(getClassification, setClassification)

    def getMessage(self):
        """
        Text to be included in the published alert.
        """
        return self._msg
    # <-- end getMessage

    def setMessage(self, value):
        self._msg = value
    # <-- end setMessage
    message = property(getMessage, setMessage)
    
    def getSubject(self):
        """
        Subject to be included in the published alert.
        """
        return self._subject
    # <-- end getSubject

    def setSubject(self, value):
        self._subject = value
    # <-- end setSubject
    subject = property(getSubject, setSubject)

 
    #--------------------------------------------------------------------------   
    # Class methods.
    #--------------------------------------------------------------------------  
    @classmethod
    def getAlerts(cls, status=None):
        """
        Retrieves all alerts from the alerts view that match the specified
        status.
        
        @param status: Status retrieved alerts must have.
        @return: List of all alerts whose status matches that provided.
        @rtype: Sequence containing Alerts 
        """
        cursor = cls.dbh.cursor()

        sql = """select alert_id, rule, channel, status, object_id, object_type,
               db_name, classification, message, subject from v_alerts %s"""
        
        if (status):
            #Status specified use in select statement.
            where = "where status=%s"
            n = cursor.execute(sql % where, (status))
        else:
            where = ""
            n = cursor.execute(sql % where)
        # <-- end if                  
        
        # Retrieve alerts
        alerts=[]
                       
        if(n):
            res = cursor.fetchall()
            alerts = [Alert(*row) for row in res]
        # <-- end if
        cursor.close()
        return alerts
    # <-- end getAlerts
    
    @classmethod
    def prepareForProcessing(cls, channel=None, rule=None):
        """
        Mark all unprocessed alerts as ready for processing.
        
        @param channel: A string specifying an alert channel. If this
                        parameter is provided then only alert with a matching
                        channel  will be prepared for processing.
                        
        @param rule:    A string containing a rule name. If this parameter is
                        provided then only alerts generated by the named rule
                        will be prepared for processing.
                           
        @return: Status of commit of updates to database.
        """
        cursor = cls.dbh.cursor()
               
        if (channel != None and rule != None):
            sql = """update alerts set status=%s 
                     where status=%s and channel = %s and rule = %s"""
            n = cursor.execute(sql, (MOPS.Alerts.Constants.AQUEUE_STATUS_READY, 
                                     MOPS.Alerts.Constants.AQUEUE_STATUS_NEW, 
                                     channel.lower(), rule.lower()))
            logger.debug("""update alerts set status = '%s' 
                     where status = '%s' and channel = '%s' and rule = '%s'""" %
                     (MOPS.Alerts.Constants.AQUEUE_STATUS_READY, 
                      MOPS.Alerts.Constants.AQUEUE_STATUS_NEW, 
                      channel.lower(), rule.lower()))
            
        elif (channel != None):
            sql = "update alerts set status=%s where status=%s and channel=%s"
            n = cursor.execute(sql, (MOPS.Alerts.Constants.AQUEUE_STATUS_READY, 
                                     MOPS.Alerts.Constants.AQUEUE_STATUS_NEW, 
                                     channel.lower()))
            logger.debug("update alerts set status = '%s' where status = '%s' and channel = '%s'" %
                         (MOPS.Alerts.Constants.AQUEUE_STATUS_READY, 
                          MOPS.Alerts.Constants.AQUEUE_STATUS_NEW, 
                          channel.lower()))
        elif (rule != None):
            sql = "update alerts set status=%s where status=%s and rule=%s" 
            n = cursor.execute(sql, (MOPS.Alerts.Constants.AQUEUE_STATUS_READY, 
                                     MOPS.Alerts.Constants.AQUEUE_STATUS_NEW, 
                                     rule.lower()))
            logger.debug("update alerts set status = '%s' where status = '%s' and rule = '%s'" %
                         (MOPS.Alerts.Constants.AQUEUE_STATUS_READY, 
                          MOPS.Alerts.Constants.AQUEUE_STATUS_NEW, 
                          rule.lower()))
            
        else:
            sql = "update alerts set status=%s where status=%s"
            n = cursor.execute(sql, (MOPS.Alerts.Constants.AQUEUE_STATUS_READY, 
                                     MOPS.Alerts.Constants.AQUEUE_STATUS_NEW))
            logger.debug("update alerts set status=%s where status=%s" %
                         (MOPS.Alerts.Constants.AQUEUE_STATUS_READY, 
                          MOPS.Alerts.Constants.AQUEUE_STATUS_NEW))
            
        # <-- end if
        logger.debug("Prepared %d alerts for processing" % n)
        cursor.close()
        return(cls.dbh.commit())
    # <-- end prepareForProcessing    

    @classmethod
    def doneWithProcessing(cls):
        """
        Mark everything with status 'R' as processed.
        """
        cursor = cls.dbh.cursor()
        sql = "update alerts set status=%s where status=%s"
        n = cursor.execute(sql, (MOPS.Alerts.Constants.AQUEUE_STATUS_DONE,
                                 MOPS.Alerts.Constants.AQUEUE_STATUS_READY))
        logger.debug("%d alerts processed" % n)
        cursor.close()
        return(cls.dbh.commit())
    # <-- end doneWithProcessing        
        
    @classmethod
    def resetProcessing(cls, channel=None, rule=None):
        """
        Mark all items marked as ready for processing back to new.
        
        @param channel: A string specifying an alert channel. If this
                        parameter is provided then only alert with a matching
                        channel  will be prepared for processing.
                        
        @param rule:    A string containing a rule name. If this parameter is
                        provided then only alerts generated by the named rule
                        will be prepared for processing.
                           
        @return: Status of commit of updates to database.
        """
        cursor = cls.dbh.cursor()
        if (channel != None and rule != None):
            sql = """update alerts set status=%s where status=%s and 
                    channel = %s and rule = %s""" \
                    % (MOPS.Alerts.Constants.AQUEUE_STATUS_NEW, 
                       MOPS.Alerts.Constants.AQUEUE_STATUS_READY, 
                       channel.lower(), rule.lower())
        elif (channel != None):
            sql = """update alerts set status=%s where status=%s and 
                    channel = %s""" \
                    % (MOPS.Alerts.Constants.AQUEUE_STATUS_NEW, 
                       MOPS.Alerts.Constants.AQUEUE_STATUS_READY, 
                       channel.lower())
        elif (rule != None):
            sql = """update alerts set status=%s where status=%s and 
                    rule = %s""" \
                    % (MOPS.Alerts.Constants.AQUEUE_STATUS_NEW, 
                       MOPS.Alerts.Constants.AQUEUE_STATUS_READY, 
                       rule.lower())
        else:
            sql = "update alerts set status=%s where status=%s" \
                    % (MOPS.Alerts.Constants.AQUEUE_STATUS_NEW, 
                       Constants.AQUEUE_STATUS_READY)
        # <-- end if    
        n = cursor.execute(sql)
        logger.debug("Prepared %d alerts for processing" % n)
        cursor.close()
        return(cls.dbh.commit())
    # <-- end resetProcessing
# <-- end Alert