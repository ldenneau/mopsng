#!/usr/bin/env python
"""
plugins.unlinked_fast_movers

MOPS Alert Rule that returns Tracklets which
    1. Are unlinked
    2. Have velocity > MIN_VELOCITY
if the current simulation mjd is > start of OC + 19


This is an example of a fairly complicated rule that does not access
DerivedObject instaces at all but rather builds it own queries to the database.
It also keeps track of its state in a specialized table: 'alerted_tracklets'.
"""

import sys
import time
import MOPS.Utilities
import MOPS.Lib as Lib
from MOPS.DerivedObject import DerivedObject
from MOPS.Tracklet import Tracklet
from MOPS.Constants import TRACKLET_STATUS_UNATTRIBUTED
from base import TrackletRule


# Constants
MIN_VELOCITY = 0.99      # Minimum velocity a tracklet must have.            

class UnlinkedFastMovers(TrackletRule):
    def isFastMover(self, tracklet):
        """
        Return True if the tracklet has a velocity > MIN_VELOCITY deg/day

        @param obj: tracklet to be tested.
        @rtype: boolean
        """
        return(tracklet.vTot > MIN_VELOCITY)
    # <-- end is FastMover
    
    def evaluate(self):
        """
        Return a list of Tracklet instances that satisfy the rule.

        Processing is a bit more complicated that usual: first determine the
        current simulation MJD. Then determine the MJD of the start of the
        current simulation OC (observation cycle) (mjd0).

        If the current MJD < mjd0 + MIN_DELTA_MJD then return None; otherwise 
        fetch all unattributed Tracklets belonging to the OC.

        Return all unattributed Tracklets which have a velocity > MIN_VELOCITY 
        deg/day.
        
        NEW NOTE
        This alert will no longer calculate the number of nights into the OC
        cycle as the alert system is designed to alert on objects on a nightly
        basis, not on historical observations made in the past. If we want to 
        do this sort of analysis then the Pan Starrs Science Database should be
        used.
        """
        alerts = []

        # Get tracklets that are ready for processing.
        for obj in self.newAlerts:
            if (obj.alertObj.status == TRACKLET_STATUS_UNATTRIBUTED):
                # Process only unattributed tracklets.
                if (self.isFastMover(obj.alertObj)):
                    alerts.append(obj)
                # <-- end if
            # <-- end if
        # <-- end for
        return(alerts)
    # <-- end evaluate
# <-- end UnlinkedFastMovers

def main(args=sys.argv[1:]):    
    rule = UnlinkedFastMovers(status = 'D')
    alerts = rule.evaluate()
    
    for alert in alerts:
        print(str(alert) + '\n\n')
#<-- end main    

if(__name__ == '__main__'):
    sys.exit(main())
# <-- end if
