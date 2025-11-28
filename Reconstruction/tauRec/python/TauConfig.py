# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType, LHCPeriod


def TauBuildAlgCfg(flags):

    result = ComponentAccumulator()

    # Schedule total noise cond alg
    from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg
    result.merge(CaloNoiseCondAlgCfg(flags, "totalNoise"))
    # Schedule electronic noise cond alg (needed for LC weights)
    result.merge(CaloNoiseCondAlgCfg(flags, "electronicNoise"))

    # get tools from holder
    import tauRec.TauToolHolder as tauTools

    tools = []
    tools.append( result.popToolsAndMerge(tauTools.JetSeedBuilderCfg(flags)) )

    # for electron-removed taus, check that seed jets are close to an electron
    if getattr(flags.Tau.ActiveConfig, 'inTauEleRM', False):
        tools.append( result.popToolsAndMerge(tauTools.TauEleOverlapChecker(flags)) )

    # run vertex finder only in case vertexing is available
    if flags.Tau.isStandalone or flags.Tracking.doVertexFinding:
        tools.append( result.popToolsAndMerge(tauTools.TauVertexFinderCfg(flags)) )

    tools.append( result.popToolsAndMerge(tauTools.TauAxisCfg(flags)) )

    # track classification + association 
    tools.append( result.popToolsAndMerge(tauTools.TauTrackFinderCfg(flags)) )
    if flags.Beam.Type is not BeamType.Cosmics and flags.Tau.doRNNTrackClass:
        tools.append( result.popToolsAndMerge(tauTools.TauTrackRNNClassifierCfg(flags)) )

    # cluster association + vertex correction
    tools.append( result.popToolsAndMerge(tauTools.TauClusterFinderCfg(flags)) )
    tools.append( result.popToolsAndMerge(tauTools.TauVertexedClusterDecoratorCfg(flags)) )

    # this needs to go before the TauCaloAlgCfg
    if flags.Tau.doPi0Clus:
        tools.append( result.popToolsAndMerge(tauTools.Pi0ClusterFinderCfg(flags)) )

    # TauBuildAlg AKA TauProcessorAlg
    TauProcessorAlg = CompFactory.getComp("TauProcessorAlg")
    BuildAlg = TauProcessorAlg(name                           = flags.Tau.ActiveConfig.prefix+"TauCoreBuilderAlg",
                               Key_jetInputContainer          = flags.Tau.ActiveConfig.SeedJetCollection,
                               Key_tauOutputContainer         = flags.Tau.ActiveConfig.TauJets_tmp,
                               Key_tauTrackOutputContainer    = flags.Tau.ActiveConfig.TauTracks,
                               Key_tauPi0CellOutputContainer  = flags.Tau.ActiveConfig.TauCommonPi0Cells,
                               MaxEta                         = flags.Tau.SeedMaxEta,
                               MinPt                          = flags.Tau.SeedMinPt,
                               MaxNTracks                     = flags.Tau.MaxNTracks,
                               Tools                          = tools,
                               CellMakerTool                  = result.popToolsAndMerge(tauTools.TauCellFinalizerCfg(flags)))

    if flags.GeoModel.Run is LHCPeriod.Run4:
        BuildAlg.PixelDetEleCollKey="ITkPixelDetectorElementCollection"
        BuildAlg.SCTDetEleCollKey="ITkStripDetectorElementCollection"
        BuildAlg.TRTDetEleContKey=""

    result.addEventAlgo(BuildAlg)
    return result


