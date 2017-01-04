#!/usr/bin/env python

import logging

LOGGER = None

##################################################
class Constants:
    # formatter = logging.Formatter('%(asctime)s | %(levelname)7s | %(module)20s | %(funcName)30s | %(message)s', 
    #                               '%Y-%m-%dT%H:%M:%S')
    formatter = logging.Formatter('%(asctime)s | %(levelname)7s | %(message)s', 
                                  '%Y-%m-%dT%H:%M:%S')

    LOGGER = logging.getLogger()
    logging_output = logging.FileHandler('/dev/stderr')
    logging_output.setFormatter(Constants.formatter)
    LOGGER.addHandler(logging_output)
    LOGGER.setLevel(logging.DEBUG)


class Parameters:
    def __init__(self):
        pass
    @staticmethod
    def initialize(arguments, tracklet):
        if len(arguments) < 4:
            raise MopsException("Not enough parameters to create stamp: [%s]" % " ".join(arguments))
        parameters = Parameters()
        index = 0
        parameters.exposure = arguments[index]
        index += 1
        parameters.ra = float(arguments[index])
        index += 1
        parameters.dec = float(arguments[index])
        index += 1
        parameters.stage = arguments[index]
        index += 1
        if parameters.stage == "diff":
            parameters.diff_id = arguments[index]
            index += 1
        if len(arguments) >= index:
            parameters.dx = int(arguments[index])
            parameters.dy = int(arguments[index+1])
        else:
            parameters.dx = 100
            parameters.dx = 100
        parameters.output = "/tmp/%s" % tracklet
        return parameters

def cleanup(directory, cleanzip = False):
    command = "/bin/rm -rf %s" % outdir
    p = subprocess.Popen(command.split(" "))
    p.wait()
    if cleanzip:
        command = "/bin/rm -f %s.zip" % outdir
        p = subprocess.Popen(command.split(" "))
        p.wait()

def package(tracklet):
    command = "/usr/bin/zip -r /tmp/%s.zip %s" % (tracklet, tracklet)
    p = subprocess.Popen(command.split(" "),
                         cwd = "/tmp")
    p.wait()

if __name__ == "__main__":
    LOGGER = logging.getLogger()
    logging_output = logging.FileHandler('/dev/stderr')
    logging_output.setFormatter(Constants.formatter)
    LOGGER.addHandler(logging_output)
    LOGGER.setLevel(logging.DEBUG)

    inputFilename = sys.argv[1]
    tracklet = None
    for line in open(inputFilename):
        if len(line) > 1:
            if tracklet is None:
                tracklet = line[:-1]
                outdir = "/tmp/%s" % tracklet
                cleanup(outdir, cleanzip = True)
                os.mkdir(outdir)
                os.mkdir("%s/tmp" % outdir)
                logging_output = logging.FileHandler("%s/tmp/log" % outdir)
                logging_output.setFormatter(Constants.formatter)
                LOGGER.addHandler(logging_output)
            else:
                arguments = line[:-1].split(" ")
                parameters = Parameters.initialize(arguments, tracklet)
                if parameters.stage == "chip":
                    (input_filename, pixcenter) = Gpc1.get_input_filename(parameters)
                    Ipp.ppstamp(input_filename, pixcenter,
                                "%d %d" % (parameters.dx, parameters.dy),
                                "%s/%s" % (parameters.output, parameters.exposure))
                elif parameters.stage == "diff":
                    input_filenames = Gpc1.get_input_filename(parameters)
                    pixcenter = "%g %g" % (parameters.ra, parameters.dec)
                    Ipp.ppstamp(input_filenames, 
                                pixcenter,
                                "%d %d" % (parameters.dx, parameters.dy),
                                "%s/%s" % (parameters.output, parameters.exposure),
                                pixels_coordinates = False)
                else:
                    fail
    package(tracklet)
    cleanup(outdir)

    #print "Now run:"
    #print "/usr/bin/scp mops@ippc18:%s.fits ." % parameters.output
