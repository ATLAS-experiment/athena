#! /usr/bin/env python

# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

## @Package test_trfArgClasses.py
#  @brief Unittests for test_trfArgClasses.py
#  @author graeme.andrew.stewart@cern.ch
#  @note Tests of ATLAS specific file formats (that thus rely on other
#  parts of Athena) live here

import os
import unittest

from AthenaConfiguration.TestDefaults import defaultTestFiles
from PyJobTransforms.trfArgClasses import argFile, argBSFile, argPOOLFile

# Stripped down key list for files which are inputs 
from PyJobTransforms.trfFileUtils import athFileInterestingKeys

class argFileEOSTests(unittest.TestCase):
    def test_SimGlobStar(self):
        hitsDir = os.path.dirname(defaultTestFiles.HITS_RUN3[0])
        hitsInputs = argFile(os.path.join(hitsDir, '*HITS.*'), io='input')
        self.assertGreater(len(hitsInputs.value), 0)

    def test_SimGlobMatchSingle(self):
        pat = defaultTestFiles.HITS_RUN3[0].replace('pool','????')
        hitsInputs = argFile(pat, io='input')
        self.assertGreater(len(hitsInputs.value), 0)

    def test_SimGlobMatchBoth(self):
        pat = defaultTestFiles.HITS_RUN3[0].replace('pool','????').replace('root','*')
        hitsInputs = argFile(pat, io='input')
        self.assertGreater(len(hitsInputs.value), 0)


class argPOOLFiles(unittest.TestCase):
    def test_argPOOLFileMetadata_ESD(self):
        testFile = defaultTestFiles.ESD_RUN3_MC[0]

        esdFile = argPOOLFile(testFile, io = 'input', type='esd')
        self.assertEqual(esdFile.getMetadata(metadataKeys = tuple(athFileInterestingKeys)),
                         {testFile: {'file_type': 'pool', 'file_guid': '0CE34C1C-660F-174E-B514-E532495EAB0F',
                                     'nentries': 100, 'file_size': 406282752}})

        esdFile = argPOOLFile(testFile, io = 'output', type='esd')
        self.assertEqual(esdFile.getMetadata(),
                         {testFile: {'file_type': 'pool', 'file_guid': '0CE34C1C-660F-174E-B514-E532495EAB0F',
                                     'nentries': 100, 'file_size': 406282752, 'integrity': True, '_exists': True, }})

        self.assertEqual(esdFile.getMetadata(metadataKeys = ('nentries',)),
                         {testFile: {'nentries': 100}})
        self.assertEqual(esdFile.prodsysDescription['type'], 'file')

    def test_argPOOLFileMetadata_AOD(self):
        testFile = defaultTestFiles.AOD_RUN3_DATA[0]

        aodFile = argPOOLFile(testFile, io = 'input', type='aod')
        self.assertEqual(aodFile.getMetadata(metadataKeys = tuple(athFileInterestingKeys)),
                         {testFile: {'file_type': 'pool', 'file_guid': '8B8A7970-C2B2-5E49-89AB-05540465451E',
                                     'nentries': 1000, 'file_size': 221977230}})

        aodFile = argPOOLFile(testFile, io = 'output', type='aod')
        self.assertEqual(aodFile.getMetadata(),
                         {testFile: {'file_type': 'pool', 'file_guid': '8B8A7970-C2B2-5E49-89AB-05540465451E',
                                     'nentries': 1000, 'file_size': 221977230, 'integrity': True, '_exists': True, }})

        self.assertEqual(aodFile.getMetadata(metadataKeys = ('nentries',)),
                         {testFile: {'nentries': 1000}})
        self.assertEqual(aodFile.prodsysDescription['type'], 'file')
        self.assertEqual(aodFile.prodsysDescription['subtype'], 'AOD')


class argBSFiles(unittest.TestCase):
    def test_argBSFileMetadata(self):
        testFile = defaultTestFiles.RAW_RUN3_DATA25[0]
        rawFile = argBSFile(testFile, io = 'input', type='bs')
        self.assertEqual(rawFile.getMetadata(),
                         {testFile: {'file_type': 'bs', 'file_guid': '5EAA1F7E-7ABA-F011-9EC3-B8CEF6D36C6E',
                                     'nentries': 135, 'file_size': 229691428, 'integrity': True, '_exists': True, }})

        self.assertEqual(rawFile.getMetadata(metadataKeys = ('nentries',)),
                         {testFile: {'nentries': 135}})
        self.assertEqual(rawFile.prodsysDescription['type'], 'file')

    def test_argBSMultiFileMetadata(self):
        testFiles = [defaultTestFiles.RAW_RUN3_DATA24_HI[0],
                     defaultTestFiles.RAW_RUN3_DATA25[0]]

        rawFile = argBSFile(testFiles, io = 'input', type = 'bs')
        self.assertEqual(rawFile.getMetadata(),
                         {testFiles[0]: {'file_type': 'bs', 'file_guid': 'FE00F02A-1FA8-EF11-B2FB-3CECEF0D9A2E',
                                         'nentries': 110, 'file_size': 146784144, 'integrity': True, '_exists': True},
                          testFiles[1]: {'file_type': 'bs', 'file_guid': '5EAA1F7E-7ABA-F011-9EC3-B8CEF6D36C6E',
                                         'nentries': 135, 'file_size': 229691428, 'integrity': True, '_exists': True}})

        self.assertEqual(rawFile.getMetadata(metadataKeys = ('nentries',)),
                         {testFiles[0]: {'nentries': 110},
                          testFiles[1]: {'nentries': 135}})

        self.assertEqual(rawFile.getMetadata(metadataKeys = ('nentries',), files = testFiles[1]),
                         {testFiles[1]: {'nentries': 135}})

        self.assertEqual(rawFile.prodsysDescription['type'], 'file')


if __name__ == '__main__':
    unittest.main()
