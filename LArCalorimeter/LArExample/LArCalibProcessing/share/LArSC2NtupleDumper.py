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
  log = Logging.logging.getLogger( 'LArSC2Ntuple' )
  
  parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)

  parser.add_argument('-i','--indir', dest='indir', default="/eos/atlas/atlastier0/rucio/data_test/calibration_pulseall/00414414/data_test.00414414.calibration_pulseall.daq.RAW/", help='input files dir', type=str)
  parser.add_argument('-p','--inprefix', dest='inpref', default="data_test", help='Input filenames prefix', type=str)
  parser.add_argument('-y','--inppatt', dest='inppatt', default="lb3512", help='Input filenames pattern', type=str)
  parser.add_argument('-f','--infile', dest='infile', default="", help='Input filename (if given indir and inprefix are ignored', type=str)
  parser.add_argument('-r','--run', dest='run', default=0, help='Run number (if not given trying to judge from input file name)', type=int)
  parser.add_argument('-m','--maxev', dest='maxev', default=-1, help='Max number of events to dump', type=int)
  parser.add_argument('-x','--outlevel', dest='olevel', default=5, help='OuputLevel for dumping algo', type=int)
  parser.add_argument('-o','--outfile', dest='outfile', default="Digits.root", help='Output root filename', type=str)
  parser.add_argument('-s','--addSamples', dest='samples', default=False, help='Add Samples to output ntuple', action="store_true")
  parser.add_argument('-a','--addSampBas', dest='samplesBas', default=False, help='Add ADC_BAS to output ntuple', action="store_true")
  parser.add_argument('-z','--addEt', dest='Et', default=False, help='Add ET to output ntuple', action="store_true")
  parser.add_argument('-g','--addEtId', dest='EtId', default=False, help='Add ET_ID to output ntuple', action="store_true")
  parser.add_argument('-l','--noLatHeader', dest='lheader', default=True, help='Add LATOME Header to output ntuple', action='store_false')
  parser.add_argument('-b','--noBCID', dest='bcid', default=True, help='Add BCID info to output ntuple', action='store_false')
  parser.add_argument('-e','--expandId', dest='expid', default=False, help='Expand online Id to fields', action='store_true')
  parser.add_argument('-n','--nsamp', dest='nsamp', default=0, help='Number of samples to dump', type=int)
  parser.add_argument('-c','--overEvNumber', dest='overEvN', default=False, help='Overwrite event number', action='store_true')
  parser.add_argument('-d','--addHash', dest='ahash', default=False, help='Add hash number to output ntuple', action='store_true')
  parser.add_argument('-j','--addOffline', dest='offline', default=False, help='Add offline Id to output ntuple', action='store_true')
  parser.add_argument('-k','--addCalib', dest='calib', default=False, help='Add calib. info to output ntuple', action='store_true')
  parser.add_argument('-t','--addGeom', dest='geom', default=False, help='Add real geom info to output ntuple', action='store_true')
  parser.add_argument('-u','--noBC', dest='bc', default=False, help='Add Bad. chan info to output ntuple', action='store_true')
  parser.add_argument('-w','--addROD', dest='rod', default=False, help='Add ROD energies sum to output ntuple', action='store_true')
  parser.add_argument('-v','--addEvTree', dest='evtree', default=False, help='Add tree with per event info to output ntuple', action='store_true')
  parser.add_argument('-q','--addNoisyRO', dest='noisyRO', default=False, help='Add reco and info from LArNoisyROSummary to output ntuple', action='store_true')
  parser.add_argument('--addTT', dest='TT', default=False, help='Add info from LArTriggerTowers to output ntuple', action='store_true')

  args = parser.parse_args()
  if help in args and args.help is not None and args.help:
     parser.print_help()
     sys.exit(0)

  for _, value in args._get_kwargs():
     if value is not None:
       log.debug(value)

  #Import the flag-container that is the arguemnt to the configuration methods
  from AthenaConfiguration.AllConfigFlags import initConfigFlags
  flags=initConfigFlags()
  #add SC dumping specific flags
  from LArCafJobs.LArSCDumperFlags import addSCDumpFlags
  addSCDumpFlags(flags)

  if len(args.infile) > 0:
     flags.Input.Files = [args.infile]
  elif len(args.inppatt) > 0:
     from LArCalibProcessing.GetInputFiles import GetInputFilesFromPattern
     flags.Input.Files = GetInputFilesFromPattern(args.indir,args.inppatt)
  else:   
     from LArCalibProcessing.GetInputFiles import GetInputFilesFromPrefix
     flags.Input.Files = GetInputFilesFromPrefix(args.indir,args.inpref)

  if args.run != 0:
     flags.Input.RunNumbers = [args.run]

  # geometry
  from AthenaConfiguration.TestDefaults import defaultGeometryTags
  flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3

  # first autoconfig
  from LArConditionsCommon.LArRunFormat import getLArDTInfoForRun
  try:
     runinfo=getLArDTInfoForRun(flags.Input.RunNumbers[0], connstring="COOLONL_LAR/CONDBR2")
  except Exception:
     log.warning("Could not get DT run info, using defaults !")
     flags.LArSCDump.doEt=True
     if args.nsamp > 0:
        flags.LArSCDump.nSamples=args.nsamp
     else:   
        flags.LArSCDump.nSamples=5
     flags.LArSCDump.nEt=1
     flags.LArSCDump.digitsKey="SC"
     CKeys=["SC_ET"]
  else:
     CKeys=[]
     flags.LArSCDump.digitsKey=""
     for i in range(0,len(runinfo.streamTypes())):
        if args.EtId and runinfo.streamTypes()[i] ==  "SelectedEnergy":
              CKeys += ["SC_ET_ID"]
              flags.LArSCDump.doEt=True
              flags.LArSCDump.nEt=runinfo.streamLengths()[i]
        elif args.Et and runinfo.streamTypes()[i] ==  "Energy":
              CKeys += ["SC_ET"]
              flags.LArSCDump.doEt=True
              flags.LArSCDump.nEt=runinfo.streamLengths()[i]
        elif args.samples and runinfo.streamTypes()[i] ==  "RawADC":
              flags.LArSCDump.digitsKey="SC"
              if args.nsamp > 0:
                 flags.LArSCDump.nSamples=args.nsamp
              else:
                 flags.LArSCDump.nSamples=runinfo.streamLengths()[i]
        elif args.samplesBas and runinfo.streamTypes()[i] ==  "ADC":
              CKeys += ["SC_ADC_BAS"]
              if args.nsamp > 0:
                 flags.LArSCDump.nSamples=args.nsamp
              else:
                 flags.LArSCDump.nSamples=runinfo.streamLengths()[i]
     if  args.nsamp > 0 and args.nsamp < flags.LArSCDump.nSamples:
        flags.LArSCDump.nSamples=args.nsamp
  
  log.info("Autoconfigured: ")
  log.info("nSamples: %d nEt: %d digitsKey %s",flags.LArSCDump.nSamples, flags.LArSCDump.nEt, flags.LArSCDump.digitsKey)
  log.info(CKeys)

  # now set flags according parsed options
  #if args.samples and not ("SC" in CKeys or flags.LArSCDump.digitsKey=="SC"):
  #   log.warning("Samples asked, but they are not in RunLogger, no output !!!!")

  if args.samplesBas and "SC_ADC_BAS" not in CKeys:
     CKeys += ["SC_ADC_BAS"]
  if args.Et and "SC_ET" not in CKeys:
     CKeys += ["SC_ET"]
  if args.EtId and "SC_ET_ID" not in CKeys:
     CKeys += ["SC_ET_ID"]
  if args.lheader and "SC_LATOME_HEADER" not in CKeys:
     CKeys += ["SC_LATOME_HEADER"]

  if args.rod:
     flags.LArSCDump.doRawChan=True  
     CKeys += ["LArRawChannels"]
     log.info("Adding ROD energies")

  # now construct the job
  flags.LAr.doAlign=False

  if args.evtree: # should include trigger info
     flags.Trigger.triggerConfig = 'DB'
     flags.Trigger.L1.doCTP = True
     flags.Trigger.L1.doMuon = False
     flags.Trigger.L1.doCalo = False
     flags.Trigger.L1.doTopo = False

     flags.Trigger.enableL1CaloLegacy = True
     flags.Trigger.enableL1CaloPhase1 = False

  if args.TT:   
     flags.Trigger.L1.doCalo = True
     flags.Trigger.triggerConfig = 'DB'

  flags.LArSCDump.fillNoisyRO=args.noisyRO
  # in case stores needs to be debugged:
  #from AthenaCommon.Constants import DEBUG
  #flags.Exec.OutputLevel=DEBUG
  #flags.Debug.DumpCondStore=True
  #flags.Debug.DumpDetStore=True
  #flags.Debug.DumpEvtStore=True

  flags.lock()
  flags.dump('LArSCDump.*')


  #Import the MainServices (boilerplate)
  from AthenaConfiguration.MainServicesConfig import MainServicesCfg
  from LArGeoAlgsNV.LArGMConfig import LArGMCfg

  acc = MainServicesCfg(flags)
  acc.merge(LArGMCfg(flags))

  if args.evtree: # should include trigger info
     from LArCafJobs.LArSCDumperSkeleton import L1CaloMenuCfg
     acc.merge(L1CaloMenuCfg(flags))
     from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
     tdt = acc.getPrimaryAndMerge(TrigDecisionToolCfg(flags))
  else: 
     tdt = None


  if args.bc:
     from LArBadChannelTool.LArBadChannelConfig import  LArBadFebCfg, LArBadChannelCfg
     acc.merge(LArBadChannelCfg(flags,None,True))
     acc.merge(LArBadFebCfg(flags))

  if args.geom:
      acc.addCondAlgo(CompFactory.CaloAlignCondAlg(LArAlignmentStore="",CaloCellPositionShiftFolder=""))
      acc.addCondAlgo(CompFactory.CaloSuperCellAlignCondAlg())

  from LArCalibTools.LArSC2NtupleConfig import LArSC2NtupleCfg
  acc.merge(LArSC2NtupleCfg(flags, AddBadChannelInfo=args.bc, AddFEBTempInfo=False, isSC=True, isFlat=False, 
                            OffId=args.offline, AddHash=args.ahash, AddCalib=args.calib, RealGeometry=args.geom, ExpandId=args.expid, # from LArCond2NtupleBase 
                            NSamples=flags.LArSCDump.nSamples, FTlist=[], FillBCID=args.bcid, ContainerKey=flags.LArSCDump.digitsKey,  # from LArDigits2Ntuple
                            SCContainerKeys=CKeys, OverwriteEventNumber = args.overEvN,                        # from LArSC2Ntuple
                            FillRODEnergy = flags.LArSCDump.doRawChan,
                            FillLB=args.evtree, FillTriggerType = args.evtree,
                            TrigNames=["L1_EM3","L1_EM7","L1_EM15","L1_EM22VHI","L1_eEM5","L1_eEM15","L1_eEM22M"],
                            TrigDecisionTool=tdt, FillTriggerTowers = args.TT,
                            OutputLevel=args.olevel
                           ))
  # ROOT file writing
  if os.path.exists(args.outfile):
      os.remove(args.outfile)
  acc.addService(CompFactory.NTupleSvc(Output = [ "FILE1 DATAFILE='"+args.outfile+"' OPT='NEW'" ]))
  acc.setAppProperty("HistogramPersistency","ROOT")

  # some logging
  log.info("Input files to be processed:")
  for f in flags.Input.Files:
      log.info(f)
  log.info("Output file: ")
  log.info(args.outfile)


  # and run
  acc.run(args.maxev)