def TauCaloAlgCfg(flags):

    result = ComponentAccumulator()

    # Schedule total noise cond alg
    from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg
    result.merge(CaloNoiseCondAlgCfg(flags,"totalNoise"))
    # Schedule electronic noise cond alg (needed for LC weights)
    result.merge(CaloNoiseCondAlgCfg(flags,"electronicNoise"))

    from CaloRec.CaloTopoClusterConfig import caloTopoCoolFolderCfg
    result.merge(caloTopoCoolFolderCfg(flags))

    from LArBadChannelTool.LArBadChannelConfig import LArBadChannelCfg
    result.merge(LArBadChannelCfg(flags))

    from TileConditions.TileBadChannelsConfig import TileBadChannelsCondAlgCfg
    result.merge( TileBadChannelsCondAlgCfg(flags) )

    # get tools from holder
    import tauRec.TauToolHolder as tauTools

    CaloClusterMaker = CompFactory.getComp("CaloClusterMaker")
    CaloTopoForTausMaker = CaloClusterMaker (flags.Tau.ActiveConfig.prefix+"TauPi0SubtractedClusterMaker")
    CaloTopoForTausMaker.ClustersOutputName = flags.Tau.ActiveConfig.TauPi0Clusters_tmp
    CaloTopoForTausMaker.ClusterMakerTools = [result.popToolsAndMerge(tauTools.TauCaloTopoClusterMakerCfg(flags)),
                                              result.popToolsAndMerge(tauTools.TauCaloTopoClusterSplitterCfg(flags))]

    CaloTopoForTausMaker.ClusterCorrectionTools += [result.popToolsAndMerge(tauTools.TauCaloClusterBadChannelCfg(flags))]
    CaloTopoForTausMaker.ClusterCorrectionTools += [result.popToolsAndMerge(tauTools.TauCaloClusterMomentsMakerCfg(flags))]

    if flags.Calo.TopoCluster.doTopoClusterLocalCalib:
        CaloTopoForTausMaker.ClusterCorrectionTools += [result.popToolsAndMerge(tauTools.TauCaloClusterLocalCalibCfg(flags)),
                                                        result.popToolsAndMerge(tauTools.TauCaloOOCCalibCfg(flags)),
                                                        result.popToolsAndMerge(tauTools.TauCaloOOCPi0CalibCfg(flags)),
                                                        result.popToolsAndMerge(tauTools.TauCaloDMCalibCfg(flags))]

    result.addEventAlgo(CaloTopoForTausMaker)

    relinkAlg = CompFactory.ClusterCellRelinkAlg(name            = flags.Tau.ActiveConfig.prefix+'ClusterCellRelinkAlg',
                                                 Cells           = 'AllCalo',
                                                 ClustersInput   = flags.Tau.ActiveConfig.TauPi0Clusters_tmp,
                                                 ClustersOutput  = flags.Tau.ActiveConfig.TauPi0Clusters,
                                                 CellLinksOutput = flags.Tau.ActiveConfig.TauPi0ClustersLinks)
    result.addEventAlgo(relinkAlg)
    return result


