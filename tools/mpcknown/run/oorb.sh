#!/bin/bash

set -e

# Create 
echo Splitting input file obs.des.
nsplit --repeat_headers --prefix --num 150 obs.des

echo Submitting Condor job.
cat /dev/null > oorb.condorlog
JOB_ID=`condor_submit oorb.job | sed '/submitted/!d; s/.*submitted to cluster //; s/\.//'` 
echo Waiting for $JOB_ID
condor_wait oorb.condorlog $JOB_ID
