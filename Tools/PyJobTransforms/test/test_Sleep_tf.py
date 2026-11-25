#! /usr/bin/env python

# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Simple test of Sleep_tf.py
#

import json
import subprocess
import unittest

from PyJobTransforms.trfLogger import msg

class Echotest(unittest.TestCase):
    
    def test_runEcho(self):
        cmd = ['Sleep_tf.py', '--dust', '10']
        msg.info('Will run this transform: {0}'.format(cmd))
        p = subprocess.run(cmd)
        self.assertEqual(p.returncode, 0)
        
        # Now load metadata and test a few important values
        with open('jobReport.json') as jr:
            md = json.load(jr)
            self.assertEqual(isinstance(md, dict), True)

if __name__ == '__main__':
    unittest.main()
