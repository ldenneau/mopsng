#!/usr/bin/env python

import sys
import subprocess

def usage(script_name):
    pass

def process_arguments(arguments):
    bypass = False
    for i in range(len(arguments)):
        if arguments[i] == "-bypass":
            bypass = True
            del arguments[i]
            break
        pass
    folder = arguments[1]
    exposures = arguments[2:]
    if not bypass:
        if not folder.startswith("20"):
            sys.stderr.write("Folder does not look like a date: [%s]\n" 
                             % folder)
            usage(arguments[0])
            sys.exit(1)
            pass
        if len(exposures) % 4 != 0:
            sys.stderr.write("Exposures count not a multiple of 4\n")
            usage(arguments[0])
            sys.exit(1)
            pass
        for exposure in exposures:
            if not exposure.startswith("o"):
                sys.stderr.write("Exposure does not like PS1: [%s]\n" 
                                 % exposure)
                usage(arguments[0])
                sys.exit(1)
                pass
            pass
        pass
    return (folder, exposures)

if __name__ == "__main__":
    arguments = sys.argv
    (folder, exposures) = process_arguments(arguments)
    where = ( "dbname='psmops_ps1_mdrm152' AND fpa_id IN (%s)"
              % ",".join(["'%s'" % exposure for exposure in exposures]) )
    command = ["/usr/bin/mysqldump",
               "--single-transaction",
               "-hnmops00",
               "-udump",
               "-pdump",
               "--where",
               "\"%s\"" % where,
               "export",
               "mpc_sub" ]
    print 
    print "Check and then run this:"
    print " ".join(command), ">", "mops_detections.sql"
    print "/usr/bin/scp mops_detections.sql ipp@ipp0222:/export/ipp022.0/ps1-outreach/%s" % folder
    pass

