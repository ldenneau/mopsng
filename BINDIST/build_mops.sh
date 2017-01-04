#!/bin/sh

# Make sure .psmops_rootpw set
cd ~/src


# Perl
for m in 
do
	cd $MOPS_HOME/src/perl5/$m
	perl Makefile.PL 
done

# Executables
for i in \
	FieldProximity \
	LinkTracklets \
	FindTracklets \
	OrbitProximity \
	DetectionProximity \
	DTCTL LODCTL PRECOV \
	
do
	echo $i
	cd $MOPS_HOME/src/$i
	make install
done
