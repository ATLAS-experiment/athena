# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def _assertPropertyValue(kwargs, key, force_value):
    if kwargs.setdefault(key, force_value) != force_value:
        raise ValueError(
            f"{key} property must be set to {force_value} (provided value is '{kwargs[key]}')"
        )

def getTTVAToolForReco(name="TTVATool", **kwargs):
    # First check that the user hasn't attempted to overwrite the AMVF vertices/weights decorations
    _assertPropertyValue(kwargs, "AMVFVerticesDeco", "TTVA_AMVFVertices_forReco")
    _assertPropertyValue(kwargs, "AMVFWeightsDeco", "TTVA_AMVFWeights_forReco")

    return CompFactory.CP.TrackVertexAssociationTool(name, **kwargs)
    

def TTVAToolCfg(flags, name, addDecoAlg=True, VertexContName="PrimaryVertices", **kwargs):
    """Create a component accumulator containing a TTVA tool

    If addDecoAlg is True, also adds an algorithm for decorating the 'used-in-fit' information
    """

    acc = ComponentAccumulator()

    kwargs.setdefault("TrackContName", "InDetTrackParticles")

    acc.setPrivateTools(getTTVAToolForReco(name, **kwargs))

    if addDecoAlg:
        from InDetConfig.UsedInVertexFitTrackDecoratorConfig import (
            UsedInVertexFitTrackDecoratorCfg)
        acc.merge(UsedInVertexFitTrackDecoratorCfg(
            flags, kwargs["TrackContName"], VertexContName))

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
 
