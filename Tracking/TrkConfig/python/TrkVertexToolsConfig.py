# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of TrkVertexTools package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from TrkConfig.VertexFindingFlags import VertexSortingSetup

def SumPt2VertexCollectionSortingToolCfg(flags, name="SumPt2VertexCollectionSortingTool", **kwargs):
    acc = ComponentAccumulator()

    if "VertexWeightCalculator" not in kwargs:
        from TrkConfig.TrkVertexWeightCalculatorsConfig import (
            SumPt2VertexWeightCalculatorCfg)
        kwargs.setdefault("VertexWeightCalculator", acc.popToolsAndMerge(
            SumPt2VertexWeightCalculatorCfg(flags)))

    kwargs.setdefault("decorationName", "sumPt2")

    acc.setPrivateTools(
        CompFactory.Trk.VertexCollectionSortingTool(name,**kwargs))
    return acc

def SumPtVertexCollectionSortingToolCfg(flags, name="SumPtVertexCollectionSortingTool", **kwargs):
    acc = ComponentAccumulator()
    
    if "VertexWeightCalculator" not in kwargs:
        from TrkConfig.TrkVertexWeightCalculatorsConfig import (
            SumPtVertexWeightCalculatorCfg)
        kwargs.setdefault("VertexWeightCalculator", acc.popToolsAndMerge(
            SumPtVertexWeightCalculatorCfg(flags)))

    kwargs.setdefault("decorationName", "sumPt")

    acc.setPrivateTools(
        CompFactory.Trk.VertexCollectionSortingTool(name,**kwargs))
    return acc


def JetRestrictedSumPt2VertexCollectionSortingToolCfg(flags, name="JetRestrictedSumPt2VertexCollectionSortingTool", **kwargs):
    acc = ComponentAccumulator()

    if "VertexWeightCalculator" not in kwargs:
        from TrkConfig.TrkVertexWeightCalculatorsConfig import (
            JetRestrictedSumPt2VertexWeightCalculatorCfg)
        kwargs.setdefault("VertexWeightCalculator", acc.popToolsAndMerge(
            JetRestrictedSumPt2VertexWeightCalculatorCfg(flags)))

    kwargs.setdefault("decorationName", "sumPt2")

    acc.setPrivateTools(
        CompFactory.Trk.VertexCollectionSortingTool(name,**kwargs))
    return acc

def GNNVertexCollectionSortingToolCfg(flags, name="GNNVertexCollectionSortingTool", **kwargs):
    acc = ComponentAccumulator()

    if "VertexWeightCalculator" not in kwargs:
        from TrkConfig.TrkVertexWeightCalculatorsConfig import GNNVertexWeightCalculatorCfg
        kwargs.setdefault("VertexWeightCalculator", acc.popToolsAndMerge(
            GNNVertexWeightCalculatorCfg(flags)))

    kwargs.setdefault("decorationName", "gnnScore")

    acc.setPrivateTools(
        CompFactory.Trk.VertexCollectionSortingTool(name,**kwargs))
    return acc

def VertexCollectionSortingToolCfg(flags, **kwargs):
    if flags.Tracking.PriVertex.sortingSetup == \
       VertexSortingSetup.SumPt2Sorting:
        return SumPt2VertexCollectionSortingToolCfg(flags, **kwargs)
    elif flags.Tracking.PriVertex.sortingSetup == \
         VertexSortingSetup.SumPtSorting:
        return SumPtVertexCollectionSortingToolCfg(flags, **kwargs)
    elif flags.Tracking.PriVertex.sortingSetup == \
         VertexSortingSetup.JetWeightedSumPt2Sorting:
        return JetRestrictedSumPt2VertexCollectionSortingToolCfg(flags, **kwargs)


def SecVertexMergingToolCfg(flags, name='SecVertexMergingTool', **kwargs):

  acc = ComponentAccumulator()

  if "VertexFitterTool" not in kwargs:
    from TrkConfig.TrkVertexFittersConfig import AdaptiveVertexFitterCfg
    kwargs.setdefault("VertexFitterTool", acc.popToolsAndMerge(AdaptiveVertexFitterCfg(flags)))

  kwargs.setdefault("MininumDistance", 5.0)
  kwargs.setdefault("CompatibilityDimension", 2)

  
  acc.setPrivateTools(CompFactory.Trk.SecVertexMergingTool(name,**kwargs))
  return acc
