#!/usr/bin/env python
"""
plugins.comets

MOPS Alert Rule that returns DerivedObjects for which
    ( a>4.5 AU && e>0.5 ) || e>0.95
"""

from base import DerivedObjectRule
import sys
from math import *
from MOPS.Alerts.plugins.Constants import CH_ALL
from decimal import *

class Comets(DerivedObjectRule):
    """
    Return all new or newly modified DerivedObject instances.
    """
    def IsComet(self, obj):
        """
        The classical comet orbital criterion is the Tisserand paramter 
        (with respect to Jupiter) which is defined by:

        T(J) = a(J) / a + 2cos(i) * [(1-e^2) * a/a(J)]^0.5

        where a(J) is the semimajor axis of Jupiter, and a,e,i are the 
        semimajor axis, eccentricity, and inclination of the object in question.  
        Comets will typically have T(J) < 3 and asteroids will typically have 
        T(J) > 3.

        While the line is a bit blurry for T(J) values very close to 3, it's 
        generally a good rule of thumb, and certainly a very widely-used one.
        """
        aj = Decimal('5.203');    # Semi-major axis of jupiter in AU
        q = Decimal(str(obj.alertObj.orbit.q))  # Perihelion distance in AU
        e = Decimal(str(obj.alertObj.orbit.e))  # Eccentricity
        a = Decimal(str(q / (Decimal(1) - e)))  # Semi-major axis in AU
        i = Decimal(str(radians(obj.alertObj.orbit.i))) # Inclination in radians
        
        Tj = Decimal(str(aj / a)) 
        Tj += Decimal (str(sqrt((1 - e ** 2) * a / aj))) 
        Tj *= Decimal(str(2 * cos(i)))
        result = (Tj < 3)
        if (result):    
            # Generate specific message to be included in alert
            msg = "<p>a: %.3f<br/>" % a
            msg += "e: %.3f<br/>" % e
            msg += "q: %.3f<br/>" % q
            msg += "i: %.3f<br/>" % i
            msg += "H: %.1f<br/>" % obj.alertObj.orbit.h_v
            msg += "Derived Object (Internal Link): <a href='http://mopshq2.ifa.hawaii.edu/model/%s/search?id=L%d'>L%d</a><br/>" % (obj.dbname, obj.alertObj._id, obj.alertObj._id)
            msg += "Derived Object (External Link): <a href='http://localhost:8080/model/%s/search?id=L%d'>L%d</a></p>" % (obj.dbname, obj.alertObj._id, obj.alertObj._id)
            obj.message = msg
                           
            # Generate subject line for alert.
            subj = "[a=%.3f,e=%.3f,q=%.3f,i=%.3f,H=%.3f]\n" % (a, e, q, i, obj.alertObj.orbit.h_v)
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
        return([obj for obj in self.newAlerts if self.IsComet(obj)])
    # <-- end def
# <-- end class

def main(args=sys.argv[1:]):
    rule = Comets(status = 'D')
    alerts = rule.evaluate()
    
    for alert in alerts:
        print(str(alert) + '\n\n')
    # <-- end for
# <-- end def
    

if(__name__ == '__main__'):
    sys.exit(main())
# <-- end if
