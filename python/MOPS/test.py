#!/usr/bin/env python
"""
Main unit-test entry point. All the unit-tests for Python modules defined in
this directory are invoked here.

Requirements/assumptions
1. The environment variable $MOPS_HOME has to be defined.
2. The environment variable $MOPS_DBINSTANCE will be defined by the script,
   overwriting with the value 'psmops_unittest_${USER}
3. The SQL file with the initial database schema and data has to be present and
   has to be called 'test.sql' (unless otherwise edited in the code below).

Usage
shell> ./test.py
"""
import os
import random
import subprocess
import time
import unittest

from MOPS.Instance import Instance
from DerivedObject import *
from Detection import *
from Field import *
from Orbit import *
from Tracklet import *




# Constants (customize here if you know what you are doing).
DB_NAME = 'psmops_unittest_%s' %(os.environ['USER'])
SQL_FILE = 'test.sql'



# Routines
def init(dbName=DB_NAME, sqlFileName=SQL_FILE):
    """
    Simply setup the whole database schema and populate it with initial data
    from sqlFileName.
    """
    # give the user to interrupt the whole thing.
    print('TEST: we are about to ERASE %s' %(dbName))
    print('TEST: you have 5 seconds to CTR-C and abort the tests.')
    time.sleep(5)

    # Get a hold of MOPS_HOME.
    mopsHome = os.environ['MOPS_HOME']

    # Now, set the $MOPS_DBINSTANCE variable to dbName.
    os.environ['MOPS_DBINSTANCE'] = dbName

    # Setup some basic directory structure.
    _initDir(os.path.join(mopsHome, 'var', dbName))

    # Get a MOPS Instance and logger.
    mopsInstance = Instance(dbName)
    logger = mopsInstance.getLogger()
    

    # Create a new MOPS database instance. We do it the hardcore way by
    # invoking create_psmops directly.
    cwd = os.getcwd()
    os.chdir(os.path.join(mopsHome, 'schema'))

    p = subprocess.Popen(args=['./create_psmops', dbName],
                         stdout=subprocess.PIPE,
                         stderr=subprocess.PIPE)
    err = p.wait()

    os.chdir(cwd)
    
    # Did the DB creation go OK?
    if(err):
        # Nope :-(
        raise(Exception('MOPS %s database creation failed.' %(dbName)))
    # <-- end if

    # Now populate the DB with initial data. To do that we have to figure out
    # the DB host name. Instance does that for us.
    dbHost = mopsInstance.db_hostname
    dbUser = mopsInstance.db_username
    dbPass = mopsInstance.db_password

    # Insert the data, again the hardcore way of calling mysql directly :-(
    f = file(sqlFileName)
    p = subprocess.Popen(args=['mysql',
                               '-h', dbHost,
                               '-u', dbUser,
                               '-p%s' %(dbPass),
                               dbName],
                         stdin=f)
    err = p.wait()
    f.close()

    # Did this one go OK?
    if(err):
        # Nope :-(
        raise(Exception('MOPS %s database population failed.' %(dbName)))
    # <-- end if

    # Well, looks like we are done.
    return


def shutdown(dbName=DB_NAME, sqlFileName=SQL_FILE):
    """
    Just cleanup the DB.
    """
    # Simply drop the DB with initial data. To do that we have to figure out
    # the DB host name. Instance does that for us.
    mopsInstance = Instance(dbName)
    dbHost = mopsInstance.db_hostname
    dbUser = mopsInstance.db_username
    dbPass = mopsInstance.db_password

    # Insert the data, again the hardcore way of calling mysql directly :-(
    p = subprocess.Popen(args=['mysqladmin',
                               '-h', dbHost,
                               '-u', dbUser,
                               '-p%s' %(dbPass),
                               'drop',
                               dbName])
    err = p.wait()

    # Did this one go OK?
    if(err):
        # Nope :-(
        raise(Exception('MOPS %s database drop failed.' %(dbName)))
    # <-- end if

    # Well, looks like we are done.
    return


def _initDir(root):
    if(os.path.exists(root)):
        print('TEST: removing old %s' %(root))
        p = subprocess.Popen(args=['rm',
                                   '-rf', root])
        err = p.wait()
        if(err):
            raise(Exception('Removing of %s failed.' %(root)))
        # <-- end if
    # <-- end if
    
    print('TEST: creating new directory structure.')
    os.mkdir(root)
    os.mkdir(os.path.join(root, 'log'))
    return



