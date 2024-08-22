#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration


from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.TestDefaults import defaultTestFiles
flags = initConfigFlags()
flags.Input.Files = defaultTestFiles.AOD_RUN3_MC
flags.PhysVal.OutputFileName = "PhysVal.root"
flags.lock()

from AthenaConfiguration.MainServicesConfig import MainServicesCfg
acc = MainServicesCfg(flags)

from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
acc.merge(PoolReadCfg(flags))

from GeneratorPhysVal.GeneratorPhysValConfig import GeneratorPhysValMonitoringToolCfg
from PhysValMonitoring.PhysValMonitoringConfig import PhysValMonitoringCfg
acc.merge(PhysValMonitoringCfg(flags, tools=[acc.popToolsAndMerge(GeneratorPhysValMonitoringToolCfg(flags))]))

sc = acc.run()
import sys
sys.exit(not sc.isSuccess())
