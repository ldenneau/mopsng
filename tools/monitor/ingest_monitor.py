#!/usr/bin/python
'''
Created on Sep 6, 2011

@author: Denver Green
'''

USAGE = """

ingest_monitor [options]

Options:
    --instance INST   The MOPS simulation instance into which new exposures are
                      to be ingested. Defaults to $MOPS_DBINSTANCE if not
                      specified.
                      
    --days DAYS       The number of days into the past to monitor for new
                      exposures. This is relative to the current day. If this
                      is not specified then it will default to a pre-configured
                      value.
                      
    --dir DIRECTORY   The directory (includes subdirectories) that is to be
                      monitored. Valid values for this option are: NOMAGIC, DIFF,
                      LAP, and TEST.
                      Defaults to the NOMAGIC staging directory if not specified.
 
    --email ADDR      The address to e-mail all warnings to.
                      Defaults to ps-mops-dev@ifa.hawaii.edu.                                          
    --help            prints this help
    
    -v                turns on debug logging

Description:
Daemon process whose sole purpose is to monitor the MOPS stage directory. 
When a new symbolic link is created in the directory that falls into the time 
window being monitored, the process will ingest the exposure using the 
current ingest tool.

Upon startup INGEST_MONITOR creates a file in the tmp directory called 
pyinotify_<db_instance>.pid which contains the process id for the running 
INGEST_MONITOR. If this file already exists in the /tmp directory then 
INGEST_MONITOR will fail to start as this is an indication that INGEST_MONITOR is 
already running and only one copy of INGEST_MONITOR can be running for any 
database instance. This file will be deleted by monitor stage when it exits. 
"""
"""
Currently exposures published by IPP to the IPP datastore are copied by the
mops mirror script to the raw subdirectory of either the 
/data/mops01.0/MOPS_STAGE/nomagic/IPP-MOPS or 
/data/mops01.0/MOPS_STAGE/diff/IPP-MOPS directories. Also within these directories
are subdirectories that are named after the observing cycle number and the night 
number. A symbolic link that points to the exposure is then created in the
ocnum/nightnum subdirectory.
Monitor stage must be able to dynamically add new ocnum and nightnum directories 
to the watch list as they are created, and remove them when they become too old.

This program makes extensive use of a third party library called pyinotify. 
This library can be downloaded from http://github.com/seb-m/pyinotify and
documentation for the library is available from http://pyinotify.sourceforge.net/
and https://github.com/seb-m/pyinotify/tree/master/python2/examples
"""
#------------------------------------------------------------------------------
# Imports.
#------------------------------------------------------------------------------
import atexit
import datetime
import logging
import optparse
import os
import pyinotify
import re
import signal
import smtplib
import subprocess
import sys
import tempfile
import time

import MOPS.Lib as Lib
import MOPS.Utilities as Utilities
import MOPS.Instance as Instance
import MOPS.Constants as Constants

from email.mime.text import MIMEText
#------------------------------------------------------------------------------
# Globals.
#------------------------------------------------------------------------------

# Logging
logger = None

# Location of file that contains the process id of the INGEST_MONITOR  process
pid_file = None

# Events that will be watched for
mask = pyinotify.IN_CREATE | pyinotify.IN_UNMOUNT