# DerivedObject
class DerivedObjectTest(unittest.TestCase):
    def setUp(self):
        """
        Just create a DerivedObject instance from data in the DB. We will use
        that instance in our tests.
        """
        self._dbName = os.environ['MOPS_DBINSTANCE']
        self._instance = mopsInstance = Instance(self._dbName)
        self._conn = self._instance.get_dbh()
        self._cursor = self._conn.cursor()

        sql = 'select d.derivedobject_id, d.object_name ' + \
              'from derivedobjects d'
        n = self._cursor.execute(sql)
        if(not n):
            raise(Exception('No DerivedObjects in the %s database.' \
                            %(self._dbName)))
        # <-- end if

        # Create a DO with a dummy orbit.
        (_id, name) = self._cursor.fetchone()
        
        o = Orbit(None, None, None, None, None, None, None, None, None)
        self._obj = DerivedObject(_id, name, o)
        return
    
    def testFetchTracklets(self):
        """
        Look at the number of tracklets that make up this object. Then invoke
        its fetchTracklets() method and compare results.
        """
        sql = 'select t.tracklet_id from tracklets t, ' + \
              'derivedobject_attrib da where ' + \
              't.tracklet_id = da.tracklet_id and ' + \
              'da.derivedobject_id = %d' %(self._obj._id)
        n = self._cursor.execute(sql)
        if(not n):
            raise(Exception('DerivedObject %d has no tracklets in %s' \
                            %(self._obj._id, self._dbName)))
        # <-- end if
        trueTrackletIds = [row[0] for row in self._cursor.fetchall()]
        trueTrackletIds.sort()
        
        # Now see if this work.
        self._obj.fetchTracklets(self._conn)
        trackletIds = [t._id for t in self._obj.tracklets]
        trackletIds.sort()

        assert(len(trackletIds), len(trueTrackletIds))
        for i in range(len(trackletIds)):
            assert(trackletIds[i], trueTrackletIds[i])
        # <-- end for
        return

    def testArcLength(self):
        """
        Look the the arc length in days of self._obj and compare it with what we
        get from self._obj.arcLength()
        """
        sql = 'select max(d.epoch_mjd) - min(d.epoch_mjd) from ' + \
              'detections d, derivedobject_attrib da, tracklet_attrib ta ' + \
              'where d.det_id = ta.det_id and ' + \
              'ta.tracklet_id = da.tracklet_id and ' + \
              'da.derivedobject_id = %d' %(self._obj._id)
        
        n = self._cursor.execute(sql)
        if(not n):
            raise(Exception('DerivedObject %d has no detections in %s' \
                            %(self._obj._id, self._dbName)))
        # <-- end if
        trueArcLength = self._cursor.fetchone()[0]

        assert(self._obj.arcLength(self._conn), trueArcLength)
        return

    def testUpdateOrbit(self):
        """
        This test is a bit different from the other two: we create a dummy Orbit
        instance, we pass it to self._obj.updateOrbit. Then we fetch it from the
        DB and compare the results.
        """
        # Fetch the current highest orbit id.
        src=[1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21]
        trueOrbit = Orbit(1, 2, 3, 4, 5, 6, 7, 8, 9, src, 12, 13, 14, '15')
        trueOrbit.save(self._conn)

        # Now fetch the orbit from the DB and compare results.
        sql = 'select o.orbit_id, o.q, o.e, o.i, o.node, o.arg_peri, ' + \
              'o.time_peri, o.h_v, o.epoch, ' + \
              'o.cov_01, o.cov_02, o.cov_03, o.cov_04, o.cov_05, o.cov_06, ' + \
              'o.cov_07, o.cov_08, o.cov_09, o.cov_10, o.cov_11, o.cov_12, ' + \
              'o.cov_13, o.cov_14, o.cov_15, o.cov_16, o.cov_17, o.cov_18, ' + \
              'o.cov_19, o.cov_20, o.cov_21, ' + \
              'o.residual, o.chi_squared, o.moid_1, o.conv_code ' + \
              'from orbits o where o.orbit_id = %d' % (trueOrbit._id)
        n = self._cursor.execute(sql)
        assert(n, 1)

        row = list(self._cursor.fetchone())
        newOrbit = Orbit(*(row[:9] + [row[9:30]] + row[30:]))

        # Compare!
        assert(str(newOrbit), str(trueOrbit))
        return


# Detection
class DetectionTest(unittest.TestCase):
    pass


