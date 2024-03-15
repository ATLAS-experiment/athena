#!/usr/bin/env python
#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

if __name__=='__main__':

   import os,sys
   import argparse
   import subprocess
   from AthenaCommon import Logging
   log = Logging.logging.getLogger( 'LArPedestalAutoCorr' )

   MinSample      = -1 
   MaxSample      = -1

   # now process the CL options and assign defaults
   parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
   #parser.add_argument('-r','--runlist', dest='runlist', default=RunNumberList, nargs='+', help='Run numbers string list', type=str)
   parser.add_argument('-r','--run', dest='run', default='00408913', help='Run number string as in input filename', type=str)
   parser.add_argument('-g','--gain', dest='gain', default="MEDIUM", help='Gain string', type=str)
   parser.add_argument('-p','--partition', dest='partition', default="All", help='Partition string', type=str)
   parser.add_argument('-f','--fileprefix', dest='fprefix', default="data24_calib", help='File prefix string', type=str)
   parser.add_argument('-i','--indirprefix', dest='dprefix', default="/eos/atlas/atlastier0/rucio/", help='Input directory prefix string', type=str)
   parser.add_argument('-d','--indir', dest='indir', default="", help='Full input dir string', type=str)
   parser.add_argument('-t','--trigger', dest='trig', default='calibration_', help='Trigger string in filename', type=str)
   parser.add_argument('-o','--outrprefix', dest='outrprefix', default="LArPedAutoCorr", help='Prefix of output root filename', type=str)
   parser.add_argument('-l','--outpprefix', dest='outpprefix', default="LArPedAutoCorr", help='Prefix of output pool filename', type=str)
   parser.add_argument('-e','--outrdir', dest='outrdir', default="/eos/atlas/atlascerngroupdisk/det-larg/Temp/Weekly/ntuples", help='Output root file directory', type=str)
   parser.add_argument('-k','--outpdir', dest='outpdir', default="/eos/atlas/atlascerngroupdisk/det-larg/Temp/Weekly/poolFiles", help='Output pool file directory', type=str)
   parser.add_argument('-n','--outsqlite', dest='outsql', default="mysql.db", help='Output sqlite file, in pool output dir.', type=str)
   parser.add_argument('-m','--subdet', dest='subdet', default="EMB", help='Subdetector, EMB, EMEC, HEC or FCAL', type=str)
   parser.add_argument('-s','--side', dest='side', default="C", help='Detector side empty (means both), C or A', type=str)
   parser.add_argument('-c','--isSC', dest='supercells', default=False, action="store_true", help='is SC data ?')
   parser.add_argument('-a','--isRawdata', dest='rawdata', default=False, action="store_true", help='is raw data ?')
   parser.add_argument('-b','--badchansqlite', dest='badsql', default="SnapshotBadChannel.db", help='Input sqlite file with bad chans.', type=str)

   args = parser.parse_args()
   if help in args and args.help is not None and args.help:
      parser.print_help()
      sys.exit(0)

   for _, value in args._get_kwargs():
    if value is not None:
        log.debug(value)

   if len(args.run) < 8:
      args.run = args.run.zfill(8)

   # now set flags according parsed options
   if args.indir != "":
      InputDir = args.indir
   else:
      gain=args.gain.lower().capitalize()
      

      if not args.supercells:
         partstr = args.partition
      else:
         partstr = args.partition+"-DT"
      if args.rawdata:
            partstr += "-RawData"
      # here - add optional nsamples
      InputDir = args.dprefix+args.fprefix+"/calibration_LArElec-Pedestal-32s-"+gain+"-"+partstr+"/"+args.run+"/"+args.fprefix+"."+args.run+".calibration_LArElec-Pedestal-32s-"+gain+"-"+partstr+".daq.RAW/"

   #Import the configution-method we want to use (here: Pedestal and AutoCorr)
   from LArCalibProcessing.LArCalib_PedestalAutoCorrConfig import LArPedestalAutoCorrCfg
   
   #Import the MainServices (boilerplate)
   from AthenaConfiguration.MainServicesConfig import MainServicesCfg
   
   #Import the flag-container that is the arguemnt to the configuration methods
   from AthenaConfiguration.AllConfigFlags import initConfigFlags
   from LArCalibProcessing.LArCalibConfigFlags import addLArCalibFlags
   flags=initConfigFlags() 
   #first needs to define, if we are with SC or not
   addLArCalibFlags(flags, args.supercells)
   
   #Now we set the flags as required for this particular job:
   #The following flags help finding the input bytestream files: 
   flags.LArCalib.Input.Dir = InputDir
   flags.LArCalib.Input.Type = args.trig
   flags.LArCalib.Input.RunNumbers = [int(args.run),]
   flags.LArCalib.Input.isRawData = args.rawdata

   from AthenaConfiguration.TestDefaults import defaultGeometryTags
   flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3

   # Input files
   flags.Input.Files=flags.LArCalib.Input.Files
   #Print the input files we found 
   log.info("Input files to be processed:")
   for f in flags.Input.Files:
       log.info(f)
   
   if len(flags.Input.Files) == 0 :
      print("Unable to find any input files. Please check the input directory:",InputDir)
      sys.exit(0)

   
   #Some configs depend on the sub-calo in question
   #(sets also the preselection of LArRawCalibDataReadingAlg)
   if not flags.LArCalib.isSC:
      if args.subdet == 'EMB' or args.subdet == 'EMEC':
         flags.LArCalib.Input.SubDet="EM"
      else:   
         flags.LArCalib.Input.SubDet=args.subdet
 
      if args.side !="":
         if args.side == "C":
            flags.LArCalib.Preselection.Side = [0]
         elif args.side == "A":   
            flags.LArCalib.Preselection.Side = [1]
         else:   
            log.warning("Bad side argument: ",args.side," using both!!") 
 
      if args.subdet == 'EMB':
         flags.LArCalib.Preselection.BEC = [0]
      else:   
         flags.LArCalib.Preselection.BEC = [1]
 
      if args.subdet == 'FCAL':
         flags.LArCalib.Preselection.FT = [6]
      elif args.subdet == 'HEC':
         flags.LArCalib.Preselection.FT = [3,10,16,22]

   #Output of this job:
   OutputPedAutoCorrRootFileName = args.outrprefix + "_" + args.run
   OutputPedAutoCorrPoolFileName = args.outpprefix + "_" + args.run
      
   if args.subdet != "" and not flags.LArCalib.isSC:
      OutputPedAutoCorrRootFileName = OutputPedAutoCorrRootFileName + "_"+args.subdet
      OutputPedAutoCorrPoolFileName = OutputPedAutoCorrPoolFileName + "_"+args.subdet
      if flags.LArCalib.Input.SubDet=="EM":
         OutputPedAutoCorrRootFileName = OutputPedAutoCorrRootFileName + args.side
         OutputPedAutoCorrPoolFileName = OutputPedAutoCorrPoolFileName + args.side
   OutputPedAutoCorrRootFileName = OutputPedAutoCorrRootFileName + ".root"
   OutputPedAutoCorrPoolFileName = OutputPedAutoCorrPoolFileName + ".pool.root"

   flags.LArCalib.Output.ROOTFile = args.outrdir + "/" + OutputPedAutoCorrRootFileName
   flags.LArCalib.Output.POOLFile = args.outpdir + "/" + OutputPedAutoCorrPoolFileName
   if args.outsql.startswith("/"):
      flags.IOVDb.DBConnection="sqlite://;schema=" + args.outsql +";dbname=CONDBR2"
   else:   
      flags.IOVDb.DBConnection="sqlite://;schema="+args.outpdir + "/" + args.outsql +";dbname=CONDBR2"

   #The global tag we are working with
   flags.IOVDb.GlobalTag = "LARCALIB-RUN2-00"
   
   #BadChannels sqlite file to be created 
   if args.badsql.startswith("/"):
      flags.LArCalib.BadChannelDB =  args.badsql
   else:   
      flags.LArCalib.BadChannelDB = args.outpdir + "/" + args.badsql

   #Other potentially useful flags-settings:
   
   #Define the global output Level:
   from AthenaCommon.Constants import INFO
   flags.Exec.OutputLevel = INFO

   from AthenaConfiguration.Enums import LHCPeriod
   flags.GeoModel.Run = LHCPeriod.Run3

   flags.lock()
   
   # create bad chan sqlite file
   cmdlinerm = (['/bin/rm', '-f', flags.LArCalib.BadChannelDB])
   if not flags.LArCalib.isSC:
      cmdline = (['AtlCoolCopy', 'COOLOFL_LAR/CONDBR2', 'sqlite://;schema='+flags.LArCalib.BadChannelDB+';dbname=CONDBR2', '-f', '/LAR/BadChannelsOfl/BadChannels',  '-f', '/LAR/BadChannelsOfl/MissingFEBs', '-t', flags.IOVDb.GlobalTag, '-c', '-a',  '-hitag'])
   else:   
      cmdline = (['AtlCoolCopy', 'COOLOFL_LAR/CONDBR2', 'sqlite://;schema='+flags.LArCalib.BadChannelDB+';dbname=CONDBR2', '-f', '/LAR/BadChannelsOfl/BadChannelsSC',  '-t', 'LARBadChannelsOflBadChannelsSC'+flags.LArCalib.BadChannelTagSC, '-c', '-a',  '-hitag', '-ch', '0'])

   try:
      cp = subprocess.run(cmdlinerm, check=True, capture_output=True )
   except Exception as e:
      print((" ").join(cmdlinerm))
      log.info('not existing BadChan sqlite file, fine')
      sys.exit(-1)
   print((" ").join(cmdlinerm))
   print(cp.stdout)
   try:
      cp = subprocess.run(cmdline, check=True, capture_output=True )
   except Exception as e:
      print(e)
      print((" ").join(cmdline))
      log.error('Could not create BadChan sqlite file !!!!')
      sys.exit(-1)
   print((" ").join(cmdline))
   print(cp.stdout)
 
   cfg=MainServicesCfg(flags)

   cfg.merge(LArPedestalAutoCorrCfg(flags))

   #run the application
   cfg.run() 

   #build tag hierarchy in output sqlite file
   if args.outsql.startswith("/"):
      cmdline = (['/afs/cern.ch/user/l/larcalib/LArDBTools/python/BuildTagHierarchy.py', args.outsql , flags.IOVDb.GlobalTag])
   else:   
      cmdline = (['/afs/cern.ch/user/l/larcalib/LArDBTools/python/BuildTagHierarchy.py',args.outpdir + "/" + args.outsql , flags.IOVDb.GlobalTag])
   log.debug(cmdline)
   try:
      subprocess.run(cmdline, check=True)
   except Exception as e:
      log.error('Could not create tag hierarchy in output sqlite file !!!!')
      sys.exit(-1)

