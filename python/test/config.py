import unittest
import pprint
pp = pprint.PrettyPrinter(indent=4)

import os
import os.path
config_dir = os.environ.get('MOPS_HOME', '/usr/local/MOPS_DEV')
cluster_cf = os.path.join(config_dir, 'config', 'cluster.cf')
backend_cf = os.path.join(config_dir, 'config', 'backend.cf')
master_cf = os.path.join(config_dir, 'config', 'master.cf')


class TestCreate(unittest.TestCase):
    def testImport(self):
        import MOPS.Config

class TestLoad(unittest.TestCase):
    def testLoad(self):
        import MOPS.Config
        cfg = MOPS.Config.LoadFile(cluster_cf)
        self.assert_(cfg)
        self.assert_(cfg.get('hosts') is not None)

class TestParse(unittest.TestCase):
    def testParse(self):
        import MOPS.Config
        cfg = MOPS.Config.LoadFile(master_cf)
        self.assert_(cfg)
        self.assert_(cfg.get('synth'))
        self.assert_(cfg.get('site')['s2n_config'])

class TestParseString(unittest.TestCase):
    def testParse(self):
        import MOPS.Config
        config_text = file(master_cf).read()
        cfg = MOPS.Config.LoadString(config_text)
        self.assert_(cfg)
        self.assert_(cfg.get('synth'))
        self.assert_(cfg.get('site')['s2n_config'])

    def testQuotedDelimiters(self):
        import MOPS.Config
        config_text = """
foo {
    buz = '[42}'
    baz = "{42]"
}
bar [
    '[42}'
    "{42]"
]
"""
        cfg = MOPS.Config.LoadString(config_text)
        self.assert_(cfg['foo']['buz'] == "[42}")
        self.assert_(cfg['foo']['baz'] == "{42]")
        self.assert_(cfg['bar'][0] == "[42}")
        self.assert_(cfg['bar'][1] == "{42]")

if __name__ == '__main__':
    unittest.main()