# Field
class FieldTest(unittest.TestCase):
    def setUp(self):
        """
        Just create a Field instance from data in the DB. We will use
        that instance in our tests. We choose a field with tracklets associated
        to it.
        """
        self._dbName = os.environ['MOPS_DBINSTANCE']
        self._instance = mopsInstance = Instance(self._dbName)
        self._conn = self._instance.get_dbh()
        self._cursor = self._conn.cursor()

        sql = 'select f.field_id, f.epoch_mjd, f.ra_deg, f.dec_deg, ' + \
              'f.survey_mode, f.time_start, f.time_stop, f.filter_id, ' + \
              'f.limiting_mag, f.ra_sigma, f.dec_sigma, f.obscode, f.de1, ' + \
              'f.de2, f.de3, f.de4, f.de5, f.de6, f.de7, f.de8, f.de9, ' + \
              'f.de10, f.ocnum, f.status, f.parent_id from `fields` f, ' + \
              'tracklets t where t.field_id = f.field_id'
        n = self._cursor.execute(sql)
        if(not n):
            raise(Exception('No Fields in the %s database.' \
                            %(self._dbName)))
        # <-- end if

        # Create a Field.
        row = list(self._cursor.fetchone())
        
        self._obj = Field(*(row[:12] + [row[12:22]] + row[22:]))
        return

    def testInsert(self):
        """
        Test the insertion of a dummy Field instance in the DB.
        """
        # Fetch the current highest field id.
        _id = 0
        sql = 'select max(field_id) from `fields`'
        n = self._cursor.execute(sql)
        if(n):
            _id = self._cursor.fetchone()[0] + 1
        # <-- end if

        # Create a dummy field instance.
        trueField = Field(_id, 2, 3, 4, '5', 6, 7, '8', 9, 10, 11, '12',
                          [1, 2, 3, 4, 5, 6, 7, 8, 9, 10], 13, '14', 15)
        trueField.insert(self._conn)

        # Now fetch it from the DB and see what happened.
        sql = 'select f.field_id, f.epoch_mjd, f.ra_deg, f.dec_deg, ' + \
              'f.survey_mode, f.time_start, f.time_stop, f.filter_id, ' + \
              'f.limiting_mag, f.ra_sigma, f.dec_sigma, f.obscode, f.de1, ' + \
              'f.de2, f.de3, f.de4, f.de5, f.de6, f.de7, f.de8, f.de9, ' + \
              'f.de10, f.ocnum, f.status, f.parent_id from `fields` f ' + \
              'where f.field_id = %d' %(_id)
        n = self._cursor.execute(sql)
        assert(n, 1)

        # Create a Field.
        row = list(self._cursor.fetchone())
        testField = Field(*(row[:12] + [row[12:22]] + row[22:]))

        # Compare the two.
        assert(testField, trueField)
        return

    def testFetchTracklets(self):
        """
        Look at the number of tracklets associatedto this field. Then invoke
        its fetchTracklets() method and compare results.
        """
        sql = 'select t.tracklet_id from tracklets t where ' + \
              't.field_id = %d ' %(self._obj._id)
        n = self._cursor.execute(sql)
        if(not n):
            raise(Exception('Field %d has no tracklets in %s' \
                  %(self._obj._id, self._dbName)))
        # <-- end if
        trueTrackletIds = [row[0] for row in self._cursor.fetchall()]
        trueTrackletIds.sort()
        
        # Now see if this work.
        self._obj.fetchTracklets(self._conn)
        trackletIds = [t._id for t in self._obj.tracklets]
        trackletIds.sort()

        assert(len(trackletIds), len(trueTrackletIds))
        for i in range(len(trackletIds)):
            assert(trackletIds[i], trueTrackletIds[i])
        # <-- end for
        return
        

