#!/usr/bin/env python
"""Run tests on G4Geometry Tool configuration

Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


if __name__ == '__main__':
  # Set up logging
  from AthenaCommon.Logging import log
  from AthenaCommon.Constants import DEBUG
  log.setLevel(DEBUG)

  #import config flags
  from AthenaConfiguration.AllConfigFlags import initConfigFlags
  from AthenaConfiguration.Enums import ProductionStep, Project
  from AthenaConfiguration.TestDefaults import defaultGeometryTags, defaultTestFiles
  flags = initConfigFlags()
  flags.Common.ProductionStep = ProductionStep.Simulation
  flags.Input.Files = defaultTestFiles.EVNT_RUN3_50
  flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3

  if flags.Common.Project is Project.AthSimulation:
    detectors =['Bpipe', 'BCM', 'Pixel', 'SCT', 'TRT', 'LAr', 'Tile', 'MBTS', 'CSC', 'MDT', 'RPC', 'TGC'] # FwdRegion geometry not currently included in AthSimulation
  else:
    detectors =['Bpipe', 'BCM', 'Pixel', 'SCT', 'TRT', 'LAr', 'Tile', 'MBTS', 'CSC', 'MDT', 'RPC', 'TGC', 'FwdRegion']

  # Setup detector flags
  from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
  setupDetectorFlags(flags, detectors, toggle_geometry=True)

  #turn the forward region off
  flags.Sim.WorldRRange = 15000
  flags.Sim.WorldZRange = 27000

  # Arbitrary configuration for Twiss Files
  flags.Sim.TwissFileBeam1 = '4.0TeV/0090.00m/nominal/v01/beam1.tfs'
  flags.Sim.TwissFileBeam2 = '4.0TeV/0090.00m/nominal/v01/beam2.tfs'
  flags.Sim.TwissFileBeta = 90000 # in mm
  flags.Sim.TwissFileNomReal = 'nominal'
  flags.Sim.TwissFileVersion = 'v01'

  # Finalize
  flags.lock()

  from G4AtlasTools.G4GeometryToolConfig import BeamPipeGeoDetectorToolCfg
  from G4AtlasTools.G4GeometryToolConfig import PixelGeoDetectorToolCfg
  from G4AtlasTools.G4GeometryToolConfig import SCTGeoDetectorToolCfg
  from G4AtlasTools.G4GeometryToolConfig import IDETEnvelopeCfg
  from G4AtlasTools.G4GeometryToolConfig import ATLASEnvelopeCfg
  from G4AtlasTools.G4GeometryToolConfig import CALOEnvelopeCfg

  ## Initialize a new component accumulator
  cfg = ComponentAccumulator()

  acc  = BeamPipeGeoDetectorToolCfg(flags)
  tool = cfg.popToolsAndMerge(acc)
  cfg.addPublicTool(tool)

  acc  = PixelGeoDetectorToolCfg(flags)
  tool = cfg.popToolsAndMerge(acc)
  cfg.addPublicTool(tool)

  acc  = SCTGeoDetectorToolCfg(flags)
  tool = cfg.popToolsAndMerge(acc)
  cfg.addPublicTool(tool)

  acc  = SCTGeoDetectorToolCfg(flags)
  tool = cfg.popToolsAndMerge(acc)
  cfg.addPublicTool(tool)

  acc  = IDETEnvelopeCfg(flags)
  tool = cfg.popToolsAndMerge(acc)
  cfg.addPublicTool(tool)

  acc  = CALOEnvelopeCfg(flags)
  tool = cfg.popToolsAndMerge(acc)
  cfg.addPublicTool(tool)

  acc  = ATLASEnvelopeCfg(flags)
  tool = cfg.popToolsAndMerge(acc)
  cfg.addPublicTool(tool)

  cfg.printConfig(withDetails=True, summariseProps = True)
  flags.dump()

  with open("test.pkl", "wb") as f:
    cfg.store(f)

  print ("-----------------finished----------------------")
