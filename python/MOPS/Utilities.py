"""
General purpose utilities.
"""
from math import floor
import MOPS.Constants
import os
import os.path
import logging



def mjd2jd(mjd):
    """
    MJD -> JD
    """
    return(mjd + MOPS.Constants.MJD2JD_OFFSET)


def js2mjd(jd):
    """
    JD -> MJD
    """
    return(jd - MOPS.Constants.MJD2JD_OFFSET)
    

def mjd2ocnum(mjd):
    return(jd2ocnum(mjd2jd(mjd)))


def ocnum2mjd(ocnum):
    return(jd2mjd(ocnum2jd(ocnum)))


def jd2ocnum(jd):
    """
    Convert a JD into a MOPS observing cycle.

    For convenience, MOPS aggregates observations according to an 'Observing
    Cycle' number. Observations between full moons generally share the same
    observing cycle number. The value of this number changes at the latest
    0h UT before a full moon.

    OC = floor((floor(t - 0.5) + 0.5 - tref2) / Tsyn)

    OC = for some Julian Date t, the largest integer less than or equal to the
    interval from reference time tref2 to the  latest 0h UT before t, divided by
    the synodic period Tsyn.

    This definition always changes value at 0h UT.

    floor(t - 0.5) + 0.5 == latest 0h UT prior to t, in JD
    tref = 2451564.69729, full moon 21 JAN 2000 (from Tholen)
    tref2 = 2451564.5, 0h UT prior to tref
    Tsyn = 29.53058867
    """
    return(int(floor((floor(jd - 0.5) + 0.5 - MOPS.Constants.OC_TREF_0HUT_JD) /\
                     MOPS.Constants.OC_SYNODIC_PERIOD)))


def ocnum2jd(ocnum):
    """
    Convert an integer OC number to the JD at which the OC begins. Note that \
    OC number is defined to change at UT=0h
    """
    ocnum = int(ocnum)
    return(MOPS.Constants.OC_TREF_0HUT_JD + \
           MOPS.Constants.OC_SYNODIC_PERIOD * ocnum)
    

def toString(anInstance, displayAttributeList):
    """
    Generate a sting listing the attributes and their values of the instance
    specified.
    
    @param anInstance: Instance of a class whose attributes are to be listed.
    @param displayAttributeList: list of attributes to be displayed. 
    """
    classLine = "%s.%s(%i):\n" % (anInstance.__class__.__module__, anInstance.__class__.__name__, id(anInstance))
    return (classLine + "\n".join(["  %s: %s" % (key.rjust(8), anInstance.__dict__[key]) for key in displayAttributeList]))


def getLogger(name=None, logFile=None, mode='a'):
    """
    Returns a logger object which can be used to output messages to a log file
    as well as to the console.
    
    @param name: Name of the logging channel
    @param logfile: Full path to the log file.
    @param mode: Mode to use when opening log file. Defaults to append.
    @return: Instance of the Logger class  
    @rtype:logging.Logger  
    """ 
    logger = logging.getLogger(name)
    formatter = logging.Formatter('%(asctime)s %(message)s', '%Y/%m/%d %H:%M:%S')

    # File handler.
    if logFile:
        fileHandler = logging.FileHandler(logFile, mode)
        fileHandler.setFormatter(formatter)
        logger.addHandler(fileHandler)
    # <-- end if

    # Console handler.
    consoleHandler = logging.StreamHandler()
    consoleHandler.setFormatter(formatter)
    logger.addHandler(consoleHandler) 

    return logger

class NullHandler(logging.Handler):
    def emit(self, record):
        pass
