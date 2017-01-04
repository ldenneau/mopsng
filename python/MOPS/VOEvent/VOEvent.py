"""
VOEvent.VOEvent

Class that encapsulate a VOEvent

For more information ob the VOEvent format, please refer to
    http://www.ivoa.net/Documents/latest/VOEvent.html
"""
import os
import tempfile
try:
    import xml.etree.cElementTree as ET
except:
    import xml.etree.ElementTree as ET

import MOPS.Constants as Constants
#from MOPS.Field import Field
#from MOPS.Orbit import Orbit
#from MOPS.Detection import Detection
from MOPS.DerivedObject import DerivedObject
from MOPS.Exceptions import *
import MOPS.Lib

import mx.DateTime as datetime


# VOEvent imports
from ParseUtils import *
from OutputUtils import *


# Constants
NUM_EPHEMS = 10                         # number of geocentric ephems.


    

def _createDetection(ephem, src):
    """
    Create a MOPS.Detection instance from the ephemerides output.

    Internal use only

    @param ephem: a list of [ra, dec, mag, mjd, raErr, decErr] values
    @param src: the SRC of the orbit used to generate the ephemerides
    """
    if(src != None):
        (ra, dec, mag, mjd, raErr, decErr) = ephem[:6]
        raErr /= Constants.ARCSECONDS_PER_DEG
        decErr /= Constants.ARCSECONDS_PER_DEG
    else:
        (ra, dec, mag, mjd) = ephem[:4]
        raErr = None
        decErr = None
    # <-- end if
    return(Detection(None, ra, dec, mjd, mag, None, None, raErr, decErr,
                     None, None, None, None, None, None, None))




class VOEvent(object):
    """
    VOEvent

    Encapsulate a VOEvent packet.

    VOEvent identifiers are built according to the following rule:
      voevent.eventId = '%s_%d_%s_%s' %(voevent.instance,
                                        voevent.objectId,
                                        voevent.eventTime,
                                        voevent.ruleName)
    Where voevent is a VOEvent instance.
    """
    @classmethod
    def parse(cls, xml):
        """
        Class method: create a VOEvent instance given its XML rpresentation.

        @param xml: VOEvent XML represeantation string.
        """
        root = ET.XML(xml)

        # Fetch eventID and from it, instance name, derivedObjectID...
        eventId = root.get('ivorn').split('#', 1)[-1]
        (inst, objId, eventTime, ruleName) = eventId.rsplit('_', 3)

        # Fetch detection list, orbit info and ephem list.
        for child in root:
            tag = child.tag.split('}', 1)[-1]
            if(tag == 'WhereWhen'):
                (dets, orbit, ephems) = parseObsData(child)
                break
            # <-- end if
        # <-- end for
        
        # Now we have all the data to create a VOEvent instance.
        voevent = cls(instanceName=inst,
                      objectId=int(objId),
                      orbit=orbit,
                      detections=dets,
                      ruleName=ruleName,
                      eventTime=eventTime,
                      ephemerides=ephems)
        return(voevent)
        
        
    def __init__(self, instanceName, objectId, orbit, detections,
                 ruleName='', eventTime=None, ephemerides=[]):
        """
        Constructor

        @param instanceName: MOPS instance name
        @param objectId: DerivedObject id
        @param orbit: MOPS Orbit instance
        @param detections: list of MOPS Detection instances
        @param ruleName: name of the alert rule that generated the event
        @param eventTime: either a mx.DateTime instance or its string
               representation. It has to be a date time string in ISO format.
               The times are assumed to be UTC.
        @param ephemerides: a list of MOPS.Detection instances that represent
               predicted positions of orbit. By convention, ephemerides should
               contain NUM_EPHEMS ephemerides, computed at 0h UTC for NUM_EPHEMS
               days after eventTime.
        """
        # Deconstruct derivedObject to extract what we need: orbit & detections.
        self.objectId = objectId
        self.orbit = orbit
        self.detections = detections
        self.instanceName = instanceName

        # Variables used to construct the event id.
        if(eventTime == None):
            self.eventTime = datetime.utc()
        else:
            self.eventTime = eventTime
        # <-- end if
        self.ruleName = ruleName
        self.ephemerides = ephemerides
        return


    def __getattr__(self, name):
        if(name == 'eventId'):
            return('%s_%d_%s_%s' %(self.instanceName,
                                   self.objectId,
                                   printDate(self.eventTime),
                                   self.ruleName))
        return(super(VOEvent, self).__getattr__(name))


    def __setattr__(self, name, value):
        if(name == 'eventId'):
            try:
                (inst, objId, eventTime, ruleName) = value.rsplit('_', 3)
                objId = int(objId)
            except:
                raise(Exception('Malformed eventId (%s).' %(value)))
            if(self.objectId != None and objId != self.objectId):
                raise(Exception('Malformed eventId (%s).' %(value)))
            # <-- end if
            self.objectId = objId
            self.instanceName = inst
            self.eventTime = eventTime
            self.ruleName = ruleName
            return
        elif(name == 'orbit'):
            self.ephemerides = []
        elif(name == 'eventTime'):
            self.ephemerides = []
            if(isinstance(value, str)):
                setattr(self,
                        name,
                        datetime.Parser.DateTimeFromString(value,
                                                           formats=['iso']))
                return
            # <-- end if
        # <-- end if
        return(super(VOEvent, self).__setattr__(name, value))


    def __repr__(self):
        """
        Return the natural (i.e. XML) representation of the VOEvent instance.
        """
        return(composeXML(self.orbit,
                           self.detections,
                           self.computeGeocentriEphemerides(),
                           self.eventId,
                           self.eventTime))

    
    def totext(self):
        """
        Return a human readable representation of the VOEvent instance.
        """
        return(composeText(self.orbit,
                            self.detections,
                            self.computeGeocentriEphemerides(),
                            self.eventId,
                            self.eventTime))
    

    def computeGeocentriEphemerides(self):
        """
        Return geocentric ephemerides at 0h UTC for NUM_EPHEMS days after
        self.eventTime. The ephemerides are instances of MOPS.Detection.

        Units are degrees for angles.
        """
        import numpy
        import ssd

        
        if(self.ephemerides):
            return(self.ephemerides)
        # <-- end if
        
        if(not self.orbit):
            # No orbit!
            return
        # <-- end if
        if(not self.orbit.hasSrc()):
            src = None
        else:
            src = numpy.array(self.orbit.src)
        # <-- end if

        # Compute the MJDs (at 0h UT)
        thisMidnightUTMJD = int(round(self.eventTime.mjd))
        times = numpy.array(range(thisMidnightUTMJD,
                                  thisMidnightUTMJD + NUM_EPHEMS))

        # Compute the ephemerides.
        orbitElements = numpy.array([self.orbit.q,
                                     self.orbit.e,
                                     self.orbit.i,
                                     self.orbit.node,
                                     self.orbit.timePeri,
                                     self.orbit.argPeri])
        ephems = ssd.geocentricEphemerides(orbitElements,
                                           self.orbit.epoch,
                                           times,
                                           self.orbit.h_v,
                                           gMag=0.150,
                                           covariance=src)
        self.ephemerides = [_createDetection(e, src) for e in ephems]
        return(self.ephemerides)

    

        

