#!/usr/bin/env python2.7

import sys
import os
import errno
import calendar
import time
import logging
import logging.handlers
import subprocess
import tempfile
import shutil

STAGES = [ "MPCORB_DOWNLOAD", 
           "MPCORB_DES_CONVERSION",
           "PROPAGATION_TO_MJD",
           "PUBLISH"
           ]

#
# A reimplementation of the condoorb.sh suite
#
# Its purpose is to:
#  * Get the most recent MPCORB.DAT file from MPC
#  * Propagate it to some date
#  * Create a "tracks" file that gives the position of all objects at MJD+0, MJD+0.5, and MJD+1
#
def usage(script_name):
    print """
Usage: %s [-m MJD|-c OBSCODE|-o PATH|-s STAGE] [-h|-v] [ORBITS.DES]

  ORBITS.DES: optional DES-formatted file of orbits; if omitted, MPCORB.DAT downloaded from MPC
  -m MJD: propagate to time MJD, otherwise use current date
  -c OBSCODE: use OBSCODE as observatory code
  -o PATH: write outputs to PATH/MJD
  -s STAGE: start at STAGE (possible values: %s)
  -v : verbose (set -x)
  -h : help

""" % (script_name, ", ".join(STAGES))

########################################################################
def setupLogging(parameters):
    log_filename = parameters.output_directory + "/condoorb.log"
    Logger = logging.getLogger('MyLogger')
    Logger.setLevel(logging.DEBUG)
    # Add the log message handler to the logger
    handler = logging.handlers.RotatingFileHandler( log_filename,
                                                    maxBytes=10000000,
                                                    backupCount=2 )
    formatter = logging.Formatter('%(asctime)s | %(levelname)8s | %(message)s')
    handler.setFormatter(formatter)
    Logger.addHandler(handler)
    return Logger

########################################################################
def move(source, target):
    """(Not-yet: Add md5sum check) Improved mv <source> <target>
    """
    shutil.copy(source, target)
    os.remove(source)

########################################################################
def get_stage_index(str_stage):
    for index in range(len(STAGES)):
        if STAGES[index] == str_stage:
            return index
        pass
    raise Exception("No such stage: [%s]" % str_stage)

########################################################################
# The various stages
def setup_environment(parameters):
    """Stage N/A (always performed): Load the environment variables necessary for correct execution. Check if the files 
    necessary for execution are properly installed

    from http://stackoverflow.com/questions/20669558/how-to-make-subprocess-called-with-call-popen-inherit-environment-variables
    """
    Logger.info("Loading environment variables")
    process = subprocess.Popen("source /home/mopspipe/mops_env psmops_ps1_mdrm152; env", stdout=subprocess.PIPE, shell=True)
    process.wait()
    output = process.communicate()[0]
    env = dict((line.split("=", 1) for line in output.split('\n') if line))
    os.environ.update(env)
    parameters.env = env
    return

def download_mpcorb(parameters):
    """Stage 0: Download MPCORB.DAT from some website 
    """
    stage_id = 0
    parameters.mpcorb_filename = parameters.output_directory + "/MPCORB.DAT"
    if stage_id < parameters.stage_index:
        Logger.info("Skipping stage [%s]" % (STAGES[stage_id]))
        return
    Logger.info("Downloading [%s]" % (parameters.url))
    import urllib
    urllib.urlretrieve(parameters.url, parameters.mpcorb_filename)
    return

def convert_mpcorb_to_des(parameters):
    """Stage 1: Convert MPCORB.DAT to DES format
    """
    stage_id = 1
    if parameters.mpcorb_des_filename is None:
        parameters.mpcorb_des_filename = parameters.output_directory + "/MPCORB.DES"
    else:
        Logger.warn("Output DES file name is set to [%s]" % parameters.mpcorb_des_filename)
        Logger.info("Skipping stage %s" % STAGES[stage_id])
        return
    if stage_id < parameters.stage_index:
        Logger.info("Skipping stage [%s]" % (STAGES[stage_id]))
        return
    Logger.info("Converting [%s] to DES format (output filename: [%s])" % (parameters.mpcorb_filename, parameters.mpcorb_des_filename))
    des_file = open(parameters.mpcorb_des_filename, "w")
    des_file_errors = open("%s.err" % parameters.mpcorb_des_filename, "w")
    command = [ "perl",
                "/home/mops/MOPS_STABLE/bin/mpcorb2des",
                "--nodup",
                parameters.mpcorb_filename ]
    Logger.info("Running [%s]" % " ".join(command))
    process = subprocess.Popen(command, stdout=des_file, stderr = des_file_errors)
    while process.poll() is None:
        des_file.flush()
        des_file_errors.flush()
        time.sleep(0.5)
        pass
    # Check process.returncode?
    des_file.close()
    des_file_errors.close()
    return
    
