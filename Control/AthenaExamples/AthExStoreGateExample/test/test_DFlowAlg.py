#!/usr/bin/env python
#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.MainServicesConfig import MainEvgenServicesCfg
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoCnvAlgCfg
import sys

flags = initConfigFlags()
flags.Input.Files = []
flags.Exec.MaxEvents = 10
flags.lock()

cfg = MainEvgenServicesCfg(flags)
cfg.merge(EventInfoCnvAlgCfg(flags, disableBeamSpot=True))

cfg.addEventAlgo( CompFactory.AthEx.DFlowAlg1("dflow_alg1") )
cfg.addEventAlgo( CompFactory.AthEx.DFlowAlg2("dflow_alg2") )
cfg.addEventAlgo( CompFactory.AthEx.DFlowAlg3("dflow_alg3") )

sys.exit(cfg.run().isFailure())