def TauRunnerAlgCfg(flags):

    result=ComponentAccumulator()

    # get tools from holder
    import tauRec.TauToolHolder as tauTools

    tools = []
    tools.append( result.popToolsAndMerge(tauTools.TauShotFinderCfg(flags)) )
    tools.append( result.popToolsAndMerge(tauTools.Pi0ClusterCreatorCfg(flags)) )
    tools.append( result.popToolsAndMerge(tauTools.Pi0ClusterScalerCfg(flags)) )
    tools.append( result.popToolsAndMerge(tauTools.Pi0ScoreCalculatorCfg(flags)) )
    tools.append( result.popToolsAndMerge(tauTools.Pi0SelectorCfg(flags)) )

    if flags.Beam.Type is not BeamType.Cosmics:
        tools.append( result.popToolsAndMerge(tauTools.EnergyCalibrationLCCfg(flags)) )

    if flags.Tau.doPanTau:
        import PanTauAlgs.JobOptions_Main_PanTau as pantau
        tools.append( result.popToolsAndMerge(pantau.PanTauCfg(flags)) )

    tools.append(result.popToolsAndMerge(tauTools.TauCombinedTESCfg(flags)) )

    # this is scheduled here because it provides variables used in the MVATES evaluation
    tools.append( result.popToolsAndMerge(tauTools.CellVariablesCfg(flags)) )
    # these tools need pantau info
    if flags.Beam.Type is not BeamType.Cosmics:
        tools.append( result.popToolsAndMerge(tauTools.MvaTESVariableDecoratorCfg(flags)) )
        tools.append( result.popToolsAndMerge(tauTools.MvaTESEvaluatorCfg(flags)) )

    # apply pt cut
    tools.append( result.popToolsAndMerge(tauTools.TauAODSelectorCfg(flags)) )

    # do some extra variable calculation
    if flags.Tau.isStandalone or flags.Tracking.doVertexFinding:
        tools.append(result.popToolsAndMerge(tauTools.TauVertexVariablesCfg(flags)) )
    tools.append( result.popToolsAndMerge(tauTools.ElectronVetoVarsCfg(flags)) )
    tools.append( result.popToolsAndMerge(tauTools.TauCommonCalcVarsCfg(flags)) )
    tools.append( result.popToolsAndMerge(tauTools.TauSubstructureCfg(flags)) )
  
    if flags.Tau.doTauDiscriminant:
        tools.append( result.popToolsAndMerge(tauTools.TauIDVarCalculatorCfg(flags)) )
        # do not schedule RNNID and eVeto for Run4
        if flags.GeoModel.Run <= LHCPeriod.Run3:
            tools.append( result.popToolsAndMerge(tauTools.TauJetRNNEvaluatorCfg(flags)) )
            tools.append( result.popToolsAndMerge(tauTools.TauWPDecoratorJetRNNCfg(flags)) )
            tools.append( result.popToolsAndMerge(tauTools.TauEleRNNEvaluatorCfg(flags)) )
            tools.append( result.popToolsAndMerge(tauTools.TauWPDecoratorEleRNNCfg(flags)) )
        tools.append( result.popToolsAndMerge(tauTools.TauDecayModeNNClassifierCfg(flags)) )
        # added for offline tau trigger monitoring at T0, not needed for TauJets_EleRM
        if not flags.Tau.ActiveConfig.inTauEleRM:
            # only compute GNTau for 1p/3p, as this is internally required by the tau trigger monitoring
            tools.append( result.popToolsAndMerge(tauTools.TauGNNEvaluatorCfg(flags, version=0, applyTightTrackSel=True)) )
            tools.append( result.popToolsAndMerge(tauTools.TauWPDecoratorGNNCfg(flags, version=0, tauContainerName=flags.Tau.ActiveConfig.TauJets)) )

    TauRunnerAlg = CompFactory.getComp("TauRunnerAlg")
    RunnerAlg = TauRunnerAlg(name                           = flags.Tau.ActiveConfig.prefix+"TauRecRunnerAlg",
                             Key_tauInputContainer          = flags.Tau.ActiveConfig.TauJets_tmp,
                             Key_Pi0ClusterInputContainer   = flags.Tau.ActiveConfig.TauPi0Clusters,
                             Key_tauOutputContainer         = flags.Tau.ActiveConfig.TauJets,
                             Key_neutralPFOOutputContainer  = flags.Tau.ActiveConfig.TauNeutralPFOs,
                             Key_hadronicPFOOutputContainer = flags.Tau.ActiveConfig.TauHadronicPFOs,
                             Key_chargedPFOOutputContainer  = flags.Tau.ActiveConfig.TauChargedPFOs,
                             Key_vertexOutputContainer      = flags.Tau.ActiveConfig.TauSecondaryVertices,
                             Key_pi0Container               = flags.Tau.ActiveConfig.TauFinalPi0s,
                             Key_tauShotClusOutputContainer = flags.Tau.ActiveConfig.TauShotClusters,
                             Key_tauShotClusLinkContainer   = flags.Tau.ActiveConfig.TauShotClustersLinks,
                             Key_tauShotPFOOutputContainer  = flags.Tau.ActiveConfig.TauShotPFOs,
                             Tools                          = tools)

    result.addEventAlgo(RunnerAlg)
    return result


