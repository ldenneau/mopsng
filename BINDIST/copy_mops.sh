#!/bin/sh

rsync -av --files-from=$HOME/BINDIST/MOPS_DEV.MANIFEST denneau@schmopc00:/usr/local/MOPS_DEV .
rsync -av --files-from=$HOME/BINDIST/MOPS_DATA.MANIFEST denneau@schmopc00:/home/MOPS_DATA data
rsync -av --files-from=$HOME/BINDIST/ROOT.MANIFEST denneau@schmopc00:MOPS/CURRENT/src/trunk/BINDIST/root .
