#!/usr/bin/env python
"""
Simple script to build and install the auton sources.
"""
import glob
import optparse
import os
import sys
import shutil



# Constants
#SUBDIRS = ['ASL_headers', 'draw', 'dset', 'extra', 'autonutils',
#           'fieldProximity', 'findTracklets', 'linkTracklets', 'orbitProximity',
#           'astroclean', 'detectionproximity',
#           'python']
SUBDIRS = [ 'autonutils', 'fieldProximity', 'findTracklets', 'linkTracklets', 'orbitProximity',
           'astroclean', 'detectionproximity', 'python']
MFLAGS = 't=fast'
USAGE_STR = 'usage: build.py all|clean|install [--prefix PREFIX]\n'
DEF_PREFIX =  os.environ.get('MOPS_HOME', '/usr/local/auton')



# Expceptions
class MakeError(Exception): pass



# Parse command line args
parser = optparse.OptionParser(USAGE_STR)
parser.add_option('--prefix',
                  dest='prefix',
                  type='string',
                  help="specify installation root directory")
(options, argv) = parser.parse_args()

# Determine the target.
TARGETS = ['all', 'clean', 'install']
if(len(argv) == 0):
    target = 'all'
elif(len(argv) > 1 or argv[0] not in TARGETS):
    parser.error('non supported target. Choose from %s.' %(TARGETS))
else:
    target = argv[0]
# <-- end if

# Determine prefix
if(not options.prefix):
    prefix = DEF_PREFIX
else:
    prefix = options.prefix
# <-- end if


# Figure out what the build directories are going to be called.
# Except for the python module, all other products are stored in a directory 
# called OS_ARCH*.X where X is what follows t= in MFLAGS and defaults to debug.
(OS, nodename, release, version, ARCH) = os.uname()
if('t=' not in MFLAGS):
    TYPE = 'debug'
else:
    TYPE = MFLAGS.split('t=')[1].split()[0]
# <-- end if
# productDir = '%s_%s*.%s' %(OS, ARCH, TYPE)
productDir = '%s_*.%s' %(OS, TYPE)


# Run the make command in each directory.
for d in SUBDIRS:
    sys.stdout.write('making %s in %s...\n' %(target, d))

    # cd to that directory and do a make there.
    os.chdir(d)

    # Treat install separately.
    if d == 'python' and target == 'install':
        sys.stderr.write('Please build the Python extension manually using setup.py install --home=$MOPS_HOME')
    elif d != 'python' and target == 'install':
        if(OS == 'Darwin'):
            libs = glob.glob(os.path.join(productDir, '*.a')) + \
                   glob.glob(os.path.join(productDir, '*.dylib'))
        else:
            libs = glob.glob(os.path.join(productDir, '*.a')) + \
                   glob.glob(os.path.join(productDir, '*.so'))
        # <-- end if
        includes = glob.glob('*.h')
        exe = glob.glob(os.path.join(productDir, d))
        
        # Copy the files into their locations.
        for lib in libs:
            shutil.copy(lib, os.path.join(prefix, 'lib'))
        for inc in includes:
            shutil.copy(inc, os.path.join(prefix, 'include'))
        if(exe and os.path.exists(exe[0])):
            shutil.copy(exe[0], os.path.join(prefix, 'bin'))
        # <-- end if
    else:
        err = os.system('make %s %s' %(MFLAGS, target))
        if(err):
            raise(MakeError('make %s %s in %s failed.' %(MFLAGS, target, d)))
        # <-- end if
    # <-- end if
    
    # Go back to where we were.
    sys.stdout.write('done.\n')
    os.chdir('..')
# <-- end for
