#!/bin/bash

set -e

# output is in stdout.

N=$1
./makeorb $N.obs.des orb.des > $N.orb.des
oorb --task=ephemeris --G=0.15 --code=F51 --conf=/home/mops/MOPS_STABLE/config/oorb.conf --obs-in=$N.obs.des --orb-in=$N.orb.des > $N.oorb.raw
./oorb2trueanom orb.des $N.oorb.raw > $N.oorb.out
