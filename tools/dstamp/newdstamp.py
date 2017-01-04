#!/usr/bin/env python2.7

#
# This tool generates chip stamps. It is called through
# mops03:/export/mops03.0/MOPS_DEVEL/htdocs.new_stamps/model/getstamps
# It is supposed to run on the IPP cluster
# and is usually installed in:
# /home/panstarrs/mops/production/dstamp/newdstamp.py

# If any problem:
# 1) Get the version that the IPP is using, usually defined 
#    in ~ipp/.tcshrc on the IPP cluster, e.g.:
#        psconfig ipp-20141024 
#    -> means that the version is ipp-20141024
#
# 2) Update the IPP_CONFIG version in this file with that value
IPP_CONFIG = "ipp-20141024"
# 3) Make sure that ~mops/.ipprc on the IPP cluster *hard-codes* 
#    that version too, e.g.:
# PATH            STR     /home/panstarrs/ipp/ippconfig/:/home/panstarrs/ipp/psconfig/ipp-20141024.lin64/share/ippconfig/
#
# That should be enough for the things to work

import sys
sys.path.append("/home/panstarrs/mops/common/lib/python2.7/site-packages")
import MySQLdb
import subprocess
import os
import pyfits
import numpy
import shutil

LOGGER = None

OTAS = [ "XY01", "XY02", "XY03", "XY04", "XY05", "XY06", 
         "XY10", "XY11", "XY12", "XY13", "XY14", "XY15", "XY16", "XY17",
         "XY20", "XY21", "XY22", "XY23", "XY24", "XY25", "XY26", "XY27",
         "XY30", "XY31", "XY32", "XY33", "XY34", "XY35", "XY36", "XY37",
         "XY40", "XY41", "XY42", "XY43", "XY44", "XY45", "XY46", "XY47",
         "XY50", "XY51", "XY52", "XY53", "XY54", "XY55", "XY56", "XY57",
         "XY60", "XY61", "XY62", "XY63", "XY64", "XY65", "XY66", "XY67",
         "XY71", "XY72", "XY73", "XY74", "XY75", "XY76", ]
TEN_ARCSECONDS_IN_DEGREES = 10./3600.
TWO_MINUTES_IN_DEGREES = 2./60.;

def in_range(value, table):
    min_table, max_table = min(table), max(table)
    return min_table <= value <= max_table

class MyException(Exception):
    def __init__(self, message):
        Exception.__init__(self, message)

