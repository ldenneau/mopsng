"""
Provides abstraction and services for a MOPS instance (database + runtime environment).
"""

__author__ = 'Larry Denneau, Institute for Astronomy, University of Hawaii'
__date__ = '$Date$'
__version__ = '$Revision$'[11:-2]

import pdb
import os
import os.path
import logging
import MySQLdb
from contextlib import contextmanager

import MOPS.Config
import MOPS.Lib
import MOPS.Alerts.Constants as Constants
import MOPS.Utilities as Utilities


class Instance:
    def __init__(self, dbname=None):

        if not os.environ['MOPS_HOME']:
            raise RuntimeError('MOPS_HOME must be set.')    # need MOPS_HOME for anything at all
        # <-- end if
        if not dbname:
            dbname = os.environ.get('MOPS_DBNAME', None)
        # <-- end if
        if not dbname:
            dbname = os.environ.get('MOPS_DBINSTANCE', None)
        # <-- end if
        if not dbname:
            raise RuntimeError("Can't figure out a DBNAME.")    # give up
        # <-- end if
        
        self._dbname = dbname 
        self._dbh = None
        self._config = None
        
        # Get logger and set logging level. Typically set level to info.
        # Add null handler to logger to avoid the No handlers could be found
        # for logger XXX error if a handler is not found in a higher level logger
        self._logger = logging.getLogger('MOPS.Instance')
        h = Utilities.NullHandler()
        self._logger.addHandler(h)


        # Store useful environment-type stuff.
        homedir = os.environ.get('MOPS_HOME', '/usr/local/MOPS')
        vardir = os.path.join(homedir, 'var', self._dbname)

        self._environment = {
            'HOMEDIR' : homedir,
            'VARDIR' : vardir,
            'CONFIGDIR' : os.path.join(vardir, 'config'),
            'LOGDIR' : os.path.join(vardir, 'log'),
            'NNDIR' : os.path.join(vardir, 'nn'),
            'OBJECTSDIR' : os.path.join(vardir, 'objects'),
            'LSDDIR' : os.path.join(vardir, 'lsd'),
        }
        
        # Database stuff.
        # Load DB and backend configs.
        self._backend_cfg = self._load_config_module('backend')
        self._cluster_cfg = self._load_config_module('cluster')

        self._db_hostname = os.environ.get('MOPS_DBHOSTNAME', self._backend_cfg.get('hostname'))
        self._db_port = os.environ.get('MOPS_DBPORT', self._backend_cfg.get('port', 3306))
        self._db_username = os.environ.get('MOPS_DBUSERNAME', self._backend_cfg.get('username'))
        self._db_password = os.environ.get('MOPS_DBPASSWORD', self._backend_cfg.get('password'))
    # <-- end __init__

    #--------------------------------------------------------------------------   
    # Getter/Setter methods.
    #--------------------------------------------------------------------------   

    @property
    def dbname(self):
        """Name of database schema to use."""
        return self._dbname
    # <-- end dbname 

    @property
    def dbh(self):
        """Connection to database."""
        if self._dbh is None:
            self._dbh = self.new_dbh()   # not defined, create one
        # <-- end if
        return self._dbh
    # <-- end dbh

    @property
    def config(self):
        """ MOPS configuration data."""
        if (self._config == None):
            # Master config, might be in DB. Retrieve configuration from database
            # only if we are not connecting to the database that hosts the alert
            # tables.
            config_text = ''
            if (self._dbname != Constants.AQUEUE_DB_NAME):
                try:
                    config_text = MOPS.Config.RetrieveDB(self.get_dbh())
                except Exception, e:
                    self._logger.warn("Database does not contain the master config; using files instead.")
                    self._logger.warn(e)
            # <-- end if
            if config_text:
                self._config = MOPS.Config.LoadString(config_text)
            else:
                self._config = self._load_config_module('master')
            # <-- end if
            self._config['backend'] = self._backend_cfg    # put in main config
            self._config['cluster'] = self._cluster_cfg    # put in main config
        # <-- end if
        return self._config
    # <-- end config

    @property
    def environment(self):
        """List of directories used by MOPS."""
        return self._environment
    # <-- end environment

    @property
    def dbHost(self):
        """"Domain name or IP address of the server hosting the MySQL server."""
        return self._db_hostname
    # <-- end dbHost

    @property
    def dbPort(self):
        """TCP Port used to communicate with the MySQL database."""
        return self._db_port
    # <-- end dbPort

    @property
    def dbUsername(self):
        """"User name used to connect to the database."""
        return self._db_username
    # <-- end dbUsername

    @property
    def logger(self):
        """Logger used by Instance to output messages."""
        return self._logger
    # <-- end logger 

    def __del__(self):
        """
        Close database connection if it's still open.
        """
        if self._dbh:
            self._dbh.close()
            self._dbh = None
        # <-- end if   
        self._logger.debug("Deleting Instance object")
    # <-- end __del__

    def __str__(self):
        showList = sorted(set(self.__dict__))
        return Utilities.toString(self, showList)
    # <-- end __str__

    def _load_config_module(self, which):
        """
        Find the config file for the specified module and read it in.  Return
        the parsed configuration object.

        First look in the instance's VARDIR, then $MOPS_HOME/config.
        """
        inst_config = os.path.join(self._environment['CONFIGDIR'], which + '.cf')
        mops_config = os.path.join(os.environ['MOPS_HOME'], 'config', which + '.cf')
        if (os.path.isfile(inst_config)):
            return MOPS.Config.LoadFile(inst_config)
        elif (os.path.isfile(mops_config)):
            return MOPS.Config.LoadFile(mops_config)
        else:
            self._logger.warn('no config found for %s' % which)
            return {}
        # <-- end if
    # <-- end _load_config_module
    
    def new_dbh(self):
        """
        Create a new database instance handle.
        """
        self._logger.debug("Opening %s as %s on host %s using port %d", 
                          self._dbname, self._db_username, self._db_hostname,
                          self._db_port)
        return MySQLdb.connect(   
            user=self._db_username,
            passwd=self._db_password,
            host=self._db_hostname,
            port=self._db_port,
            db=self._dbname
        )
    # <-- end new_dbh

    def forget_dbh(self):
        """
        Unset our DB handle to that next attempt at usage establishes
        a new connection.  Normally you would only use this after a potentially
        lengthy operation where the database connection might time out (yes, should
        be fixed in mysql configs).
        """
        if (self._dbh):
            self._dbh.close()
        # <-- end if
        self._dbh = None
    # <-- end forget_dbh

    def getEnvironment(self, key):
        return self._environment.get(key)
    # <-- end getEnvironment

    def getConfig(self):
        return self.config
    # <-- end getConfig
    
    def getLogger(self):
        return self.logger
    # <-- end getLogger

    def get_dbh(self):
        return self.dbh
    # <-- end get_dbh

    def makeNNDir(self, nn, subsys, force_empty=False):
        """
        Given a processing night number and pipeline subsystem,
        create a directory for processing in the MOPS instance's
        directory tree.
        """
        new_dir = os.path.join(self.getEnvironment('NNDIR'), "%05d" % nn, subsys);

        if force_empty and os.path.isdir(new_dir):
            tries = 0
            while tries < 10:
                test_name = newdir + '.' + str(tries)
                if not os.path.isdir(test_name):
                    try:
                        os.rename(new_dir, test_name)
                        break
                    except Exception, e: 
                        raise RuntimeException("can't rename %s to %s" % (new_dir, test_name))
                    # <- try
                tries += 1
            # <- while
            if os.path.isdir(new_dir):
                raise RuntimeException("can't prevent collision with " + new_dir)
        # <- if

        if not os.path.isdir(new_dir):
            os.makedirs(new_dir)
            self._logger.info("Created " + new_dir)
        # <-- end if
        return new_dir
    # <-- end makeNNDir
# <-- end instance

@contextmanager
def Transaction(connection):
    """
    Provide Python context manager services for easy transactions/atomic DB commits.
    Usage:

    from MOPS.ContextManager import Transaction
    db = DatabaseConnection()
    with MOPS.Instance.Transaction(db) as cursor:
      do_stuff()
    """
    cursor = connection.cursor()
    try:
        yield cursor
    except:
        connection.rollback()
        raise
    else:
        connection.commit()

