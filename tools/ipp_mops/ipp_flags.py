#!/usr/bin/env python

import sys

IPP_FLAGS = []
IPP_FLAGS2 = []
FLAG_REMOVAL = 0x3888

class Flag:
    def __init__(self, name, value, comment):
        self.name = name
        self.value = value
        self.comment = comment
    @staticmethod
    def init_flags():
        IPP_FLAGS.append(Flag("PSFMODEL", 	0x00000001, 	"Source fitted with a psf model (linear or non-linear)"))
        IPP_FLAGS.append(Flag("EXTMODEL", 	0x00000002, 	"Source fitted with an extended-source model"))
        IPP_FLAGS.append(Flag("FITTED", 	0x00000004, 	"Source fitted with non-linear model (PSF or EXT; good or bad)"))
        IPP_FLAGS.append(Flag("FITFAIL", 	0x00000008, 	"Fit (non-linear) failed (non-converge, off-edge, run to zero)"))
        IPP_FLAGS.append(Flag("POORFIT", 	0x00000010, 	"Fit succeeds, but low-SN, high-Chisq, or large (for PSF -- drop?)"))
        IPP_FLAGS.append(Flag("PAIR", 	        0x00000020, 	"Source fitted with a double psf"))
        IPP_FLAGS.append(Flag("PSFSTAR",	0x00000040, 	"Source used to define PSF model"))
        IPP_FLAGS.append(Flag("SATSTAR", 	0x00000080, 	"Source model peak is above saturation"))
        IPP_FLAGS.append(Flag("BLEND", 	        0x00000100, 	"Source is a blend with other sourcers"))
        IPP_FLAGS.append(Flag("EXTERNALPOS", 	0x00000200, 	"Source based on supplied input position"))
        IPP_FLAGS.append(Flag("BADPSF", 	0x00000400, 	"Failed to get good estimate of object's PSF"))
        IPP_FLAGS.append(Flag("DEFECT", 	0x00000800, 	"Source is thought to be a defect"))
        IPP_FLAGS.append(Flag("SATURATED", 	0x00001000, 	"Source is thought to be saturated pixels (bleed trail)"))
        IPP_FLAGS.append(Flag("CR_LIMIT", 	0x00002000, 	"Source has crNsigma above limit"))
        IPP_FLAGS.append(Flag("EXT_LIMIT",	0x00004000, 	"Source has extNsigma above limit"))
        IPP_FLAGS.append(Flag("MOMENTS_FAILURE",0x00008000, 	"could not measure the moments"))
        IPP_FLAGS.append(Flag("SKY_FAILURE", 	0x00010000, 	"could not measure the local sky"))
        IPP_FLAGS.append(Flag("SKYVAR_FAILURE", 0x00020000, 	"could not measure the local sky variance"))
        IPP_FLAGS.append(Flag("MOMENTS_SN", 	0x00040000, 	"moments not measured due to low S/N"))
        IPP_FLAGS.append(Flag("",	        0x00080000, 	"this bit is not defined"))
        IPP_FLAGS.append(Flag("BIG_RADIUS", 	0x00100000, 	"poor moments for small radius, try large radius"))
        IPP_FLAGS.append(Flag("AP_MAGS", 	0x00200000, 	"source has an aperture magnitude"))
        IPP_FLAGS.append(Flag("BLEND_FIT", 	0x00400000, 	"source was fitted as a blend"))
        IPP_FLAGS.append(Flag("EXTENDED_FIT", 	0x00800000, 	"full extended fit was used"))
        IPP_FLAGS.append(Flag("EXTENDED_STATS", 0x01000000, 	"extended aperture stats calculated"))
        IPP_FLAGS.append(Flag("LINEAR_FIT", 	0x02000000, 	"source fitted with the linear fit"))
        IPP_FLAGS.append(Flag("NONLINEAR_FIT", 	0x04000000, 	"source fitted with the non-linear fit"))
        IPP_FLAGS.append(Flag("RADIAL_FLUX", 	0x08000000, 	"radial flux measurements calculated"))
        IPP_FLAGS.append(Flag("SIZE_SKIPPED", 	0x10000000, 	"Warning:: if set, size could be determined"))
        IPP_FLAGS.append(Flag("ON_SPIKE", 	0x20000000, 	"peak lands on diffraction spike"))
        IPP_FLAGS.append(Flag("ON_GHOST",	0x40000000, 	"peak lands on ghost or glint"))
        IPP_FLAGS.append(Flag("OFF_CHIP", 	0x80000000, 	"peak lands off edge of chip"))

        IPP_FLAGS2.append(Flag("DIFF_WITH_SINGLE", 	0x00000001, 	"diff source matched to a single positive detection"))
        IPP_FLAGS2.append(Flag("DIFF_WITH_DOUBLE", 	0x00000002, 	"diff source matched to positive detections in both images"))
        IPP_FLAGS2.append(Flag("MATCHED", 	0x00000004, 	"source was supplied at this location from somewhere else (eg, another image, forced photometry location, etc)"))
        IPP_FLAGS2.append(Flag("ON_SPIKE", 	0x00000008, 	"> 25% of (PSF-weighted) pixels land on diffraction spike"))
        IPP_FLAGS2.append(Flag("ON_STARCORE", 	0x00000010, 	"> 25% of (PSF-weighted) pixels land on star core"))
        IPP_FLAGS2.append(Flag("ON_BURNTOOL", 	0x00000020, 	"> 25% of (PSF-weighted) pixels land on burntool subtraction region"))
        IPP_FLAGS2.append(Flag("ON_CONVPOOR", 	0x00000040, 	"> 25% of (PSF-weighted) pixels land on region where convolution had substantial masked fraction contribution"))
        IPP_FLAGS2.append(Flag("PASS1_SRC", 	0x00000080, 	"source was detected in the first pass of psphot (bright detection stage)"))
        IPP_FLAGS2.append(Flag("HAS_BRIGHTER_NEIGHBOR", 	0x00000100, 	"peak is not the brightest in its footprint"))
        IPP_FLAGS2.append(Flag("BRIGHT_NEIGHBOR_1", 	0x00000200, 	"flux_n / (r2 flux_p) > 1"))
        IPP_FLAGS2.append(Flag("BRIGHT_NEIGHBOR_10", 	0x00000400, 	"flux_n / (r2 flux_p) > 10"))
        IPP_FLAGS2.append(Flag("DIFF_SELF_MATCH", 	0x00000800, 	"positive detection match is probably this source"))
        IPP_FLAGS2.append(Flag("SATSTAR_PROFILE", 	0x00001000, 	"saturated source is modeled with a radial profile "))

