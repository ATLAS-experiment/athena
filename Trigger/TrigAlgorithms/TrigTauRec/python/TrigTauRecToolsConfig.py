# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.SystemOfUnits import mm

def trigTauVertexFinderCfg(flags, name=''):
    '''Algorithm that overwrites numTrack() and charge() of tauJets in container'''
    acc = ComponentAccumulator()

    acc.setPrivateTools(CompFactory.TauVertexFinder(
        name                            = name,
        UseTJVA                         = False,
        AssociatedTracks                = 'GhostTrack',
        InDetTrackSelectionToolForTJVA  = '',
        Key_trackPartInputContainer     = '',
        Key_vertexInputContainer        = '',
        OnlineMaxTransverseDistance     = 2.5*mm,   
        OnlineMaxZ0SinTheta             = 3.0*mm,
        TVATool                         = '', 
    ))

    return acc

def trigTauTrackFinderCfg(flags, name='', TrackParticlesContainer=''):
    '''Tau track association'''
    acc = ComponentAccumulator()

    from TrkConfig.TrkVertexFitterUtilsConfig import AtlasTrackToVertexIPEstimatorCfg
    AtlasTrackToVertexIPEstimator = acc.popToolsAndMerge(AtlasTrackToVertexIPEstimatorCfg(flags))

    from TrackToVertex.TrackToVertexConfig import TrackToVertexCfg
    TrackToVertexTool = acc.popToolsAndMerge(TrackToVertexCfg(flags))

    from TrackToCalo.TrackToCaloConfig import ParticleCaloExtensionToolCfg
    ParticleCaloExtensionTool = acc.popToolsAndMerge(ParticleCaloExtensionToolCfg(flags))

    from InDetConfig.InDetTrackSelectorToolConfig import TrigTauInDetTrackSelectorToolCfg
    TrigTauInDetTrackSelectorTool = acc.popToolsAndMerge(TrigTauInDetTrackSelectorToolCfg(flags))

    acc.setPrivateTools(CompFactory.TauTrackFinder(
        name                            = name,
        MaxJetDrTau                     = 0.2,
        MaxJetDrWide                    = 0.4,
        TrackSelectorToolTau            = TrigTauInDetTrackSelectorTool,
        TrackToVertexTool               = TrackToVertexTool,
        Key_trackPartInputContainer     = TrackParticlesContainer,
        maxDeltaZ0wrtLeadTrk            = 0.75*mm,
        removeTracksOutsideZ0wrtLeadTrk = True,
        ParticleCaloExtensionTool       = ParticleCaloExtensionTool,
        BypassSelector                  = False,
        BypassExtrapolator              = True,
        tauParticleCache                = "",
        TrackToVertexIPEstimator        = AtlasTrackToVertexIPEstimator,
    ))

    return acc

def tauVertexVariablesCfg(flags, name=''):
    '''Vertex variables calculation'''
    acc = ComponentAccumulator()

    from TrkConfig.TrkVertexFittersConfig import TauAdaptiveVertexFitterCfg
    TauAdaptiveVertexFitter = acc.popToolsAndMerge(TauAdaptiveVertexFitterCfg(flags))

    from TrkConfig.TrkVertexSeedFinderToolsConfig import CrossDistancesSeedFinderCfg
    CrossDistancesSeedFinder = acc.popToolsAndMerge(CrossDistancesSeedFinderCfg(flags))

    acc.setPrivateTools(CompFactory.TauVertexVariables(
        name            = name,
        VertexFitter    = TauAdaptiveVertexFitter,
        SeedFinder      = CrossDistancesSeedFinder,
    ))

    return acc
    
def trigTauJetLVNNEvaluatorCfg(flags, tau_id='', use_taujet_rnnscore=True):
    '''TauJet identification inference based on LVNN models, for RNNs and DeepSets'''
    acc = ComponentAccumulator()

    try: id_flags = getattr(flags.Trigger.Offline.Tau, tau_id)
    except NameError: raise ValueError(f'Invalid LVNN (RNN, DeepSet) TauID configuration: {tau_id}')

    # For legacy mediumRNN/tightRNN_tracktwoMVA/tracktwoLLP/trackLRT chains
    output_variable = 'RNNJetScore' if use_taujet_rnnscore else f'{tau_id}_Score'

    # This is to prevent clashes between the same ID running over the legacy chains, and the new decorator-based chains
    sfx = '_RNNJetScore' if use_taujet_rnnscore else ''

    acc.setPrivateTools(CompFactory.TauJetRNNEvaluator(
        name                = f'TrigTau_TauJetLVNNEvaluator_{tau_id}{sfx}',

        # Network config:
        NetworkFile0P       = id_flags.NetworkConfig[0],
        NetworkFile1P       = id_flags.NetworkConfig[1],
        NetworkFile3P       = id_flags.NetworkConfig[2],
        InputLayerScalar    = 'scalar',
        InputLayerTracks    = 'tracks',
        InputLayerClusters  = 'clusters',
        OutputLayer         = 'rnnid_output',
        OutputNode          = 'sig_prob',

        # Inputs:
        MaxTracks           = id_flags.MaxTracks, 
        MaxClusters         = id_flags.MaxClusters,
        MaxClusterDR        = 1.0,
        VertexCorrection    = False,
        TrackClassification = False,

        # Decorated TauJet variable names:
        OutputVarname       = output_variable,
    ))

    return acc

