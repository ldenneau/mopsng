"""
MOPS.VOEvent.OutputUtils

Pretty printing utilities. Internal use.
"""
from MOPS.Orbit import Orbit

from Templates import *


def composeXML(orbit, detections, ephemerides, eventId, eventTime):
    """
    Compose the XML representation of the VOEvent.

    @param orbit: MOPS.Orbit instance
    @param detections: list of MOPS.Detection instances
    @param ephemerides: list of MOPS.Detection instances
    @param eventId: string identifier for the event
    @param eventTime: UTC timestamp for the event
    """
    if(detections):
        detXML = '\n'.join([OBS_TMPLT % (d.mag, d.mjd,  d.ra, d.dec) \
                            for d in detections])
    else:
        detXML = ''
    if(ephemerides):
        ephemXML = '\n'.join([EPHEM_TMPLT % (e.mag,
                                             e.mjd,
                                             e.ra,
                                             e.dec,
                                             str(e.raErr),
                                             str(e.decErr)) \
                              for e in ephemerides])
    else:
        ephemXML = ''
    if(orbit and isinstance(orbit, Orbit)):
        # Do we have the orbit SRC?
        if(not orbit.hasSrc()):
            srcXML = 'None'
        else:
            srcXML = SRC_TMPLT % tuple(orbit.src)
        # <-- end if
        orbitXML = ORBIT_TMPLT % (orbit.epoch,
                                  orbit.q,
                                  orbit.e,
                                  orbit.i,
                                  orbit.node,
                                  orbit.argPeri,
                                  orbit.timePeri,
                                  srcXML,
                                  str(orbit.h_v),
                                  str(orbit.residuals),
                                  str(orbit.chiSq),
                                  str(orbit.convCode),
                                  str(orbit.moid_1),
                                  str(orbit.moid_2))
    else:
        orbitXML = ''
    # <-- end ifs

    # Create the final XML string.
    xml = VOEVENT_TMPLT % (eventId,
                           printDate(eventTime),
                           detXML,
                           orbitXML,
                           ephemXML)
    return(xml)


def composeText(orbit, detections, ephemerides, eventId, eventTime):
    """
    Compose the Human readable (no XML) representation of the VOEvent.

    @param orbit: MOPS.Orbit instance
    @param detections: list of MOPS.Detection instances
    @param ephemerides: list of MOPS.Detection instances
    @param eventId: string identifier for the event
    @param eventTime: UTC timestamp for the event
    """
    if(detections):
        detSTR = '\n'.join([OBS_TEXT_TMPLT % (d.mag, d.mjd,  d.ra, d.dec) \
                            for d in detections])
    else:
        detSTR = ''
    if(ephemerides):
        ephemSTR = '\n'.join([EPHEM_TEXT_TMPLT % (e.mag,
                                                  e.mjd,
                                                  e.ra,
                                                  e.dec,
                                                  str(e.raErr),
                                                  str(e.decErr)) \
                              for e in ephemerides])
    else:
        ephemSTR = ''
    if(orbit and isinstance(orbit, Orbit)):
        # Do we have the orbit SRC?
        if(not orbit.hasSrc()):
            srcSTR = 'None'
        else:
            srcSTR = str(orbit.src)
        # <-- end if
        orbitSTR = ORBIT_TEXT_TMPLT % (orbit.epoch,
                                       orbit.q,
                                       orbit.e,
                                       orbit.i,
                                       orbit.node,
                                       orbit.argPeri,
                                       orbit.timePeri,
                                       srcSTR,
                                       str(orbit.h_v),
                                       str(orbit.residuals),
                                       str(orbit.chiSq),
                                       str(orbit.convCode),
                                       str(orbit.moid_1),
                                       str(orbit.moid_2))
    else:
        orbitSTR = ''
    # <-- end ifs

    # Create the final string.
    strng = VOEVENT_TEXT_TMPLT % (eventId,
                                  printDate(eventTime),
                                  detSTR,
                                  orbitSTR,
                                  ephemSTR)
    return(strng)


def printDate(d):
    """
    Since mx.DateTime has a bug when the date ends with .59, we need to handle
    the string conversion ourselves.
    """
    return("%04d-%02d-%02dT%02d:%02d:%05.2f" \
           % (d.year, d.month, d.day, d.hour, d.minute, d.second))
