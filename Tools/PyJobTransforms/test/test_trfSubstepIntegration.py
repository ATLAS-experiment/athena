#! /usr/bin/env python

# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

## @Package test_substepIntegration.py
#  @brief Test substep import works properly for common compound transforms
#  @author atlas-comp-transforms-dev@cern.ch

import re
import subprocess
import unittest

import logging
msg = logging.getLogger(__name__)



## Unittests for this module
class trfExitTests(unittest.TestCase):
    
    def test_Reco_tf(self):
        cmd = ['Reco_tf.py', '--showSteps']
        msg.info('Will run this transform: {0}'.format(cmd))
        steps = []
        p = subprocess.run(cmd, capture_output = True, text = True)
        for line in p.stdout.splitlines():
            print(line, end=' ')
            m = re.match(r"Executor Step: (\w+)", line)
            if m:
                steps.append(m.group(1))
        self.assertEqual(p.returncode, 0)
        self.assertTrue("HITtoRDO" in steps)
        self.assertTrue("RAWtoALL" in steps)

    def test_FullChain_tf(self):
        cmd = ['FullChain_tf.py', '--showSteps']
        msg.info('Will run this transform: {0}'.format(cmd))
        steps = []
        p = subprocess.run(cmd, capture_output = True, text = True)
        for line in p.stdout.splitlines():
            print(line, end=' ')
            m = re.match(r"Executor Step: (\w+)", line)
            if m:
                steps.append(m.group(1))
        self.assertEqual(p.returncode, 0)
        self.assertTrue("EVNTtoHITS" in steps)
        self.assertTrue("HITtoRDO" in steps)
        self.assertTrue("RAWtoALL" in steps)
        
if __name__ == '__main__':
    unittest.main()
    
