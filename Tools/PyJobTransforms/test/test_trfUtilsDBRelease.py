#! /usr/bin/env python

# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Test the various DBRelease scenarios
#

import subprocess
import os
import os.path
import shutil
import unittest

from PyJobTransforms.trfExitCodes import trfExit as trfExit

from PyJobTransforms.trfLogger import msg

class DBReleasetest(unittest.TestCase):
    
    # Standard setup using CVMFS
    def test_cvmfsStandard(self):
        cmd = ['Athena_tf.py', '--CA', '--DBRelease', '23.3.1']
        msg.info('Will run this transform: {0}'.format(cmd))
        p = subprocess.run(cmd)
        self.assertEqual(p.returncode, 0)
    
    # Setup using CVMFS 'current'
    def test_cvmfsCurrent(self):
        cmd = ['Athena_tf.py', '--CA', '--DBRelease', 'current']
        msg.info('Will run this transform: {0}'.format(cmd))
        p = subprocess.run(cmd)
        self.assertEqual(p.returncode, 0)
    
    # Test using a DBRelease file which exists
    def test_tarball(self):
        try:
            os.symlink('/path/to/nonexistent/DBRelease-22.7.1.tar.gz', 'DBRelease-22.7.1.tar.gz')
        except FileExistsError:  # link already exists
            pass

        cmd = ['Athena_tf.py', '--CA', '--DBRelease', 'DBRelease-22.7.1.tar.gz']
        msg.info('Will run this transform: {0}'.format(cmd))
        p = subprocess.run(cmd)
        self.assertEqual(p.returncode, 0)

    # Negative test - use an illegal name format
    def test_illegalName(self):
        cmd = ['Athena_tf.py', '--CA', '--DBRelease', 'FailMeHarder']
        msg.info('Will run this transform: {0}'.format(cmd))
        p = subprocess.run(cmd, stdout = subprocess.PIPE, stderr = subprocess.STDOUT, text=True)
        print(p.stdout, end="")
        self.assertEqual(p.returncode, trfExit.nameToCode('TRF_DBRELEASE_PROBLEM'))
        self.assertRegex(p.stdout, 'Unable to interpret DBRelease')
        
def tearDownModule():
    if os.path.isdir('DBRelease'):
        shutil.rmtree('DBRelease')

if __name__ == '__main__':
    unittest.main()
