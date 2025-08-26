# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def SeedJetBuilderCfg(flags, name="DiTauRec_SeedJetBuilder", **kwargs):
    """Configure the seed jet builder"""
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.SeedJetBuilder(name, **kwargs))
    return acc


def SubjetBuilderCfg(flags, name="DiTauRec_SubjetBuilder", **kwargs):
    """Configure the subjet builder"""
    acc = ComponentAccumulator()

    kwargs.setdefault("Rsubjet", 0.2)
    kwargs.setdefault("ptminsubjet", 10000)

    acc.setPrivateTools(CompFactory.SubjetBuilder(name, **kwargs))
    return acc

def TVAToolCfg(flags, name="TVATool_forDiTaus", **kwargs):
    """Configure the TVA tool"""
    acc = ComponentAccumulator()

    kwargs.setdefault("TrackParticleContainer", "InDetTrackParticles")
    kwargs.setdefault("TrackVertexAssociation", "JetTrackVtxAssoc_forDiTaus")
    kwargs.setdefault("VertexContainer", "PrimaryVertices")
    kwargs.setdefault("MaxTransverseDistance", 2.5) # in mm
    kwargs.setdefault("MaxLongitudinalDistance", 2) # in mm

    acc.setPrivateTools(CompFactory.TrackVertexAssociationTool(name, **kwargs))
    return acc

def JetAlgCfg(flags, name="DiTauRec_JetAlgorithm", **kwargs): # Name changed wrt legacy config DiTauRec_TVATool
    """Configure the JetAlgorithm"""
    acc = ComponentAccumulator()

    tools = [acc.popToolsAndMerge(TVAToolCfg(flags))]
    kwargs.setdefault("Tools", tools)

    acc.addEventAlgo(CompFactory.JetAlgorithm(name, **kwargs))
    return acc

# require TrackVertexAssociation to be produced by TVA tool - see above
def VertexFinderCfg(flags, name="DiTauRec_VertexFinder", **kwargs):
    """Configure the vertex finder"""
    acc = ComponentAccumulator()

    kwargs.setdefault("PrimVtxContainerName", "PrimaryVertices")
    kwargs.setdefault("AssociatedTracks", "GhostTrack")
    kwargs.setdefault("TrackVertexAssociation", "JetTrackVtxAssoc_forDiTaus")
    kwargs.setdefault("UseTJVA", flags.Tau.doTJVA)

    acc.setPrivateTools(CompFactory.VertexFinder(name, **kwargs))
    return acc

def DiTauTrackFinderCfg(flags, name="DiTauRec_DiTauTrackFinder", **kwargs):
    """Configure the di-tau track finder"""    
    acc = ComponentAccumulator()
    
    kwargs.setdefault("MaxDrJet", 1.0)
    kwargs.setdefault("MaxDrSubjet", 0.2)
    kwargs.setdefault("MaxNTracksSubjet", -1)
    kwargs.setdefault("TrackParticleContainer", "InDetTrackParticles")

    if "TrackSelectorTool" not in kwargs:
        from InDetConfig.InDetTrackSelectorToolConfig import TauRecInDetTrackSelectorToolCfg
        InDetTrackSelectorTool = acc.popToolsAndMerge(TauRecInDetTrackSelectorToolCfg(flags))
        acc.addPublicTool(InDetTrackSelectorTool)
        kwargs.setdefault("TrackSelectorTool", InDetTrackSelectorTool)

    acc.setPrivateTools(CompFactory.DiTauTrackFinder(name, **kwargs))
    return acc

def CellFinderCfg(flags, name="DiTauRec_CellFinder", **kwargs):
    """Configure the cell finder"""
    acc = ComponentAccumulator()

    kwargs.setdefault("Rsubjet", 0.2)

    CellFinder = CompFactory.CellFinder(name, **kwargs)
    acc.setPrivateTools(CellFinder)
    return acc

def DiTauConstituentFinderCfg(flags, name="DiTauRec_DiTauConstituentFinder", **kwargs):
    """Configure the di-tau constituent finder"""
    acc = ComponentAccumulator()
    kwargs.setdefault("Rsubjet", 0.2)
    kwargs.setdefault("UseRawConstit", True)

    acc.setPrivateTools(CompFactory.DiTauConstituentFinder(name, **kwargs))
    return acc

def DiTauIDVarDecoratorCfg(flags, name="DiTauRec_IDVarDecorator", **kwargs):
    """Configure the IDVarDecorator"""
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.DiTauIDVarDecorator(name, **kwargs))
    return acc

def DiTauOnnxScoreCalculatorCfg(flags, name="DiTauRec_OnnxScoreCalculator", **kwargs):
    """Configure the OnnxScoreCalculator"""
    acc = ComponentAccumulator()
    kwargs.setdefault("onnxModelPath", "DiTauRec/omni.onnx")
    kwargs.setdefault("maxTracks", 10)
    acc.setPrivateTools(CompFactory.DiTauOnnxDiscriminantTool(name, **kwargs))
    return acc
