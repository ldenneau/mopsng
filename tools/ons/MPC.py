"""
Module to do some MPC conversions.
"""

import math
import slalib

def mpc2dict(line):
    """
    Convert a single MPC line to a dict containing the following keys:
      id
      epoch_mjd
      ra_deg
      dec_deg
      mag
      filt
      obscode

    If the mag is not reported, mag is set to None.
    """

    try:
        trk_id = line[5:12]
        year = int(line[15:19])
        month = int(line[20:22])
        day = int(line[23:25])
        frac = float(line[25:32])
        ra_sx = line[32:44]
        dec_sx = line[44:56]
        obscode = line[77:80]
        filt = line[70]
        mag = line[65:70]
        mag = mag.strip()
        if not mag:
            mag = None
        else:
            mag = float(mag)

        # Convert our stuff.
        ra_deg = math.degrees(slalib.sla_dafin(ra_sx, 1)[1] * 15)
        dec_deg = math.degrees(slalib.sla_dafin(dec_sx, 1)[1])
        epoch_mjd = slalib.sla_cldj(year, month, day)[0] + frac
    except ValueError, e:
        raise ValueError(str(e) + ": " + trk_id)

    return {
        'id': trk_id,
        'epoch_mjd': epoch_mjd,
        'ra_deg': ra_deg,
        'dec_deg': dec_deg,
        'mag': mag,
        'filt': filt,               # might be ' '
        'obscode': obscode,
    }

if __name__ == "__main__":
    raw = """\
     s46064   C2004 06 12.17542014 12 46.569+25 42 56.64               e     645
     s46064   C2004 06 12.17889214 12 46.553+25 42 55.47               e     645
     s46065   C2004 06 12.17917014 18 41.531+25 28 34.09         21.22Ve     645
     s46065   C2004 06 12.18264214 18 41.503+25 28 30.43         21.93 e     645
     s4606d   C2004 06 12.24655015 58 25.350+17 54 16.70         19.92Ve     645
     s4606d   C2004 06 12.25002215 58 25.313+17 54 17.33         20.44 e     645
     s4606f   C2004 06 12.25571016 11 16.125+16 40 01.10         16.07Ve     645
     s4606f   C2004 06 12.25918216 11 16.107+16 40 00.52         16.86 e     645
     s46070   C2004 06 12.27658016 39 53.484+13 39 22.09         20.98Ve     645
     s46070   C2004 06 12.28005216 39 53.225+13 39 19.55         21.72 e     645
"""
    lines = raw.split('\n')
    for line in lines:
        if not line:
            continue        # handle last, empty line
        print mpc2dict(line)
