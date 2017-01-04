#!/usr/bin/env python2.7

import subprocess
import os
import datetime
import time

MOPS_DBINSTANCE = "psmops_ps1_mdrm152"
MOPS_HOME = "/home/mops/MOPS_STABLE"
ATLAS_MPCDES = "http://atlas-base-adm01/mpcorb/mpc.des"
LOCAL_MPCDES = "%s/data/mpcorb/mpc.des" % MOPS_HOME
ATLAS_MPCSQLITE = "http://atlas-base-adm01/mpcorb/mpc.sqlite3"
LOCAL_MPCSQLITE = "%s/data/mpcorb/mpc.sqlite3" % MOPS_HOME
OORB_2BODY_CONF = "%s/config/oorb.2body.conf" % MOPS_HOME
OORB_NBODY_CONF = "%s/config/oorb.nbody.conf" % MOPS_HOME
OORB = "%s/bin/oorb" % MOPS_HOME

########################################################################
def setup_environment():
    """Load the environment variables necessary for correct execution. 
    from http://stackoverflow.com/questions/20669558/how-to-make-subprocess-called-with-call-popen-inherit-environment-variables
    """
    process = subprocess.Popen("source /home/mopspipe/mops_env psmops_ps1_mdrm152; env", stdout=subprocess.PIPE, shell=True)
    process.wait()
    output = process.communicate()[0]
    env = dict((line.split("=", 1) for line in output.split('\n') if line))
    os.environ.update(env)
    return env

########################################################################
# This is the code of qknown for ephemerides preparation
def propagate_ephemerides(mjd, environment):
    logfile = open("%s/log/qknown.log" % environment["MOPS_VAR"], 'a')
    # Propagation
    tmp_orb = "%s/eph/tmp.%d.des" % (environment["MOPS_VAR"], mjd)
    arguments = [ OORB,
                  "--conf=%s" % OORB_2BODY_CONF,
                  "--task=propagation",
                  "--epoch-mjd-utc=%d" % mjd,
                  "--orb-in=%s" % LOCAL_MPCDES,
                  "--orb-out=%s" % tmp_orb]
    process = subprocess.Popen( arguments, 
                                env = environment,
                                stdout = subprocess.PIPE,
                                stderr = subprocess.PIPE)
    status = process.wait()
    out, err = process.communicate()
    if status != 0:
        logfile.write("Propagation [%s] may have failed\n" % " ".join(arguments))
        logfile.write("stderr = [%s]\n" % err)
        logfile.write("stdout = [%s]\n" % out)
        pass
    # Ephemerides
    eph = "%s/eph/%d.eph" % (environment["MOPS_VAR"], mjd)
    arguments = [ OORB,
                  "--conf=%s" % OORB_2BODY_CONF,
                  "--task=ephemeris",
                  "--code=500", # ? Shouldn't it be F51 or something?
                  "--orb-in=%s" % tmp_orb ]
    process = subprocess.Popen( arguments, 
                                env = environment,
                                stdout = subprocess.PIPE,
                                stderr = subprocess.PIPE)
    out, err = process.communicate()
    status = process.poll()
    if status != 0:
        logfile.write("Ephemerides [%s] may have failed\n" % " ".join(arguments))
        pass
    eph_file = open(eph, "w")
    for line in out.split(os.linesep):
        fields = line.split()
        if len(fields) >= 10:
            eph_file.write("%s %s %s %s %s %s %s %s\n" % (fields[0],
                                                          fields[1],
                                                          fields[2],
                                                          fields[4],
                                                          fields[5],
                                                          fields[7],
                                                          fields[8],
                                                          fields[9]))
            pass
        pass
    eph_file.flush()
    eph_file.close()
    os.remove(tmp_orb)
    logfile.close()
    return eph

########################################################################
def get_current_mjd():
    import calendar
    import time
    return (calendar.timegm(time.gmtime())/86400+40587)

########################################################################
def download_atlas_catalogs():
    import urllib
    urllib.urlretrieve(ATLAS_MPCDES, LOCAL_MPCDES)
    urllib.urlretrieve(ATLAS_MPCSQLITE, LOCAL_MPCSQLITE)
    pass

########################################################################
def secondsTillTomorrow(h, m):
    tomorrow = datetime.datetime.replace(datetime.datetime.now() + datetime.timedelta(days=1), 
                         hour=h, minute=m, second=0)
    delta = tomorrow - datetime.datetime.now()
    return delta.seconds

########################################################################
if __name__ == "__main__":
    environment = setup_environment()
    # For some reason cron and oorb don't like each other.
    # Sleep during 24 hours or so then run again...
    while True:
        download_atlas_catalogs()
        mjd = get_current_mjd()
        print "Running for ", mjd
        propagate_ephemerides(mjd, environment)
        # Evaluate time left to tomorrow... say 14h20
        time.sleep(secondsTillTomorrow(14, 20))
        pass
    pass

