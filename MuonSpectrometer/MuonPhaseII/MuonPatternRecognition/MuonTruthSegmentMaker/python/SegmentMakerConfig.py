# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
def TruthSegmentMakerCfg(flags, name = "TruthSegmentMakerAlg", **kwargs):
    result = ComponentAccumulator()
    from ActsAlignmentAlgs.AlignmentAlgsConfig import ActsGeometryContextAlgCfg
    result.merge(ActsGeometryContextAlgCfg(flags))
    containerNames = []
    ## If the tester runs on MC add the truth information
    if flags.Input.isMC:
        if flags.Detector.EnableMDT: containerNames+=["xMdtSimHits"]       
        if flags.Detector.EnableRPC: containerNames+=["xRpcSimHits"]
        if flags.Detector.EnableTGC: containerNames+=["xTgcSimHits"]
        if flags.Detector.EnableMM: containerNames+=["xMmSimHits"]
        if flags.Detector.EnablesTGC: containerNames+=["xStgcSimHits"] 
    kwargs.setdefault("SimHitKeys", containerNames)
    the_alg = CompFactory.MuonR4.TruthSegmentMaker(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result
