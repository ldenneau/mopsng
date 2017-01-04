#!/usr/bin/python
# $Id$
'''
Created on May 15, 2012

@author: Denver Green
'''

USAGE = '''USAGE:

propagate_ssm [options]

Propagates the Solar System Model of the given simulation to the given date or
to the current date if a date is not specified.
'''

#------------------------------------------------------------------------------
# Imports.
#------------------------------------------------------------------------------
import sys
import os
import os.path
import logging
import sched
import time
import optparse
import fnmatch
import MOPS.Lib as Lib
import MOPS.Utilities as Utilities
import MOPS.Condor as Condor
from MOPS.SSM import SSM 
from MOPS.Instance import Instance
try:
    from ConfigParser import SafeConfigParser
except ImportError:
    from configparser import ConfigParser as SafeConfigParser

#------------------------------------------------------------------------------
# Global variables.
#------------------------------------------------------------------------------
gLogger = None

#------------------------------------------------------------------------------
# Function Definitions.
#------------------------------------------------------------------------------ 
def update_db_ssm(dbh, ssm_file):
    """
    Updates the SSM table in the given database using the SSM file provided.
    
    @param dbh:         Handle to the database containing the SSM to be updated.
    
    @param ssm_file:    MOPS MITI orbit format SSM file 

    """
    
    global gLogger 
    
    # Open SSM file and for each object in it update the corresponding row in 
    # the SSM table with the data read from the file.
    f = open(ssm_file, "r")
    while True:
        line = f.readline()
        if len(line) == 0: # Zero length indicates EOF
            break
        # <-- end if
        
        # Parse the line for its orbital elements.
        cols = line.split(" ")
        q = cols[0]
        e = cols[1]
        i = cols[2]
        node = cols[3]
        argPeri = cols[4]
        timePeri = cols[5]
        h_v = cols[6]
        epoch = cols[7]
        object_name = cols[8].strip()   #Remove trailing newline.
        
        # Create a SSM object and use it to update the corresponding row in the DB
        orb = SSM(q, e, i, node, argPeri, timePeri, h_v, epoch, object_name)
        try :
            orb.update(dbh)
        except Exception, e:
            gLogger.error("PROPAGATE_SSM: %s" %(str(e)))
    # <-- end while
    f.close()
# <-- end def 
    
def createCondorJob(inst, log_dir, out_dir, initial_dir, cmd, num_workers, epoch):
    """
    Setup the job manifesto for the Condor job.
    """
    job_file_root = 'ssm'
        
    # Create the job instance.
    job = Condor.Job(inst, universe='vanilla', executable=cmd, queue=num_workers)

    # Specify input/output and args.
    job.add_opt('epoch_mjd', epoch)
    job.add_file_arg('%s.$(Process)' % (job_file_root))

    # Set where STDOUT/STDERR output is written to on submitting machine.
    job.set_stderr_file(os.path.join(log_dir, job_file_root + '.stderr.$(Process)'))
    job.set_stdout_file(os.path.join(out_dir, job_file_root + '.$(Process)'))
    
    # Set where the condor log file is written on submitting machine.
    job.set_log_file(os.path.join(log_dir, job_file_root + '.condorlog'))

    # Extra options.
    job.transfer_files = True
    job.initial_dir = initial_dir
    return job
# <-- end def
            
