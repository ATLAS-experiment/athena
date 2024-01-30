# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory 
from AthenaConfiguration.MainServicesConfig import MainEvgenServicesCfg

def LArHVScaleCorr2NtupleCfg(flags, rootfile="hvcorr_read.root"):

    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    result=LArGMCfg(flags)

    from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg
    result.merge(LArOnOffIdMappingCfg(flags))

    from LArBadChannelTool.LArBadChannelConfig import LArBadChannelCfg
    result.merge(LArBadChannelCfg(flags))

    from LArConfiguration.LArElecCalibDBConfig import LArElecCalibDBCfg
    result.merge(LArElecCalibDBCfg(flags,["HVScaleCorr"]))


    theLArHVScaleCorr2Ntuple = CompFactory.LArHVScaleCorr2Ntuple("LArHVScaleCorr2Ntuple")
    theLArHVScaleCorr2Ntuple.AddFEBTempInfo = False
    theLArHVScaleCorr2Ntuple.OffId=True
    result.addEventAlgo(theLArHVScaleCorr2Ntuple)

    import os
    if os.path.exists(rootfile):
        os.remove(rootfile)
    result.addService(CompFactory.NTupleSvc(Output = [ "FILE1 DATAFILE='"+rootfile+"' OPT='NEW'" ]))
    result.setAppProperty("HistogramPersistency","ROOT")

    return result

if __name__=="__main__":
    import sys
    from time import strptime
    from calendar import timegm

    if len(sys.argv)<2:
        print("Usage:")
        print("%s <time> <outputfile> <globaltag>" % sys.argv[0])
        sys.exit(-1)
    

    try:
        ts=strptime(sys.argv[1]+'/UTC','%Y-%m-%d:%H:%M:%S/%Z')
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


    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    from LArCalibProcessing.LArCalibConfigFlags import addLArCalibFlags
    addLArCalibFlags(flags)

    flags.Input.RunNumbers=[rlb[0]]
    flags.Input.TimeStamps=[TimeStamp]
    flags.Input.Files=[]
    flags.IOVDb.DatabaseInstance="CONDBR2"

    rootfile="hvcorr_read.root"
    if len(sys.argv)>2:
        rootFile=sys.argv[2]

    if len(sys.argv)>3:
        flags.IOVDb.GlobalTag=sys.argv[3]
        
    flags.lock()
    cfg=MainEvgenServicesCfg(flags)
    cfg.merge(LArHVScaleCorr2NtupleCfg(flags))
    
    
    print("Start running...")
    cfg.run(1)
