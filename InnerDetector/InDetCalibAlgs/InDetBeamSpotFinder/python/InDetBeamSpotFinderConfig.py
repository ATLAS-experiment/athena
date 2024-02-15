# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of InDetBeamSpotFinder algorithms
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def InDetBeamSpotRooFitCfg(flags, name="InDetBeamSpotRooFit", **kwargs):
    result = ComponentAccumulator()
    
    result.setPrivateTools(
        CompFactory.InDet.InDetBeamSpotRooFit(name, **kwargs))
    return result

def InDetBeamSpotVertexCfg(flags, name="InDetBeamSpotVertex", **kwargs):
    result = ComponentAccumulator()

    kwargs.setdefault("MaxSigmaTr", 20.)
    kwargs.setdefault("OutlierChi2Tr", 20.)
    kwargs.setdefault("TruncatedRMS", True)

    result.setPrivateTools(
        CompFactory.InDet.InDetBeamSpotVertex(name, **kwargs))
    return result

def InDetBeamSpotFinderCfg(flags, name="InDetBeamSpotFinder", **kwargs):
    # Add BunchCrossingCondData
    from LumiBlockComps.BunchCrossingCondAlgConfig import BunchCrossingCondAlgCfg
    result = BunchCrossingCondAlgCfg(flags)

    if "BeamSpotToolList" not in kwargs:
        kwargs.setdefault("BeamSpotToolList", [
            result.addPublicTool(result.popToolsAndMerge(
                InDetBeamSpotRooFitCfg(flags))),
            result.addPublicTool(result.popToolsAndMerge(
                InDetBeamSpotVertexCfg(flags))) ])

    kwargs.setdefault("VertexTreeName", "Vertices") 
    kwargs.setdefault("VertexTypes", ["PriVtx"])

    result.addEventAlgo(CompFactory.InDet.InDetBeamSpotFinder(name, **kwargs))
    return result
