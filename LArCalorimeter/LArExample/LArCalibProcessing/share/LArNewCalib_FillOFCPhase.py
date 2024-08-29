#!/usr/bin/env python
#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

if __name__=='__main__':

  import os,sys
  import argparse
  import subprocess
  from AthenaCommon import Logging
  log = Logging.logging.getLogger( 'LArSC2Ntuple' )
  
  parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)

  parser.add_argument('-i','--infile', dest='inpfile', default="OFC_phase.dat", help='Input filename', type=str)
  parser.add_argument('-k','--outkey', dest='outkey', default="LArOFCPhase", help='phase container key', type=str)
  parser.add_argument('-c','--isSC', dest='supercells', default=False, action="store_true", help='is SC data ?')
  parser.add_argument('-a','--hasid', dest='hasid', default=False, action="store_true", help='has ID in input ?')
  parser.add_argument('-d','--default', dest='defaultphase', default=0, help='if not in input', type=int)
  parser.add_argument('-f','--folder', dest='folder', default="/LAR/ElecCalibOfl/OFCBin/Dummy", help=' folder name', type=str)
  parser.add_argument('-t','--tag', dest='tag', default="LArOFCPhase-01", help='folder tag', type=str)
  parser.add_argument('-o','--outsql', dest='outsql', default="OFCPhase.db", help='output sqlite filename', type=str)
  parser.add_argument('-p','--outp', dest='outpool', default="ofc_phase.pool.root", help='output pool filename', type=str)
  parser.add_argument('--poolcat', dest='poolcat', default="PoolFileCatalog.xml", help='Catalog of POOL files', type=str)


  args = parser.parse_args()
  if help in args and args.help is not None and args.help:
     parser.print_help()
     sys.exit(0)

  for _, value in args._get_kwargs():
   if value is not None:
       print(value)

  # now set flags according parsed options
   
  #Import the MainServices (boilerplate)
  from AthenaConfiguration.MainServicesConfig import MainServicesCfg
  
  #Import the flag-container that is the arguemnt to the configuration methods
  from AthenaConfiguration.AllConfigFlags import initConfigFlags
  flags=initConfigFlags()
  from LArCalibProcessing.LArCalibConfigFlags import addLArCalibFlags
  addLArCalibFlags(flags, args.supercells)
   
  flags.Input.Files=[]
  flags.LArCalib.Input.Files = [ ]
  flags.LArCalib.Input.RunNumbers = [9999999]

  flags.LArCalib.IOVStart=0

  flags.LArCalib.isSC = args.supercells

  flags.IOVDb.DBConnection="sqlite://;schema=" + args.outsql +";dbname=CONDBR2"

  #The global tag we are working with
  flags.IOVDb.GlobalTag = "CONDBR2-BLKPA-2024-02"

   
  from AthenaConfiguration.TestDefaults import defaultGeometryTags
  flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3

  #Define the global output Level:
  from AthenaCommon.Constants import INFO,DEBUG
  flags.Exec.OutputLevel = INFO

  flags.lock()

  cfg=MainServicesCfg(flags)

  #Get basic services and cond-algos
  from LArCalibProcessing.LArCalibBaseConfig import LArCalibBaseCfg
  cfg.merge(LArCalibBaseCfg(flags))


  LArOFPhaseFill = CompFactory.getComp('LArOFPhaseFill')("LArOFPhaseFill")
  LArOFPhaseFill.InputFile = args.inpfile
  LArOFPhaseFill.keyOFCBin = args.outkey
  LArOFPhaseFill.isSC = args.supercells
  LArOFPhaseFill.isID = args.hasid
  LArOFPhaseFill.DefaultPhase = args.defaultphase
  LArOFPhaseFill.GroupingType = "ExtendedSubDetector" if not args.supercells else "SuperCells"
  LArOFPhaseFill.OutputLevel = DEBUG

  cfg.addEventAlgo(LArOFPhaseFill)

  from RegistrationServices.OutputConditionsAlgConfig import OutputConditionsAlgCfg
  cfg.merge(OutputConditionsAlgCfg(flags,
                                      outputFile=args.outpool,
                                      ObjectList=["LArOFCBinComplete#"+args.outkey+"#"+args.folder,],
                                      IOVTagList=[args.tag,],
                                      Run1=flags.LArCalib.IOVStart,
                                      Run2=flags.LArCalib.IOVEnd
                                  ))
  cfg.addService(CompFactory.IOVRegistrationSvc(RecreateFolders = False))

  #MC Event selector since we have no input data file 
  from McEventSelector.McEventSelectorConfig import McEventSelectorCfg
  cfg.merge(McEventSelectorCfg(flags,
                                  RunNumber = flags.LArCalib.Input.RunNumbers[0],
                                  EventsPerRun      = 1,
                                  FirstEvent	      = 1,
                                  InitialTimeStamp  = 0,
                                  TimeStampInterval = 1))

  cfg.getService("IOVDbSvc").DBInstance="CONDBR2"
  cfg.getService("MessageSvc").defaultLimit=9999999
  cfg.getService("PoolSvc").WriteCatalog="xmlcatalog_file:%s"%args.poolcat
     
  print("Start running...")
  cfg.run(1)


