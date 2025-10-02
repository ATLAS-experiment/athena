#!/usr/bin/env python
"""Run tests on BCM_G4_SD configuration

Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""


if __name__ == '__main__':
  # Set up logging and config behaviour
  from AthenaCommon.Logging import log
  from AthenaCommon.Constants import DEBUG
  log.setLevel(DEBUG)

  # Setup config flags
  from AthenaConfiguration.AllConfigFlags import initConfigFlags
  from AthenaConfiguration.TestDefaults import defaultGeometryTags, defaultTestFiles
  flags = initConfigFlags()
  flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3
  flags.Sim.ISFRun = True
  flags.Input.Files = defaultTestFiles.EVNT_RUN3_50
  flags.lock()

  # Setup the tool
  from BCM_G4_SD.BCM_G4_SDToolConfig import BCMSensorSDCfg
  cfg = BCMSensorSDCfg(flags)
  cfg.printConfig(withDetails=True, summariseProps = True)
  flags.dump()

  with open("test.pkl", "wb") as f:
    cfg.store(f)

  print(cfg._privateTools)
  print("-----------------finished----------------------")
