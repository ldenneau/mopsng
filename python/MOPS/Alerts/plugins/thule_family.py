#!/usr/bin/env python
"""
plugins.thule_family

MOPS Alert Rule that detects Thule like asteroids in 4:3 resonance with Jupiter.
    a>=4.25 AU && a<=4.35 AU
"""

import sys
from base import DerivedObjectRule
from MOPS.Alerts.plugins.Constants import CH_ALL
from decimal import *

class Thule_Family(DerivedObjectRule):
    def IsThuleFam(self, obj):
        """
        Return True if obj has 4.25 AU <= a <=4.35 AU

        q = a * ( 1 - e )

        @param obj: DerivedObject instance
        """
        getcontext().prec = 3
        i = Decimal(str(obj.alertObj.orbit.i))
        e = Decimal(str(obj.alertObj.orbit.e))
        q = Decimal(str(obj.alertObj.orbit.q))
        h = Decimal(str(obj.alertObj.orbit.h_v))
        a = q / (Decimal(1) - e)

        result = (a <= Decimal("4.35") and a >= Decimal("4.25"))
        if (result):
            # Generate specific message to be included in alert
            msg = "<p>a: %.3f<br/>" % a
            msg += "e: %.3f<br/>" % e
            msg += "q: %.3f<br/>" % q
            msg += "i: %.3f<br/>" % i
            msg += "H: %.1f<br/>" % h
            msg += "Derived Object (Internal Link): <a href='http://mopshq2.ifa.hawaii.edu/model/%s/search?id=L%d'>L%d</a><br/>" % (obj.dbname, obj.alertObj._id, obj.alertObj._id)
            msg += "Derived Object (External Link): <a href='http://localhost:8080/model/%s/search?id=L%d'>L%d</a></p>" % (obj.dbname, obj.alertObj._id, obj.alertObj._id)
            obj.message = msg
                           
            # Generate subject line for alert.
            subj = "[a=%.3f,e=%.3f,q=%.3f,i=%.3f,H=%.3f]\n" % (a, e, q, i, h)
            obj.subject = subj
        # <-- end if
        return(result)        
    # <-- end def

    def evaluate(self):
        """
        Return Derived Objects that have a been submitted to the MPC that
        contain a tracklet with a digest score above MIN_DIGEST
        """
        alerts = []
        #Set the channel that will be used to publish the alert.
        self.channel = CH_ALL
        
        return([obj for obj in self.newAlerts if self.IsThuleFam(obj)])
    #<-- end evaluate
#<-- end class

# Testing stub
def main(args=sys.argv[1:]): 
   
    rule = ThuleFam(status = 'N')
    alerts = rule.evaluate()
    
    for alert in alerts:
        print(str(alert) + '\n\n')
#<-- end main    

if(__name__ == '__main__'):
    sys.exit(main())
#<-- end if