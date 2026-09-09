#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.MainServicesConfig import MainServicesCfg, MainEvgenServicesCfg

import importlib
import unittest
from unittest import TestCase, skipUnless

_flags = initConfigFlags()

class Test(TestCase):
   """Unit tests for MainServicesCfg"""

   def setUp(self):
      self.flags = initConfigFlags()

   def test_Legacy(self):
      flags = initConfigFlags()
      flags.Concurrency.NumThreads = 0
      flags.lock()
      MainServicesCfg(flags).wasMerged()

   def test_MT(self):
      flags = initConfigFlags()
      flags.Concurrency.NumThreads = 1
      flags.lock()
      MainServicesCfg(flags).wasMerged()

   def test_MTEventService(self):
      flags = initConfigFlags()
      flags.Concurrency.NumThreads = 1
      flags.Exec.MTEventService = True
      flags.lock()
      MainServicesCfg(flags).wasMerged()

   @skipUnless(importlib.util.find_spec("AthenaMP"), "AthenaMP module not available")
   def test_MP(self):
      flags = initConfigFlags()
      flags.Concurrency.NumProcs = 1
      flags.lock()
      MainServicesCfg(flags).wasMerged()

   @skipUnless(_flags.hasCategory("Overlay"), "Overlay flags not available")
   def test_MPOverlay(self):
      flags = initConfigFlags()
      flags.Concurrency.NumProcs = 1
      flags.Common.isOverlay = True
      flags.lock()
      MainServicesCfg(flags).wasMerged()

   def test_MPI(self):
      flags = initConfigFlags()
      flags.Concurrency.NumThreads = 1
      flags.Exec.MPI = True
      flags.lock()
      MainServicesCfg(flags).wasMerged()

   def test_interactive(self):
      flags = initConfigFlags()
      flags.Exec.Interactive = 'run'
      flags.lock()
      MainServicesCfg(flags).wasMerged()

   @skipUnless(importlib.util.find_spec("McEventSelector"), "McEventSelector module not available")
   def test_Evgen(self):
      flags = initConfigFlags()
      flags.Input.RunNumbers = [284500] # Set to either MC DSID or MC Run Number
      flags.Input.TimeStamps = [1] # dummy value
      flags.lock()
      MainEvgenServicesCfg(flags, withSequences=True).wasMerged()


if __name__ == "__main__":
   unittest.main()