class Flags:
    @staticmethod
    def what(flag_value):
        int_flag_value = int(flag_value)
        if int_flag_value == 0:
            return
        print "  Flags for %d / 0x%x" % (int_flag_value, int_flag_value)
        for flag in IPP_FLAGS:
            if int_flag_value & flag.value:
                print "    %20s --- %s" % (flag.name, flag.comment)
    @staticmethod
    def has_flags_for_removal(flag_value):
        int_flag_value = int(flag_value)
        if int_flag_value == 0:
            return
        if int_flag_value & FLAG_REMOVAL:
            print "    => Is flagged for removal"
        else:
            print "    => Is not flagged for removal"

class Flags2:
    @staticmethod
    def what(flag_value):
        int_flag_value = int(flag_value)
        if int_flag_value == 0:
            return
        print "  Flags2 for %d / 0x%x" % (int_flag_value, int_flag_value)
        for flag in IPP_FLAGS2:
            if int_flag_value & flag.value:
                print "    %20s --- %s" % (flag.name, flag.comment)
    @staticmethod
    def has_flags_for_removal(flag_value):
        int_flag_value = int(flag_value)
        if int_flag_value == 0:
            return
        print "    => no flag2 is used for removal at the moment"

def usage():
    print
    print "  ipp_flags.py \"<flags1> <flags2>\" ..."
    print "    Interpret the flags and tell if they are used in mops for removal"
    print
    print "  ipp_flags.py -h"
    print "    Show this help"
    print
    print "  Example:"
    print "  shell> ipp_flags.py \"111165447 2178\" \"7635421 30\""
    print "     [...]"
    print 
    print "  The current values which are currently used for removal are:"
    Flags.what(FLAG_REMOVAL)
    print "  No flag2 is used for removal"
    print 

if __name__ == "__main__":
    Flag.init_flags()

    if len(sys.argv) == 1:
        usage()
        sys.exit(1)
    elif sys.argv[1] == "-h":
        usage()
        sys.exit(0)

    for flags in sys.argv[1:]:
        print "############## %s #################" % flags
        (flag1, flag2) = flags.split(" ")
        Flags.what(flag1)
        Flags.has_flags_for_removal(flag1)
        Flags2.what(flag2)
        Flags2.has_flags_for_removal(flag2)
