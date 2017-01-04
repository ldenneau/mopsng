#!/usr/bin/python
'''
Created on July 3, 2011

@author: Denver Green
'''
USAGE = """

alertupdate [options]

Options:
    --instance INST   use MOPS sim instance INST (default $MOPS_DBINSTANCE)
    --nn              night number to search
    --help            prints this help
    -v                turns on debug logging

Description:
Searches the provided database instance for Tracklets and Derived objects that
were created or updated on the night number indicated. This is the first 
program to be executed in the alert workflow.

  ______________        __________        ___________
 |alertupdate.py| ---> |alertcheck| ---> |alertsubmit|
  --------------        ----------        -----------  
"""
#------------------------------------------------------------------------------
# Imports.
#------------------------------------------------------------------------------
import logging
import MOPS.Alerts.Constants as Constants
import MOPS.Utilities as Utilities
import optparse
import os
import pprint
import sys

from MOPS.Instance import Instance

#------------------------------------------------------------------------------
# Globals.
#------------------------------------------------------------------------------

# Store useful environment-type stuff.
homedir = os.environ.get('MOPS_HOME', '/usr/local/MOPS')
vardir = os.path.join(homedir, 'var', 'alerts')

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
logfile = os.path.join(environment['LOGDIR'], Constants.ALERT_LOG_FILE)
logger = Utilities.getLogger(None, logfile)

#------------------------------------------------------------------------------
# Function Definitions.
#------------------------------------------------------------------------------ 
def findDerivedObjs(dbh, options):
    global logger
    
    # Do a join of the derivedobject_attrib, tracklets, and fields table, using 
    # the tracklet_id, and field_id columns, and select the records from the 
    # named database instance which matches the given night number.
    cursor = dbh.cursor()
    sql = """
        select derivedobject_id, classification from derivedobject_attrib d,
        tracklets t, fields f
        where d.tracklet_id = t.tracklet_id 
        and f.field_id = t.field_id 
        and f.nn = %s
    """
    n = cursor.execute(sql, (options.nightNum))
    logger.info("ALERTUPDATE: Found %s new derived objects for night number %s" % 
                (n, options.nightNum))    
    derivedObjs = cursor.fetchall()
    
    # Set the default(current) database on the connection to 
    # that containing the AQueue table.               
    dbh.select_db(Constants.AQUEUE_DB_NAME)
    sql = """
        insert into aqueue (
        status, object_id, object_type, 
        nn, db_name, classification) 
        values (%s, %s, %s, %s, %s, %s)
    """

    for row in derivedObjs:
        # First see if this derived object has already been inserted. Update the
        # entry if new tracklets have been added to the derived object through
        # attribution. If the derived object has not been altered then do 
        # nothing
        existing = cursor.execute("""select alert_id, nn from aqueue 
            where object_id=%s and object_type=%s and db_name=%s""" ,
            (row[0], Constants.AQUEUE_TYPE_DERIVED, options.instance))
        if (existing):
            (alertId, nn) = cursor.fetchone();
            if (long(options.nightNum) > nn):
                cursor.execute("""update aqueue set nn = %s, status = %s, 
                    insert_timestamp = Now() where alert_id = %s""", 
                    (options.nightNum, Constants.AQUEUE_STATUS_NEW, alertId ))
                logger.info("ALERTUPDATE: Updated derived object alertId: %s changed nn from %s to %s" %
                             (alertId, nn, options.nightNum))
            # <-- end if
            continue
        # <-- end if
        n = cursor.execute(sql, (Constants.AQUEUE_STATUS_NEW, row[0], 
                                 Constants.AQUEUE_TYPE_DERIVED, 
                                 options.nightNum, options.instance, row[1]))
        if not n:
            raise AlertException('Insert into AQueue table failed.')
        # <-- end if
        # logger.debug("Inserted objID: %s Class: %s ObjType: %s" % 
        #             (row[0], row[1], Constants.AQUEUE_TYPE_DERIVED))
    # <-- end for
    dbh.commit()
        
    # Set the default(current) database on the connection back to the  
    # that instance containing the tracklets table
    dbh.select_db(options.instance)
    cursor.close()
# <-- end findDerivedObjs

