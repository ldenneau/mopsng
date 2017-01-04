#!/usr/bin/env python
'''
Created on July 3, 2011

@author: Denver Green
'''
USAGE = """

alertsubmit.py [OPTIONS]

Options:
    -h, --help              show this help message and exit   
    --channel CHANNEL       publish alerts associated with the named channel
    --rule RULE             publish alerts associated with the named rule
    --include_synthetic     include synthetic objects (default false)
    -v                      turns on debug logging
    --noTweet               alerts will not be sent via twitter

Description
alertsubmit iterates over the alerts found by alertcheck and creates the 
voEvent XML object containing information about the alert. It then writes the
voEvent to the database and generates a twitter alert which references the
voEvent.

  ______________        _____________        ______________
 |alertupdate.py| ---> |alertcheck.py| ---> |alertsubmit.py|
  --------------        -------------        --------------  
"""
import sys
import os
import logging
import smtplib
import tempfile
import twitter
import MOPS.Utilities as Utilities
import MOPS.VOEvent.VOEvent as VOEvent
import MOPS.Alerts.Constants as Constants
import MOPS.Alerts.Exceptions

from email.mime.text import MIMEText
from email.mime.multipart import MIMEMultipart
from MOPS.Alerts.Alert import DerivedObjAlert, TrackletAlert, Alert, AlertObj
from MOPS.Alerts.plugins.Constants import CH_ALL
from MOPS.Alerts.TweetAlert import TweetAlert, config_filename
from MOPS.Instance import Instance

#------------------------------------------------------------------------------
# Class Definitions.
#------------------------------------------------------------------------------
class EmailAlert(object):
    
    # Class variables.   
    inst = Instance(Constants.AQUEUE_DB_NAME)
    dbh = inst.get_dbh()

    """
    This class handles the sending of alerts by email.
    """
    def __init__(self):
        # Hash of email address keyed on rule name. Serves as a cache to avoid 
        # multiple queries to the database.
        self._addresses = {} 
        
        # Open connection to email server    
        self._email = smtplib.SMTP('hale.ifa.hawaii.edu')
    # <-- end def
    
    def __del__(self):
        # Close email connection
        self._email.quit()       
    # <-- end def
   
    def getAddr(self, rule):
        """
        Gets the email addresses of all clients who are interested in receiving 
        the alert via email from the database if they have not previously been 
        retrieved.
        """
        if (not self._addresses.has_key(rule)):
            cursor = EmailAlert.dbh.cursor()
            sql = '''\
SELECT DISTINCT email_addr FROM alert_users INNER JOIN email_alerts
USING (user_id) WHERE (rule = %s or rule = 'all') and status = 'A'
'''
            n = cursor.execute(sql, (rule))
            if(not n):
                # No addresses found for rule return as there are no users
                # interested in being emailed alerts for this rule
                return
            # <-- end if
            to = []     # List of email addresses
            eAddr = cursor.fetchone()
            while(eAddr):
                to.append(eAddr) 
                eAddr = cursor.fetchone()
            # <-- end while
            
            # Cache retrieved email addresses
            self._addresses[rule] = to
        # <-- end if
    # <-- end def
    
    def send(self, alert): 
        """
        Sends alert using email.
        """
        # Get addresses to send alert to.
        self.getAddr(alert.rule)
        
        # Create text for email message and send it.
        alertURL = "%s/%s.xml" % (Constants.ALERT_WEB_SERVER, alert.alertId)
        body = "Pan-STARRS Rule: %s %s </br> %s" % (alert.rule, alert.message, alertURL)
        reply = 'mops@ifa.hawaii.edu'
        html = """
                <html>
                    <head></head>
                    <body>
                        %s
                    </body>
                </html>
                """ % (body)
 
        msg = MIMEMultipart()
        msg.attach(MIMEText(html, 'html'))

        msg['Subject'] = 'Pan-STARRS Alert: %s - %s' % (alert.rule, alert.subject)
        msg['From'] = reply
        msg['To'] = ""
        self._email.sendmail(reply, self._addresses[alert.rule], msg.as_string())
    # <-- end def
# <-- end class
   
