#!/usr/bin/env python
'''
Created on July 3, 2011

@author: Denver Green
'''
USAGE = """

alertcheck.py [OPTIONS]

Options:
    --include_synthetic     include synthetic objects (default false)
    --arc                   minumum required arc length for derivied object alerts
    -v                      turns on debug logging

Description:
alertcheck dynamically discovers Alert Rules by scanning the MOPS.Alerts plugins
directory. It then loops over all available Rules and collects the derived
objects that satisfy at least one Rule.

Each of these objects will then be used to generate an alert, which is then 
inserted into the MOPS database ready to be processed by alertsubmit.

This is the second program to be executed in the alert workflow.
  ______________        _____________        ______________
 |alertupdate.py| ---> |alertcheck.py| ---> |alertsubmit.py|
  --------------        -------------        --------------  
"""
import sys
import copy
import datetime
import logging
import os
import traceback
from functools import partial

import MOPS
import MOPS.Instance as Instance
import MOPS.Alerts.Constants as Constants
import MOPS.Utilities as Utilities
from MOPS.Alerts.plugins import rules as AlertRules
from MOPS.Alerts.AQueue import AQueue
from MOPS.Alerts.Alert import Alert

import time
import pdb

#------------------------------------------------------------------------------
# Function Definitions.
#------------------------------------------------------------------------------ 
def alertCheck(includeSyntheticObjects=False, minArcLength=None,
               logger=None):
    """
    Query the MOPS database at instance to see if any of the AlertRules is
    satisfied. A rule is satisfied for a DerivedObject instance.

    Collect all DerivedObject instances that satisfy AlertRules, generate the
    corresponding VOEvents and send the events off to the server/broker.

    If the alert queue database table is locked, then raise an Exception.

    @param instance: MOPS.Instance object
    @param includeSyntheticObjects: boolean
    @param minArcLength: minimum arc length in days for a DerivedObject to be 
           an alert candidate.
    """
    alertCache = {}                  # {obj.alertId: AlertObj}
    chCache = {}                     # {obj.alertId: [(channel, ruleName),]}

    # Check and see if there are any AQueue elements with status="R". That would
    # mean one of two things:
    # 1. Another alertcheck.py process is running (which would be BAD).
    # 2. The previous time we ran, we crashed.
    # 1. can never happen since we are using a locking strategy and we would not
    # be here if another instance was running. Then we do not need to worry
    # about them: they will be picked up by our rules anyway.

    # Start off by marking all unprocessed items in AQueue as available.
    logger.info('alertcheck: Preparing alert queue for processing.')
    AQueue.prepareForProcessing(None)

    # Instantiate the various rules and skip those who crash.
    logger.info('alertcheck: Fetching rules.')
    rules = []

    for Rule in AlertRules:
        try:
            logger.info('alertcheck: Instantiating %s rule.' % (Rule.__name__))
            rules.append(Rule(includeSyntheticObjects,
                              minArcLength=minArcLength))
        except:
            # Ops! The rule crashed. Notify the user and keep going.
            # This is the place where we might want to remove the rule.
            logger.exception('alertcheck: Rule %s instantiation crashed. Skipped.' 
                             %(Rule.__name__))
        # <-- end try
    # <-- end for
    
    # Check each rule.
    for rule in rules:
        # Dictionary that contains alert objects that match the rule.
        # {obj.alertId: AlertObj}
        ruleAlertCache = {} 
        
        # Dictionary that contains a tuple made up of a channel and rule name.
        # {obj.alertId:(Channel Name, Rule Name)}
        ruleChCache = {}
        
        # Put everything in a try...except so that if a rule crashes, we do not.
        try:
            logger.info('alertcheck: Evaluating %s rule.' % 
                        (rule.__class__.__name__))
            results = rule.evaluate()
            logger.info('alertcheck: %d objects found by the %s rule.' % 
                        (len(results), rule.__class__.__name__))
            for a in results:
                # Update the alert object hash with alert objects selected by 
                # the rule. The hash is keyed on the alert id and contains the
                # alert object associated with alert id.
                if(not ruleAlertCache.has_key(a.alertId)):
                    ruleAlertCache[a.alertId] = a
                # <-- end if

                # Update the channel cache with the name of the rule and the
                # channel it publishes alerts to. The channel cache is keyed
                # on Alert ID. I.E an alert object may be selected by multiple
                # rules.
                if(not ruleChCache.has_key(a.alertId)):
                    # First time alert has matched a rule add it to cache.
                    ruleChCache[a.alertId] = [(rule.channel,
                                             rule.__class__.__name__) ]
                else:
                    # Alert has already matched a previous rule. Append channel
                    # and name to cache. 
                    ruleChCache[a.alertId].append((rule.channel,
                                                 rule.__class__.__name__))
                # <-- end if
            # <-- end for
        except:
            # Ops! The rule crashed. Notify the user and keep going.
            # This is the place where we might want to remove the rule.
            logger.exception('alertcheck: Rule %s evaluation crashed. Skipped.' 
                             %(rule.__class__.__name__))
        else:         
            # Merge the rule specific dictionaries with the system-wide ones.
            # We need to do this because we do not want a rule that crashes to
            # 'pollute' system-wide caches.
            alertCache.update(ruleAlertCache)
            for objId in ruleChCache.keys():
                if(not chCache.has_key(objId)):
                    chCache[objId] = ruleChCache[objId]
                else:
                    chCache[objId] += ruleChCache[objId]
                # <-- end if
            # <-- end for
        # <-- end try
    # <-- end for
    
    # Now alertCache has a list of unique Alert instances that satisfy at
    # least one rule. Insert the contents of the list into the alerts table 
    logger.info('alertcheck: Adding alerts to alert table.')
    for a in alertCache.values():
        # Create an Alert using the aQueue alert object
        for i in range(0,len(chCache[a.alertId])):
            (channel, rule) = chCache[a.alertId][i]
            newAlert = Alert(a.alertId, rule, channel, Constants.AQUEUE_STATUS_NEW,
                             a.objId, a.objType, a.dbname, a.classification,
                             a.message, a.subject)
            newAlert.save()
        # <-- end for
    # <-- end for    

    # Give alert rules the chance to cleanup.
    for rule in rules:
        try:
            rule.cleanup()
        except:
            # Ops! The rule crashed. Notify the user and keep going.
            # This is the place where we might want to remove the rule.
            logger.exception('alertcheck: Rule %s cleanup crashed. Skipped.' 
                             %(Rule.__name__))
        # <-- end try
    # <-- end for

    # Release the lock on the table.
    AQueue.doneWithProcessing()
    return
# <-- end AlertCheck


def main(args=sys.argv[1:]):
    import optparse
     
    # Constants
    
    # Get user input (tracks file) and make sure that it exists.
    parser = optparse.OptionParser(USAGE)
    parser.add_option('--include_synthetic',
                      action='store_true',
                      dest='includeSynthetic',
                      default=False)
    parser.add_option('--arc',
                      action='store',
                      dest='minArcLength',
                      default=0)
    # Verbose flag
    parser.add_option('-v',
                      action='store_true',
                      dest='verbose',
                      default=False)
    
    # Get the command line options and also whatever is passed on STDIN.
    (options, args) = parser.parse_args()

    # Set logging level
    if (options.verbose):
        logger.setLevel(logging.DEBUG)
    else:
        logger.setLevel(logging.INFO)
 
    # Everything is fine, set the ball rolling!
    sys.exit(alertCheck(includeSyntheticObjects=options.includeSynthetic,
                        minArcLength=options.minArcLength,
                        logger=logger))
# <-- end main    

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
# Entry point.
#------------------------------------------------------------------------------
if(__name__ == '__main__'):
    sys.exit(main())
# <-- end if