def TauOutputCfg(flags):

    from OutputStreamAthenaPool.OutputStreamConfig import addToESD,addToAOD
    result=ComponentAccumulator()

    # common to AOD and ESD
    TauAODList = []
    TauAODList += [ f"xAOD::TauJetContainer#{flags.Tau.ActiveConfig.TauJets}" ]
    TauAODList += [ f"xAOD::TauTrackContainer#{flags.Tau.ActiveConfig.TauTracks}" ]
    TauAODList += [ f"xAOD::TauTrackAuxContainer#{flags.Tau.ActiveConfig.TauTracks}Aux." ]
    TauAODList += [ f"xAOD::VertexContainer#{flags.Tau.ActiveConfig.TauSecondaryVertices}" ]
    TauAODList += [ f"xAOD::VertexAuxContainer#{flags.Tau.ActiveConfig.TauSecondaryVertices}Aux.-vxTrackAtVertex" ]
    TauAODList += [ f"xAOD::CaloClusterContainer#{flags.Tau.ActiveConfig.TauPi0Clusters}" ]
    TauAODList += [ f"xAOD::CaloClusterAuxContainer#{flags.Tau.ActiveConfig.TauPi0Clusters}Aux." ]
    TauAODList += [ f"CaloClusterCellLinkContainer#{flags.Tau.ActiveConfig.TauPi0Clusters}_links" ]
    TauAODList += [ f"xAOD::CaloClusterContainer#{flags.Tau.ActiveConfig.TauShotClusters}" ]
    TauAODList += [ f"xAOD::CaloClusterAuxContainer#{flags.Tau.ActiveConfig.TauShotClusters}Aux." ]
    TauAODList += [ f"CaloClusterCellLinkContainer#{flags.Tau.ActiveConfig.TauShotClusters}_links" ]
    TauAODList += [ f"xAOD::ParticleContainer#{flags.Tau.ActiveConfig.TauFinalPi0s}" ]
    TauAODList += [ f"xAOD::ParticleAuxContainer#{flags.Tau.ActiveConfig.TauFinalPi0s}Aux." ]
    TauAODList += [ f"xAOD::PFOContainer#{flags.Tau.ActiveConfig.TauShotPFOs}" ]
    TauAODList += [ f"xAOD::PFOAuxContainer#{flags.Tau.ActiveConfig.TauShotPFOs}Aux." ]
    TauAODList += [ f"xAOD::PFOContainer#{flags.Tau.ActiveConfig.TauNeutralPFOs}" ]
    TauAODList += [ f"xAOD::PFOAuxContainer#{flags.Tau.ActiveConfig.TauNeutralPFOs}Aux." ]
    TauAODList += [ f"xAOD::PFOContainer#{flags.Tau.ActiveConfig.TauHadronicPFOs}" ]
    TauAODList += [ f"xAOD::PFOAuxContainer#{flags.Tau.ActiveConfig.TauHadronicPFOs}Aux." ]

    # Set common to ESD too
    TauESDList = list(TauAODList)

    # AOD specific
    # remove GlobalFELinks - these are links between FlowElement (FE) containers created in jet finding and taus. Since these transient FE containers are not in the AOD, we should not write out these links.
    removeAODvars = "-VertexedClusters.-shotCells.-mu.-nVtxPU.-ABS_ETA_LEAD_TRACK.-TAU_ABSDELTAPHI.-TAU_ABSDELTAETA.-absipSigLeadTrk.-passThinning.-chargedGlobalFELinks.-neutralGlobalFELinks"
    if not flags.Tau.ActiveConfig.inTauEleRM:
        removeAODvars += f".-{flags.Tau.GNTauScoreName[0]}.-{flags.Tau.GNTauTransScoreName[0]}.-{flags.Tau.GNTauDecorWPNames[0][0]}.-{flags.Tau.GNTauDecorWPNames[0][1]}.-{flags.Tau.GNTauDecorWPNames[0][2]}.-{flags.Tau.GNTauDecorWPNames[0][3]}.-GNTauProbTau.-GNTauProbJet"
    TauAODList += [ "xAOD::TauJetAuxContainer#{}Aux.{}".format(flags.Tau.ActiveConfig.TauJets, removeAODvars) ]

    # ESD specific
    removeESDvars = "-VertexedClusters.-shotCells.-chargedGlobalFELinks.-neutralGlobalFELinks"
    if not flags.Tau.ActiveConfig.inTauEleRM:
        removeESDvars += f".-{flags.Tau.GNTauScoreName[0]}.-{flags.Tau.GNTauTransScoreName[0]}.-{flags.Tau.GNTauDecorWPNames[0][0]}.-{flags.Tau.GNTauDecorWPNames[0][1]}.-{flags.Tau.GNTauDecorWPNames[0][2]}.-{flags.Tau.GNTauDecorWPNames[0][3]}.-GNTauProbTau.-GNTauProbJet"
    TauESDList += [ "xAOD::TauJetAuxContainer#{}Aux.{}".format(flags.Tau.ActiveConfig.TauJets, removeESDvars) ]
    TauESDList += [ "xAOD::PFOContainer#{}"        .format(flags.Tau.ActiveConfig.TauChargedPFOs) ]
    TauESDList += [ "xAOD::PFOAuxContainer#{}Aux." .format(flags.Tau.ActiveConfig.TauChargedPFOs) ]

    result.merge(addToESD(flags,TauESDList))
    result.merge(addToAOD(flags,TauAODList))
    return result


