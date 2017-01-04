import unittest
import tempfile
import shutil
import os
import os.path

import pprint
pp = pprint.PrettyPrinter(indent=4)
import MOPS.Lib

DBNAME = 'psmops_unittest'


class TestCreate(unittest.TestCase):
    def testImport(self):
        import MOPS.Instance

class TestInstance(unittest.TestCase):
    def setUp(self):
        import MOPS.Instance
        self.inst = MOPS.Instance.Instance(dbname=DBNAME)

    def testCreate(self):
        self.assert_(self.inst)

    def testEnvironment(self):
        self.assert_(self.inst.environment.get('HOMEDIR'))
        self.assert_(self.inst.environment.get('VARDIR'))
        self.assert_(self.inst.environment.get('CONFIGDIR'))
        self.assert_(self.inst.environment.get('LOGDIR'))
        self.assert_(self.inst.environment.get('NNDIR'))
        self.assert_(self.inst.environment.get('OBJECTSDIR'))

    def testConfig(self):
        self.assert_(self.inst.getConfig())


class TestDirs(unittest.TestCase):
    def setUp(self):
        import MOPS.Instance
        self.inst = MOPS.Instance.Instance(dbname=DBNAME)
        self.tmpdir = tempfile.mkdtemp()

    def testMakeNNDir(self):
        nndir = self.inst.makeNNDir(nn=54200, subsys='foo')
        self.failUnless(os.path.isdir(os.path.join(self.inst.getEnvironment('NNDIR'), "%05d" % 54200, 'foo')),
            msg="couldn't create directory")

    def tearDown(self):
        shutil.rmtree(self.tmpdir)


class TestDB(unittest.TestCase):
    def setUp(self):
        import MOPS.Instance
        self.inst = MOPS.Instance.Instance(dbname=DBNAME)

    def testGetDBH(self):
        self.assert_(self.inst.get_dbh())

    def testNewDBH(self):
        old_dbh = self.inst.get_dbh()
        self.inst.forget_dbh()                  # clear dbh
        self.assert_(self.inst.dbh is None)     # check for it

        new_dbh = self.inst.new_dbh()           # get new one
        self.assert_(new_dbh)
        self.assert_(new_dbh != self.inst.get_dbh()) # check not same as instance's

if __name__ == '__main__':
    import pdb
    pdb.set_trace()
    unittest.main()