#------------------------------------------------------------------------------
# Class Definitions.
#------------------------------------------------------------------------------ 
class EventHandler(pyinotify.ProcessEvent):
    # Globals used
    global logger
    global mask
    global pid_file
    
    def my_init(self, notifier,wm, wd, base, options):
        """
        Method automatically called from ProcessEvent.__init__(). Additional
        keyworded arguments passed to ProcessEvent.__init__() are then
        delegated to my_init().
        """
        self._notifier = notifier
        self._wm = wm                   # Watch manager.
        self._wd = wd                   # List of watch descriptors
        self._base = base               # Base directory to be watch.
        self._options = options
    # <-- end def
    
    def process_IN_UNMOUNT(self, event):
        # log unmount event to log file.
        logger.info("INGEST_MONITOR: The mops01:/export/mops01.0 backing fs was unmounted.")
        logger.info("               resetting watch directories.")
        
        # send email informing admin of filesystem unmount
        reply = 'noreply@ifa.hawaii.edu'
        to = self._options.email
        html = """\
            <html>
                <head></head>
                <body>
                    <p>The mops01:/export/mops01.0 backing fs was unmounted.<br>
                       Please verify that the staging directories are still being monitored.
                    </p>
                </body>
            </html>
        """
        msg = MIMEText(html, 'html')
        msg['Subject'] = 'INGEST_MONITOR: BACKING FILESYSTEM UNMOUNT WARNING'
        msg['From'] = reply
        msg['To'] = to
        s = smtplib.SMTP('hale.ifa.hawaii.edu')
        s.sendmail(reply, [to], msg.as_string())
        s.quit()
              
        # Wait for a few seconds so as to allow the filesystem to be remounted.
        time.sleep(1)
        logger.info("INGEST_MONITOR: Removing %s and restarting ingest_monitor." % pid_file)
        
        # Remove the pid file so that we can start a new instance of ingest_monitor.
        # pid_file = '/tmp/pyinotify_%s.pid' % self._options.instance
        try:
            os.unlink(pid_file)
        except:
            # ignore error
            pass
            
        # Start a new ingest_monitor process and replace the current process
        # with it. The sys.argv object contains the list of command line  
        # arguments passed to a Python script. argv[0] is the script name 
        os.execvp(os.path.basename(sys.argv[0]), sys.argv[1:])

    # <-- end def
            
    def process_IN_CREATE(self, event):
        obj = os.path.join(event.path, event.name)
        logger.info("INGEST_MONITOR: Detected the creation of %s" % obj)
        if (event.path == self._base and os.path.isdir(obj)):
            # The creation event occured in the root stage directory and it was
            # probably the creation of a new ocnum directory and we need to 
            # update the directories being watched by adding the new one and
            # removing an old one.
            try:
                int(event.name, 10)
            except ValueError,e:
                # Name of item created contained non numeric characters don't
                # update watch items.
                return 
            # <-- end try
            self.resetWatchDirs()             
        else:
            # If the name of the created object does not match the pattern used 
            # for exposures names then do nothing and return.
            pattern = r"^o[0-9]{4}g[0-9]{4}o"
            regExp = re.compile(pattern)
            if not(regExp.match(event.name)):
                logger.info("INGEST_MONITOR: Not importing as %s does not conform with the recognized exposure naming standard" % event.name) 
                return
            # <-- end if
            
            # Determine if the object created falls within the monitoring
            # window specified by the day parameter.
            # 1. Get the night number for yesterday.          
            todayUTC = datetime.datetime.utcnow()
            jd = Lib.gd2jd(todayUTC.year, todayUTC.month, todayUTC.day, 0, 0, 0)
            yesterday = int(Lib.jd2mjd(jd) - 1) # Subtract one as we want yesterday.
            
            # 2. Get the night number from the path of the created object.
            (head, sep, nn) = event.path.rpartition('/')
            try:
                nn = int(nn, 10)
            except ValueError,e:
                logger.error("INGEST_MONITOR: The path %s of the created object does not conform with the recognized naming standard" % event.path)
                return 
            # <-- end try
            
            # 3. Calculate the difference between the two and if less than
            #    or equal to the value specified in the days parameter 
            #    ingest the exposure.
            if ((yesterday - nn) > self._options.window):
                logger.info("""INGEST_MONITOR: Not importing as %s is outside 
                            of the %s day monitoring window""" % (obj, self._options.window))
                return
            # <-- end if
            logger.info("INGEST_MONITOR: Ingesting %s." % (obj))
            result = subprocess.call(["ingest", "--instance", 
                                      self._options.instance, event.name])
            if (result != 0):
                logger.error("INGEST_MONITOR: Failed to ingest %s" % (obj))
            else :
                # 4. Set the status of the ingested exposure to 
                # FIELD_STATUS_INGESTED. This will prevent mopper from processing
                # the field until the status is changed to FIELD_STATUS_READY.
                inst = Instance.Instance(self._options.instance)
                # Connect to the database.
                dbh = inst.get_dbh()
                cursor = dbh.cursor()
                sql = """update %s.fields set status = '%s' where fpa_id = '%s' and status = '%s'""" % \
                    (self._options.instance,  
                     Constants.FIELD_STATUS_INGESTED, 
                     event.name,
                     Constants.FIELD_STATUS_NEW)
                n = cursor.execute(sql)
                dbh.commit()
            # <-- end if
        # <-- end if
    # <-- end def
    
    def resetWatchDirs(self):
        logger.info("INGEST_MONITOR: Resetting watch directories") 
            
        # Remove all watches except for the one on the stage directory.
        if ( len(self._wd) > 0 ):
            self._wm.rm_watch(self._wd, rec=True)
        # <-- end if
        
        # Add new directories to be monitored to the watch manager. 
        dirs = getDirsToMonitor(self._base)
        self._wd = list()
        for d in dirs:
            wdd = self._wm.add_watch(d, mask, rec=True, 
                                     auto_add=True)
            if d in wdd: 
                logger.info("INGEST_MONITOR: Monitoring %s for new exposures." % (d))
                self._wd.append(wdd[d])
            else:
                logger.error("INGEST_MONITOR: Failed to add a watch on %s." % (d))
            # <-- end if
        # <-- end for
    # <-- end def
