#!/usr/bin/env python
"""
Fake differential corrector.

Usage
    fake_diffcor.py [options] IOD_file Tracks_file Orbits_file

    Input
        IOD_file
        Tracks_file

    Output
        Orbits_file

    Options
        --debug ignored
        --verbose ignored
        --fallback_iod XXX ignored
        --covariance ignored
        --report_all ignored
        --threshold_arcseconds XXX ignored
        --instance XXX ignored

Description
    fake_diffcor.py reads the two input files (IOD_file and Tracks_file),
    associates IODs to tracks based on their object_name field and writes to the
    output file (Orbits_file) the same input IODs with the convergence code set
    to D9 instead of I.

    A simple check is performed to make sure that the orbits are clean, meaning
    that detections associated to different object do not end up in an orbit. In
    those cases (mixed orbits) no orbit is written to the output file.
"""
import sys


def diffcor(iod, tracks, out):
    """
    iod: IOD file name
    tracks: fracks file name
    out: output orbits file name
    """
    if(iod == '-'):
        iods = sys.stdin.readlines()
    else:
        iods = file(iod).readlines()
    if(tracks == '-'):
        tracks = sys.stdin.readlines()
    else:
        tracks = file(tracks).readlines()
    orbits = file(out, 'w')

    # Cluster the tracks together by object.
    objects = {}                                           # object name: tracks
    for track in tracks:
        objName = track.split()[0]
        if(objects.has_key(objName)):
            objects[objName].append(track)
        else:
            objects[objName] = [track, ]
    # <-- end for

    # Now look at the IODs and spit out only those for which we have tracks.
    for iod in iods:
        tokens = iod.strip().split()
        objName = tokens[1]
        if(objName not in objects.keys()):
            continue

        # The convergence code is the last token.
        tokens[-1] = 'D9'
        orbits.write(' '.join(tokens) + '\n')
    # <-- end for
    orbits.close()
    return


if(__name__ == '__main__'):
    import optparse

    usage = "usage: %prog [options] iod tracks out"
    parser = optparse.OptionParser(usage)
    parser.add_option("-d", "--debug", action="store_true", dest="debug",
                      help="debugging on (ignored)")
    parser.add_option("-v", "--verbose", action="store_true", dest="verbose",
                      help="verbose on (ignored)")
    parser.add_option("-c", "--covariance", action="store_true", dest="covariance",
                      help="instance (ignored)")
    parser.add_option("-f", "--fallback_iod", dest="fallback_iod",
                      action="store", type="int",
                      help="fallback IOD (ignored)")
    parser.add_option("-r", "--report_all", dest="report_all",
                      help="report all (ignored)")
    parser.add_option("-t", "--threshold_arcseconds", dest="threshold_arcseconds",
                      action="store", type="float",
                      help="threshold (arcseconds) (ignored)")
    parser.add_option("-i", "--instance", dest="instance",
                      help="instance (ignored)")

    (options, args) = parser.parse_args()
    if(len(args) != 3):
        parser.error('Wrong number of arguments.')

    # Call the main routine.
    diffcor(iod=args[0], tracks=args[1], out=args[2])
# <-- end if



















