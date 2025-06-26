#!/usr/bin/env python
"""Run tests on G4AtlasFieldConfig

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
  from AthenaConfiguration.Enums import Project
  from AthenaConfiguration.TestDefaults import defaultGeometryTags, defaultTestFiles
  flags = initConfigFlags()
  flags.Input.Files = defaultTestFiles.EVNT_RUN3_50
  flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3
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

  from G4AtlasTools.G4FieldConfig import ATLASFieldManagerToolCfg, TightMuonsATLASFieldManagerToolCfg, Q1FwdFieldManagerToolCfg
  #add the algorithm
  acc1 = ATLASFieldManagerToolCfg(flags)
  acc2 = TightMuonsATLASFieldManagerToolCfg(flags)

  cfg.popToolsAndMerge(acc1)
  cfg.popToolsAndMerge(acc2)

  #don't run for simulation only tests (todo - make new general test)
  if flags.Common.Project is not Project.AthSimulation:
    acc3 = Q1FwdFieldManagerToolCfg(flags)
    cfg.popToolsAndMerge(acc3)

  # Dump config
  #cfg.getService("StoreGateSvc").Dump = True
  #cfg.getService("ConditionStore").Dump = True
  cfg.printConfig(withDetails=True, summariseProps = True)
  flags.dump()

  with open("test.pkl", "wb") as f:
    cfg.store(f)

  print("-----------------finished----------------------")
