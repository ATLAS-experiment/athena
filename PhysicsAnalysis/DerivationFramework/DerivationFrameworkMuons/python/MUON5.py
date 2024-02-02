# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python
#====================================================================
# DAOD_MUON5.py
# This defines the component accumulator version of DAOD_MUON5 
# It requires the flag MUON5 in Derivation_tf.py   
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

# Main algorithm config
def MUON5KernelCfg(ConfigFlags, name='MUON5Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for MUON5"""
    acc = ComponentAccumulator()

    # --------------------
    # Common augmentations
    # --------------------
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(ConfigFlags, TriggerListsHelper = kwargs['TriggerListsHelper']))

    # ------------
    # Augmentation
    # ------------

    # Augmentation tools
    MUON5AugmentTools = []                                     
    muonThinFlags = []
    trkThinFlags = []
    
    ### isolation decorations
    from DerivationFrameworkMuons.TrackIsolationDecoratorConfig import TrackIsolationCfg
    acc.merge(TrackIsolationCfg(ConfigFlags,TrackCollection="InDetTrackParticles", TrackSelections = trkThinFlags))
    acc.merge(TrackIsolationCfg(ConfigFlags,TrackCollection="ExtrapolatedMuonTrackParticles"))
    ### Calo deposits 
    from DerivationFrameworkMuons.MuonsToolsConfig import MuonCaloDepositAlgCfg
    acc.merge(MuonCaloDepositAlgCfg(ConfigFlags,
                                    ContainerKey="Muons")) ### Decorate directly the muons
    acc.merge(MuonCaloDepositAlgCfg(ConfigFlags, 
                                    name = "IdTrkCaloDepsitDecorator",
                                    ContainerKey="InDetTrackParticles")) ### Decorate the ID tracks
    ### Flavour tagging impact parameter decorators
    from BTagging.BTagTrackAugmenterAlgConfig import BTagTrackAugmenterAlgCfg
    acc.merge(BTagTrackAugmenterAlgCfg(
        ConfigFlags,
        prefix="btagIp_",
        TrackCollection="InDetTrackParticles",
        PrimaryVertexCollectionName="PrimaryVertices"
    ))

    # --------
    # Skimming
    # --------
    
    # Apply skimming requirement: at least one electron, muon or tau
    eleRequirements = '(Electrons.pt > 5*GeV) && (abs(Electrons.eta) < 2.6) && (Electrons.Loose || Electrons.DFCommonElectronsLHLoose)'
    muRequirements  = '(Muons.muonType == 0) && (Muons.pt > 5*GeV) && (abs(Muons.eta) < 2.6)'
    tauRequirements = '(TauJets.pt > 15*GeV) && (abs(TauJets.charge)==1.0) && ((TauJets.nTracks == 1) || (TauJets.nTracks == 3)) && (abs(TauJets.eta) < 2.6)'

    eSelection   = '(count('+eleRequirements+') >= 1)'
    mSelection   = '(count('+muRequirements +') >= 1)'
    tauSelection = '(count('+tauRequirements+') >= 1)'
    
    lepSelection = eSelection+' || '+mSelection+' || '+tauSelection
    
    MUON5SkimmingTools = []
    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import xAODStringSkimmingToolCfg
    # if we are running on upgrade MC (no trigger), we disable the TDT inside the skimming tool. 
    # The TDT is not actually used as we skim only on offline objects. 
    SkimArgs={}
    if  not (ConfigFlags.Reco.EnableTrigger or ConfigFlags.Trigger.triggerConfig == 'INFILE'): 
        SkimArgs["TrigDecisionTool"] = ""
    MUON5SkimmingTool1 = acc.getPrimaryAndMerge(xAODStringSkimmingToolCfg(ConfigFlags,
                                                                          name       = "MUON5SkimmingTool1",
                                                                          expression = lepSelection,
                                                                          **SkimArgs))
    MUON5SkimmingTools.append(MUON5SkimmingTool1) 

    # --------
    # Thinning
    # --------
    
    MUON5ThinningTools = [] 
    
    # Track thinning: only keep tracks with |z0| at primary vertex < 10 mm
    from DerivationFrameworkInDet.InDetToolsConfig import TrackParticleThinningCfg
    MUON5TrackThinningTool = acc.getPrimaryAndMerge(TrackParticleThinningCfg(ConfigFlags,
                                                                        name                    = "MUON5TrackThinningTool",
                                                                        StreamName              = kwargs['StreamName'],
                                                                        SelectionString         = "abs(DFCommonInDetTrackZ0AtPV) < 10.0",
                                                                        InDetTrackParticlesKey  = "InDetTrackParticles")
                                                    )
    
    MUON5ThinningTools.append(MUON5TrackThinningTool)

    # Thinning tools
    from DerivationFrameworkCalo.DerivationFrameworkCaloConfig import CaloClusterThinningCfg
    from DerivationFrameworkCalo.CaloCellDFGetterConfig import thinCaloCellsForDFCfg
    from DerivationFrameworkMuons.MuonsToolsConfig import AnalysisMuonThinningAlgCfg
    
    # protect against missing forward tracks in Upgrade 
    fwdTracks = "InDetForwardTrackParticles"
    if fwdTracks not in ConfigFlags.Input.Collections:
        fwdTracks = "" 
    acc.merge(AnalysisMuonThinningAlgCfg(ConfigFlags,
                                         MuonPassFlags = muonThinFlags,
                                         TrkPassFlags = trkThinFlags,
                                         StreamName = kwargs['StreamName'], 
                                         IdTrkFwdThinning=fwdTracks))
    

    # keep topoclusters around muons
    MUON5ThinningTool1 = acc.getPrimaryAndMerge(CaloClusterThinningCfg(ConfigFlags,
                                                                       name                    = "MUON5ThinningTool1",
                                                                       StreamName              = kwargs['StreamName'],
                                                                       SGKey                   = "Muons",
                                                                       SelectionString         = "Muons.pt>4*GeV",
                                                                       TopoClCollectionSGKey   = "CaloCalTopoClusters",
                                                                       ConeSize                = 0.5))
    MUON5ThinningTools.append(MUON5ThinningTool1)
    
    # keep egammaclusters around electrons
    MUON5ThinningTool2 = acc.getPrimaryAndMerge(CaloClusterThinningCfg(ConfigFlags,
                                                                       name                    = "MUON5ThinningTool2",
                                                                       StreamName              = kwargs['StreamName'],
                                                                       SGKey                   = "Electrons",
                                                                       SelectionString         = "Electrons.pt>4*GeV",
                                                                       CaloClCollectionSGKey   = "egammaClusters",
                                                                       ConeSize                = 0.4))
    MUON5ThinningTools.append(MUON5ThinningTool2)
    
    ### cell thinning
    acc.merge(thinCaloCellsForDFCfg(ConfigFlags,
                                    inputClusterKeys = ["MuonClusterCollection"],
                                    streamName       = kwargs['StreamName'],
                                    outputCellKey    = "DFMUONCellContainer"))
  
    ### Truth thinning
    if ConfigFlags.Input.isMC:        
        from DerivationFrameworkMCTruth.TruthDerivationToolsConfig import MenuTruthThinningCfg
        MUON5TruthThinningTool = acc.getPrimaryAndMerge(MenuTruthThinningCfg(ConfigFlags,
                                                                             name                            = "MUON5TruthThinningTool",
                                                                             StreamName                      = kwargs['StreamName'],
                                                                             WritePartons                    = False,
                                                                             WriteHadrons                    = False,
                                                                             WriteCHadrons                   = True,
                                                                             WriteBHadrons                   = True,
                                                                             WriteGeant                      = False,
                                                                             WriteTauHad                     = True,
                                                                             PartonPtThresh                  = -1.0,
                                                                             WriteBSM                        = True,
                                                                             WriteBosons                     = True,
                                                                             WriteBosonProducts              = True,
                                                                             WriteBSMProducts                = True,
                                                                             WriteTopAndDecays               = True,
                                                                             WriteEverything                 = False,
                                                                             WriteAllLeptons                 = True,
                                                                             WriteLeptonsNotFromHadrons      = False,
                                                                             WriteStatus3                    = False,
                                                                             WriteFirstN                     = -1,
                                                                             PreserveAncestors               = False,
                                                                             PreserveParentsSiblingsChildren = True,
                                                                             PreserveGeneratorDescendants    = False))
                                                                             # Not sure what this should be set to nowadays
                                                                             #SimBarcodeOffset                = DerivationFrameworkSimBarcodeOffset)  
        MUON5ThinningTools.append(MUON5TruthThinningTool)

    # --------------------
    # The kernel algorithm
    # --------------------
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name,
                                      AugmentationTools = MUON5AugmentTools,
                                      SkimmingTools     = MUON5SkimmingTools, 
                                      ThinningTools     = MUON5ThinningTools))       
    return acc


