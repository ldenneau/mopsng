#!/usr/bin/env python

import MySQLdb
import sys
import subprocess
import os
import tempfile
import json

from mops import Mops

from ipp import Ipp
from mopsLogging import Logger
from mopsException import MopsException
from stamp import Stamp
from tracklet import Tracklet

def usage():
    documentation = """
NAME
  _TOOLNAME_ - Generate stamps for MOPS

SYNOPSIS
  _TOOLNAME_ [OPTION]... [INPUT FILENAME] [OUTPUT DIRECTORY]

DESCRIPTION
  TODO

  --noout: No /dev/stderr logging output

  --debug: Verbose mode

INPUT FILENAME
  JSON format:

  Text format:

OUTPUT ARCHIVE

AUTHOR
  TODO  

REPORTING BUGS
  TODO  

SEE ALSO
  TODO

"""
    sys.stderr.write(documentation.replace("_TOOLNAME_", 
                                           "_TOOLNAME_TODO_"))

class PackagingFormat:
    """How do we package the stamps?"""

    # Stamps will be put in T<tracklet_id>/<det_id>.fits
    # This format is used when stamps are downloaded with a browser
    BY_TRAIL = 0

    # Stamps will be put in <nn>/<\d>/D<det_id>.fits
    # This format is used when stamps are generated in bulks
    BY_DETECTION = 1 

    @staticmethod
    def fromText(text):
        utext = text.upper()
        if utext == "BY_DETECTION":
            return PackagingFormat.BY_DETECTION
        elif utext == "BY_TRAIL":
            return PackagingFormat.BY_TRAIL
        raise MopsException("Unsupported output format: [%s] (must be BY_DETECTION or BY_TRAIL)" % text)

class Requests:
    def __init__(self, outdir = None, packagingFormat = PackagingFormat.BY_TRAIL):
        self.stage = None
        self.dx = None
        self.dy = None
        self.tracklets = [ ]
        if outdir is None:
            self.outdir = tempfile.mkdtemp(dir = Mops.DSTAMP)
        else:
            self.outdir = outdir
        cleanup(self.outdir)
        os.makedirs("%s/tmp" % self.outdir)
        self.packagingFormat = packagingFormat

    def setStage(self, stage):
        if self.stage is None:
            self.stage = stage
        elif self.stage != stage:
            raise MopsException("Different stage: expected [%s], actual [%s]" % (self.stage, stage))
    def setDx(self, dx):
        if self.dx is None:
            self.dx = int(dx)
        elif self.dx != dx:
            raise MopsException("Different dx: expected [%d], actual [%d]" % (self.dx, dx))
    def setDy(self, dy):
        if self.dy is None:
            self.dy = int(dy)
        elif self.dy != dy:
            raise MopsException("Different dy: expected [%d], actual [%d]" % (self.dy, dy))

    def addTracklet(self, tracklet):
        alreadyInTracklets = None
        index = 0
        while alreadyInTracklets is None and index<len(self.tracklets):
            if self.tracklets[index].name == tracklet.name:
                alreadyInTracklets = self.tracklets[index]
            index += 1
        if alreadyInTracklets is None:
            self.tracklets.append(tracklet)
            return
        # Update the stamps list with the new one
        alreadyInTracklets.stamps.extend(tracklet.stamps)

    @staticmethod
    def fromText(filename, outdir):
        """On each line:
PACKAGING_FORMAT = BY_DETECTION or BY_TRAIL
T<t_id> D<det_id> <exposure> <ra> <dec> <dx> <dy> <stage>
"""
        infile = open(filename)
        lines = infile.read().split("\n")
        requests = None
        for line in lines:
            if line.startswith("#"):
                pass # Ignore comments
            elif line.startswith("PACKAGING_FORMAT"):
                packagingFormat = PackagingFormat.fromText(line.replace(" ", "").split("=")[1])
                if requests is None:
                    requests = Requests(outdir = outdir, packagingFormat = packagingFormat)
            elif line.startswith("T"):
                # Deal with possible multi-spaces:
                # http://stackoverflow.com/questions/8270092/python-remove-all-whitespace-in-a-string
                data = (" ".join(line.split())).split()
                index = 0
                trackletId = data[index] ; index+=1
                detId = data[index] ; index+=1
                exposure = data[index] ; index+=1
                ra = float(data[index]) ; index+=1
                dec = float(data[index]) ; index+=1
                stageId = data[index] ; index+=1
                requests.setStage(stageId)
                dx = int(data[index]) ; index+=1
                requests.setDx(dx)
                dy = int(data[index]) ; index+=1
                requests.setDy(dy)
                stamp = Stamp(detId, exposure, ra, dec, stageId, trackletId)
                tracklet = Tracklet(trackletId)
                tracklet.addStamp(stamp)
                requests.addTracklet(tracklet)
        return requests

    @staticmethod
    def unjsonize(filename, outdir):
        infile = open(filename)
        json_data = infile.read()
        data = json.loads(json_data)
        index = 0
        requests = Requests(outdir)
        requests.packagingFormat = PackagingFormat.fromText(data[index])
        index += 1
        requests.stage = data[index]
        index += 1
        requests.md = None
        if requests.stage.startswith("diff"):
            # The IPP uses different tessellation for the MDs
            # => The stage could be either "diff" or "diff:MDxy"
            fields = requests.stage.split(":")
            if len(fields) == 2:
                requests.stage = field[0]
                requests.md = field[1]
        requests.dx = data[index][0]
        requests.dy = data[index][1]
        index += 1
        for item in data[index].iteritems():
            requests.tracklets.append(Tracklet.unjsonize(item))
        return requests
    def countStamps(self):
        count = 0
        for tracklet in self.tracklets:
            count += len(tracklet.stamps)
        return count
    def counts(self):
        return (len(self.tracklets), self.countStamps())