#------------------------------------------------------------------------------
# Entry point.
#------------------------------------------------------------------------------
def main(args=sys.argv[1:]):
    """
    
    """
    AGE = 1            # Default max age for SSM
    prop_date = None 
    
    parser = optparse.OptionParser(USAGE)
    parser.add_option('--age',
                      action='store',
                      dest='age',
                      type='int',
                      default=AGE,
                      help="""The max age the SSM can be in whole days. Once the 
                        difference between epoch of the SSM and the current 
                        date exceeds the age then the SSM will be propagated 
                        to the current date.\n""")
    parser.add_option('--ignore_db',
                      action='store_true',
                      dest='ignore_db',
                      default=False,
                      help="If specified then the SSM stored in the simulation database will not be propagated\n")
    parser.add_option('--instance',
                      action='store',
                      dest='instance',
                      type='string',
                      default=os.environ.get('MOPS_DBINSTANCE', None),
                      help="Name of the simulation whose SSM is to be propagated.\n")
    parser.add_option('--epoch',
                      action='store',
                      dest='epoch',
                      type='int',
                      default=None,
                      help="The date specified as a modified julian date to propagate the SSM to.\n")
    # Verbose flag
    parser.add_option('-v',
                      action='store_true',
                      dest='verbose',
                      default=False,
                      help="Prints extra information to log file.\n")                    
    
    # Get the command line options and also whatever is passed on STDIN.
    (options, args) = parser.parse_args()

    # Make sure that we have what we need. 
    if(not options.instance): 
       parser.error('No --instance specified and no $MOPS_DBINSTANCE defined.') 
    # <-- end if 

    # Determine the name of MOPS DB instance.
    inst = Instance(dbname=options.instance)  

    # Get logger and set logging level. Typically set level to info.
    # Add null handler to logger to avoid the No handlers could be found
    # for logger XXX error if a handler is not found in a higher level logger
    global gLogger  
    logfile = os.path.join(inst.environment['LOGDIR'], 'mops.log')
    gLogger = Utilities.getLogger('MOPS', logfile)
    if (options.verbose):
        gLogger.setLevel(logging.DEBUG)
    else:
        gLogger.setLevel(logging.INFO)
    # <-- end if
     
    gLogger.info("PROPAGATE_SSM: Starting propagate_ssm")
    
    # Determine location of SSM on disc
    ssm_dir = inst.getEnvironment('OBJECTSDIR')
    if not os.path.exists(ssm_dir):
        gLogger.error("PROPAGATE_SSM: The SSM directory %s does not exist." % (ssm_dir))
        raise RuntimeError("PROPAGATE_SSM: The SSM directory %s does not exist." % (ssm_dir))
    # <-- end if
    
    # Set-up log directory
    log_dir = os.path.join(ssm_dir, 'log')
    if (os.path.exists(log_dir)) :
        # Clean out log directory
        for f in os.listdir(log_dir):
            os.remove(os.path.join(log_dir,f))
        # <-- end for
    else:
        # Create log directory if necessary.
        os.mkdir(log_dir)
    # <-- end if
    
    # Set up directory that will contain the propogated SSM
    current_dir = os.path.join(ssm_dir, 'current')
    ssm_file = os.path.join(current_dir, 'ssm.0')
    if os.path.exists(current_dir):
        # Get the MJD epoch of the SSM.
        if not os.path.exists(ssm_file):
            gLogger.warn("PROPAGATE_SSM: The SSM file %s does not exist." % (ssm_file))
            ssm_date = 0
        else:     
            # Determine the epoch of the SSM            
            f = open(ssm_file, "r")
            line = f.readline()
            if len(line) == 0:
                gLogger.warn("PROPAGATE_SSM: The SSM file %s is empty." % (ssm_file))
                ssm_date = 0
            else:
                # Epoch is the 7th element of the line (counting starts at 0)
                ssm_date = line.split(" ")[7]
            # <-- end if
            f.close()
        # <-- end if       
    else:
        # Directory to contain propagate SSM does not exist, create it.
        os.mkdir(current_dir)
        # Ensure that SSM age is greater than max age so that SSM will be propagated.
        ssm_date = 0 
    # <-- end if

    # Determine the number of worker threads to create to propagate the SSM.
    # Create 1 worker for each ssm.# file in the ssm directory.
    num_workers = 0
    for f in os.listdir(ssm_dir):
        if fnmatch.fnmatch(f, 'ssm.*'):
            num_workers += 1
        # <-- end if
    # <-- end for

    if (options.epoch):
        # Set prop_date to the epoch given on the command line.
        prop_date = options.epoch
    else :
        # Set prop_date to the current UT date and convert it to a modified
        # julian date (mjd).
        prop_date = time.gmtime()
        prop_date = Lib.gd2jd(prop_date.tm_year, prop_date.tm_mon, prop_date.tm_mday)
        prop_date = Lib.jd2mjd(prop_date)
    # <-- end if
        
    # Determine age of the SSM stored in the current directory with respect to 
    # the propagation date.
    ssm_age = abs(float(prop_date) - float(ssm_date))

    # Compare age of the SSM in current with the max age. If SSM age is greater
    # then propagate the SSM in the current directory to the propagation date.
    try:
        if (ssm_age > options.age):
            # Clean out the SSM in the current directory
            for f in os.listdir(current_dir):
                os.remove(os.path.join(current_dir,f))
            # <-- end for
            
            # Create a condor job object which will be used to propagate the ssm on
            # the condor cluster
            cmd = os.path.join(inst.getEnvironment('HOMEDIR'), 'bin', 'propagateOrbits')
            job = createCondorJob(inst, log_dir, current_dir, ssm_dir,cmd, num_workers, prop_date)

            gLogger.info("PROPAGATE_SSM: Propagating SSM of %s to %s" % (options.instance, prop_date))
            gLogger.info("PROPAGATE_SSM: Using the SSM stored in %s" % (ssm_dir))
            gLogger.info("PROPAGATE_SSM: Storing propagated SSM in %s" % (current_dir))
        
            # Submit the job to the cluster and wait until it is done.
            err = Condor.submit(job, removeJobFile=True)
        
            # Create sentinel file in current_dir to indicate that it contains a 
            # propogated SSM
            f = open(os.path.join(current_dir, "sentinel"), "w")
            f.close()
        
            if options.ignore_db:
                # Ignore database option was specifed don't propagate the SSM in the DB
                gLogger.info("PROPAGATE_SSM: Ignore database option was given. SSM in db will not be propagated.")
                pass
            else:
                # Propagate the SSM in the DB
                gLogger.info("PROPAGATE_SSM: Propagating the SSM stored in the database.")
                for f in os.listdir(current_dir):
                    if not fnmatch.fnmatch(f, 'ssm.*'):
                        continue
                    # <-- end if
                    update_db_ssm(inst.get_dbh(), os.path.join(current_dir, f))
                # <-- end for
            # <-- end if
        else:
            gLogger.info("PROPAGATE_SSM: Age of SSM is %s which is not greater than the max age of %s for the SSM." % (ssm_age, options.age))
            gLogger.info("PROPAGATE_SSM: Propagation of the SSM is not necessary.")
        # <-- end if
    except Exception, e:
        gLogger.exception("PROPAGATE_SSM: %s" % (str(e)))
        sys.stderr.write(str(e))
        raise SystemExit(1)
    # <-- end try
# <-- end def

if __name__ == '__main__':
    sys.exit(main())
# <-- end if