def MUON5Cfg(ConfigFlags):
    stream_name = 'StreamDAOD_MUON5'
    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    MUON5TriggerListsHelper = TriggerListsHelper(ConfigFlags)

    # J/psi vertexing
    from DerivationFrameworkMuons.JPsiVertexFitSetupCfg import AddMCPJPsiVertexFitCfg
    acc.merge(AddMCPJPsiVertexFitCfg(ConfigFlags, prefix = 'MUON5'))  

    # Common augmentations
    acc.merge(MUON5KernelCfg(ConfigFlags, name="MUON5Kernel", StreamName = stream_name, TriggerListsHelper = MUON5TriggerListsHelper))
    
    #Adding Lepton Taggers
    from LeptonTaggers.LeptonTaggersConfig import DecorateImprovedPromptLeptonAlgsCfg
    acc.merge(DecorateImprovedPromptLeptonAlgsCfg(ConfigFlags))

    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    
    MUON5SlimmingHelper = SlimmingHelper("MUON5SlimmingHelper", NamesAndTypes = ConfigFlags.Input.TypedCollections, ConfigFlags = ConfigFlags)
    # Many of these are added to AllVariables below as well. We add
    # these items in both places in case some of the smart collections
    # add variables from some other collection.
    MUON5SlimmingHelper.SmartCollections = ["EventInfo",
                                            "PrimaryVertices",
                                            "InDetTrackParticles",
                                            "Electrons",
                                            "Photons",
                                            "Muons",
                                            "AntiKt4EMPFlowJets",
                                            "AntiKtVR30Rmax4Rmin02PV0TrackJets",
                                            "BTagging_AntiKt4EMPFlow",
                                            "BTagging_AntiKtVR30Rmax4Rmin02Track",
                                            "MET_Baseline_AntiKt4EMPFlow",
                                            "TauJets",
                                            "TauJets_MuonRM",
                                            "DiTauJets",
                                            "DiTauJetsLowPt",
                                          ]
    

    MUON5SlimmingHelper.AllVariables = ["egammaClusters",
                                        "CaloCalTopoClusters",
                                        "MuonClusterCollection",
                                        "TopoClusterIsoCentralEventShape",
                                        "TopoClusterIsoForwardEventShape",
                                        "GSFConversionVertices",
                                        "GSFTrackParticles"
                                        "PrimaryVertices",
                                        "AntiKtVR30Rmax4Rmin02PV0TrackJets",
                                        ]

    excludedVertexAuxData = "-vxTrackAtVertex.-MvfFitInfo.-isInitialized.-VTAV"
    StaticContent = []
    StaticContent += ["xAOD::VertexContainer#SoftBVrtClusterTool_Tight_Vertices"]
    StaticContent += ["xAOD::VertexAuxContainer#SoftBVrtClusterTool_Tight_VerticesAux." + excludedVertexAuxData]
    StaticContent += ["xAOD::VertexContainer#SoftBVrtClusterTool_Medium_Vertices"]
    StaticContent += ["xAOD::VertexAuxContainer#SoftBVrtClusterTool_Medium_VerticesAux." + excludedVertexAuxData]
    StaticContent += ["xAOD::VertexContainer#SoftBVrtClusterTool_Loose_Vertices"]
    StaticContent += ["xAOD::VertexAuxContainer#SoftBVrtClusterTool_Loose_VerticesAux." + excludedVertexAuxData]   

    MUON5SlimmingHelper.StaticContent = StaticContent
   
    # Extra content
    CommonEgammaContent= [
        "ptcone20","ptcone30","ptcone40", "ptvarcone20", "ptvarcone30", "ptvarcone40", "topoetcone30",
        "neflowisol20", "neflowisol30", "neflowisol40",
        "ptvarcone20_Nonprompt_All_MaxWeightTTVA_pt500" ,"ptvarcone20_Nonprompt_All_MaxWeightTTVA_pt1000",
        "ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt500","ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt1000",
        "ptvarcone40_Nonprompt_All_MaxWeightTTVA_pt500","ptvarcone40_Nonprompt_All_MaxWeightTTVA_pt1000",
        "ptcone20_Nonprompt_All_MaxWeightTTVA_pt500", "ptcone20_Nonprompt_All_MaxWeightTTVA_pt1000",
        "ptcone30_Nonprompt_All_MaxWeightTTVA_pt500", "ptcone30_Nonprompt_All_MaxWeightTTVA_pt1000",
        "ptcone40_Nonprompt_All_MaxWeightTTVA_pt500", "ptcone40_Nonprompt_All_MaxWeightTTVA_pt1000",
        "topoetconecoreConeEnergyCorrection", "topoetconecoreConeSCEnergyCorrection",
    ]
    ElectronsExtraContent = [
        ".".join(
            [
                "Electrons",
                "deltaPhiRescaled2","deltaPhiFromLastMeasurement",              
                "originalTrackParticle"
            ]  +   CommonEgammaContent
        )
    ]

    PhotonsExtraContent = [
        ".".join(["Photons"] + CommonEgammaContent )
    ]

    MuonsExtraContent = [
        ".".join(
            [
                "Muons",
                "MeasEnergyLoss.MeasEnergyLossSigma.EnergyLossSigma.ParamEnergyLoss",
                "ParamEnergyLossSigmaMinus.ParamEnergyLossSigmaPlus.clusterLink.scatteringCurvatureSignificance",
                "deltaPhiRescaled2.deltaPhiFromLastMeasurement.scatteringNeighbourSignificance",
                "ptcone20.ptcone30.ptcone40.ptvarcone20.ptvarcone30.ptvarcone40.topoetcone30",
                "neflowisol20.neflowisol30.neflowisol40.ptvarcone20_Nonprompt_All_MaxWeightTTVA_pt500",
                "ptvarcone20_Nonprompt_All_MaxWeightTTVA_pt1000.ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt500",
                "ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt1000.ptvarcone40_Nonprompt_All_MaxWeightTTVA_pt500",
                "ptvarcone40_Nonprompt_All_MaxWeightTTVA_pt1000.ptcone20_Nonprompt_All_MaxWeightTTVA_pt500",
                "ptcone20_Nonprompt_All_MaxWeightTTVA_pt1000.ptcone30_Nonprompt_All_MaxWeightTTVA_pt500",
                "ptcone30_Nonprompt_All_MaxWeightTTVA_pt1000.ptcone40_Nonprompt_All_MaxWeightTTVA_pt500",
                "ptcone40_Nonprompt_All_MaxWeightTTVA_pt1000"
            ]   
        )
    ]

    InDetTrackParticlesExtraContent = [
        ".".join(
            [
                "InDetTrackParticles",
                "btagIp_d0.btagIp_z0SinTheta.btagIp_d0Uncertainty.btagIp_z0SinThetaUncertainty",
                "numberOfNextToInnermostPixelLayerHits.numberOfInnermostPixelLayerSharedHits",
                "numberOfInnermostPixelLayerSplitHits.numberOfPixelSplitHits.leptonID"
            ]   
        )
    ]

    ExtraVariables = ElectronsExtraContent + PhotonsExtraContent + MuonsExtraContent + InDetTrackParticlesExtraContent
    MUON5SlimmingHelper.ExtraVariables += ExtraVariables
    MUON5SlimmingHelper.ExtraVariables += [
                                              "CombinedMuonTrackParticles.definingParametersCovMatrix.definingParametersCovMatrix.vertexLink",
                                              "ExtrapolatedMuonTrackParticles.definingParametersCovMatrix.vertexLink",
                                              "MuonSpectrometerTrackParticles.definingParametersCovMatrix.vertexLink",
                                              "CaloCalTopoClusters.calE.calEta.calM.calPhi.e_sampl.rawM.rawPhi.rawEta.rawE",
                                              "EventInfo.GenFiltHT.GenFiltMET.GenFiltHTinclNu.GenFiltPTZ.GenFiltFatJ",
                                              "TauJets.jetLink",
                                           ]
    from LeptonTaggers.LeptonTaggersConfig import GetExtraPromptVariablesForDxAOD, GetExtraImprovedPromptVariablesForDxAOD
    MUON5SlimmingHelper.ExtraVariables += GetExtraPromptVariablesForDxAOD(onlyBDT=False)
    MUON5SlimmingHelper.ExtraVariables += GetExtraImprovedPromptVariablesForDxAOD()
    
    from DerivationFrameworkEGamma.ElectronsCPDetailedContent import ElectronsCPDetailedContent
    MUON5SlimmingHelper.ExtraVariables += ElectronsCPDetailedContent
    from DerivationFrameworkEGamma.ElectronsCPDetailedContent import GSFTracksCPDetailedContent
    MUON5SlimmingHelper.ExtraVariables += GSFTracksCPDetailedContent

    # Truth content
    if ConfigFlags.Input.isMC:
        MUON5SlimmingHelper.SmartCollections += [
                                                  "AntiKt4TruthJets",
                                                  "AntiKt4TruthDressedWZJets",
                                                ]
        MUON5SlimmingHelper.AllVariables += [
                                                "TruthBottom", 
                                                "TruthCharm",
                                                "TruthElectrons",
                                                "TruthMuons",
                                                "TruthNeutrinos",
                                                "TruthPhotons",
                                                "TruthTaus",
                                                "TruthEvents", 
                                                "TruthPrimaryVertices",
                                                "TruthVertices",
                                                ]
        MUON5SlimmingHelper.ExtraVariables+= [  
                                                "TruthPrimaryVertices.t.x.y.z",
                                                "InDetTrackParticles.ftagTruthTypeLabel.ftagTruthOriginLabel.ftagTruthVertexIndex"
                                              ]
                                                

    # Trigger content
    MUON5SlimmingHelper.IncludeTriggerNavigation = False
    MUON5SlimmingHelper.IncludeJetTriggerContent = False
    MUON5SlimmingHelper.IncludeMuonTriggerContent = False
    MUON5SlimmingHelper.IncludeEGammaTriggerContent = False
    MUON5SlimmingHelper.IncludeJetTauEtMissTriggerContent = False
    MUON5SlimmingHelper.IncludeTauTriggerContent = False
    MUON5SlimmingHelper.IncludeEtMissTriggerContent = False
    MUON5SlimmingHelper.IncludeBJetTriggerContent = False
    MUON5SlimmingHelper.IncludeBPhysTriggerContent = False
    MUON5SlimmingHelper.IncludeMinBiasTriggerContent = False

    # Trigger matching
    # Run 2
    if ConfigFlags.Trigger.EDMVersion == 2:
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = MUON5SlimmingHelper, 
                                         OutputContainerPrefix = "TrigMatch_", 
                                         TriggerList = MUON5TriggerListsHelper.Run2TriggerNamesTau)
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = MUON5SlimmingHelper, 
                                         OutputContainerPrefix = "TrigMatch_",
                                         TriggerList = MUON5TriggerListsHelper.Run2TriggerNamesNoTau)
    # Run 3, or Run 2 with navigation conversion
    if ConfigFlags.Trigger.EDMVersion == 3 or (ConfigFlags.Trigger.EDMVersion == 2 and ConfigFlags.Trigger.doEDMVersionConversion):
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import AddRun3TrigNavSlimmingCollectionsToSlimmingHelper
        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(MUON5SlimmingHelper)
        ##################################################### 
        ## NOTE: This block is temporary, during validation of the doEDMVersionConversion flag.
        ## This adds a LOT of containers to the output! In order to help validate the conversion.
        ## It should be removed once doEDMVersionConversion goes into production use.
        if ConfigFlags.Trigger.doEDMVersionConversion:   
            from DerivationFrameworkTrigger.TrigSlimmingHelper import addTrigEDMSetToOutput
            from AthenaCommon.Logging import logging
            msg = logging.getLogger('MUON5Cfg')
            msg.warn('doEDMVersionConversion is still in validation, WRITING FULL TRIGGER EDM TO THE DAOD!')
            addTrigEDMSetToOutput(ConfigFlags, MUON5SlimmingHelper, "AODFULL")
            MUON5SlimmingHelper.AppendToDictionary.update({'HLTNav_R2ToR3Summary':'xAOD::TrigCompositeContainer','HLTNav_R2ToR3SummaryAux':'xAOD::TrigCompositeAuxContainer'})
            MUON5SlimmingHelper.AllVariables += ['HLTNav_R2ToR3Summary']
        ##
        #####################################################
    
    # Output stream    
    MUON5ItemList = MUON5SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(ConfigFlags, "DAOD_MUON5", ItemList=MUON5ItemList, AcceptAlgs=["MUON5Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(ConfigFlags, "DAOD_MUON5", AcceptAlgs=["MUON5Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData, MetadataCategory.TruthMetaData]))
    return acc
