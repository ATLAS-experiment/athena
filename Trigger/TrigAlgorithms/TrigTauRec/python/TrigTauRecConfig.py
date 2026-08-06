# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

from TrigTauRec.TrigTauRecToolsConfig import trigTauJetONNXEvaluatorCfg, trigTauWPDecoratorCfg

from AthenaCommon.Logging import logging
log = logging.getLogger('TrigTauRecConfig')

def trigTauRecMergedPrecisionMVACfg(
    flags: AthConfigFlags,
    name: str,
    tau_ids: list[str] | None = None,
    input_rois: str = '',
    input_tracks: str = '',
    input_taus: str = '',
    input_tau_tracks: str = '',
    output_name: str = '',
    decors_to_copy: list[str] | None = None,
) -> ComponentAccumulator:
    '''
    Reconstruct the precision TauJet, from the first-step CaloMVA TauJet and precision-refitted tracks.

    :param flags: Config flags.
    :param name: Suffix for the main TrigTauRecMerged algorithm name.
    :param tau_ids: List of inference algorithms to execute.
                    The specific configuration will be loaded from the matching ConfigFlags (Trigger.Offline.Tau.<alg-name>)
                    Currently, only the `DeepSet` and `RNNLLP` algorithms will use the LVNN inference setup (json config files);
                    all other ID algorithms will use the ONNX inference setup by default.
                    If the algorithm name (`name` input variable) is `MVA`, `LLP` or `LRT`, and `tau_ids=['DeepSet', 'MesonCuts']` or `tau_ids=['RNNLLP']`,
                    then the default TauJet RNN score and WP `isTau` decorators will be used (for the legacy
                    `mediumRNN/tightRNN_tracktwoMVA/tracktwoLLP/trackLRT` triggers).
                    Otherwise, all scores and WPs will be stored as `{tau_id}_Score`, `{tau_id}_ScoreSigTrans`, and `{tau_id}_{wp_name}`.
    :param input_rois: RoIs container, where the reconstruction will be run.
    :param input_tracks: TrackParticle container, with the refitted precision tracks.
    :param input_taus: TauJet container, with the input tau jets.
    :param input_tau_tracks: TauTrack container, with the input tau tracks (usually dummies from the CaloMVA or CaloHits steps).
    :param output_name: Suffix for the output TauJet and TauTrack collections. If empty, `name` will be used.
    :param decors_to_copy: List of decorated/aux variables to copy from the input to the output TauJet container.

    :return: CA with the TauJet Precision reconstruction sequence.
    '''

    # Output collections
    if not output_name: output_name = name
    from TrigEDMConfig.TriggerEDM import recordable
    trigTauJetOutputContainer = recordable(f'HLT_TrigTauRecMerged_{output_name}')
    trigTauTrackOutputContainer = recordable(f'HLT_tautrack_{output_name}')

    # Main CA
    acc = ComponentAccumulator()


    # The TauJet reconstruction is handled by a set of tools, executed in the following order:
    vftools = []        # Vertex Finder tools
    tools_beforetf = [] # Common tools, ran before the Track Finder tools
    tftools = []        # Track Finder tools
    tools = []          # Common tools
    vvtools = []        # Vertex Vars tools
    idtools = []        # ID tools


    from TrigTauRec.TrigTauRecToolsConfig import trigTauVertexFinderCfg, trigTauTrackFinderCfg, tauVertexVariablesCfg
    from AthenaConfiguration.ComponentFactory import CompFactory

    # Associate RoI vertex or Beamspot to the tau - don't use TJVA
    vftools.append(acc.popToolsAndMerge(trigTauVertexFinderCfg(flags, name='TrigTau_TauVertexFinder')))
    
    # Set LC energy scale (0.2 cone) and intermediate axis (corrected for vertex: useless at trigger)       
    tools_beforetf.append(CompFactory.TauAxisSetter(name='TrigTau_TauAxis', VertexCorrection=False)) 

    # Associate tracks to the tau
    tftools.append(acc.popToolsAndMerge(trigTauTrackFinderCfg(flags, name='TrigTauTightDZ_TauTrackFinder', TrackParticlesContainer=input_tracks)))

    # Decorate the clusters
    tools.append(CompFactory.TauClusterFinder(name='TrigTau_TauClusterFinder', UseOriginalCluster=False))
    tools.append(CompFactory.TauVertexedClusterDecorator(name='TrigTau_TauVertexedClusterDecorator', SeedJet=''))

    # Calculate cell-based quantities: strip variables, EM and Had energies/radii, centFrac, isolFrac and ring energies
    tools.append(CompFactory.TauCellVariables(name='TrigTau_CellVariables', VertexCorrection=False))

    # Compute MVA TES (ATR-17649), stores MVA TES as default tau pt
    tools.append(CompFactory.MvaTESVariableDecorator(name='TrigTau_MvaTESVariableDecorator', Key_vertexInputContainer='', EventShapeKey='', VertexCorrection=False))
    acc.addPublicTool(tools[-1])
    tools.append(CompFactory.MvaTESEvaluator(name='TrigTau_MvaTESEvaluator', WeightFileName=flags.Trigger.Offline.Tau.MvaTESConfig))
    acc.addPublicTool(tools[-1])

    # Vertex variables
    vvtools.append(acc.popToolsAndMerge(tauVertexVariablesCfg(flags, name='TrigTau_TauVertexVariables')))

    # Variables combining tracking and calorimeter information
    idtools.append(CompFactory.TauCommonCalcVars(name='TrigTau_TauCommonCalcVars'))

    # Cluster-based sub-structure, with dRMax
    idtools.append(CompFactory.TauSubstructureVariables(name='TrigTau_TauSubstructure', VertexCorrection=False))

    #---------------------------------------------------------------
    # Tau ID and score flattenning
    #---------------------------------------------------------------
    # We can run multiple inferences at once. Each will be stored on different decorated variables
    # (or isTau(...) flags in the case of the legacy RNN/DeepSet tracktwoMVA/LLP/LRT triggers

    # We first "remove" the "ids" that don't require any inference, and any duplicates
    tau_ids = sorted(list(set(tau_ids if tau_ids else []) - {'perf', 'idperf', 'MesonCuts'}))

    from TriggerMenuMT.HLT.Tau.TauConfigurationTools import getTauIDScoreVariables
    id_score_monitoring = {}

    # We can only have at most one TauID algorithm score/WPs being stored in the built-in TauJet RNN variables
    used_builtin_rnnscore = False

    for tau_id in tau_ids:
        # First check that the TauID algorithm has the necesary config flags defined
        try: id_flags = getattr(flags.Trigger.Offline.Tau, tau_id)
        except NameError: raise ValueError(f'Missing TauID ConfigFlags: Trigger.Offline.Tau.{tau_id}')

        # Now check if it's an ONNX-based TauID, or an LVNN-based TauID
        is_onnx = hasattr(id_flags, 'ONNXConfig')

        if is_onnx: # ONNX inference
            log.debug('Configuring TrigTauRecMerged with the ONNX Tau ID score inference: %s', tau_id)

            # ONNX (GNTau) inference
            idtools.append(acc.popToolsAndMerge(trigTauJetONNXEvaluatorCfg(flags, tau_id=tau_id)))
            acc.addPublicTool(idtools[-1])

            # ID score flattening and WPs
            idtools.append(acc.popToolsAndMerge(trigTauWPDecoratorCfg(flags, tau_id=tau_id, precision_seq_name=name, tauContainerName=trigTauJetOutputContainer)))
            acc.addPublicTool(idtools[-1])


        else: # LVNN inference
            log.debug('Configuring TrigTauRecMerged %s with the LVNN Tau ID score inference: %s', name, tau_id)

            from TriggerMenuMT.HLT.Tau.TauConfigurationTools import useBuiltInTauJetRNNScore

            # To support the legacy tracktwoMVA/LLP/LRT chains, only in those cases we store the
            # passed WPs in the built-in TauJet variables
            use_builtin_rnnscore = useBuiltInTauJetRNNScore(tau_id)
            if use_builtin_rnnscore:
                if used_builtin_rnnscore:
                    log.error('Cannot store more than one TauID score in the built-in TauJet RNN score variables')
                    raise ValueError()
                used_builtin_rnnscore = True
            
            # LVNN (RNN/DeepSet) inference
            from TrigTauRec.TrigTauRecToolsConfig import trigTauJetLVNNEvaluatorCfg
            idtools.append(acc.popToolsAndMerge(trigTauJetLVNNEvaluatorCfg(flags, tau_id=tau_id, use_taujet_rnnscore=use_builtin_rnnscore)))
            acc.addPublicTool(idtools[-1])

            # ID score flattening and WPs
            if use_builtin_rnnscore:
                from TrigTauRec.TrigTauRecToolsConfig import trigTauWPDecoratorRNNCfg
                idtools.append(acc.popToolsAndMerge(trigTauWPDecoratorRNNCfg(flags, tau_id=tau_id, precision_seq_name=name)))
                acc.addPublicTool(idtools[-1])
            else:
                idtools.append(acc.popToolsAndMerge(trigTauWPDecoratorCfg(flags, tau_id=tau_id, precision_seq_name=name, tauContainerName=trigTauJetOutputContainer)))
                acc.addPublicTool(idtools[-1])

        id_score_monitoring[tau_id] = getTauIDScoreVariables(tau_id)


    # Set trigger-specific configuration for all the reconstruction tools
    for tool in vftools + tools_beforetf + tftools + tools + vvtools + idtools:
        tool.inTrigger = True
        tool.calibFolder = flags.Trigger.Offline.Tau.tauRecToolsCVMFSPath


    from TrigTauRec.TrigTauRecMonitoring import tauMonitoringPrecision
    acc.addEventAlgo(CompFactory.TrigTauRecMerged(
        name=f'TrigTauRecMerged_Precision_{name}',
        VertexFinderTools=vftools,
        CommonToolsBeforeTF=tools_beforetf,
        TrackFinderTools=tftools,
        CommonTools=tools,
        VertexVarsTools=vvtools,
        IDTools=idtools,
        MonTool=tauMonitoringPrecision(flags, RoI_name='tauLRT' if 'LRT' in name else 'tauIso', tau_ids=id_score_monitoring.keys(), alg_name=name),
        MonitoredIDScores=id_score_monitoring,
        InputRoIs=input_rois,
        InputVertexContainer=flags.Tracking.ActiveConfig.vertex,
        InputTauTrackContainer=input_tau_tracks,
        InputTauJetContainer=input_taus,
        InputTauJetCopyDecorKeys=decors_to_copy if decors_to_copy else [],
        OutputTauTrackContainer=trigTauTrackOutputContainer,
        OutputTauJetContainer=trigTauJetOutputContainer,
    ))

    return acc



