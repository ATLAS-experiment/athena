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
  parser.add_argument(     '--addAccSamples', dest='accsamples', default=False, help='work on accumulated samples', action="store_true")
  parser.add_argument(     '--addAccCalibSamples', dest='acccalibsamples', default=False, help='work on accumulated samples', action="store_true")
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
  parser.add_argument('--EMF', dest='emf', default=False, help='Is it for EMF', action='store_true')
  parser.add_argument('--FW6', dest='fw6', default=False, help='Is it for fw v. 6', action='store_true')
  parser.add_argument('--FTs', dest='ft', default=[], nargs="+", type=int, help='list of FT which will be read out (space separated).')
  parser.add_argument('--posneg', dest='posneg', default=[], nargs="+", help='side to read out (-1 means both), can give multiple arguments (space separated). Default %(default)s.', type=int,choices=range(-1,2))
  parser.add_argument('--barrel_ec', dest='be', default=[], nargs="+", help='subdet to read out (-1 means both), can give multiple arguments (space separated) Default %(default)s.', type=int,choices=range(-1,2))
  parser.add_argument('--ETThresh', dest='etthresh', default=-1., help='ET threshold to dump info', type=float)
  parser.add_argument('--ETThreshMain', dest='etthreshmain', default=-1., help='ET threshold from Main to dump info', type=float)
  parser.add_argument('--ADCThresh', dest='adcthresh', default=-1, help='ADC threshold to dump info', type=int)

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
  if args.accsamples or args.acccalibsamples:
    from LArCalibProcessing.LArCalibConfigFlags import addLArCalibFlags
    addLArCalibFlags(flags, True)
    if len(args.posneg) >= 0:
       flags.LArCalib.Preselection.Side = args.posneg
    if len(args.be) >=0:
       flags.LArCalib.Preselection.BEC = args.be
    if len(args.ft) > 0:
       flags.LArCalib.Preselection.FT = args.ft   
  #add SC dumping specific flags
  from LArCafJobs.LArSCDumperFlags import addSCDumpFlags
  addSCDumpFlags(flags)

  # check samples combination:
  if (args.accsamples or args.acccalibsamples) and (args.samples or args.samplesBas):
     log.error('Could not dump both samples and accumulated calib samples')
     sys.exit(1)
  if args.accsamples and args.acccalibsamples:
     log.error('Could not dump both accsamples and acc calib samples')
     sys.exit(1)

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

  if args.accsamples:
     flags.LArSCDump.accdigitsKey="accSC"
  else:   
     flags.LArSCDump.accdigitsKey=""
  if args.acccalibsamples:
     flags.LArSCDump.acccalibdigitsKey="acccalibSC"
  else:   
     flags.LArSCDump.acccalibdigitsKey=""

  flags.LArSCDump.doBC = args.bc
  bckey='LArBadChannel'

  flags.LArSCDump.digitsKey=""
  CKeys=[]
  fwversion=5

  #  autoconfig
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
     if args.samples:
        flags.LArSCDump.digitsKey="SC"
     CKeys=["SC_ET"]
     log.debug(runinfo.streamTypes(), ' ',runinfo.streamLengths())
  else:
     fwversion=runinfo.FWversion()   
     if not (args.accsamples or args.acccalibsamples):   
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
  
  # calib runs do not have info about accumulation
  if args.accsamples or args.acccalibsamples:
     flags.Input.OverrideRunNumber = True

  # now set flags according parsed options
  #if args.samples and not ("SC" in CKeys or flags.LArSCDump.digitsKey=="SC"):
  #   log.warning("Samples asked, but they are not in RunLogger, no output !!!!")

  if args.samples and not ("SC" in flags.LArSCDump.digitsKey):
     flags.LArSCDump.digitsKey="SC" 
  if args.samplesBas and "SC_ADC_BAS" not in CKeys:
     CKeys += ["SC_ADC_BAS"]
     flags.LArSCDump.doSamplesBas=True
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

  log.info("Autoconfigured: ")
  log.info("nSamples: %d nEt: %d digitsKey %s accdigitsKey %s acccalibdigitsKey %s",flags.LArSCDump.nSamples, flags.LArSCDump.nEt, flags.LArSCDump.digitsKey, flags.LArSCDump.accdigitsKey, flags.LArSCDump.acccalibdigitsKey)
  log.info(CKeys)

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
  flags.Exec.OutputLevel=args.olevel
  #flags.Debug.DumpCondStore=True
  #flags.Debug.DumpDetStore=True
  #flags.Debug.DumpEvtStore=True

  if args.emf:
     # additions for EMF
     flags.IOVDb.SqliteInput="/afs/cern.ch/user/p/pavol/public/EMF_otherCond.db"
     flags.IOVDb.SqliteFolders = ("/LAR/BadChannelsOfl/BadChannelsSC","/LAR/BadChannels/BadChannelsSC","/LAR/Identifier/OnOffIdMap",)
    
  if args.etthresh > 0.:
     flags.LArSCDump.ETThresh = args.etthresh

  if args.etthreshmain > 0.:
     flags.LArSCDump.ETThreshMain = args.etthreshmain

  flags.lock()
  flags.dump('LArSCDump.*')



  #Import the MainServices (boilerplate)
  from AthenaConfiguration.MainServicesConfig import MainServicesCfg
  from LArGeoAlgsNV.LArGMConfig import LArGMCfg

  acc = MainServicesCfg(flags)
  acc.merge(LArGMCfg(flags))

  if args.accsamples:
     from LArCalibProcessing.LArCalibBaseConfig import LArCalibBaseCfg
     acc.merge(LArCalibBaseCfg(flags))
     from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
     acc.merge(ByteStreamReadCfg(flags))

  if args.evtree: # should include trigger info
     from LArCafJobs.LArSCDumperSkeleton import L1CaloMenuCfg
     acc.merge(L1CaloMenuCfg(flags))
     from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
     tdt = acc.getPrimaryAndMerge(TrigDecisionToolCfg(flags))
  else: 
     tdt = None


  if args.fw6 or fwversion==6:
     # addition for new firmware
     from IOVDbSvc.IOVDbSvcConfig import addOverride
     if args.emf:
        acc.merge(addOverride(flags,"/LAR/Identifier/LatomeMapping","LARIdentifierLatomeMapping-emf-fw6"))
     else:   
        acc.merge(addOverride(flags,"/LAR/Identifier/LatomeMapping","LARIdentifierLatomeMapping-fw6"))
  if args.bc:
     from LArBadChannelTool.LArBadChannelConfig import  LArBadFebCfg, LArBadChannelCfg
     acc.merge(LArBadChannelCfg(flags,None,True))
     acc.merge(LArBadFebCfg(flags))
     bckey+='SC'

  if args.geom:
     acc.addCondAlgo(CompFactory.CaloAlignCondAlg(LArAlignmentStore="",CaloCellPositionShiftFolder=""))
     acc.addCondAlgo(CompFactory.CaloSuperCellAlignCondAlg())
     args.offline=True

  from LArCalibTools.LArSC2NtupleConfig import LArSC2NtupleCfg
  acc.merge(LArSC2NtupleCfg(flags, isEmf = args.emf, AddBadChannelInfo=args.bc, AddFEBTempInfo=False, isSC=True, isFlat=False, 
                            OffId=args.offline, AddHash=args.ahash, AddCalib=args.calib, RealGeometry=args.geom, ExpandId=args.expid, BadChanKey=bckey, # from LArCond2NtupleBase 
                            NSamples=flags.LArSCDump.nSamples, FTlist=[], FillBCID=args.bcid, ContainerKey=flags.LArSCDump.digitsKey, AccContainerKey=flags.LArSCDump.accdigitsKey, AccCalibContainerKey=flags.LArSCDump.acccalibdigitsKey,# from LArDigits2Ntuple
                            SCContainerKeys=CKeys, OverwriteEventNumber = args.overEvN,                        # from LArSC2Ntuple
                            FillRODEnergy = flags.LArSCDump.doRawChan,
                            FillLB=args.evtree, FillTriggerType = args.evtree,
                            ETThreshold = flags.LArSCDump.ETThresh, ETThresholdMain = flags.LArSCDump.ETThreshMain, ADCThreshold=args.adcthresh,
                            TrigNames=["L1_EM3","L1_EM7","L1_EM15","L1_EM22VHI","L1_eEM5","L1_eEM15","L1_eEM22M"],
                            TrigDecisionTool=tdt, FillTriggerTowers = args.TT,
                            OutputLevel=args.olevel
                           ))
  # ROOT file writing
  if os.path.exists(args.outfile):
      os.remove(args.outfile)
  acc.addService(CompFactory.NTupleSvc(Output = [ "FILE1 DATAFILE='"+args.outfile+"' OPT='NEW'" ]))
  acc.setAppProperty("HistogramPersistency","ROOT")

  # calib runs do not have proper run number in metadata
  if args.accsamples or  args.acccalibsamples:
     acc.getService("IOVDbSvc").forceRunNumber=int(args.run) 
  
  acc.getService("MessageSvc").defaultLimit=999999

  # some logging
  acc.getService("MessageSvc").defaultLimit=99999999 
  log.info("Input files to be processed:")
  for f in flags.Input.Files:
      log.info(f)
  log.info("Output file: ")
  log.info(args.outfile)

  # and run
  acc.run(args.maxev)
