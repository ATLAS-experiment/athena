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
  flags = initConfigFlags()
  flags.Common.ProductionStep = ProductionStep.Simulation

  from AthenaConfiguration.TestDefaults import defaultGeometryTags, defaultTestFiles
  flags.Input.Files = defaultTestFiles.EVNT_RUN3_50
  flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3

  if flags.Common.Project is Project.AthSimulation:
    detectors = ['Bpipe', 'BCM', 'Pixel', 'SCT', 'TRT', 'LAr', 'Tile', 'MBTS', 'CSC', 'MDT', 'RPC', 'TGC'] # Forward Detector geometry not currently included in AthSimulation
  else:
    detectors = ['Bpipe', 'BCM', 'Pixel', 'SCT', 'TRT', 'LAr', 'Tile', 'MBTS', 'CSC', 'MDT', 'RPC', 'TGC', 'FwdRegion', 'Lucid', 'ZDC', 'ALFA', 'AFP']

  # Setup detector flags
  from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
  setupDetectorFlags(flags, detectors, toggle_geometry=True)

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
  from G4AtlasTools.G4GeometryToolConfig import TRTGeoDetectorToolCfg
  from G4AtlasTools.G4GeometryToolConfig import IDETEnvelopeCfg
  from G4AtlasTools.G4GeometryToolConfig import ATLASEnvelopeCfg
  from G4AtlasTools.G4GeometryToolConfig import CALOEnvelopeCfg
  from G4AtlasTools.G4GeometryToolConfig import LucidGeoDetectorToolCfg
  from G4AtlasTools.G4GeometryToolConfig import ALFAGeoDetectorToolCfg
  from G4AtlasTools.G4GeometryToolConfig import ZDCGeoDetectorToolCfg
  from G4AtlasTools.G4GeometryToolConfig import AFPGeoDetectorToolCfg
  from G4AtlasTools.G4GeometryToolConfig import ForwardRegionEnvelopeCfg
  from G4AtlasTools.G4GeometryToolConfig import MaterialDescriptionToolCfg

  ## Initialize a new component accumulator
  cfg = ComponentAccumulator()

  tool = cfg.popToolsAndMerge(BeamPipeGeoDetectorToolCfg(flags))
  cfg.addPublicTool(tool)

  tool = cfg.popToolsAndMerge(PixelGeoDetectorToolCfg(flags))
  cfg.addPublicTool(tool)

  tool = cfg.popToolsAndMerge(SCTGeoDetectorToolCfg(flags))
  cfg.addPublicTool(tool)

  tool = cfg.popToolsAndMerge(TRTGeoDetectorToolCfg(flags))
  cfg.addPublicTool(tool)

  tool = cfg.popToolsAndMerge(IDETEnvelopeCfg(flags))
  cfg.addPublicTool(tool)

  tool = cfg.popToolsAndMerge(ForwardRegionEnvelopeCfg(flags))
  cfg.addPublicTool(tool)

  tool = cfg.popToolsAndMerge(CALOEnvelopeCfg(flags))
  cfg.addPublicTool(tool)

  tool = cfg.popToolsAndMerge(LucidGeoDetectorToolCfg(flags))
  cfg.addPublicTool(tool)

  tool = cfg.popToolsAndMerge(ALFAGeoDetectorToolCfg(flags))
  cfg.addPublicTool(tool)

  tool = cfg.popToolsAndMerge(ZDCGeoDetectorToolCfg(flags))
  cfg.addPublicTool(tool)

  tool = cfg.popToolsAndMerge(AFPGeoDetectorToolCfg(flags))
  cfg.addPublicTool(tool)

  tool = cfg.popToolsAndMerge(ATLASEnvelopeCfg(flags))
  cfg.addPublicTool(tool)

  tool = cfg.popToolsAndMerge(MaterialDescriptionToolCfg(flags))
  cfg.addPublicTool(tool)

  cfg.printConfig(withDetails=True, summariseProps = True)
  flags.dump()

  with open("test.pkl", "wb") as f:
    cfg.store(f)

  print("-----------------finished----------------------")