#------------------------------------------------------------------------------
# Function Definitions.
#------------------------------------------------------------------------------ 
def publishAlerts(channel=None, rule=None, noTweet=False, noEmail=False):
    # Get all alert channels that can be used to publish alerts.
    TweetAlert.loadConfig(config_filename)
    channels = TweetAlert.getChannels()
    tweet = {}
    
    if (noTweet):
        logger.info('''ALERTSUBMIT: noTweet option specified. Alerts will NOT 
                       be sent using Twitter.''')
    # <-- end if

    if (noEmail):
        logger.info('''ALERTSUBMIT: noEmail option specified. Alerts will NOT 
                       be sent using email.''')
    # <-- end if
    

    # Open a twitter connection on all configured channels.
    for ch in channels.keys():
        tweet[ch] = TweetAlert(ch)
    # <-- end for
    
    # Create an email object which will be used to send alerts by email.
    email = EmailAlert();
    
    # Get alerts and prepare them for processing.
    Alert.prepareForProcessing(channel, rule)
    alerts = Alert.getAlerts(Constants.AQUEUE_STATUS_READY)
    for a in alerts:
        try:
            # Create VOEvent for each alert.
            if (a.objType == Constants.AQUEUE_TYPE_DERIVED):
                alert = DerivedObjAlert(AlertObj(a.alertId, a.status, 
                                                 a.objId, a.objType, a.dbname, 
                                                 a.classification))
                orbit = alert.alertObj.orbit
                tracklets = alert.alertObj.fetchTracklets()
            else:
                alert = TrackletAlert(AlertObj(a.alertId, a.status, 
                                               a.objId, a.objType, a.dbname,
                                               a.classification))
                orbit = None
                tracklets = (alert.alertObj,)
            # <-- end if
            event = VOEvent(a.dbname, a.objId, orbit,
                            reduce(lambda x, y:x+y,
                                   [t.detections for t in tracklets]))
            # Write voEvent to database.
            a.updateVoEvent(event)
            
            # Write voEvent to psps so that it is available to external users.
            # import pdb; pdb.set_trace()  # Turns on python debugger
            outXML = tempfile.NamedTemporaryFile()
            outXML.write(repr(event))
            outXML.flush()
            logger.debug('ALERTSUBMIT: Writting XML to %s' % (outXML.name))
            os.fsync(outXML.fileno())
            os.system("chmod 644 %s" % (outXML.name))
            os.system("rsync %s mops@%s:/var/www/html/mops/alerts/%s.xml" % 
                      (outXML.name, Constants.ALERT_FILE_SERVER, a.alertId))
            outXML.close()
           
            #-------------------------------------------------------------------
            # Send alert via twitter
            #------------------------------------------------------------------- 
            if (not noTweet):
                # Create text for twitter message and send it.
                alertURL = "%s/%s.xml" % \
                    (Constants.ALERT_WEB_SERVER, a.alertId)
                twitString = "Pan-STARRS Alert: %s \n %s \n %s" % \
                    (a.rule, a.subject, alertURL)
                if (len(twitString) > 140):
                    logger.warn('ALERTSUBMIT: Tweet string "%s" is longer than 140 characters.' % (twitString))
                    twitString = "Pan-STARRS Alert: %s \n %s" % (a.subject, alertURL)
                    logger.warn('ALERTSUBMIT: Shortened string to "%s".' % (twitString))
                # <-- end if
                logger.info("ALERTSUBMIT: %s" % (twitString))
                tweet[a.channel].send(twitString)
                # If channel used to send first tweet is not the ALL channel
                # then send a second tweet on the ALL channel.
                if (a.channel.strip() != CH_ALL.strip()):
                    tweet[CH_ALL].send(twitString)
                # <-- end if
            # <-- end if
            
            #-------------------------------------------------------------------
            # Send alert via email
            #------------------------------------------------------------------- 
            if (not noEmail):
                email.send(a)
            # <-- end if                     
            
            # Update alert in database to indicate that it has been sent.
            a.updateStatus(Constants.AQUEUE_STATUS_DONE)
        except twitter.api.TwitterHTTPError, e:
            logger.error('ALERTSUBMIT: %s' % (str(e)))
        except Exception, e:
            logger.exception('ALERTSUBMIT: %s' % (str(e)))
        # <-- end try
    # <-- end for
# <-- end publishAlerts

def main(args=sys.argv[1:]):
    import optparse
    import sys
    
    # Get user input (tracks file) and make sure that it exists.
    parser = optparse.OptionParser(USAGE)
    
    parser.add_option('--channel',
                      action='store',
                      dest='channel',
                      default=None)
    parser.add_option('--rule',
                      action='store',
                      dest='rule',
                      default=None)
    parser.add_option('--include_synthetic',
                      action='store_true',
                      dest='incSynthetics',
                      default=False)
    parser.add_option('--noTweet',
                      action='store_true',
                      dest='noTweet',
                      default=False)
    parser.add_option('--noEmail',
                      action='store_true',
                      dest='noEmail',
                      default=False)

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

    # Read the alert files and set the ball rolling!
    sys.exit(publishAlerts(options.channel, options.rule, options.noTweet, options.noEmail))
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