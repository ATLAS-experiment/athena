#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaConfiguration.AllConfigFlags import initConfigFlags

flags = initConfigFlags()
flags.Exec.MaxEvents = 1
flags.fillFromArgs()
flags.lock()

cfg = MainServicesCfg(flags)
cfg.addEventAlgo(CompFactory.asg.EventStoreTestAlg("EventStoreTestAlg",
                   Tool = CompFactory.asg.EventStoreTestTool("EventStoreTestTool")))

import sys
sys.exit(cfg.run().isFailure())