def DiTauOutputCfg(flags):

   from OutputStreamAthenaPool.OutputStreamConfig import addToESD,addToAOD
   result=ComponentAccumulator()

   DiTauOutputList  = [ "xAOD::DiTauJetContainer#DiTauJets" ]
   DiTauOutputList += [ "xAOD::DiTauJetAuxContainer#DiTauJetsAux." ]

   result.merge(addToESD(flags,DiTauOutputList))
   result.merge(addToAOD(flags,DiTauOutputList))
   return result


def TauxAODthinngCfg(flags):

    result = ComponentAccumulator()

    tauThinAlg = CompFactory.TauThinningAlg(name                 = flags.Tau.ActiveConfig.prefix+"TauThinningAlg",
                                            Taus                 = flags.Tau.ActiveConfig.TauJets,
                                            TauTracks            = flags.Tau.ActiveConfig.TauTracks,
                                            TauNeutralPFOs       = flags.Tau.ActiveConfig.TauNeutralPFOs,
                                            TauPi0Clusters       = flags.Tau.ActiveConfig.TauPi0Clusters,
                                            TauPi0CellLinks      = flags.Tau.ActiveConfig.TauPi0ClustersLinks,
                                            TauFinalPi0s         = flags.Tau.ActiveConfig.TauFinalPi0s,
                                            TauShotPFOs          = flags.Tau.ActiveConfig.TauShotPFOs,
                                            TauShotClusters      = flags.Tau.ActiveConfig.TauShotClusters,
                                            TauShotCellLinks     = flags.Tau.ActiveConfig.TauShotClustersLinks,
                                            TauHadronicPFOs      = flags.Tau.ActiveConfig.TauHadronicPFOs,
                                            TauSecondaryVertices = flags.Tau.ActiveConfig.TauSecondaryVertices)
    result.addEventAlgo(tauThinAlg)
    return result


