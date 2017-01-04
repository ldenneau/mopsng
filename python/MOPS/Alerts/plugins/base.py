"""
plugins.base

This module implements the base classes used to create alert plugins.
"""
import functools
import imp
import os
import sys
import logging

import MOPS.Instance as Instance
import MOPS.Alerts.Constants
import MOPS.Utilities as Utilities
import MOPS.Alerts.Alert as Alert
from MOPS.Alerts.Support import fetchDerivedObject, fetchAllDerivedObjects
from MOPS.Alerts.AQueue import AQueue

__all__ = ['Rule', 'TrackletRule', 'DerivedObjectRule']


# This is the base class for all plugins.
class Rule(object):
    """
    Base/abstract class for all MOPS alert plugins. It exports three instance
    variables and one method:
      alerts: a list of all known alert instances.
      newalerts: a list of newly created/modified alert instances.
      channel: the name of the alert channel to use
      evaluate(): evaluate the rule and return the list of alert
        instances that satisfy it.
    """
    
    #--------------------------------------------------------------------------   
    # Instance methods.
    #--------------------------------------------------------------------------              
    def __init__(self, includeSyntheticObjects=False, 
                 channel= MOPS.Alerts.Constants.ALERT_DEFAULT_CHANNEL, 
                 status=MOPS.Alerts.Constants.AQUEUE_STATUS_READY):
        """
        constructor
        @param includeSyntheticObjects: boolean that controls whether or not
               synthetic objects are included in the alert processing.
               Default False.
               
        @param channel: The channel that the rule will use to publish alerts 
        """
        self._newAlerts = None
        self._includeSyntheticObjects = includeSyntheticObjects
        self._channel = channel
        # This property was added to facilitate testing. This allows a rule
        # to be run against aqueue table entries with a status of done or new 
        # as well as ready.
        self._status = status 
        
        # Get logger and set logging level. Typically set level to info.
        # Add null handler to logger to aboid the "No handlers could be found
        # for logger XXX" error if a handler is not found in a higher level 
        # logger
        self.logger = logging.getLogger('MOPS.Alerts.plugins.base')
        h = Utilities.NullHandler()
        self.logger.addHandler(h)
        return
    # <-- end __init__

    def __str__(self):
        showList = sorted(set(self.__dict__))
        return Utilities.toString(self, showList)
    # <-- end __str__
        
    def cleanup(self):
        """
        Perform any clenup after the alerts have been successfully sent on the
        objects returned by self.evaluate()
        """
        return
    # <-- end cleanup
    
    def evaluate(self):
        """
        Return a list of alertObject instances that satisfy the rule.

        Subclasses HAVE to implement this method. The present implementation
        raises a NotImplementedError exception.
        """
        msg = 'The evaluate method has to be implemented by each subclass.'
        self.logger.error(msg)
        raise(NotImplementedError(msg))
    # <-- end evaluate

    #--------------------------------------------------------------------------   
    # Getter/Setter methods.
    #--------------------------------------------------------------------------   
    @property
    def newAlerts(self):
        """
        Implement lazy-ish parameter initialization.
        The alerts and newAlertObject instance variables will be 
        initialized only when they are first accessed, not when the class is 
        created.
        """
        return(self.getNewAlerts())
    # <-- end newAlerts 

    def getChannel(self):
        """Name of channel to use when publishing the alert."""
        return self._channel
    # <-- end getChannel

    def setChannel(self, value):
        self._channel = value
    # <-- end setChannel
    channel = property(getChannel, setChannel)

    def getNewAlerts(self):
        """
        Accessor method for self.newalerts
        @return list of new alert objects.
        Subclasses HAVE to implement this method. The present implementation
        raises a NotImplementedError exception.
        """
        msg = 'The getNewAlerts method has to be implemented by each subclass.'
        self.logger.error(msg)
        raise(NotImplementedError(msg))
    # <-- end getNewAlerts

    def _getNewAlerts(self):
        """
        Fetch the AQueue and retrieve its content in the form of a list of
        alert instances. Only fetch those queue items that are ready for
        processing.
        
        Subclasses HAVE to implement this method. The present implementation
        raises a NotImplementedError exception.
        """
        msg = 'The _getNewAlerts method has to be implemented by each subclass.'
        self.logger.error(msg)
        raise(NotImplementedError(msg))
    # <-- end _getNewAlerts
# <-- end class

