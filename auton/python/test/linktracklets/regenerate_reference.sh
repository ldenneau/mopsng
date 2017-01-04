#!/bin/sh
linktracklets \
    file 1087K.tracklets \
    trackfile /dev/null \
    idsfile /dev/null \
    summaryfile 1087K.sum \
    min_obs 6 \
    min_sup 3 \
    vtree_thresh 0.0004 pred_thresh 0.0008 \
    plate_width .0001
