import unittest

from wsutils import *

class TestCase(unittest.TestCase):
    def testCamels(self):
        self.assertEquals(camelCase('studlyCaps'), 'StudlyCaps')

if __name__=='__main__':
    unittest.main()
