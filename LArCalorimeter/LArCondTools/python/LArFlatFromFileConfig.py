# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory

def LArFlatFromFileCfg(flags):

   #Get basic services and cond-algos
   from LArCalibProcessing.LArCalibBaseConfig import LArCalibBaseCfg
   result=LArCalibBaseCfg(flags)

   if flags.LArCalib.isSC:
      ckey="LArOnOffIdMapSC"
      ngain=1
   else:   
      ckey="LArOnOffIdMap"
      ngain=3
   result.addEventAlgo(CompFactory.LArFlatFromFile(SuperCells=flags.LArCalib.isSC,
                                                   CablingKey=ckey,NGains=ngain,
                                                   OFCInput="",
                                                   SingleInput=flags.LArCalib.Input.Files[0],
                                                   Folder=flags.LArCalib.DetCellParams.Folder,
                                                   Blob=flags.LArCalib.Input.SubDet
                                                   ))

   from RegistrationServices.OutputConditionsAlgConfig import OutputConditionsAlgCfg
   result.merge(OutputConditionsAlgCfg(flags,
                                       outputFile="dummy.root",
                                       ObjectList=["CondAttrListCollection#"+flags.LArCalib.DetCellParams.Folder, ],
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
    parser.add_argument('-i','--infile', dest='infile', default="", help='Input file with constants`', type=str)
    parser.add_argument('-f','--folder', dest='fld', default="", help='Folder for constants`', type=str)
    parser.add_argument('-t','--tag', dest='tag', default="", help='Folder tag for constants`', type=str)
    parser.add_argument('-b','--blob', dest='blb', default="", help='Blob name for constants`', type=str)
    parser.add_argument('-o','--outfile', dest='outfile', default="Float.db", help='Output sqlite file', type=str)
    parser.add_argument('-c','--isSC', dest='supercells', default=False, action="store_true", help='is SC data ?')
 
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
    addLArCalibFlags(flags, args.supercells)

    flags.Input.Files=[]
    flags.LArCalib.isSC = args.supercells
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
    flags.LArCalib.DetCellParams.Folder=args.fld
    flags.LArCalib.Input.Files=[args.infile]
    flags.LArCalib.Input.Type=args.tag
    flags.LArCalib.Input.SubDet=args.blb

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

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg

    cfg=MainServicesCfg(flags)
    cfg.merge(LArFlatFromFileCfg(flags))


    cfg.run(1)

