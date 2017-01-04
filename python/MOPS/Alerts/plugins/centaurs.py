#!/usr/bin/env python
"""
plugin.centaurs

MOPS Alert Rule that returns DerivedObjects that can be defined as
Centaurs (5.2 < a < 29, 5.2 < q < 29).

Added May 14th 2008 by Tommy Grav.
"""
from base import DerivedObjectRule
from MOPS.Alerts.plugins.Constants import CH_ALL
import sys


class Centaurs(DerivedObjectRule):
    """
    Return all new or newly modified DerivedObjects instances.
    """
    def is_centaur(self,obj):
        """
        Return True if obj has 5.2 < a < 29 and 5.2 < q < 29,
        where q = a*(1 - e)
        
        @param obj: DerivedObject instance
        """
        a = obj.alertObj.orbit.q/(1 - obj.alertObj.orbit.e)
        result = (5.2 < a < 29. and 5.2 < obj.alertObj.orbit.q < 29)
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
        Return a list of DerivedObjects that satisfy the rule.
        """
        #Set the channel that will be used to publish the alert.
        self.channel = CH_ALL
        return([obj for obj in self.newAlerts if self.is_centaur(obj)])
# <-- end class

def main(args=sys.argv[1:]):
    rule = Centaurs()
    alerts = rule.evaluate()
    
    for alert in alerts:
        print(alert + '\n\n')
# <-- end main    

if(__name__ == '__main__'):
    sys.exit(main())
# <-- end if