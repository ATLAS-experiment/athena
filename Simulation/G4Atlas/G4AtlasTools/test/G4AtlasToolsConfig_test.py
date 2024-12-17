#!/usr/bin/env python
"""Run tests on G4AtlasToolsConfig

Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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
  from AthenaConfiguration.TestDefaults import defaultGeometryTags, defaultTestFiles
  flags = initConfigFlags()
  flags.Common.ProductionStep = ProductionStep.Simulation
  flags.Input.Files = defaultTestFiles.EVNT
  flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3
  flags.Sim.ISFRun = True

  # Setup detector flags
  from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
  setupDetectorFlags(flags, ['BCM', 'Pixel', 'SCT', 'TRT', 'LAr', 'MBTS'], toggle_geometry=True)
  # Arbitrary configuration for Twiss Files
  flags.Sim.TwissFileBeam1 = '4.0TeV/0090.00m/nominal/v01/beam1.tfs'
  flags.Sim.TwissFileBeam2 = '4.0TeV/0090.00m/nominal/v01/beam2.tfs'
  flags.Sim.TwissFileBeta = 90000 # in mm
  flags.Sim.TwissFileNomReal = 'nominal'
  flags.Sim.TwissFileVersion = 'v01'

  # Finalize
  flags.lock()

  ## Initialize a new component accumulator
  cfg = MainServicesCfg(flags)

  from G4AtlasTools.G4AtlasToolsConfig import SensitiveDetectorMasterToolCfg
  acc  = SensitiveDetectorMasterToolCfg(flags)
  tool = cfg.popToolsAndMerge(acc)
  cfg.setPrivateTools(tool)

  cfg.printConfig(withDetails=True, summariseProps = True)
  flags.dump()

  #cfg not being used so complains ...fine now!
  with open("test.pkl", "wb") as f:
    cfg.store(f) #sets wasmerged = true

  cfg.popPrivateTools()
  print ("-----------------finished----------------------")
