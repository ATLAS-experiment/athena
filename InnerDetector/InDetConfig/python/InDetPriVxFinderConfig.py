# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of InDetPriVxFinder package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def InDetPriVxFinderCfg(flags, name="InDetPriVxFinder", **kwargs):
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

    if flags.Tracking.perigeeExpression == "Vertex":
        from xAODTrackingCnv.xAODTrackingCnvConfig import (
            BeamLineTrackParticleCnvAlgCfg)
        from InDetConfig.TrackRecoConfig import (
            ClusterSplitProbabilityContainerName)
        acc.merge(BeamLineTrackParticleCnvAlgCfg(
            flags,
            ClusterSplitProbabilityName = \
            ClusterSplitProbabilityContainerName(flags),
            AssociationMapName = "PRDtoTrackMapCombinedInDetTracks",
            xAODTrackParticlesFromTracksContainerName = \
            "InDetTrackParticlesTemporary"))
        kwargs["TracksName"]="InDetTrackParticlesTemporary"

    acc.addEventAlgo(CompFactory.InDet.InDetPriVxFinder(name, **kwargs))
    return acc

def InDetTrigPriVxFinderCfg(flags, inputTracks, outputVtx, name="InDetTrigPriVxFinder",
                            **kwargs):

    acc = ComponentAccumulator()

    kwargs["TracksName"] = inputTracks
    kwargs["VxCandidatesOutputName"] = outputVtx
    
    if "VertexFinderTool" not in kwargs:
        from InDetConfig.InDetPriVxFinderToolConfig import (
            TrigVertexFinderToolCfg)
        kwargs.setdefault("VertexFinderTool", acc.popToolsAndMerge(
            TrigVertexFinderToolCfg(flags)))

    if "VertexCollectionSortingTool" not in kwargs:
        from TrkConfig.TrkVertexToolsConfig import (
            SumPt2VertexCollectionSortingToolCfg)
        kwargs.setdefault("VertexCollectionSortingTool", acc.popToolsAndMerge(
            SumPt2VertexCollectionSortingToolCfg(flags)))

    if "PriVxMonTool" not in kwargs:
        from InDetPriVxFinder.InDetPriVxFinderMonitoring import (
            InDetPriVxFinderMonitoringTool)
        kwargs.setdefault("PriVxMonTool", InDetPriVxFinderMonitoringTool(flags))

    kwargs.setdefault("doVertexSorting", True)

    acc.addEventAlgo(CompFactory.InDet.InDetPriVxFinder(name+flags.Tracking.ActiveConfig.input_name,
                                                        **kwargs))
    return acc


def primaryVertexFindingCfg(flags, vxCandidatesOutputName="PrimaryVertices"):

    acc = InDetPriVxFinderCfg(flags, VxCandidatesOutputName=vxCandidatesOutputName)

    if flags.Tracking.perigeeExpression == "Vertex":
        from xAODTrackingCnv.xAODTrackingCnvConfig import TrackParticleCnvAlgCfg
        from InDetConfig.TrackRecoConfig import (
            ClusterSplitProbabilityContainerName)
        acc.merge(TrackParticleCnvAlgCfg(
            flags,
            ClusterSplitProbabilityName=ClusterSplitProbabilityContainerName(
                flags),
            AssociationMapName="PRDtoTrackMapCombinedInDetTracks"))

    from OutputStreamAthenaPool.OutputStreamConfig import addToESD, addToAOD

    excludedVtxAuxData = "-vxTrackAtVertex.-MvfFitInfo.-isInitialized.-VTAV.-TruthEventMatchingInfos.-TruthEventRawMatchingInfos.-VertexMatchType"
    verticesContainer = [
        f"xAOD::VertexContainer#{vxCandidatesOutputName}",
        f"xAOD::VertexAuxContainer#{vxCandidatesOutputName}Aux." + excludedVtxAuxData,
    ]

    acc.merge(addToAOD(flags, verticesContainer))
    acc.merge(addToESD(flags, verticesContainer))

    return acc

if __name__ == "__main__":
    from AthenaCommon.Logging import logging
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from AthenaConfiguration.ComponentAccumulator import printProperties
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from TrkConfig.VertexFindingFlags import VertexSetup

    flags.Input.Files = defaultTestFiles.RDO_RUN2
    import sys
    if 'ActsGaussAdaptiveMultiFinding' in sys.argv:
        flags.Tracking.PriVertex.setup = VertexSetup.ActsGaussAMVF
    elif "IterativeFinding" in sys.argv:
        flags.Tracking.PriVertex.setup = VertexSetup.IVF
    elif "FastIterativeFinding" in sys.argv:
        flags.Tracking.PriVertex.setup = VertexSetup.FastIVF
    flags.lock()

    acc = MainServicesCfg(flags)
    acc.merge(primaryVertexFindingCfg(flags))

    mlog = logging.getLogger("primaryVertexFindingConfigTest")
    mlog.info("Configuring  primaryVertexFinding: ")

    printProperties(
        mlog,
        acc.getEventAlgo("InDetPriVxFinder"),
        nestLevel=2,
        printDefaults=True,
    )

    acc.wasMerged()
