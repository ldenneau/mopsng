#!/bin/bash

python $HOME/dev/dstamp/dpstamp.py $@

last=${@: -1}
echo "/usr/bin/scp mops@ippc18:$last.fits ."
