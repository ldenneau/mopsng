#!/bin/bash

export ORBFIT_DATA=/home/mops/MOPS_STABLE/data/orbfit CAET_DATA=/home/mops/MOPS_STABLE/data/caet_data OORB_DATA=/home/mops/MOPS_STABLE/data/oorb MOPS_VAR=/home/mops/MOPS_STABLE/var/psmops_ps1_mdrm152 MOPS_HOME=/home/mops/MOPS_STABLE MOPS_DBPASSWORD=epip PATH=/home/mops/MOPS_STABLE/var/psmops_ps1_mdrm152/bin:/home/mops/MOPS_STABLE/bin:/bin:/usr/bin:/usr/local/bin:/opt/bin:/home/mopspipe/bin:/home/mopspipe/jdk/bin:/opt/intel/fce/10.0.023/bin:/opt/intel/idbe/10.0.023/bin:/usr/local/bin:/usr/bin:/bin:/opt/bin:/usr/x86_64-pc-linux-gnu/gcc-bin/4.1.2:/opt/blackdown-jdk-1.4.2.03/bin:/opt/blackdown-jdk-1.4.2.03/jre/bin:/usr/games/bin:/usr/local/condor/bin:/usr/local/bin:/home/mopspipe/bin PERL5LIB=/home/mops/MOPS_STABLE/var/psmops_ps1_mdrm152/lib/perl5:/home/mops/MOPS_STABLE/lib/perl5 MOPS_DBINSTANCE=psmops_ps1_mdrm152

DATE=`date -u +%s | awk '{printf "%.6f\n", $1/86400+40587.0}'`
MJD=`echo $DATE | sed 's/\..*//'`

MPCDES="$MOPS_HOME/data/mpcorb/mpc.des"
MPCSQ="$MOPS_HOME/data/mpcorb/mpc.sqlite3"

TRASHDIR=$MOPS_HOME/data/mpcorb/trash/$MJD
mkdir -p $TRASHDIR

if [ -e $MPCDES  ]; then
  /bin/mv -f $MPCDES $TRASHDIR
fi
if [ -e $MPCSQ  ]; then
  /bin/mv -f $MPCSQ $TRASHDIR
fi

/usr/bin/wget -q -O $MPCDES http://atlas-base-adm01/mpcorb/mpc.des 
/usr/bin/wget -q -O $MPCSQ http://atlas-base-adm01/mpcorb/mpc.sqlite3

if [ -e $MOPS_VAR/eph/$MJD.eph ]; then
  echo "Moving exisitng $MOPS_VAR/eph/$MJD.eph to $TRASHDIR"
  /bin/mv -f $MOPS_VAR/eph/$MJD.eph $TRASHDIR
fi

/home/mops/MOPS_STABLE/bin/qknown --instance=$MOPS_DBINSTANCE --prep $DATE >> $MOPS_VAR/log/qknown.log
