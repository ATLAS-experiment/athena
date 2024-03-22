# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory 

def LArNoiseCfg(flags):

    result=ComponentAccumulator()

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    result.merge(PoolReadCfg(flags))
    
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    result.merge(LArGMCfg(flags))
    from TileGeoModel.TileGMConfig import TileGMCfg
    result.merge(TileGMCfg(flags))

    #Setup cabling
    from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg
    result.merge(LArOnOffIdMappingCfg(flags))
    # setup bad chan and missing febs
    from LArBadChannelTool.LArBadChannelConfig import LArBadChannelCfg,LArBadFebCfg
    result.merge(LArBadChannelCfg(flags))
    result.merge(LArBadFebCfg(flags))

    from LArCellRec.LArCollisionTimeConfig import LArCollisionTimeCfg
    result.merge(LArCollisionTimeCfg(flags))

    from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
    tdt = result.getPrimaryAndMerge(TrigDecisionToolCfg(flags))

    noiseAlg=CompFactory.LArNoiseBursts("LArNoiseBursts")
    noiseAlg.SigmaCut = flags.LArNoise.SigmaCut
    noiseAlg.NumberOfBunchesInFront = flags.LArNoise.NumberOfBunchesInFront
    noiseAlg.KeepOnlyCellID = flags.LArNoise.KeepOnlyCellID
    noiseAlg.TrigDecisionTool = tdt
    result.addEventAlgo(noiseAlg)

    if (flags.LArNoise.outNtupLAr!=""):
        result.addService(CompFactory.THistSvc(Output=["TTREE DATAFILE='"+flags.LArNoise.outNtupLAr+"' OPT='RECREATE'",]))

    if (flags.LArNoise.HECNoiseNtup!=""):
        hecAlg=CompFactory.LArHECNoise("LArHECNoise")
        hecAlg.MinDigitADC = flags.LArNoise.MinDigitADC
        hecAlg.MaxDeltaT = flags.LArNoise.MaxDeltaT
        from IOVDbSvc.IOVDbSvcConfig import addFolders
        result.merge(addFolders(flags, "/LAR/ElecCalibFlat/Pedestal", "LAR_ONL", className="CondAttrListCollection", db="CONDBR2"))
        LArPedestalCondAlg =  CompFactory.getComp("LArFlatConditionsAlg<LArPedestalFlat>")
        result.addCondAlgo(LArPedestalCondAlg(ReadKey="/LAR/ElecCalibFlat/Pedestal", WriteKey="LArPedestal"))

        result.addEventAlgo(hecAlg)
        if result.getService("THistSvc") is not None:
           result.getService("THistSvc").Output += ["HEC DATAFILE='"+flags.LArNoise.HECNoiseNtup+"' OPT='RECREATE'",]
        else:   
           result.addService(CompFactory.THistSvc(Output=["HEC DATAFILE='"+flags.LArNoise.HECNoiseNtup+"' OPT='RECREATE'",]))

    return result

