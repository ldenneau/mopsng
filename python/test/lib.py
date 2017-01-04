from __future__ import division

import unittest
import pprint
pp = pprint.PrettyPrinter(indent=4)

import os
import os.path
import re
import sys
import math

import MOPS.Constants
from MOPS.Lib import *

class TestV2Filt(unittest.TestCase):
    def testFilt2V(self):
        mag = 22.0
        self.assertAlmostEqual(filt2v(mag, 'V'), 22.0, msg='filt2v(V)')
        self.assertAlmostEqual(filt2v(mag, ' '), 22.0, msg='filt2v( )')
        self.assertAlmostEqual(filt2v(mag, 'w'), 21.5, msg='filt2v(g)')
        self.assertAlmostEqual(filt2v(mag, 'g'), 21.5, msg='filt2v(g)')
        self.assertAlmostEqual(filt2v(mag, 'r'), 21.9, msg='filt2v(r)')
        self.assertAlmostEqual(filt2v(mag, 'i'), 22.3, msg='filt2v(i)')
        self.assertAlmostEqual(filt2v(mag, 'z'), 21.9, msg='filt2v(z)')
        self.assertAlmostEqual(filt2v(mag, 'y'), 22.0, msg='filt2v(z)')

    def testV2Filt(self):
        mag = 22.0
        self.assertAlmostEqual(v2filt(mag, 'V'), 22.0, msg='v2filt(V)')
        self.assertAlmostEqual(v2filt(mag, ' '), 22.0, msg='v2filt( )')
        self.assertAlmostEqual(v2filt(mag, 'w'), 22.5, msg='v2filt(g)')
        self.assertAlmostEqual(v2filt(mag, 'g'), 22.5, msg='v2filt(g)')
        self.assertAlmostEqual(v2filt(mag, 'r'), 22.1, msg='v2filt(r)')
        self.assertAlmostEqual(v2filt(mag, 'i'), 21.7, msg='v2filt(i)')
        self.assertAlmostEqual(v2filt(mag, 'z'), 22.1, msg='v2filt(z)')
        self.assertAlmostEqual(v2filt(mag, 'y'), 22.0, msg='v2filt(z)')

class TestClassifications(unittest.TestCase):
    def testClassifyClean(self):
        names = ['S1'] * 6
        self.assert_(classifyNames(names) == MOPS.Constants.MOPS_EFF_CLEAN)

    def testClassifyBad(self):
        names = ['S1'] * 4 + [None] * 2
        self.assert_(classifyNames(names) == MOPS.Constants.MOPS_EFF_BAD)
        names = ['S1'] * 4 + [None] * 2 + ['S2'] * 4
        self.assert_(classifyNames(names) == MOPS.Constants.MOPS_EFF_BAD)

    def testClassifyMixed(self):
        names = ['S1'] * 6 + ['S2'] * 4
        self.assert_(classifyNames(names) == MOPS.Constants.MOPS_EFF_MIXED)

    def testClassifyNonsynthetic(self):
        names = [None] * 6
        self.assert_(classifyNames(names) == MOPS.Constants.MOPS_EFF_NONSYNTHETIC)
        names = [''] * 6
        self.assert_(classifyNames(names) == MOPS.Constants.MOPS_EFF_NONSYNTHETIC)
        names = [''] * 4 + [None] * 2
        self.assert_(classifyNames(names) == MOPS.Constants.MOPS_EFF_NONSYNTHETIC)
        names = ['NS'] * 6
        self.assert_(classifyNames(names) == MOPS.Constants.MOPS_EFF_NONSYNTHETIC)
        names = ['NS'] * 4 + [None] * 2
        self.assert_(classifyNames(names) == MOPS.Constants.MOPS_EFF_NONSYNTHETIC)

