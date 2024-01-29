#!/usr/bin/env python

# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.MainServicesConfig import MainServicesCfg    
from AthenaCommon.Logging import log

## Just enable ID for the moment.
flags = initConfigFlags()
flags.Input.isMC             = True

flags.Input.Files = []

flags.IOVDb.GlobalTag        = "OFLCOND-SIM-00-00-00"    
flags.GeoModel.AtlasVersion  = "ATLAS-R2-2016-01-00-01"
flags.Detector.GeometryBpipe = True
flags.Detector.GeometryID    = True
flags.Detector.GeometryPixel = True
flags.Detector.GeometrySCT   = True

flags.Detector.GeometryTRT   = False
flags.Detector.GeometryCalo  = False
flags.Detector.GeometryMuon  = False

# This should run serially for the moment.
flags.Concurrency.NumThreads = 1
flags.Concurrency.NumConcurrentEvents = 1

log.debug('Lock config flags now.')
flags.lock()

log.debug('dumping config flags now.')
flags.dump()

cfg=MainServicesCfg(flags)    

from TrkExUnitTests.TrkExUnitTestsConfig import ExtrapolationEngineTestCfg
topoAcc=ExtrapolationEngineTestCfg(flags,
                                   NumberOfTestsPerEvent   = 100,
                                   # parameters mode: 0 - neutral tracks, 1 - charged particles 
                                   ParametersMode          = 1,
                                   # do the full test backwards as well            
                                   BackExtrapolation       = False,
                                   # Smear the production vertex - standard primary vertex paramters
                                   SmearOrigin             = True,
                                   SimgaOriginD0           = 2./3.,
                                   SimgaOriginZ0           = 50.,
                                   # pT range for testing
                                   PtMin                   = 1000,
                                   PtMax                   = 1000,
                                   # The test range in Eta                      
                                   EtaMin                  =  -3.,
                                   EtaMax                  =   3.,
                                   # Configure how you wanna run                  
                                   CollectSensitive        = True,
                                   CollectPassive          = True,
                                   CollectBoundary         = True,
                                   CollectMaterial         = True,
                                   UseHGTD                 = False,
                                   # the path limit to test                        
                                   PathLimit               = -1.,
                                   )
cfg.merge(topoAcc)

cfg.printConfig()

cfg.run(10)
