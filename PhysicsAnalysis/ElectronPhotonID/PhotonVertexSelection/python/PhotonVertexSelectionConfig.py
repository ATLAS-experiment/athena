# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def PhotonPointingToolCfg(flags, name="PhotonPointingTool", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("isSimulation", flags.Input.isMC)
    acc.setPrivateTools(CompFactory.CP.PhotonPointingTool(name, **kwargs))
    return acc


def PhotonVertexSelectionToolCfg(flags, name="PhotonVertexSelectionTool", **kwargs):
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.CP.PhotonVertexSelectionTool(**kwargs))
    return acc


def DecoratePhotonPointingAlgCfg(flags, name="DecoratePhotonPointingAlg", **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("PhotonContainerKey", "Photons")

    if not kwargs.get("PhotonPointingTool", None):
        toolAcc = PhotonPointingToolCfg(flags, ContainerName=kwargs['PhotonContainerKey'])
        tool = toolAcc.popPrivateTools()
        acc.merge(toolAcc)
        kwargs["PhotonPointingTool"] = tool

    alg = CompFactory.DecoratePhotonPointingAlg("DecoratePhotonPointingAlg", **kwargs)
    acc.addEventAlgo(alg)
    return acc


def BuildVertexPointingAlgCfg(flags, name="BuildVertexPointingAlg", **kwargs):
    from AthenaCommon.SystemOfUnits import GeV

    acc = ComponentAccumulator()
    kwargs.setdefault(
        "selectionTool",
        CompFactory.CP.AsgPtEtaSelectionTool(
            "CutPhotonPtEta",
            minPt=20 * GeV,
            maxEta=2.37,
            etaGapLow=1.37,
            etaGapHigh=1.52,
            useClusterEta=True,
        ),
    )
    kwargs.setdefault(
        "goodPhotonSelectionTool",
        CompFactory.CP.AsgFlagSelectionTool(
            "GoodPhotonSelectionTool",
            selectionFlags=[
                "Tight"
            ],  # TODO: why not DFCommonPhotonsIsEMTight (not working)?
        ),
    )
    kwargs.setdefault("nphotonsToUse", 2)
    kwargs.setdefault("PhotonContainerKey", "Photons")
    alg = CompFactory.BuildVertexPointingAlg("BuildVertexPointingAlg", **kwargs)
    acc.addEventAlgo(alg)
    return acc

