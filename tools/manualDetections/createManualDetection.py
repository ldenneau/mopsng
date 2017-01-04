#!/usr/bin/env python

import sys
import os
import MySQLdb
from collections import deque
import traceback

########################################################
#
# ManualDetection
#
########################################################
class ManualDetection:
    def __init__(self):
        self.container = { }
        self.container["user"] = os.environ["USER"]
        try:
            self.container["dbname"] = os.environ["MOPS_DBINSTANCE"]
        except KeyError:
            pass # Ignore for the moment
        self.container["dbhost"] = "mops01"
        self.container["dbuser"] = "mopspipe"
        self.container["dbpassword"] = "epip"
        self.container["ddet"] = -1
        pass

    def __getitem__(self, key):
        return self.container[key]
    def __setitem__(self, key, value):
        self.container[key] = value
    def keys(self):
        return self.container.keys()

    @staticmethod
    def fromCommandLineArguments(arguments):
        remainingArguments = deque()
        md = ManualDetection()
        while len(arguments) != 0:
            currentArgument = arguments.popleft()
            if currentArgument.startswith("ra"):
                md["ra"] = currentArgument.split("=")[1]
                float(md["ra"]) # Make sure it's a float
            elif currentArgument.startswith("dec"):
                md["dec"] = currentArgument.split("=")[1]
                float(md["dec"]) # Make sure it's a float
            elif currentArgument.startswith("mag"):
                md["mag"] = currentArgument.split("=")[1]
                float(md["mag"]) # Make sure it's a float
            elif currentArgument.startswith("exp"):
                md.container["exp"] = currentArgument.split("=")[1]
            elif currentArgument.startswith("mjd"):
                md.container["mjd"] = float(currentArgument.split("=")[1])
            elif currentArgument.startswith("dtime"):
                value = currentArgument.split("=")[1]
                if value[-1] == "s":
                    md.container["dtime"] = float(value[:-1])/86400.
                else:
                    md.container["dtime"] = float(value)
            elif currentArgument.startswith("ddetId"):
                md.container["ddet"] = long(currentArgument.split("=")[1])
            elif currentArgument.startswith("user"):
                md.container["user"] = currentArgument.split("=")[1]
            else:
                remainingArguments.append(currentArgument)
                pass
        return (md, remainingArguments)

    def check(self):
        errors = [ ]
        if not "ra" in self.container.keys():
            errors.append("Error: ra undefined")
            pass
        if not "dec" in self.container.keys():
            errors.append("Error: dec undefined")
            pass
        if not "mag" in self.container.keys():
            errors.append("Error: mag undefined")
            pass
        if not "exp" in self.container.keys():
            errors.append("Error: exp undefined")
            pass
        if "mjd" in self.container.keys() and "dtime" in self.container.keys():
            errors.append("Error: cannot define both mjd and dtime")
            pass
        elif "mjd" in self.container.keys():
            print "mjd is set: will supersede exposure mjd"
            pass
        elif "dtime" in self.container.keys():
            print "dtime is set: will modify exposure mjd"
            pass
        else:
            pass
        return errors
    pass

########################################################
#
# processArguments
#
########################################################
def processArguments(commandLineArguments):
    arguments = deque(commandLineArguments)
    scriptName = arguments.popleft()
    (manualDetection, arguments) = ManualDetection.fromCommandLineArguments(arguments)
    while len(arguments) != 0:
        currentArgument = arguments.popleft()
        if currentArgument == "--instance":
            manualDetection.container["dbname"] = arguments.popleft()
        elif currentArgument == "--help" or currentArgument == "-h":
            usage(scriptName)
        else:
            raise Exception("Unknown argument: [%s]" % (currentArgument))
        pass
    if "dbname" not in manualDetection.keys():
        raise Exception("Need to either define MOPS_DBINSTANCE, or use --instance <dbname>")
    return manualDetection

########################################################
#
# processArguments
#
########################################################
def createDetection(detection):
    db = MySQLdb.connect(detection["dbhost"], 
                         detection["dbuser"], detection["dbpassword"],
                         detection["dbname"])
    cursor = db.cursor()
    query = "SELECT field_id, epoch_mjd, filter_id, obscode FROM fields WHERE fpa_id = '%s' ORDER BY field_id DESC LIMIT 1" % detection["exp"]
    cursor.execute(query)
    rows = cursor.fetchall()
    if len(rows) == 0:
        raise Exception("No exposure named [%s] in the database" % detection["exp"])
    fieldId = rows[0][0]
    filterId = rows[0][2]
    obsCode = rows[0][3]
    if not "mjd" in detection.keys():
        # Derive it from the exposure
        detection["mjd"] = rows[0][1]
        if "dtime" in detection.keys():
            detection["mjd"] += detection["dtime"]
            pass
        print "INFO: epoch MJD will be %.8f" % detection["mjd"]
        pass
    query = """
INSERT 
  detections(field_id, ra_deg, dec_deg, epoch_mjd, mag, filter_id, obscode, status)
VALUES
  (%d, %s, %s, %.8f, %s, '%s', '%s', 'F')
""" % (fieldId, detection["ra"], detection["dec"], detection["mjd"], 
       detection["mag"], filterId, obsCode)
    cursor.execute(query)
    query = "SELECT LAST_INSERT_ID()"
    cursor.execute(query)
    rows = cursor.fetchall()
    nDetId = rows[0][0]
    query = """
INSERT
   manual_detections(det_id, related_det_id, mode, creator)
VALUES
   (%d, %d, 'NEW', '%s')
""" % (nDetId, detection["ddet"], detection["user"])
    cursor.execute(query)
    db.commit()
    return nDetId

