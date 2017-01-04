#!/usr/bin/env python
"""
PS-MOPS Installation Script.
"""
import os
import shutil
import time




# Constants
# Modules/packages to exclude from compilation and install. This includes both
# oftware that is not used anymore but also packages which are build as part
# of the preflight procedure (this case for performance reason only and not
# realy important). All names mut include their path relative to the root src
# directory.
# FIXME: Do not add the '.' dir to EXCLUDE.
EXCLUDE = (os.path.join('.', 'tools', 'template'),
           os.path.join('.', 'JPL', 'ssd_2'),
           os.path.join('.', 'perl5', 'PS', 'MOPS', 'DB'),
           )


# Template for the backup directory names (see cleanupAndArchive())
BKP_TEMPLATE = 'BACKUP_%04d-%02d-%02dT%02d:%02d:%02d'
# Number of letters from the start of BKP_TEMPLATE which are constant.
BKP_TMPL_KN = 7

# Build command for perl modules.
# TODO: Add testing facilities.
# FIXME: Determine arch programmaticaly.
ARCH = 'i386-linux-thread-multi'
MAKEMAKE_BUILD_CMD = 'pushd %s; \
                      perl Makefile.PL PREFIX="%s" \
                                       INSTALLSITELIB="%s" \
                                       INSTALLPRIVLIB=%s/' + ARCH + '; \
                      make install; \
                      popd'
MODBUILD_BUILD_CMD = 'pushd %s; \
                      perl Build.PL --install_base "%s"; \
                      echo "%s"; \
                      echo %s/' + ARCH + '; \
                      ./Build install; \
                      popd'
# FIXME: do a make install here.
MAKE_BUILD_CMD ='pushd %s; \
                 make; \
                 make install; \
                 popd'
BUILD_CMD = {'MakeMake': MAKEMAKE_BUILD_CMD,
             'ModBuild': MODBUILD_BUILD_CMD,
             'make': MAKE_BUILD_CMD}
# Where to ind the fortran compiler.
FORTRAN_BIN_DIR = '/usr/local/lf9562/bin'



def selectBuildCommand(key, srcDir, prefix, libPrefix):
    if(key == 'make'):
        return(BUILD_CMD[key] % (srcDir))
    return(BUILD_CMD[key] % (srcDir, prefix, libPrefix, libPrefix))


def findAndInstallModules(srcRootDir, prefix, libPrefix):
    """
    Install modules from srcRootDir into prefix.
    """

    modules = findModules(srcRootDir)
    for module in modules.keys():
        # FIXME: Handle failures.
        err = buildModule(modules[module], module, prefix, libPrefix)
    return


def buildModule(method, module, prefix, libPrefix):
    """
    Convenience routine.
    """
    buildCMD = selectBuildCommand(method, module, prefix, libPrefix)
    return(os.system(buildCMD))


def findModules(srcRootDir='.', outFormat='dict'):
    """
    Find modules in srcRootDir. Modules are defined, as far as this routine is
    concerned, as directories with either a Makefile.PL, a Build.PL, a
    Makefile or a makefile file in them. Not that modules with a Makefile.PL
    need to be treated differently that those with Build.PL or a Makefile.

    Return
    If outFormat == 'dict':
        a dictionary of perl module directory names and corresponding build
        methods of the form
            {module: build method, }
        wehere build method is either MakeMake or ModBuild.
    if outFormat == 'list':
        a list so that dict(list) is the same output as in the case above.

    Notes: Some comlitaion/linking errors are "normal" in that the CVS tree
    still has some old code no more in use. That old code (e.g. PS/MOPS/DB/*)
    does not have all the dependencies it needs satisfied anymore.
    """
    modules = []
    for entry in os.listdir(srcRootDir):
        entry = os.path.join(srcRootDir, entry)
        if(entry in EXCLUDE):
            continue

        if(not os.path.isdir(entry)):
            continue

        if(os.path.exists(os.path.join(entry, 'Build.PL'))):
            modules.append((entry, 'ModBuild'))
        elif(os.path.exists(os.path.join(entry, 'Makefile.PL'))):
            modules.append((entry, 'MakeMake'))
        elif(os.path.exists(os.path.join(entry, 'Makefile')) or
             os.path.exists(os.path.join(entry, 'makefile'))):
            modules.append((entry, 'make'))
        else:
            # Try and see if there is any module deep inside some of these
            # directories.
            t = findModules(srcRootDir=entry, outFormat='list')
            modules += t
            continue
    if(outFormat == 'list'):
        return(modules)
    return(dict(modules))


