# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def PFlowAugmentationToolCfg(ConfigFlags):
    """Configure the PFlow augmentation tool"""
    acc = ComponentAccumulator()
    wPFOTool = CompFactory.getComp('CP::WeightPFOTool')("PFAugmentationWeightTool",DoEoverPWeight=True)
    pfoAugTool = CompFactory.DerivationFramework.PFlowAugmentationTool("PFlowAugmentationTool",
                                                                       WeightPFOTool=wPFOTool)
    acc.addPublicTool(pfoAugTool, primary=True)
    return acc


def TVAAugmentationToolCfg(ConfigFlags, preFix, workingPoint="Nominal"):
    """Configure the TVA augmentation tool"""
    acc = ComponentAccumulator()

    tvaTool =  CompFactory.getComp('CP::TrackVertexAssociationTool')(workingPoint+"TVATool",
                                                                     WorkingPoint=workingPoint)

    tvaAugTool = CompFactory.DerivationFramework.TVAAugmentationTool(workingPoint+"TVAAugmentationTool",
                                                                     LinkName = preFix+workingPoint+"TVA",
                                                                     TrackName = "InDetTrackParticles",
                                                                     TVATool = tvaTool)

    acc.addPublicTool(tvaAugTool, primary=True)
    return acc