# <-- end class

#------------------------------------------------------------------------------
# Function Definitions.
#------------------------------------------------------------------------------ 
def cleanup(signum, stack_frame):
    '''
     Removes the pid file created when INGEST_MONITOR is started.
    '''
    # Globals used
    global pid_file
    
    logger.info("INGEST_MONITOR: Termination signal received. Deleting %s and shutting down."
                % pid_file)
                
    # Surround in try catch block so as to suppress any error messages generated 
    # by the delete of the pid file.
    try:
        os.unlink(pid_file)
    except:
        pass
    
    sys.exit(1)
# <-- end def

def getDirsToMonitor(dir):
    '''
     Lists the contents of the staging directory to be monitored and removes  
     any entries that are not a three digit number from the list. Sorts the  
     contents of the list in ascending order and then removes the last two  
     entries which corresponds with the last two lunar observing cycles. The 
     contents of these two directories will be monitored by INGEST_MONITOR.
    '''
    regExp = re.compile(r'^[0-9]{3}$') #Matches all three digit entries.
    contents = os.listdir(dir)
    for i in contents:
        if not(regExp.match(i)):
            contents.remove(i)
        # <-- end if
    # <-- end if
    contents.sort()
    results = list()
    results.append(os.path.join(dir, contents.pop()))
    results.append(os.path.join(dir, contents.pop()))
    return results
# <-- end def

def startMonitor(options, base):
    # Globals used
    global logger
    global mask
    global pid_file  
    
    #import pdb; pdb.set_trace(); import pprint
    # The watch manager stores the watches and provides operations on watches
    wm = pyinotify.WatchManager()
    
    # Add directories to be monitored to the watch manager. 
    wm.add_watch(base, mask, rec=False)
    dirs = getDirsToMonitor(base)
    watchDesc = list()
    for d in dirs:
        wdd = wm.add_watch(d, mask, rec=True, auto_add=True)
        if d in wdd: 
            logger.info("INGEST_MONITOR: Monitoring %s for new exposures." % (d))
            watchDesc.append(wdd[d])
        else:
            logger.error("INGEST_MONITOR: Failed to add a watch on %s." % (d))
        # <-- end if
    # <-- end for       

    # Internally, 'handler' is a callable object which on new events will be  
    # called like this: handler(new_event)
    notifier = None
    notifier = pyinotify.Notifier(wm, EventHandler(notifier=notifier,
                                                   wm=wm, 
                                                   wd=watchDesc, 
                                                   base=base,
                                                   options=options))   
    try:    
        # Enter processing loop
        pid_file = '/tmp/pyinotify_%s.pid' % options.instance
        notifier.loop(daemonize=True, pid_file=pid_file)
    except Exception, e:
        notifier.stop()
        logger.exception(e)
        raise SystemExit(1)
    # <-- end try 
