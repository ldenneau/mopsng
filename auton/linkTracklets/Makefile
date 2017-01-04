# Makefile for dirlib ~/g/neos
# Created by jkubica on:  Thu Sep 11 15:24:27 EDT 2003

here		= linkTracklets

includes        = neos_header.h obs.h plates.h track.h sb_graph.h t_tree.h \
		  rdvv_tree.h MHT.h plate_tree.h rdt_tree.h linker.h

sources         = obs.c plates.c track.c sb_graph.c t_tree.c \
		  rdvv_tree.c MHT.c plate_tree.c rdt_tree.c linker.c

private_sources = 

# Warning: Make sure the siblings are ordered so that, for example, if
#          a depends on b then b is to the right of a in the following list

siblings	= autonutils

t:=debug
include ../gmake-magic/Make.common

