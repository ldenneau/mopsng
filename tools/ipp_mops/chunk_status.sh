#!/bin/bash

# $Id: chunk_status.sh 4190 2010-10-14 19:08:46Z schastel $

usage() {
        echo "Usage: $0 [date] [chunk]"
	echo "      Shows the status which names match [chunk] for some [date]"
	echo "	    Both arguments are optional (default value: today, OSS)"
	echo "        [date]: today, today-1, today-2, 2010-10-10"
	echo "	      [chunk]: chunk pattern, e.g. OSS, W+6"
	echo "      The script depends on ipp_mops_query to query all chunks for the [date]. The lines"
	echo "      are grep'd using [chunk] (you can use regex)"
	echo "   "
        echo "   or: $0 [--help|-h]"
        echo "          Show this help"
	echo ""
	echo "This script can be used without risk for the MOPS subsystem"
	exit 0
}

if [ $# -gt 0 ]; then
  case $1 in
  --help) 
  	usage
	;;
  -h) 
  	usage
	;;
  esac
fi

DEFAULT_DATE="today"
DEFAULT_CHUNK="OSS"
_date=${1:-$DEFAULT_DATE}
_chunk=${2:-$DEFAULT_CHUNK}

now=`date --rfc-3339=seconds`
echo "$now: Running for date=[$_date] and chunk=[$_chunk]"

UNFOUND=`ipp_mops_query --summary $_date | grep $_chunk | grep UNFOUND | wc -l`
echo "UNFOUND:$UNFOUND"
FOUND=`ipp_mops_query --summary $_date | grep $_chunk | grep -v UNFOUND | wc -l`
echo "FOUND:$FOUND"

# Tell which command should be run
if [ "$UNFOUND" -eq "0" ]; then
    if [ "$FOUND" -ne "0" ]; then 
	chunks=`ipp_mops_query --summary $_date | grep $_chunk | cut -f3 -d' ' | sed 's/\.[griwy]$//g' | sort -u | tr '\n' ' '`
	echo "You will likely run the following command now:"
	echo "	ipp_mops_query --chunk $chunks"
	echo "and, once its output confirmed:"
	echo "	ingest \`ipp_mops_query --chunk $chunks\`"
    else
	echo "No data for $_date"
    fi
else
	echo "Data are likely still being processed by IPP"
fi
