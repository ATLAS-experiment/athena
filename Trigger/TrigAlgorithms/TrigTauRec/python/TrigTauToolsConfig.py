# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.SystemOfUnits import mm    

def trigTauVertexFinderCfg(flags,name=''):
    acc = ComponentAccumulator()

    # Algorithm that overwrites numTrack() and charge() of tauJets in container
    TauVertexFinder = CompFactory.TauVertexFinder(name=name,
                                      UseTJVA                        =        False,
                                      AssociatedTracks               = "GhostTrack",
                                      InDetTrackSelectionToolForTJVA =           "",
                                      Key_trackPartInputContainer    =           "",
                                      Key_vertexInputContainer       =           "",
                                      OnlineMaxTransverseDistance    =       2.5*mm,   
                                      OnlineMaxZ0SinTheta            =       3.0*mm,
                                      TVATool                        =           "", 
                                      )

    acc.setPrivateTools(TauVertexFinder)
    return acc

def trigTauTrackFinderCfg(flags,name='',TrackParticlesContainer=''):
    acc = ComponentAccumulator()

    from TrkConfig.TrkVertexFitterUtilsConfig import (
        AtlasTrackToVertexIPEstimatorCfg)
    AtlasTrackToVertexIPEstimator = acc.popToolsAndMerge(
        AtlasTrackToVertexIPEstimatorCfg(flags))

    from TrackToVertex.TrackToVertexConfig import TrackToVertexCfg
    TrackToVertexTool = acc.popToolsAndMerge(TrackToVertexCfg(flags))

    from TrackToCalo.TrackToCaloConfig import ParticleCaloExtensionToolCfg
    ParticleCaloExtensionTool = acc.popToolsAndMerge(
        ParticleCaloExtensionToolCfg(flags))

    from InDetConfig.InDetTrackSelectorToolConfig import (
        TrigTauInDetTrackSelectorToolCfg)
    TrigTauInDetTrackSelectorTool = acc.popToolsAndMerge(
        TrigTauInDetTrackSelectorToolCfg(flags))


    TauTrackFinder = CompFactory.TauTrackFinder(name=name,
                                    MaxJetDrTau                     = 0.2,
                                    MaxJetDrWide                    = 0.4,
                                    TrackSelectorToolTau            = TrigTauInDetTrackSelectorTool,
                                    TrackToVertexTool               = TrackToVertexTool,
                                    Key_trackPartInputContainer     = TrackParticlesContainer,
                                    maxDeltaZ0wrtLeadTrk            = 0.75*mm, #in mm
                                    removeTracksOutsideZ0wrtLeadTrk = True,
                                    ParticleCaloExtensionTool       = ParticleCaloExtensionTool,
                                    BypassSelector                  = False,
                                    BypassExtrapolator              = True,
                                    tauParticleCache                = "",
                                    TrackToVertexIPEstimator        = AtlasTrackToVertexIPEstimator,
                                    )

                                    
    acc.setPrivateTools(TauTrackFinder)
    return acc

def tauVertexVariablesCfg(flags,name=''):
    acc = ComponentAccumulator()

    from TrkConfig.TrkVertexFittersConfig import TauAdaptiveVertexFitterCfg
    TauAdaptiveVertexFitter = acc.popToolsAndMerge(
        TauAdaptiveVertexFitterCfg(flags))

    from TrkConfig.TrkVertexSeedFinderToolsConfig import (
        CrossDistancesSeedFinderCfg)
    CrossDistancesSeedFinder = acc.popToolsAndMerge(
        CrossDistancesSeedFinderCfg(flags))

    TauVertexVariables = CompFactory.TauVertexVariables(name=name,
                                                        VertexFitter =  TauAdaptiveVertexFitter ,
                                                        SeedFinder =    CrossDistancesSeedFinder  )
    acc.setPrivateTools(TauVertexVariables)
    return acc
    
def trigTauJetRNNEvaluatorCfg(flags,name='',LLP = False):

    acc = ComponentAccumulator()

    name += '_LLP' if LLP else ''

    (NetworkFile0P, NetworkFile1P, NetworkFile3P) = \
        flags.Trigger.Offline.Tau.TauJetRNNConfigLLP if LLP \
        else flags.Trigger.Offline.Tau.TauJetRNNConfig


    MyTauJetRNNEvaluator = CompFactory.TauJetRNNEvaluator(name = name,
                                            NetworkFile0P = NetworkFile0P,
                                            NetworkFile1P = NetworkFile1P,
                                            NetworkFile3P = NetworkFile3P,
                                            OutputVarname = "RNNJetScore",
                                            MaxTracks = 10,
                                            MaxClusters = 6,
                                            MaxClusterDR = 1.0,
                                            VertexCorrection = False,
                                            TrackClassification = False,
                                            InputLayerScalar = "scalar",
                                            InputLayerTracks = "tracks",
                                            InputLayerClusters = "clusters",
                                            OutputLayer = "rnnid_output",
                                            OutputNode = "sig_prob")


    acc.setPrivateTools(MyTauJetRNNEvaluator)
    return acc

def trigTauWPDecoratorJetRNNCfg(flags,name='',LLP = False):
    import PyUtils.RootUtils as ru
    ROOT = ru.import_root()
    import cppyy
    cppyy.load_library('libxAODTau_cDict')

    acc = ComponentAccumulator()
    name += '_LLP' if LLP else ''

    # currently the target efficiencies are the same for regular tau ID and LLP tau ID
    # if we need different WPs, we can define new flags

    (flatteningFile0Prong, flatteningFile1Prong, flatteningFile3Prong) = \
        flags.Trigger.Offline.Tau.TauJetRNNWPConfigLLP if LLP \
        else flags.Trigger.Offline.Tau.TauJetRNNWPConfig

    (targetEff0Prong, targetEff1Prong, targetEff3Prong) = \
        flags.Trigger.Offline.Tau.TauJetRNNLLPTargetEff if LLP \
        else flags.Trigger.Offline.Tau.TauJetRNNTargetEff

    MyTauWPDecorator =CompFactory.TauWPDecorator( name=name,
                                     flatteningFile0Prong = flatteningFile0Prong,
                                     flatteningFile1Prong = flatteningFile1Prong,
                                     flatteningFile3Prong = flatteningFile3Prong,
                                     CutEnumVals =
                                     [ ROOT.xAOD.TauJetParameters.IsTauFlag.JetRNNSigVeryLoose, ROOT.xAOD.TauJetParameters.IsTauFlag.JetRNNSigLoose,
                                       ROOT.xAOD.TauJetParameters.IsTauFlag.JetRNNSigMedium, ROOT.xAOD.TauJetParameters.IsTauFlag.JetRNNSigTight ],
                                     SigEff0P = targetEff0Prong,
                                     SigEff1P = targetEff1Prong,
                                     SigEff3P = targetEff3Prong,
                                     ScoreName = "RNNJetScore",
                                     NewScoreName = "RNNJetScoreSigTrans",
                                     DefineWPs = True )

    acc.setPrivateTools(MyTauWPDecorator)
    return acc
