#!/usr/bin/env python
"""
Run a simulation given some user parameters.
"""

# Imports
import os
from install import BKP_TEMPLATE, cleanupAndArchive


# Constants
IOD_PROGRAM = {'orsa': '"mops_iod"',
               'find_orb': '"iod.x --max_resid 10 --method gh"'}



# Interfaces (by means of decorators).
# Non assigned instance variables are not supported.
# Optional instance variables are not supported.
# All of the instance variables have to be defined.
def require(*REQUIRED):
    def decorator(func):
        def wrapper(*__argv, **__kw):
            assert len(__argv) != 0, \
                   'Non assigned instance variables are not supported.'
            assert list(REQUIRED).sort() == __kw.keys().sort(), \
                   'All of the instance variables have to be defined.'
            return(func(*__argv, **__kw))
        wrapper.__name__ = func.__name__
        wrapper.__dict__ = func.__dict__
        wrapper.__doc__ = func.__doc__
        return(wrapper)
    return(decorator)


# Globals
class Config(object):
    def __init__(self, *argv, **keywords):
        for arg in argv:
            setattr(self, key, None)
        for key in keywords.keys():
            setattr(self, key, keywords[key])
        return

    def __repr__(self):
        repr = '%s {\n' % (self.__class__.__name__)
        repr += reduce(lambda x, y: ' %s\n %s' %(x, y),
                       ['%s = %s' % (str(var), str(getattr(self, var))) \
                        for var in self.__dict__])
        repr += '\n}'
        return(repr)

# TODO: Try and avoid so much code duplication. Metaclasses?
class ssm(Config):
    @require('exposure_time_s',
             'field_size_deg2',
             'add_astrometric_error',
             'add_false_detections',
             'add_shape_mag')
    def __init__(self, *argv, **keywords):
        return(super(ssm, self).__init__(*argv, **keywords))

class findtracklets(Config):
    @require('maxv_degperday',
             'minobs',
             'extended',
             'maxt')
    def __init__(self, *argv, **keywords):
        return(super(findtracklets, self).__init__(*argv, **keywords))

class dtctl(Config):
    @require('attribution_resid_threshold_arcsec',
             'attribution_proximity_threshold_arcsec')
    def __init__(self, *argv, **keywords):
        return(super(dtctl, self).__init__(*argv, **keywords))

class lodctl(Config):
    @require('iod_threshold_arcsec',
             'diffcor_threshold_arcsec',
             'fallback_iod',
             'max_link_days',
             'min_nights',
             'slow_minv_degperday',
             'slow_maxv_degperday',
             'slow_grouped_radius_deg',
             'fast_minv_degperday',
             'fast_maxv_degperday',
             'fast_grouped_radius_deg',
             'allow_multiple_attributions',
             'iod_program')
    def __init__(self, *argv, **keywords):
        return(super(lodctl, self).__init__(*argv, **keywords))

class linktracklets(Config):
    @require('slow_link_opts',
             'fast_link_opts')
    def __init__(self, *argv, **keywords):
        return(super(linktracklets, self).__init__(*argv, **keywords))

class debug(Config):
    @require('force_empty_attribution_list',
             'disable_attributions',
             'disable_precovery',
             'disable_orbit_identification',
             'disable_orbit_determination')
    def __init__(self, *argv, **keywords):
        return(super(debug, self).__init__(*argv, **keywords))

class site(Config):
    @require('obscode',
             'gmt_offset_hours',
             'limiting_mag',
             'fwhm_arcseconds',
             'astrometric_error_arcseconds',
             's2n')
    def __init__(self, *argv, **keywords):
        return(super(site, self).__init__(*argv, **keywords))


# Exception
class DBMSException(Exception):
    pass



def setupEnvironment(mopsDBInstance):
    """
    Several component in MOPS assume that $MOPS_HOME and MOPS_DBINSTANCE are
    defined. Here we simply export MOPS_DBINSTANCE, as the other one should
    already be defined (and if not we are in bigger troubles anyway).
    """
    os.environ['MOPS_DBINSTANCE'] = mopsDBInstance
    return



def createDirStructure(mopsHome, mopsDBInstance):
    """
    MOPS requires a particular directory structure in order for it to work.
    """
    # Determine the root level directory that we need to create.
    baseDir = os.path.join(mopsHome, 'var', mopsDBInstance)
    cleanupAndArchive(baseDir)

    # Create baseDir and sub directories (we are garanteed by
    # cleanupAndArchive() that baseDir exists).
    os.mkdir(os.path.join(baseDir, 'config'))
    os.mkdir(os.path.join(baseDir, 'bin'))
    os.mkdir(os.path.join(baseDir, 'log'))
    return(baseDir)


def populateDatabase(mopsDBInstance):
    """
    Every MOPS run is asociated with a DB instance, meaning a DBMS database
    with empty tables to support MOPS processing.
    """
    # TODO: interface with the DBMS directly.
    DIR = os.path.join(os.environ['MOPS_HOME'], 'schema')
    EXE = './create_psmops'
    if(os.system('pushd %s; %s %s; popd' % (DIR, EXE, mopsDBInstance))):
        raise(DBMSException('Fatal error in populating the database %s.') \
            % (mopsDBInstance))
    return