class TestSphericalDistance(unittest.TestCase):
    def test1(self):
        pos1 = [0.0, 0.0]
        pos2 = [0.0, 0.1]
        self.assertAlmostEqual(sphericalDistance_arcsec(pos1, pos2), 0.1 * MOPS.Constants.ARCSECONDS_PER_DEG)

    def test2(self):
        pos1 = [0.0, 0.0]
        pos2 = [0.0, -0.1]
        self.assertAlmostEqual(sphericalDistance_arcsec(pos1, pos2), 0.1 * MOPS.Constants.ARCSECONDS_PER_DEG)

    def test3(self):
        pos1 = [0.0, 0.0]
        pos2 = [0.1, 0.0]
        self.assertAlmostEqual(sphericalDistance_arcsec(pos1, pos2), 0.1 * MOPS.Constants.ARCSECONDS_PER_DEG)

    def test1(self):
        pos1 = [0.0, 0.0]
        pos2 = [-0.1, 0.0]
        self.assertAlmostEqual(sphericalDistance_arcsec(pos1, pos2), 0.1 * MOPS.Constants.ARCSECONDS_PER_DEG)


class TestFMOD(unittest.TestCase):
    epsilon = 1e-10

    def chk(self, a, b):
        return abs(a - b) < self.epsilon

    def test1(self):
        self.assert_(self.chk(MOPS.Lib._fmod(1, 1), 0))               # 1 + 0 = 1
        self.assert_(self.chk(MOPS.Lib._fmod(5, 3), 2))               # 3 + 2 = 5
        self.assert_(self.chk(MOPS.Lib._fmod(5.3, 3), 2.3))           # 3 + 2.3 = 5.3
        self.assert_(self.chk(MOPS.Lib._fmod(371.5, 360), 11.5))      # 360 + 11.5 = 371.5
        self.assert_(self.chk(MOPS.Lib._fmod(-1, 1), 0))              # -1 + 1 = 0
        self.assert_(self.chk(MOPS.Lib._fmod(-5, 3), 1))              # -6 + 1 = -5
        self.assert_(self.chk(MOPS.Lib._fmod(-5.3, 3), 0.7))          # -6 + 0.7 = -5.3
        self.assert_(self.chk(MOPS.Lib._fmod(-371.5, 360), 348.5))    # -720 + 348.5 = -371.5


class TestNormalizeRADEC(unittest.TestCase):
    epsilon = 1e-10

    def chk(self, pos, pos_target):
        ra = pos[0]
        dec = pos[1]
        tra = pos_target[0]
        tdec = pos_target[1]
        return abs(ra - tra) < self.epsilon and abs(dec - tdec) < self.epsilon

    def test1(self):
        self.assert_(self.chk(normalizeRADEC(0.1, 0.1), (0.1, 0.1)))
        self.assert_(self.chk(normalizeRADEC(90.1, 0.1), (90.1, 0.1)))
        self.assert_(self.chk(normalizeRADEC(180.1, 0.1), (180.1, 0.1)))
        self.assert_(self.chk(normalizeRADEC(270.1, 0.1), (270.1, 0.1)))
        self.assert_(self.chk(normalizeRADEC(360.1, 0.1), (0.1, 0.1)))
        self.assert_(self.chk(normalizeRADEC(50.1, 45), (50.1, 45)))
        self.assert_(self.chk(normalizeRADEC(360 + 50.1, 45), (50.1, 45)))
        self.assert_(self.chk(normalizeRADEC(720 + 50.1, 45), (50.1, 45)))
        self.assert_(self.chk(normalizeRADEC(-360 + 50.1, 45), (50.1, 45)))
        self.assert_(self.chk(normalizeRADEC(-720 + 50.1, 45), (50.1, 45)))
        self.assert_(self.chk(normalizeRADEC(50.1, 360 + 45), (50.1, 45)))
        self.assert_(self.chk(normalizeRADEC(50.1, 720 + 45), (50.1, 45)))
        self.assert_(self.chk(normalizeRADEC(50.1, -360 + 45), (50.1, 45)))
        self.assert_(self.chk(normalizeRADEC(50.1, -720 + 45), (50.1, 45)))
        self.assert_(self.chk(normalizeRADEC(50.1, 90 + 45), (180 + 50.1, 45)))
        self.assert_(self.chk(normalizeRADEC(50.1, -45), (50.1, -45)))
        self.assert_(self.chk(normalizeRADEC(50.1, -90 - 45), (180 + 50.1, -45)))
        self.assert_(self.chk(normalizeRADEC(50.1, 90 + 45), (180 + 50.1, 45)))

        
