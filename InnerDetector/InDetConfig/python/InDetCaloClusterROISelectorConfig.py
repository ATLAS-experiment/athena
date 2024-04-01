# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
# Configuration of InDetCaloClusterROISelector package
from AthenaConfiguration.ComponentFactory import CompFactory
import AthenaCommon.SystemOfUnits as Units


def CaloClusterROIPhiRZContainerMakerCfg(
        flags, name="CaloClusterROIPhiRZContainerMaker", **kwargs):
    from egammaAlgs.egammaTopoClusterCopierConfig import (
        indetTopoClusterCopierCfg)
    result = indetTopoClusterCopierCfg(flags)

    if "CaloSurfaceBuilder" not in kwargs:
        from CaloTrackingGeometry.CaloTrackingGeometryConfig import (
            CaloSurfaceBuilderEntranceCfg)
        kwargs.setdefault("CaloSurfaceBuilder", result.popToolsAndMerge(
            CaloSurfaceBuilderEntranceCfg(flags)))

    kwargs.setdefault("InputClusterContainerName",
                      flags.Tracking.EgammaTopoClusters)
    kwargs.setdefault("EMEnergyOnly", True)

    OutputROIContainerName = []
    minPt = []
    phiWidth = []

    if flags.Tracking.BackTracking.doRoISeeded:
        # TRT_TrackSegmentsFinder
        # TRT_SeededTrackFinder
        pt_cut = flags.Tracking.BackTracking.minRoIClusterEt
        OutputROIContainerName.append(
            'InDetCaloClusterROIPhiRZ%.0fGeVBackTracking' % (pt_cut/Units.GeV))
        minPt.append(pt_cut)
        # no phi ordering, no Roi duplication close to +- pi
        phiWidth.append(0.)

    if flags.Tracking.doCaloSeededBrem:
        OutputROIContainerName.append('InDetCaloClusterROIPhiRZ0GeV')
        minPt.append(0)
        # must be equal or larger than phiWidth of its clients: InDetSiTrackMaker (phiWidt)
        phiWidth.append(flags.Tracking.phiWidthBrem)

        OutputROIContainerName.append('InDetCaloClusterROIPhiRZ5GeV')
        minPt.append(5000)
        # must be equal or larger than phiWidth of its clients: InDetNNScoringTool (phiWidthEM)
        phiWidth.append(0.075)

    if flags.Tracking.doCaloSeededAmbi:
        OutputROIContainerName.append('InDetCaloClusterROIPhiRZ10GeV')
        minPt.append(10000)
        # must be equal or larger than phiWidth of its clients: InDetAmbiTrackSelectionTool
        phiWidth.append(0.05)

    kwargs.setdefault("OutputROIContainerName", OutputROIContainerName)
    kwargs.setdefault("minPt", minPt)
    kwargs.setdefault("phiWidth", phiWidth)

    if "egammaCaloClusterSelector" not in kwargs:
        from egammaCaloTools.egammaCaloToolsConfig import (
            egammaCaloClusterSelectorCfg)
        kwargs.setdefault("egammaCaloClusterSelector", result.popToolsAndMerge(
            egammaCaloClusterSelectorCfg(flags)))

    result.addEventAlgo(CompFactory.InDet.CaloClusterROIPhiRZContainerMaker(
        name, **kwargs), primary=True)
    return result


def ITkCaloClusterROIPhiRZContainerMakerCfg(
        flags,
        name="ITkCaloClusterROIPhiRZContainerMaker",
        **kwargs):

    from egammaAlgs.egammaTopoClusterCopierConfig import (
        indetTopoClusterCopierCfg)
    result = indetTopoClusterCopierCfg(flags)

    if "CaloSurfaceBuilder" not in kwargs:
        from CaloTrackingGeometry.CaloTrackingGeometryConfig import (
            CaloSurfaceBuilderEntranceCfg)
        kwargs.setdefault("CaloSurfaceBuilder", result.popToolsAndMerge(
            CaloSurfaceBuilderEntranceCfg(flags)))

    kwargs.setdefault("InputClusterContainerName",
                      flags.Tracking.EgammaTopoClusters)
    kwargs.setdefault("EMEnergyOnly", True)

    OutputROIContainerName = []
    minPt = []
    phiWidth = []

    if flags.Tracking.doCaloSeededBrem:
        OutputROIContainerName.append('ITkCaloClusterROIPhiRZ0GeV')
        minPt.append(0)
        # value from central eta bin
        phiWidth.append(flags.Tracking.phiWidthBrem)
        # must be equal or larger than phiWidth of its clients: InDetSiTrackMaker (phiWidth)

        OutputROIContainerName.append('ITkCaloClusterROIPhiRZ5GeV')
        minPt.append(5000)
        # must be equal or larger than phiWidth of its clients: InDetNNScoringTool (phiWidthEM)
        phiWidth.append(0.075)

    if flags.Tracking.doCaloSeededAmbi:
        OutputROIContainerName.append('ITkCaloClusterROIPhiRZ10GeV')
        minPt.append(10000)
        # must be equal or larger than phiWidth of its clients: InDetAmbiTrackSelectionTool
        phiWidth.append(0.05)

    if flags.Tracking.doITkConversion or flags.Acts.doITkConversion:
        OutputROIContainerName.append('ITkCaloClusterROIPhiRZ15GeVUnordered')
        minPt.append(15000)
        # no phi ordering, no Roi duplication close to +- pi
        phiWidth.append(0.)

    kwargs.setdefault("OutputROIContainerName", OutputROIContainerName)
    kwargs.setdefault("minPt", minPt)
    kwargs.setdefault("phiWidth", phiWidth)

    if "egammaCaloClusterSelector" not in kwargs:
        from egammaCaloTools.egammaCaloToolsConfig import (
            egammaCaloClusterSelectorCfg)
        kwargs.setdefault("egammaCaloClusterSelector", result.popToolsAndMerge(
            egammaCaloClusterSelectorCfg(flags)))

    result.addEventAlgo(CompFactory.InDet.CaloClusterROIPhiRZContainerMaker(
        name, **kwargs), primary=True)
    return result


