#!/usr/bin/env python
"""Run tests on G4AtlasServicesConfig

Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""

if __name__ == '__main__':
  from AthenaConfiguration.MainServicesConfig import MainServicesCfg

  # Set up logging
  from AthenaCommon.Logging import log
  from AthenaCommon.Constants import DEBUG
  log.setLevel(DEBUG)

  #import config flags
  from AthenaConfiguration.AllConfigFlags import initConfigFlags
  from AthenaConfiguration.Enums import ProductionStep
  flags = initConfigFlags()
  flags.Common.ProductionStep = ProductionStep.Simulation

  from AthenaConfiguration.TestDefaults import defaultGeometryTags, defaultTestFiles
  flags.Input.Files = defaultTestFiles.EVNT
  flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2

  from SimulationConfig.SimEnums import CavernBackground
  flags.Sim.CavernBackground = CavernBackground.Signal  #for it to go via atlas?
  flags.Sim.WorldRRange = 15000
  flags.Sim.WorldZRange = 27000
  # Finalize
  flags.lock()

  ## Initialize a new component accumulator
  cfg = MainServicesCfg(flags)

  # Dump config
  #cfg.getService("StoreGateSvc").Dump = True
  #cfg.getService("ConditionStore").Dump = True
  cfg.printConfig(withDetails=True, summariseProps = True)
  flags.dump()

  with open("test.pkl", "wb") as f:
    cfg.store(f)

  print ("-----------------finished----------------------")
