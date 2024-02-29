#!/usr/bin/env python

# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.MainServicesConfig import MainServicesCfg    
from AthenaCommon.Logging import log
import sys

if len(sys.argv[1:]):
  cmdargs=dict(arg.split('=') for arg in sys.argv[1:])
  if 'MisalignMode' not in cmdargs.keys():
    MisalignMode = -1
  else:
    MisalignMode=int(cmdargs.get('MisalignMode',11))
    print("Looking for misalignment files for mode "+str(MisalignMode))
else:
  MisalignMode = -1

flags = initConfigFlags()

flags.Input.isMC             = True

flags.Input.Files = []

from AthenaConfiguration.TestDefaults import defaultGeometryTags
flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN4

#Toggle this to use a local geometry input
flags.ITk.Geometry.AllLocal = False

if flags.ITk.Geometry.AllLocal:
  ## Just enable ID for the moment.
  detectors = [
    "ITkPixel",
    "ITkStrip",
    "Bpipe"
  ]
  from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
  setupDetectorFlags(flags, detectors, toggle_geometry=True)

  flags.TrackingGeometry.MaterialSource = "Input"
  flags.Detector.GeometryHGTD = False

flags.IOVDb.GlobalTag = "OFLCOND-MC21-SDR-RUN4-01"
flags.GeoModel.Align.Dynamic = False
if(MisalignMode!=-1):
  tag=""
  BFile=""
  DBFile="MisalignmentSet"+str(MisalignMode)+".db"
  if(MisalignMode==0):
    tag="InDetSi_MisalignmentMode_no Misalignment"
  elif(MisalignMode==1):
    tag="InDetSi_MisalignmentMode_misalignment by 6 parameters"
  elif(MisalignMode==2):
    tag="InDetSi_MisalignmentMode_random misalignment"
  elif(MisalignMode==3):
    tag="InDetSi_MisalignmentMode_IBL-stave temperature dependent bowing"
  elif(MisalignMode==11):
    tag="InDetSi_MisalignmentMode_R deltaR (radial expansion)"
  elif(MisalignMode==21):
    tag="InDetSi_MisalignmentMode_R deltaPhi (curl)"
  elif(MisalignMode==31):
    tag="InDetSi_MisalignmentMode_R deltaZ (telescope)"
  elif(MisalignMode==99):
    tag="InDetSi_MisalignmentMode_99"
    DBFile="MisalignmentSet99.db"
  DBName="OFLCOND"
  flags.IOVDb.DBConnection ="sqlite://;schema="+DBFile+";dbname="+DBName

flags.Detector.GeometryCalo = False
flags.Detector.GeometryMuon = False

# This should run serially for the moment.
flags.Concurrency.NumThreads = 1
flags.Concurrency.NumConcurrentEvents = 1

log.debug('Lock config flags now.')
flags.lock()

log.debug('dumping config flags now.')
flags.dump()

cfg=MainServicesCfg(flags)    
if(MisalignMode!=-1):
  from IOVDbSvc.IOVDbSvcConfig import addFolders
  cfg.merge(addFolders(flags,"/Indet/AlignITk",db=DBName,detDb=DBFile,tag=tag))

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
                                   EtaMin                  =  -5.,
                                   EtaMax                  =   5.,
                                   # Configure how you wanna run
                                   CollectSensitive        = True,
                                   CollectPassive          = True,
                                   CollectBoundary         = True,
                                   CollectMaterial         = True,
                                   UseHGTD                 = flags.Detector.GeometryHGTD,
                                   # the path limit to test
                                   PathLimit               = -1.,
                                   )

cfg.merge(topoAcc)

cfg.printConfig()
if(MisalignMode!=-1):
  cfg.getService("GeoModelSvc").DetectorTools["ITk::PixelDetectorTool"].Alignable=True
  print("Pixel manager alignability has been set to: "+ str(cfg.getService("GeoModelSvc").DetectorTools["ITk::PixelDetectorTool"].Alignable))
  cfg.getService("GeoModelSvc").DetectorTools["ITk::StripDetectorTool"].Alignable=True
  print("Strip manager alignability has been set to:"+str(cfg.getService("GeoModelSvc").DetectorTools["ITk::StripDetectorTool"].Alignable))

cfg.run(10)
