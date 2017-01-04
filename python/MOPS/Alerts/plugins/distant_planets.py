#!/usr/bin/env python
"""
plugins.distant_planets

MOPS Alert Rule that returns DerivedObjects for which
    a > 60 AU
"""

from base import DerivedObjectRule
from MOPS.Alerts.plugins.Constants import CH_ALL
import sys


class DistantPlanets(DerivedObjectRule):
    """
    Return all new or newly modified DerivedObject instances.
    """
    def isDistantPlanet(self, obj):
        """
        Return True if obj has a > 60 AU

        q = a * ( 1 - e )

        @param obj: DerivedObject instance
        """
        a = obj.alertObj.orbit.q / (1 - obj.alertObj.orbit.e)
        result = (a > 60)
        if (result):    
            # Generate specific message to be included in alert
            msg = "<p>a: %.3f<br/>" % a
            msg += "e: %.3f<br/>" % obj.alertObj.orbit.e
            msg += "q: %.3f<br/>" % obj.alertObj.orbit.q
            msg += "i: %.3f<br/>" % obj.alertObj.orbit.i
            msg += "H: %.1f<br/>" % obj.alertObj.orbit.h_v
            msg += "Derived Object (Internal Link): <a href='http://mopshq2.ifa.hawaii.edu/model/%s/search?id=L%d'>L%d</a><br/>" % (obj.dbname, obj.alertObj._id, obj.alertObj._id)
            msg += "Derived Object (External Link): <a href='http://localhost:8080/model/%s/search?id=L%d'>L%d</a></p>" % (obj.dbname, obj.alertObj._id, obj.alertObj._id)
            obj.message = msg
                           
            # Generate subject line for alert.
            subj = "[a=%.3f,e=%.3f,q=%.3f,i=%.3f,H=%.3f]\n" % (a, obj.alertObj.orbit.e, obj.alertObj.orbit.q, obj.alertObj.orbit.i, obj.alertObj.orbit.h_v)
            obj.subject = subj
        # <-- end if
        return(result)        
    # <-- end def
    
    def evaluate(self):
        """
        Return a list of DerivedObject instances that satisfy the rule.
        """
        #Set the channel that will be used to publish the alert.
        self.channel = CH_ALL
        return([obj for obj in self.newAlerts \
                if self.isDistantPlanet(obj)])
# <-- end class

def main(args=sys.argv[1:]):
    rule = DistantPlanets()
    alerts = rule.evaluate()
    
    for alert in alerts:
        print(alert + '\n\n')
# <-- end main    

if(__name__ == '__main__'):
    sys.exit(main())
# <-- end if