def cleanup(directory, cleanzip = False):
    command = "/bin/rm -rf %s" % directory
    Logger.debug("Executing: [%s]" % command)
    p = subprocess.Popen(command.split(" "))
    p.wait()
    if cleanzip:
        command = "/bin/rm -f %s.zip" % directory
        p = subprocess.Popen(command.split(" "))
        p.wait()

def package(requests):
    # Zip files in a different way if we work in batches or with a single tracklet
    zipFilename = "%s.zip" % requests.outdir
    if requests.packagingFormat == PackagingFormat.BY_TRAIL:
        if len(requests.tracklets) > 1:
            command = "/usr/bin/zip -r %s %s" % (zipFilename, 
                                                 requests.outdir.replace("/tmp/dstamp/", ""))
            Logger.debug("Zip command: [%s]" % command)
            p = subprocess.Popen(command.split(" "),
                                 cwd = "/tmp/dstamp",
                                 stdout = subprocess.PIPE,
                                 stderr = subprocess.PIPE)
            p.wait()
        else:
            # Only one tracklet: move the tmp directory to the tracklet directory and zip from there
            tid = "%s" % requests.tracklets[0].name
            newoutdir = "%s/%s" % (requests.outdir, tid)
            command = "/bin/mv %s/tmp %s" % (requests.outdir, newoutdir)
            Logger.debug("mv command: [%s]" % command)
            p = subprocess.Popen(command.split(" "))
            p.wait()
            command = "/usr/bin/zip -r %s %s" % (zipFilename, tid)
            Logger.debug("Zip command: [%s]" % command)
            p = subprocess.Popen(command.split(" "),
                                 cwd = requests.outdir,
                                 stdout = subprocess.PIPE,
                                 stderr = subprocess.PIPE)
            p.wait()
    elif requests.packagingFormat == PackagingFormat.BY_DETECTION:
        detdir = "%s/detections" % requests.outdir
        os.makedirs(detdir)
        for c in "0123456789":
            os.makedirs("%s/%s" % (detdir, c))
        for tracklet in requests.tracklets:
            for stamp in tracklet.stamps:
                source = "%s/%s/%d.fits" % (requests.outdir, tracklet.name, stamp.detId)
                lastCharacter = stamp.detId % 10
                target = "%s/%d/D%d.fits" % (detdir, lastCharacter, stamp.detId)
                command = "/bin/mv %s %s" % (source, target)
                Logger.debug("mv command: [%s]" % command)
                p = subprocess.Popen(command.split(" "))
                p.wait()
        detectionsDirShort = detdir.replace(requests.outdir, "").replace("/", "")
        command = "/usr/bin/zip -r %s %s" % (zipFilename, detectionsDirShort)
        Logger.debug("Zip command: [%s], run from directory [%s]" % (command, requests.outdir))
        p = subprocess.Popen(command.split(" "),
                             cwd = requests.outdir,
                             stdout = subprocess.PIPE,
                             stderr = subprocess.PIPE)
        p.wait()
    else:
        raise MopsException("Packaging format [%s] is not implemented" % str(requests.packagingFormat))
    return zipFilename

def processArguments(arguments):
    index = 0
    inputFilename = None
    outdir = None
    packagingFormat = PackagingFormat.BY_TRAIL
    while index < len(arguments):
        if arguments[index] == "-noout" or arguments[index] == "--noout":
            Logger.ACTIVE = False
            index += 1
        elif arguments[index] == "-debug" or arguments[index] == "--debug":
            Logger.LEVEL = Logger.DEBUG
            Logger.setLevel(Logger.LEVEL)
            index += 1
        elif inputFilename is None:
            inputFilename = arguments[index]
            index += 1
        elif outdir is None:
            inputFilename = arguments[index]
            index += 1
        else:
            sys.stderr.write("Unsupported argument(s): [%s]" % (arguments))
            sys.exit(1)
    return (inputFilename, outdir)

if __name__ == "__main__":
    if len(sys.argv) == 1:
        usage()
        sys.exit(1)
    try:
        os.makedirs(Mops.DSTAMP)
    except OSError, e:
        if "File exists" not in str(e):
            raise e
        # else ignore
    (inputFilename, outdir) = processArguments(sys.argv[1:])
    if inputFilename.endswith("json"):
        requests = Requests.unjsonize(inputFilename, outdir = outdir)
    elif inputFilename.endswith("txt"):
        requests = Requests.fromText(inputFilename, outdir = outdir)
    else:
        raise MopsException("Unsupported input format: [%s]" % inputFilename)
    Logger.debug("To be built, out of %d tracklets, %d stamps" % requests.counts())
    Logger.info("Temporary directory is: %s" % requests.outdir)
    for tracklet in requests.tracklets:
        os.makedirs("%s/%s" % (requests.outdir, tracklet.name))
    Ipp.makeStamps(requests)
    zipFilename = package(requests)
    cleanup(requests.outdir)
    Logger.info("Output file is: %s" % zipFilename)

    #print "Now run:"
    #print "/usr/bin/scp mops@ippc18:%s.fits ." % parameters.output