class Ipp:
    def __init__(self):
        pass

    @staticmethod
    def ppCoord(smf, ra_degrees, dec_degrees):
        ra_offset = 0
        if ra_degrees-160 > 0:
            ra_offset = -100
            pass
        dec_offset = 0
        if dec_degrees-70 > 0:
            dec_offset = -20
            pass
        ra_degrees += ra_offset
        dec_degrees += dec_offset
        hdu_list = pyfits.open(smf)
        for ota in OTAS:
            hdu = hdu_list["%s.psf" % ota]
            ra_psf = hdu.data.field("RA_PSF") + ra_offset
            if in_range(ra_degrees, ra_psf):
                dec_psf = hdu.data.field("DEC_PSF") + dec_offset
                if in_range(dec_degrees, dec_psf):
                    #print "Could be in %s" % ota
                    intersection = []
                    threshold = 5*TEN_ARCSECONDS_IN_DEGREES
                    while len(intersection)<2 and threshold < TWO_MINUTES_IN_DEGREES:
                        ra_indices = numpy.where(numpy.abs(ra_psf-ra_degrees)<=threshold)
                        dec_indices = numpy.where(numpy.abs(dec_psf-dec_degrees)<=threshold)
                        intersection = numpy.intersect1d(ra_indices[0], dec_indices[0])
                        threshold += TEN_ARCSECONDS_IN_DEGREES
                        pass
                    if len(intersection) >= 2:
                        # Apparently the IPP fits with a quadratic model
                        a = numpy.array([ra_psf[intersection]*ra_psf[intersection], ra_psf[intersection], 
                                         dec_psf[intersection]*dec_psf[intersection], dec_psf[intersection]])
                        a = numpy.asmatrix(a).transpose()
                        b = hdu.data.field("X_PSF")[intersection]
                        (x, residuals, rank, s) = numpy.linalg.lstsq(a, b)
                        x_psf = numpy.dot(x, numpy.array([ra_degrees*ra_degrees, ra_degrees, 
                                                          dec_degrees*dec_degrees, dec_degrees]))
                        b = hdu.data.field("Y_PSF")[intersection]
                        (x, residuals, rank, s) = numpy.linalg.lstsq(a, b)
                        y_psf = numpy.dot(x, numpy.array([ra_degrees*ra_degrees, ra_degrees, 
                                                          dec_degrees*dec_degrees, dec_degrees]))
                        hdu_list.close()
                        return (ota,(x_psf, y_psf))
                    else: # Not found in this chip
                        pass
                pass
            pass
        hdu_list.close()
        raise MyException("Can't find any point in point neighborhood")

    @staticmethod
    def ppstamp(input_filename, astrom_filename, skycenter, pixrange, output_filename, parameters):
        LOGGER.write("In ppstamp\n")
        LOGGER.write("input_filename: [%s]\n" % input_filename)
        LOGGER.write("astrom_filename: [%s]\n" % astrom_filename)
        LOGGER.write("skycenter: [%f, %f]\n" % skycenter)
        LOGGER.write("pixrange: [%s]\n" % pixrange)
        LOGGER.write("output filename: [%s]\n" % output_filename)
        pixrange_elts = pixrange.split(" ")
        arguments = ["/home/panstarrs/ipp/psconfig/%s.lin64/bin/ppstamp" % IPP_CONFIG,
                     "-astrom", astrom_filename,
                     "-file", input_filename,
                     "-skycenter", "%s" % skycenter[0], "%s" % skycenter[1],
                     "-pixrange", pixrange_elts[0], pixrange_elts[1],
                     output_filename]
        LOGGER.write("Calling [%s]\n" % " ".join(arguments))
        p = subprocess.Popen( arguments,
                              stdout = subprocess.PIPE,
                              stderr = subprocess.PIPE,
			      env = parameters.env)
        p.wait()
        LOGGER.write("ppstamp status: %d\n" % p.returncode)
	for line in p.stderr:
   	    LOGGER.write("ppstamp stderr: %s" % line)
	for line in p.stdout:
	    LOGGER.write("ppstamp stdout: %s" % line)
        # for line in p.stderr:
        #     print line
        # for line in p.stdout:
        #     print line

class Nebulous:
    nebulous = None
    def __init__(self):
        self.db = MySQLdb.connect(host = "ippdb00",
                                  user = "ipp",
                                  passwd = "ipp",
                                  db = "nebulous")
    @staticmethod
    def get_filename(url):
        if Nebulous.nebulous is None:
            Nebulous.nebulous = Nebulous()
        fields = url.split("/")
        # Remove all characters before "gpc1"
        position = fields.index("gpc1")
        LOGGER.write("%s\n" % url)
        key = "/".join([field for field in fields[position:] if field != ""])
        LOGGER.write("[%s]\n" % key)
        stmt = """SELECT uri
FROM instance
JOIN storage_object USING(so_id)
WHERE ext_id='%s'""" % key
        cursor = Nebulous.nebulous.db.cursor()
        cursor.execute(stmt)
        return cursor.fetchone()[0].replace("file://", "")

