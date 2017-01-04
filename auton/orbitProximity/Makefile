# Makefile for dirlib ~/g/orbitproximity
# Created by jkubica on:  Tue Apr 19 16:06:24 EDT 2005

here		= orbitProximity

includes	= orbit.h orbprox.h orbit_tree.h

sources		= orbit.c orbprox.c orbit_tree.c

private_sources = 

# Warning: Make sure the siblings are ordered so that, for example, if
#          a depends on b then b is to the right of a in the following list

siblings	= linkTracklets autonutils

t:=debug
include ../gmake-magic/Make.common

