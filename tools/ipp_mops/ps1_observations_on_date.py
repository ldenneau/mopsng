#!/usr/bin/env python

import sys
import datetime
import MySQLdb
import math

def usage():
    print """
ps1_observations_for_date.py:
  Gives the list of PS1 observation times, exposure names, observation mode, and comment on a given date

  ps1_observations_on_date.py <YYYY-MM-DD>
  e.g.:
  ps1_observations_on_date.py 2011-01-30
  ps1_observations_on_date.py 2012-02-29

"""

if __name__ == "__main__":
    if len(sys.argv) == 1:
        usage()
        sys.exit(1)

    begin = datetime.datetime.strptime(sys.argv[1], "%Y-%m-%d")
    end = begin + datetime.timedelta(days = 1)

    #gpc1 = MySQLdb.connect('ipp0012', 'mops', 'mops', 'gpc1')
    gpc1 = MySQLdb.connect('ipp0012', 'ipp', 'ipp', 'gpc1')
    cursor = gpc1.cursor()
    cursor.execute("""
SELECT 
  dateobs, exp_name, ra, decl, comment, object 
FROM 
  rawExp 
WHERE 
  dateobs >= '%s' 
  AND dateobs <= '%s'
ORDER BY dateobs
""" % (begin, end) )
    for row in cursor.fetchall():
        date = datetime.datetime.strftime(row[0], "%Y-%m-%d %H:%M:%S")
        exp_name = row[1]
        ra = row[2]
        dec = row[3]
        comment = row[4]
        mode = row[5]
        print "%10s  %10s  %10.6f  %10.6f  %10s  %s" % (date, exp_name, math.degrees(ra), math.degrees(dec), mode, comment)
    cursor.close()
    gpc1.close()
