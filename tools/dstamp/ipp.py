#!/usr/bin/env python

import sys
import subprocess
import os

from mops import Mops
from mopsLogging import Logger
from gpc1 import Gpc1

##################################################
class Ipp:
    def __init__(self):
        pass

    # Should be the only method to be publicly called
    @staticmethod
    def makeStamps(requests):
        if requests.stage == "chip":
            Ipp.makeChipStamps(requests)
        elif requests.stage == "diff":
            Ipp.makeDiffStamps(requests)
        else:
            raise UnknownStageName(requests.stage)

    # ppstamp-related methods
    @staticmethod
    def ppstamp(inputFilename, x, y, dx, dy, outputFilename, distribute = Mops.DISTRIBUTE):
        # Condorize here. But for the moment call with subprocess
        arguments = Ipp.build_ppstamp_command(inputFilename, x, y, dx, dy, outputFilename)
        if distribute:
            raise NotImplemented
        else:
            Logger.debug("Calling [%s]" % " ".join(arguments))
            p = subprocess.Popen( arguments,
                                  stdout = subprocess.PIPE,
                                  stderr = subprocess.PIPE)
            p.wait()
            Logger.debug("ppstamp process exit status: %d" % p.returncode)

    @staticmethod
    def build_ppstamp_command(inputFilename, x, y, dx, dy, outputFilename):
        arguments = ["ppstamp",
                     "-file", inputFilename,
                     "-pixcenter", "%g" % x, "%g" % y,
                     "-pixrange", "%d" % dx, "%d" % dy,
                     outputFilename]
        return arguments

    # CHIP PROCESSING
    @staticmethod
    def makeChipStamps(requests):
        # We reorganize by exposure name
        requestsByExposures = { }
        for tracklet in requests.tracklets:
            for stamp in tracklet.stamps:
                try:
                    requestsByExposures[stamp.exposure].append(stamp)
                except KeyError:
                    requestsByExposures[stamp.exposure] = [ ]
                    requestsByExposures[stamp.exposure].append(stamp)
        for exposure, stamps in requestsByExposures.iteritems():
            Logger.debug("%s: %s" % (exposure, [str(x) for x in stamps]))
            Ipp.makeChipExposureStamps(exposure, stamps, requests)

    @staticmethod
    def makeChipExposureStamps(exposure, stamps, context):
        filenames = Gpc1.getRelatedFilenamesChip(exposure)
        Logger.debug("SMF filename: [%s]" % filenames["smf"])
        # Get the ota and pixel center for each stamp
        Ipp.dvoImagesAtCoordsChip(stamps, filenames["smf"], context.outdir, exposure)
        for stamp in stamps:
            Ipp.ppstamp(filenames[stamp.ota],
                        stamp.x, stamp.y,
                        context.dx, context.dy,
                        "%s/%s/%s" % (context.outdir, stamp.trackletId, stamp.detId))

    @staticmethod
    def dvoImagesAtCoordsChip(stamps, smfFilename, outdir = None, exposure = None):
        if outdir is None:
            coordinatesFilename = "%s/coordinates.dvo" % Mops.DSTAMP
        else:
            coordinatesFilename = "%s/tmp/%s.dvo" % (outdir, exposure)
        coordinatesFile = open(coordinatesFilename, "w")
        count = 0
        for stamp in stamps:
            coordinatesFile.write("%d %g %g\n" % (count, stamp.ra, stamp.dec))
            count += 1
        coordinatesFile.close()
        command = [ "dvoImagesAtCoords",
                    "-astrom", smfFilename,
                    "-listchipcoords",
                    "-coords", coordinatesFilename] 
        Logger.debug("Calling [%s]" % " ".join(command))
        p = subprocess.Popen( command,
                              stdout = subprocess.PIPE,
                              stderr = subprocess.PIPE)
        status = p.wait()
        for line in p.stdout:
            fields = " ".join(line[:-1].split()).split(" ") # The (join, first split) is to remove multi-spacing
            stamps[int(fields[0])].ota = fields[3]
            stamps[int(fields[0])].x = float(fields[4])
            stamps[int(fields[0])].y = float(fields[5])

    # DIFF PROCESSING
    @staticmethod
    def makeDiffStamps(requests):
        # Get the skycells in which each detection falls
        all_stamps = [ ]
        for tracklet in requests.tracklets:
            all_stamps.extend(tracklet.stamps)
        Ipp.dvoImagesAtCoordsDiff(all_stamps, requests.md, requests.outdir, "diff_coordinates")
        for stamp in all_stamps:
            Ipp.ppstamp(stamp.inputFilename, 
                        stamp.x, stamp.y, 
                        requests.dx, requests.dy, 
                        "%s/%s/%s" % (requests.outdir, stamp.trackletId, stamp.detId))

    @staticmethod
    def dvoImagesAtCoordsDiff(stamps, md, outdir = None, exposure = None):
        if md is None:
            catdir = "/local/ipp/gpc1/tess/RINGS.V3"
        else:
            catdir = "/local/ipp/gpc1/tess/%s.V3" % md
        if outdir is None:
            coordinatesFilename = "/tmp/coordinates.dvo"
        else:
            coordinatesFilename = "%s/tmp/%s.dvo" % (outdir, exposure)
        coordinatesFile = open(coordinatesFilename, "w")
        count = 0
        Logger.debug("count ra dec stageId exposure")
        for stamp in stamps:
            Logger.debug("%d %g %g %d %s" % (count, stamp.ra, stamp.dec, stamp.stageId, stamp.exposure))
            coordinatesFile.write("%d %g %g\n" % (count, stamp.ra, stamp.dec))
            count += 1
        coordinatesFile.close()
        command = [ "dvoImagesAtCoords",
                    "-D", "CATDIR", catdir,
                    "-listchipcoords",
                    "-coords", coordinatesFilename] 
        Logger.debug("Calling [%s]" % " ".join(command))
        p = subprocess.Popen( command,
                              stdout = subprocess.PIPE,
                              stderr = subprocess.PIPE)
        status = p.wait()
        for line in p.stdout:
            fields = " ".join(line[:-1].split()).split(" ") # The (join, first split) is to remove multi-spacing
            stamps[int(fields[0])].ota = fields[3] # ota is the skycell id in this case
            stamps[int(fields[0])].x = float(fields[4])
            stamps[int(fields[0])].y = float(fields[5])
        for stamp in stamps:
            stamp.inputFilename = Gpc1.getRelatedFilenamesDiff(stamp.exposure, stamp.stageId, stamp.ota)
