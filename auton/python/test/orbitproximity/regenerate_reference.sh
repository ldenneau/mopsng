#!/bin/sh
orbitproximity data DATA.orbits \
    queries QUERIES.orbits \
    matchfile MATCH.orbits \
    q_thresh 0.10 \
    e_thresh 0.050 \
    use_names no