########################################################
#
# usage
#
########################################################
def usage(scriptName):
    print """

                       <scriptname>

    <scriptname> [--instance <dbname>] parameters

DESCRIPTION

    Ingest a detection built manually in a MOPS database.

OPTIONS
    --help, -h
        Displays this help and exit

    --instance <dbname>
        Optional if MOPS_DBINSTANCE is defined
        Force the creation of a detection in the MOPS database named 
        <dbname>, otherwise use MOPS_DBINSTANCE.
        If neither MOPS_DBINSTANCE, nor --instance <dbname> are present
        the program will exit with an error.
        If both MOPS_DBINSTANCE and --instance <dbname> are present,
        the program will use the value specified by --instance.

PARAMETERS

    Each parameter is created using a <keyword>=<sequence> value. Note
    that there is no space, no carriage return, or no tabulation in 
    a parameter.
    For instance:
        ra=12.3456 will be a valid parameter
        ra= 12.3456 will not be a valid parameter
    The order of the parameters does not matter.
    Mandatory parameters must be present.

    ra=<value> [mandatory]
        Value of the right ascension in decimal degrees
        e.g.:
            <scriptname> [...] ra=12.3

    dec=<value> [mandatory]
        Value of the declination in decimal degrees
        e.g.:
            <scriptname> [...] dec=4.56

    mag=<value> [mandatory]
        Value of the magnitude
        e.g.:
            <scriptname> [...] mag=22.2

    exp=<value> [mandatory]
        The name of the exposure where the detection is observed. The 
        exposure must exist in the database (or the program will exist
        with an error)
        e.g.:
            <scriptname> [...] exp=o7205g0123o

    mjd=<value> [optional, default value: special, cannot be used with the dtime parameter]
        The value of the epoch MJD for the detection.
        By default, if neither mjd nor dtime is specified, the epoch MJD of
        the detection is the epoch MJD of the exposure.
        e.g.:
            <scriptname> [...] mjd=57205.549052

    dtime=<value> [optional, default value: special, cannot be used with the mjd parameter]
        The value of the time difference compared to the epoch MJD of the
        exposure for the detection.
        If <value> finishes with an "s", the time difference is assumed to be
        in seconds.
        In <value> doest not finish with an "s", the time difference is assumed 
        to be in days.
        e.g.:
            <scriptname> [...] dtime=-0.00026041666666
            <scriptname> [...] dtime=-22.5s
        are equivalent
        By default, if neither mjd nor dtime is specified, the epoch MJD of
        the detection is the epoch MJD of the exposure.

    ddetId=<value> [optional, default value: 'undefined'/NULL]
        The det_id of a detection from which this detection is derived (could 
        be used for trailed objects), e.g.:
        The end of the detection named 123456 is on an edge. It would make sense 
        to create a detection by, e.g.:
            <scriptname> [...] ddetid=123456
        The detection must exist in the database or the program will exit with
        an error.

    user=<value> [optional, default value: env["USER"]]
        The name of the user that will be written in the database, not sent 
        to the MPC. 
        If not specified, the value of the USER environment variable (i.e. very 
        likely your login) will be used.
        e.g.:
            <scriptname> [...] user=lilly
        if you need to supersede schunova for instance.

QUESTIONS, COMMENTS, CORRECTIONS:

    Please send an e-mail to schastel@ifa.hawaii.edu

""".replace("<scriptname>", scriptName)
    sys.exit(0)

########################################################
#
# main
#
########################################################
if __name__ == "__main__":
    try:
        manualDetection = processArguments(sys.argv)
        errors = manualDetection.check()
        if len(errors) != 0:
            print
            for error in errors:
                print error
                pass
            raise Exception("\nUndefined parameters\n")
        detId = createDetection(manualDetection)
        print "Detection created with det_id = %d" % detId
    except Exception, e:
        print
        print "Error: %s" % e
        print
        traceback.print_exc()
        print
        print "Tip: Execute '%s -h' for help" % (sys.argv[0])
        print
    pass
