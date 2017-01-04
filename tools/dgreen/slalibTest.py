#!/usr/bin/env python

import slalib as S
import numpy as N
import numpy.testing as T
import math, unittest

class TestSLALIBFunctions(unittest.TestCase):

    def testdat(self):
        T.assert_almost_equal(S.sla_dat(50083), 30, 0, 'sla_dat, 50083')
        T.assert_almost_equal(S.sla_dat(50630), 31, 0, 'sla_dat, 50630')
        T.assert_almost_equal(S.sla_dat(51179), 32, 0, 'sla_dat, 51179')
        T.assert_almost_equal(S.sla_dat(53736), 33, 0, 'sla_dat, 53736')
        T.assert_almost_equal(S.sla_dat(54832), 34, 0, 'sla_dat, 54832')
        T.assert_almost_equal(S.sla_dat(56109), 35, 0, 'sla_dat, 56109')
    #<-- end def
#<-- end class

if(__name__ == '__main__'):
    unittest.main()
#<-- end if    