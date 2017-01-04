#!/usr/bin/env python
"""
plugins.unusual_light_curve

MOPS Alert Rule that returns Tracklets for which either one is true:
    1. The known semi-major axis (a) is greater than 3.0 and the known absolute 
       magnitude (h_v) is < 13.0. This will filter out close and small 
       asteroids. 

    2. The difference in magnitude between detections in a given tracklet
       is > 0.5 mag.
       
    3. There are at least three detections in the tracklet.
"""

from base import TrackletRule
from MOPS.Instance import Instance
from MOPS.Alerts.plugins.Constants import CH_ALL
from decimal import *
import sys
import MOPS.Lib

class UnusualLightCurves(TrackletRule):

    """
    Class variables.
    """
    instCache = {}
    filtCache = {}
    
    def isUnusual(self, obj):
        """
        Return True if the following is true for obj:
         1. The known semi-major axis (a) is greater than 3.0 and the known  
            absolute magnitude (h_v) is < 13.0 (object is brighter than 
            magnitude 13). This will filter out close and small asteroids.
             
         2. The difference in magnitude between detections in a given tracklet
            is > 0.5 mag.
            
         3. There are at least three detections in the tracklet where the
            PSF_Quality > 0.9 and 1.0 < PSF_Extent < 3.0 .

        @param obj: Tracklet instance
        """
        tracklet = obj.alertObj;
        minDetections = Decimal("3")
        magnitudeCut = Decimal("13")
        minSemiMajorAxis = Decimal("3.0")
        magDiff = Decimal("0.5")
        minPSFExtent = Decimal("1.0")
        maxPSFExtent = Decimal("3.0")
        minPSFQuality = Decimal("0.9")


        # Check to see if handle to database we want has already been cached.
        # If it has then retrieve it from the cache, otherwise create a handle
        # and add it to the cache.
        if (UnusualLightCurves.instCache.has_key(obj.getDbName())):
            dbh = UnusualLightCurves.instCache[obj.getDbName()].get_dbh()
        else :
            inst = Instance(obj.getDbName())
            UnusualLightCurves.instCache[obj.getDbName()] = inst
            dbh = UnusualLightCurves.instCache[obj.getDbName()].get_dbh()
        # <-- end if
        
        known = tracklet.fetchKnown(dbh)
        if not known:
            # Object is not known return false.
            return (False)
        # <-- end if
        e = Decimal(str(known.e))
        a = Decimal(Decimal(str(known.q)) / (1 - e))
        h_v = Decimal(str(known.h_v))

        # First, check rule number 1.
        if a < minSemiMajorAxis or h_v > magnitudeCut:
            return (False)
        # <-- end if
        
        # Retrieve the detections that make up the tracklet.
        allMags = []
        detections = tracklet.fetchDetections(dbh)
        
        # Check that the PSF quality and PSF extent of the detections meet the
        # required criteria. If not then remove the detection from the tracklet
        for det in detections :
            attr = det.fetchAttributes(dbh)
            psfQuality = attr.psf_quality
            if (psfQuality <= minPSFQuality):
                detections.remove(det)
                continue
            # <-- end if
            psfExtent = Decimal(str(attr.moments_r1))**2 / (Decimal(str(attr.psf_major)) * Decimal(str(attr.psf_minor)))
            if (psfExtent <= minPSFExtent or psfExtent >= maxPSFExtent) :
                detections.remove(det)
                continue
            # <-- end if
        # <-- end for
        if (len(detections) < minDetections):
            return (False)
        # <-- end if
        
        # Convert filter magnitudes to absolute magnitudes then sort magnitudes 
        # in ascending order and subtract the last magnitude
        # from the first magnitude and if the difference >= magDiff return true.
        if (UnusualLightCurves.filtCache.has_key(obj.getDbName())) :
            filt2v = UnusualLightCurves.filtCache[obj.getDbName()]
        else :
            config = UnusualLightCurves.instCache[obj.getDbName()].getConfig()
            filt2v = config['site']['v2filt']
            UnusualLightCurves.filtCache[obj.getDbName()] = filt2v
        # <-- end if

        mags = []
        for det in detections :
            mags.append(MOPS.Lib.filt2v(det.mag, det.filt, filt2v))
        mags.sort()
        diff = abs(Decimal(str(mags[-1])) - Decimal(str(mags[0])))
        if (diff >= magDiff):
            # Generate specific message to be included in alert
            name = known.known_name.lstrip('(').rstrip(')') # Strip brackets
            msg =  "<p>Min  V mag: %.2f<br/>" % mags[0]
            msg += "Max V mag: %.2f<br/>" % mags[-1]
            msg += "Delta mag: %.2f<br/>" % diff
            msg += "Known As: <a href='http://www.minorplanetcenter.net/db_search/show_object?object_id=%s&commit=Show'>%s</a><br/>" % (name, name)
            msg += "Tracklet (Internal Link): <a href='http://mopshq2.ifa.hawaii.edu/model/%s/search?id=T%d'>T%d</a><br/>" % (obj.dbname, tracklet._id, tracklet._id)
            msg += "Tracklet (External Link): <a href='http://localhost:8080/model/%s/search?id=T%d'>T%d</a></p>" % (obj.dbname, tracklet._id, tracklet._id)
            obj.message = msg
            
            # Generate subject line for alert.
            i = Decimal(str(known.i))
            subj = "%s [h_v=%.1f, a=%.3f, e=%.3f, i=%.3f]" % (known.known_name, h_v, a, e, i)
            obj.subject = subj
            
            # Return true to indicate that an alert is to be generated.
            return(True)
        # <-- end if
        
        # If we get here then all tests have failed.
        return(False)
    # <-- end def
    
    def evaluate(self):
        """
        Return a list of tracklet instances that satisfy the rule.
        """
        #Set the channel that will be used to publish the alert.
        self.channel = CH_ALL
        return([obj for obj in self.newAlerts if self.isUnusual(obj)])
    # <-- end def
# <-- end class

def main(args=sys.argv[1:]):
    rule = UnusualLightCurves(status = 'R')
    alerts = rule.evaluate()
    
    for alert in alerts:
        print(str(alert) + '\n\n')
    # <-- end for
# <-- end main    

if(__name__ == '__main__'):
    sys.exit(main())
# <-- end if