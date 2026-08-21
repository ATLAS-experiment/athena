# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def getTTVAToolForReco(name="TTVATool", **kwargs):
    
    # set default values for AMVF decoration properties, in case they are not set
    # passing other values in kwargs will override these, if you are using a custom vertex container (eg. HIGG1D1 jet reconstruction)
    kwargs.setdefault("AMVFVerticesDeco", "TTVA_AMVFVertices_forReco")
    kwargs.setdefault("AMVFWeightsDeco",  "TTVA_AMVFWeights_forReco")

    return CompFactory.CP.TrackVertexAssociationTool(name, **kwargs)

def TTVAToolCfg(flags, name, addDecoAlg=True, VertexContName="PrimaryVertices", **kwargs):
    """Create a component accumulator containing a TTVA tool

    If addDecoAlg is True, also adds an algorithm for decorating the 'used-in-fit' information
    """

    acc = ComponentAccumulator()

    kwargs.setdefault("TrackContName", "InDetTrackParticles")
    kwargs.setdefault("AMVFVerticesDeco", "TTVA_AMVFVertices_forReco")
    kwargs.setdefault("AMVFWeightsDeco",  "TTVA_AMVFWeights_forReco")

    acc.setPrivateTools(getTTVAToolForReco(name, **kwargs))

    if addDecoAlg:
        from InDetUsedInFitTrackDecoratorTool.UsedInVertexFitTrackDecoratorConfig import (
            UsedInVertexFitTrackDecoratorCfg)
        acc.merge(UsedInVertexFitTrackDecoratorCfg(
            flags, trackCont=kwargs["TrackContName"], vtxCont=VertexContName,
            vertexDeco=kwargs["AMVFVerticesDeco"], weightDeco=kwargs["AMVFWeightsDeco"]))

    return acc


def isoTTVAToolCfg(flags, name="ttvaToolForIso", **kwargs):
    kwargs.setdefault("WorkingPoint", "Nonprompt_All_MaxWeight")
    kwargs.setdefault("HardScatterLinkDeco", "")
    return TTVAToolCfg(flags, name, **kwargs)


def TauTTVAToolCfg(flags, name="TVATool", **kwargs):
    kwargs.setdefault("WorkingPoint", "Nonprompt_Hard_MaxWeight")
    kwargs.setdefault("HardScatterLinkDeco", "")
    kwargs.setdefault("TrackContName", flags.Tau.ActiveConfig.TrackCollection)
    kwargs.setdefault("VertexContName", flags.Tau.ActiveConfig.VertexCollection)
    return TTVAToolCfg(flags, flags.Tau.ActiveConfig.prefix + name, **kwargs)


def CVF_TTVAToolCfg(flags, name="CVF_TTVATool", **kwargs):
    kwargs.setdefault("WorkingPoint", "Custom")
    kwargs.setdefault("HardScatterLinkDeco", "")
    kwargs.setdefault("d0_cut", 2.0)
    kwargs.setdefault("dzSinTheta_cut", 2.0)
    return TTVAToolCfg(flags, name, **kwargs)
 

def GNNHS_TTVAToolCfg(flags, name="TrackVertexAssociationTool_GNNHS", **kwargs):
    # The import should be done here as TrkConfig is not part of AthAnalysis
    from TrkConfig.VertexFindingFlags import VertexSortingSetup

    kwargs.setdefault("WorkingPoint", "Prompt_MaxWeight")
    kwargs.setdefault("VertexContName", "PrimaryVertices_initial"
                      if flags.Tracking.PriVertex.sortingSetup is VertexSortingSetup.GNNSorting else
                      "PrimaryVertices")
    kwargs.setdefault("AMVFVerticesDeco", "TTVA_AMVFVertices_forGNN")
    kwargs.setdefault("AMVFWeightsDeco", "TTVA_AMVFWeights_forGNN")
    return TTVAToolCfg(flags, name, **kwargs)
