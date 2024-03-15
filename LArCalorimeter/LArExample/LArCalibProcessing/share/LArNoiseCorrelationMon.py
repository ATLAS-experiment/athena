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
  log = Logging.logging.getLogger( 'LArNoiseCorrelation' )
  
  parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
  parser.add_argument('-i','--indir', dest='indir', default="/eos/atlas/atlastier0/rucio/data_test/calibration_pulseall/00414414/data_test.00414414.calibration_pulseall.daq.RAW/", help='input files dir', type=str)
  parser.add_argument('-p','--inprefix', dest='inpref', default="data_test", help='Input filenames prefix', type=str)
  parser.add_argument('-y','--inppatt', dest='inppatt', default="lb3512", help='Input filenames pattern', type=str)
  parser.add_argument('-l','--infile', dest='infile', default="", help='Input filename (if given indir and inprefix are ignored', type=str)
  parser.add_argument('-r','--run', dest='run', default=0, help='Run number (if not given trying to judge from input file name)', type=int)
  parser.add_argument('-n','--cnf', dest='cnf', default=False, action="store_true", help='Run also CNF computation')
  parser.add_argument('-c','--calib', dest='calib', default=False, action="store_true", help='Is this calibration run ?')
  parser.add_argument('-g','--gain', dest='gain', default="HIGH", help='Gain in case of calib. data', type=str)
  parser.add_argument('-f','--febs', dest='febs', default=[], nargs="+", help='space-separated list of FEB names, which will be monitored', type=str)
  parser.add_argument('-m','--maxev', dest='maxev', default=-1, help='Max number of events to dump', type=int)
  parser.add_argument('-s','--skipev', dest='skipev', default=-1, help='events to skip', type=int)
  parser.add_argument('-x','--outlevel', dest='olevel', default=3, help='OuputLevel for dumping algo', type=int)
  parser.add_argument('-o','--outfile', dest='outfile', default="LArNoiseCorr.root", help='Output root filename', type=str)
  parser.add_argument('-a','--postproc', dest='postp', default=False, action="store_true", help='Do a postprocessing ?')
  parser.add_argument('-t','--FTs', dest='ft', default=[], nargs="+", type=int, help='list of FT which will be read out (space separated).')
  parser.add_argument('-v','--posneg', dest='posneg', default=[], nargs="+", help='side to read out (-1 means both), can give multiple arguments (space separated). Default %(default)s.', type=int,choices=range(-1,2))
  parser.add_argument('-b','--barrel_ec', dest='be', default=[], nargs="+", help='subdet to read out (-1 means both), can give multiple arguments (space separated) Default %(default)s.', type=int,choices=range(-1,2))

  args = parser.parse_args()
  if args.help:
     parser.print_help()
     sys.exit(0)

  for _, value in args._get_kwargs():
     if value is not None:
       log.debug(value)

  # Import the flag-container that is the argument to the configuration methods
  from AthenaConfiguration.AllConfigFlags import initConfigFlags
  from LArMonitoring.LArMonConfigFlags import addLArMonFlags
  from LArCalibProcessing.LArCalibConfigFlags import addLArCalibFlags
  flags=initConfigFlags()
  flags.addFlagsCategory("LArMon", addLArMonFlags)
  addLArCalibFlags(flags)


  if len(args.infile) > 0:
     flags.Input.Files = [args.infile]
  elif len(args.inppatt) > 0:
     from LArCalibProcessing.GetInputFiles import GetInputFilesFromPattern
     flags.Input.Files = GetInputFilesFromPattern(args.indir,args.inppatt)
  else:   
     from LArCalibProcessing.GetInputFiles import GetInputFilesFromPrefix
     flags.Input.Files = GetInputFilesFromPrefix(args.indir,args.inpref)

  if args.run != 0:
     flags.Input.RunNumber = [args.run]

  if len(args.febs) > 0:
     flags.LArMon.customFEBsToMonitor = args.febs 

  if args.calib:
     flags.LArMon.LArDigitKey = args.gain
     flags.LArMon.calibRun = True
  else:   
     flags.LArMon.LArDigitKey = 'FREE' 

  if len(args.posneg) >= 0:
     flags.LArCalib.Preselection.Side = args.posneg
  if len(args.be) >=0:
     flags.LArCalib.Preselection.BEC = args.be
  if len(args.ft) > 0:
     flags.LArCalib.Preselection.FT = args.ft


  flags.Output.HISTFileName = args.outfile
  flags.DQ.enableLumiAccess = False
  flags.DQ.useTrigger = False
  flags.lock()   

  from AthenaConfiguration.MainServicesConfig import MainServicesCfg
  cfg = MainServicesCfg(flags)
  from LArCalibProcessing.LArCalibBaseConfig import LArCalibBaseCfg
  cfg.merge(LArCalibBaseCfg(flags))

  if args.calib:
     from LArByteStream.LArRawCalibDataReadingConfig import LArRawCalibDataReadingCfg 
     cfg.merge(LArRawCalibDataReadingCfg(flags,gain=flags.LArMon.LArDigitKey,doDigit=True))
  else:   
     from LArByteStream.LArRawDataReadingConfig import LArRawDataReadingCfg
     cfg.merge(LArRawDataReadingCfg(flags,LArDigitKey=flags.LArMon.LArDigitKey))
     if len(flags.LArCalib.Preselection.Side) > 0 or len(flags.LArCalib.Preselection.BEC) > 0 or len(flags.LArCalib.Preselection.FT) > 0:
        log.warning('No preselection yet in reading physics data !!!') 
            
  # we need pedestals
  from LArConfiguration.LArElecCalibDBConfig import LArElecCalibDBCfg
  cfg.merge(LArElecCalibDBCfg(flags,["Pedestal"]))

  from LArMonitoring.LArNoiseCorrelationMonAlg import LArNoiseCorrelationMonConfig
  cfg.merge(LArNoiseCorrelationMonConfig(flags))

  if args.cnf:
    from LArMonitoring.LArNoiseCorrelationMonAlg import LArCoherentNoisefractionConfig
    cfg.merge(LArCoherentNoisefractionConfig(flags))

  if args.postp:
     # needs to add postprocessing
     from DataQualityUtils.DQPostProcessingAlg import DQPostProcessingAlg
     ppa = DQPostProcessingAlg("DQPostProcessingAlg")
     ppa.Interval = 1000000 # Big number (>evtMax) to do postprocessing during finalization
     rn=flags.Input.RunNumber[0]
     ppa.FileKey = f'/{flags.DQ.FileKey}/run_{rn}/'
     cfg.addEventAlgo(ppa, sequenceName='AthEndSeq')
  
  if args.skipev > 0:
     cfg.getService("EventSelector").SkipEvents=int(args.skipev)

  # and run
  cfg.run(args.maxev)

