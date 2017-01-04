#!/usr/bin/env python

import os
from database import Database
from mopsLogging import Logger

##################################################
class Nebulous(Database):
    nebulous = None
    checkExistence = False
    debug = False

    def __init__(self):
        Database.__init__(self,
                          host = "ippdb00",
                          user = "ipp", passwd = "ipp",
                          db = "nebulous")
    @staticmethod
    def getAbsoluteFilename(url):
        if Nebulous.nebulous is None:
            Nebulous.nebulous = Nebulous()
        fields = url.split("/")
        # Remove all characters before "gpc1"
        position = fields.index("gpc1")
        if Nebulous.debug:
            Logger.debug("URL without gpc1 part: %s" % url)
        key = "/".join([field for field in fields[position:] if field != ""])
        if Nebulous.debug:
            Logger.debug("Nebulous key: [%s]" % key)
        stmt = """SELECT uri
FROM instance
JOIN storage_object USING(so_id)
WHERE ext_id='%s'""" % key
        uriFilenames = Nebulous.nebulous.select(stmt)
        if Nebulous.checkExistence:
            for uriFilename in uriFilenames:
                absoluteFilename = uriFilename[0].replace("file://", "")
                if os.access(absoluteFilename, os.R_OK):
                    return absoluteFilename
        else:
            return uriFilenames[0][0].replace("file://", "")
