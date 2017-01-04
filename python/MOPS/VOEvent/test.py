import unittest
import pprint
pp = pprint.PrettyPrinter(indent=4)


class TestVOEvent(unittest.TestCase):
    def setUp(self):
        return

    def testImport(self):
        import MOPS.VOEvent as VOEvent
        return

    def testParse(self):
        import MOPS.VOEvent as VOEvent
        import xml.etree.ElementTree as ET
        import glob

        for fileName in glob.glob('test/test_*.xml'):
            xml = open(fileName).read()
            voevent = VOEvent.VOEvent.parse(xml)

            self.assertEqual(ET.tostring(ET.fromstring(xml)),
                             ET.tostring(ET.fromstring(str(voevent))))
        return

    def testMalformedParse(self):
        import MOPS.VOEvent as VOEvent

        xml = open('test/malformed.txt').read()
        self.assertRaises(SyntaxError, VOEvent.VOEvent.parse, xml)
        return

    def testParseAndCreate(self):
        import MOPS.VOEvent as VOEvent
        import glob

        for fileName in glob.glob('test/test_*.xml'):
            voevent = VOEvent.VOEvent.parse(open(fileName).read())
            new_voevent = VOEvent.VOEvent(voevent.instanceName,
                                          voevent.objectId,
                                          voevent.orbit,
                                          voevent.detections,
                                          voevent.ruleName)
            self.assert_(new_voevent)
        return

    def testParseCreateAndCompare(self):
        import MOPS.VOEvent as VOEvent
        import glob

        for fileName in glob.glob('test/test_*.xml'):
            voevent = VOEvent.VOEvent.parse(open(fileName).read())
            new_voevent = VOEvent.VOEvent(voevent.instanceName,
                                          voevent.objectId,
                                          voevent.orbit,
                                          voevent.detections,
                                          voevent.ruleName,
                                          voevent.eventTime,
                                          voevent.ephemerides)

            self.assertEqual(str(voevent), str(new_voevent))
        return

    def testParseCreateAndCompareToText(self):
        import MOPS.VOEvent as VOEvent
        import glob

        for fileName in glob.glob('test/test_*.xml'):
            voevent = VOEvent.VOEvent.parse(open(fileName).read())
            new_voevent = VOEvent.VOEvent(voevent.instanceName,
                                          voevent.objectId,
                                          voevent.orbit,
                                          voevent.detections,
                                          voevent.ruleName,
                                          voevent.eventTime,
                                          voevent.ephemerides)

            self.assertEqual(voevent.totext(), new_voevent.totext())
        return
        
        



if(__name__) == '__main__':
    unittest.main()
