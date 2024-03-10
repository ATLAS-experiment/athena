#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory

def LArToCoolInlineCfg(flags,inputFolders,singleV=True):

    from LArCalibProcessing.LArCalibBaseConfig import LArCalibBaseCfg
    result=LArCalibBaseCfg(flags)
    from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg 
    result.merge(LArOnOffIdMappingCfg(flags))
    if flags.LArCalib.isSC:
       from LArCabling.LArCablingConfig import LArOnOffIdMappingSCCfg
       result.merge(LArOnOffIdMappingSCCfg(flags))

    theLArCompleteToFlat = CompFactory.LArCompleteToFlat(FakeEMBPSLowGain=True,isSC=flags.LArCalib.isSC)
    theLArCompleteToFlat.OutputLevel = 2

    outTypes = []
    overrides = []
    outTags=[]
    for (fldr,ftag,key,classtype) in inputFolders:
      if "Pedestal" in fldr:
        outTypes.append("Pedestal")
        theLArCompleteToFlat.PedestalInput=key
        overrides.extend(["Pedestal", "PedestalRMS"])
      elif "Ramp" in fldr:
        outTypes.append("Ramp")
        theLArCompleteToFlat.RampInput=key
        overrides.extend(["RampVec"])
      elif "OFC" in fldr:
        outTypes.append("OFC")
        theLArCompleteToFlat.OFCInput=key
        overrides.extend(["OFCa", "OFCb","TimeOffset"])
      elif "MphysOverMcal" in fldr:
        outTypes.append("MphysOverMcal")
        theLArCompleteToFlat.MphysOverMcalInput=key
        overrides.extend(["MphysOverMcal"])
      elif "Shape" in fldr:
        outTypes.append("Shape")
        theLArCompleteToFlat.ShapeInput=key
        overrides.extend(["Shape","ShapeDer"])

      from IOVDbSvc.IOVDbSvcConfig import addFolders  
      if len(ftag):
         result.merge(addFolders(flags,fldr,detDb=flags.LArCalib.Input.Database,tag=ftag))
         if not singleV: 
            outTags.append(ftag)
         else:   
            outTags.append("")
      else:
         result.merge(addFolders(flags,fldr,detDb=flags.LArCalib.Input.Database))
         outTags.append("")
      pass
    
    result.addEventAlgo(theLArCompleteToFlat)
    
    flatName="ElecCalibFlat"
    if flags.LArCalib.isSC:
       flatName+="SC"
    if "outObjects" not in dir():
       outObjects=["CondAttrListCollection#/LAR/"+flatName+"/"+ot for ot in outTypes] 
    
    from RegistrationServices.OutputConditionsAlgConfig import OutputConditionsAlgCfg
    result.merge(OutputConditionsAlgCfg(flags,
                                        outputFile="dummy.root",
                                        ObjectList=outObjects,
                                        IOVTagList=outTags,
                                        Run1=flags.LArCalib.IOVStart,
                                        Run2=flags.LArCalib.IOVEnd
                                    ))
    #RegistrationSvc    
    print("OVERRIDES ARE:",overrides)
    types=[]
    for i in range(len(overrides)):
       types.append("Blob16M")
    result.addService(CompFactory.IOVRegistrationSvc(RecreateFolders = True, SVFolder=True,OverrideNames = overrides, OverrideTypes = types))
    result.getService("IOVDbSvc").DBInstance=""

    #MC Event selector since we have no input data file 
    from McEventSelector.McEventSelectorConfig import McEventSelectorCfg
    result.merge(McEventSelectorCfg(flags,
                                    RunNumber         = flags.LArCalib.Input.RunNumbers[0],
                                    EventsPerRun      = 1,
                                    FirstEvent	      = 1,
                                    InitialTimeStamp  = 0,
                                    TimeStampInterval = 1))

       
    return result

if __name__=="__main__":
    import sys,os
    import argparse

    # now process the CL options and assign defaults
    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('-r','--run', dest='run', default=str(0x7FFFFFFF), help='Run number to query input DB', type=str)
    parser.add_argument('-i','--insqlite', dest='insql', default="freshConstants_AP.db", help='Input sqlite file containing the (merged) output of the AP.', type=str)
    parser.add_argument('-f','--infolders', dest='infold', default="ConvertToInline.py", help='python file with inputFolders definition', type=str)
    parser.add_argument('-o','--outsqlite', dest='outsql', default="freshConstants_merged.db", help='Output sqlite file', type=str)
    parser.add_argument('--iovstart',dest="iovstart", default=0, help="IOV start (run-number)", type=int)
    parser.add_argument('--isSC', dest='supercells', default=False, help='is SC data ?', action="store_true")
    parser.add_argument('--poolcat', dest='poolcat', default="freshConstants.xml", help='Catalog of POOL files', type=str)
    args = parser.parse_args()
    if help in args and args.help is not None and args.help:
        parser.print_help()
        sys.exit(0)



    
    #Import the MainServices (boilerplate)
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
   
    #Import the flag-container that is the arguemnt to the configuration methods
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from LArCalibProcessing.LArCalibConfigFlags import addLArCalibFlags
    flags=initConfigFlags()
    addLArCalibFlags(flags, args.supercells)

    #Now we set the flags as required for this particular job:
    flags.LArCalib.Input.Database = args.insql
       
    flags.IOVDb.DBConnection="sqlite://;schema="+args.outsql +";dbname=CONDBR2"

    #The global tag we are working with
    flags.IOVDb.GlobalTag = "LARCALIB-RUN2-00"
    # geometry
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3

    flags.Input.Files=[]
    flags.LArCalib.Input.Files = [ ]
    flags.LArCalib.IOVStart = args.iovstart

    flags.LArCalib.Input.RunNumbers=[int(args.run),]
    flags.Input.RunNumbers=flags.LArCalib.Input.RunNumbers

    flags.dump()
    flags.lock()
   
    try:
       import importlib
       mypath=os.getcwd()
       from pathlib import Path
       path=Path(mypath)
       sys.path.append(str(path.parent))
       module = importlib.import_module('.'+args.infold, package=str(path.name))
    except Exception as e:
       print(e)
       sys.exit(-1)

    cfg=MainServicesCfg(flags)
    cfg.merge(LArToCoolInlineCfg(flags,module.inputFolders))

    cfg.getService("PoolSvc").WriteCatalog=("xmlcatalog_file:%s"%args.poolcat)
    cfg.getService("PoolSvc").ReadCatalog+=["xmlcatalog_file:PoolFileCatalog.xml",]
    cfg.getService("PoolSvc").SortReplicas = False 

    cfg.getService("MessageSvc").debugLimit=9999999

    cfg.run(1)
