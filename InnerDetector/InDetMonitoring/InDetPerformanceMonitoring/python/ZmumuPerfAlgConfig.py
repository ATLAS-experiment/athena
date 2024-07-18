#
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

"""
@file ZmumuPerfAlgConfig.py
@author Salvador Marti
@date 2024
@brief Configuration for Run 3 IDAlignment performance based on Zmumu events
"""

################################################################## 
def ZmumuPerfAlgCfg(flags, **kwargs):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    # add the HistSvc to the component accumulator
    from AthenaConfiguration.ComponentFactory     import CompFactory
    histsvc = CompFactory.THistSvc(name="THistSvc", Output=flags.Output.HISTFileName)
    acc.addService ( histsvc )

    # track extrapolator
    from TrkConfig.AtlasExtrapolatorConfig import InDetExtrapolatorCfg
    InDetExtrapolator = acc.popToolsAndMerge(InDetExtrapolatorCfg(flags))
    acc.addPublicTool(InDetExtrapolator)

    # Track to vertex
    from TrackToVertex.TrackToVertexConfig import TrackToVertexCfg
    TrackToVertexTool = acc.popToolsAndMerge(TrackToVertexCfg(flags))
    acc.addPublicTool(TrackToVertexTool)

    # track to IP
    from TrkConfig.TrkVertexFitterUtilsConfig import TrackToVertexIPEstimatorCfg
    TrackToVertexIPEstimatorTool = acc.popToolsAndMerge(TrackToVertexIPEstimatorCfg(flags))
    acc.addPublicTool(TrackToVertexIPEstimatorTool)
    
    # Track refit with looser chi2
    from TrkConfig.TrkGlobalChi2FitterConfig import InDetGlobalChi2FitterCfg
    GX2TrackFitter = acc.popToolsAndMerge(InDetGlobalChi2FitterCfg(flags, TrackChi2PerNDFCut = 10))

    # Refitter tool: 
    MuonRefitterTool = CompFactory.egammaTrkRefitterTool (name         = 'MuonRefitterTool',
                                                          FitterTool   = GX2TrackFitter,
                                                          matEffects   = 2,
                                                          OutputLevel  = 4)
    acc.addPublicTool(MuonRefitterTool)

    # refitter tool for Silicon Only tracks (removing TRT hits)
    SiOnlyRefitterTool = CompFactory.egammaTrkRefitterTool (name          = 'SiOnlyRefitterTool',
                                                            FitterTool    = GX2TrackFitter,
                                                            matEffects    = 2,
                                                            RemoveTRTHits = True,
                                                            OutputLevel   = 4)
    acc.addPublicTool(SiOnlyRefitterTool)

    # muon selector tool
    from MuonSelectorTools.MuonSelectorToolsConfig import MuonSelectionToolCfg
    MuonSelectorIDAl = acc.popToolsAndMerge(MuonSelectionToolCfg(flags,
                                                                 name            = "MuonSelectorIDalign",
                                                                 MaxEta          = 3.,
                                                                 MuQuality       = 1,
                                                                 #IsRun3Geo       = False,
                                                                 TurnOffMomCorr  = True)) 
    acc.addPublicTool(MuonSelectorIDAl)
    

    #Configure the ID performance monitoring using Zmumu events 
    theIDPerfMonZmumu = CompFactory.IDPerfMonZmumu (name                     = "IDPerfMonZmumu",
                                                    TrackParticleName        = 'CombinedTrackParticle',
                                                    isMC                     = flags.Input.isMC,
                                                    UseTrackSelectionTool    = True,
                                                    doRefit                  = True,
                                                    ReFitterTool1            = MuonRefitterTool, 
                                                    ReFitterTool2            = SiOnlyRefitterTool, 
                                                    OutputTracksName         = "SelectedMuons",
                                                    Extrapolator             = InDetExtrapolator,
                                                    doIPextrToPV             = False,
                                                    UseTrigger               = True, 
                                                    TrackToVertexTool        = TrackToVertexTool,
                                                    TrackToVertexIPEstimator = TrackToVertexIPEstimatorTool,
                                                    commonTreeFolder         = "/ZmumuValidationUserSel/common",
                                                    MinLumiBlock             = 0, # if MaxLumiBlock == MinLumiBlock --> no LumiBlock selection
                                                    MaxLumiBlock             = 0,
                                                    doZmumuEventDebug        = False, # default False
                                                    useCustomMuonSelector    = True, ## default selector need calibrated pT --> needed for MC
                                                    MuonSelector             = MuonSelectorIDAl,
                                                    MassWindowLow            = 0.1, # in GeV
                                                    MassWindowHigh           = 2000., #in GeV
                                                    PtLeadingMuon            = 5., #in GeV
                                                    PtSecondMuon             = 5., 
                                                    **kwargs) 
    acc.addEventAlgo(theIDPerfMonZmumu)

    return acc

