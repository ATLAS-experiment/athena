# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def TruthSegmentMakerCfg(flags, name = "TruthSegmentMakerAlg", **kwargs):
    result = ComponentAccumulator()
    if not flags.Input.isMC:
        return result

    from ActsAlignmentAlgs.AlignmentAlgsConfig import ActsGeometryContextAlgCfg
    result.merge(ActsGeometryContextAlgCfg(flags))
    containerNames = []
    ## If the tester runs on MC add the truth information
    if flags.Detector.EnableMDT: containerNames+=["MDT_SDO"]       
    if flags.Detector.EnableRPC: containerNames+=["RPC_SDO"]
    if flags.Detector.EnableTGC: containerNames+=["TGC_SDO"]
    if flags.Detector.EnableMM: containerNames+=["MM_SDO"]
    if flags.Detector.EnablesTGC: containerNames+=["sTGC_SDO"] 
    kwargs.setdefault("SimHitKeys", containerNames)

    the_alg = CompFactory.MuonR4.TruthSegmentMaker(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def MeasToSimHitAssocAlgCfg(flags, name="MeasToSimHitConvAlg", **kwargs):
    result = ComponentAccumulator()
    from ActsAlignmentAlgs.AlignmentAlgsConfig import ActsGeometryContextAlgCfg
    result.merge(ActsGeometryContextAlgCfg(flags))
    the_alg = CompFactory.MuonR4.PrepDataToSimHitAssocAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)    
    return result

def TruthHitAssociationCfg(flags):
    result = ComponentAccumulator()
    if not flags.Input.isMC:
        return result
    from MuonConfig.MuonDataPrepConfig import PrimaryMeasContNamesCfg
    for cont_name in PrimaryMeasContNamesCfg(flags):
        simHits = ""
        if "xMdt" in cont_name: 
            simHits = "MDT_SDO"
        elif "xRpc" in cont_name:
            simHits = "RPC_SDO"
        elif "xTgc" in cont_name:
            simHits = "TGC_SDO"
        elif "MM" in cont_name:
            simHits = "MM_SDO"
        else:
            simHits = "sTGC_SDO"
        result.merge(MeasToSimHitAssocAlgCfg(flags,
                                             name=f"{cont_name}PrepDataToSimHitAssoc",
                                             SimHits = simHits,
                                             Measurements=cont_name,
                                             AssocPull = 1. if cont_name=="xAODsTgcPads" else 3. ))
    return result

def PrdMultiTruthMakerCfg(flags):
    result = ComponentAccumulator()
    from MuonConfig.MuonDataPrepConfig import PrimaryMeasContNamesCfg
    prdContainer = PrimaryMeasContNamesCfg(flags)
    if flags.Detector.GeometryMDT: 
        the_alg = CompFactory.MuonR4.PrdMultiTruthMaker("PrdMultiTruthMakerMdt",
                                                        PrdContainer = [prd for prd in prdContainer if "xMdt" in prd], 
                                                        WriteKey = "MDT_TruthMap")
        result.addEventAlgo(the_alg)
    if flags.Detector.GeometryRPC:
        the_alg = CompFactory.MuonR4.PrdMultiTruthMaker("PrdMultiTruthMakerRpc",
                                                        PrdContainer = [prd for prd in prdContainer if "xRpc" in prd], 
                                                        WriteKey = "RPC_TruthMap")
        result.addEventAlgo(the_alg)
    if flags.Detector.GeometryTGC: 
        the_alg = CompFactory.MuonR4.PrdMultiTruthMaker("PrdMultiTruthMakerTgc",
                                                        PrdContainer = [prd for prd in prdContainer if "xTgc" in prd],
                                                        WriteKey = "TGC_TruthMap")
        result.addEventAlgo(the_alg)       
    if flags.Detector.GeometryMM: 
        the_alg = CompFactory.MuonR4.PrdMultiTruthMaker("PrdMultiTruthMakerMm",
                                                        PrdContainer = [prd for prd in prdContainer if "MM" in prd], 
                                                        WriteKey = "MM_TruthMap")
        result.addEventAlgo(the_alg) 
    if flags.Detector.GeometrysTGC: 
        the_alg = CompFactory.MuonR4.PrdMultiTruthMaker("PrdMultiTruthMakerSTGC",
                                                        PrdContainer = [prd for prd in prdContainer if "sTgc" in prd], 
                                                        WriteKey = "STGC_TruthMap")
        result.addEventAlgo(the_alg) 
    return result

def TruthSegmentToTruthPartCfg(flags, name="MuonTruthSegmentToTruthAssocAlg", **kwargs):
    result = ComponentAccumulator()
    hitDecors = []
    if flags.Detector.GeometryMDT:
        hitDecors+=["truthMdtHits"]
    if flags.Detector.GeometryRPC:
        hitDecors+=["truthRpcHits"]
    if flags.Detector.GeometryTGC:
        hitDecors+=["truthTgcHits"]
    if flags.Detector.GeometryMM:
        hitDecors+=["truthMMHits"]
    if flags.Detector.GeometrysTGC:
        hitDecors+=["truthStgcHits"]
    kwargs.setdefault("SimHitIds", hitDecors)
    the_alg = CompFactory.MuonR4.TruthSegToTruthPartAssocAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

@AccumulatorCache
def MuonTruthAlgsCfg(flags):
    result = ComponentAccumulator()
    if not flags.Input.isMC:
        return result
    result.merge(TruthHitAssociationCfg(flags))
    from MuonConfig.MuonDataPrepConfig import PrimaryMeasContNamesCfg
    PrdLinkInputs = [( "xAOD::UncalibratedMeasurementContainer" , 
                     "StoreGateSvc+{cont_name}.simHitLink".format(cont_name = cont_name)) for cont_name in PrimaryMeasContNamesCfg(flags) ]
    result.merge(TruthSegmentMakerCfg(flags, ExtraInputs = PrdLinkInputs))
    result.merge(PrdMultiTruthMakerCfg(flags))
    from MuonConfig.MuonTruthAlgsConfig import MuonTruthClassificationAlgCfg, MuonTruthHitCountsAlgCfg

    result.merge(MuonTruthClassificationAlgCfg(flags))
    result.merge(MuonTruthHitCountsAlgCfg(flags))
    #### Disable for the moment because tracking geometry explodes for R4
    ### from MuonConfig.MuonTruthAlgsConfig import MuonTruthAddTrackRecordsAlgCfg
    ### result.merge(MuonTruthAddTrackRecordsAlgCfg(flags))
    result.merge(TruthSegmentToTruthPartCfg(flags))
    return result
