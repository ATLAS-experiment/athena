#!/usr/bin/env python3
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration


from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.MainServicesConfig import MainEvgenServicesCfg

def HVCorrConfig(flags, outputName="hvcorr", runOut=0, lbOut=0, isHI=False, hipatch=1.4, **hvscaleprops):

    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    result=LArGMCfg(flags)
    
    from LArCalibUtils.LArHVScaleConfig import LArHVScaleCfg
    result.merge(LArHVScaleCfg(flags, keyOutputFullCorr="NewLArHVScaleCorr", **hvscaleprops))

    from LArCabling.LArCablingConfig import LArOnOffIdMappingSCCfg
    result.merge(LArOnOffIdMappingSCCfg(flags))
    result.addEventAlgo(CompFactory.LArHVCorrToSCHVCorr(ContainerKey="NewLArHVScaleCorr",OutputKey="NewSCLArHVScaleCorr",
                                                        OutputFolder="/LAR/ElecCalibFlatSC/HVScaleCorrNew",
                                                        IsHeavyIons=isHI,  PatchInHeavyIons=hipatch,
                                                        PhysicsWeights="TrigT1CaloCalibUtils/HVcorrPhysicsWeights.txt"))

    #The LArHVCorrMaker creates a flat blob in a CondAttrListCollection
    #Input: The HV Scale Correction computed by the LArHVCondAlg based on the DCS HV values
    result.addEventAlgo(CompFactory.LArHVCorrMaker(LArHVScaleCorr="NewLArHVScaleCorr",folderName="/LAR/ElecCalibFlat/HVScaleCorrNew"))
    ##Also for SC:
    #result.addEventAlgo(CompFactory.LArHVCorrMaker("LArHVCorrSCMaker",SuperCell=True,LArHVScaleCorr="NewLArHVScaleCorr",
    #                                               folderName="/LAR/ElecCalibFlatSC/HVScaleCorr",CablingKey="LArOnOffIdMapSC"))

    #Ntuple writing ... 
    from LArCalibTools.LArCalib_HVScale2NtupleConfig import LArHVScaleCorr2NtupleCfg
    result.merge(LArHVScaleCorr2NtupleCfg(flags, rootfile=outputName+'_ntuple.root',
                                          hvcorr="NewLArHVScaleCorr", hvcorrSC="NewSCLArHVScaleCorr"))

    #sqlite writing ... 
    from RegistrationServices.OutputConditionsAlgConfig import OutputConditionsAlgCfg
    result.merge(OutputConditionsAlgCfg(flags,
                                        outputFile="dummy.root",
                                        ObjectList=["CondAttrListCollection#/LAR/ElecCalibFlat/HVScaleCorrNew#/LAR/ElecCalibFlat/HVScaleCorr",
                                                    "CondAttrListCollection#/LAR/ElecCalibFlatSC/HVScaleCorrNew#/LAR/ElecCalibFlatSC/HVScaleCorr",],
                                        Run1=runOut,
                                        LB1=lbOut
                                    ))
    


    #RegistrationSvc    
    result.addService(CompFactory.IOVRegistrationSvc(RecreateFolders = True,
                                                     SVFolder=True,
                                                     OverrideNames = ["HVScaleCorr"],
                                                     OverrideTypes = ["Blob16M"],
                                                 ))
    return result


def addHVCorrToSCHVCorrParserArgs(parser):
    # also used in CaloNoiseConfig.py
    parser.add_argument('--isHI', dest='hi', default=False, help='is for HI ?', action='store_true')
    parser.add_argument('--patchHI', dest='patchhi', type=float, default=1.4, help="Patching value for HI")


if __name__=="__main__":
    import argparse
    from time import time
    from LArCalibUtils.LArHVScaleConfig import addHvScaleParserArgs, buildHvScaleProps
    parser= argparse.ArgumentParser(description="Recalculate HV corrections based on DCS values")
    parser.add_argument('datestamp',help="time specification like 2007-05-25:14:01:00")
    parser.add_argument('Run',type=int, nargs='?', default=0,help="IOV start (run-number)")
    parser.add_argument('LB',type=int, nargs='?', default=0,help="IOV start (run-number)")
    parser.add_argument('-g', '--globaltag', type=str, help="Global conditions Tag ")
    parser.add_argument('-o', '--output',type=str,default="hvcorr",help="name stub for root and sqlite output files")
    parser.add_argument('-l','--olevel',type=int, default=3,help="OutputLevel")
    parser.add_argument('-s', '--sqlite',type=str,default="",help="name of sqlite file to be used instead of COOL")
    addHvScaleParserArgs(parser)
    addHVCorrToSCHVCorrParserArgs(parser)
    args = parser.parse_args()
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from LArCalibProcessing.TimeStampToRunLumi import fillInputFlags
    ConfigFlags = initConfigFlags()
    fillInputFlags(ConfigFlags, args.datestamp + "/UTC", infiniteRun=0xFFFFFFF-1)
    print(f"---> Working on run {ConfigFlags.Input.RunNumbers[0]} LB {ConfigFlags.Input.LumiBlockNumbers[0]} "
          f"Timestamp: {ConfigFlags.Input.TimeStamps[0]}")
    timediff=int(time()-ConfigFlags.Input.TimeStamps[0])
    if timediff<0:
        print("ERROR: Timestamp in the future???")
    else:
        (days,remainder)=divmod(timediff,24*60*60)
        (hours,seconds)=divmod(remainder,60*60)
        print ("---> Timestamp is %i days %i hours and %i minutes ago" % (days,hours,int(seconds/60)))
    pass
    print("Output IOV will be from run %i lumiblock %i to INF" % (args.Run,args.LB))
    outputName=args.output
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    if args.globaltag:
        ConfigFlags.IOVDb.GlobalTag=args.globaltag
    ConfigFlags.Input.Files=[]
    ConfigFlags.IOVDb.DatabaseInstance="CONDBR2"
    ConfigFlags.IOVDb.DBConnection="sqlite://;schema="+outputName+".db;dbname=CONDBR2"
    if len(args.sqlite)>0:
       ConfigFlags.IOVDb.SqliteInput=args.sqlite
       ConfigFlags.IOVDb.SqliteFolders=("/LAR/ElecCalibFlat/HVScaleCorr",)
    ConfigFlags.GeoModel.AtlasVersion=defaultGeometryTags.RUN3
    ConfigFlags.Exec.OutputLevel=args.olevel
    ConfigFlags.lock()
    hvsp=buildHvScaleProps(args)
    hvsp["keyOutputResidualCorr"]=""  # tentativelydon't produce these (only full corr.) to save resources
    cfg=MainEvgenServicesCfg(ConfigFlags)
    #First LB not set by McEventSelectorCfg, set it here:
    cfg.getService("EventSelector").FirstLB=ConfigFlags.Input.LumiBlockNumbers[0]
    cfg.merge(HVCorrConfig(ConfigFlags, outputName, runOut=args.Run, lbOut=args.LB,
                           isHI=args.hi, hipatch=args.patchhi, **hvsp))  
    print("Start running...")
    import sys
    sys.exit(cfg.run(1).isFailure())
