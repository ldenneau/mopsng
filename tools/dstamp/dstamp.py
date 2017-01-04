#!/usr/bin/env python

import MySQLdb
import sys
import subprocess

class Ipp:
    def __init__(self):
        pass
    @staticmethod
    def ppCoord(smf, ra, dec):
        outfilename = "/tmp/blah.radec"
        outfile = open(outfilename, "w")
        outfile.write("%f %f\n" % (ra, dec))
        outfile.close()
        p = subprocess.Popen( ["ppCoord",
                               "-astrom", smf,
                               "-radec", outfilename],
                              stdout = subprocess.PIPE,
                              stderr = subprocess.PIPE)
        status = p.wait()
        for line in p.stdout:
            if "-->" in line:
                chip_line = line[:-1]
                print "[%s]" % chip_line
        data = chip_line.split(' ')
        chip = data[-1]
        pixcenter = " ".join(data[-3:-1])
        return (chip, pixcenter)
    @staticmethod
    def ppstamp(input_filename, pixcenter, pixrange, output_filename):
        pixcenter_elts = pixcenter.split(" ")
        pixrange_elts = pixrange.split(" ")
        arguments = ["ppstamp",
                     "-file", input_filename,
                     "-pixcenter", pixcenter_elts[0], pixcenter_elts[1],
                     "-pixrange", pixrange_elts[0], pixrange_elts[1],
                     output_filename]
        print "Calling [%s]" % " ".join(arguments)
        p = subprocess.Popen( arguments,
                              stdout = subprocess.PIPE,
                              stderr = subprocess.PIPE)
        p.wait()
        print "ppstamp status: ", p.returncode
        # for line in p.stderr:
        #     print line
        # for line in p.stdout:
        #     print line

class Nebulous:
    nebulous = None
    def __init__(self):
        self.db = MySQLdb.connect(host = "ippdb02",
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
        print url
        key = "/".join([field for field in fields[position:] if field != ""])
        print "[%s]" % key
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
        self.db = MySQLdb.connect(host = "ippdb01",
                                  user = "ipp",
                                  passwd = "ipp",
                                  db = "gpc1")
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
            print "!!! More than one smf !!!"
        cam_filename = "%s.smf" % candidates[0][0]
        return Nebulous.get_filename(cam_filename)
    def get_chip_input_filename(self, parameters):
        cam_filename = self.get_cam_filename(parameters)
        print cam_filename
        ( class_id, 
          pixcenter) = Ipp.ppCoord(cam_filename, parameters.ra, parameters.dec)
        stmt = """SELECT uri
FROM chipProcessedImfile 
  JOIN rawExp USING(exp_id) 
WHERE exp_name='%s' AND class_id='%s'
""" % (parameters.exposure, class_id)
        cursor = self.db.cursor()
        cursor.execute(stmt)
        candidates = cursor.fetchall()
        if len(candidates) > 1:
            print "!!! More than one chip filename !!!"
        return (Nebulous.get_filename(candidates[0][0]), pixcenter)
    def get_diff_input_filename(self, parameters):
        pass

class Parameters:
    def __init__(self):
        pass
    @staticmethod
    def initialize(arguments):
        parameters = Parameters()
        parameters.exposure = arguments[0]
        parameters.ra = float(arguments[1])
        parameters.dec = float(arguments[2])
        parameters.stage = arguments[3]
        parameters.output = "/tmp/blah"
        if len(arguments) > 4:
            parameters.stage_id = arguments[4]
        else:
            parameters.stage_id = None
        return parameters

if __name__ == "__main__":
    arguments = sys.argv[1:]
    parameters = Parameters.initialize(arguments)
    (input_filename, pixcenter) = Gpc1.get_input_filename(parameters)
    Ipp.ppstamp(input_filename, pixcenter, 
                "1000 1000", 
                parameters.output)
    #print "Now run:"
    #print "/usr/bin/scp mops@ippc18:%s.fits ." % parameters.output