def installPerlTestModules(baseDir, perlBaseDir):
    raise(NotImplementedError('This routine has not been implemented yet.'))


def setupEnvironment(baseDir, perlBaseDir):
    """
    The PS-MOPS installation assumes that $MOPS_HOME and PERL5LIB are defined
    and equal to the root installation directory (baseDir) and root perl
    installation directory (perlBaseDir) respectively.
    """
    os.environ['MOPS_HOME'] = baseDir
    os.environ['PERL5LIB'] = perlBaseDir

    # Add the fortran compiler to the PATH.
    os.environ['PATH'] = '%s:%s' % (FORTRAN_BIN_DIR,
                                    os.environ['PATH'])

    # Add $MOPS_HOME/bin to the PATH.
    os.environ['PATH'] = '%s:%s' % (os.path.join(baseDir, 'bin'),
                                    os.environ['PATH'])
    return


def installHeaderFiles(baseDir):
    """
    The PS-MOPS build process assumes header files (currently in ./include) to
    be installed in $MOPS_HOME/include
    """
    for entry in os.listdir('include'):
        if(os.path.isdir(os.path.join('include', entry))):
            shutil.copytree(os.path.join('include', entry),
                            os.path.join(baseDir, 'include', entry))
        else:
            shutil.copy2(os.path.join('include', entry),
                         os.path.join(baseDir, 'include'))
    return


def preflight(srcDir, baseDir, perlBaseDir):
    """
    Some modules have dependencies that are not taken care of. Try and fix
    this.
    """
    # Before doing anything else, better setup the environment and take care
    # of header/include files. The reason being that most modules assume
    # that $MOPS_HOME is defined and look for header files in
    # $MOPS_HOME/include
    setupEnvironment(baseDir, perlBaseDir)

    # Create basic directory structure for all the rest.
    createDirStructure(baseDir)

    # Install header files.
    installHeaderFiles(baseDir)

    # Install the conig package.
    module = os.path.join(srcDir, 'config')
    method = 'make'
    prefix = baseDir
    libPrefix = None
    err = buildModule(method, module, prefix, libPrefix)

    # A lot of modules depend on libmiti to be already nstalled.
    module = os.path.join(srcDir, 'libmiti')
    method = 'make'
    prefix = baseDir
    libPrefix = None
    err = buildModule(method, module, prefix, libPrefix)

    # Many others depend on JPL/ssd_1 to be installed as well.
    module = os.path.join(srcDir, 'JPL', 'ssd_1')
    method = 'make'
    prefix = baseDir
    libPrefix = None
    err = buildModule(method, module, prefix, libPrefix)

    # Some tests require tools/insertSyntheticOrbits to be installed.
    module = os.path.join(srcDir, 'tools', 'insertSyntheticOrbits')
    method = 'make'
    prefix = baseDir
    libPrefix = None
    err = buildModule(method, module, prefix, libPrefix)

    # Some Perl modules require PS::MOPS::DC::Iterator.
    module = os.path.join(srcDir, 'perl5', 'PS', 'MOPS', 'DC', 'Iterator')
    method = 'MakeMake'
    prefix = baseDir
    libPrefix = perlBaseDir
    err = buildModule(method, module, prefix, libPrefix)
    return


def postflight(srcDir, baseDir, perlBaseDir):
    """
    Perform any cleanup action necessary.
    """
    # Make sure that everything in $MOPS_BIN/bin is executable.
    err = os.system('chmod 755 %s' % (os.path.join(baseDir, 'bin', '*')))
    return


def createDirStructure(baseDir):
    """
    Create the usual bin, lib and include directories in baseDir.
    """
    os.mkdir(os.path.join(baseDir, 'bin'))
    os.mkdir(os.path.join(baseDir, 'lib'))
    os.mkdir(os.path.join(baseDir, 'include'))
    os.mkdir(os.path.join(baseDir, 'schema'))
    os.mkdir(os.path.join(baseDir, 'config'))
    os.mkdir(os.path.join(baseDir, 'data'))
    os.mkdir(os.path.join(baseDir, 'var'))
    os.mkdir(os.path.join(baseDir, 'apache'))
    os.mkdir(os.path.join(baseDir, 'subsys'))
    return


