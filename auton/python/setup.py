#!/usr/bin/env python

from distutils.core import setup,Extension
from distutils.command.build_ext import build_ext
from distutils.sysconfig import *
from distutils.command.install import install
from distutils.command.install_data import install_data

AUTON_DIRS = [
              '../astroclean',
              '../detectionproximity',
              '../fieldProximity',
              '../findTracklets',
              '../linkTracklets',
              '../orbitProximity',
              '../autonutils']

setup(
  name = "auton",
  ext_modules=[ 
    Extension("auton", 
              ["autonmodule.c"], 
              define_macros = [('AMFAST', None),
                               ('__USE_FIXED_PROTOTYPES__', None)],
              include_dirs = AUTON_DIRS + ['/usr/X11R6/include', ],
              library_dirs = AUTON_DIRS,
              libraries = ['linkTracklets',
                           'findTracklets',
                           'fieldProximity',
                           'orbitProximity',
                           'detectionproximity',
                           'autonutils'])
    ],
  cmdclass = {'build_ext': build_ext}
)
