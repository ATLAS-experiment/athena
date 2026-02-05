#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.MainServicesConfig import MainEvgenServicesCfg
from AthenaConfiguration.AllConfigFlags import initConfigFlags
import sys

flags = initConfigFlags()
flags.Input.Files = []
flags.Input.RunNumbers = [284500]  # dummy value
flags.Input.TimeStamps = [1]  # dummy value
flags.Exec.FPE = 0  # FPE w/o stack trace
flags.fillFromArgs()
flags.lock()

cfg = MainEvgenServicesCfg(flags)
cfg.addEventAlgo(CompFactory.AthExAlgWithFPE(),sequenceName="AthAlgSeq")
  
sys.exit(cfg.run(3).isFailure())
