#!/usr/bin/env python
"""
Unit Tets for the auton module

FieldProximity, OrbitProximity, FindTracklets and LinkTracklets are tested. Test
input data was produced by using MOPS simulation data. Output is checked against
the output from the auton command line executables of the same name, compiled on
Fedora Core 6 32-bit systems at IfA.

Note: the underlying assumption is that the command line executables work fine.
Thesting the command line executable is not the intent here.
"""
import os
import unittest

import auton



# Define the main TestSuite.
# autonTests = unittest.TestSuite()


class TestFieldProximity(unittest.TestCase):
    """
    Unit test for auton.fieldproximity

    The output produced by the command line version is different that the one
    produced by the python wrappers. The command line executable outputs the
    line numbers of orbits in the orbits file and fields in the fields file as
    well. No code uses those pieces of information which are consequently
    omitted by the python module.
    """
    fields = []
    orbits = []
    groundTruth = {}
    fieldFileName = 'fieldproximity/54061.fields'
    orbitFileName = 'fieldproximity/54061.coarse'
    referenceFileName = 'fieldproximity/54061.fp'
    
    def setUp(self):
        self.fields = self._parseFieldFile()
        self.orbits = self._parseOrbitFile()
        self.groundTruth = self._parseReferenceFile()
        
        return

    def _parseFieldFile(self):
        fields = []
        f = file(self.fieldFileName)
        for line in f.readlines():
            (fieldID, mjd, ra, dec, radius) = line.strip().split()
            fields.append((int(fieldID),
                           float(mjd),
                           float(ra),
                           float(dec),
                           float(radius)))
        # <-- end for
        f.close()
        return(fields)

    def _parseOrbitFile(self):
        orbits = []
        f = file(self.orbitFileName)
        for line in f.readlines():
            (objName, mjd, ra, dec, mag) = line.strip().split()
            orbits.append((objName,
                           float(mjd),
                           float(ra),
                           float(dec),
                           float(mag)))
        # <-- end for
        f.close()
        return(orbits)

    def _parseReferenceFile(self):
        """
        The reference file (i.e. the output of the command line version) has the
        form
        
            fieldID orbitName

        The mapping produced by the auton module is a dictionary of the form

            {fieldID: [orbitName1, orbitName2...]}

        Parse the reference file and produce the necessary mapping.
        """
        mapping = {}
        f = file(self.referenceFileName)
        for line in f.readlines():
            (fieldID, orbitName) = line.strip().split()
            
            if(not mapping.has_key(fieldID)):
                mapping[fieldID] = [orbitName, ]
            else:
                mapping[fieldID].append(orbitName)
            # <-- end if
        # <-- end for
        return(mapping)
    
    def testFieldProxymity(self):
        mapping = auton.fieldproximity(fields=self.fields,
                                       orbits=self.orbits,
                                       method=1)
        # Compare with ground truth:
        # Same keys.
        trueFields = self.groundTruth.keys()
        trueFields.sort()
        fields = mapping.keys()
        fields.sort()
        self.assertEqual(trueFields, fields)
        # For each key, same values.
        for field in trueFields:
            trueOrbits = self.groundTruth[field]
            trueOrbits.sort()
            orbits = mapping[field]
            orbits.sort()
            self.assertEqual(trueOrbits, orbits)
        # <-- end for
        return


class TestOrbitProximity(unittest.TestCase):
    """
    Unit test for auton.orbitproximity

    The output produced by the command line version is different that the one
    produced by the python wrappers. The command line executable outputs

        queriesLineNo dataLineNo

    i.e. instead of writing down the name of the `query` orbit and the name of
    the matching `data` orbit, it outputs the respective line numbers (0
    indexed) in the input files.
    """
    dataOrbits = []
    queryOrbits = []
    groundTruth = []
    dataFileName = 'orbitproximity/DATA.orbits'
    queryFileName = 'orbitproximity/QUERIES.orbits'
    referenceFileName = 'orbitproximity/MATCH.orbits'
    
    def setUp(self):
        self.dataOrbits = self._parseOrbitFile(self.dataFileName)
        self.queryOrbits = self._parseOrbitFile(self.queryFileName)
        self.groundTruth = self._parseReferenceFile()
        return

    def _parseOrbitFile(self, fileName):
        orbits = []
        f = file(fileName)
        for line in f.readlines():
            (q, e, i, node, argPeri, timePeri,
             hV, epoch, name) = line.strip().split()
            orbits.append((float(q),
                           float(e),
                           float(i),
                           float(node),
                           float(argPeri),
                           float(timePeri),
                           float(hV),
                           float(epoch),
                           name))
        # <-- end for
        f.close()
        return(orbits)

    def _parseReferenceFile(self):
        """
        The reference file (i.e. the output of the command line version) has the
        form
        
            queryOrbitLineNo dataOrbitLineNo

        The mapping produced by the auton module is a dictionary of the form

            {queryOrbitID: [dataOrbitID1, dataOrbitID2...]}

        Parse the reference file and produce the necessary mapping.
        """
        mapping = {}
        f = file(self.referenceFileName)
        for line in f.readlines():
            (queryIdx, dataIdx) = line.strip().split()
            queryOrbitName = self.queryOrbits[int(queryIdx)][-1]
            dataOrbitName = self.dataOrbits[int(dataIdx)][-1]
            
            if(not mapping.has_key(queryOrbitName)):
                mapping[queryOrbitName] = [dataOrbitName, ]
            else:
                mapping[queryOrbitName].append(dataOrbitName)
            # <-- end if
        # <-- end for
        return(mapping)

    
    def testOrbitProxymity(self):
        mapping = auton.orbitproximity(knownOrbits=self.dataOrbits,
                                       queryOrbits=self.queryOrbits,
                                       q_thresh=0.10,
                                       e_thresh=0.050)
        
        # Compare with ground truth:
        # Same keys.
        trueQueryOrbits = self.groundTruth.keys()
        trueQueryOrbits.sort()
        queryOrbits = mapping.keys()
        queryOrbits.sort()
        self.assertEqual(trueQueryOrbits, queryOrbits)
        # For each key, same values.
        for orbit in trueQueryOrbits:
            trueDataOrbits = self.groundTruth[orbit]
            trueDataOrbits.sort()
            dataOrbits = mapping[orbit]
            dataOrbits.sort()
            self.assertEqual(trueDataOrbits, dataOrbits)
        # <-- end for
        return