def TauReconstructionCfg(flags):

    result = ComponentAccumulator()

    # Schedule the custom jets needed for tau seeding
    from JetRecConfig.JetRecConfig import JetRecCfg
    from JetRecConfig.StandardJetConstits import stdConstitDic as cst
    from JetRecConfig.JetDefinition import  JetDefinition
    from JetRecConfig.StandardSmallRJets import flavourghosts, calibmods_noCut, standardmods, truthmods
    minimalghosts = ["Track","MuonSegment","Truth"]

    #Check if the specific jet collection is needed based on flags
    if flags.Tau.TauRec.SeedJetCollection == "AntiKt4EMPFlowMLJets":
        from JetRecConfig.StandardSmallRJets import AntiKt4EMPFlowML
        result.merge(JetRecCfg(flags, AntiKt4EMPFlowML))
    if flags.Tau.TauRec.SeedJetCollection == "AntiKt4EMPFlow10GeVCutTauSeedJets":
        AntiKt4EMPFlow10GeVCutTauSeed = JetDefinition("AntiKt",0.4,cst.GPFlow,
                                      infix = "10GeVCutTauSeed",
                                      ghostdefs = minimalghosts+flavourghosts,
                                      modifiers = calibmods_noCut+("Filter:1",)+truthmods+standardmods+("JetPtAssociation","CaloEnergiesClus"),
                                      ptmin = 10000.,
                                      lock = True)
        result.merge(JetRecCfg(flags, AntiKt4EMPFlow10GeVCutTauSeed))
    if flags.Tau.TauRec.SeedJetCollection == "AntiKt4EMPFlow5GeVCutTauSeedJets":
        AntiKt4EMPFlow5GeVCutTauSeed = JetDefinition("AntiKt",0.4,cst.GPFlow,
                                      infix = "5GeVCutTauSeed",
                                      ghostdefs = minimalghosts+flavourghosts,
                                      modifiers = calibmods_noCut+("Filter:1",)+truthmods+standardmods+("JetPtAssociation","CaloEnergiesClus"),
                                      ptmin = 5000.,
                                      lock = True)
        result.merge(JetRecCfg(flags, AntiKt4EMPFlow5GeVCutTauSeed))
    if flags.Tau.TauRec.SeedJetCollection == "AntiKt4EMPFlowNoPtCutTauSeedJets":
        AntiKt4EMPFlowNoPtCutTauSeed = JetDefinition("AntiKt",0.4,cst.GPFlow,
                                      infix = "NoPtCutTauSeed",
                                      ghostdefs = minimalghosts+flavourghosts,
                                      modifiers = calibmods_noCut+("Filter:1",)+truthmods+standardmods+("JetPtAssociation","CaloEnergiesClus"),
                                      ptmin = 1,
                                      lock = True)
        result.merge(JetRecCfg(flags, AntiKt4EMPFlowNoPtCutTauSeed))
    # --- > End of Schedule the custom jets needed for tau seeding < ---


    # standard tau reconstruction
    flags_TauRec = flags.cloneAndReplace("Tau.ActiveConfig", "Tau.TauRec")

    result.merge(TauBuildAlgCfg(flags_TauRec))

    result.merge(TauCaloAlgCfg(flags_TauRec))

    result.merge(TauRunnerAlgCfg(flags_TauRec))

    if (flags.Output.doWriteESD or flags.Output.doWriteAOD):
        result.merge(TauOutputCfg(flags_TauRec))

    if (flags.Output.doWriteAOD and flags.Tau.ThinTaus):
        result.merge(TauxAODthinngCfg(flags_TauRec))

    # electron-subtracted tau reconstruction
    if flags.Tau.doTauEleRMRec:

        flags_TauEleRM = flags.cloneAndReplace("Tau.ActiveConfig", "Tau.TauEleRM")

        result.merge(TauElecSubtractAlgCfg(flags_TauEleRM))

        # jet reclustering
        from JetRecConfig.JetRecConfig import JetRecCfg
        if 'PFlow' in flags.Tau.TauRec.SeedJetCollection:
           from JetRecConfig.JetRecConfig import JetRecCfg
           from JetRecConfig.StandardSmallRJets import AntiKt4EMPFlow_tauSeedEleRM 
           result.merge( JetRecCfg(flags_TauEleRM,AntiKt4EMPFlow_tauSeedEleRM ))  
        else:
           from JetRecConfig.StandardSmallRJets import AntiKt4LCTopo
           AntiKt4LCTopo_EleRM = AntiKt4LCTopo.clone(suffix="_EleRM")
           AntiKt4LCTopo_EleRM.inputdef.name = flags_TauEleRM.Tau.ActiveConfig.LCTopoOrigin_EleRM
           AntiKt4LCTopo_EleRM.inputdef.inputname = flags_TauEleRM.Tau.ActiveConfig.CaloCalTopoClusters_EleRM
           AntiKt4LCTopo_EleRM.inputdef.containername = flags_TauEleRM.Tau.ActiveConfig.LCOriginTopoClusters_EleRM
           AntiKt4LCTopo_EleRM.standardRecoMode = True
           AntiKt4LCTopo_EleRM.context = "EleRM"
           result.merge(JetRecCfg(flags_TauEleRM, AntiKt4LCTopo_EleRM))

        result.merge(TauBuildAlgCfg(flags_TauEleRM))

        result.merge(TauCaloAlgCfg(flags_TauEleRM))

        result.merge(TauRunnerAlgCfg(flags_TauEleRM))

        if (flags.Output.doWriteESD or flags.Output.doWriteAOD):
            result.merge(TauOutputCfg(flags_TauEleRM))

        if (flags.Output.doWriteAOD and flags.Tau.ThinTaus):
            result.merge(TauxAODthinngCfg(flags_TauEleRM))

    # had-had boosted ditaus
    if flags.DiTau.doDiTauRec:
        from DiTauRec.DiTauBuilderConfig import DiTauBuilderCfg
        result.merge(DiTauBuilderCfg(flags))

        if (flags.Output.doWriteESD or flags.Output.doWriteAOD):
            result.merge(DiTauOutputCfg(flags))

    return result


