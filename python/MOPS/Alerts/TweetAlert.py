#!/usr/bin/python
'''
Created on May 19, 2011

@author: Denver Green
'''

USAGE = '''USAGE:

 TweetAlert [options]


OPTIONS:

--config        read username and password from given config
                file (default ~/.twitter)
                
--channel       MOPS alert channel to send the alert to.

 -h --help      prints this help

CONFIG FILE

 The config file is named .twitter.
 Each channel you wish to have will have it's own section, with the section 
 name being the name of the channel. Each channel will define one name value 
 pair which specifies the file in which the Twitter oAuth authentication
 token for the channel is.

EXAMPLE CONFIG FILE

    [all]
    oAuth_filename: ~/.twitter_all

    [comets]
    oAuth_filename: ~/.twitter_comets

    [asteroids]
    oAuth_filename: ~/.twitter_asteroids
'''
#------------------------------------------------------------------------------
# Imports.
#------------------------------------------------------------------------------

import sys
import os.path
import logging
import twitter
import random
import MOPS.Alerts.Constants as Constants
import MOPS.Utilities as Utilities
from MOPS.Alerts.Exceptions import AlertException
from MOPS.Alerts.oauth_dance import oauth_dance
from twitter import OAuth, read_token_file, Twitter
 
try:
    from ConfigParser import SafeConfigParser
except ImportError:
    from configparser import ConfigParser as SafeConfigParser

#------------------------------------------------------------------------------
# Global variables.
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
config_path = os.path.join(environment['HOMEDIR'],'config','alerts')
config_filename = os.path.join(config_path, Constants.ALERT_CONFIG_FILE)

#------------------------------------------------------------------------------
# Class Definitions.
#------------------------------------------------------------------------------
class TweetAlert(object):
    """
    Class that handles the publishing of alerts to a channel which is 
    associated with a Twitter account.
    """

    channels={}
    # Get logger and set logging level. Typically set level to info.
    # Add null handler to logger to avoid the No handlers could be found
    # for logger XXX error if a handler is not found in a higher level logger
    logger = logging.getLogger('MOPS.Alerts.TweetAlert')
    h = Utilities.NullHandler()
    logger.addHandler(h)

    #--------------------------------------------------------------------------   
    # Class methods.
    #--------------------------------------------------------------------------  
    @classmethod
    def loadConfig(cls, filename):
        """ Parse the contents of the provided configuration file. This is a 
        static class method so that it can be used directly by the code in the 
        main function.
    
        @param filename: Name and location of the MOP Alerts configuration file.
    
        @return: Returns a dictionary keyed using the channel name containing  
                 the name and location of the file containing the oAuth 
                 authentication token for the Twitter account used by the 
                 channel
             
        @rtype:  dict
        """
        if os.path.exists(filename):
            cp = SafeConfigParser()
            cp.read([filename])
            # For each channel get oAuth file.
            for channel in cp.sections():
                if cp.has_option(channel, 'oAuth_filename'):
                    cls.channels[channel] = cp.get(channel, 'oAuth_filename')
                # <-- end if
            # <-- end for
        else:
            cls.logger.error("TWEETALERT: Cannot open configuration file %s." % filename)
            raise SystemExit(1)
        # <-- end if
        return cls.channels
    # <-- end loadConfig

    @classmethod
    def getChannels(cls):
        if (cls.channels == None):
            TweetAlert.logger.error("TWEETALERT: Configuration file must be loaded first using loadConfig!")
        # <-- end if
        return (cls.channels)
    # <-- end getChannels

    #--------------------------------------------------------------------------   
    # Instance methods.
    #--------------------------------------------------------------------------              
    def __init__(self, channel=Constants.ALERT_DEFAULT_CHANNEL, configFile=config_filename):
        """
        Constructor
        """
        self.channel = channel
        self._configFile = configFile
        self.twitter = None
        self.openChannel()
    # <-- end __init__

    def openChannel(self):
        """ Open the specified MOPS alert channel.
    
        @param channel: MOPS alert channel to open. If not defined then the all
                        channel will be opened.
    
        @param configFile: Full path to the configuration file that specifies  
                           all of the MOPS alert channels and the location of 
                           the oAuth token for the channel.
                    
        @return: Returns a fully initialized Twitter object that can be used to 
                 post and retrieve tweets from twitter.
             
        @rtype: twitter.api.Twitter
        """
    
        # Verify validity of provided channel
        if (TweetAlert.channels == None):
            TweetAlert.channels = self.loadConfig(config_file)
        if (self.channel not in TweetAlert.channels):
            TweetAlert.logger("TWEETALERT: %s is not a recognized alert channel." % 
                              self.channel)
            raise SystemExit(1)
        # <-- end if
    
        # Get the name and location of the file containing the Twitter oAuth 
        # token. If the file does not exist then throw an exception.
        oauth_filename = os.path.join(config_path, TweetAlert.channels[self.channel])
        if (not os.path.exists(oauth_filename)):
            logger.error("TWEETALERT: Cannot find the oAuth token file %s" % 
                         (oauth_filename))
            raise AlertException('TWEETALERT: Cannot find the oAuth token file %s' % 
                                 (oauth_filename))
        # <-- end if
    
        # Retrieve oAuth token and open a connection to Twitter.
        oauth_token, oauth_token_secret = read_token_file(oauth_filename)
        self.twitter = Twitter(auth=OAuth(oauth_token, oauth_token_secret, 
                               Constants.ALERT_CONSUMER_KEY, 
                               Constants.ALERT_CONSUMER_SECRET),
                               secure=True,
                               api_version='1',
                               domain='api.twitter.com')
        return self.twitter
    # <-- end openChannel    
        
    def send(self, alert):
        """
        Publishes the alert to Twitter.
        
        @param alert: Message to be sent.
        """
        self.twitter.statuses.update(status=alert)
    # <-- end send
# <-- end TweetAlert   
     
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
    the files (one for each channel) that contain the oAuth tokens issued by
    Twitter from the command line. Each MOP Alert channel will be associated 
    with a twitter account in a one to one relationship.
    """
    import optparse
    parser = optparse.OptionParser(USAGE)
    parser.add_option('--config',
                      action='store',
                      dest='configFile',
                      default=config_filename)
    parser.add_option('--channel',
                      action='store',
                      dest='channel',
                      default=Constants.ALERT_DEFAULT_CHANNEL)
    
    # Get the command line options and also whatever is passed on STDIN.
    (options, args) = parser.parse_args()

    try:
        channels = TweetAlert.loadConfig(options.configFile)
        for channel in channels:
            alert = TweetAlert(channel, options.configFile)
            alert.openChannel()
            wordLen = 0
            total = 0
            sentence = " "
            while (total < 100):
                wordLen = random.randint(4, 15)
                word = getRandomWord(wordLen)
                sentence += word + " "
                total += wordLen
            # <-- while
            msg = "%s channel:" % (channel) + sentence
            alert.send(msg)
        # <-- end for
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