def findTracklets(dbh, options):
    global logger
    
    # Do a join of the tracklets and fields table using the field_id column, 
    # and select the records from the named database instance which matches
    # the given night number.
    cursor = dbh.cursor()
    sql = """
        select tracklet_id, classification from tracklets t, fields f
        where f.field_id = t.field_id 
        and f.nn = %s
    """
    n = cursor.execute(sql, (options.nightNum))
    logger.info("ALERTUPDATE: Found %s new tracklets for night number %s" % 
                (n, options.nightNum))
    tracklets = cursor.fetchall()

    # Set the default(current) database on the connection to 
    # that containing the AQueue table.               
    dbh.select_db(Constants.AQUEUE_DB_NAME)
    sql = """
        insert into aqueue (
        status, object_id, object_type, 
        nn, db_name, classification) 
        values (%s, %s, %s, %s, %s, %s)
    """

    for row in tracklets:
        # First see if this object is already there. It it is then don't
        # insert it.
        existing = cursor.execute("""select alert_id, nn from aqueue 
            where object_id=%s and object_type=%s and db_name=%s""" ,
            (row[0], Constants.AQUEUE_TYPE_TRACKLET, options.instance))
        if (existing):
            (alertId, nn) = cursor.fetchone();
            if (long(options.nightNum) > nn):
                cursor.execute("""update aqueue set nn = %s, status = %s, 
                    insert_timestamp = Now() where alert_id = %s""", 
                    (options.nightNum, Constants.AQUEUE_STATUS_NEW, alertId ))
                logger.info("ALERTUPDATE: Updated tracklet alertId: %s changed nn from %s to %s" %
                             (alertId, nn, options.nightNum))
            # <-- end if
            continue
        # <-- end if
        n = cursor.execute(sql, (Constants.AQUEUE_STATUS_NEW, row[0], 
                                 Constants.AQUEUE_TYPE_TRACKLET, 
                                 options.nightNum, options.instance, row[1]))
        if not n:
            raise AlertException('Insert into AQueue table failed.')
        # <-- end if
        logger.debug("ALERTUPDATE: Inserted objID: %s Class: %s ObjType: %s" % 
                    (row[0], row[1], Constants.AQUEUE_TYPE_TRACKLET))
    # <-- end for
    dbh.commit()
        
    # Set the default(current) database on the connection back to the  
    # that instance containing the tracklets table
    dbh.select_db(options.instance)
    cursor.close()
# <-- end findTracklets

def main(args=sys.argv[1:]):
    global logger
    
    """
    This program will find all tracklets that was created or updated on the 
    night given in the database instance specified.
    """ 
    parser = optparse.OptionParser(USAGE)
    parser.add_option('--instance',
                      action='store',
                      dest='instance',
                      default=os.environ.get('MOPS_DBINSTANCE', None))
    parser.add_option('--nn',
                      action='store',
                      dest='nightNum')
    # Verbose flag
    parser.add_option('-v',
                      action='store_true',
                      dest='verbose',
                      default=False)    
    
    # Get the command line options and also whatever is passed on STDIN.
    (options, args) = parser.parse_args()

    # Make sure that we have what we need. 
    if(not options.instance): 
       parser.error('No --instance specified and no $MOPS_DBINSTANCE defined.') 
    # <-- end if 

    if (not options.nightNum):
        parser.error('The night number (--nn) must be specified.')
    # <-- end if
    
    # Set logging level
    if (options.verbose):
        logger.setLevel(logging.DEBUG)
    else:
        logger.setLevel(logging.INFO)
    
    # Check to see if alerts are enabled for the database if it is not then
    # quit.
    inst= Instance(options.instance)
    config = inst.getConfig()
    if (not(config['main']['enable_alert'])):
        logger.warn('ALERTUPDATE: Alerts are not enabled for %s' % (options.instance))
        logger.warn('ALERTUPDATE: Use editConfig to add the line "enable_alert = 1" to the main section.')
        return
    # <-- end if
    
    # Connect to the database.
    dbh = inst.get_dbh()

    try:
        findTracklets(dbh, options)
        findDerivedObjs(dbh, options)
    except Exception, e:
        logger.exception(e)
        raise SystemExit(1)
    # <-- end try
# <-- end def

#------------------------------------------------------------------------------
# Entry point.
#------------------------------------------------------------------------------
if __name__ == '__main__':
    sys.exit(main())
# <-- end if