def TauElecSubtractAlgCfg(flags):

    result = ComponentAccumulator()

    from ElectronPhotonSelectorTools.AsgElectronLikelihoodToolsConfig import AsgElectronLikelihoodToolCfg
    from ElectronPhotonSelectorTools.LikelihoodEnums import LikeEnum
    from ElectronPhotonSelectorTools.ElectronLikelihoodToolMapping import electronLHmenu
    ElectronLHSelectorEleRM = result.popToolsAndMerge(
        AsgElectronLikelihoodToolCfg(
            flags,
            name    = flags.Tau.ActiveConfig.prefix+"ElectronLHSelector",
            quality = getattr(LikeEnum, flags.Tau.ActiveConfig.EleRM_ElectronWorkingPoint),
            menu    = electronLHmenu.offlineMC21,
        )
    )

    tauElecSubtractAlg = CompFactory.TauElecSubtractAlg(
        name                        = flags.Tau.ActiveConfig.prefix+"TauElecSubtractAlg",
        Key_ElectronsInput          = 'Electrons',
        Key_ClustersInput           = 'CaloCalTopoClusters',
        Key_ClustersOutput          = flags.Tau.ActiveConfig.CaloCalTopoClusters_EleRM,
        Key_IDTracksInput           = 'InDetTrackParticles',
        Key_IDTracksOutput          = flags.Tau.ActiveConfig.TrackCollection,
        Key_RemovedClustersOutput   = flags.Tau.ActiveConfig.RemovedElectronClusters,
        Key_RemovedTracksOutput     = flags.Tau.ActiveConfig.RemovedElectronTracks,
        ElectronLHTool              = ElectronLHSelectorEleRM,
        doNothing                   = False,
    )
    result.addEventAlgo(tauElecSubtractAlg)
    return result

# Run with python -m tauRec.TauConfig
def TauConfigTest(flags=None):

    if flags is None:
        from AthenaConfiguration.AllConfigFlags import initConfigFlags
        from AthenaConfiguration.TestDefaults import defaultTestFiles, defaultConditionsTags

        flags = initConfigFlags()

        flags.Input.Files = defaultTestFiles.RDO_RUN3
        flags.IOVDb.GlobalTag = defaultConditionsTags.RUN3_MC

        flags.Output.AODFileName = "AOD.pool.root"
        flags.Exec.MaxEvents = 50
        
        flags.Scheduler.ShowDataDeps = True
        flags.Scheduler.ShowDataFlow = True
        flags.Scheduler.ShowControlFlow = True

        flags.Concurrency.NumThreads = 1
        flags.Concurrency.NumConcurrentEvents = 1

        from tauRec.ConfigurationHelpers import StandaloneTauRecoFlags
        StandaloneTauRecoFlags(flags)

        flags.lock()
   
    from RecJobTransforms.RecoSteering import RecoSteering
    cfg = RecoSteering(flags)

    from RecJobTransforms.RecoConfigFlags import printRecoFlags
    printRecoFlags(flags)

    cfg.run()


if __name__=="__main__":
    TauConfigTest()