def addSSMObjects(mopsDBInstance, model):
    """
    Once the database instance has been created, Solar System Model (SSM)
    Objects can be added to it. These provide MOPS with a control sample of
    known transients.
    """
    DIR = os.path.join(os.environ['MOPS_HOME'],
                       'data',
                       'ssm',
                       'orbits',
                       model)
    EXE = 'rm -f done*; make insert'
    if(os.system('pushd %s; %s; popd' % (DIR, EXE))):
        raise(DBMSException('Fatal error in adding SSM objects to %s.') \
            % (mopsDBInstance))
    return


def addObservations():
    """
    MOPS uses records in the Fields table (part of $MOPS_DBINSTANCE database)
    to determine the observation strategy for the survey. Fill that table up
    with LSST fields.
    """
    DIR = os.path.join(os.environ['HOME'], 'LSST')
    EXE = 'perl insertLSSTFields --obscode 807 ' + \
                                '--limiting_mag 24 ' + \
                                '--start_date 54009 ' + \
                                '--exp_time 31 ' + \
                                '--exp_sep 1 ' + \
                                '--proposal 68 ' + \
                                '--proposal 69 ' + \
                                '--instance %s ' + \
                                'session28cadence' % (mopsDBInstance)
    if(os.system('pushd %s; %s; popd' % (DIR, EXE))):
        raise(DBMSException('Fatal error in adding fields to %s.') \
            % (mopsDBInstance))
    return


def createConfigFiles(baseDir, masterConfigData):
    """
    MOPS expects a master config file (master.cf), a description file and a
    version file to be in
    @param baseDir / config

    It also expects empty mops.log, dtctl.log, lodctl.log, precovery.log files
    to be in
    @param baseDir / log
    """
    configDir = os.path.join(baseDir, 'config')
    logDir = os.path.join(baseDir, 'log')

    # Create an empty version file (needed by the web interface).
    vers = file(os.path.join(configDir, 'version'), 'w')
    vers.close()

    # Extract the name of the DB nstance from baseDir.
    instanceName = os.path.basename(baseDir)

    # Write it into baseDir/config/description
    desc = file(os.path.join(configDir, 'description'), 'w')
    desc.write('%s\n' % (instanceName))
    desc.close()

    # Create the master.cf comfig file.
    conf = file(os.path.join(configDir, 'master.cf'), 'w')
    conf.write('%s\n' % (masterConfigData))
    conf.close()

    # Touch the log files.
    for name in ('mops.log', 'dtctl.log', 'lodctl.log', 'precovery.log'):
        log = file(os.path.join(logDir, name), 'w')
        log.close()
    return


def run(mopsDBInstance, masterConfigData):
    """
    Main entry point.
    """
    mopsHome = os.environ['MOPS_HOME']

    setupEnvironment(mopsDBInstance)
    instanceDir = createDirStructure(mopsHome, mopsDBInstance)
    createConfigFiles(instanceDir, masterConfigData)
    populateDatabase(mopsDBInstance)
    addSSMObjects(mopsDBInstance, model='1000')
    return


if(__name__ == '__main__'):
    # TODO: Make this interactive.
    ssmConfig = ssm(exposure_time_s=15.,
                    field_size_deg2=9.6,
                    add_astrometric_error=1,
                    add_false_detections=0,
                    add_shape_mag=0)
    findtrackletsConfig = findtracklets(maxv_degperday=2.,
                                        minobs=2,
                                        extended=1,
                                        maxt=0.001)
    dtctlConfig = dtctl(attribution_resid_threshold_arcsec=0.30,
                        attribution_proximity_threshold_arcsec=600)
    lodctlConfig = lodctl(iod_threshold_arcsec=10,
                          diffcor_threshold_arcsec=0.40,
                          fallback_iod=30,
                          max_link_days=15,
                          min_nights=3,
                          slow_minv_degperday=0.,
                          slow_maxv_degperday=0.5,
                          slow_grouped_radius_deg=4,
                          fast_minv_degperday=0.4,
                          fast_maxv_degperday=7,
                          fast_grouped_radius_deg=8,
                          allow_multiple_attributions=1,
                          iod_program = IOD_PROGRAM['find_orb'])
    linktrackletsConfig = linktracklets(slow_link_opts='"vtree_thresh 0.0004 pred_thresh 0.0008"',
                                        fast_link_opts='"vtree_thresh 0.0030 pred_thresh 0.0050"')
    siteConfig = site(obscode=807,
                      gmt_offset_hours=-4,
                      limiting_mag=25,
                      fwhm_arcseconds=0.7,
                      astrometric_error_arcseconds=0.05,
                      s2n=5.0)
    debugConfig = debug(force_empty_attribution_list=0,
                        disable_attributions=0,
                        disable_precovery=0,
                        disable_orbit_identification=0,
                        disable_orbit_determination=0)

    configText = '%s\n%s\n%s\n%s\n%s\n%s\n%s\n' % (str(ssmConfig),
                                                   str(findtrackletsConfig),
                                                   str(dtctlConfig),
                                                   str(lodctlConfig),
                                                   str(linktrackletsConfig),
                                                   str(siteConfig),
                                                   str(debugConfig))

    run(mopsDBInstance=os.environ['MOPS_DBINSTANCE'], masterConfigData=configText)
# <-- end if