class TestLinkTracklets(unittest.TestCase):
    """
    Unit test for auton.linktracklets

    The output produced by the command line version is different that the one
    produced by the python wrappers. The command line executable outputs

        trackID trackletID trackletLineNo

    where trackID is a progressive index and has no correspondence with any
    input data. It is also not used by any other piece of software (just like
    the line number). Its only use is to group detections into clusters.

    The module builds clusters of trackletIDs in the form

        [[trackletID1, trackletID2, ...], ...]

    Each sub array is a track.
    """
    detections = []
    groundTruth = []
    detectionFileName = 'linktracklets/1087K.tracklets'
    referenceFileName = 'linktracklets/1087K.sum'
    
    def setUp(self):
        self.detections = self._parseDetectionFile()
        self.groundTruth = self._parseReferenceFile()
        return
    
    def _parseDetectionFile(self):
        dets = []
        f = file(self.detectionFileName)
        for line in f.readlines():
            (detID, mjd, ra, dec, mag, obscode, name) = line.strip().split()
            dets.append((int(detID),
                         float(mjd),
                         float(ra),
                         float(dec),
                         float(mag),
                         int(obscode),
                         name))
        # <-- end for
        f.close()
        return(dets)

    def _parseReferenceFile(self):
        """
        Parse the reference file and produce the necessary mapping. Also take 
        care of extracting the unique tracklet IDs only.
        """
        mapping = []
        f = file(self.referenceFileName)

        currentTrackID = None
        currentTrackletID = None
        track = []
        for line in f.readlines():
            (trackID, trackletID, lineNo) = line.strip().split()
            
            if(trackID == currentTrackID and currentTrackletID != trackletID):
                # Old cluster, new trackletID: append
                track.append(long(trackletID))
                
                # Reset currentTrackletID.
                currentTrackletID = trackletID
            elif(trackID == currentTrackID and currentTrackletID == trackletID):
                # Old cluster, old trackletID: ignore
                continue
            else:
                # New cluster: add the old one to mapping and start a new one.
                if(track):
                    mapping.append(track)
                track = [long(trackletID), ]
                
                # Reset currentTrackletID.
                currentTrackletID = trackletID
                
                # Reset currentTrackID.
                currentTrackID = trackID
            # <-- end if
        # <-- end for

        # Add the last tracklet.
        mapping.append(track)
        return(mapping)

    
    def testLinkTracklets(self):
        mapping = auton.linktracklets(detections=self.detections, 
                                      min_obs=6,
                                      min_sup=3,
                                      vtree_thresh=.0004,
                                      pred_thresh=.0008,
                                      plate_width=.0001)
        
        # Compare with ground truth:
        # Same number of tracklets.
        self.assertEqual(len(self.groundTruth),
                         len(mapping))
        # For each tracklet, same members.
        for i in range(len(self.groundTruth)):
            trueTracklet = self.groundTruth[i]
            trueTracklet.sort()
            tracklet = mapping[i]
            tracklet.sort()
            self.assertEqual(trueTracklet, tracklet)
        # <-- end for
        return