class Gpc1:
    gpc1 = None
    def __init__(self):
        self.db = MySQLdb.connect(host = "ippdb05",
                                  user = "ippuser",
                                  passwd = "ippuser",
                                  db = "gpc1")
    @staticmethod
    def get_input_filename(parameters):
        if Gpc1.gpc1 is None:
            Gpc1.gpc1 = Gpc1()
        if parameters.stage == "chip":
            return Gpc1.gpc1.get_chip_input_filename(parameters)
        else:
            raise exceptionToDo
    def get_cam_filename(self, parameters):
        stmt = """SELECT path_base 
FROM camProcessedExp 
  JOIN camRun USING(cam_id) 
  JOIN chipRun USING(chip_id) 
  JOIN rawExp USING(exp_id) 
WHERE exp_name='%s' ORDER BY cam_id DESC""" % parameters.exposure
        cursor = self.db.cursor()
        cursor.execute(stmt)
        candidates = cursor.fetchall()
        if len(candidates) > 1:
            LOGGER.write("!!! More than one smf !!!\n")
        cam_filename = "%s.smf" % candidates[0][0]
        return Nebulous.get_filename(cam_filename)
    def get_chip_input_filename(self, parameters):
        cam_filename = self.get_cam_filename(parameters)
        LOGGER.write("smf = %s\n" % cam_filename)
        ( class_id, pixcenter) = Ipp.ppCoord(cam_filename, parameters.ra, parameters.dec)
        stmt = """SELECT uri
FROM chipProcessedImfile 
  JOIN rawExp USING(exp_id) 
WHERE exp_name='%s' AND class_id='%s'
""" % (parameters.exposure, class_id)
        cursor = self.db.cursor()
        cursor.execute(stmt)
        candidates = cursor.fetchall()
        if len(candidates) > 1:
            LOGGER.write("!!! More than one chip filename !!!\n")
        return (Nebulous.get_filename(candidates[0][0]), 
                cam_filename, 
                (parameters.ra, parameters.dec))
    def get_diff_input_filename(self, parameters):
        pass

class Parameters:
    def __init__(self):
        pass
    @staticmethod
    def initialize(arguments, tracklet):
        if len(arguments) < 4:
            raise MyException("Not enough parameters to create stamp: [%s]" % " ".join(arguments))
        parameters = Parameters()
        parameters.exposure = arguments[0]
        parameters.ra = float(arguments[1])
        parameters.dec = float(arguments[2])
        parameters.stage = arguments[3]
        if len(arguments) >= 4:
            parameters.dx = int(arguments[4])
            parameters.dy = int(arguments[5])
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
    inputFilename = sys.argv[1]
    tracklet = None
    # Load IPP environment
    PSCONFIG = "/home/panstarrs/ipp/psconfig/psconfig.bash"
    process = subprocess.Popen("source %s %s ; env" % (PSCONFIG, IPP_CONFIG), 
                               stdout=subprocess.PIPE, shell=True)
    process.wait()
    output = process.communicate()[0]
    env = dict((line.split("=", 1) for line in output.split('\n') if line))
    #print env
    os.environ.update(env)
    # Processing...
    for line in open(inputFilename):
        if len(line) > 1:
            if tracklet is None:
                tracklet = line[:-1]
                outdir = "/tmp/%s" % tracklet
                cleanup(outdir, cleanzip = True)
                os.mkdir(outdir)
                os.mkdir("%s/tmp" % outdir)
                LOGGER = open("%s/tmp/log" % outdir, "w")
		LOGGER.write("Environment should show IPP: %s\n" % os.environ['PATH'])
		shutil.copy(inputFilename, "%s/tmp" % outdir)
            else:
                arguments = line[:-1].split(" ")
                LOGGER.write("arguments = [%s]\n" % arguments)
                parameters = Parameters.initialize(arguments, tracklet)
                parameters.env = env
                LOGGER.write("Parameters are now initialized\n")
                (input_filename, astrom_filename, skycenter) = Gpc1.get_input_filename(parameters)
                LOGGER.write("[%s] / [%s]\n" % (input_filename, skycenter))
                Ipp.ppstamp(input_filename, astrom_filename,
                            skycenter,
                            "%d %d" % (parameters.dx, parameters.dy),
                            "%s/%s" % (parameters.output, parameters.exposure),
			    parameters)
    LOGGER.close()
    package(tracklet)
    cleanup(outdir)

    #print "Now run:"
    #print "/usr/bin/scp mops@ippc18:%s.fits ." % parameters.output
