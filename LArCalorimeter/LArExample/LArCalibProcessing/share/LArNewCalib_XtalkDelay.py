#!/usr/bin/env python
#
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#

if __name__=='__main__':

   import os,sys
   import argparse
   from AthenaCommon import Logging
   log = Logging.logging.getLogger( 'LArXtalkDelay' )

   # now process the CL options and assign defaults
   parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
   parser.add_argument('-r','--run', dest='run', default='00408918', help='Run number string as in input filename', type=str)
   parser.add_argument('-g','--gain', dest='gain', default="MEDIUM", help='Gain string', type=str)
   parser.add_argument('-p','--partition', dest='partition', default="Em", help='Data taking partition string', type=str)
   parser.add_argument('-f','--fileprefix', dest='fprefix', default="data25_calib", help='File prefix string', type=str)
   parser.add_argument('-d','--indir', dest='indir', default="", help='Full input dir string', type=str)
   parser.add_argument('-t','--trigger', dest='trig', default='calibration_', help='Trigger string in filename', type=str)
   parser.add_argument('-o','--outrwaveprefix', dest='outrwaveprefix', default="LArCaliWave", help='Prefix of CaliWave output root filename', type=str)
   parser.add_argument('-e','--outrdir', dest='outrdir', default="/eos/atlas/atlascerngroupdisk/det-larg/Temp/Weekly/ntuples", help='Output root file directory', type=str)
   parser.add_argument('-q','--insqlite', dest='insql', default="", help='Input sqlite file with pedestals, in pool output dir.', type=str)
   parser.add_argument('-m','--subdet', dest='subdet', default="EMB", help='Subdetector, EMB, EMEC, HEC, FCAL or HECFCAL', type=str)
   parser.add_argument('-s','--side', dest='side', default="C", help='Detector side empty (means both), C or A', type=str)
   parser.add_argument('-c','--isSC', dest='supercells', default=False, action="store_true", help='is SC data ?')
   parser.add_argument('-a','--isRawdata', dest='rawdata', default=False, action="store_true", help='is raw data ?')
   parser.add_argument('-x','--ignoreBarrel', dest='ignoreB', default=False, action="store_true", help='ignore Barrel channels ?')
   parser.add_argument('-v','--ignoreEndcap', dest='ignoreE', default=False, action="store_true", help='ignore Endcap channels ?')
   parser.add_argument('-b','--badchansqlite', dest='badsql', default="", help='Output sqlite file, in pool output dir.', type=str)
   parser.add_argument('-i','--pattdir', dest='pdir', default="", help='Full input pattern dir string', type=str)
   parser.add_argument('-j','--ntrigg', dest='ntrig', default=100, help='Number of trigger per step', type=int)
   parser.add_argument('-u','--usepatt', dest='pattern', default=1, help='Number of trigger per step', type=int)
   parser.add_argument('-n','--nsteps', dest='nstep', default=5, help='Number of delay steps', type=int)
   parser.add_argument('-y','--nsubstep', dest='nsubstep', default=1, help='Number of subststeps', type=int)
   parser.add_argument('--maxev', dest='maxev', default=-1, help='Number of events to process', type=int)

   args = parser.parse_args()

   for _, value in args._get_kwargs():
    if value is not None:
        print(value)

   if len(args.run) < 8:
      args.run = args.run.zfill(8)

   # now set flags according parsed options
   if args.indir != "":
      InputDir = args.indir
   else:
      log.error("Need input dir!")
      sys.exit(-1)

   #Import the configution-method we want to use (here: Pedestal and AutoCorr)
   from LArCalibProcessing.LArCalib_Delay_OFCCaliConfig import LArXtalkDelayCfg
   
   #Import the MainServices (boilerplate)
   from AthenaConfiguration.MainServicesConfig import MainServicesCfg
   
   #Import the flag-container that is the arguemnt to the configuration methods
   from AthenaConfiguration.AllConfigFlags import initConfigFlags
   from LArCalibProcessing.LArCalibConfigFlags import addLArCalibFlags
   flags=initConfigFlags()
   addLArCalibFlags(flags, args.supercells)

   #Now we set the flags as required for this particular job:
   #The following flags help finding the input bytestream files: 
   flags.LArCalib.Input.Dir = InputDir
   flags.LArCalib.Input.Type = args.trig
   flags.LArCalib.Input.RunNumbers = [int(args.run),]
   flags.LArCalib.Input.isRawData = args.rawdata
   gainNumMap={"HIGH":0,"MEDIUM":1,"LOW":2}
   flags.LArCalib.Gain=gainNumMap[args.gain.upper()]


   # Input files
   flags.Input.Files=flags.LArCalib.Input.Files
   #Print the input files we found 
   print ("Input files to be processed:")
   for f in flags.Input.Files:
       print (f)
   
   if len(flags.Input.Files) == 0 :
      print("Unable to find any input files. Please check the input directory:",InputDir)
      sys.exit(0)

   #Some configs depend on the sub-calo in question
   #(sets also the preselection of LArRawCalibDataReadingAlg)
   if not flags.LArCalib.isSC:
      if args.subdet == 'EMB' or args.subdet == 'EMEC':
         flags.LArCalib.Input.SubDet="EM"
      elif args.subdet:   
         flags.LArCalib.Input.SubDet=args.subdet
 
      if not args.side:   
         flags.LArCalib.Preselection.Side = [0,1]
      elif args.side == "C":
         flags.LArCalib.Preselection.Side = [0]
      elif args.side == "A":   
         flags.LArCalib.Preselection.Side = [1]
      else:   
         print("unknown side ",args.side)
         sys.exit(-1)
 
      if args.subdet != "EM":
         if args.subdet == 'EMB':
            flags.LArCalib.Preselection.BEC = [0]
         else:   
            flags.LArCalib.Preselection.BEC = [1]
 
      if args.subdet == 'FCAL':
         flags.LArCalib.Preselection.FT = [6]
      elif args.subdet == 'HEC':
         flags.LArCalib.Preselection.FT = [3,10,16,22]
      elif args.subdet == 'HECFCAL':
         flags.LArCalib.Preselection.FT = [3,6,10,16,22]
   
   #Configure the Bad-Channel database we are reading 
   #(the AP typically uses a snapshot in an sqlite file

   if args.badsql.startswith("/"):
      flags.LArCalib.BadChannelDB =  args.badsql
   else:        
      flags.LArCalib.BadChannelDB = "LAR_OFL"
   
   #Output of this job:
   OutputCaliWaveRootFileName = args.outrwaveprefix + "_" + args.run

   if args.subdet != "" and not flags.LArCalib.isSC:
      OutputCaliWaveRootFileName += "_"+args.subdet

      if flags.LArCalib.Input.SubDet=="EM":
         OutputCaliWaveRootFileName +=  args.side

   OutputCaliWaveRootFileName += ".root"

   flags.LArCalib.Output.ROOTFile = args.outrdir + "/" + OutputCaliWaveRootFileName
   #The global tag we are working with
   flags.IOVDb.GlobalTag = "LARCALIB-RUN2-00"
   
   #Other potentially useful flags-settings:
   
   # nsteps      
   flags.LArCalib.CaliWave.Nsteps=args.nstep
   # patterns file 
   pfile = args.pdir + '/parameters.dat'
   flags.LArCalib.Input.paramsFile = pfile
   # number of triggers (misusing OFC flags, which are not use in this job)
   flags.LArCalib.OFC.Ncoll = args.ntrig
   # number of substeps
   flags.LArCalib.CaliWave.NSubSteps = args.nsubstep
   # which pattern to use 
   flags.LArCalib.OFC.Nsamples = args.pattern

   if args.insql:
      flags.LArCalib.Input.Database = args.insql
   else:   
      flags.LArCalib.Input.Database = "LAR_ONL"
      flags.LArCalib.Pedestal.Folder = "/LAR/ElecCalibFlat/Pedestal"

   #Define the global output Level:
   from AthenaCommon.Constants import INFO,DEBUG
   flags.Exec.OutputLevel = INFO
   
   from AthenaConfiguration.Enums import LHCPeriod
   flags.GeoModel.Run = LHCPeriod.Run3

   from AthenaConfiguration.TestDefaults import defaultGeometryTags
   flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3

   flags.lock()
   flags.dump(evaluate=True)
   
   cfg=MainServicesCfg(flags)
   
   cfg.merge(LArXtalkDelayCfg(flags))

   if flags.LArCalib.isSC:
      fwversion=5
      # autoconfig
      from LArConditionsCommon.LArRunFormat import getLArDTInfoForRun
      try:
         runinfo=getLArDTInfoForRun(flags.Input.RunNumbers[0], connstring="COOLONL_LAR/CONDBR2")
         log.info("Got DT run info !")
      except Exception:
         log.warning("Could not get DT run info, using defaults !")
      else:   
         fwversion=runinfo.FWversion()   
      if args.fw6 or fwversion==6:
         from IOVDbSvc.IOVDbSvcConfig import addOverride
         cfg.merge(addOverride(flags,"/LAR/Identifier/LatomeMapping","LARIdentifierLatomeMapping-fw6"))   


   # ignore some channels ?
   if args.ignoreB:
      if args.rawdata:
         cfg.getEventAlgo("LArRawSCDataReadingAlg").LATOMEDecoder.IgnoreBarrelChannels=args.ignoreB
      else:
         cfg.getEventAlgo("LArRawSCCalibDataReadingAlg").LATOMEDecoder.IgnoreBarrelChannels=args.ignoreB
   if args.ignoreE:
      if args.rawdata:
         cfg.getEventAlgo("LArRawSCDataReadingAlg").LATOMEDecoder.IgnoreEndcapChannels=args.ignoreE
      else:
         cfg.getEventAlgo("LArRawSCCalibDataReadingAlg").LATOMEDecoder.IgnoreEndcapChannels=args.ignoreE

   if args.insql:
      cfg.getService("IOVDbSvc").DBInstance=""
   else:   
      cfg.getService("IOVDbSvc").DBInstance="LAR_OFL"

   #cfg.getService("MessageSvc").defaultLimit=2000000 #more messages
   #cfg.getEventAlgo("LArCalibDigitMaker").OutputLevel=DEBUG
   #cfg.getEventAlgo("LArCalibDigitsAccumulator").OutputLevel=2
   #cfg.getEventAlgo("LArCaliWaveBuilder").OutputLevel=DEBUG
   #cfg.getEventAlgo("LArCaliWaves2Ntuple").OutputLevel=DEBUG
   #cfg.getEventAlgo("LArCalibDigitsAccumulator").OutputLevel=2

   #run the application
   cfg.run(args.maxev) 