# <-- end def

def main(args=sys.argv[1:]):
    
    # Globals used
    global logger
    global pid_file
    
        
    # The number of days into the past to monitor for new exposures.        
    default_window     = 3 
    
    # Staging directories
    basedir = "/data/mops01.0/MOPS_STAGE"
    stagedir = {
        'NOMAGIC' : os.path.join(basedir, 'nomagic/IPP-MOPS'),
        'DIFF' : os.path.join(basedir, 'diff/IPP-MOPS'),
        'LAP' : os.path.join(basedir, 'lap/IPP-MOPS'),
        'TEST' : os.path.join(basedir, 'test2/IPP-MOPS'),
    }
        
    # clean up upon exit
    #atexit.register(cleanup)
    signal.signal(signal.SIGTERM, cleanup)
    signal.signal(signal.SIGINT, cleanup)
    signal.signal(signal.SIGQUIT, cleanup)     
        
    # Parse command line for options.
    parser = optparse.OptionParser(USAGE)
    parser.add_option('--instance', 
                      action='store',
                      dest='instance', 
                      default=os.environ.get('MOPS_DBINSTANCE', None))
    parser.add_option('--dir', 
                      action='store', 
                      dest='dir', 
                      default='NOMAGIC')
    parser.add_option('--days', 
                      action='store', 
                      dest='window', 
                      type='int',
                      default=default_window)
    parser.add_option('--email',
                      action='store',
                      dest='email',
                      default='ps-mops-dev@ifa.hawaii.edu')
    # Verbose flag
    parser.add_option('-v', 
                      action='store_true', 
                      dest='verbose', 
                      default=False)    
    
    # Get the command line options and also whatever is passed on STDIN.
    (options, args) = parser.parse_args()

    # Store useful environment-type stuff.
    homedir = os.environ.get('MOPS_HOME', '/usr/local/MOPS')
    vardir = os.path.join(homedir, 'var', options.instance)

    environment = {
        'HOMEDIR' : homedir,
        'VARDIR' : vardir,
        'CONFIGDIR' : os.path.join(vardir, 'config'),
        'LOGDIR' : os.path.join(vardir, 'log'),
        'NNDIR' : os.path.join(vardir, 'nn'),
        'OBJECTSDIR' : os.path.join(vardir, 'objects'),
        'LSDDIR' : os.path.join(vardir, 'lsd'),
    }
    
    # Logging.
    logfile = os.path.join(environment['LOGDIR'], 'ingest.log')
    logger = Utilities.getLogger(None, logfile)
   
    # Set logging level
    if (options.verbose):
        logger.setLevel(logging.DEBUG)
    else:
        logger.setLevel(logging.INFO)
    # <-- end if 

    # Make sure we have a valid schema to import into. 
    if(not options.instance): 
        parser.error('No --instance specified and no $MOPS_DBINSTANCE defined.') 
    # <-- end if 
    
    # Verify that a recognized staging directory was specified.
    options.dir = options.dir.upper()
    if (not stagedir.has_key(options.dir)):
        logger.error("INGEST_MONITOR: %s is not a recognized staging directory.",
                     (options.dir)) 
        logger.error("INGEST_MONITOR: Valid values for dir are NOMAGIC, DIFF, LAP, and TEST")
        return
    # <-- end if
    
    logger.info("INGEST_MONITOR: Ingesting exposures into %s." % (options.instance))
    logger.info("INGEST_MONITOR: Monitoring window is %s days." % (options.window))
    
    # Create a temporary file in basedir so as to prevent automount from 
    # unmounting the /data/mops01.0 share.
    f = tempfile.TemporaryFile(dir=basedir)

    # Start monitoring watch directories.
    startMonitor(options, stagedir[options.dir])

# <-- end def
    
#------------------------------------------------------------------------------
# Entry point.
#------------------------------------------------------------------------------        
if __name__ == '__main__':
    sys.exit(main())
# <-- end if
