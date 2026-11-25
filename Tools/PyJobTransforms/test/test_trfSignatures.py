#! /usr/bin/env python

# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Test generation of transform signatures
#

import os
import subprocess
import unittest

from PyJobTransforms.trfLogger import msg

class SignatureTest(unittest.TestCase):
    
    def tearDown(self):
        for f in 'test.json', :
            try:
                os.unlink(f)
            except OSError:
                pass
    
    def test_Signatures(self):
        cmd = ['makeTrfJSONSignatures.py',  '--output', 'test.json']
        msg.info('Will generate transform signatures: {}'.format(cmd))
        p = subprocess.run(cmd)
        self.assertEqual(p.returncode, 0)

if __name__ == '__main__':
    unittest.main()