class TestDANG(unittest.TestCase):
    """
    Test various angle manipulations to ensure we wrap correctly.
    """
    def test1(self):
        """ basic """
        self.assert_(dang(1, 5) == -4)

        """ basic wraparound """
        self.assert_(dang(1, 5) == -4)
        self.assert_(dang(5, 1) == 4)
        self.assert_(dang(-3, -5) == 2)
        self.assert_(dang(355, 10) == -15)
        self.assert_(dang(10, 355) == 15)
        self.assert_(dang(355, -10) == 5)
        self.assert_(dang(-10, 355) == -5)
        self.assert_(dang(359, -2) == 1)

        """ multiples of 360 """
        self.assert_(dang(-3, -5) == 2)
        self.assert_(dang(355 + 360, 10) == -15)
        self.assert_(dang(355 + 720, 10) == -15)
        self.assert_(dang(10 - 360, 355) == 15)
        self.assert_(dang(10 - 720, 355) == 15)
        self.assert_(dang(355 + 360, -10 - 720) == 5)
        self.assert_(dang(720, -360) == 0)


class testValidOrbit(unittest.TestCase):
    """
    Test whether we validate orbits (idiot-check-style) correctly.
    """

    def test1(self):
        lines = """\
PASS 2.606465812689 0.16876388479 7.8620110364 262.529646578809 196.153689710529 54034.0444531623 16.309537970215 54973
PASS 2.606465812689 0.16876388479 0.0000000000 262.529646578809 196.153689710529 54034.0444531623 16.309537970215 54973
PASS 2.606465812689 0.16876388479 179.99999990 262.529646578809 196.153689710529 54034.0444531623 16.309537970215 54973
PASS 2.606465812689 0.00000000000 7.8620110364 262.529646578809 196.153689710529 54034.0444531623 16.309537970215 54973
FAIL 0.000000000000 0.16876388479 7.8620110364 262.529646578809 196.153689710529 54034.0444531623 16.309537970215 54973
FAIL -2.60646581268 0.16876388479 7.8620110364 262.529646578809 196.153689710529 54034.0444531623 16.309537970215 54973
FAIL 2.606465812689 -3.0000000000 7.8620110364 262.529646578809 196.153689710529 54034.0444531623 16.309537970215 54973
FAIL 2.606465812689 0.16901000000 180.00010364 262.529646578809 196.153689710529 54034.0444531623 16.309537970215 54973
FAIL 2.606465812689 -3.0000388479 7.8620110364 262.529646578809 196.153689710529 54034.0444531623 16.309537970215 54973
FAIL 2.606465812689 176571538    .516876388479 262.529646578809 196.153689710529 54034.0444531623 16.309537970215 54973
FAIL 2.606465812689 -3.0000388479 7.8620110364 -262.529646578809 196.153689710529 54034.0444531623 16.309537970215 54973
FAIL 2.606465812689 176571538    .516876388479 1262.529646578809 196.153689710529 54034.0444531623 16.309537970215 54973
FAIL 2.606465812689 -3.0000388479 7.8620110364 262.529646578809 -196.153689710529 54034.0444531623 16.309537970215 54973
FAIL 2.606465812689 176571538    .516876388479 262.529646578809 1196.153689710529 54034.0444531623 16.309537970215 54973\
"""
        class foo:
            pass

        for line in lines.split("\n"):
            buz = foo()
            vals = line.split()
            buz.orbitId = vals[0]
            buz.q, buz.e, buz.i, buz.node, buz.argPeri, buz.timePeri, buz.mag, buz.epoch = map(lambda x: float(x), vals[1:])
            if re.match(r'^PASS', buz.orbitId):
                self.assert_(validOrbit(buz))
            else:
                self.assert_(not validOrbit(buz))

