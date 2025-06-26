#!/usr/bin/env python
"""Run tests on ISF_ServicesConfig.py

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
  flags = initConfigFlags()
  flags.Input.Files = defaultTestFiles.EVNT_RUN3_50
  flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3

  flags.Sim.WorldRRange = 15000
  flags.Sim.WorldZRange = 27000 #change defaults?
  detectors = ['Bpipe', 'BCM', 'Pixel', 'SCT', 'TRT', 'LAr', 'Tile', 'MBTS', 'CSC', 'MDT', 'RPC', 'TGC']
  # Setup detector flags
  from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
  setupDetectorFlags(flags, detectors, toggle_geometry=True)

  # Finalize
  flags.lock()

  from ISF_Services.ISF_ServicesConfig import MC15aPlusTruthServiceCfg, InputConverterCfg
  from ISF_Services.ISF_ServicesCoreConfig import GeoIDSvcCfg

  ## Initialize a new component accumulator
  cfg = MainServicesCfg(flags)

  #add the algorithm
  cfg.merge(MC15aPlusTruthServiceCfg(flags))
  cfg.merge(InputConverterCfg(flags))
  cfg.merge(GeoIDSvcCfg(flags))

  # Dump config
  cfg.printConfig(withDetails=True, summariseProps = True)
  flags.dump()

  with open("test.pkl", "wb") as f:
    cfg.store(f)
