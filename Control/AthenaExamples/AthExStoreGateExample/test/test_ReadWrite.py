#!/usr/bin/env athena
#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoCnvAlgCfg
import sys

flags = initConfigFlags()
flags.Input.Files = []
flags.Input.RunNumbers = [1]
flags.Exec.MaxEvents = 3
flags.lock()

cfg = MainServicesCfg(flags)
cfg.merge(EventInfoCnvAlgCfg(flags, inputKey="", disableBeamSpot=True))
cfg.addEventAlgo( CompFactory.WriteData() )
cfg.addEventAlgo( CompFactory.ReadData(DataProducer = "WriteData") )

sys.exit(cfg.run().isFailure())
