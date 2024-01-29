# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

def trigTauRecMergedPrecisionMVACfg(flags, name='', inputRoIs='', tracks=''):

    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaConfiguration.ComponentFactory import CompFactory
    from TrigEDMConfig.TriggerEDM import recordable
    acc = ComponentAccumulator()

    doLLP = False
    postfix = ''

    if "MVA" in name:
      postfix = "MVA"
      trigTauJetOutputContainer   = recordable("HLT_TrigTauRecMerged_MVA")
      trigTauTrackOutputContainer = recordable("HLT_tautrack_MVA")
    elif "LLP" in name:
      postfix = "LLP"
      trigTauJetOutputContainer   = recordable("HLT_TrigTauRecMerged_LLP")
      trigTauTrackOutputContainer = recordable("HLT_tautrack_LLP")
      doLLP = True
    elif "LRT" in name:
      postfix = "LRT"
      trigTauJetOutputContainer   = recordable("HLT_TrigTauRecMerged_LRT")
      trigTauTrackOutputContainer = recordable("HLT_tautrack_LRT")
      doLLP = True
    else:
      raise Exception( "algTauPrecision : called with incorrect non existent name: "+name )
      return None

    # prepare tools 
    tools = []
    vftools = []
    tftools = []
    vvtools = []
    idtools = []


    from TrigTauRec.TrigTauToolsConfig import (trigTauVertexFinderCfg, trigTauTrackFinderCfg, tauVertexVariablesCfg, trigTauJetRNNEvaluatorCfg, trigTauWPDecoratorJetRNNCfg)

    # Associate RoI vertex or Beamspot to tau - don't use TJVA
    vftools.append(acc.popToolsAndMerge(trigTauVertexFinderCfg(flags,name='TrigTau_TauVertexFinder')))
    
    # Set LC energy scale (0.2 cone) and intermediate axis (corrected for vertex: useless at trigger)       
    tools.append(CompFactory.TauAxisSetter(name='TrigTau_TauAxis',VertexCorrection = False)) 

    # tightened to 0.75 mm for tracktwoMVA (until the track BDT can be used)
    tftools.append(acc.popToolsAndMerge(trigTauTrackFinderCfg(flags,name='TrigTauTightDZ_TauTrackFinder',TrackParticlesContainer=tracks)))

    # Decorate the clusters
    tools.append(CompFactory.TauClusterFinder(name='TrigTau_TauClusterFinder',UseOriginalCluster = False))
    tools.append(CompFactory.TauVertexedClusterDecorator(name='TrigTau_TauVertexedClusterDecorator',SeedJet = ''))

    # Calculate cell-based quantities: strip variables, EM and Had energies/radii, centFrac, isolFrac and ring energies
    tools.append(CompFactory.TauCellVariables(name='TrigTau_CellVariables',VertexCorrection = False))

    # Compute MVA TES (ATR-17649), stores MVA TES as default tau pt()                                                            
    tools.append(CompFactory.MvaTESVariableDecorator(name='TrigTau_MvaTESVariableDecorator',
                                                     Key_vertexInputContainer='',
                                                     EventShapeKey='',
                                                     VertexCorrection = False))
    acc.addPublicTool(tools[-1])
    tools.append(CompFactory.MvaTESEvaluator(name='TrigTau_MvaTESEvaluator', 
                                             WeightFileName = flags.Trigger.Offline.Tau.MvaTESConfig))
    acc.addPublicTool(tools[-1])

    vvtools.append(acc.popToolsAndMerge(tauVertexVariablesCfg(flags,name='TrigTau_TauVertexVariables')))

    # Variables combining tracking and calorimeter information
    idtools.append(CompFactory.TauCommonCalcVars(name='TrigTau_TauCommonCalcVars'))

    # Cluster-based sub-structure, with dRMax also
    idtools.append(CompFactory.TauSubstructureVariables(name='TrigTau_TauSubstructure',VertexCorrection = False))

    # RNN tau ID, either nominal or LLP
    idtools.append(acc.popToolsAndMerge(trigTauJetRNNEvaluatorCfg(flags,name="TrigTau_TauJetRNNEvaluator",LLP=doLLP)))
    acc.addPublicTool(idtools[-1])

    # flattened RNN score and WP
    idtools.append(acc.popToolsAndMerge(trigTauWPDecoratorJetRNNCfg(flags,name="TrigTau_TauWPDecoratorJetRNN",LLP=doLLP)))
    acc.addPublicTool(idtools[-1])

    for tool in (tools + vftools + tftools + vvtools + idtools):
        tool.inTrigger = True
        tool.calibFolder = flags.Trigger.Offline.Tau.tauRecToolsCVMFSPath

    from TrigTauRec.TrigTauRecMonitoring import tauMonitoringPrecisionMVA

    alg = CompFactory.TrigTauRecMerged("TrigTauRecMerged_TauPrecision_Precision"+postfix,
                                       ComTools = tools,
                                       VFTools  = vftools,
                                       TFTools  = tftools,
                                       VVTools  = vvtools,
                                       IDTools  = idtools, 
                                       MonTool =  tauMonitoringPrecisionMVA(flags),
                                       Key_trigTauTrackInputContainer  = "HLT_tautrack_dummy",
                                       Key_trigTauJetInputContainer    = "HLT_TrigTauRecMerged_CaloMVAOnly",
                                       Key_trigJetSeedOutputKey        = recordable("HLT_jet_seed"),
                                       RoIInputKey                     = inputRoIs,
                                       clustersKey                     = "",
                                       Key_vertexInputContainer        = flags.Tracking.ActiveConfig.vertex,
                                       Key_trackPartInputContainer     = tracks,
                                       Key_trigTauJetOutputContainer   = trigTauJetOutputContainer,
                                       Key_trigTauTrackOutputContainer = trigTauTrackOutputContainer
                                       )

    acc.addEventAlgo(alg)

    return acc