class TestFindTracklets(unittest.TestCase):
    """
    Unit test for auton.findtracklets

    The output produced by the command line version is different that the one
    produced by the python wrappers. The command line executable outputs

        dectionLineNo1, dectionLineNo2, ...

    one line per tracklet. As usual line numbers are 0 indexed.

    The module builds clusters of detectionIDs in the form

        [[detectionID1, detectionID2, ...], ...]

    Each sub array is a tracklet.
    """
    detections = []
    groundTruth = []
    detectionFileName = 'findtracklets/54593.dets'
    referenceFileName = 'findtracklets/54593.pairs'
    
    def setUp(self):
        self.detections = self._parseDetectionFile()
        self.groundTruth = self._parseReferenceFile()
        return

    def _parseDetectionFile(self):
        dets = []
        f = file(self.detectionFileName)
        for line in f.readlines():
            (detID, mjd, ra, dec, mag, obscode, name,
             length, angle) = line.strip().split()
            dets.append((int(detID),
                         float(mjd),
                         float(ra),
                         float(dec),
                         float(mag),
                         int(obscode),
                         name,
                         float(length),
                         float(angle)))
        # <-- end for
        f.close()
        return(dets)

    def _parseReferenceFile(self):
        """
        Parse the reference file and produce the necessary mapping.
        """
        mapping = []
        f = file(self.referenceFileName)

        for line in f.readlines():
            tracklet = line.strip().split()
            mapping.append([long(self.detections[int(i)][0]) for i in tracklet])
        # <-- end for
        return(mapping)

    
    def testFindTracklets(self):
        mapping = auton.findtracklets(detections=self.detections, 
                                      maxv=2.0,
                                      minobs=2,
                                      maxt=0.050,
                                      etime=30.0)
        
        # Compare with ground truth:
        # Same number of tracklets.
        self.assertEqual(len(self.groundTruth),
                         len(mapping))
        # For each tracklet, same members.
        # mapping and ground truth might be sorted differently.
        self.groundTruth.sort()
        mapping.sort()
        
        for i in range(len(self.groundTruth)):
            trueTracklet = self.groundTruth[i]
            trueTracklet.sort()
            tracklet = mapping[i]
            tracklet.sort()
            self.assertEqual(trueTracklet, tracklet)
        # <-- end for
        return


class TestDetectionProximity(unittest.TestCase):
    """
    Unit test for auton.detectionproximity

    The output produced by the command line version is different that the one
    produced by the python wrappers. The command line executable outputs

        queryDectionLineNo1 dataDectionLineNo1
        queryDectionLineNo1 dataDectionLineNo2

    one line per query-data detection pair. As usual line numbers are 0 indexed.

    The module builds clusters of detectionIDs in the form

        {queryDetectionID: [dataDetectionID1, dataDetectionID2, ...], ...}

    Each sub array is made of data detection ID corresponding to the one query
    detection ID (the corresponding dictionary key).
    """
    detections = []
    groundTruth = []
    dataFileName = 'detectionproximity/54593.dets'
    queryFileName = 'detectionproximity/54593.query'
    referenceFileName = 'detectionproximity/54593.truth'
    
    def setUp(self):
        self.dataDetections = self._parseDetectionFile(self.dataFileName)
        self.queryDetections = self._parseDetectionFile(self.queryFileName)
        self.groundTruth = self._parseReferenceFile()
        return

    def _parseDetectionFile(self, fileName):
        dets = []
        f = file(fileName)
        for line in f.readlines():
            (detID, mjd, ra, dec, mag, obscode, name,
             length, angle) = line.strip().split()
            dets.append((int(detID),
                         float(mjd),
                         float(ra),
                         float(dec),
                         float(mag),
                         int(obscode),
                         name,
                         float(length),
                         float(angle)))
        # <-- end for
        f.close()
        return(dets)

    def _parseReferenceFile(self):
        """
        Parse the reference file and produce the necessary mapping. It assumes
        that the data and query files have already been parsed.
        """
        mapping = {}
        f = file(self.referenceFileName)

        for line in f.readlines():
            lineNumbers = line.strip().split()
            queryLineNumber = lineNumbers[0]
            dataLineNumbers = lineNumbers[1:]

            key = self.queryDetections[int(queryLineNumber)][0]
            val = [self.dataDetections[int(i)][0] for i in dataLineNumbers]
            if(mapping.has_key(key)):
                mapping[key] += val
            else:
                mapping[key] = val
            # <-- end if
        # <-- end for
        return(mapping)

    
    def testDetectionProximity(self):
        mapping = auton.detectionproximity(dataDets=self.dataDetections,
                                           queryDets=self.queryDetections)
        
        # Compare with ground truth:
        # Same number of tracklets.
        self.assertEqual(len(self.groundTruth.keys()),
                         len(mapping.keys()))
        # For each query detection, same data detections.
        for key in self.groundTruth.keys():
            trueDataDetections = self.groundTruth[key]
            trueDataDetections.sort()
            dataDetections = mapping[key]
            dataDetections.sort()
            self.assertEqual(trueDataDetections, dataDetections)
        # <-- end for
        return




if(__name__ == '__main__'):
#    import pdb
#    pdb.set_trace()
    unittest.main()
    
    

