#!/usr/bin/env python
"""
plugins.asteroid_1990_KN1

MOPS Alert Rule that issues an alert whenever the asteroids
    1. 1990 KN1
    2. J90K01N
is seen.
"""

import sys
import MOPS.Alerts.Constants
from sets import Set
from base import TrackletRule
from MOPS.Alerts.plugins.Constants import CH_ALL


# Constants

# List of asteroid names. Names must be in all caps as the name of the object
# being compared is automatically converted to upper case prior to the 
#comparision.
ASTEROIDS = Set(['1990 KN1', 'J90K01N'])

class KnownAsteroid(TrackletRule):
    def evaluate(self):
        """
        Return tracklets that contain asteroids in ASTEROIDS set.
        """
        alerts = []
        #Set the channel that will be used to publish the alert.
        self.channel = CH_ALL
        
        for obj in self.newAlerts:
            # Get tracklet and determine if it is known.
            tracklet = obj.alertObj
            
            # Set the default(current) database on the connection to 
            # that containing the subject of the alert. 
            #obj.dbh.select_db(obj.dbname)
            #known = tracklet.fetchKnown(obj.dbh)
            # Restore connection back to database containing alert tables.
            #obj.dbh.select_db(MOPS.Alerts.Constants.AQUEUE_DB_NAME)
            #if (known == None):
                # Tracklet is not known get next tracklet.
            #    continue
            #<-- end if
            
            #if (known.known_name.upper() in ASTEROIDS):
            if (tracklet.knownName and tracklet.knownName.upper() in ASTEROIDS):
                # Flagged object observed generate specific message to be 
                # included in alert
                #msg = "<p>Known Asteroid: %s<br/>" % known.known_name
                msg = "<p>Known Asteroid: %s<br/>" % tracklet.knownName
                msg += "Right Acension: %.3f<br/>" % tracklet.vRa
                msg += "Declination: %.3f<br/>" % tracklet.vDec
                msg += "Velocity: %.3f<br/>" % tracklet.vTot
                msg += "Epoch: %.2f<br/>" % tracklet.extEpoch
                msg += "Magnitude: %.1f<br/>" % tracklet.extMag
                obj.message = msg
                        
                # Generate subject line for alert.
                subj = "Known Object %s [epoch=%.1f, ra=%.3f, dec=%.3f, vel=%.3f]" % (tracklet.knownName, tracklet.extEpoch, tracklet.vRa, tracklet.vDec, tracklet.vTot)
                obj.subject = subj

                alerts.append(obj)
            #<-- end if
        #<-- end for
        return(alerts)
    #<-- end evaluate
#<-- end class

def main(args=sys.argv[1:]):    
    rule = KnownAsteroid(status = 'D')
    alerts = rule.evaluate()
    
    for alert in alerts:
        print(str(alert) + '\n\n')
#<-- end main    

if(__name__ == '__main__'):
    sys.exit(main())
#<-- end if