def trigTauRecMergedCaloHitsCfg(
    flags: AthConfigFlags, 
    name: str, 
    hitz_algs: list[str] | None = None, 
    presel_algs: list[str] | None = None, 
    input_rois: str = '', 
) -> ComponentAccumulator:
    '''
    Reconstruct the precision TauJet, from the first-step CaloMVA TauJet and precision-refitted tracks.

    :param flags: Config flags.
    :param name: Suffix for the main TrigTauRecMerged algorithm name.
    :param hitz_algs: List of hitz inference algorithms to execute, using the ONNX inference setup.
                    The specific configuration will be loaded from the matching ConfigFlags (Trigger.Offline.Tau.<alg-name>)
    :param presel_algs: List of preselection inference algorithms to execute, using the ONNX inference setup.
                    The specific configuration will be loaded from the matching ConfigFlags (Trigger.Offline.Tau.<alg-name>).
                    For these algorithms, the score flattening and WP decorator tools will be used.
    :param input_rois: RoIs container, where the reconstruction will be run.

    :return: CA with the TauJet Precision reconstruction sequence.
    '''
    from TrigEDMConfig.TriggerEDM import recordable
    output_taus = recordable('HLT_TrigTauRecMerged_CaloHits')
    output_tau_tracks = 'HLT_tautrack_CaloHits_dummy'

    # Main CA
    acc = ComponentAccumulator()

    from AthenaConfiguration.ComponentFactory import CompFactory

    # Decorate pixel hits with their local position relative to the beamspot
    acc.addEventAlgo(CompFactory.FlavorTagDiscriminants.HitBeamSpotDataDecoratorAlg(
        name='TrigTauPixelHitsDecoratorAlg',
        hitContainer='PixelClusters',
        beamSpotKey='BeamSpotData', # Online beamspot conditions
    ))


    # Associate hits to the HLT TauJets
    # We use the previous step's TauJets as input, because we will use the hits in the tools 
    # executed within TrigTauRecMerged

    # We first need to figure out how many hits we'll need to associate
    max_hits = 0

    if hitz_algs is None: hitz_algs = []
    if presel_algs is None: presel_algs = []
    for alg in hitz_algs + presel_algs:
        # First check that the algorithm has the necesary config flags defined
        try: alg_flags = getattr(flags.Trigger.Offline.Tau, alg)
        except NameError: raise ValueError(f'Missing algorithm ConfigFlags: Trigger.Offline.Tau.{alg}')

        if hasattr(alg_flags, 'MaxHits') and alg_flags.MaxHits > max_hits:
            max_hits = alg_flags.MaxHits

    if max_hits > 0:
        hits_decoration = f'associatedHits_{name}'
        acc.addEventAlgo(CompFactory.FlavorTagDiscriminants.JetHitAssociationAlg(
            name=f'TrigTauJetHitAssociationAlg_{name}',
            jetContainer='HLT_TrigTauRecMerged_CaloMVAOnly',
            hitContainer='PixelClusters',
            hitAssociation=hits_decoration,
            dphiHitToJet=flags.Tracking.ActiveConfig.phiHalfWidth,
            detaHitToJet=flags.Tracking.ActiveConfig.etaHalfWidth,
            dzHitToVertex=180, # Upper bound, still ok if the RoI is smaller
            maxHits=max_hits,
            removeBadIDPixelHits=False, # TODO: Test difference if enabled
        ))
    else:
        hits_decoration = '' # Disabled


    # The TauJet reconstruction is handled by a set of tools, executed in the following order:
    tools = []          # Common tools

    #---------------------------------------------------------------
    # HitZ + CaloHits preselection inference
    #---------------------------------------------------------------
    # We can run multiple inferences at once. Each will be stored on different decorated variables

    from TriggerMenuMT.HLT.Tau.TauConfigurationTools import getTauIDScoreVariables
    hitz_monitoring = {}
    hitz_shift_to_detector = {}
    id_score_monitoring = {}

    from TriggerMenuMT.HLT.Tau.TauConfigurationTools import getHitZVariables
    for alg in hitz_algs + presel_algs:
        log.debug('Configuring TrigTauRecMerged with the ONNX Tau inference: %s', alg)

        ptau_sfx = None
        if alg in hitz_algs:
            hitz_monitoring[alg] = (z0_var, _) = getHitZVariables(alg)

            # HitZ z0 regressions are w.r.t. beamspot; will shift it in TrigTauRec
            alg_flags = getattr(flags.Trigger.Offline.Tau, alg)
            if hasattr(alg_flags, 'BeamSpotCoordinates') and alg_flags.BeamSpotCoordinates:
                ptau_sfx = 'wrt_beamspot'
                hitz_shift_to_detector[f'{z0_var}_{ptau_sfx}'] = z0_var


        # ONNX inference
        tools.append(acc.popToolsAndMerge(trigTauJetONNXEvaluatorCfg(
            flags, 
            tau_id=alg, 
            tau_container=output_taus,
            hits_decoration_container=hits_decoration,
            ptau_sfx=ptau_sfx,
        )))
        acc.addPublicTool(tools[-1])

        if alg in presel_algs:
            # ID score flattening and WPs
            tools.append(acc.popToolsAndMerge(trigTauWPDecoratorCfg(flags, tau_id=alg, precision_seq_name=name)))
            acc.addPublicTool(tools[-1])

            id_score_monitoring[alg] = getTauIDScoreVariables(alg)


    # Set trigger-specific configuration for all the reconstruction tools
    for tool in tools:
        tool.inTrigger = True
        tool.calibFolder = flags.Trigger.Offline.Tau.tauRecToolsCVMFSPath


    from TrigTauRec.TrigTauRecMonitoring import tauMonitoringCaloHits
    acc.addEventAlgo(CompFactory.TrigTauRecMerged(
        name=f'TrigTauRecMerged_CaloHits_{name}',
        CommonTools=tools,
        MonTool=tauMonitoringCaloHits(
            flags, 
            f'CaloHits_{name}', 
            hitz_algs=hitz_monitoring.keys(), 
            tau_ids=id_score_monitoring.keys(), 
        ),
        MonitoredHitZRegressions=hitz_monitoring,
        MonitoredIDScores=id_score_monitoring,
        ShiftToDetectorCoordinates=hitz_shift_to_detector,
        InputRoIs=input_rois,
        InputTauJetContainer='HLT_TrigTauRecMerged_CaloMVAOnly',
        InputTauJetHitsKey=hits_decoration,
        OutputTauTrackContainer=output_tau_tracks,
        OutputTauJetContainer=output_taus,
    ))

    return acc



