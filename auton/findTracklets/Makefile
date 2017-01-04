# Makefile for dirlib ~/g/findtracklets
# Created by jkubica on:  Thu Sep 11 15:24:27 EDT 2003

here		= findTracklets

includes        = tracklet_mht.h findtrackletsapi.h gcf.h d2model.h digest2.h

sources         = tracklet_mht.c findtrackletsapi.c gcfmath.c d2model.c digest2.c d2mpc.c

private_sources = 

# Warning: Make sure the siblings are ordered so that, for example, if
#          a depends on b then b is to the right of a in the following list

siblings	= linkTracklets autonutils

t:=debug
include ../gmake-magic/Make.common

