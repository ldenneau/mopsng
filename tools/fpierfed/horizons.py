#!/usr/bin/env python

import logging
import optparse
import os
import os.path
import re
import smtplib
import sys
import telnetlib
import time

import MOPS.Utilities as Utilities

#------------------------------
# Constants
#------------------------------
USAGE = '''USAGE:

horizons.py [options] input_file

Given an orbit file in DES format and a list of times in MJD format, connect to 
the HORIZONS telnet server and retrieve the ephemerides for each of the objects 
in the input file covering the times provided.

Print out the values on a per object basis.
'''

HORIZONS_HOST = 'horizons.jpl.nasa.gov'
HORIZONS_PORT = 6775

#------------------------------
# Functions
#------------------------------
def mjd2jd(mjd):
    return(mjd + 2400000.5)
# <-- end def

def jd2mjd(jd):
    return(jd - 2400000.5)
# <-- end def

#------------------------------
# Functions
#------------------------------
class Horizons(object):
    """
    Encapsulate a connection with HORIZONS.
    """
    def __init__(self, debug=0):
        """
        Connect to HORIZONS.
        """
        self._connection = telnetlib.Telnet(HORIZONS_HOST, HORIZONS_PORT)
        if(debug > 0):
            self._connection.set_debuglevel(debug)
        return
    # <-- end def
    
    def _restart(self):
        """
        Restart the ephemerides computation.
        """
        while(True):
            prompt = self._connection.read_some().strip()
            print prompt
            if(prompt.endswith('to continue -->')):
                # Horizons could not guess the screen size.
                self._connection.write('\n')
                self._connection.write('tty 140 200\n')
                continue
            elif(prompt.endswith('s>')):
                # Ready to roll!
                self._connection.write(';\n')
                return
            elif(prompt.endswith('[R]edisplay, ? :')):
                 self._connection.write('N\n')
                 self._connection.write(';\n')
                 self._connection.read_until('Clear previous object   [ Yes, No, ? ]: ')
                 self._connection.write('Yes\n')
                 return
            else:
                continue
            # <-- end if
        # <-- end while
        return
    # <-- end def
    
    def _input_elements(self, epoch, q, e, i, node, arg_peri, time_peri,
                        mag=99, g=0.150, src=None, name='pippo'):
        self._connection.read_until('Input heliocentric ecliptic osculating elements -- "?" for info, <cr> when done')
        self._connection.write('EPOCH= %f\n' %(mjd2jd(epoch)))
        self._connection.read_until(':')
        self._connection.write('EC= %f\n' %(e))
        self._connection.read_until(':')
        self._connection.write('QR= %f\n' %(q))
        self._connection.read_until(':')
        self._connection.write('TP= %f\n' %(mjd2jd(time_peri)))
        self._connection.read_until(':')
        self._connection.write('OM= %f\n' %(node))
        self._connection.read_until(':')
        self._connection.write('W = %f\n' %(arg_peri))
        self._connection.read_until(':')
        self._connection.write('IN= %f\n' %(i))
        self._connection.read_until(':')
        if(mag > 0 and mag < 99):
            self._connection.write('H = %f\n' %(mag))
            self._connection.read_until(':')
            self._connection.write('G = %f\n' %(g))
            self._connection.read_until(':')
        if(src and len(src) == 21):
            self._connection.write('SRC= %e %e %e\n' %tuple(src[:3]))
            self._connection.read_until(':')
            self._connection.write('     %e %e %e\n' %tuple(src[3:6]))
            self._connection.read_until(':')
            self._connection.write('     %e %e %e\n' %tuple(src[6:9]))
            self._connection.read_until(':')
            self._connection.write('     %e %e %e\n' %tuple(src[9:12]))
            self._connection.read_until(':')
            self._connection.write('     %e %e %e\n' %tuple(src[12:15]))
            self._connection.read_until(':')
            self._connection.write('     %e %e %e\n' %tuple(src[15:18]))
            self._connection.read_until(':')
            self._connection.write('     %e %e %e\n' %tuple(src[18:]))
            self._connection.read_until(':')
        self._connection.write('\n\n')
        self._connection.read_some()
        self._connection.read_until(' Ecliptic frame of input [J2000, B1950]: ')
        self._connection.write('J2000\n')
        self._connection.read_until(' Optional name of object (24-char max) : ')
        self._connection.write('%s\n' %(name))
        return
    # <-- end def

    def _setup_output(self, obscode, start_time, end_time, step):
        self._connection.read_until(' Select ... [A]pproaches, [E]phemeris, [F]tp,[M]ail,[R]edisplay, [S]PK,?,<cr>: ')
        self._connection.write('e\n')

        self._connection.read_until(' Observe, Elements, Vectors  [o,e,v,?] : ')
        self._connection.write('o\n')

        data = self._connection.read_until(': ')
        if(data.endswith('Use previous center  [ cr=(y), n, ? ] : ')):
            self._connection.write('n\n')
            self._connection.read_until(' Coordinate center [ <id>,coord,geo  ] : ')
            self._connection.write('%s\n' %(obscode))
        elif(data.endswith('Coordinate center [ <id>,coord,geo  ] : ')):
            self._connection.write('%s\n' %(obscode))
        else:
            raise(SyntaxError('Unknown question: "%s"' %(data)))
        self._connection.read_until(' Confirm selected station    [ y/n ] --> ')
        self._connection.write('y\n')
        
        self._connection.read_until(' Starting UT  [>=   1599-Dec-10 23:59] : ')
        self._connection.write('JD %f\n' %(mjd2jd(start_time)))
        self._connection.read_until(' Ending   UT  [<=   2201-Feb-17 23:58] : ')
        self._connection.write('JD %f\n' %(mjd2jd(end_time)))
        self._connection.read_until(' Output interval [ex: 10m, 1h, 1d, ? ] : ')
        self._connection.write('%dd\n' %(step))

        self._connection.read_until(' Accept default output [ cr=(y), n, ?] : ')
        self._connection.write('n\n')
        self._connection.read_until(' Select table quantities [ <#,#..>, ?] : ')
        self._connection.write('1,9,36,37\n')
        self._connection.read_until(' Output reference frame [J2000, B1950] : ')
        self._connection.write('J2000\n')
        self._connection.read_until(' Time-zone correction   [ UT=00:00,? ] : ')
        self._connection.write('\n')
        self._connection.read_until(' Output UT time format   [JD,CAL,BOTH] : ')
        self._connection.write('JD\n')
        self._connection.read_until(' Output time digits  [MIN,SEC,FRACSEC] : ')
        self._connection.write('SEC\n')
        self._connection.read_until(' Output R.A. format       [ HMS, DEG ] : ')
        self._connection.write('DEG\n')
        self._connection.read_until(' Output high precision RA/DEC [YES,NO] : ')
        self._connection.write('YES\n')
        self._connection.read_until(' Output APPARENT [ Airless,Refracted ] : ')
        self._connection.write('Airless\n')
        self._connection.read_until(' Set units for RANGE output [ KM, AU ] : ')
        self._connection.write('AU\n')
        self._connection.read_until(' Suppress RANGE_RATE output [ YES,NO ] : ')
        self._connection.write('NO\n')
        self._connection.read_until(' Minimum elevation [ -90 <= elv <= 90] : ')
        self._connection.write('\n')
        self._connection.read_until(' Maximum air-mass  [ 1 <=   a  <= 38 ] : ')
        self._connection.write('\n')
        self._connection.read_until(' Print rise-transit-set only [N,T,G,R] : ')
        self._connection.write('N\n')
        self._connection.read_until(' Skip printout during daylight [ Y,N ] : ')
        self._connection.write('N\n')
        self._connection.read_until(' Solar elongation cut-off   [ 0, 180 ] : ')
        self._connection.write('\n')
        self._connection.read_until(' Spreadsheet CSV format        [ Y,N ] : ')
        self._connection.write('Y\n')
        return
    # <-- end def
    
    def _parse_results(self):
        # Returns an array containing the ephemerides calculated by JPL.
        # The format of the returned array is:
        #   epoch, ra, dec, mag, err_ra, err_dec, smaa, smia, pa
        self._connection.read_until('$$SOE')

        res = []
        raw_data = self._connection.read_until('$$EOE')
        for line in raw_data.split('\n'):
            line = line.strip()
            line = line.replace('\r', '')
            if(not line):
                continue
            elif(line.startswith('$$EOE')):
                break
            (t, a, b, ra, dec, mag, err_ra, err_dec, smaa, smia, pa, area, d) = line.split(',')
            t = jd2mjd(float(t))
            ra = float(ra)
            dec = float(dec)
            mag = float(mag)
            if( 'n.a.' in err_ra):
                err_ra = None
            else:
                err_ra = float(err_ra)
            if('n.a.' in err_dec):
                err_dec = None
            else:
                err_dec = float(err_dec)
            if('n.a.' in smaa):
                smaa = None
            else:
                smaa = float(smaa)
            if('n.a.' in smia):
                smia = None
            else:
                smia = float(smia)
            if('n.a.' in pa):
                pa = None
            else:
                pa = float(pa)
            res.append((t, ra, dec, mag, err_ra, err_dec, smaa, smia, pa))
        # <-- end for
        return(res)
    # <-- end def

    def ephemerides(self, epoch, q, e, i, node, arg_peri, time_peri, times,
                    mag=99, g=0.150, obscode="F51", oid=None, src=None):
        """
        Given an object (specified by its 6 obital parameters, the orbit epoch
        and, optionally magnitude, g parameter, src, and object id) and a list of times,
        retrieve the ephemerides for that object at those times from HORIZONS.

        times is a 3 element list of the form [start_mjd, end_mjd, delta_mjd]
        """

        self._restart()
        self._input_elements(epoch, q, e, i, node, arg_peri, time_peri,
                             mag, g, src)
        self._setup_output(obscode, times[0], times[1], times[2])
        res = self._parse_results()
        # Loop through results and insert oid as the first element of each
        # result tupple.
        i = 0
        for e in res :
            e = list(e)
            e.insert(0,oid)
            res[i] = tuple(e)
            i += 1
        # <-- end for 
        return(res)
    # <-- end def
