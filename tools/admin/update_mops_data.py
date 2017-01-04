#!/usr/bin/python
# $Id$
'''
Created on August 27th, 2012

@author: Denver Green
'''

USAGE = '''USAGE:

update_mops_data [options]

Downloads various data files used by MOPS that change over time, and installs them into the MOPS environment.
'''
import gzip
import logging
import optparse
import os.path
import re
import shutil
import smtplib
import subprocess as sub
import sys
import tarfile
import tempfile 
import urllib2

import MOPS.Utilities as Utilities

from email.MIMEMultipart import MIMEMultipart
from email.MIMEText import MIMEText
from email.MIMEBase import MIMEBase

class MOPSUpdater(object):
    
    #--------------------------------------------------------------------------   
    # Class variables.
    #--------------------------------------------------------------------------                  
    FAIL = 1
    SUCCESS = 0
    LOG_FILE = "/export/mops02.0/tmp/mops_update.log" 

    #------------------------------------------------------------------------------
    # Instance Methods
    #------------------------------------------------------------------------------
    def __init__(self, workDir, email, logger=None):
        if (logger == None):
            self._logger = Utilities.getLogger(logFile=MOPSUpdater.LOG_FILE, mode="w")
            self._logger.setLevel(logging.INFO)
        else:
            self._logger = logger
        # <-- end if
        self._workDir = workDir
        self._email = email
    # <-- end __init__

    def get_obscode(self, dest_list):
        """
        Retrieves the observatory codes list from the minor planet center website
        and installs it into MOPS
        
        @param dest_list: List of directories to copy the obscode file to.
        
        @return result: Result of function. 
                        0        - success
                        non-zero - failure
        """
        OBSCODE_SRC  = "http://www.minorplanetcenter.net/iau/lists/ObsCodes.html"
        OBSCODE_NAME = "OBSCODE.dat"
    
        out_fh = None
        in_fh = None  
    
        # get observatory codes from http://www.minorplanetcenter.net/iau/lists/ObsCodes.html
        obscode_file = self.get_file(OBSCODE_SRC)
        if (obscode_file == None): return MOPSUpdater.FAIL
            
        try:
            # remove the first two and last line from obscode.
            in_fh = open(obscode_file, "r")
            lines = in_fh.readlines()
            out_fh = open(os.path.join(self._workDir, OBSCODE_NAME), "w")
            out_fh.writelines(lines[2:-1])
        except Exception, e:
            self._logger.error("UPDATE_MOPS_DATA: %s" % (str(e)))
            return MOPSUpdater.FAIL
        finally:
            if (in_fh): in_fh.close()
            if (out_fh): out_fh.close()
        # <-- end try
        
        try:
            # Install OBSCODE.dat in mops
            for d in dest_list:
                # Verify destination directory
                if (not os.path.exists(os.path.dirname(d))):
                    self._logger.error("UPDATE_MOPS_DATA: The destination directory %s does not exist." % (os.path.dirname(d)))
                    continue
                # <-- end if    
                shutil.copyfile(os.path.join(self._workDir, OBSCODE_NAME),d)
            # <-- end for   
        except Exception, e:
            self._logger.error("UPDATE_MOPS_DATA: %s" % (str(e)))    
            return MOPSUpdater.FAIL
        else:
            self._logger.info("UPDATE_MOPS_DATA: obscode.dat file update complete.")   
            self._logger.debug("UPDATE_MOPS_DATA: obscode.dat file contents")
            self._logger.debug("%s" % (lines[2:-1]))
            return MOPSUpdater.SUCCESS
        # <-- end try
    # <-- end def
            
    def get_etut(self, dest_list):
        """
        Retrieves a file which reports the difference between Terrestrial Time (TT) 
        and Universal Time (UT1). It is a measure of the difference between a time 
        scale based on the rotation of the Earth (UT1) and an idealised uniform 
        timescale at the surface of the Earth (TT). TT is realised in practice by 
        TAI, International Atomic Time, where TT = TAI + 32.184 seconds. In order 
        to predict the circumstances of an event on the surface of the Earth such 
        as a solar eclipse, a prediction of Delta T must be made for that instant 
        of TT. 
        
        @param dest_list: List of directories to copy the et-ut.dat file to.
        """
        
        ETUT_SRC = "/export/mops02.0/MOPS_DATA/ET-UT/ET-UT.dat"
    
        out_fh = None
        in_fh = None  
        
        try:
            if (not os.path.exists(ETUT_SRC)):
                self._logger.error("UPDATE_MOPS_DATA: ET-UT.dat file does not exist in the /export/mops02.0/MOPS_DATA/ET-UT directory.")
                self._logger.error("UPDATE_MOPS_DATA: Please execute the ET_minus_UT script which will generate an updated ET-UT.dat file.")
                return MOPSUpdater.FAIL
            # <-- end if
            
            # Install et-ut.dat in mops
            for d in dest_list :
                # Verify destination directory
                if (not os.path.exists(os.path.dirname(d))):
                    self._logger.error("UPDATE_MOPS_DATA: The destination directory %s does not exist." % (os.path.dirname(d)))
                    continue
                # <-- end if    
                shutil.copyfile(ETUT_SRC,d)
            # <-- end for            
        except Exception, e:
            self._logger.error("UPDATE_MOPS_DATA: %s" % (str(e)))
            return MOPSUpdater.FAIL
        else:
            self._logger.info("UPDATE_MOPS_DATA: ET-UT.dat file update complete.")   
            return MOPSUpdater.SUCCESS        
        # <-- end try           
    # <-- end def 
           
    def get_eopc0462(self, dest_list):
        """
        Retrieves the earth orientation parameter file which describes the 
        irregularities of the Earth's rotation with respect to a non-rotating 
        refrence frame from the l' Observatoire de Paris web site. The file 
        retrieved is the C04 combined series file.
        
        @param dest_list: List of directories to copy the eopc04.62-now file to.
        """
        EOPC462_SRC = "ftp://hpiers.obspm.fr/eop-pc/eop/eopc04/eopc04.62-now"
        
        
        # get et-ut data file from http://maia.usno.navy.mil/ser7/deltat.data
        eop_file = self.get_file(EOPC462_SRC)
        
        try:
            # Install eop file in mops
            for d in dest_list :
                # Verify destination directory
                if (not os.path.exists(os.path.dirname(d))):
                    self._logger.error("UPDATE_MOPS_DATA: The destination directory %s does not exist." % (os.path.dirname(d)))
                    continue
                # <-- end if    
                shutil.copyfile(os.path.join(self._workDir, os.path.basename(eop_file)),d)
            # <-- end for            
        except Exception, e:
            self._logger.error("UPDATE_MOPS_DATA: %s" % (str(e)))
            return MOPSUpdater.FAIL
        else:
            self._logger.info("UPDATE_MOPS_DATA: eopc04.62-now file update complete.")
            return MOPSUpdater.SUCCESS
        # <-- end try          
    # <-- end def 
           
    def get_eop(self, dest_list):
        """
        Retrieves the earth orientation parameter file which describes the 
        irregularities of the Earth's rotation with respect to a non-rotating 
        refrence frame from the l' Observatoire de Paris web site. The file 
        retrieved is the C04 combined series file.
        
        @param dest_list: List of directories to copy the jpl eop file to.
        """
        EOP_SRC = "ftp://ssd.jpl.nasa.gov/pub/ssd/latest.long"
        
        # get et-ut data file from http://maia.usno.navy.mil/ser7/deltat.data
        eop_file = self.get_file(EOP_SRC)
        
        try :
            # Install eop file in mops
            for d in dest_list :
                # Verify destination directory
                if (not os.path.exists(os.path.dirname(d))):
                    self._logger.error("UPDATE_MOPS_DATA: The destination directory %s does not exist." % (os.path.dirname(d)))
                    continue
                # <-- end if    
                shutil.copyfile(os.path.join(self._workDir, os.path.basename(eop_file)),d)
            # <-- end for            
        except Exception, e:
            self._logger.error("UPDATE_MOPS_DATA: %s" % (str(e)))
            return MOPSUpdater.FAIL
        else:
            self._logger.info("UPDATE_MOPS_DATA: JPL EOP file update complete.")
            return MOPSUpdater.SUCCESS
        # <-- end try          
    # <-- end def 
    
    def get_mpc_catalog(self, dest_list):
        """
        Retrieves the minor planet center catalog from the MPC web site.
        
        @param dest_list: List of directories to copy the mpc catalog to.
        """
        out_fh = None
        in_fh = None  
    
        MPC_SRC = "http://www.minorplanetcenter.net/iau/MPCORB/MPCORB.DAT.gz"
        
        # get mpc catalog
        mpc_file = self.get_file(MPC_SRC)
        if (mpc_file == None): return MOPSUpdater.FAIL
            
        out_file = os.path.join(os.path.dirname(mpc_file), 'MPCORB.DES')
        
        try:
            # Determine output file name.
            i = mpc_file.rfind('.')
            out_fh = open(mpc_file[:i], 'w')
                
            # Decompress the catalog
            self._logger.info("UPDATE_MOPS_DATA: Decompressing %s" % (mpc_file))
            in_fh = gzip.open(mpc_file, 'rb')
            for line in in_fh:
                out_fh.write(line)
            # <-- end for
            self._logger.info("UPDATE_MOPS_DATA: Coverting %s to %s" % (mpc_file[:i], out_file))
            in_fh.close()
            out_fh.close()
            
            # Convert MPC catalog to DES format
            p = sub.Popen(["mpcorb2des", mpc_file[:i]], stdout=open(out_file, 'w'), stderr=sub.PIPE)
            (output, errors) = p.communicate()
            self._logger.info(errors)
            
            # Install MPC catalog
            for d in dest_list :
                # Verify destination directory
                if (not os.path.exists(os.path.dirname(d))):
                    self._logger.error("UPDATE_MOPS_DATA: The destination directory %s does not exist." % (os.path.dirname(d)))
                    continue
                # <-- end if    
                self._logger.info("UPDATE_MOPS_DATA: Installing %s into %s" % (out_file, d))
                shutil.copyfile(out_file,d)
            # <-- end for            
        except Exception, e:
            self._logger.exception("UPDATE_MOPS_DATA: %s" % (str(e)))
            return MOPSUpdater.FAIL
        else:
            self._logger.info("UPDATE_MOPS_DATA: MPC catalog update complete.")
            return MOPSUpdater.SUCCESS
        finally:
            if (out_fh): out_fh.close()
            if (in_fh): in_fh.close()       
        # <-- end try          
    # <-- end def 
    
    def get_known_catalog(self, dest_list):
        tar = None
        out_fh = None
        in_fh = None
        tmp_dir = None
        pattern = None
        catalog_dir = None
        i = None
        
        KNOWN_SRC = "mops@mops03:/home/davide/fitconvtars_cbm10_latest"
        
        # Set-up work directory for configuring known catalog
        tmp_dir = os.path.join(self._workDir, 'known')
        if not(os.path.exists(tmp_dir)): os.mkdir(tmp_dir)
        
        # Set current directory to the work directory
        os.chdir(tmp_dir)
        
        # get fitcmult, fitcnumb, and fitcsing tar balls
        os.system("rsync -avx %s/* %s" % (KNOWN_SRC, tmp_dir))
    
        # Regular expression patterns to be used for cleanup and F51.tracklet file
        f51_pat   = re.compile(r"\bF51\b", re.IGNORECASE)
        aster_pat = re.compile(r"\*\*\*\*\*")
        
        for f in os.listdir(tmp_dir):
            # Verify that file is a tar archive.
            if not(tarfile.is_tarfile(f)): continue
                
            # unzip and unpack tar archive
            self._logger.info("UPDATE_MOPS_DATA: Expanding %s archive." % (f))
            
            # All fatal tar errors are raised as OSError or IOError exceptions, all 
            # non-fatal errors are raised as TarError exceptions
            try:
                # Extract contents of tar file 
                tar = tarfile.open(name=f, mode='r')
                tar.errorlevel = 2
                tar.extractall()
    
                # Determine the name of the directory that holds the extracted catalog
                # It's name is the same as the tar archive without the extensions.
                i = f.find('.')        # Find . which denotes start of extension(s)
                catalog_dir = f[:i]  # Remove extension(s)
    
                # Backup the fitconv.tracklet and fitconv.residual files they will
                # be edited.
                shutil.move(os.path.join(catalog_dir, "fitconv.tracklet"),
                            os.path.join(catalog_dir, "fitconv.tracklet.DIST"))
                shutil.move(os.path.join(catalog_dir, "fitconv.residual"),
                            os.path.join(catalog_dir, "fitconv.residual.DIST"))
                    
                # File to extract F51 observations from.                
                in_fh = open(os.path.join(catalog_dir, "fitconv.tracklet.DIST"), 'r')
                
                # File to write extracted F51 observations to.
                out_fh = open(os.path.join(catalog_dir, "F51.tracklet"), 'w')
               
                # Create F51.tracklet file from fitconv.tracklet file
                for line in in_fh:
                    if f51_pat.search(line): out_fh.write(line)
                # <-- end for
                
                # Clean tracklet and residuals files by removing all lines that 
                # contain the string *****
                in_fh.seek(0)   # Reset file pointer to start of file.
                out_fh.close()
                out_fh = open(os.path.join(catalog_dir, "fitconv.tracklet"), 'w')
                for line in in_fh:
                    if aster_pat.search(line):
                        pass
                    else:
                        out_fh.write(line)
                    # <-- end if
                # <-- end for
                out_fh.close()
                in_fh.close()
                in_fh = open(os.path.join(catalog_dir, "fitconv.residual.DIST"), 'r')
                out_fh = open(os.path.join(catalog_dir, "fitconv.residual"), 'w')
                for line in in_fh:
                    if aster_pat.search(line):
                        pass
                    else:
                        out_fh.write(line)
                    # <-- end if
                # <-- end for
                
            except Exception, e:
                self._logger.error("UPDATE_MOPS_DATA: %s" % (str(e)))
                return MOPSUpdater.FAIL
            finally:
                os.remove(f)
                if (tar): tar.close()
                if (out_fh): out_fh.close()
                if (in_fh): in_fh.close()
            # <--- end try        
        # <-- end for
            
        # Copy known catalog to installation directories
        try:
            for d in dest_list:
                # Verify destination directory
                if (not os.path.exists(os.path.dirname(d))):
                    self._logger.error("UPDATE_MOPS_DATA: The destination directory %s does not exist." % (os.path.dirname(d)))
                    continue
                # <-- end if
                
                # Copy new known catalog to a temporary directory.
                tmp_dir = os.path.join(os.path.dirname(d), "tmp")
                if (os.path.exists(tmp_dir)):
                    self._logger.debug("UPDATE_MOPS_DATA: Deleting old temporary directory %s." % (tmp_dir))
                    shutil.rmtree(tmp_dir)
                # <-- end if 
                
                self._logger.debug("UPDATE_MOPS_DATA: Installing new known catalog into temporary directory %s." % (tmp_dir))  
                shutil.copytree(os.path.join(self._workDir, 'known'), tmp_dir)
                
                # Backup previous catalog
                backup_dir = os.path.join(os.path.dirname(d), "fitconvtars_cbm10_previous")
                if (os.path.exists(backup_dir)):
                    self._logger.info("UPDATE_MOPS_DATA: Deleting old back up directory %s." % (backup_dir))
                    shutil.rmtree(backup_dir)
                # <-- end if 
                if (os.path.exists(d)):
                    self._logger.info("UPDATE_MOPS_DATA: Backing up %s to %s." % (d, backup_dir))                
                    shutil.move(d, backup_dir)
                # <-- end if
                 
                # Install new known catalog
                self._logger.info("UPDATE_MOPS_DATA: Installing new known catalog into %s." % (d))  
                shutil.move(tmp_dir, d)
            # <-- end for            
        except Exception, e:
            self._logger.error("UPDATE_MOPS_DATA: %s" % (str(e)))
            return MOPSUpdater.FAIL
        else:
            self._logger.info("UPDATE_MOPS_DATA: Known catalog update complete.")
            return MOPSUpdater.SUCCESS
        # <-- end try          
    # <-- end def     
    
    def fail_exit(self, component="MOPS data files"):
        self._logger.info("UPDATE_MOPS_DATA: Error occured during the update of %s. Exiting" % (component))
        logging.shutdown()

        # send email informing admin of update error
        reply = 'mopspipe@ifa.hawaii.edu'
        to = self._email
        html = """
            <html>
                <head>
                    <meta http-equiv="Content-Type" content="text/html; charset=utf-8">
                </head>
                <body>
                    <p>Error while updating %s.<br>
                       Please see attached mops updater log file for details.
                    </p>
                </body>
            </html>
        """ % (component)
        msg = MIMEMultipart()
        msg.attach(MIMEText(html, 'html'))
        msg['Subject'] = 'MOPS_UPDATER: Failure while updating %s' % (component)
        msg['From'] = reply
        msg['To'] = to
        
        part = MIMEBase('text', 'plain')
        part.set_payload(open(MOPSUpdater.LOG_FILE, 'r').read())
        part.add_header('Content-Disposition', 'attachment; filename="%s"' % os.path.basename(MOPSUpdater.LOG_FILE))
        msg.attach(part)
        s = smtplib.SMTP('hale.ifa.hawaii.edu')
        s.sendmail(reply, [to], msg.as_string())
        s.quit()
        sys.exit(MOPSUpdater.FAIL)
    # <-- end def
    
    def get_file(self, file_url):
        out_fh = None
        in_fh = None  
        
        # Strip off the file name portion of the file url.
        i = file_url.rfind('/')
        file_name = file_url[i+1:]
        out_file = os.path.join(self._workDir, file_name)
        self._logger.info("UPDATE_MOPS_DATA: Downloading %s to %s." % (file_url, out_file))
        
        try: 
            # Retrieve file and save it to disk      
            in_fh = urllib2.urlopen(file_url)
    
            # Open file to which retrieved file will be written to    
            out_fh = open(out_file, 'w')
    
            for line in in_fh:
                out_fh.write(line)
            # <-- end for
            return out_fh.name
        except Exception, e:
            self._logger.error("UPDATE_MOPS_DATA: %s" % (str(e)))
            return None
        finally:
            if (out_fh): out_fh.close()
            if (in_fh): in_fh.close()            
        # <-- end try   
    # <-- end def
    
    def get_file_name(self, file_path):
        i = file_path.rfind('/')
        return file_path[i+1:]
    # <-- end def
    
    def remove_html_tags(self, data):
        p = re.compile(r'<.*?>')
        return p.sub('', data)
    # <-- end def     
    
    def update_mpc(self, obscode_list):
        # Update observatory codes 
        self._logger.info("UPDATE_MOPS_DATA: Updating MPC observatories codes")
        res = self.get_obscode(obscode_list)      
        if (res == MOPSUpdater.FAIL) : self.fail_exit("MPC osbservatory codes file")
    # <-- end def

    def update_etut(self, etut_list):
        # Update et-ut.dat files
        self._logger.info("UPDATE_MOPS_DATA: Updating ET-UT data file.")
        res = self.get_etut(etut_list)
        if (res == MOPSUpdater.FAIL) : self.fail_exit("ET-UT.dat file")         
    # <-- end def
    
    def update_eop(self, eopc0462_list):
        # Update eopc04.62-now files
        self._logger.info("UPDATE_MOPS_DATA: Updating Earth Orbit Parameter file.")
        res = self.get_eopc0462(eopc0462_list)
        if (res == MOPSUpdater.FAIL) : self.fail_exit("Earth orbit parameters file")
    # <-- end def
    
    def update_jpl(self, eop_list):
        # Update et-ut.dat files
        self._logger.info("UPDATE_MOPS_DATA: Updating JPL Earth Orbit Parameter file.")
        res = self.get_eop(eop_list)
        if (res == MOPSUpdater.FAIL) : self.fail_exit("JPL Earth orbit parameters file.")
    # <-- end def

    def update_known(self, known_list):
        # Update known server catalog
        self._logger.info("UPDATE_MOPS_DATA: Updating Know Server catalog.")
        res = self.get_known_catalog(known_list)
        if (res == MOPSUpdater.FAIL) : self.fail_exit("Known Server catalog")
    # <-- end def
    
    def update_mpc_cat(self, mpc_list):   
        # Update MPC catalog
        self._logger.info("UPDATE_MOPS_DATA: Updating MPC catalog.")
        res = self.get_mpc_catalog(mpc_list)
        if (res == MOPSUpdater.FAIL) : self.fail_exit("Minor Planet Center catalog")\
    # <-- end def
