#!/usr/bin/env python
"""
plugins.phaethon_family

MOPS Alert Rule that detects objects that are part of the Phaethon family.
    a>=0.9 AU && a<=1.8 AU && e>0.7 && i>14.0
"""

import sys
from base import DerivedObjectRule
from MOPS.Alerts.plugins.Constants import CH_ALL
from decimal import *

class Phaethon_Family(DerivedObjectRule):
    def IsPhaethonFam(self, obj):
        """
        Return True if obj has 0.9 AU <= a <= 1.8 AU && e>0.7 && i>14.0

        q = a * ( 1 - e )

        @param obj: DerivedObject instance
        """
        getcontext().prec = 3
        i = Decimal(str(obj.alertObj.orbit.i))
        e = Decimal(str(obj.alertObj.orbit.e))
        q = Decimal(str(obj.alertObj.orbit.q))
        h = Decimal(str(obj.alertObj.orbit.h_v))
        a = q / (Decimal(1) - e)

        result = (e > Decimal("0.7") and a <= Decimal("1.8") and 
                  a >= Decimal("0.9") and i > Decimal("14.0"))
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
        alerts = []
        #Set the channel that will be used to publish the alert.
        self.channel = CH_ALL

        return([obj for obj in self.newAlerts if self.IsPhaethonFam(obj)])
    #<-- end def
#<-- end class

# Testing stub
def main(args=sys.argv[1:]): 
   
    rule = PhaethonFam(status = 'N')
    alerts = rule.evaluate()
    
    for alert in alerts:
        print(str(alert) + '\n\n')
#<-- end main    

if(__name__ == '__main__'):
    sys.exit(main())
#<-- end if
