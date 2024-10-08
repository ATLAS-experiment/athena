# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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

def MuonTruthAlgsCfg(flags):
    result = ComponentAccumulator()
    result.merge(TruthHitAssociationCfg(flags))
    from MuonConfig.MuonDataPrepConfig import PrimaryMeasContNamesCfg
    PrdLinkInputs = [( "xAOD::UncalibratedMeasurementContainer" , 
                     "StoreGateSvc+{cont_name}.simHitLink".format(cont_name = cont_name)) for cont_name in PrimaryMeasContNamesCfg(flags) ]
    result.merge(TruthSegmentMakerCfg(flags, ExtraInputs = PrdLinkInputs ))
    return result
