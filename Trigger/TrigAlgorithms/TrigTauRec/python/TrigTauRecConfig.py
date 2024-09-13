# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

def trigTauRecMergedPrecisionMVACfg(flags, name='', inputRoIs='', tracks=''):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    from TrigEDMConfig.TriggerEDM import recordable
    trigTauJetOutputContainer   = recordable(f'HLT_TrigTauRecMerged_{name}')
    trigTauTrackOutputContainer = recordable(f'HLT_tautrack_{name}')

    # The TauJet reconstruction handled by a set of tools, executed in the following order:
    vftools = []        # Vertex Finder tools
    tools_beforetf = [] # Common tools, ran before the Track Finder tools
    tftools = []        # Track Finder tools
    tools = []          # Common tools
    vvtools = []        # Vertex Vars tools
    idtools = []        # ID tools


    from TrigTauRec.TrigTauToolsConfig import trigTauVertexFinderCfg, trigTauTrackFinderCfg, tauVertexVariablesCfg
    from AthenaConfiguration.ComponentFactory import CompFactory

    # Associate RoI vertex or Beamspot to the tau - don't use TJVA
    vftools.append(acc.popToolsAndMerge(trigTauVertexFinderCfg(flags, name='TrigTau_TauVertexFinder')))
    
    # Set LC energy scale (0.2 cone) and intermediate axis (corrected for vertex: useless at trigger)       
    tools_beforetf.append(CompFactory.TauAxisSetter(name='TrigTau_TauAxis', VertexCorrection=False)) 

    # Associate tracks to the tau
    tftools.append(acc.popToolsAndMerge(trigTauTrackFinderCfg(flags, name='TrigTauTightDZ_TauTrackFinder', TrackParticlesContainer=tracks)))

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
    vvtools.append(acc.popToolsAndMerge(tauVertexVariablesCfg(flags,name='TrigTau_TauVertexVariables')))

    # Variables combining tracking and calorimeter information
    idtools.append(CompFactory.TauCommonCalcVars(name='TrigTau_TauCommonCalcVars'))

    # Cluster-based sub-structure, with dRMax
    idtools.append(CompFactory.TauSubstructureVariables(name='TrigTau_TauSubstructure', VertexCorrection=False))

    # Tau ID and score flattenning
    if name in ['MVA', 'LLP', 'LRT']:
        # Use the LLP ID if we're running on a LLP or LRT chain
        use_LLP_ID = name in ['LLP', 'LRT']

        from TrigTauRec.TrigTauToolsConfig import trigTauJetRNNEvaluatorCfg, trigTauWPDecoratorJetRNNCfg

        # RNN/DeepSet ID
        idtools.append(acc.popToolsAndMerge(trigTauJetRNNEvaluatorCfg(flags, name='TrigTau_TauJetRNNEvaluator', LLP=use_LLP_ID)))
        acc.addPublicTool(idtools[-1])

        # Flattened ID score and WP
        idtools.append(acc.popToolsAndMerge(trigTauWPDecoratorJetRNNCfg(flags, name='TrigTau_TauWPDecoratorJetRNN', LLP=use_LLP_ID)))
        acc.addPublicTool(idtools[-1])
    else:
        raise ValueError(f'Invalid configuration: {name}')

    for tool in vftools + tools_beforetf + tftools + tools + vvtools + idtools:
        tool.inTrigger = True
        tool.calibFolder = flags.Trigger.Offline.Tau.tauRecToolsCVMFSPath


    from TrigTauRec.TrigTauRecMonitoring import tauMonitoringPrecisionMVA

    alg = CompFactory.TrigTauRecMerged(f'TrigTauRecMerged_TauPrecision_Precision{name}',
                                       VertexFinderTools   = vftools,
                                       CommonToolsBeforeTF = tools_beforetf,
                                       TrackFinderTools    = tftools,
                                       CommonTools         = tools,
                                       VertexVarsTools     = vvtools,
                                       IDTools             = idtools,
                                       MonTool             = tauMonitoringPrecisionMVA(flags),
                                       InputRoIs                   = inputRoIs,
                                       InputCaloClusterContainer   = '',
                                       InputVertexContainer        = flags.Tracking.ActiveConfig.vertex,
                                       InputTauTrackContainer      = 'HLT_tautrack_dummy',
                                       InputTauJetContainer        = 'HLT_TrigTauRecMerged_CaloMVAOnly',
                                       OutputTauTrackContainer     = trigTauTrackOutputContainer,
                                       OutputTauJetContainer       = trigTauJetOutputContainer,
                                       OutputJetSeed               = '',
                                       )

    acc.addEventAlgo(alg)

    return acc


def trigTauRecMergedCaloOnlyMVACfg(flags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

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
    from AthenaCommon.SystemOfUnits import GeV
    tools.append(CompFactory.TauCellVariables(StripEthreshold=0.2*GeV, CellCone=0.2, VertexCorrection = False))

    # Compute MVA TES (ATR-17649), stores MVA TES as the default tau pt
    tools.append(CompFactory.MvaTESVariableDecorator(Key_vertexInputContainer='', EventShapeKey='', VertexCorrection=False))
    acc.addPublicTool(tools[-1])
    tools.append(CompFactory.MvaTESEvaluator(WeightFileName=flags.Trigger.Offline.Tau.MvaTESConfig))
    acc.addPublicTool(tools[-1])

    for tool in tools:
        tool.inTrigger = True
        tool.calibFolder = flags.Trigger.Offline.Tau.tauRecToolsCVMFSPath


    from TrigEDMConfig.TriggerEDM import recordable
    from TrigTauRec.TrigTauRecMonitoring import tauMonitoringCaloOnlyMVA

    alg = CompFactory.TrigTauRecMerged("TrigTauRecMerged_TauCaloOnlyMVA",
                                       CommonTools = tools,
                                       MonTool     = tauMonitoringCaloOnlyMVA(flags),
                                       InputRoIs                   = 'UpdatedCaloRoI',
                                       InputCaloClusterContainer   = 'HLT_TopoCaloClustersLC',
                                       InputVertexContainer        = '',
                                       InputTauTrackContainer      = '',
                                       InputTauJetContainer        = '',
                                       OutputTauTrackContainer     = 'HLT_tautrack_dummy',
                                       OutputTauJetContainer       = 'HLT_TrigTauRecMerged_CaloMVAOnly',
                                       OutputJetSeed               = recordable('HLT_jet_seed'),
                                       )

    acc.addEventAlgo(alg)

    return acc



if __name__ == '__main__':
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN2
    flags.lock()

    acc = trigTauRecMergedCaloOnlyMVACfg(flags)
    acc.printConfig(withDetails=True, summariseProps=True)
    acc.wasMerged() # Do not run, do not save, we just want to see the config