class OrbitTests(unittest.TestCase):
    def setUp(self):
        """
        Just create an Orbit instance from data in the DB. We will use
        that instance in our tests. We choose a field with tracklets associated
        to it.
        """
        self._dbName = os.environ['MOPS_DBINSTANCE']
        self._instance = mopsInstance = Instance(self._dbName)
        self._conn = self._instance.get_dbh()
        self._cursor = self._conn.cursor()

        sql = 'select o.orbit_id, o.q, o.e, o.i, o.node, o.arg_peri, ' + \
              'o.time_peri, o.h_v, o.epoch, o.cov_01, o.cov_02, o.cov_03, ' + \
              'o.cov_04, o.cov_05, o.cov_06, o.cov_07, o.cov_08, o.cov_09, ' + \
              'o.cov_10, o.cov_11, o.cov_12, o.cov_13, o.cov_14, o.cov_15, ' + \
              'o.cov_16, o.cov_17, o.cov_18, o.cov_19, o.cov_20, o.cov_21, ' + \
              'o.residual, o.chi_squared, o.moid_1, o.conv_code from orbits o'
        n = self._cursor.execute(sql)
        if(not n):
            raise(Exception('No Orbits in the %s database.' \
                            %(self._dbName)))
        # <-- end if

        # Create an Orbit.
        row = list(self._cursor.fetchone())
        
        self._obj = Orbit(*(row[:9] + [row[9:30]] + row[30:]))
        return

    def testSave(self):
        """
        Test the insertion of a dummy Orbit instance in the DB.
        """
        # Fetch the current highest field id.
        # Create a dummy orbit instance.
        src=[1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21]
        trueOrbit = Orbit(1, 2, 3, 4, 5, 6, 7, 8, 9, src, 10, 11, 12, '13')
        trueOrbit.save(self._conn)

        # Now fetch it from the DB and see what happened.
        sql = 'select o.orbit_id, o.q, o.e, o.i, o.node, o.arg_peri, ' + \
              'o.time_peri, o.h_v, o.epoch, o.cov_01, o.cov_02, o.cov_03, ' + \
              'o.cov_04, o.cov_05, o.cov_06, o.cov_07, o.cov_08, o.cov_09, ' + \
              'o.cov_10, o.cov_11, o.cov_12, o.cov_13, o.cov_14, o.cov_15, ' + \
              'o.cov_16, o.cov_17, o.cov_18, o.cov_19, o.cov_20, o.cov_21, ' + \
              'o.residual, o.chi_squared, o.moid_1, o.conv_code ' + \
              'from orbits o where o.orbit_id = %d' %(trueOrbit._id)
        n = self._cursor.execute(sql)
        assert(n, 1)

        # Create an Orbit.
        row = list(self._cursor.fetchone())
        testOrbit = Orbit(*(row[:9] + [row[9:30]] + row[30:]))

        # Compare the two.
        assert(testOrbit, trueOrbit)
        return


class TrackletTests(unittest.TestCase):
    def setUp(self):
        """
        Just create an Orbit instance from data in the DB. We will use
        that instance in our tests. We choose a field with tracklets associated
        to it.
        """
        self._dbName = os.environ['MOPS_DBINSTANCE']
        self._instance = mopsInstance = Instance(self._dbName)
        self._conn = self._instance.get_dbh()
        self._cursor = self._conn.cursor()

        sql = 'select tracklet_id, v_ra, v_dec, v_tot, ' + \
              'v_ra_sigma, v_dec_sigma, pos_ang_deg, ' + \
              'gcr_arcsec, ext_epoch, ext_ra, ext_dec, ext_mag, ' + \
              'probability, digest, ' + \
              'status, known_id, field_id from tracklets'
        n = self._cursor.execute(sql)
        if(not n):
            raise(Exception('No Tracklets in the %s database.' \
                            %(self._dbName)))
        # <-- end if

        # Create a Field.
        row = list(self._cursor.fetchone())
        
        self._obj = Tracklet(*row)
        return

    def testFetchDetections(self):
        """
        Look at the number of detections associatedto this tracklet. Then invoke
        its fetchDetections() method and compare results.
        """
        sql = 'select ta.det_id from tracklet_attrib ta where ' + \
              'ta.tracklet_id = %d ' %(self._obj._id)
        n = self._cursor.execute(sql)
        if(not n):
            raise(Exception('Tracklet %d has no detections in %s' \
                  %(self._obj._id, self._dbName)))
        # <-- end if
        trueDetIds = [row[0] for row in self._cursor.fetchall()]
        trueDetIds.sort()
        
        # Now see if this work.
        detIds = self._obj.fetchDetections(self._conn)
        detIds = [t._id for t in self._obj.detections]
        detIds.sort()

        assert(len(detIds), len(trueDetIds))
        for i in range(len(trueDetIds)):
            assert(detIds[i], trueDetIds[i])
        # <-- end for
        return


class ResidualTests(unittest.TestCase):
    def setUp(self):
        """
        """
        self._dbName = os.environ['MOPS_DBINSTANCE']
        self._instance = mopsInstance = Instance(self._dbName)
        self._conn = self._instance.get_dbh()
        self._cursor = self._conn.cursor()
        return

    def testStoreResiduals(self):
        """
        """
        detId = 1 
        trackletId = 1
        raResid_deg = .1
        decResid_deg = .11
        raError_deg = .01
        decError_deg = .011
        astReject = 0

        foo = Residuals(detId, trackletId, raResid_deg, decResid_deg, raError_deg, decError_Deg, astReject)
        foo.insert()
        return
        

if(__name__ == '__main__'):
    init()
    unittest.main()
    shutdown()
