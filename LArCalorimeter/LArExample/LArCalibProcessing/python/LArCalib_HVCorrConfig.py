# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration


from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.MainServicesConfig import MainEvgenServicesCfg

def HVCorrConfig(flags,outputName="hvcorr",runOut=0, lbOut=0):

    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    result=LArGMCfg(flags)
    
    #Use the standard LArHVScaleCfg and adjust properties afterwards as needed 
    from LArCalibUtils.LArHVScaleConfig import LArHVScaleCfg

    result.merge(LArHVScaleCfg(flags))
    result.getCondAlgo("LArHVCondAlg").UndoOnlineHVCorr=False
    result.getCondAlgo("LArHVCondAlg").keyOutputCorr="NewLArHVScaleCorr"

    from LArCabling.LArCablingConfig import LArOnOffIdMappingSCCfg
    result.merge(LArOnOffIdMappingSCCfg(flags))
    result.addEventAlgo(CompFactory.LArHVCorrToSCHVCorr(ContainerKey="NewLArHVScaleCorr",OutputKey="NewSCLArHVScaleCorr",
                                                        OutputFolder="/LAR/ElecCalibFlatSC/HVScaleCorrNew",
                                                        IsHeavyIons=False,  PatchInHeavyIons=1.0,
                                                        PhysicsWeights="TrigT1CaloCalibUtils/HVcorrPhysicsWeights.txt"))

    #The LArHVCorrMaker creates a flat blob in a CondAttrListCollection
    #Input: The HV Scale Correction computed by the LArHVCondAlg based on the DCS HV values
    result.addEventAlgo(CompFactory.LArHVCorrMaker(LArHVScaleCorr="NewLArHVScaleCorr",folderName="/LAR/ElecCalibFlat/HVScaleCorrNew"))
    ##Also for SC:
    #result.addEventAlgo(CompFactory.LArHVCorrMaker("LArHVCorrSCMaker",SuperCell=True,LArHVScaleCorr="NewLArHVScaleCorr",
    #                                               folderName="/LAR/ElecCalibFlatSC/HVScaleCorr",CablingKey="LArOnOffIdMapSC"))

    #Ntuple writing ... 
    from LArCalibTools.LArCalib_HVScale2NtupleConfig import LArHVScaleCorr2NtupleCfg
    result.merge(LArHVScaleCorr2NtupleCfg(flags,rootfile=outputName+'_ntuple.root',addSC=True))
    result.getEventAlgo("LArHVScaleCorr2Ntuple").ContainerKey="NewLArHVScaleCorr" 
    result.getEventAlgo("LArSCHVScaleCorr2Ntuple").ContainerKey="NewSCLArHVScaleCorr" 
    result.getEventAlgo("LArSCHVScaleCorr2Ntuple").NtuplePath="/NTUPLES/FILE1/HVSCALESC"

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

    result.getService("IOVDbSvc").DBInstance=""
    return result


if __name__=="__main__":
    import sys
    from time import time,strptime
    from calendar import timegm
    import argparse
    parser= argparse.ArgumentParser(description="Recalclulate HV corrections based on DCS values")
    parser.add_argument('datestamp',help="time specification like 2007-05-25:14:01:00")
    parser.add_argument('Run',type=int, nargs='?', default=0,help="IOV start (run-number)")
    parser.add_argument('LB',type=int, nargs='?', default=0,help="IOV start (run-number)")
    parser.add_argument('-g', '--globaltag', type=str, help="Geometry Tag ")
    parser.add_argument('-o', '--output',type=str,default="hvcorr",help="name stub for root and sqlite output files")
    parser.add_argument('-l','--olevel',type=int, default=3,help="OutputLevel")
                        
    args = parser.parse_args()
    try:
        ts=strptime(args.datestamp+'/UTC','%Y-%m-%d:%H:%M:%S/%Z')
        TimeStamp=int(timegm(ts))
        TimeStamp_ns=TimeStamp*1000000000
    except ValueError as e:
        print("ERROR in time specification, use e.g. 2007-05-25:14:01:00")
        print(e)
        sys.exit(-1)

    from LArCalibProcessing.TimeStampToRunLumi import TimeStampToRunLumi
    
    rlb=TimeStampToRunLumi(TimeStamp_ns)
    if rlb is None:
        rlb=[0xFFFFFFF-1,0]
        print("WARNING: Failed to convert time",TimeStamp_ns,"into a run/lumi number. Using 'infinite' run-number",rlb[0])

    
    print("---> Working on run",rlb[0],"LB",rlb[1],"Timestamp:",TimeStamp)
    timediff=int(time()-TimeStamp)
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
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    ConfigFlags=initConfigFlags()
                        
    if args.globaltag:
        ConfigFlags.IOVDb.GlobalTag=args.globaltag

    ConfigFlags.Input.RunNumbers=[rlb[0]]
    ConfigFlags.Input.LumiBlockNumbers=[rlb[1]]
    ConfigFlags.Input.TimeStamps=[TimeStamp]
    ConfigFlags.Input.Files=[]
    ConfigFlags.IOVDb.DatabaseInstance="CONDBR2"
    ConfigFlags.IOVDb.DBConnection="sqlite://;schema="+outputName+".sqlite;dbname=CONDBR2"
    ConfigFlags.GeoModel.AtlasVersion=defaultGeometryTags.RUN3
    ConfigFlags.Exec.OutputLevel=args.olevel
    ConfigFlags.lock()
    cfg=MainEvgenServicesCfg(ConfigFlags)
    #First LB not set by McEventSelectorCfg, set it here:
    cfg.getService("EventSelector").FirstLB=ConfigFlags.Input.LumiBlockNumbers[0]
    cfg.merge(HVCorrConfig(ConfigFlags,outputName,args.Run,args.LB))
    
    
    print("Start running...")
    cfg.run(1)
