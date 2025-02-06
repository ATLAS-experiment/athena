# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory

def LArFillMinBiasCfg(flags):

   #Get basic services and cond-algos
   from LArCalibProcessing.LArCalibBaseConfig import LArCalibBaseCfg
   result=LArCalibBaseCfg(flags)

   if flags.LArCalib.isSC:
      ckey="LArOnOffIdMapSC"
      result.addCondAlgo(CompFactory.LArMCSymCondAlg("LArMCSymCondAlgSC",SuperCell=flags.LArCalib.isSC,ReadKey=ckey))
      result.addEventAlgo(CompFactory.FixLArElecSCCalib(FixFlag=3,
                                                   SCCablingKey=ckey,
                                                   InputFile=flags.LArCalib.Input.Files[0],
                                                   ))
      obj="CondAttrListCollection#/LAR/ElecCalibMCSC/MinBias"
   else:   
      ckey="LArOnOffIdMap"
      obj="CondAttrListCollection#/LAR/ElecCalibMC/MinBias"
      result.addEventAlgo(CompFactory.FixLArElecCalib(FixFlag=14,
                                                   CablingKey=ckey,
                                                   InputFile=flags.LArCalib.Input.Files[0],
                                                   ))

   from RegistrationServices.OutputConditionsAlgConfig import OutputConditionsAlgCfg
   result.merge(OutputConditionsAlgCfg(flags,
                                       outputFile="dummy.root",
                                       ObjectList=[obj, ],
                                       IOVTagList=[flags.LArCalib.Input.Type],
                                       Run1=flags.LArCalib.IOVStart,
                                       Run2=flags.LArCalib.IOVEnd
                                   ))

   #RegistrationSvc
   result.addService(CompFactory.IOVRegistrationSvc(RecreateFolders = True, SVFolder=False,
                                     OverrideNames = [flags.LArCalib.Input.SubDet], OverrideTypes = ["Blob16M"]))
   result.getService("IOVDbSvc").DBInstance=""

   #MC Event selector since we have no input data file
   from McEventSelector.McEventSelectorConfig import McEventSelectorCfg
   result.merge(McEventSelectorCfg(flags,
                                   RunNumber         = flags.LArCalib.Input.RunNumbers[0],
                                   EventsPerRun      = 1,
                                   FirstEvent        = 1,
                                   InitialTimeStamp  = 0,
                                   TimeStampInterval = 1))

   return result

if __name__=="__main__":

    import sys
    import argparse
 
    # now process the CL options and assign defaults
    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('-i','--infile', dest='infile', default="ntuple.root", help='Input file with constants`', type=str)
    parser.add_argument('-t','--tag', dest='tag', default="LARElecCalibMCSCMinBias-mc16-Epos-A3-s3687", help='Folder tag for constants`', type=str)
    parser.add_argument('-o','--outfile', dest='outfile', default="MinBias.db", help='Output sqlite file', type=str)
    parser.add_argument('-s','--isSC', dest='supercell', default=False, action='store_true', help='Running for SC')
 
    args = parser.parse_args()
    if help in args and args.help is not None and args.help:
       parser.print_help()
       sys.exit(0)
 
    for _, value in args._get_kwargs():
     if value is not None:
         print(value)

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags=initConfigFlags()
    from LArCalibProcessing.LArCalibConfigFlags import addLArCalibFlags
    addLArCalibFlags(flags, args.supercell)

    flags.Input.Files=[]
    flags.LArCalib.Input.RunNumbers = [404400,]
    flags.Input.RunNumbers=flags.LArCalib.Input.RunNumbers

    flags.IOVDb.DatabaseInstance="CONDBR2"
    flags.IOVDb.DBConnection="sqlite://;schema=" + args.outfile +";dbname=CONDBR2"

    flags.LAr.doAlign=False
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3

    #The global tag we are working with
    flags.IOVDb.GlobalTag = "LARCALIB-RUN2-00"

    # misusing these flags, but do not want to introduce new ones
    flags.LArCalib.Input.Files=[args.infile]
    flags.LArCalib.Input.Type=args.tag
    flags.LArCalib.Input.SubDet="MinBias"

    #Define the global output Level:
    from AthenaCommon.Constants import INFO
    flags.Exec.OutputLevel = INFO

    flags.Detector.GeometryID = False
    flags.Detector.GeometryITk = False
    flags.Detector.GeometryHGTD = False
    flags.Detector.GeometryCalo = False
    flags.Detector.GeometryMuon = False
    flags.Detector.GeometryForward = False

    flags.lock()
    flags.dump()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg

    cfg=MainServicesCfg(flags)
    cfg.merge(LArFillMinBiasCfg(flags))


    cfg.run(1)

