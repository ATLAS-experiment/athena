#!/usr/bin/env athena
#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaConfiguration.AllConfigFlags import initConfigFlags
import sys

flags = initConfigFlags()
flags.Input.Files = []
flags.Exec.MaxEvents = 3
flags.lock()

cfg = MainServicesCfg(flags)

cfg.addEventAlgo( CompFactory.WriteDataReentrant(DObjKeyArray = ['x1', 'x2', 'x3']) )
cfg.addEventAlgo( CompFactory.ReadDataReentrant(DObjKeyArray = ['x1', 'x2', 'x3']) )

sys.exit(cfg.run().isFailure())
