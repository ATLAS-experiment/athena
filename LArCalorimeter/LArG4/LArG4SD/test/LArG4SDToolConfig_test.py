#!/usr/bin/env python
"""Run tests on LArG4SD configuration

Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
ComponentAccumulator.debugMode = 'trackCA'


if __name__ == '__main__':

  # Set up logging
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

  ## Initialize a new component accumulator
  cfg = ComponentAccumulator()

  from LArG4SD.LArG4SDToolConfig import LArEMBSensitiveDetectorCfg
  from LArG4SD.LArG4SDToolConfig import LArEMECSensitiveDetectorCfg
  from LArG4SD.LArG4SDToolConfig import LArFCALSensitiveDetectorCfg
  from LArG4SD.LArG4SDToolConfig import LArHECSensitiveDetectorCfg
  from LArG4SD.LArG4SDToolConfig import LArDeadSensitiveDetectorToolCfg
  from LArG4SD.LArG4SDToolConfig import LArActiveSensitiveDetectorToolCfg
  from LArG4SD.LArG4SDToolConfig import LArInactiveSensitiveDetectorToolCfg

  acc1 = LArEMBSensitiveDetectorCfg(flags)
  tool1 = cfg.popToolsAndMerge(acc1)
  
  acc2 = LArEMECSensitiveDetectorCfg(flags)
  tool2 = cfg.popToolsAndMerge(acc2)

  acc3 = LArFCALSensitiveDetectorCfg(flags)
  tool3 = cfg.popToolsAndMerge(acc3)
  
  acc4 = LArHECSensitiveDetectorCfg(flags)
  tool4 = cfg.popToolsAndMerge(acc4)

  acc5 = LArDeadSensitiveDetectorToolCfg(flags)
  tool5 = cfg.popToolsAndMerge(acc5)

  toolActiveSensitiveDetector = LArActiveSensitiveDetectorToolCfg(flags)
  cfg.popToolsAndMerge(toolActiveSensitiveDetector)

  toolInactiveSensitiveDetector = LArInactiveSensitiveDetectorToolCfg(flags)
  cfg.popToolsAndMerge(toolInactiveSensitiveDetector)

  cfg.printConfig(withDetails=True, summariseProps = True)
  flags.dump()

  f=open("test.pkl","wb")
  cfg.store(f)
  f.close()

  print("-----------------finished----------------------")