def LArNoiseFromRawCfg(flags):

    result=ComponentAccumulator()

    #from LArByteStream.LArRawDataReadingConfig import LArRawDataReadingCfg
    #result.merge(LArRawDataReadingCfg(flags))

    ##Setup cabling
    #from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg
    #result.merge(LArOnOffIdMappingCfg(flags))
    ## setup bad chan and missing febs
    #from LArBadChannelTool.LArBadChannelConfig import LArBadChannelCfg,LArBadFebCfg
    #LArBadChannelCfg(flags)
    #LArBadFebCfg(flags)

    from CaloRec.CaloRecoConfig import CaloRecoCfg
    result.merge(CaloRecoCfg(flags))

    from LumiBlockComps.BunchCrossingCondAlgConfig import BunchCrossingCondAlgCfg
    result.merge(BunchCrossingCondAlgCfg(flags))
    
    from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg
    result.merge(CaloNoiseCondAlgCfg(flags,"totalNoise"))

    from LArROD.LArFebErrorSummaryMakerConfig import LArFebErrorSummaryMakerCfg
    result.merge(LArFebErrorSummaryMakerCfg(flags))
    result.getEventAlgo("LArFebErrorSummaryMaker").CheckAllFEB=False

    if (flags.LArNoise.outNtupLAr != "" or flags.LArNoise.HECNoiseNtup!=""):
       from LArCellRec.LArTimeVetoAlgConfig import LArTimeVetoAlgCfg
       result.merge(LArTimeVetoAlgCfg(flags))

       from LArCafJobs.LArSCDumperSkeleton import L1CaloMenuCfg
       result.merge(L1CaloMenuCfg(flags))

       from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
       tdt = result.getPrimaryAndMerge(TrigDecisionToolCfg(flags))


    if (flags.LArNoise.outNtupLAr != ""):
       noiseAlg=CompFactory.LArNoiseBursts("LArNoiseBursts")
       noiseAlg.SigmaCut = flags.LArNoise.SigmaCut
       noiseAlg.NumberOfBunchesInFront = flags.LArNoise.NumberOfBunchesInFront
       noiseAlg.KeepOnlyCellID = flags.LArNoise.KeepOnlyCellID
       noiseAlg.TrigDecisionTool = tdt
       result.addEventAlgo(noiseAlg)

       result.addService(CompFactory.THistSvc(Output=["TTREE DATAFILE='"+flags.LArNoise.outNtupLAr+"' OPT='RECREATE'",]))

    if (flags.LArNoise.HECNoiseNtup!=""):
       hecAlg=CompFactory.LArHECNoise("LArHECNoise")
       hecAlg.MinDigitADC = flags.LArNoise.MinDigitADC
       hecAlg.MaxDeltaT = flags.LArNoise.MaxDeltaT
       from IOVDbSvc.IOVDbSvcConfig import addFolders
       result.merge(addFolders(flags, "/LAR/ElecCalibFlat/Pedestal", "LAR_ONL", className="CondAttrListCollection", db="CONDBR2"))
       LArPedestalCondAlg =  CompFactory.getComp("LArFlatConditionsAlg<LArPedestalFlat>")
       result.addCondAlgo(LArPedestalCondAlg(ReadKey="/LAR/ElecCalibFlat/Pedestal", WriteKey="LArPedestal"))

       result.addEventAlgo(hecAlg)
       if result.getService("THistSvc") is not None:
           result.getService("THistSvc").Output += ["HEC DATAFILE='"+flags.LArNoise.HECNoiseNtup+"' OPT='RECREATE'",]
       else:   
           result.addService(CompFactory.THistSvc(Output=["HEC DATAFILE='"+flags.LArNoise.HECNoiseNtup+"' OPT='RECREATE'",]))

    if (flags.LArNoise.outHistLAr != ""):
       from LArCellRec.LArNoisyROSummaryConfig import LArNoisyROSummaryCfg
       result.merge(LArNoisyROSummaryCfg(flags))
       from LArMonitoring.LArNoisyROMonAlg import LArNoisyROMonConfig 
       result.merge(LArNoisyROMonConfig(flags, inKey="LArNoisyROSummary"))

    return result



if __name__=="__main__":
    
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags=initConfigFlags()
    
    from LArNoiseFlags import addNoiseFlags
    addNoiseFlags(flags)
    #flags.Input.Files=['/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrigP1Test/data23_cos.00457007.physics_CosmicMuons.merge.RAW._lb0043._SFO-ALL._0001.1']
    flags.Input.Files=['/eos/atlas/atlastier0/rucio/data23_13p6TeV/express_express/00461002/data23_13p6TeV.00461002.express_express.merge.RAW/data23_13p6TeV.00461002.express_express.merge.RAW._lb0922._SFO-ALL._0001.1']

    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3

    flags.Trigger.triggerConfig = 'DB'    
    flags.Trigger.doID=False
    flags.Trigger.doMuon=False
    flags.Trigger.doLVL1=False
    flags.Trigger.doHLT=False
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    
    cfg=MainServicesCfg(flags)
    #cfg.merge(LArNoiseCfg(flags))
    cfg.merge(LArNoiseFromRawCfg(flags))

    cfg.run(10)
