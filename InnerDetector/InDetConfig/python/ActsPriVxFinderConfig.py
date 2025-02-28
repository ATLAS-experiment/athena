# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def ActsPriVxFinderCfg(flags,
                        name: str = "ActsPriVxFinder",
                        **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    if "VertexCollectionSortingTool" not in kwargs:
        from TrkConfig.TrkVertexToolsConfig import (
            VertexCollectionSortingToolCfg)
        kwargs.setdefault("VertexCollectionSortingTool", acc.popToolsAndMerge(
            VertexCollectionSortingToolCfg(flags)))

    if "VertexFinderTool" not in kwargs:
        from InDetConfig.InDetPriVxFinderToolConfig import (
            VertexFinderToolCfg)
        kwargs.setdefault("VertexFinderTool", acc.popToolsAndMerge(
            VertexFinderToolCfg(flags)))

    kwargs.setdefault("doVertexSorting", True)
    acc.addEventAlgo(CompFactory.InDet.InDetPriVxFinder(name, **kwargs))
    return acc


def primaryVertexFindingCfg(flags,
                            name: str = "InDetPriVxFinder",
                            *,
                            TracksName: str,
                            vxCandidatesOutputName: str = None,
                            **kwargs) -> ComponentAccumulator:
    # Handle vertex collection name
    if not vxCandidatesOutputName:
        vxCandidatesOutputName = "PrimaryVertices"
        from AthenaConfiguration.Enums import ProductionStep
        if flags.Common.ProductionStep in [ProductionStep.MinbiasPreprocessing]:
            vxCandidatesOutputName = f"{flags.Overlay.BkgPrefix}{vxCandidatesOutputName}"

    # Compute the primary vertex on the track particle collection
    # Creation of track particles is handled externally
    acc = ActsPriVxFinderCfg(flags,
                             name = name,
                             VxCandidatesOutputName = vxCandidatesOutputName,
                             TracksName = TracksName,
                             **kwargs)

    # Persistification to both AOD and ESD
    from OutputStreamAthenaPool.OutputStreamConfig import addToESD, addToAOD
    
    excludedVtxAuxData = "-vxTrackAtVertex.-MvfFitInfo.-isInitialized.-VTAV.-TruthEventMatchingInfos.-TruthEventRawMatchingInfos.-VertexMatchType"
    verticesContainer = [
        f"xAOD::VertexContainer#{vxCandidatesOutputName}",
        f"xAOD::VertexAuxContainer#{vxCandidatesOutputName}Aux." + excludedVtxAuxData,
    ]

    acc.merge(addToAOD(flags, verticesContainer))
    acc.merge(addToESD(flags, verticesContainer))
    return acc

