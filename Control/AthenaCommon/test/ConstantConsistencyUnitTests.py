#!/usr/bin/env python

# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

"""Unit tests for verifying the consistency of SystemOfUnits and PhysicalConstants between Athena and Gaudi."""

import unittest, sys

from types import ModuleType
def attrs(module):
   """Get all public attributes of module (without sub-modules)"""
   return {k:v for k,v in vars(module).items() if not (k.startswith('__') or isinstance(v, ModuleType))}


class ConsistencyOfConstantsTestCase( unittest.TestCase ):

   def testSystemOfUnits( self ):
      """Consistency of unit defintions"""
      import AthenaCommon.SystemOfUnits
      import GaudiKernel.SystemOfUnits
      self.maxDiff = None  # no limit to diff length
      self.assertDictEqual(attrs(AthenaCommon.SystemOfUnits),
                     attrs(GaudiKernel.SystemOfUnits))

   def testPhysicalConstants( self ):
      """Consistency of unit defintions"""
      import AthenaCommon.PhysicalConstants
      import GaudiKernel.PhysicalConstants
      self.maxDiff = None  # no limit to diff length
      self.assertDictEqual(attrs(AthenaCommon.PhysicalConstants),
                     attrs(GaudiKernel.PhysicalConstants))

if __name__ == '__main__':
   unittest.main()
