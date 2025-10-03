# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def FPGAClusterSortingAlgCfg(flags, name="FPGAClusterSortingAlg", **kwargs):
    """Return a ComponentAccumulator configured for FPGAClusterSorting"""
    acc = ComponentAccumulator()

    kwargs.setdefault('xAODPixelClusterContainer', 'FPGAPixelClusters')
    kwargs.setdefault('xAODStripClusterContainer', 'FPGAStripClusters')
    kwargs.setdefault('sortedxAODPixelClusterContainer', 'SortedFPGAPixelClusters')
    kwargs.setdefault('sortedxAODStripClusterContainer', 'SortedFPGAStripClusters')
    
    ClustrerSorting = CompFactory.FPGAClusterSortingAlg(name,**kwargs)

    # Add the algorithm to the accumulator
    acc.addEventAlgo(ClustrerSorting)

    return acc


def FPGAClusterDataVectorSortingAlgCfg(flags, name="FPGAClusterDataVectorSortingAlg", **kwargs):
    """Return a ComponentAccumulator configured for FPGAClusterDataVectorSortingAlg"""
    acc = ComponentAccumulator()

    kwargs.setdefault('xAODPixelClusterContainer', 'FPGAPixelClusters')
    kwargs.setdefault('xAODStripClusterContainer', 'FPGAStripClusters')
    kwargs.setdefault('sortedxAODPixelClusterContainer', 'SortedFPGAPixelClusters')
    kwargs.setdefault('sortedxAODStripClusterContainer', 'SortedFPGAStripClusters')
    
    ClustrerSorting = CompFactory.FPGAClusterDataVectorSortingAlg(name,**kwargs)

    # Add the algorithm to the accumulator
    acc.addEventAlgo(ClustrerSorting)

    return acc