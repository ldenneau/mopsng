#!/usr/bin/env python
'''
Created on July 22, 2011

@author: Denver Green
'''
USAGE = """

alerttoken.py [OPTIONS]

Options:
    --file file_name        Full path of where to save tokens returned by twitter.
    --channel channel_name  Name of the channel that is to be associated with
                            the twitter account.

Description:
alerttoken will retrieve the oAuth tokens required to connect to twitter and 
saves these to the file specified. After retrieving the token alerttoken will
test them by sending a test message to the associated twitter accounts. 
  
"""
#------------------------------------------------------------------------------
# Imports.
#------------------------------------------------------------------------------
import logging
import MOPS.Alerts.Constants as Constants
import MOPS.Utilities as Utilities
import optparse
import os
import sys
import random
from MOPS.Alerts.oauth_dance import oauth_dance
from twitter import OAuth, read_token_file, Twitter, TwitterHTTPError

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
def getRandomWord(wordLen):
    word = ''
    for i in range(wordLen):
        word += random.choice('ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789')
    return word
# <-- end getRandomWord

def main(args=sys.argv[1:]):
    """
    Main purpose of this method is to allow the MOPS administrator to create
    the file that contains the oAuth tokens issued by Twitter from the command 
    line. Each MOP Alert channel will be associated with a twitter account in 
    a one to one relationship.
    """
    import optparse
    parser = optparse.OptionParser(USAGE)
    parser.add_option('--file',
                      action='store',
                      dest='authFile',
                      default=None)
    parser.add_option('--channel',
                      action='store',
                      dest='channel',
                      default=None)
    
    # Get the command line options and also whatever is passed on STDIN.
    (options, args) = parser.parse_args()
    
    # Make sure that we have what we need. 
    if(not options.authFile): 
       parser.error('No --file specified. Please specify the full path of where to save oAuth tokens.') 
    # <-- end if 

    if (not options.channel):
        parser.error('No --channel specified. Please specify the name of the alert channel')
    # <-- end if

    try:
        # Get the name and location of the file containing the Twitter oAuth 
        # token. If the file does not exist then go through the process of 
        # logging into the Twitter account in order to obtain the oAuth token 
        # and write it to disk.
        oauth_filename = os.path.expanduser(options.authFile)
        
        # Check to see if file already exists. If it does ask user if it should
        # be overwritten.
        if (os.path.exists(oauth_filename)):
            response = raw_input('%s already exists, do you want to over write it (Y/N)? [N]: ' % oauth_filename)
            if (response.upper() != "Y" and response.upper() != "YES"):
                return
            # <-- end if
        # <-- end if
        
        
        # Get directory portion and exit if directory does not exist.
        oauth_dir = os.path.dirname(oauth_filename)
        if (oauth_dir and not os.path.exists(oauth_dir)):
            logger.error("The directory %s does not exists!" % (oauth_dir))
            return
        # <-- end if
        
        # Go through the process of logging into the Twitter account in order 
        # to obtain the oAuth token and write it to disk.    
        oauth_dance("MOPS %s Alerts" % (options.channel), 
                        Constants.ALERT_CONSUMER_KEY, 
                        Constants.ALERT_CONSUMER_SECRET,
                        oauth_filename)
        # <-- end if
        
        # Retrieve oAuth token and open a connection to Twitter.
        oauth_token, oauth_token_secret = read_token_file(oauth_filename)
        twitter = Twitter(auth=OAuth(oauth_token, oauth_token_secret, 
                          Constants.ALERT_CONSUMER_KEY, 
                          Constants.ALERT_CONSUMER_SECRET),
                          secure=True,
                          api_version='1',
                          domain='api.twitter.com')

        # Create a random message to send using token just created.
        wordLen = 0
        total = 0
        sentence = " "
        while (total < 40):
            wordLen = random.randint(4, 15)
            word = getRandomWord(wordLen)
            sentence += word + " "
            total += wordLen
        # <-- while
        msg = "Test message to %s channel:" % (options.channel) + sentence
        twitter.statuses.update(status=msg)
        logger.info("Sent the following tweet.")
        logger.info(msg)
    except TwitterHTTPError, e:
        logger.error(str(e))
        raise SystemExit(1)
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