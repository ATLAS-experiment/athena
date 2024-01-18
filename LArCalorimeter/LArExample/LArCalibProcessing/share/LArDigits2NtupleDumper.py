#!/usr/bin/env python
#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

if __name__=='__main__':

  import os,sys
  import argparse
  import subprocess
  from AthenaCommon import Logging
  log = Logging.logging.getLogger( 'LArDigits2Ntuple' )
  
  parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)

  parser.add_argument('-i','--indir', dest='indir', default="/eos/atlas/atlastier0/rucio/data_test/calibration_pulseall/00414414/data_test.00414414.calibration_pulseall.daq.RAW/", help='input files dir', type=str)
  parser.add_argument('-p','--inprefix', dest='inpref', default="data_test", help='Input filenames prefix', type=str)
  parser.add_argument('-y','--inppatt', dest='inppatt', default="lb3512", help='Input filenames pattern', type=str)
  parser.add_argument('-f','--infile', dest='infile', default="", help='Input filename (if given indir and inprefix are ignored', type=str)
  parser.add_argument('-r','--run', dest='run', default=0, help='Run number (if not given trying to judge from input file name)', type=int)
  parser.add_argument('-m','--maxev', dest='maxev', default=-1, help='Max number of events to dump', type=int)
  parser.add_argument('-x','--outlevel', dest='olevel', default=3, help='OuputLevel for dumping algo', type=int)
  parser.add_argument('-o','--outfile', dest='outfile', default="Digits.root", help='Output root filename', type=str)
  parser.add_argument('-n','--nsamp', dest='nsamp', default=0, help='Number of samples to dump', type=int)
  parser.add_argument('-d','--addHash', dest='ahash', default=False, help='Add hash number to output ntuple', type=bool)
  parser.add_argument('-j','--addOffline', dest='offline', default=False, help='Add offline Id to output ntuple', type=bool)
  parser.add_argument('-k','--addCalib', dest='calib', default=False, help='Add calib. info to output ntuple', type=bool)
  parser.add_argument('-t','--addGeom', dest='geom', default=False, help='Add real geom info to output ntuple', type=bool)
  parser.add_argument('-u','--addBC', dest='bc', default=False, help='Add Bad. chan info to output ntuple', type=bool)
  parser.add_argument('-v','--addEvTree', dest='evtree', default=False, help='Add tree with per event info to output ntuple', type=bool)

  args = parser.parse_args()
  if help in args and args.help is not None and args.help:
     parser.print_help()
     sys.exit(0)

  for _, value in args._get_kwargs():
     if value is not None:
       log.debug(value)

  #Import the flag-container that is the arguemnt to the configuration methods
  from AthenaConfiguration.AllConfigFlags import ConfigFlags
  # add SCDump flags, here re-used for digitsdump
  from LArCafJobs.LArSCDumperFlags import addSCDumpFlags
  addSCDumpFlags(ConfigFlags)


  if len(args.infile) > 0:
     ConfigFlags.Input.Files = [args.infile]
  elif len(args.inppatt) > 0:
     from LArCalibProcessing.GetInputFiles import GetInputFilesFromPattern
     ConfigFlags.Input.Files = GetInputFilesFromPattern(args.indir,args.inppatt)
  else:   
     from LArCalibProcessing.GetInputFiles import GetInputFilesFromPrefix
     ConfigFlags.Input.Files = GetInputFilesFromPrefix(args.indir,args.inpref)

  if args.run != 0:
     ConfigFlags.Input.RunNumbers = [args.run]

  # first autoconfig
  from LArConditionsCommon.LArRunFormat import getLArFormatForRun
  try:
     runinfo=getLArFormatForRun(ConfigFlags.Input.RunNumbers[0], connstring="COOLONL_LAR/CONDBR2")
  except Exception:
     log.warning("Could not get  run info, using defaults !")
     if args.nsamp > 0:
        ConfigFlags.LArSCDump.nSamples=args.nsamp
     else:   
        ConfigFlags.LArSCDump.nSamples=4
  else:
     ConfigFlags.LArSCDump.nSamples=runinfo.nSamples()

  ConfigFlags.LArSCDump.digitsKey="FREE"
  if  args.nsamp > 0 and args.nsamp < ConfigFlags.LArSCDump.nSamples:
      ConfigFlags.LArSCDump.nSamples=args.nsamp
  
  log.info("Autoconfigured: ")
  log.info("nSamples: %d digitsKey %s",ConfigFlags.LArSCDump.nSamples, ConfigFlags.LArSCDump.digitsKey)

  # now construct the job
  ConfigFlags.LAr.doAlign=False

  if args.evtree: # should include trigger info
     ConfigFlags.Trigger.triggerConfig = 'DB'
     ConfigFlags.Trigger.L1.doCTP = True
     ConfigFlags.Trigger.L1.doMuon = False
     ConfigFlags.Trigger.L1.doCalo = False
     ConfigFlags.Trigger.L1.doTopo = False

     ConfigFlags.Trigger.enableL1CaloLegacy = True
     ConfigFlags.Trigger.enableL1CaloPhase1 = True

  ConfigFlags.lock()

  #Import the MainServices (boilerplate)
  from AthenaConfiguration.MainServicesConfig import MainServicesCfg
  from LArGeoAlgsNV.LArGMConfig import LArGMCfg

  acc = MainServicesCfg(ConfigFlags)
  acc.merge(LArGMCfg(ConfigFlags))

  from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg
  acc.merge(LArOnOffIdMappingCfg(ConfigFlags))

  if args.evtree: # should include trigger info
     from LArCafJobs.LArSCDumperSkeleton import L1CaloMenuCfg
     acc.merge(L1CaloMenuCfg(ConfigFlags))
     from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
     tdt = acc.getPrimaryAndMerge(TrigDecisionToolCfg(ConfigFlags))
  else: 
     tdt = None


  if args.bc:
     from LArBadChannelTool.LArBadChannelConfig import  LArBadFebCfg, LArBadChannelCfg
     acc.merge(LArBadChannelCfg(ConfigFlags))
     acc.merge(LArBadFebCfg(ConfigFlags))

  if args.geom:
      log.warning("Adding real geometry is not working yet")
      args.geom=False
      # FIXME
      #acc.addCondAlgo(CompFactory.CaloAlignCondAlg(LArAlignmentStore="",CaloCellPositionShiftFolder=""))
      #acc.addCondAlgo(CompFactory.CaloSuperCellAlignCondAlg())
      #AthReadAlg_ExtraInputs.append(('CaloSuperCellDetDescrManager', 'ConditionStore+CaloSuperCellDetDescrManager')) 

  from LArCalibTools.LArDigits2NtupleConfig import LArDigits2NtupleCfg
  acc.merge(LArDigits2NtupleCfg(ConfigFlags, AddBadChannelInfo=args.bc, AddFEBTempInfo=False, isSC=False, isFlat=True, 
                            OffId=args.offline, AddHash=args.ahash, AddCalib=args.calib, RealGeometry=args.geom, # from LArCond2NtupleBase 
                            NSamples=ConfigFlags.LArSCDump.nSamples, FTlist={}, ContainerKey=ConfigFlags.LArSCDump.digitsKey,  # from LArDigits2Ntuple
                            FillLB=args.evtree, 
                            OutputLevel=args.olevel
                           ))
  # ROOT file writing
  if os.path.exists(args.outfile):
      os.remove(args.outfile)
  acc.addService(CompFactory.NTupleSvc(Output = [ "FILE1 DATAFILE='"+args.outfile+"' OPT='NEW'" ]))
  acc.setAppProperty("HistogramPersistency","ROOT")

  # some logging
  log.info("Input files to be processed:")
  for f in ConfigFlags.Input.Files:
      log.info(f)
  log.info("Output file: ")
  log.info(args.outfile)


  # and run
  acc.run(args.maxev)
