#!/usr/bin/env python
#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory

if __name__=='__main__':

  import os,sys
  import argparse
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
  parser.add_argument('-a','--addHash', dest='ahash', default=False, help='Add hash number to output ntuple', action="store_true")
  parser.add_argument('-j','--addOffline', dest='offline', default=False, help='Add offline Id to output ntuple', action="store_true")
  parser.add_argument('-k','--addCalib', dest='calib', default=False, help='Add calib. info to output ntuple', action="store_true")
  parser.add_argument('-t','--addGeom', dest='geom', default=False, help='Add real geom info to output ntuple', action="store_true")
  parser.add_argument('-u','--addBC', dest='bc', default=False, help='Add Bad. chan info to output ntuple', action="store_true")
  parser.add_argument('--addBCID', dest='bcid', default=False, help='Add BCID info to output ntuple', action="store_true")
  parser.add_argument('-v','--addEvTree', dest='evtree', default=False, help='Add tree with per event info to output ntuple', action="store_true")
  parser.add_argument('--EMF', dest='emf', default=False, help='Is it for EMF', action='store_true')
  parser.add_argument('-d','--digikey', dest='dkey', default="FREE", help='Input digits key', type=str)
  parser.add_argument('-e','--acckey', dest='acckey', default="", help='Input accumulated calib digits key', type=str)
  parser.add_argument('--CALIB', dest='iscalib', default=False, help='Is it from calib run ?', action='store_true')
  parser.add_argument('--FTs', dest='ft', default=[], nargs="+", type=int, help='list of FT which will be read out (space separated).')
  parser.add_argument('--posneg', dest='posneg', default=[], nargs="+", help='side to read out (-1 means both), can give multiple arguments (space separated). Default %(default)s.', type=int,choices=range(-1,2))
  parser.add_argument('--barrel_ec', dest='be', default=[], nargs="+", help='subdet to read out (-1 means both), can give multiple arguments (space separated) Default %(default)s.', type=int,choices=range(-1,2))

  args = parser.parse_args()
  if help in args and args.help is not None and args.help:
     parser.print_help()
     sys.exit(0)

  for _, value in args._get_kwargs():
     if value is not None:
       log.debug(value)

  #Import the flag-container that is the arguemnt to the configuration methods
  from AthenaConfiguration.AllConfigFlags import initConfigFlags
  flags = initConfigFlags()

  # add SCDump flags, here re-used for digitsdump
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

  from LArCalibProcessing.LArCalibConfigFlags import addLArCalibFlags
  addLArCalibFlags(flags)  
  if len(args.posneg) >= 0:
     flags.LArCalib.Preselection.Side = args.posneg
  if len(args.be) >=0:
     flags.LArCalib.Preselection.BEC = args.be
  if len(args.ft) > 0:
     flags.LArCalib.Preselection.FT = args.ft   
  # first autoconfig
  from LArConditionsCommon.LArRunFormat import getLArFormatForRun
  try:
     runinfo=getLArFormatForRun(flags.Input.RunNumbers[0], connstring="COOLONL_LAR/CONDBR2")
  except Exception:
     log.warning("Could not get  run info, using defaults !")
     if args.nsamp > 0:
        flags.LArSCDump.nSamples=args.nsamp
     else:   
        flags.LArSCDump.nSamples=4
  else:
     flags.LArSCDump.nSamples=runinfo.nSamples()

  flags.LArSCDump.digitsKey=args.dkey
  if  args.nsamp > 0 and args.nsamp < flags.LArSCDump.nSamples:
      flags.LArSCDump.nSamples=args.nsamp
  
  log.info("Autoconfigured: ")
  log.info("nSamples: %d digitsKey %s accKey %s",flags.LArSCDump.nSamples, flags.LArSCDump.digitsKey, args.acckey)

  #GEometry
  from AthenaConfiguration.TestDefaults import defaultGeometryTags
  flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3

  # now construct the job
  flags.LAr.doAlign=False

  if args.evtree: # should include trigger info
     flags.Trigger.triggerConfig = 'DB'
     flags.Trigger.L1.doCTP = True
     flags.Trigger.L1.doMuon = False
     flags.Trigger.L1.doCalo = False
     flags.Trigger.L1.doTopo = False

     flags.Trigger.enableL1CaloLegacy = True
     flags.Trigger.enableL1CaloPhase1 = True

  # additions for EMF
  if args.emf:
     flags.IOVDb.SqliteInput="/afs/cern.ch/user/p/pavol/public/EMF_otherCond.db"
     flags.IOVDb.SqliteFolders = ("/LAR/BadChannelsOfl/BadChannels","/LAR/BadChannelsOfl/KnownBADFEBs","/LAR/BadChannelsOfl/KnownMNBFEBs","/LAR/BadChannelsOfl/MissingFEBs","/LAR/Identifier/OnOffIdMap",)

  flags.lock()
  flags.dump()  

  #Import the MainServices (boilerplate)
  from AthenaConfiguration.MainServicesConfig import MainServicesCfg
  from LArGeoAlgsNV.LArGMConfig import LArGMCfg

  acc = MainServicesCfg(flags)
  acc.merge(LArGMCfg(flags))

  from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg
  acc.merge(LArOnOffIdMappingCfg(flags))

  if args.evtree: # should include trigger info
     from LArCafJobs.LArSCDumperSkeleton import L1CaloMenuCfg
     acc.merge(L1CaloMenuCfg(flags))
     from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
     tdt = acc.getPrimaryAndMerge(TrigDecisionToolCfg(flags))
  else: 
     tdt = None


  if args.bc:
     from LArBadChannelTool.LArBadChannelConfig import  LArBadFebCfg, LArBadChannelCfg
     acc.merge(LArBadChannelCfg(flags))
     acc.merge(LArBadFebCfg(flags))

  if args.geom:
      log.warning("Adding real geometry is not working yet")
      args.geom=False
      # FIXME
      #acc.addCondAlgo(CompFactory.CaloAlignCondAlg(LArAlignmentStore="",CaloCellPositionShiftFolder=""))
      #acc.addCondAlgo(CompFactory.CaloSuperCellAlignCondAlg())
      #AthReadAlg_ExtraInputs.append(('CaloSuperCellDetDescrManager', 'ConditionStore+CaloSuperCellDetDescrManager')) 

  from LArCalibTools.LArDigits2NtupleConfig import LArDigits2NtupleCfg
  acc.merge(LArDigits2NtupleCfg(flags, AddBadChannelInfo=args.bc, AddFEBTempInfo=False, isSC=False, isFlat=True, 
                            OffId=args.offline, AddHash=args.ahash, AddCalib=args.calib, RealGeometry=args.geom, # from LArCond2NtupleBase 
                            NSamples=flags.LArSCDump.nSamples, FTlist=flags.LArCalib.Preselection.FT, Slotlist=flags.LArCalib.Preselection.Slot, BElist=flags.LArCalib.Preselection.BEC, Sidelist=flags.LArCalib.Preselection.Side,
                            ContainerKey=flags.LArSCDump.digitsKey,  AccContainerKey=args.acckey, isCalib=args.iscalib, # from LArDigits2Ntuple
                            FillLB=args.evtree, FillBCID=args.bcid, 
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
