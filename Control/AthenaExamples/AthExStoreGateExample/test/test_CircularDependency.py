#!/usr/bin/env python
#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# File: AthExStoreGateExample/test/test_CircularDependency.py
# Author: scott snyder
# Date: Oct, 2019
# Brief: Test for cross-component circular dependency warning suppression
#        of WriteDecorHandleKey.
#

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.MainServicesConfig import MainEvgenServicesCfg
from AthenaConfiguration.AllConfigFlags import initConfigFlags
import sys

flags = initConfigFlags()
flags.Input.Files = []
flags.Exec.MaxEvents = 1
flags.lock()

cfg = MainEvgenServicesCfg(flags)

# This should not get a circular dependency warning.
cfg.addEventAlgo( CompFactory.AthEx.HandleTestAlg ('testalg1') )

# But this should.
cfg.addEventAlgo( CompFactory.AthEx.HandleTestAlg (
   'testalg2',
   Tool1 = CompFactory.AthEx.HandleTestTool2 ('testool2'),
   Tool2 = CompFactory.AthEx.HandleTestTool1 ('testool1')) )

sys.exit(cfg.run().isFailure())
