#!/usr/bin/env python
"""Run tests on PixelG4_SD configuration

Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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
  flags.Input.Files = defaultTestFiles.EVNT
  flags.lock()

  ## Initialize the main component accumulator
  from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
  from PixelG4_SD.PixelG4_SDToolConfig import PixelSensorSDCfg
  from PixelG4_SD.PixelG4_SDToolConfig import PixelSensor_CTBCfg
  from PixelG4_SD.PixelG4_SDToolConfig import DBMSensorSDCfg

  tools = []
  cfg = ComponentAccumulator()
  tools += [ cfg.popToolsAndMerge(PixelSensorSDCfg(flags)) ]
  tools += [ cfg.popToolsAndMerge(PixelSensor_CTBCfg(flags)) ]
  tools += [ cfg.popToolsAndMerge(DBMSensorSDCfg(flags)) ]

  cfg.setPrivateTools(tools)
  cfg.printConfig(withDetails=True, summariseProps = True)
  flags.dump()

  with open("test.pkl", "wb") as f:
    cfg.store(f)

  print(cfg._privateTools)
  cfg.popPrivateTools()
  print("-----------------finished----------------------")
