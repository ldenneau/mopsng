#!/usr/bin/env python 
from distutils.core import setup, Extension


setup(name='MOPS.config',
    version='0.1',
    description='MOPS config file read-only interface',
    author='Larry Denneau, Instisute for Astronomy, University of Hawaii',
    author_email='denneau@ifa.hawaii.edu',
    url='http://pan-starrs.ifa.hawaii.edu',
    requires=['MySQLdb', 'cjson'],
    packages = ['MOPS',
                'MOPS.LSD',
                'MOPS.Condor',
                'MOPS.Torque',
                'MOPS.Alerts',
                'MOPS.Alerts.plugins',
                'MOPS.VOEvent',
                ],
    ext_modules = [],
)
