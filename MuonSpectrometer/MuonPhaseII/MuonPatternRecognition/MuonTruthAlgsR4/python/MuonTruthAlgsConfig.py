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
    #### Mdt calib db alg
    from MuonConfig.MuonCalibrationConfig import MdtCalibDbAlgCfg, NswErrorCalibDbAlgCfg
    if flags.Detector.EnableMDT:
        result.merge(MdtCalibDbAlgCfg(flags))
    if flags.Detector.EnableMM or flags.Detector.EnablesTGC:
        result.merge(NswErrorCalibDbAlgCfg(flags))
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

def SimHitToTruthPartAlgCfg(flags, useSDO = False):
    result = ComponentAccumulator()
    if flags.Detector.GeometryMDT: 
        container = "xMdtSimHits" if not useSDO else "MDT_SDO"
        the_alg = CompFactory.MuonR4.SimHitToTruthPartAssocAlg("MuonMdtSimHitToTruthMappingAlg",
                                                        SimHitContainer = container, 
                                                        HitIdDecoration = "truthMdtHits")
        result.addEventAlgo(the_alg)
    if flags.Detector.GeometryRPC:
        container = "xRpcSimHits" if not useSDO else "RPC_SDO"
        the_alg = CompFactory.MuonR4.SimHitToTruthPartAssocAlg("MuonRpcSimHitToTruthMappingAlg",
                                                        SimHitContainer = container, 
                                                        HitIdDecoration = "truthRpcHits")
        result.addEventAlgo(the_alg)
    if flags.Detector.GeometryTGC: 
        container = "xTgcSimHits" if not useSDO else "TGC_SDO"
        the_alg = CompFactory.MuonR4.SimHitToTruthPartAssocAlg("MuonTgcSimHitToTruthMappingAlg",
                                                        SimHitContainer = container,
                                                        HitIdDecoration = "truthTgcHits")
        result.addEventAlgo(the_alg)       
    if flags.Detector.GeometryMM: 
        container = "xMmSimHits" if not useSDO else "MM_SDO"
        the_alg = CompFactory.MuonR4.SimHitToTruthPartAssocAlg("MuonMmSimHitToTruthMappingAlg",
                                                        SimHitContainer = container, 
                                                        HitIdDecoration = "truthMMHits")
        result.addEventAlgo(the_alg) 
    if flags.Detector.GeometrysTGC: 
        container = "xStgcSimHits" if not useSDO else "sTGC_SDO"
        the_alg = CompFactory.MuonR4.SimHitToTruthPartAssocAlg("MuonsTGCSimHitToTruthMappingAlg",
                                                        SimHitContainer = container, 
                                                        HitIdDecoration = "truthStgcHits")
        result.addEventAlgo(the_alg) 

    return result

def TruthHitSummaryAlgCfg(flags, name="MuonTruthHitSummaryAlg", **kwargs):
    result = ComponentAccumulator()
    from MuonTrackFindingAlgs.TrackFindingConfig import TrackSummaryToolCfg
    kwargs.setdefault("SummaryTool", result.popToolsAndMerge(TrackSummaryToolCfg(flags,  
                                                                fillHoles=False, fillOutliers=False, 
                                                                recomputeSegment =False)))
    the_alg = CompFactory.MuonR4.TruthHitSummaryAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def RecoSegmentTruthAssocCfg(flags, **kwargs):
    result = ComponentAccumulator()
    ### Ensure that this configuration is always set externally
    kwargs.setdefault("SegmentKey", "")
    the_alg = CompFactory.MuonR4.RecoSegToTruthAssocAlg(**kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def TruthSegmentToTruthPartAssocCfg(flags, name="MuonTruthSegmentToTruthAssocAlg", **kwargs):
    result = ComponentAccumulator()
    hitDecors = []
    if flags.Detector.GeometryMDT: hitDecors+=["truthMdtHits"]
    if flags.Detector.GeometryRPC: hitDecors+=["truthRpcHits"]
    if flags.Detector.GeometryTGC: hitDecors+=["truthTgcHits"]
    if flags.Detector.GeometryMM: hitDecors+=["truthMMHits"]
    if flags.Detector.GeometrysTGC: hitDecors+=["truthStgcHits"]
    kwargs.setdefault("SimHitIds", hitDecors)
    the_alg = CompFactory.MuonR4.TruthSegToTruthPartAssocAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def TrackToTruthPartAssocCfg(flags, **kwargs):
    result = ComponentAccumulator()
    hitDecors = []
    if flags.Detector.GeometryMDT: hitDecors+=["truthMdtHits"]
    if flags.Detector.GeometryRPC: hitDecors+=["truthRpcHits"]
    if flags.Detector.GeometryTGC: hitDecors+=["truthTgcHits"]
    if flags.Detector.GeometryMM: hitDecors+=["truthMMHits"]
    if flags.Detector.GeometrysTGC: hitDecors+=["truthStgcHits"]
    kwargs.setdefault("SimHitIds", hitDecors)
    the_alg = CompFactory.MuonR4.TrackToTruthPartAssocAlg(**kwargs)
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
   
    from MuonConfig.MuonTruthAlgsConfig import TruthMuonMakerAlgCfg
    result.merge(TruthMuonMakerAlgCfg(flags))
    result.merge(SimHitToTruthPartAlgCfg(flags))
    result.merge(TruthHitSummaryAlgCfg(flags))
    # result.merge(MuonTruthHitCountsAlgCfg(flags))
    #### Disable for the moment because tracking geometry explodes for R4
    ### from MuonConfig.MuonTruthAlgsConfig import MuonTruthAddTrackRecordsAlgCfg
    ### result.merge(MuonTruthAddTrackRecordsAlgCfg(flags))
    result.merge(TruthSegmentToTruthPartAssocCfg(flags))
    return result
