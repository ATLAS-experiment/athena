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
   parser.add_argument('-d','--insqlitefile', dest='insqlitefile', default="", help='Input sqlite file with PhysWaves`', type=str)
   parser.add_argument('-e','--outrdir', dest='outrdir', default="/eos/atlas/atlascerngroupdisk/det-larg/Temp/Weekly/ntuples", help='Output root file directory', type=str)
   parser.add_argument('-k','--outpdir', dest='outpdir', default="/eos/atlas/atlascerngroupdisk/det-larg/Temp/Weekly/poolFiles", help='Output root file directory', type=str)
   parser.add_argument('-l','--outprefix', dest='outprefix', default="LArPhysWave", help='Output root file name', type=str)
   parser.add_argument('-n','--outsqlite', dest='outsql', default="mysql_physwave.db", help='Output sqlite file, in pool output dir.', type=str)
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
   
   from LArCalibProcessing.LArCalib_PhysWavePredictionConfig import LArPhysWavePredictionCfg
   
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

   # others flags settings
   flags.LArCalib.Input.SubDet=args.subdet

   #Configure the Bad-Channel database we are reading 
   #(the AP typically uses a snapshot in an sqlite file
   flags.LArCalib.BadChannelTag = "-RUN2-UPD3-00"
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
   
   cfg.merge(LArPhysWavePredictionCfg(flags))

   cfg.printConfig(withDetails=True,printDefaults=True)

   cfg.getService("MessageSvc").defaultLimit = 9999999  # all messages

   #run the application
   cfg.run(1) 

   #Copying the FCAL phys wave to sqlite here 
   if  "HEC" in args.subdet.upper():
      copycmd='AtlCoolCopy "COOLOFL_LAR/CONDBR2" "'+flags.IOVDb.DBConnection+'" -f '+ flags.LArCalib.FCALPhysWave.Folder+' -t LARElecCalibOflPhysWavesFCALFromTB-calib-01 -of /LAR/ElecCalibOfl/PhysWaves/RTM  -ot LARElecCalibOflPhysWavesRTM-RUN2-UPD3-00  -a '
      from subprocess import getstatusoutput
      stat,out=getstatusoutput(copycmd)
 
      if (stat):
          print("ERROR could not copy the FCAL PhysWaves")
          print(out)
          sys.exit(-1)