def cleanupAndArchive(dir):
    """
    Prepare dir for installation. This means putting whatevere is there is a
    new directory.
    """
    # See what is there.
    if(not os.path.exists(dir)):
        # Nothing to do! Just create dir and exit.
        try:
            os.mkdir(dir)
        except:
            pass
        return
    oldFiles = os.listdir(dir)
    if(not oldFiles):
        # Nothing to do!
        return

    # Create the directory where we will move all pre-existing files/dirs.
    now = time.localtime()
    backupDir = safeMkdir(os.path.join(dir, BKP_TEMPLATE % (now[0],
                                                            now[1],
                                                            now[2],
                                                            now[3],
                                                            now[4],
                                                            now[5])))
    # Move all the other files and directories there.
    for entry in oldFiles:
        entryBaseName = entry
        entry = os.path.join(dir, entry)

        if(os.path.isdir(entry) and
           entryBaseName[:BKP_TMPL_KN] == BKP_TEMPLATE[:BKP_TMPL_KN]):
            continue
        elif(os.path.isdir(entry)):
            shutil.copytree(entry, os.path.join(backupDir, entryBaseName))
            shutil.rmtree(entry)
        elif(not os.path.isdir(entry)):    # a simple else would have been OK.
            shutil.copy2(entry, backupDir)
            os.remove(entry)
    return


def safeMkdir(dir):
    """
    Create directory dir. If that already exists, try and create
    "%s_1" % (dir). If that does not work either increase the counter and try
    again. Return the name of the directory created.
    """
    # FIXME: really implement safeMkdir.
    os.mkdir(dir)
    return(dir)

# def install(baseDir, perlBaseDir=os.path.join(baseDir, 'lib', 'perl5')):
# Argh! once more, Python is not Lisp...
def install(baseDir, perlBaseDir):
    """
    Install the PS-MOPS system from a fresh CVS checkout. Assume installation
    on an empty system with no MOPS component present. If baseDir is not empty
    simply archive the old copy and write the new system on it.

    Throw exception if anything fails.
    """
    # Constants
    ROOT_DIR = '.'



    # Prepare baseDir and perlBaseDir for installation.
    cleanupAndArchive(baseDir)
    cleanupAndArchive(perlBaseDir)

    # We can probably infer perlBaseDir from baseDir.
    if(perlBaseDir == None):
        perlBaseDir=os.path.join(baseDir, 'lib', 'perl5')

    # Do any other preparatory steps.
    preflight(ROOT_DIR, baseDir, perlBaseDir)

    # The installation of a new MOPS system happens in stanges. We first
    # install per modules present in ./perl5
    findAndInstallModules(srcRootDir=ROOT_DIR,
                          prefix=baseDir,
                          libPrefix=perlBaseDir)

    # After the main perl modules are in place, install the per test modules.
    # TODO: Install Perl Test modules.
    # installPerlTestModules(baseDir, perlBaseDir)

    # Any cleanup or finishing touches.
    postflight(ROOT_DIR, baseDir, perlBaseDir)

    # Exit
    return(0)



if(__name__ == '__main__'):
    import getopt
    import sys


    # Constants
    USAGE = 'install.py [--mops-home=<path>] [--perl5lib=<path>]'

    # Grab comman line arguments
    opts, args = getopt.getopt(sys.argv[1:], '', ['mops-home=',
                                                  'perl5lib='])

    # Set defualt values for MOPSHome and perl5Lib to either None or, if
    # possible, from $MOPS_HOME and $PERL%LIB respectively.
    try:
        MOPSHome = os.environ['MOPS_HOME']
    except:
        MOPSHome = None
    try:
        perl5Lib = os.environ['PERL5LIB']
    except:
        perl5Lib = None

    # Parse command line arguments.
    for key, val in opts:
        if(key == '--mops-home'):
            MOPSHome = val
        elif(key == '--perl5lib'):
            perl5Lib = val
        else:
            sys.stderr.write('Warning: flag %s not supported. Ignored.' \
                % (key))

    # Sanity check (minimal).
    if(MOPSHome == None or perl5Lib == None):
        sys.stderr.write('usage: %s\n' % (USAGE))
        sys.exit(1)

    # Invoke the main routine and exit.
    err = install(baseDir=MOPSHome, perlBaseDir=perl5Lib)
    sys.exit(err)
# <-- end if













