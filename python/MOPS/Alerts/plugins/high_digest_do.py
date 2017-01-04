#!/usr/bin/env python
"""
plugins.high_digest_do

MOPS Alert Rule that returns Derived Objects which
    1. Have a digest score > MIN_DIGEST
    2. Have been submitted to the NEOCP
"""

import sys
from base import DerivedObjectRule
from MOPS.Alerts.plugins.Constants import CH_ALL


# Constants
MIN_DIGEST = 20      # Minimum digest score a tracklet must exceed.            

class HighDigestDerivedObj(DerivedObjectRule):
    def evaluate(self):
        """
        Return Derived Objects that have a been submitted to the MPC that
        contain a tracklet with a digest score above MIN_DIGEST
        """
        alerts = []
        #Set the channel that will be used to publish the alert.
        self.channel = CH_ALL
        
        # Get derived objects that are ready for processing.
        for obj in self.newAlerts:
            tracklets = obj.alertObj.fetchTracklets(obj.dbh)
            orbit = obj.alertObj.orbit
            found = False
            for t in tracklets:
                submissions = t.fetchSubmissons(obj.dbh)
                if (submissions):
                    s = max(submissions, key = lambda x: x.digest)
                    if (s.digest > MIN_DIGEST):
                        # Generate specific message to be included in alert
                        msg = "<p>Designation: %s<br/>" % s.desig
                        msg += "Digest: %d<br/>" % s.digest
                        msg += "e: %.3f<br/>" % orbit.e
                        msg += "q: %.3f<br/>" % orbit.q
                        msg += "i: %.3f<br/>" % orbit.i
                        msg += "H: %.1f<br/>" % orbit.h_v
                        msg += "Derived Object (Internal Link): <a href='http://mopshq2.ifa.hawaii.edu/model/%s/search?id=L%d'>L%d</a><br/>" % (obj.dbname, obj.alertObj._id, obj.alertObj._id)
                        msg += "Derived Object (External Link): <a href='http://localhost:8080/model/%s/search?id=L%d'>L%d</a></p>" % (obj.dbname, obj.alertObj._id, obj.alertObj._id)
                        obj.message = msg
                        
                        # Generate subject line for alert.
                        subj = "%s [h_v=%.1f, q=%.3f, e=%.3f, i=%.3f]" % (s.desig, orbit.h_v, orbit.q, orbit.e, orbit.i)
                        obj.subject = subj

                        alerts.append(obj)
                        break
                    #<-- end if
                #<-- end if
            #<-- end for
        #<-- end for
        return(alerts)
    #<-- end evaluate
#<-- end class

def main(args=sys.argv[1:]):    
    rule = HighDigestDerivedObj(status = 'D')
    alerts = rule.evaluate()
    
    for alert in alerts:
        print(str(alert) + '\n\n')
#<-- end main    

if(__name__ == '__main__'):
    sys.exit(main())
#<-- end if
