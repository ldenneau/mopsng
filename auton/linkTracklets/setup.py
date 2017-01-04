#!/usr/bin/env python

from distutils.core import setup,Extension
from distutils.command.build_ext import build_ext
from distutils.sysconfig import *
from distutils.command.install import install
from distutils.command.install_data import install_data

setup(
  name = "linktracklets",
  ext_modules=[ 
    Extension("linktracklets", 
              ["linktrackletsmodule.c"], 
              define_macros = [('AMFAST', None),
                               ('__USE_FIXED_PROTOTYPES__', None)],
              include_dirs = ['.', 
                              '..', 
                              '../extra', 
                              '../draw',
                              '../ASL_headers',
                              '../dset',
                              '../utils',
                              '/usr/X11R6/include'],
              library_dirs = ['./Darwin_x86__gcc.fast'],
              libraries = ['linktracklets',
                           'extra',
                           'draw',
                           'ASL_headers',
                           'dset',
                           'utils'])
    ],
  cmdclass = {'build_ext': build_ext}
)