# <-- end class

#------------------------------------------------------------------------------
# Entry point.
#------------------------------------------------------------------------------
def main(args=sys.argv[1:]):
    parser = optparse.OptionParser(USAGE)
    parser.add_option('--start',
                      action='store',
                      dest='start_mjd',
                      type='float',
                      #default=AGE,
                      help="The start date for ephemerides generation.\n")
    parser.add_option('--end',
                      action='store',
                      dest='end_mjd',
                      type='float',
                      #default=False,
                      help="The end date for ephemerides generation.\n")
    parser.add_option('--delta',
                      action='store',
                      dest='delta',
                      type='float',
                      default=1,
                      help="""The time seperating each generated ephemerides.\n
                              Defaults to one day.\n""")
    parser.add_option('--obs',
                      action='store',
                      dest='obscode',
                      type='string',
                      default="F51",
                      help="""The code of the observatory for which the ephemerides is to be generated.\n
                              Defaults to F51 if not specified. \n""")
                              
    parser.add_option('--out',
                      action='store',
                      dest='out_file',
                      type='string',
                      help="""A file to write output to. \n""")                              
    # Verbose flag
    parser.add_option('-v',
                      action='store_true',
                      dest='verbose',
                      default=False,
                      help="Prints extra information to log file.\n")                    
    
    # Get the command line options and also whatever is passed on STDIN.
    (options, args) = parser.parse_args()

    # Make sure that we have what we need. 
    if(not options.start_mjd): 
       parser.error('No --start specified.') 
    # <-- end if 

    if(not options.end_mjd): 
       parser.error('No --end specified.') 
    # <-- end if 

    # Verify that an input file was specified
    if (len(args) < 1):
        parser.error('An input file was not specified.')
    # <-- end if
    
    # Get logger and set logging level. Typically set level to info.
    # Add null handler to logger to avoid the No handlers could be found
    # for logger XXX error if a handler is not found in a higher level logger
    global gLogger  
    gLogger = Utilities.getLogger('JPL', None)
    if (options.verbose):
        gLogger.setLevel(logging.DEBUG)
    else:
        gLogger.setLevel(logging.INFO)
    # <-- end if
    
    # Verify that input file exists
    input_file = args[0]
    if (not os.path.exists(input_file)) :
        gLogger.error("HORIZONS: %s does not exist!!" % (input_file))
        raise RuntimeError("HORIZONS: %s does not exist!!" % (input_file))
    # <-- end if
    
    h = Horizons(debug=0)
    res = []
    gLogger.info("HORIZONS: Opening %s" % (input_file))
    try:
        fh = open(input_file, "r")
        while True:
            line = fh.readline()
            if len(line) == 0: # Zero length indicates EOF
                break
            # <-- end if
            
            match = re.search(r'^!', line)
            if match:
                # DES header or comment skip line.
                continue
            # <-- end if

            match = re.search(r'^\s+$', line)
            if match:
                # Blank line skip.
                continue
            # <-- end if

            # Parse the line for its orbital elements.
            cols = line.split(" ")
            q = float(cols[2])
            e = float(cols[3])
            i = float(cols[4])
            node = float(cols[5])
            argPeri = float(cols[6])
            timePeri = float(cols[7])
            h_v = float(cols[8])
            epoch = float(cols[9])
            object_id = cols[0]

            # Ask JPL horizons for ephemerides
            times = (options.start_mjd, options.end_mjd, options.delta)
            res += h.ephemerides(epoch, q, e, i, node, argPeri, timePeri, times, h_v, .150, options.obscode, object_id)
            """
                res = h.ephemerides(epoch=53458.,
                        q=.6201443674443176,
                        e=.5887584958766938,
                        i=4.222821075488327,
                        node=224.2500776466688,
                        arg_peri=90.6966378352936,
                        time_peri=53260.8239252879284,
                        times=(53522, 53523, 1),
                        mag=18.848,
                        g=.150,
                        src=[8.918907774759833E-9, -1.142810787742955E-8,
                             1.668718350363152E-8, 2.209344232000432E-8,
                             -4.46902785491213E-8, 9.630403388871625E-6,
                             -2.773490120559766E-8, 6.580845530891771E-8, 
                             5.770190812395115E-6, 1.784156547157364E-7, 
                             -1.53148919367182E-8, 3.494932893594178E-8,
                             -9.96775894966775E-9, -3.115210512717402E-7, 
                             3.788909051693304E-7, -1.976329090803309E-9, 
                             5.428725864025983E-9, -4.674612067825687E-7,
                             3.141170291580454E-7, -3.067995347802948E-7, 
                             1.163429608362596E-7],
                        obscode=568)
            """
        # <-- end while
        fh.close()
        
        # Write ephemerides out
        if (options.out_file) :
            sys.stdout = open(options.out_file, "w")
        # <-- end if
        
        for ephem in res :
            sys.stdout.write("%s %s %s %s %s %s %s %s %s %s\n" % (ephem[0], ephem[1], ephem[2], ephem[3], ephem[4], ephem[5], ephem[6], ephem[7], ephem[8], ephem[9]))
        # <-- end for
        if (options.out_file) :
            sys.stdout.close()
            # Restore standard output.
            sys.stdout = sys.__stdout__
        # <-- end if
                    
    except Exception, e:
        gLogger.exception("HORIZON: %s" % (str(e)))
        sys.stderr.write(str(e))
        raise SystemExit(1)
    # <-- end try
# <-- end def

if __name__ == '__main__':
    sys.exit(main())
# <-- end if            