#!/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory


if __name__=='__main__':

  import os,sys
  import argparse

  # now process the CL options and assign defaults
  parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
  parser.add_argument('-s','--stime', dest='time', default=1726837122, help='TimeStamp (in s)', type=int)
  parser.add_argument('-r','--run', dest='run', default=999999, help='Run Number', type=int)
  parser.add_argument('-t','--tag',dest='dbtag',default=None,help="Global conditions tag", type=str)
  parser.add_argument('-o','--out', dest='out', default="LArHV.root", help='Output root file', type=str)

  args = parser.parse_args()
  if help in args and args.help is not None and args.help:
    parser.print_help()
    sys.exit(0)

 
  from AthenaConfiguration.AllConfigFlags import initConfigFlags
  flags=initConfigFlags()
   
  flags.Input.RunNumbers=[args.run]
  from AthenaConfiguration.TestDefaults import defaultGeometryTags
  flags.GeoModel.AtlasVersion=defaultGeometryTags.RUN3

  flags.Input.Files=[]

  flags.Input.isMC=False

  flags.LAr.doAlign=False

  from AthenaConfiguration.Enums import LHCPeriod
  if flags.Input.RunNumbers[0] < 222222:
    #Set to run1 for early run-numbers
    flags.GeoModel.Run=LHCPeriod.Run1 
    flags.IOVDb.DatabaseInstance="OFLP200" if flags.Input.isMC else "COMP200" 
  else:
    flags.GeoModel.Run=LHCPeriod.Run2
    flags.IOVDb.DatabaseInstance="OFLP200" if flags.Input.isMC else "CONDBR2"

  if args.dbtag:
    flags.IOVDb.GlobalTag=args.dbtag
  elif flags.IOVDb.DatabaseInstance == "COMP200":
    flags.IOVDb.GlobalTag="COMCOND-BLKPA-RUN1-09"
  else: 
    flags.IOVDb.GlobalTag="CONDBR2-BLKPA-2024-03"


  flags.lock()
  
  from AthenaConfiguration.MainServicesConfig import MainServicesCfg
  cfg=MainServicesCfg(flags)

  #MC Event selector since we have no input data file
  from McEventSelector.McEventSelectorConfig import McEventSelectorCfg
  cfg.merge(McEventSelectorCfg(flags,
                               EventsPerRun      = 1,
                               FirstEvent	      = 1,
                               InitialTimeStamp  = args.time,
                               TimeStampInterval = 1))



  from LArCalibUtils.LArHVScaleConfig import LArHVScaleCfg
  cfg.merge(LArHVScaleCfg(flags))
    
  
  cfg.addEventAlgo(CompFactory.LArHV2Ntuple())


  rootfile=args.out
  if os.path.exists(rootfile):
    os.remove(rootfile)
  cfg.addService(CompFactory.THistSvc(Output = [ "file1 DATAFILE='"+rootfile+"' TYP='ROOT' OPT='RECREATE'" ]))
  cfg.setAppProperty("HistogramPersistency","ROOT")

  cfg.run(1)
  sys.exit(0)
