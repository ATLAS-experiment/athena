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

    if flags.Detector.EnableMDT: 
        result.merge(MeasToSimHitAssocAlgCfg(flags,
                                             name="MdtPrepDataToSimHitAssoc",
                                             SimHits = "MDT_SDO",
                                             Measurements="xMdtDriftCircles"))
        result.merge(MeasToSimHitAssocAlgCfg(flags,
                                             name="MdtTwinPrepDataToSimHitAssoc",
                                             SimHits = "MDT_SDO",
                                             Measurements="xMdtTwinDriftCircles"))

    if flags.Detector.EnableRPC: 
        result.merge(MeasToSimHitAssocAlgCfg(flags,
                                             name="RpcPrepDataToSimHitAssoc",
                                             SimHits = "RPC_SDO",
                                             Measurements="xRpcMeasurements"))
        
    if flags.Detector.EnableTGC: 
        result.merge(MeasToSimHitAssocAlgCfg(flags,
                                             name="TgcPrepDataToSimHitAssoc",
                                             SimHits = "TGC_SDO",
                                             Measurements="xTgcStrips"))
    if flags.Detector.EnableMM: 
        result.merge(MeasToSimHitAssocAlgCfg(flags,
                                             name="MmPrepDataToSimHitAssoc",
                                             SimHits = "MM_SDO",
                                             Measurements="xAODMMClusters"))

    if flags.Detector.EnablesTGC: 
        result.merge(MeasToSimHitAssocAlgCfg(flags,
                                             name="sTgcStripToSimHitAssoc",
                                             SimHits = "sTGC_SDO",
                                             Measurements="xAODsTgcStrips"))
        result.merge(MeasToSimHitAssocAlgCfg(flags,
                                             name="sTgcWireToSimHitAssoc",
                                             SimHits = "sTGC_SDO",
                                             Measurements="xAODsTgcWires"))
        result.merge(MeasToSimHitAssocAlgCfg(flags,
                                             name="sTgcPadToSimHitAssoc",
                                             SimHits = "sTGC_SDO",
                                             Measurements="xAODsTgcPads",
                                             AssocPull=1.))
    return result

def MuonTruthAlgsCfg(flags):
    result = ComponentAccumulator()
    result.merge(TruthHitAssociationCfg(flags))
    PrdLinkInputs = []
    if flags.Detector.EnableMDT: 
        PrdLinkInputs+=[( 'xAOD::UncalibratedMeasurementContainer' , 'StoreGateSvc+xMdtDriftCircles.simHitLink' )]       
        PrdLinkInputs+=[( 'xAOD::UncalibratedMeasurementContainer' , 'StoreGateSvc+xMdtTwinDriftCircles.simHitLink' )]       
    if flags.Detector.EnableRPC: 
        PrdLinkInputs+=[ ('xAOD::UncalibratedMeasurementContainer' , 'StoreGateSvc+xRpcMeasurements.simHitLink' )]
    if flags.Detector.EnableTGC: 
        PrdLinkInputs+=[('xAOD::UncalibratedMeasurementContainer' , 'StoreGateSvc+xTgcStrips.simHitLink' )] 
    if flags.Detector.EnableMM:
        PrdLinkInputs+=[('xAOD::UncalibratedMeasurementContainer' , 'StoreGateSvc+xAODMMClusters.simHitLink' )] 

    if flags.Detector.EnablesTGC: 
        PrdLinkInputs+=[('xAOD::UncalibratedMeasurementContainer' , 'StoreGateSvc+xAODsTgcStrips.simHitLink' )] 
        PrdLinkInputs+=[('xAOD::UncalibratedMeasurementContainer' , 'StoreGateSvc+xAODsTgcWires.simHitLink' )] 
        PrdLinkInputs+=[('xAOD::UncalibratedMeasurementContainer' , 'StoreGateSvc+xAODsTgcPads.simHitLink' )] 

    result.merge(TruthSegmentMakerCfg(flags, ExtraInputs =PrdLinkInputs ))
    return result
