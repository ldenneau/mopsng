#!/usr/bin/env python

import MySQLdb
import sys

def usage(program_name):
    return """
%s [exposure name] [...]

Check if the smf of <exposure name> have been published to the datastore

-h, -help, --help:
  Displays this help and exits
"""

def process_arguments(arguments):
    if len(arguments)<2:
        sys.stderr.write(usage(arguments[0]))
        sys.exit(1)
    index = 1
    if arguments[index] == "-h" or arguments[index] == "-help" or arguments[index] == "--help":
        sys.stderr.write(usage(arguments[0]))
        sys.exit(0)
    return arguments[index:]

if __name__ == "__main__":
    exposures = process_arguments(sys.argv)
    gpc1 = MySQLdb.connect('ippdb01', 'ipp', 'ipp', 'gpc1')
    cursor = gpc1.cursor()
    for exposure in exposures:
        statement = "SELECT COUNT(distinct(distRun.dist_id)) FROM distRun JOIN distComponent USING(dist_id) JOIN camRun ON camRun.cam_id=distRun.stage_id JOIN chipRun USING(chip_id) JOIN rawExp USING(exp_id) WHERE exp_name = '%s' AND distRun.stage='camera'" % exposure
        cursor.execute(statement)
        count=cursor.fetchall()[0][0]
        if count != 0:
            print "%s: OK (%d published)" % (exposure, count)
        else:
            print "%s: FAIL (%d published)" % (exposure, count)