def propagation_to_mjd(parameters):
    """Stage 2: Propagation to MJD phase 1
    """
    stage_id = 2
    parameters.des_filename = "%s.des" % (parameters.output_basename)
    parameters.tracks_filename = "%s.tracks" % (parameters.output_basename)    
    if stage_id < parameters.stage_index:
        Logger.info("Skipping stage %s" % STAGES[stage_id])
        return
    Logger.info("In stage [%s]" % (STAGES[stage_id]))
    # Compute the number of jobs
    command = ["/usr/local/bin/condor_status", "-total"]
    process = subprocess.Popen(command, stdout=subprocess.PIPE)
    process.wait()
    for line in process.stdout:
        if 'Total' in line and 'Claimed' not in line:
            parameters.numjobs = int(line.split()[1])
            pass
        pass
    Logger.info("Number of jobs: %d" % parameters.numjobs)
    # Split the DES file into parameters.numjobs DES files
    des_file = open(parameters.mpcorb_des_filename)
    all_lines = des_file.readlines()
    header = all_lines[0]
    lines = all_lines[1:-1]
    des_file.close()
    chunk_size = (len(lines)-1)/parameters.numjobs
    tempdir = tempfile.mkdtemp(prefix = "tmpcondor", dir = parameters.output_directory)
    for index in range(parameters.numjobs):
        partial_des = open("%s/%d.MPCORB.DES" % (tempdir, index), "w")
        partial_des.write(header)
        # I don't know if there is a good reason to take every other <parameters.numjobs> lines
        partial_des.writelines(lines[index:len(lines):parameters.numjobs]) 
        partial_des.close()
        pass
    # Generate the condor script
    condoorb_remote = open("%s/condoorb_remote.sh" % (tempdir), "w")
    condoorb_remote.write("""#!/bin/bash

set -e
set -x

N=$1
MJD="%s"
oorb --conf=oorb.nbody.conf --task=propagation --epoch-mjd-utc=$MJD --orb-in=$N.MPCORB.DES --orb-out=$N.n.des
oorb --conf=oorb.2body.conf --task=ephemeris --timespan=1 --step=0.5 --obscode=F51 --orb-in=$N.n.des

""" % (parameters.mjd))
    condoorb_remote.close()
    # Generate condor job file
    condoorb_job = open("%s/condoorb.job" % (tempdir), "w")
    condoorb_job.write("""universe = vanilla
executable = condoorb_remote.sh
arguments = $(Process)
environment = "OORB_DATA=/home/mops/MOPS_STABLE/data/oorb PATH=/home/mops/MOPS_STABLE/bin:/usr/local/bin:/usr/bin:/bin"
should_transfer_files = YES
when_to_transfer_output = ON_EXIT
transfer_input_files = $(Process).MPCORB.DES,oorb.nbody.conf,oorb.2body.conf
initialdir = .
log = condoorb.condorlog
error = condoorb.stderr.$(Process)
output = condoorb.stdout.$(Process)
input = /dev/null
notification = Error
queue %d
""" % (parameters.numjobs))
    condoorb_job.close()
    # Copy the oorb.nbody.conf to the temporary directory
    shutil.copyfile("%s/config/oorb/oorb.nbody.conf" % (os.environ["MOPS_HOME"]), 
                    "%s/oorb.nbody.conf" % (tempdir))
    shutil.copyfile("%s/config/oorb/oorb.2body.conf" % (os.environ["MOPS_HOME"]), 
                    "%s/oorb.2body.conf" % (tempdir))
    # Create an empty condorlog file
    condoorb_logname = "%s/condoorb.condorlog" % (tempdir)
    condoorb_log = open(condoorb_logname, "w")
    condoorb_log.close()
    # Submit the jobs
    command = ["/usr/local/bin/condor_submit", "condoorb.job" ]
    process = subprocess.Popen(command, cwd=tempdir, stdout = subprocess.PIPE)
    process.wait()
    for line in process.stdout:
        if "submitted to cluster" in line:
            htcondor_cluster_id = line.split()[-1][0:-1] #There is a trailing "."
            pass
        pass
    Logger.info("Jobs submitted as HTCondor cluster id %s" % htcondor_cluster_id)
    # Wait for their completion
    command = ["/home/mops/MOPS_STABLE/bin/condor_wait", condoorb_logname, htcondor_cluster_id ]
    process = subprocess.Popen(command, cwd=tempdir, stdout = subprocess.PIPE, stderr = subprocess.PIPE)
    process.wait()
    process.stdout.flush()
    process.stderr.flush()
    Logger.info("HTCondor jobs completed")
    # Build the final des output
    orbits = [ ]
    for index in range(parameters.numjobs):
        orb_file = open("%s/%d.n.des" % (tempdir, index))
        lines = orb_file.readlines()
        orb_file.close()
        header = lines[0]
        orbits.extend(lines[1:-1])
        pass
    des_file = open(parameters.des_filename, "w")
    des_file.write(header)
    des_file.writelines(sorted(orbits))
    des_file.close()
    Logger.info("Built [%s]" % parameters.des_filename)
    # Build the final tracks output
    tracks_file = open(parameters.tracks_filename, "w")
    for index in range(parameters.numjobs):
        tmpout = open("%s/condoorb.stdout.%d" % (tempdir, index))
        lines = tmpout.readlines()
        # header = lines[0]
        # Check that header.split()[0, 2, 4, 5] are "Designation", "MJD_UTC/UT1", "RA", and "Dec"
        for line in lines[1:-1]:
            fields = line.split()
            tracks_file.write("%s %s %s %s\n" % (fields[0], fields[2], fields[4], fields[5]))
            pass
        tmpout.close()
        pass
    tracks_file.close()
    # Now delete the temporary directory
    command = ["/bin/rm", "-rf", tempdir]
    process = subprocess.Popen(command)
    process.wait()

