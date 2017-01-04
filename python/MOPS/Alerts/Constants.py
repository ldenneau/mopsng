"""
Alert specific constants.
"""

# Alert queue.  The status name indicates what has *been done* to some object,
# and not what the object is "ready for".
AQUEUE_STATUS_NEW = 'N'             # DO yet to be run against alert generation.
AQUEUE_STATUS_READY = 'R'           # DO ready for processing.
AQUEUE_STATUS_DONE = 'D'            # DO processed against alert generation.

AQUEUE_ALLOWED_STATUS = (
    AQUEUE_STATUS_NEW, 
    AQUEUE_STATUS_READY, 
    AQUEUE_STATUS_DONE
)
AQUEUE_LOCK_ACQUIRED = 'L'          # DO lock the aqueue table.
AQUEUE_LOCK_RELEASED = 'N'          # DO unlock the aqueue table.

AQUEUE_DB_NAME = "export"           # Name of the database instance that hosts
                                    # alert related tables.
AQUEUE_TYPE_DERIVED = 'D'
AQUEUE_TYPE_TRACKLET = 'T'
AQUEUE_ALLOWED_TYPE = (AQUEUE_TYPE_DERIVED, AQUEUE_TYPE_TRACKLET)

ALERT_DEFAULT_CHANNEL = 'all'
ALERT_LOG_FILE = 'alerts.log'
ALERT_FILE_SERVER = 'web01.psps.ifa.hawaii.edu'
ALERT_WEB_SERVER = 'http://%s/mops/alerts' % ALERT_FILE_SERVER
ALERT_CONSUMER_KEY='nxiAcpr26suOcHv2jfOpg'
ALERT_CONSUMER_SECRET='SalrBJ8s4r0QdMPYpSpgvWeergqiBgep1xo8xd8PoM'
ALERT_CONFIG_FILE ='twitterConfig'