def trigTauJetONNXEvaluatorCfg(flags, tau_id=''):
    '''TauJet identification inference based on ONNX models, for GNNs, transformers, etc...'''
    acc = ComponentAccumulator()

    try: id_flags = getattr(flags.Trigger.Offline.Tau, tau_id)
    except NameError: raise ValueError(f'Invalid ONNX TauID configuration: {tau_id}')

    acc.setPrivateTools(CompFactory.TauGNNEvaluator(
        name                = f'TrigTau_TauJetONNXEvaluator_{tau_id}',

        # Network config:
        NetworkFile0P       = id_flags.ONNXConfig[0],
        NetworkFile1P       = id_flags.ONNXConfig[1],
        NetworkFile3P       = id_flags.ONNXConfig[2],
        InputLayerScalar    = 'tau_vars',
        InputLayerTracks    = 'track_vars',
        InputLayerClusters  = 'cluster_vars',
        NodeNameTau         = 'pTau',
        NodeNameJet         = 'pJet',

        # Inputs:
        MaxTracks           = id_flags.MaxTracks,
        MaxClusters         = id_flags.MaxClusters,
        MaxClusterDR        = 1.0,
        VertexCorrection    = False,
        TrackClassification = False,

        # Decorated TauJet variable names:
        OutputVarname       = f'{tau_id}_Score',
        OutputDiscriminant  = id_flags.OutputDiscriminant,
        OutputPTau          = f'{tau_id}_ProbTau',
        OutputPJet          = f'{tau_id}_ProbJet',

        # Tau prongness selection
        MinProngTrackPt     = id_flags.MinProngTrackPt if hasattr(id_flags, 'MinProngTrackPt') else 0,
    ))

    return acc

def trigTauWPDecoratorRNNCfg(flags, tau_id: str, precision_seq_name: str):
    '''
    TauJet signal transformed score and ID WPs decorator tool,
    for the legacy mediumRNN/tightRNN_tracktwoMVA/tracktwoLLP/trackLRT chains ONLY!
    '''

    acc = ComponentAccumulator()

    try: id_flags = getattr(flags.Trigger.Offline.Tau, tau_id)
    except NameError: raise ValueError(f'Invalid TauID configuration: {tau_id}')

    # In this version we store the WPs as flags accessable through
    # tau->isTau(xAOD::TauJetParameters::IsTauFlags::JetRNNSig...)
    import PyUtils.RootUtils as ru
    ROOT = ru.import_root()
    import cppyy
    cppyy.load_library('libxAODTau_cDict')
    WPEnumVals = [
        ROOT.xAOD.TauJetParameters.IsTauFlag.JetRNNSigVeryLoose,
        ROOT.xAOD.TauJetParameters.IsTauFlag.JetRNNSigLoose,
        ROOT.xAOD.TauJetParameters.IsTauFlag.JetRNNSigMedium,
        ROOT.xAOD.TauJetParameters.IsTauFlag.JetRNNSigTight,
    ]

    acc.setPrivateTools(CompFactory.TauWPDecorator(
        name=f'TrigTau_TauWPDecoratorRNN_{precision_seq_name}_{tau_id}',
        flatteningFile0Prong=id_flags.ScoreFlatteningConfig[0],
        flatteningFile1Prong=id_flags.ScoreFlatteningConfig[1],
        flatteningFile3Prong=id_flags.ScoreFlatteningConfig[2],
        CutEnumVals=WPEnumVals,
        SigEff0P=id_flags.TargetEff[0],
        SigEff1P=id_flags.TargetEff[1],
        SigEff3P=id_flags.TargetEff[2],
        ScoreName='RNNJetScore',
        NewScoreName='RNNJetScoreSigTrans',
        DefineWPs=True,
    ))

    return acc

def trigTauWPDecoratorCfg(flags, tau_id: str, precision_seq_name: str):
    '''TauJet signal transformed score and ID WPs decorator tool'''
    acc = ComponentAccumulator()

    try: id_flags = getattr(flags.Trigger.Offline.Tau, tau_id)
    except NameError: raise ValueError(f'Invalid TauID configuration: {tau_id}')

    acc.setPrivateTools(CompFactory.TauWPDecorator(
        name=f'TrigTau_TauWPDecoratorRNN_{precision_seq_name}_{tau_id}',
        flatteningFile0Prong=id_flags.ScoreFlatteningConfig[0],
        flatteningFile1Prong=id_flags.ScoreFlatteningConfig[1],
        flatteningFile3Prong=id_flags.ScoreFlatteningConfig[2],
        DecorWPNames=[f'{tau_id}_{wp}' for wp in id_flags.WPNames],
        DecorWPCutEffs0P=id_flags.TargetEff[0],
        DecorWPCutEffs1P=id_flags.TargetEff[1],
        DecorWPCutEffs3P=id_flags.TargetEff[2],
        ScoreName=f'{tau_id}_Score',
        NewScoreName=f'{tau_id}_ScoreSigTrans',
        DefineWPs=True,
    ))

    return acc