def HadCaloClusterROIPhiRZContainerMakerCfg(
        flags,
        name="HadCaloClusterROIPhiRZContainerMaker",
        **kwargs):
    from egammaAlgs.egammaTopoClusterCopierConfig import (
        indetTopoClusterCopierCfg)
    result = indetTopoClusterCopierCfg(flags)

    kwargs.setdefault("InputClusterContainerName",  "CaloCalTopoClusters")

    if "CaloSurfaceBuilder" not in kwargs:
        from CaloTrackingGeometry.CaloTrackingGeometryConfig import (
            CaloSurfaceBuilderEntranceCfg)
        kwargs.setdefault("CaloSurfaceBuilder", result.popToolsAndMerge(
            CaloSurfaceBuilderEntranceCfg(flags)))

    OutputROIContainerName = []
    minPt = []
    phiWidth = []

    if flags.Tracking.doHadCaloSeededSSS:
        OutputROIContainerName.append("InDetHadCaloClusterROIPhiRZ")
        minPt.append(0)
        # must be equal or larger than phiWidth of its clients: InDetSiTrackMaker (phiWidth)
        phiWidth.append(flags.Tracking.phiWidthBrem)

    if flags.Tracking.doCaloSeededAmbi:
        OutputROIContainerName.append("InDetHadCaloClusterROIPhiRZBjet")
        minPt.append(0)
        # must be equal or larger than phiWidth of its clients: InDetAmbiTrackSelectionTool
        phiWidth.append(0.05)

    kwargs.setdefault("OutputROIContainerName", OutputROIContainerName)
    kwargs.setdefault("minPt", minPt)
    kwargs.setdefault("phiWidth", phiWidth)

    if "egammaCaloClusterSelector" not in kwargs:
        from egammaCaloTools.egammaCaloToolsConfig import (
            egammaHadCaloClusterSelectorCfg)
        kwargs.setdefault("egammaCaloClusterSelector", result.popToolsAndMerge(
            egammaHadCaloClusterSelectorCfg(flags)))

    result.addEventAlgo(CompFactory.InDet.CaloClusterROIPhiRZContainerMaker(
        name, **kwargs), primary=True)
    return result


def ITkHadCaloClusterROIPhiRZContainerMakerCfg(
        flags,
        name="ITkHadCaloClusterROIPhiRZContainerMaker",
        **kwargs):
    from egammaAlgs.egammaTopoClusterCopierConfig import (
        indetTopoClusterCopierCfg)
    result = indetTopoClusterCopierCfg(flags)

    kwargs.setdefault("InputClusterContainerName",  "CaloCalTopoClusters")

    if "CaloSurfaceBuilder" not in kwargs:
        from CaloTrackingGeometry.CaloTrackingGeometryConfig import (
            CaloSurfaceBuilderEntranceCfg)
        kwargs.setdefault("CaloSurfaceBuilder", result.popToolsAndMerge(
            CaloSurfaceBuilderEntranceCfg(flags)))

    OutputROIContainerName = []
    minPt = []
    phiWidth = []

    if flags.Tracking.doHadCaloSeededSSS:
        OutputROIContainerName.append("ITkHadCaloClusterROIPhiRZ")
        minPt.append(0)
        # value from central eta bin
        phiWidth.append(flags.Tracking.phiWidthBrem)
        # must be equal or larger than phiWidth of its clients: InDetSiTrackMaker (phiWidth)

    if flags.Tracking.doCaloSeededAmbi:
        OutputROIContainerName.append("ITkHadCaloClusterROIPhiRZBjet")
        minPt.append(0)
        # must be equal or larger than phiWidth of its clients: InDetAmbiTrackSelectionTool
        phiWidth.append(0.05)

    kwargs.setdefault("OutputROIContainerName", OutputROIContainerName)
    kwargs.setdefault("minPt", minPt)
    kwargs.setdefault("phiWidth", phiWidth)

    if "egammaCaloClusterSelector" not in kwargs:
        from egammaCaloTools.egammaCaloToolsConfig import (
            egammaHadCaloClusterSelectorCfg)
        kwargs.setdefault("egammaCaloClusterSelector", result.popToolsAndMerge(
            egammaHadCaloClusterSelectorCfg(flags)))

    result.addEventAlgo(CompFactory.InDet.CaloClusterROIPhiRZContainerMaker(
        name, **kwargs), primary=True)
    return result
