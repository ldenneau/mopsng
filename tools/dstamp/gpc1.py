#!/usr/bin/env python

import MySQLdb
import sys
import subprocess
import os
import tempfile

from database import Database
from mopsLogging import Logger
from nebulous import Nebulous

##################################################
class Gpc1(Database):
    gpc1 = None
    debug = False
    def __init__(self):
        Database.__init__(self,
                          host = "ippdb01",
                          user = "ipp", passwd = "ipp",
                          db = "gpc1")

    @staticmethod
    def getRelatedFilenamesChip(exposure):
        # Returns absolute filenames associated to an exposure at Chip stage
        # result["smf"] and 60 result["XYxy"]
        if Gpc1.gpc1 is None:
            Gpc1.gpc1 = Gpc1()
        filenames = { }
        filenames["smf"] = Gpc1.gpc1.getSmfFilename(exposure)
        stmt = """SELECT class_id, uri
FROM chipProcessedImfile 
  JOIN rawExp USING(exp_id) 
WHERE exp_name='%s'
""" % (exposure)
        data = Gpc1.gpc1.select(stmt)
        for datum in data:
            filenames[datum[0]] = Nebulous.getAbsoluteFilename(datum[1])
        return filenames

    @staticmethod
    def get_input_filename(parameters):
        if Gpc1.gpc1 is None:
            Gpc1.gpc1 = Gpc1()
        if parameters.stage == "chip":
            return Gpc1.gpc1.get_chip_input_filename(parameters)
        elif parameters.stage == "diff":
            return Gpc1.gpc1.get_diff_input_filename(parameters)
        else:
            raise exceptionToDo

    def getSmfFilename(self, exposure):
        stmt = """SELECT path_base 
FROM camProcessedExp 
  JOIN camRun USING(cam_id) 
  JOIN chipRun USING(chip_id) 
  JOIN rawExp USING(exp_id) 
WHERE exp_name='%s' ORDER BY cam_id DESC""" % exposure
        camFilename = "%s.smf" % self.select(stmt)[0][0]
        return Nebulous.getAbsoluteFilename(camFilename)

    def get_chip_input_filename(self, parameters):
        cam_filename = self.get_cam_filename(parameters)
        if Gpc1.debug:
            Logger.debug("%s" % cam_filename)
        ( class_id, 
          pixcenter) = Ipp.ppCoord(cam_filename, parameters.ra, parameters.dec,
                                   "%s/tmp/%s.radec" % (parameters.output, parameters.exposure))
        stmt = """SELECT uri
FROM chipProcessedImfile 
  JOIN rawExp USING(exp_id) 
WHERE exp_name='%s' AND class_id='%s'
""" % (parameters.exposure, class_id)
        cursor = self.db.cursor()
        cursor.execute(stmt)
        candidates = cursor.fetchall()
        if len(candidates) > 1:
            Logger.info("!!! More than one chip filename !!!")
        return (Nebulous.getAbsoluteFilename(candidates[0][0]), pixcenter)

    def get_diff_input_filename(self, parameters):
        cam_filename = self.get_cam_filename(parameters)
        Logger.info("%s" % cam_filename)
        stmt = """SELECT path_base
FROM diffSkyfile 
WHERE diff_id = %s""" % parameters.diff_id
        cursor = self.db.cursor()
        cursor.execute(stmt)
        candidates = cursor.fetchall()
        filenames = [ ]
        for row in candidates:
            filenames.append(Nebulous.getAbsoluteFilename("%s.fits" % row[0]))
        return filenames

    def getWarpId(self, exposure):
        stmt = """
SELECT warp_id 
FROM warpRun 
  LEFT JOIN fakeRun USING(fake_id) 
  LEFT JOIN camRun USING(cam_id) 
  LEFT JOIN chipRun USING(chip_id) 
  LEFT JOIN rawExp USING(exp_id)
WHERE exp_name = '%s' 
ORDER BY warp_id DESC""" % exposure
        return self.selectFirst(stmt)[0]

    def getDiffInputFilename(self, exposure, diffId, skycell):
        warpId = self.getWarpId(exposure)
        if diffId == -1:
            diffId = Gpc1.gpc1.getMostRecentDiffId(warpId)
        Logger.debug("%s -> warp_id = %d" % (exposure, warpId))
        stmt = """
SELECT warp1, stack1, warp2, stack2
FROM diffInputSkyfile 
WHERE 
  diff_id = %s AND
  skycell_id = '%s'
""" % (diffId, skycell)
        (warp1, stack1, warp2, stack2) = self.selectFirst(stmt)
        if warp1 == warpId:
            fileExtension = "fits"
        elif warp2 == warpId:
            fileExtension = "inv.fits"
        else:
            raise NotImplemented
        Logger.debug("For %s: extension is %s" % (exposure, fileExtension))
        stmt = """
SELECT path_base
FROM diffSkyfile 
WHERE 
  diff_id = %s AND
  skycell_id = '%s'
""" % (diffId, skycell)
        nebPath = "%s.%s" % (self.selectFirst(stmt)[0], fileExtension)
        return Nebulous.getAbsoluteFilename(nebPath)

    def getMostRecentDiffId(self, warpId):
        stmt = """
SELECT DISTINCT diff_id 
FROM diffInputSkyfile 
WHERE 
  warp1=%d
  OR warp2=%d;
""" % (warpId, warpId)
        return self.selectFirst(stmt)[0]

    @staticmethod
    def getRelatedFilenamesDiff(exposure, diffId, skycell):
        # Returns absolute filenames associated to an exposure at Diff stage
        # result["smf"] and result["skycell.X.Y"]
        if Gpc1.gpc1 is None:
            Gpc1.gpc1 = Gpc1()
        return Gpc1.gpc1.getDiffInputFilename(exposure, diffId, skycell)