def trigTauRecMergedCaloMVACfg(flags: AthConfigFlags) -> ComponentAccumulator:
    '''
    Reconstruct the CaloMVA TauJet from the calo-clusters.

    :param flags: Config flags.
    :return: CA with the TauJet CaloMVA reconstruction sequence.
    '''
    # Main CA
    acc = ComponentAccumulator()

    tools = []

    from AthenaConfiguration.ComponentFactory import CompFactory

    # Set seedcalo energy scale (Full RoI)
    tools.append(CompFactory.JetSeedBuilder())

    # Set LC energy scale (0.2 cone) and intermediate axis (corrected for vertex: useless at trigger)
    tools.append(CompFactory.TauAxisSetter(ClusterCone=0.2, VertexCorrection=False))

    # Decorate the clusters
    tools.append(CompFactory.TauClusterFinder(UseOriginalCluster=False)) # TODO: use JetRec.doVertexCorrection once available
    tools.append(CompFactory.TauVertexedClusterDecorator(SeedJet=''))

    # Calculate cell-based quantities: strip variables, EM and Had energies/radii, centFrac, isolFrac and ring energies
    tools.append(CompFactory.TauCellVariables(CellCone=0.2, VertexCorrection = False))

    # Compute MVA TES (ATR-17649), stores MVA TES as the default tau pt
    tools.append(CompFactory.MvaTESVariableDecorator(Key_vertexInputContainer='', EventShapeKey='', VertexCorrection=False))
    acc.addPublicTool(tools[-1])
    tools.append(CompFactory.MvaTESEvaluator(WeightFileName=flags.Trigger.Offline.Tau.MvaTESConfig))
    acc.addPublicTool(tools[-1])


    # Set trigger-specific configuration for all the reconstruction tools
    for tool in tools:
        tool.inTrigger = True
        tool.calibFolder = flags.Trigger.Offline.Tau.tauRecToolsCVMFSPath


    from TrigEDMConfig.TriggerEDM import recordable
    from TrigTauRec.TrigTauRecMonitoring import tauMonitoringCaloOnlyMVA
    acc.addEventAlgo(CompFactory.TrigTauRecMerged(
        name='TrigTauRecMerged_TauCaloOnlyMVA',
        CommonTools=tools,
        MonTool=tauMonitoringCaloOnlyMVA(flags),
        InputRoIs='UpdatedCaloRoI',
        InputCaloClusterContainer='HLT_TopoCaloClustersLC',
        OutputTauTrackContainer='HLT_tautrack_dummy',
        OutputTauJetContainer='HLT_TrigTauRecMerged_CaloMVAOnly',
        OutputJetSeed=recordable('HLT_jet_seed'),
    ))

    return acc



if __name__ == '__main__':
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN2
    flags.lock()

    acc = trigTauRecMergedCaloMVACfg(flags)
    acc.printConfig(withDetails=True, summariseProps=True)
    acc.wasMerged() # Do not run, do not save, we just want to see the config