class testFormatTimingMsg(unittest.TestCase):
    """
    Test formatting of timing messages.
    """

    def test1(self):
        str1 = "TIMING DTCTL 345.600 127"
        str2 = "TIMING DTCTL/FOO 345.600 127 # comment"
        str3 = "TIMING DTCTL 123.400 0"

        self.assert_(formatTimingMsg(subsystem='DTCTL', time_sec=345.6, nn=127) == str1)
        self.assert_(formatTimingMsg(subsystem='DTCTL', subsubsystem='FOO', time_sec=345.6, nn=127, comment='comment') == str2)
        self.assert_(formatTimingMsg(subsystem='DTCTL', time_sec=123.4, nn=0) == str3)
        self.assert_(formatTimingMsg(subsystem='DTCTL', time_sec=123.4, nn=None) == str3)

class testXYIdx(unittest.TestCase):
    """
    Test XY indexing of detections.
    """

    def test1(self):
        class MockField(object):
            def __init__(self, ra_deg, dec_deg, fov_deg, xyidx_size):
                self.ra = ra_deg
                self.dec = dec_deg
                self.FOV_deg = fov_deg
                self.xyidxSize = xyidx_size

        class MockDetection(object):
            def __init__(self, ra_deg, dec_deg):
                self.ra = ra_deg
                self.dec = dec_deg

        fra_deg = 20
        fdec_deg = 20
        field_area_deg2 = 7
        fov_deg = math.sqrt(field_area_deg2 / math.pi) * 2
        half_fov_deg = fov_deg / 2
        field = MockField(fra_deg, fdec_deg, fov_deg, 5)
        size = field.xyidxSize

        self.assert_(computeXYIdx(field, fra_deg + 0, fdec_deg + 0, fov_deg) == int(size * size / 2))

        self.assert_(computeXYIdx(field, fra_deg + 0, fdec_deg + half_fov_deg, fov_deg) == size * size - size + int(size / 2))
        self.assert_(computeXYIdx(field, fra_deg + 0, fdec_deg - half_fov_deg, fov_deg) == int(size / 2))
        self.assert_(computeXYIdx(field, fra_deg - half_fov_deg, fdec_deg + 0, fov_deg) == int(size / 2) * size)
        self.assert_(computeXYIdx(field, fra_deg + half_fov_deg, fdec_deg + 0, fov_deg) == int(size / 2) * size + size - 1)

        self.assert_(computeXYIdx(field, fra_deg - half_fov_deg, fdec_deg + half_fov_deg, fov_deg) == size * size - size)
        self.assert_(computeXYIdx(field, fra_deg + half_fov_deg, fdec_deg + half_fov_deg, fov_deg) == size * size - 1)
        self.assert_(computeXYIdx(field, fra_deg + half_fov_deg, fdec_deg - half_fov_deg, fov_deg) == size - 1)
        self.assert_(computeXYIdx(field, fra_deg - half_fov_deg, fdec_deg - half_fov_deg, fov_deg) == 0)

