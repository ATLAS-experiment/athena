#!/usr/bin/env python

"""Run tests on G4PhysicsRegionConfig configuration

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
  from AthenaConfiguration.TestDefaults import defaultGeometryTags, defaultTestFiles
  inputDir = defaultTestFiles.d
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

  #import the following tool configs
  from G4AtlasTools.G4PhysicsRegionConfig import *
  #add the tools
  cfg.addPublicTool(cfg.popToolsAndMerge(BeampipeFwdCutPhysicsRegionToolCfg(flags)))
  cfg.addPublicTool(cfg.popToolsAndMerge(FWDBeamLinePhysicsRegionToolCfg(flags)))
  cfg.addPublicTool(cfg.popToolsAndMerge(FwdRegionPhysicsRegionToolCfg(flags)))
  cfg.addPublicTool(cfg.popToolsAndMerge(PixelPhysicsRegionToolCfg(flags)))
  cfg.addPublicTool(cfg.popToolsAndMerge(SCTPhysicsRegionToolCfg(flags)))
  cfg.addPublicTool(cfg.popToolsAndMerge(TRTPhysicsRegionToolCfg(flags)))
  cfg.addPublicTool(cfg.popToolsAndMerge(TRT_ArPhysicsRegionToolCfg(flags)))
  cfg.addPublicTool(cfg.popToolsAndMerge(TRT_KrPhysicsRegionToolCfg(flags)))
  cfg.addPublicTool(cfg.popToolsAndMerge(EMBPhysicsRegionToolCfg(flags)))
  cfg.addPublicTool(cfg.popToolsAndMerge(EMECPhysicsRegionToolCfg(flags)))
  cfg.addPublicTool(cfg.popToolsAndMerge(HECPhysicsRegionToolCfg(flags)))
  cfg.addPublicTool(cfg.popToolsAndMerge(FCALPhysicsRegionToolCfg(flags)))
  cfg.addPublicTool(cfg.popToolsAndMerge(EMECParaPhysicsRegionToolCfg(flags)))
  cfg.addPublicTool(cfg.popToolsAndMerge(FCALParaPhysicsRegionToolCfg(flags)))
  cfg.addPublicTool(cfg.popToolsAndMerge(FCAL2ParaPhysicsRegionToolCfg(flags)))
  cfg.addPublicTool(cfg.popToolsAndMerge(PreSampLArPhysicsRegionToolCfg(flags)))
  cfg.addPublicTool(cfg.popToolsAndMerge(DeadMaterialPhysicsRegionToolCfg(flags)))
  cfg.addPublicTool(cfg.popToolsAndMerge(DriftWallPhysicsRegionToolCfg(flags)))
  cfg.addPublicTool(cfg.popToolsAndMerge(DriftWall1PhysicsRegionToolCfg(flags)))
  cfg.addPublicTool(cfg.popToolsAndMerge(DriftWall2PhysicsRegionToolCfg(flags)))
  cfg.addPublicTool(cfg.popToolsAndMerge(MuonSystemFastPhysicsRegionToolCfg(flags)))
  cfg.addPublicTool(cfg.popToolsAndMerge(MuonPhysicsRegionToolCfg(flags)))
  cfg.addPublicTool(cfg.popToolsAndMerge(SX1PhysicsRegionToolCfg(flags)))
  cfg.addPublicTool(cfg.popToolsAndMerge(BedrockPhysicsRegionToolCfg(flags)))
  cfg.addPublicTool(cfg.popToolsAndMerge(CavernShaftsConcretePhysicsRegionToolCfg(flags)))
  cfg.addPublicTool(cfg.popToolsAndMerge(SCTSiliconPhysicsRegionToolCfg(flags)))

  # Dump config
  # cfg.getService("StoreGateSvc").Dump = True
  # cfg.getService("ConditionStore").Dump = True
  cfg.printConfig(withDetails=True, summariseProps = True)
  flags.dump()

  with open("test.pkl", "wb") as f:
    cfg.store(f)

  print("-----------------finished----------------------")
