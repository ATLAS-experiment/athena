#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#

"""
@file ZeePerfAlgConfig.py
@author Guillem Arbona Ferrer
@date 2025
@brief Configuration for Run 3 IDAlignment performance based on Zee events
"""
#
################################################################## 
def EoverP_PerfAlgCfg(flags, **kwargs):
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
    ElectronRefitterTool = CompFactory.egammaTrkRefitterTool (name         = 'ElectronRefitterTool',
                                                              FitterTool   = GX2TrackFitter,
                                                              matEffects   = 2,
                                                              OutputLevel  = 4)
    acc.addPublicTool(ElectronRefitterTool)

    # refitter tool for Silicon Only tracks (removing TRT hits)
    SiOnlyRefitterTool = CompFactory.egammaTrkRefitterTool (name          = 'SiOnlyRefitterTool',
                                                            FitterTool    = GX2TrackFitter,
                                                            matEffects    = 2,
                                                            RemoveTRTHits = True,
                                                            OutputLevel   = 4)
    acc.addPublicTool(SiOnlyRefitterTool)

    #
    #Configure the ID performance monitoring using EoverP 
    theIDPerfMonEoverP = CompFactory.IDPerfMonEoverP (name                     = "IDPerfMonEoverP",
                                                      ReFitterTool = ElectronRefitterTool,
                                                      ReFitterTool2 = SiOnlyRefitterTool,
                                                      InputElectronContainerName = "Electrons",
                                                      #RefittedElectronTrackContainer1 = GSFTrackCollection,
                                                      #RefittedElectronTrackContainer2 = DNATrackCollection,
                                                      RefitTracks = False,
                                                      isDATA = True,
                                                      ValidationMode = True,
                                                      FillDetailedTree = True,
                                                      **kwargs) 
    acc.addEventAlgo( theIDPerfMonEoverP )

    return acc