class TrackletRule(Rule):
    """
    Base/abstract class for all MOPS alert plugins that monitor tracklet 
    objects.
    """
    #--------------------------------------------------------------------------   
    # Class variables.
    #--------------------------------------------------------------------------              
    
    # newObjCache is a class variable used to cache new/updated derivied or 
    # tracklet objects found during the current MOPS processing run. The cache
    # is a dictionary whose key is a three element tuple (status, type, synthetic)
    # Making the cache a dictionary allows it to seperately cache tracklets and
    # derived objects.
    __newObjCache = {}

    #--------------------------------------------------------------------------   
    # Instance methods.
    #--------------------------------------------------------------------------              
    def __init__(self,includeSyntheticObjects=False, 
                 channel=MOPS.Alerts.Constants.ALERT_DEFAULT_CHANNEL,
                 minArcLength=None,
                 status=MOPS.Alerts.Constants.AQUEUE_STATUS_READY):
        """
        @param includeSyntheticObjects: boolean that controls whether or not
               synthetic objects are included in the alert processing.
               Default False.
        @param channel: name of the channel to publish alerts to. Default 'all'
        """
        super(TrackletRule, self).__init__(includeSyntheticObjects, channel, status)
        return
    # <-- end __init__

    def __str__(self):
        showList = sorted(set(self.__dict__))
        return Utilities.toString(self, showList)
    # <-- end __str__
    
    def getNewAlerts(self):
        """
        Accessor method for self.newalerts
        """
        if(self._newAlerts == None):
            # Fetch all of the available alerts.
            self._newAlerts = [obj for obj in self._getNewAlerts()] 
        # <-- end if
        return(self._newAlerts)
    # <-- end getNewAlerts

    def _getNewAlerts(self):
        """
        Fetch the AQueue and retrieve its content in the form of a list of
        TrackletAlert instances. Only fetch those queue items that have the 
        status given
        """
        type = MOPS.Alerts.Constants.AQUEUE_TYPE_TRACKLET
        cacheKey = (self._status, self._includeSyntheticObjects)
        if TrackletRule.__newObjCache.has_key(cacheKey):
            # Alerts to be processed are already in cache.
            queue = TrackletRule.__newObjCache[cacheKey]
            self.logger.debug("%d new tracklets retrieved for processing from cache", len(queue))
        else:
            # Alerts to be processed are not in cache. Retrieve alerts and add 
            # them to the cache.
            TrackletRule.__newObjCache[cacheKey] = \
                [Alert.TrackletAlert(a)for a in AQueue.retrieve(self._status, 
                 type, self._includeSyntheticObjects)]
            queue = TrackletRule.__newObjCache[cacheKey]
            self.logger.debug("%d new tracklets retrieved for processing from database", len(queue))
        # <-- end if        
        return queue
    # <-- end _getNewAlerts
# <-- end class


class DerivedObjectRule(Rule):
    """
    Base/abstract class for all MOPS alert plugins that monitor derived objects.
    """
    #--------------------------------------------------------------------------   
    # Class variables.
    #--------------------------------------------------------------------------              
    
    # newObjCache is a class variable used to cache new/updated derivied or 
    # tracklet objects found during the current MOPS processing run. The cache
    # is a dictionary whose key is a three element tuple (status, type, synthetic)
    # Making the cache a dictionary allows it to seperately cache tracklets and
    # derived objects.
    __newObjCache = {}

    #--------------------------------------------------------------------------   
    # Instance methods.
    #--------------------------------------------------------------------------              
    def __init__(self, includeSyntheticObjects=False, 
                 channel=MOPS.Alerts.Constants.ALERT_DEFAULT_CHANNEL, 
                 minArcLength=None, 
                 status=MOPS.Alerts.Constants.AQUEUE_STATUS_READY):
        """
        @param channel: name of the channel to publish alerts to. Default 'all'
        @param includeSyntheticObjects: boolean that controls whether or not
               synthetic objects are included in the alert processing.
               Default False.
        @param minArcLength: if not None or 0, only operate on
               MOPS.DerivedObject instances which have an arc length or at least
               minArcLength days.
        """
        super(DerivedObjectRule, self).__init__(includeSyntheticObjects, 
                                                channel, status)
        self._minArcLength = minArcLength
        return
    # <-- end __init__

    def __str__(self):
        showList = sorted(set(self.__dict__))
        return Utilities.toString(self, showList)
    # <-- end __str__ 

    #--------------------------------------------------------------------------   
    # Getter/Setter methods.
    #--------------------------------------------------------------------------   
    def getMinArcLength(self):
        return(self._minArcLength)
    # <-- end getMinArcLength 

    def setMinArcLength(self, value):
        self._minArcLength = value
    # <-- end setMinArcLength 
    minArcLength = property(getMinArcLength, setMinArcLength)
       
    def getNewAlerts(self):
        """
        Accessor method for self.newalerts
        """
        if(self._newAlerts == None):
            if (self.minArcLength):
                self._newAlerts = \
                    [obj for obj in self._getNewAlerts() \
                     if float(obj.alertObj.arcLength()) >= float(self.minArcLength)]
                # Code below is for debugging purposes.
                #for obj in self._getNewAlerts():
                #    if float(obj.alertObj.arcLength()) >= float(self.minArcLength):
                #        self.logger.debug("Arc length min of %s met %s" %
                #                          (self.minArcLength,obj.alertObj.arcLength()))
                #    else :
                #        self.logger.debug("Arc length min of %s NOT met %s" %
                #                          (self.minArcLength,obj.alertObj.arcLength()))
            else:
                self._newAlerts = \
                    [obj for obj in self._getNewAlerts()]
            # <-- end if
        # <-- end if
        return(self._newAlerts)
    # <-- end getNewAlerts

    def _getNewAlerts(self):
        """
        Fetch AQueue and retrieve its content in the form of a list of
        alert instances. Only fetch those queue items that are ready for
        processing.
        """
        type = MOPS.Alerts.Constants.AQUEUE_TYPE_DERIVED
        cacheKey = (self._status, self._includeSyntheticObjects)
        if DerivedObjectRule.__newObjCache.has_key(cacheKey):
            # Alerts to be processed are already in cache.
            queue = DerivedObjectRule.__newObjCache[cacheKey]
            self.logger.debug("%d new derived objects retrieved for processing from cache", len(queue))
        else:
            # Alerts to be processed are not in cache. Retrieve alerts and add 
            # them to the cache.
            DerivedObjectRule.__newObjCache[cacheKey] = \
                [Alert.DerivedObjAlert(a)for a in AQueue.retrieve(self._status, type, self._includeSyntheticObjects)]         
            queue = DerivedObjectRule.__newObjCache[cacheKey]
            self.logger.debug("%d new derived objects retrieved for processing from database", len(queue))
        # <-- end if
        return queue
    # <-- end _getNewAlerts    
# <-- end class