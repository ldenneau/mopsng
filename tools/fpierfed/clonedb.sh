#!/bin/sh
USER=mops
PASS=mops

if [ $# -ne 2 ]; then
	echo usage: clonedb.sh databasename remotehost
	exit 1
fi

HOST=$2
DB=$1

echo Cleaning up remote site ($HOST)...
mysqladmin -u $USER -p$PASS -h $HOST drop $DB
mysqladmin -u $USER -p$PASS -h $HOST create $DB

echo Cloning database $DB to $HOST..
mysqldump --opt -u $USER -p$PASS --add_drop_table $DB | mysql -u $USER -p$PASS -h $HOST $DB
echo Done.
