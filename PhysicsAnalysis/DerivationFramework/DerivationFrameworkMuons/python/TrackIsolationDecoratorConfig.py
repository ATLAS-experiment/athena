# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# Import the xAOD isolation parameters.
from xAODPrimitives.xAODIso import xAODIso as isoPar
deco_ptcones = [isoPar.ptcone40, isoPar.ptcone30]
deco_topoetcones = [isoPar.topoetcone40, isoPar.topoetcone20]
deco_prefix = 'MUON_'

def TrackIsolationToolCfg(ConfigFlags,name= "TrackIsolationTool", **kwargs):

    acc = ComponentAccumulator()
    from InDetTrackSelectionTool.InDetTrackSelectionToolConfig import (
        InDetTrackSelectionTool_TrackTools_Cfg)
    kwargs.setdefault("TrackSelectionTool",acc.popToolsAndMerge(InDetTrackSelectionTool_TrackTools_Cfg(ConfigFlags,
                                                                                     maxZ0SinTheta = 3.,
                                                                                     minPt         = 1000.,
                                                                                     CutLevel      = "Loose")))
       
    TrackIsoTool = CompFactory.xAOD.TrackIsolationTool(name, **kwargs)
    acc.setPrivateTools(TrackIsoTool)
    return acc


def MuonTrackIsolationDecorAlgCfg(
        ConfigFlags, name="MuonTrackIsolationDecorator",
        ttvaWP = "Nonprompt_All_MaxWeight", trackPt=500., **kwargs):

    result = ComponentAccumulator()
    from TrackVertexAssociationTool.TrackVertexAssociationToolConfig import isoTTVAToolCfg
    ttvaTool = result.popToolsAndMerge(
        isoTTVAToolCfg(ConfigFlags, WorkingPoint=ttvaWP))
    from InDetTrackSelectionTool.InDetTrackSelectionToolConfig import isoTrackSelectionToolCfg
    trackSelTool = result.popToolsAndMerge(
        isoTrackSelectionToolCfg(ConfigFlags, minPt=trackPt))
   
    wpName = "{WP}TTVA_pt{ptCut}".format(WP = ttvaWP, ptCut = trackPt)
    kwargs.setdefault("customName", wpName)
    ## Minimal pt cut on the ID tracks
    kwargs.setdefault("PtMin", 2500.)
    from IsolationAlgs.IsoToolsConfig import TrackIsolationToolCfg
    kwargs.setdefault("IsolationTool", result.popToolsAndMerge(
        TrackIsolationToolCfg(ConfigFlags,
                              TTVATool=ttvaTool,
                              TrackSelectionTool=trackSelTool)))
    theAlg = CompFactory.DerivationFramework.TrackIsolationDecorAlg(name = name, **kwargs)
    result.addEventAlgo(theAlg, primary = True)
    return result


def TrackCaloIsolationDecorAlgCfg(ConfigFlags, name = "MuonCaloIsolationDecorator", **kwargs):
    from IsolationAlgs.IsoToolsConfig import MuonCaloIsolationToolCfg
    result = ComponentAccumulator()
    kwargs.setdefault("PtMin", 2500.)
    kwargs.setdefault("IsolationTool", result.getPrimaryAndMerge(MuonCaloIsolationToolCfg(ConfigFlags, 
                                                                        saveOnlyRequestedCorrections=True)))
    the_alg = CompFactory.DerivationFramework.CaloIsolationDecorAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary=True)
    return result

def TrackPflowIsolationDecorAlgCfg(ConfigFlags, name = "MuonPflowIsolationDecorator", **kwargs):
    from IsolationAlgs.IsoToolsConfig import MuonCaloIsolationToolCfg
    result = ComponentAccumulator()
    kwargs.setdefault("PtMin", 2500.)
    kwargs.setdefault("IsolationTool", result.getPrimaryAndMerge(MuonCaloIsolationToolCfg(ConfigFlags, 
                                                                    saveOnlyRequestedCorrections=True)))
    the_alg = CompFactory.DerivationFramework.PflowIsolationDecorAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary=True)
    return result


def TrackIsolationCfg(ConfigFlags, TrackCollection="InDetTrackParticles", TrackSelections = []):
    result = ComponentAccumulator()
    for WP in ['Nonprompt_All_MaxWeight', 'Tight']:
        for trackPt in 500, 1000:
            result.merge(MuonTrackIsolationDecorAlgCfg(ConfigFlags,
                                                       name = "TrackIsoDecorAlg{container}{WP}{Pt}".format(container = TrackCollection,
                                                                                                           WP = WP, Pt = trackPt),
                                                       ttvaWP = WP,
                                                       trackPt = trackPt,
                                                       TrackCollection = TrackCollection,
                                                       TrackSelections = TrackSelections))
    result.merge(TrackCaloIsolationDecorAlgCfg(ConfigFlags,
                                               name = "CaloIsoDecorAlg{container}".format(container = TrackCollection),
                                               TrackCollection = TrackCollection,
                                               TrackSelections = TrackSelections))
    
    result.merge(TrackPflowIsolationDecorAlgCfg(ConfigFlags,
                                               name = "PflowIsoDecorAlg{container}".format(container = TrackCollection),
                                               TrackCollection = TrackCollection,
                                               TrackSelections = TrackSelections))
    return result
