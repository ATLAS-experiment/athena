#  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
# Configuration of InDetHardScatterSelectionTool package

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from TrkConfig.VertexFindingFlags import VertexSortingSetup

def InDetHardScatterSelectionToolCfg(flags, name="InDetHardScatterSelectionTool", **kwargs):
    """Configure the InDet hard scatter selection tool"""
    acc = ComponentAccumulator()

    sortingSetup = flags.Tracking.PriVertex.sortingSetup
    selectionMode = -1
    if sortingSetup is VertexSortingSetup.SumPt2Sorting:
        selectionMode = 0
    elif sortingSetup is VertexSortingSetup.SumPtSorting:
        selectionMode = 1
    elif sortingSetup is VertexSortingSetup.JetWeightedSorting:
        selectionMode = 2
        kwargs.setdefault("JetContainer", "AntiKt4EMTopoJets")
    elif sortingSetup is VertexSortingSetup.GNNSorting:
        selectionMode = 3

    kwargs.setdefault("SelectionMode", selectionMode)

    acc.setPrivateTools(
        CompFactory.InDet.InDetHardScatterSelectionTool(name, **kwargs))
    return acc
