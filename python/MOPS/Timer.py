"""
$Id$

Little timer class for help emitting timing data.

"""

__author__ = 'Larry Denneau, Institute for Astronomy, University of Hawaii'
__date__ = '$Date$'
__version__ = '$Revision$'[11:-2]


import sys
import time
import MOPS.Lib


class Timer(object):
    """
    Just a little utility class to do timings within a module.
    """
    def __init__(self, logger=None):
        # Set up.
        self.t0 = time.time()
        self.logger = logger

    def mark(self, subsys, subsubsys=None):
        # Print at message with delta_t from the previous mark; save mark.
        t1 = time.time()
        msg = MOPS.Lib.formatTimingMsg(
                subsystem=subsys, 
                nn=0, 
                subsubsystem=subsubsys,
                time_sec=(t1 - self.t0)
        )
        if self.logger:
            self.logger.info(msg)
        else:
            sys.stderr.write(msg + "\n")
        self.t0 = t1


if __name__ == "__main__":
    foo = Timer()
    time.sleep(1)
    foo.mark('TEST')