# <-- end class
   
#------------------------------------------------------------------------------
# Entry point.
#------------------------------------------------------------------------------
def main(args=sys.argv[1:]):

    MOPS_DATA = "/export/mops02.0/MOPS_DATA"
    KNOWN_DATA = "/data/mops12.0/KNOWN"
    
    parser = optparse.OptionParser(USAGE)
    parser.add_option('--obscode',
                      action='store_true',
                      dest='obscode',
                      default=False,
                      help="Updates MPC observatory codes.\n")
    parser.add_option('--deltaT',
                      action='store_true',
                      dest='deltaT',
                      default=False,
                      help="Updates the ET-UT file.\n")
    parser.add_option('--eop',
                      action='store_true',
                      dest='eop',
                      default=False,
                      help="Updates the earth orbit parameter file.\n")
    parser.add_option('--jpl',
                      action='store_true',
                      dest='jpl',
                      default=False,
                      help="Updates the JPL earth orbit parameter file.\n")
    parser.add_option('--known',
                      action='store_true',
                      dest='known',
                      default=False,
                      help="Updates the Known server catalog.\n")
    parser.add_option('--mpc',
                      action='store_true',
                      dest='mpc',
                      default=False,
                      help="Updates the Minor Planet Center catalog.\n")
    parser.add_option('--all',
                      action='store_true',
                      dest='all',
                      default=False,
                      help="Updates all of the MOPS data files and catalogs.\n") 
    parser.add_option('--email',
                      action='store',
                      dest='email',
                      default='mopsczar@ifa.hawaii.edu',
                      help="Email address to notify if there is an error during the update.\n")                                           
    parser.add_option('-v',
                      action='store_true',
                      dest='verbose',
                      default=False,
                      help="Prints extra information to log file.\n")                    
    
    # Get the command line options and also whatever is passed on STDIN.
    (options, args) = parser.parse_args()
    
    # Get logger and set logging level. Typically set level to info.
    # Add null handler to logger to avoid the No handlers could be found
    # for logger XXX error if a handler is not found in a higher level logger
    logger = Utilities.getLogger(logFile=MOPSUpdater.LOG_FILE, mode="w")
    if (options.verbose):
        logger.setLevel(logging.DEBUG)
    else:
        logger.setLevel(logging.INFO)
    # <-- end if
     
    logger.info("UPDATE_MOPS_DATA: Starting update_mops_data")
    
    # Create a temporary directory to hold downloaded files.
    workDir = tempfile.mkdtemp(dir="/export/mops02.0/tmp")
    
    # Create a MOPSUpdater object which will update mops.
    updater = MOPSUpdater(workDir, options.email, logger)
    
    #---------------------------------------------------------------------------
    # UPDATE MPC OBSERVATORY CODES FILE.
    #---------------------------------------------------------------------------
    if (options.obscode or options.all ):   
        # Locations to save obscode file
        obscode_list = [ ''.join([MOPS_DATA, '/caet_data/obscode.mpc']),
                         ''.join([MOPS_DATA, '/oorb/OBSCODE.dat']),
                         ''.join([MOPS_DATA, '/orbfit/OBSCODE.dat']),
                         ''.join([MOPS_DATA, '/orsa/obscode.mpc']),
                         ''.join([MOPS_DATA, '/scripts/OBSCODE.dat']),
                         ''.join([KNOWN_DATA, '/lib/OBSCODE.dat']) ]
        updater.update_mpc(obscode_list)
    # <-- end if
    
    #---------------------------------------------------------------------------
    # UPDATE ET-UT.DAT FILE.
    #---------------------------------------------------------------------------
    if (options.deltaT or options.all ):     
        # Locations to save ET-UT.dat file
        etut_list = [ ''.join([MOPS_DATA, '/oorb/ET-UT.dat']),
                      ''.join([MOPS_DATA, '/orbfit/ET-UT.dat']),
                      ''.join([KNOWN_DATA, '/lib/ET-UT.dat']) ]
        updater.update_etut(etut_list)
    # <-- end if
    
    #---------------------------------------------------------------------------
    # UPDATE EOP.DAT FILE.
    #---------------------------------------------------------------------------
    if (options.eop or options.all ):
        # Locations to save eopc04.62-now file
        eopc0462_list = [ ''.join([MOPS_DATA, '/orbfit/eopc04/eopc04.62-now']),
                          ''.join([KNOWN_DATA, '/lib/eopc04/eopc04.62-now']) ]
        updater.update_eop(eopc0462_list)
    # <-- end if
    
    #---------------------------------------------------------------------------
    # UPDATE JPL EOP.DAT FILE.
    #---------------------------------------------------------------------------
    if (options.jpl or options.all ):
        # Locations to save eop.dat file
        eop_list = [ ''.join([MOPS_DATA, '/caet_data/EOP/latest']) ]
        updater.update_jpl(eop_list)      
    # <-- end if
    
    #---------------------------------------------------------------------------
    # UPDATE KNOWN SERVER CATALOG.
    #---------------------------------------------------------------------------
    if (options.known or options.all ):
        # Locations to save known server catalog
        known_list = [ ''.join([KNOWN_DATA, '/fitconvtars_cbm10_latest']) ]
        updater.update_known(known_list)
    # <-- end if
    
    #---------------------------------------------------------------------------
    # UPDATE MPC CATALOG FILE.
    #---------------------------------------------------------------------------
    if (options.mpc or options.all ):
        # Locations to save Minor Planet Center catalog
        mpc_list = [ ''.join([MOPS_DATA, '/mpcorb/MPCORB.DES']) ]
        updater.update_mpc_cat(mpc_list)
    # <-- end if 
    
    #---------------------------------------------------------------------------
    # SYNC FILES TO CLUSTER.
    #---------------------------------------------------------------------------
    logger.info("UPDATE_MOPS_DATA: Synchronizing updated files to MOPS cluster.")
    p = sub.Popen(["rsyncdata"], stdout=sub.PIPE, stderr=sub.PIPE)
    (output, errors) = p.communicate()
    logger.info(output)

    # Clean up temporary directory used to hold downloaded files.
    shutil.rmtree(workDir)
    logging.shutdown()
# <-- end def

if __name__ == '__main__':
    sys.exit(main())
# <-- end if    