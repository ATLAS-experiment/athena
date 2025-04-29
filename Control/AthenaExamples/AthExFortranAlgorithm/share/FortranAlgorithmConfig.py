#!/usr/bin/env athena.py
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaConfiguration.AllConfigFlags import initConfigFlags

flags = initConfigFlags()
flags.Exec.MaxEvents = 1
flags.fillFromArgs()
flags.lock()

cfg = MainServicesCfg(flags)
cfg.addEventAlgo( CompFactory.FortranAlgorithm(
   LUN=42, fileName="FortranAlgorithmInput.data") )

import sys
sys.exit(cfg.run().isFailure())
