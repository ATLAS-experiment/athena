#!/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Set the Athena configuration flags
from AthenaConfiguration.AllConfigFlags import initConfigFlags
flags = initConfigFlags()
flags.fillFromArgs()
flags.lock()


from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
cfg = MainServicesCfg(flags)
cfg.merge(PoolReadCfg(flags))

from  AthenaMonitoring.TriggerInterface import TrigDecisionToolCfg

cfg.merge(TrigDecisionToolCfg(flags))
checker = CompFactory.TrigDecisionChecker()
checker.WriteEventDecision = True
checker.WriteOutFilename = "trigger.counts.log"

cfg.addEventAlgo(checker)
cfg.run()
