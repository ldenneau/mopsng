#!/usr/bin/env python
"""
IMPORTANT: this is trow-away code!!!! Do not use it for anything serious.
"""
import datetime
import glob
import sys
import time



header = [' DTCTL:  %s',
          ' LODCTL: %s',
          ' PRECOV: %s',
          ' ATTRIB: %s']



for fileName in sys.argv[1:]:
    if(not fileName.endswith('.log')):
        continue
    # <-- end if
    
    times = [[], [], [], []]
    
    f = file(fileName)
    line = f.readline()
    # We are only interested in lines with starting and exiting information for
    # PRECOV, LODCTL and DTCTL.
    while(line):
        # DTCTL
        if(line.endswith('Starting DTCTL. \n')):
            times[0].append(line[:line.find(' ', 12)])
        elif('DTCTL exiting:' in line):
            times[0].append(line[:line.find(' ', 12)])
        # LODCTL
        elif(line.endswith('Starting LODCTL. \n')):
            times[1].append(line[:line.find(' ', 12)])
        elif('LODCTL exiting:' in line):
            times[1].append(line[:line.find(' ', 12)])
        # PRECOV
        elif(line.endswith('PRECOV: starting.\n')):
            times[2].append(line[:line.find(' ', 12)])
        elif(line.endswith('PRECOV: done.\n')):
            times[2].append(line[:line.find(' ', 12)])
        # ATTRIB
        elif(line.endswith('ATTRIB: starting.\n')):
            times[3].append(line[:line.find(' ', 12)])
        elif(line.endswith('ATTRIB: done.\n')):
            times[3].append(line[:line.find(' ', 12)])
        # Skip
        else:
            pass
        # <-- end if
        line = f.readline()
    # <-- end while

    # Print out results.
    print('--- %s ---' %(fileName))
    
    # Just a simple sanity check.
    for i in range(len(times)):
        durations = []
        
        if(len(times[i]) % 2 != 0):
            # We must have processed an incomplete log.
            times[i].pop(-1)
        # <-- end if

        # Compute the time deltas.
        fmt = '%Y/%m/%d %H:%M:%S'
        dts = [datetime.datetime(*time.strptime(t, fmt)[:6]) for t in times[i]]
        deltas = [dts[j+1] - dts[j] for j in range(0, len(dts), 2)]
        print(header[i] %(' '.join([str(d.seconds) for d in deltas])))
# <-- end for

        
        
        