def trigTauRecMergedCaloOnlyMVACfg(flags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaConfiguration.ComponentFactory import CompFactory
    acc = ComponentAccumulator()
    # prepare tools 
    tools = []
    # Set seedcalo energy scale (Full RoI)
    tools.append(CompFactory.JetSeedBuilder())

    # Set LC energy scale (0.2 cone) and intermediate axis (corrected for vertex: useless at trigger)
    tools.append(CompFactory.TauAxisSetter(ClusterCone = 0.2,
                                           VertexCorrection = False))
    # Decorate the clusters
    tools.append(CompFactory.TauClusterFinder(UseOriginalCluster = False)) # TODO use JetRec.doVertexCorrection once available

    tools.append(CompFactory.TauVertexedClusterDecorator(SeedJet = ''))

    # Calculate cell-based quantities: strip variables, EM and Had energies/radii, centFrac, isolFrac and ring energies
    from AthenaCommon.SystemOfUnits import GeV
    tools.append(CompFactory.TauCellVariables(StripEthreshold = 0.2*GeV,
                                                CellCone = 0.2,
                                                VertexCorrection = False))
    # Compute MVA TES (ATR-17649), stores MVA TES as default tau pt()
    tools.append(CompFactory.MvaTESVariableDecorator(Key_vertexInputContainer='',
                                                     EventShapeKey='',
                                                     VertexCorrection = False))
    acc.addPublicTool(tools[-1])
    tools.append(CompFactory.MvaTESEvaluator(WeightFileName = flags.Trigger.Offline.Tau.MvaTESConfig))
    acc.addPublicTool(tools[-1])

    for tool in tools:
        tool.inTrigger = True
        tool.calibFolder = flags.Trigger.Offline.Tau.tauRecToolsCVMFSPath

    from TrigTauRec.TrigTauRecMonitoring import tauMonitoringCaloOnlyMVA

    alg = CompFactory.TrigTauRecMerged("TrigTauRecMerged_TauCaloOnlyMVA",
                                        ComTools = tools,
                                        MonTool = tauMonitoringCaloOnlyMVA(flags),
                                        Key_trackPartInputContainer = '',
                                        Key_trigJetSeedOutputKey = 'HLT_jet_seed',
                                        Key_trigTauJetInputContainer = '',
                                        Key_trigTauJetOutputContainer = 'HLT_TrigTauRecMerged_CaloMVAOnly',
                                        Key_trigTauTrackInputContainer = '',
                                        Key_trigTauTrackOutputContainer = 'HLT_tautrack_dummy',
                                        Key_vertexInputContainer = '',
                                        clustersKey = 'HLT_TopoCaloClustersLC',
                                        RoIInputKey = 'UpdatedCaloRoI')
    acc.addEventAlgo(alg)

    return acc


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN2
    flags.lock()

    acc = trigTauRecMergedCaloOnlyMVACfg(flags)
    acc.printConfig(withDetails=True, summariseProps=True)
    acc.wasMerged() # do not run, do not save, we just want to see the config