def publish(parameters):
    stage_id = 3
    if stage_id < parameters.stage_index:
        Logger.info("Skipping stage %s" % STAGES[stage_id])
        return
    move(parameters.mpcorb_filename, parameters.publish_target)
    move(parameters.des_filename, "%s/condoorb" % parameters.publish_target)
    move(parameters.tracks_filename, "%s/condoorb" % parameters.publish_target)

########################################################################
class Parameters:
    def __init__(self):
        self.mjd = None
        self.obscode = "F51"
        self.url = "http://www.minorplanetcenter.net/iau/MPCORB/MPCORB.DAT"
        self.mpcorb_des_filename = None
        self.output_directory = "/tmp/condoorb"
        self.des_filename = None
        self.output_basename = None
        self.stage_index = 0
        self.publish_target = "/data/mops03.0/MOPS_DEVEL/htdocs/mpc"
        pass

    @staticmethod
    def setup(arguments):
        index = 1
        parameters = Parameters()
        while index<len(arguments):
            if arguments[index] == "-m":
                parameters.mjd = arguments[index+1]
                index += 2
                pass
            elif arguments[index] == "-c":
                parameters.obscode = arguments[index+1]
                index += 2
                pass
            elif arguments[index] == "-u":
                parameters.url = arguments[index+1]
                index += 2
                pass
            elif arguments[index] == "-o":
                parameters.output_directory = arguments[index+1]
                index += 2
                pass
            elif arguments[index] == "-s":
                parameters.stage_index = get_stage_index(arguments[index+1])
                index += 2
                pass
            elif arguments[index] == "-h":
                usage(arguments[0])
                sys.exit(0)
                pass
            elif arguments[index].startswith("-"):
                sys.stderr.write("Unknown option [%s]" % arguments[index])
                usage(arguments[1])
                sys.exit(0)
                pass
            else:
                self.des_filename = arguments[index+1]
                index += 1
            pass
        if parameters.mjd is None:
            # Get MJD. U is Unix time. MJD is (U / 86400) + 40587.
            seconds_since_epoch = calendar.timegm(time.gmtime())
            parameters.mjd = (seconds_since_epoch/86400) + 40587
        parameters.output_basename = parameters.output_directory + "/" + str(parameters.mjd)
        try:
            os.makedirs(parameters.output_directory)
        except OSError as exc: # Python >2.5
            if exc.errno == errno.EEXIST and os.path.isdir(parameters.output_directory):
                pass
            else: 
                raise exc
        return parameters
    pass # End of Parameters class definition
########################################################################

if __name__ == "__main__":
    parameters = Parameters.setup(sys.argv)
    Logger = setupLogging(parameters)
    Logger.info("Running [%s]" % " ".join(sys.argv))
    Logger.info("Starting stage: %s (%d)" % (STAGES[parameters.stage_index], parameters.stage_index))
    setup_environment(parameters)
    Logger.info("MOPS_HOME environment is [%s]" % (os.environ["MOPS_HOME"]))
    download_mpcorb(parameters)
    Logger.info("MPCORB.DAT is at [%s]" % parameters.mpcorb_filename)
    convert_mpcorb_to_des(parameters)
    Logger.info("MPCORB.DES is at [%s]" % parameters.mpcorb_des_filename)
    propagation_to_mjd(parameters)
    Logger.info("DES and TRACKS file have been generated")
    publish(parameters)
    Logger.info("DES and TRACKS file have been published")
    Logger.info("End of [%s]" % " ".join(sys.argv))
    