class testInField(unittest.TestCase):
    """
    Test inField() routine.
    """

    def test1(self):
        class MockField(object):
            def __init__(self, ra_deg, dec_deg, fov_deg, xyidx_size):
                self.ra = ra_deg
                self.dec = dec_deg
                self.FOV_deg = fov_deg
                self.xyidxSize = xyidx_size

        field_area_deg2 = 7         # radius = 1.49...
        fov_deg = math.sqrt(field_area_deg2 / math.pi) * 2
        pos_err_deg = .1

        fra_deg = 20
        fdec_deg = 20
        field = MockField(fra_deg, fdec_deg, fov_deg, 5)
        self.assert_(inField(field, 'circle', fra_deg + 1.2, fdec_deg, pos_err_deg))
        self.assert_(not inField(field, 'circle', fra_deg + 2.0, fdec_deg, pos_err_deg))

        fra_deg = -20
        fdec_deg = 0
        field = MockField(fra_deg, fdec_deg, fov_deg, 5)
        self.assert_(inField(field, 'circle', fra_deg + 1.48, fdec_deg, pos_err_deg))   # in FOV
        self.assert_(inField(field, 'circle', fra_deg + 1.58, fdec_deg, pos_err_deg))   # in FOV+err
        self.assert_(not inField(field, 'circle', fra_deg + 1.61, fdec_deg, pos_err_deg))   # outside FOV+err

class testDCriterion(unittest.TestCase):
    """
    Test D-criterion calculations.
    """

    class MockOrbit(object):
        def __init__(self, q, e, i, node, argPeri, timePeri, hV, epoch):
            self.q = q
            self.e = e
            self.i = i
            self.node = node
            self.argPeri = argPeri
            self.timePeri = timePeri
            self.hV = hV
            self.epoch = epoch

    def setUp(self):
        self.orb1 = self.MockOrbit(
            q=1.252068, 
            e=0.382, 
            i=9.3, 
            node=252.1, 
            argPeri=185.6,
            timePeri=53015.0711320161,
            hV=10.315,
            epoch=52860,
        )
        self.orb2 = self.MockOrbit(
            q=0.74812,
            e=0.528,
            i=10.2,
            node=201.6,
            argPeri=31.8,
            timePeri=52787.5192209622,
            hV=18.064,
            epoch=52860,
        )
        self.orb3 = self.MockOrbit(
            q=0.74812,
            e=0.528,
            i=10.2,
            node=179.9999,
            argPeri=179.9999,
            timePeri=52787.5192209622,
            hV=18.064,
            epoch=52860,
        )
        self.orb4 = self.MockOrbit(
            q=0.74812,
            e=0.528,
            i=10.2,
            node=180.0001,
            argPeri=180.0001,
            timePeri=52787.5192209622,
            hV=18.064,
            epoch=52860,
        )

    def testSame(self):
        d3, d4 = calculateDCriterion(self.orb1, self.orb1)
        self.assertAlmostEqual(d3, 0, 5, 'same object D3')    # 5 => decimals
        self.assertAlmostEqual(d4, 0, 5, 'same object D4')    # 5 => decimals

    def testBasic(self):
        ref_d3 = 0.5443853748
        ref_d4 = 1.043862275
        d3, d4 = calculateDCriterion(self.orb1, self.orb2)
        self.assertAlmostEqual(d3, ref_d3, 5, 'ref D3')    # 5 => decimals
        self.assertAlmostEqual(d4, ref_d4, 5, 'ref D4')    # 5 => decimals

    def test180(self):
        self.orb3.argPeri = -0.00001
        self.orb4.argPeri = +0.00001
        d3, d4 = calculateDCriterion(self.orb3, self.orb4)
        self.assertAlmostEqual(d3, 0, 5) 
        self.assertAlmostEqual(d4, 0, 5) 

    def testArgPeriZero(self):
        self.orb3.argPeri = -0.00001
        self.orb4.argPeri = +0.00001
        d3, d4 = calculateDCriterion(self.orb3, self.orb4)
        self.assertAlmostEqual(d3, 0, 5) 
        self.assertAlmostEqual(d4, 0, 5) 

    def testNodeZero(self):
        self.orb3.node = -0.00001
        self.orb4.node = +0.00001
        d3, d4 = calculateDCriterion(self.orb3, self.orb4)
        self.assertAlmostEqual(d3, 0, 5) 
        self.assertAlmostEqual(d4, 0, 5) 


if __name__ == '__main__':
    import pdb
    pdb.set_trace()
    unittest.main()
