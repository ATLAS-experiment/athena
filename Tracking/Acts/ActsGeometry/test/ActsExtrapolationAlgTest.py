#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration 

"""
This job options file will run an example extrapolation using the
Acts tracking geometry and the Acts extrapolation toolchain.
"""

if "__main__" == __name__:
  from AthenaCommon.Logging import log
  from AthenaCommon.Constants import INFO
  from AthenaConfiguration.AllConfigFlags import initConfigFlags
  flags = initConfigFlags()

  ## Just enable ID for the moment.
  flags.Input.isMC             = True
  flags.Input.Files = ['/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.evgen.EVNT.e4993.EVNT.08166201._000012.pool.root.1']
  flags.GeoModel.AtlasVersion  = "ATLAS-R2-2016-01-00-01"
  flags.IOVDb.GlobalTag        = "OFLCOND-SIM-00-00-00"
  flags.Detector.GeometryBpipe = True
  flags.Detector.GeometryID    = True
  flags.Detector.GeometryPixel = True
  flags.Detector.GeometrySCT   = True
  flags.Detector.GeometryCalo  = False
  flags.Detector.GeometryMuon  = False
  flags.Detector.GeometryTRT   = True
  flags.Acts.TrackingGeometry.MaterialSource = "None"

  flags.Concurrency.NumThreads = 10
  flags.Concurrency.NumConcurrentEvents = 10

  flags.Exec.MaxEvents = 100

  flags.lock()
  flags.dump()

  from AthenaConfiguration.MainServicesConfig import MainServicesCfg
  cfg = MainServicesCfg(flags)

  from BeamPipeGeoModel.BeamPipeGMConfig import BeamPipeGeometryCfg
  cfg.merge(BeamPipeGeometryCfg(flags))

  from ActsConfig.ActsGeometryConfig import ActsExtrapolationAlgCfg
  alg = ActsExtrapolationAlgCfg(flags,
                                OutputLevel=INFO,
                                NParticlesPerEvent = int(100),
                                WritePropStep = True,
                                EtaRange = [-2.5, 2.5],
                                PtRange = [20, 100])

  cfg.merge(alg)

  tgSvc = cfg.getService("ActsTrackingGeometrySvc")

  # Service will have removed TRT and Calo
  # We want them enabled for testing
  # Temporarily disabling Calo though
  tgSvc.BuildSubDetectors += [
    "TRT"
    #"Calo"
  ]

  # needed to construct the calo geometry in ACTS
  #from AthenaConfiguration.ComponentFactory import CompFactory
  #tgSvc.CaloVolumeBuilder = CompFactory.ActsCaloTrackingVolumeBuilder()

  cfg.printConfig()

  log.info("CONFIG DONE")

  sc = cfg.run()
  
  if sc.isFailure():
    import sys
    sys.exit(1)

