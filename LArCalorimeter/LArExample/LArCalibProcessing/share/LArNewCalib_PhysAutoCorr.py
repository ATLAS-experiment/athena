#!/usr/bin/env python
#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

if __name__=='__main__':

   import os,sys
   import argparse

   # now process the CL options and assign defaults
   parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
   parser.add_argument('-r','--run', dest='run', default='00408918', help='Run number string as in input filename', type=str)
   parser.add_argument('-g','--gain', dest='gain', default="MEDIUM", help='Gain string', type=str)
   parser.add_argument('-d','--insqlitefile', dest='insqlitefile', default="", help='Input sqlite file with CaliWaves`', type=str)
   parser.add_argument('-e','--outrdir', dest='outrdir', default="/eos/atlas/atlascerngroupdisk/det-larg/Temp/Weekly/ntuples", help='Output root file directory', type=str)
   parser.add_argument('-k','--outpdir', dest='outpdir', default="/eos/atlas/atlascerngroupdisk/det-larg/Temp/Weekly/poolFiles", help='Output root file directory', type=str)
   parser.add_argument('-l','--outprefix', dest='outprefix', default="LArPhysAutoCorr", help='Output root file name', type=str)
   parser.add_argument('-n','--outsqlite', dest='outsql', default="mysql_acorr.db", help='Output sqlite file, in pool output dir.', type=str)
   parser.add_argument('-c','--isSC', dest='supercells', default=False, action="store_true", help='is SC data ?')
   parser.add_argument('-b','--badchansqlite', dest='badsql', default="SnapshotBadChannel.db", help='Input sqlite file for bad channels', type=str)
   parser.add_argument('-m','--subdet', dest='subdet', default="EMB", help='Subdetector, EMB, EMEC, HEC or FCAL', type=str)
   parser.add_argument('-s','--side', dest='side', default="C", help='Detector side empty (means both), C or A', type=str)

   args = parser.parse_args()
   if help in args and args.help is not None and args.help:
      parser.print_help()
      sys.exit(0)

   for _, value in args._get_kwargs():
    if value is not None:
        print(value)

   # now set flags according parsed options
   
   from LArCalibProcessing.LArCalib_PileUpAutoCorrConfig import LArPileUpAutoCorrCfg
   
   #Import the MainServices (boilerplate)
   from AthenaConfiguration.MainServicesConfig import MainServicesCfg
   
   #Import the flag-container that is the arguemnt to the configuration methods
   from AthenaConfiguration.AllConfigFlags import initConfigFlags
   flags=initConfigFlags()
   from LArCalibProcessing.LArCalibConfigFlags import addLArCalibFlags
   addLArCalibFlags(flags, args.supercells)
   
   #Now we set the flags as required for this particular job:
   #The following flags help finding the input bytestream files: 
   flags.Input.Files=[]
   flags.LArCalib.Input.Files = [ ]
   flags.LArCalib.Input.RunNumbers = [int(args.run),]
   gainNumMap={"HIGH":0,"MEDIUM":1,"LOW":2}
   flags.LArCalib.Gain=gainNumMap[args.gain.upper()]

   if args.insqlitefile.startswith("/"):
      flags.LArCalib.Input.Database = args.insqlitefile
   else:
      flags.LArCalib.Input.Database = args.outpdir + "/" + args.insqlitefile

   # pileup normalisation
   flags.LArCalib.OFC.Ncoll = 60
   flags.LArCalib.OFC.Nsamples = 32
   from AthenaCommon.SystemOfUnits import ns
   flags.Beam.BunchSpacing = 25.*ns
   flags.Beam.NumberOfCollisions = flags.LArCalib.OFC.Ncoll

   # others flags settings
   flags.LArCalib.isSC = args.supercells
   flags.LArCalib.Input.SubDet=args.subdet

   #Configure the Bad-Channel database we are reading 
   #(the AP typically uses a snapshot in an sqlite file
   flags.LArCalib.BadChannelDB = args.badsql
   
   #Output of this job 
   OutputPoolFileName = args.outprefix+"_"+args.run
   if args.subdet != "":
      OutputPoolFileName += "_"+args.subdet
   OutputPoolFileName += ".pool.root"

   idx=OutputPoolFileName.find('.pool.root')
   if idx != -1:
      OutputRootFileName = OutputPoolFileName[0:idx]+'.root'
   else:   
      OutputRootFileName = OutputPoolFileName+'.root'

   flags.LArCalib.Output.ROOTFile = args.outrdir + "/" + OutputRootFileName
   flags.LArCalib.Output.POOLFile = args.outpdir + "/" + OutputPoolFileName
   if args.outsql.startswith("/"):
      flags.IOVDb.DBConnection="sqlite://;schema=" + args.outsql +";dbname=CONDBR2"
   else:
      flags.IOVDb.DBConnection="sqlite://;schema="+args.outpdir + "/" + args.outsql +";dbname=CONDBR2"

   #The global tag we are working with
   flags.IOVDb.GlobalTag = "LARCALIB-RUN2-00"
   
   from AthenaConfiguration.TestDefaults import defaultGeometryTags
   flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3

   #Define the global output Level:
   from AthenaCommon.Constants import INFO
   flags.Exec.OutputLevel = INFO
   
   flags.lock()
   
   cfg=MainServicesCfg(flags)
   
   cfg.merge(LArPileUpAutoCorrCfg(flags))

   # uncomment in case a debug is needed:
   #from AthenaCommon.Constants import DEBUG
   #cfg.getService("MessageSvc").OutputLevel = DEBUG
   cfg.getService("MessageSvc").defaultLimit = 9999999  # all messages

   #run the application
   cfg.run(1) 

