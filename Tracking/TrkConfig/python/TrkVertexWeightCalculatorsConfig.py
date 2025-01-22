# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
# Configuration of TrkVertexWeightCalculators package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def SumPt2VertexWeightCalculatorCfg(flags, name="SumPt2VertexWeightCalculator",
                                    **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("DoSumPt2Selection", True)
    acc.setPrivateTools(
        CompFactory.Trk.SumPtVertexWeightCalculator(name, **kwargs))
    return acc


def SumPtVertexWeightCalculatorCfg(flags, name="SumPtVertexWeightCalculator",
                                   **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("DoSumPt2Selection", False)
    acc.setPrivateTools(
        CompFactory.Trk.SumPtVertexWeightCalculator(name, **kwargs))
    return acc


def GNNVertexWeightCalculatorCfg(flags, name="GNNVertexWeightCalculator",
                                 **kwargs):
    acc = ComponentAccumulator()
    acc.setPrivateTools(
        CompFactory.GNNVertexWeightCalculator(name, **kwargs))
    return acc

def BDTVertexWeightCalculatorCfg(flags, **kwargs):
    """
    Configure the BDTVertexWeightCalculator. Note: this tool needs to be run after the
    DecoratePhotonPointingAlg and BuildVertexPointingAlg.
    Use BDTVertexWeightCalculatorSeqCfg to have the full sequence.
    """
    acc = ComponentAccumulator()
    kwargs.setdefault(
        "BDTFile", "PhotonVertexSelection/BDT/2023-02-28/global_ggHW_phcount_BDT.root"
    )
    kwargs.setdefault("BDTName", "lgbm")
    kwargs.setdefault("PointingVertexContainerKey", "PhotonPointingVertices")
    tool = CompFactory.BDTVertexWeightCalculator(
        "BDTVertexWeightCalculator", **kwargs
    )
    acc.setPrivateTools(tool)
    return acc


def BDTVertexWeightCalculatorSeqCfg(flags, container='Photons', **kwargs):
    """
    Configure BDTVertexWeightCalculator and the algorithms that are needed to run before.
    Optional parameters are passed only to the tool.
    """
    acc = ComponentAccumulator()

    from PhotonVertexSelection.PhotonVertexSelectionConfig import (
        DecoratePhotonPointingAlgCfg,
        BuildVertexPointingAlgCfg,
    )

    # this algorithm decorates the photons with the pointing information
    acc.merge(DecoratePhotonPointingAlgCfg(flags, "DecoratePhotonPointingAlg", PhotonContainerKey=container))

    # this algorithm creates the vertex from photon pointing
    acc.merge(
        BuildVertexPointingAlgCfg(
            flags,
            "BuildVertexPointingAlg",
            PhotonContainerKey=container,
            PointingVertexContainerKey=kwargs.get(
                "PointingVertexContainerKey", "PhotonPointingVertices"
            ),
        )
    )

    accTool = BDTVertexWeightCalculatorCfg(flags, **kwargs)
    tool = acc.popToolsAndMerge(accTool)
    acc.setPrivateTools(tool)
    return acc


def JetRestrictedSumPt2VertexWeightCalculatorCfg(
    flags,
    name="JetRestrictedSumPt2VertexWeightCalculator",
    TrackParticleLocation="InDetTrackParticles",
    **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("DoSumPt2Selection", True)
    kwargs.setdefault("JetContainer", "AntiKt4EMTopoJets")
    acc.setPrivateTools(
        CompFactory.Trk.JetRestrictedSumPtVertexWeightCalculator(
            name,
            TracksInConeTool=CompFactory.xAOD.TrackParticlesInConeTool(
                'JetVertexTracksInCone',
                TrackParticleLocation=TrackParticleLocation
            ),
            **kwargs,
    ))
    return acc
