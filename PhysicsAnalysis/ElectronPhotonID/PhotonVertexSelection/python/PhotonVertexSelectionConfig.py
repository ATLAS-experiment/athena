# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def PhotonPointingToolCfg(flags, name="PhotonPointingTool", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("isSimulation", flags.Input.isMC)
    acc.setPrivateTools(CompFactory.CP.PhotonPointingTool(name, **kwargs))
    return acc


def PhotonVertexSelectionToolCfg(
        flags, name="PhotonVertexSelectionTool", **kwargs):
    acc = ComponentAccumulator()
    acc.setPrivateTools( CompFactory.CP.PhotonVertexSelectionTool( **kwargs ) )
    return acc


def DecoratePhotonPointingAlgCfg(flags, name="DecoratePhotonPointingAlg", **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("PhotonContainerKey", "Photons")

    if "PhotonPointingTool" not in kwargs:
        kwargs.setdefault("PhotonPointingTool", acc.popToolsAndMerge(
           PhotonPointingToolCfg(flags, ContainerName=kwargs['PhotonContainerKey'])))

    acc.addEventAlgo(CompFactory.DecoratePhotonPointingAlg(name, **kwargs))
    return acc
