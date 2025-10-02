#!/usr/bin/env python
"""Run tests on G4AtlasFieldServices

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
  from AthenaConfiguration.TestDefaults import defaultTestFiles
  flags = initConfigFlags()
  flags.Input.Files = defaultTestFiles.EVNT_RUN3_50
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

  from G4AtlasServices.G4AtlasFieldServices import StandardFieldSvcCfg
  from G4AtlasServices.G4AtlasFieldServices import ForwardFieldSvcCfg
  from G4AtlasServices.G4AtlasFieldServices import Q1FwdG4FieldSvcCfg

  #add the algorithm
  acc1 = StandardFieldSvcCfg(flags)
  acc2 = ForwardFieldSvcCfg(flags)

  # don't run for simulation only tests (todo - make new general test)
  if flags.Common.Project is not Project.AthSimulation:
    acc3 = Q1FwdG4FieldSvcCfg(flags)
    cfg.merge(acc3)

  cfg.merge(acc1)
  cfg.merge(acc2)

  # Dump config
  #cfg.getService("StoreGateSvc").Dump = True
  #cfg.getService("ConditionStore").Dump = True
  cfg.printConfig(withDetails=True, summariseProps = True)
  flags.dump()

  with open("test.pkl", "wb") as f:
    cfg.store(f)

  print ("-----------------finished----------------------")
