#!/bin/sh
findTracklets \
    file 54593.dets \
    pairfile 54593.pairs \
    summaryfile /dev/null \
    minv 0.0 \
    maxv 2.0 \
    minobs 2 \
    maxt 0.050 \
    etime 30.0
