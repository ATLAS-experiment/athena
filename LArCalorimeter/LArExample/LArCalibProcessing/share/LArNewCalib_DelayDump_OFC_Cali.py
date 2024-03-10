#!/usr/bin/env python
#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

if __name__=='__main__':

   import os,sys
   import argparse

   # now process the CL options and assign defaults
   parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
   #parser.add_argument('-r','--runlist', dest='runlist', default=RunNumberList, nargs='+', help='Run numbers string list', type=str)
   parser.add_argument('-r','--run', dest='run', default='00408918', help='Run number string as in input filename', type=str)
   parser.add_argument('-g','--gain', dest='gain', default="MEDIUM", help='Gain string', type=str)
   parser.add_argument('-i','--infile', dest='infile', default="", help='Input POOL file to dump', type=str)
   parser.add_argument('-e','--outrdir', dest='outrdir', default="/eos/atlas/atlascerngroupdisk/det-larg/Temp/Weekly/ntuples", help='Output root file directory', type=str)
   parser.add_argument('-o','--outrwavefile', dest='outrwavefile', default="", help='Output CaliWave root file name', type=str)
   parser.add_argument('-p','--outrofcfile', dest='outrofcfile', default="", help='Output OFC root file name', type=str)
   parser.add_argument('-c','--isSC', dest='supercells', default=False, help='is SC data ?', type=bool)
   parser.add_argument('-b','--badchansqlite', dest='badsql', default="SnapshotBadChannel.db", help='Input sqlite file for bad channels', type=str)

   args = parser.parse_args()
   if help in args and args.help is not None and args.help:
      parser.print_help()
      sys.exit(0)

   for _, value in args._get_kwargs():
    if value is not None:
        print(value)

   # now set flags according parsed options
   
   #Import the configution-method we want to use (here: Pedestal and AutoCorr)
   from LArCalibProcessing.LArCalib_Delay_OFCCaliConfig import LArDelay_OFCCali_PoolDumpCfg
   
   #Import the MainServices (boilerplate)
   from AthenaConfiguration.MainServicesConfig import MainServicesCfg
   
   #Import the flag-container that is the arguemnt to the configuration methods
   from AthenaConfiguration.AllConfigFlags import initConfigFlags
   flags=initConfigFlags()
   from LArCalibProcessing.LArCalibConfigFlags import addLArCalibFlags
   addLArCalibFlags(flags)
   
   #Now we set the flags as required for this particular job:
   #The following flags help finding the input bytestream files: 
   flags.Input.Files=[]
   flags.LArCalib.Input.Files = [ args.infile ]
   flags.LArCalib.Input.RunNumbers = [int(args.run),]
   flags.Input.RunNumbers = [int(args.run)]
   gainNumMap={"HIGH":0,"MEDIUM":1,"LOW":2}
   flags.LArCalib.Gain=gainNumMap[args.gain.upper()]

   # others flags settings
   flags.LArCalib.isSC = args.supercells

   #Configure the Bad-Channel database we are reading 
   #(the AP typically uses a snapshot in an sqlite file
   flags.LArCalib.BadChannelTag = "-RUN2-UPD3-00"
   flags.LArCalib.BadChannelDB = args.badsql
   
   #Output of this job 
   # if filenames not defined, change .pool.root to .root in input file name
   # and add CaliWave or OFC, if not part of the name
   import os.path
   if not args.outrwavefile:  
      inbase=os.path.basename(args.infile)
      idx=inbase.find('.pool.root')
      if idx != -1:
         OutputCaliWaveRootFileName = inbase[0:idx]+'.root'
      else:   
         OutputCaliWaveRootFileName = 'LArCaliWave.root'
   else:      
      OutputCaliWaveRootFileName = args.outrwavefile

   if not args.outrofcfile:  
      inbase=os.path.basename(args.infile)
      idx=inbase.find('.pool.root')
      if idx != -1:
         OutputOFCCaliRootFileName = inbase[0:idx]+'.root'
      else:   
         OutputOFCCaliRootFileName = 'LArOFCCali.root'
   else:      
      OutputOFCCaliRootFileName = args.outrofcfile

   flags.LArCalib.Output.ROOTFile = args.outrdir + "/" + OutputCaliWaveRootFileName
   flags.LArCalib.Output.ROOTFile2 = args.outrdir + "/" + OutputOFCCaliRootFileName

   #The global tag we are working with
   flags.IOVDb.GlobalTag = "LARCALIB-RUN2-00"

   from AthenaConfiguration.TestDefaults import defaultGeometryTags
   flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3
   
   #Define the global output Level:
   from AthenaCommon.Constants import INFO 
   flags.Exec.OutputLevel = INFO
   
   flags.lock()
   
   cfg = MainServicesCfg(flags)
   cfg.merge(LArDelay_OFCCali_PoolDumpCfg(flags))
   cfg.getService("IOVDbSvc").DBInstance=""
   cfg.getService("IOVDbSvc").forceRunNumber=int(args.run)

   #MC Event selector since we have no input data file 
   from AthenaConfiguration.ComponentFactory import CompFactory
   mcCnvSvc = CompFactory.McCnvSvc()
   cfg.addService(mcCnvSvc)
   cfg.addService(CompFactory.EvtPersistencySvc("EventPersistencySvc",CnvServices=[mcCnvSvc.getFullJobOptName(),]))
   eventSelector=CompFactory.McEventSelector("EventSelector",
                                             RunNumber = flags.LArCalib.Input.RunNumbers[0],
                                             EventsPerRun      = 1,
                                             FirstEvent           = 0,
                                             InitialTimeStamp  = 0,
                                             TimeStampInterval = 1
                                         )

   cfg.addService(eventSelector)

   #run the application
   cfg.run(1) 

