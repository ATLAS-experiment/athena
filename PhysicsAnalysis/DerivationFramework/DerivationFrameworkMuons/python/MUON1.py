# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python
#====================================================================
# DAOD_MUON1.py
# This defines the component accumulator version of DAOD_MUON1 
# It requires the flag MUON1 in Derivation_tf.py   
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory
import AthenaCommon.SystemOfUnits as Units


def Muon1SelectionCfg(flags, 
                      MuonContainer="Muons", 
                      IdTrackContainer="InDetTrackParticles",
                      applyTrigger = False):
    acc = ComponentAccumulator()
    from DerivationFrameworkMuons.MuonsToolsConfig import DiMuonTaggingAlgCfg
    ### Z mumu OC events
    acc.merge(DiMuonTaggingAlgCfg(flags,
                                  name                    = "DiMuonTaggingZmumuOC",
                                  Mu1PtMin                = 4.*Units.GeV,
                                  Mu1AbsEtaMax            = 2.75,
                                  Mu1RequireQual          = True,
                                  Mu2PtMin                = 2.*Units.GeV,
                                  UseTrackProbe           = True,
                                  MuonContainerKey        = MuonContainer,
                                  TrackContainerKey       = IdTrackContainer, 
                                  InvariantMassLow        = 60*Units.GeV,
                                  IDTrackThinningConeSize = 0.4,
                                  applyTrigger            = applyTrigger, 
                                  BranchPrefix            = "Muon1ZmumuOC"))
    
    ### Z mumu SC events
    acc.merge(DiMuonTaggingAlgCfg(flags,
                                  name                    = "DiMuonTaggingZmumuSC",
                                  Mu1PtMin                = 4.*Units.GeV,
                                  Mu1AbsEtaMax            = 2.75,
                                  Mu1RequireQual          = True,
                                  Mu2PtMin                = 2.*Units.GeV,
                                  UseTrackProbe           = True,
                                  MuonContainerKey        = MuonContainer,
                                  TrackContainerKey       = IdTrackContainer,
                                  OppositeCharge          = False,
                                  applyTrigger            = applyTrigger,
                                  InvariantMassLow        = 60*Units.GeV,
                                  BranchPrefix            = "Muon1ZmumuSC"))
    # Jpsi
    acc.merge(DiMuonTaggingAlgCfg(flags,
                                  name                    = "DiMuonTaggingJpsi",
                                  Mu1PtMin                = 4.*Units.GeV,
                                  Mu1AbsEtaMax            = 2.75,
                                  Mu1RequireQual          = True,
                                  Mu2PtMin                = 2.*Units.GeV,
                                  UseTrackProbe           = True,
                                  MuonContainerKey        = MuonContainer,
                                  TrackContainerKey       = IdTrackContainer,
                                  applyTrigger            = applyTrigger,
                                  InvariantMassLow        = 2.0*Units.GeV,
                                  InvariantMassHigh       = 4.8*Units.GeV,
                                  BranchPrefix            = "Muon1JPsi"))
    ### Upsilon tagging
    acc.merge(DiMuonTaggingAlgCfg(flags,
                                  name                    = "DiMuonTaggingUpsilon",
                                  Mu1PtMin                = 4.*Units.GeV,
                                  Mu1AbsEtaMax            = 2.75,
                                  Mu1RequireQual          = True,
                                  Mu2PtMin                = 2.*Units.GeV,
                                  MuonContainerKey        = MuonContainer,
                                  TrackContainerKey       = IdTrackContainer,
                                  InvariantMassLow        = 7.0*Units.GeV,
                                  InvariantMassHigh       = 13.*Units.GeV,
                                  applyTrigger            = applyTrigger,
                                  BranchPrefix            = "Muon1Upsilon"))
    if flags.Input.isMC:
        ### Accept every muon around a truth particle
        acc.merge(DiMuonTaggingAlgCfg(flags,
                                      name                    = "MuonTruthTagging",
                                      Mu1PtMin                = 2.*Units.GeV,
                                      Mu1RequireQual          = True,
                                      Mu2PtMin                = 60.*Units.TeV, # Dummy value
                                      UseTrackProbe           = True,
                                      MuonContainerKey        = MuonContainer,
                                      TrackContainerKey       = IdTrackContainer,
                                      OppositeCharge          = False,
                                      InvariantMassLow        = 60*Units.TeV, # Dummy value
                                      applyTrigger            = applyTrigger,
                                      IDTrackThinningConeSize = 0.1,
                                      BranchPrefix            = "TruthMuon")) 

    return acc


# Main algorithm config
def MUON1KernelCfg(flags, name='MUON1Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for MUON1"""
    acc = ComponentAccumulator()
    
    kwargs.setdefault("MuonContainer", "Muons")
    kwargs.setdefault("IdTrkContainer", "InDetTrackParticles")
    kwargs.setdefault("MsTrkContainer", "ExtrapolatedMuonTrackParticles")
    kwargs.setdefault("scheduleThinning", True)
    
    
    # --------------------
    # Common augmentations
    # --------------------
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(flags, TriggerListsHelper = kwargs['TriggerListsHelper']))

    ### Basic muon selection
    diMuonSelAcc = Muon1SelectionCfg(flags,
                                     MuonContainer= kwargs["MuonContainer"], 
                                     IdTrackContainer=kwargs["IdTrkContainer"])
    # ------------
    # Augmentation
    # ------------
    # Strings for applying cuts based on augmentations
    muonThinFlags = ["pass{flag}".format(flag = algo.BranchPrefix) for algo in diMuonSelAcc.getEventAlgos()]
    skimmingORs = [f"DIMU_{flag} > 0" for flag in muonThinFlags]  
    trkThinFlags = [muonThinFlags[i] for i, algo in enumerate(diMuonSelAcc.getEventAlgos()) if algo.UseTrackProbe ]
    acc.merge(diMuonSelAcc)


    ## MC truth classification and isolation
    if flags.Input.isMC:        
        from DerivationFrameworkMCTruth.TruthDerivationToolsConfig import MuonTruthClassifierFallbackCfg
        MUON1MuonTruthClassifierFallback = acc.getPrimaryAndMerge(MuonTruthClassifierFallbackCfg(flags,
                                                                                                 name         = "MUON1MuonTruthClassifierFallback",
                                                                                                 ContainerKey = kwargs["MuonContainer"]))
        acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel("MuonTruthClassifierFallBack",
                                                                          AugmentationTools=[MUON1MuonTruthClassifierFallback]))
        from DerivationFrameworkMCTruth.TruthDerivationToolsConfig import MuonTruthIsolationDecorAlgCfg
        acc.merge(MuonTruthIsolationDecorAlgCfg(flags, 
                                                name         = "MUON1MuonTruthIsolationAlg",
                                                ContainerKey = kwargs["MuonContainer"]))

    
    ### J/psi vertexing
    from DerivationFrameworkMuons.JPsiVertexFitSetupCfg import AddMCPJPsiVertexFitCfg
    acc.merge(AddMCPJPsiVertexFitCfg(flags, 
                                     prefix='Muon1', 
                                     IdTrkContainer = kwargs["IdTrkContainer"], 
                                     MuonContainer = kwargs["MuonContainer"]))
    ### Track isolation deccorations
    from DerivationFrameworkMuons.TrackIsolationDecoratorConfig import TrackIsolationCfg
    acc.merge(TrackIsolationCfg(flags, 
                                TrackCollection=kwargs["IdTrkContainer"], 
                                TrackSelections = trkThinFlags))
    acc.merge(TrackIsolationCfg(flags,
                                TrackCollection=kwargs["MsTrkContainer"]))
    
    ### Calo deposits 
    from DerivationFrameworkMuons.MuonsToolsConfig import MuonCaloDepositAlgCfg
    acc.merge(MuonCaloDepositAlgCfg(flags,
                                    ContainerKey= kwargs["MuonContainer"],
                                    TrackSelections = muonThinFlags))    
    acc.merge(MuonCaloDepositAlgCfg(flags,
                                     name = "IdTrkCaloDepsitDecorator",
                                     ContainerKey= kwargs["IdTrkContainer"],
                                     TrackSelections = trkThinFlags))
    
    #### Extrapolation of the ID tracks to the trigger plane
    from DerivationFrameworkMuons.MuonsToolsConfig import MuonTPExtrapolationAlgCfg 
    acc.merge(MuonTPExtrapolationAlgCfg(flags,
                                        ContainerKey= kwargs["MuonContainer"],
                                        TrackSelections = ["passMuon1JPsi"]))
    
    acc.merge(MuonTPExtrapolationAlgCfg(flags,
                                        name = "MuonTPTrigExtrapolation",
                                        ContainerKey= kwargs["IdTrkContainer"],
                                        TrackSelections = ["passMuon1JPsi"]))


    # --------
    # Skimming
    # --------
    MUON1SkimmingTools = []
    skimming_expression = '||'.join(skimmingORs)
    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import xAODStringSkimmingToolCfg
    MUON1SkimmingTool1 = acc.getPrimaryAndMerge(xAODStringSkimmingToolCfg(flags,
                                                                          name       = "MUON1SkimmingTool1",
                                                                          expression = skimming_expression))
    MUON1SkimmingTools.append(MUON1SkimmingTool1) 

    # --------
    # Thinning
    # --------

    # Thinning tools
    from DerivationFrameworkCalo.DerivationFrameworkCaloConfig import CaloClusterThinningCfg
    from DerivationFrameworkCalo.CaloCellDFGetterConfig import thinCaloCellsForDFCfg
    from DerivationFrameworkMuons.MuonsToolsConfig import AnalysisMuonThinningAlgCfg
    MUON1ThinningTools = [] 
    if kwargs["scheduleThinning"]:
        acc.merge(AnalysisMuonThinningAlgCfg(flags,
                                             MuonPassFlags = ["{cont}.{passDecor}".format(cont = kwargs["MuonContainer"],
                                                                                          passDecor = passDecor) for passDecor in muonThinFlags],
                                             TrkPassFlags =["{cont}.{passDecor}".format(cont = kwargs["IdTrkContainer"],
                                                                                        passDecor = passDecor) for passDecor in trkThinFlags],
                                             StreamName = kwargs['StreamName']))


        # keep topoclusters around muons
        MUON1ThinningTool1 = acc.getPrimaryAndMerge(CaloClusterThinningCfg(flags,
                                                                           name                    = "MUON1ThinningTool4",
                                                                           StreamName              = kwargs['StreamName'],
                                                                           SGKey                   = "Muons",
                                                                           SelectionString         = "Muons.pt>4*GeV",
                                                                           TopoClCollectionSGKey   = "CaloCalTopoClusters",
                                                                           ConeSize                = 0.5))
        MUON1ThinningTools.append(MUON1ThinningTool1)
    
        ### cell thinning
        acc.merge(thinCaloCellsForDFCfg(flags,
                                        inputClusterKeys = ["MuonClusterCollection"],
                                        streamName       = kwargs['StreamName'],
                                        outputCellKey    = "DFMUONCellContainer"))

        ### Tracks associated with fitted vertices
        from DerivationFrameworkBPhys.commonBPHYMethodsCfg import Thin_vtxTrkCfg 
        MUON1Thin_vtxTrk = acc.getPrimaryAndMerge(Thin_vtxTrkCfg(flags,
                                                                 name                       = "MUON1Thin_vtxTrk",
                                                                 StreamName                 = kwargs['StreamName'],
                                                                 TrackParticleContainerName = "InDetTrackParticles",
                                                                 VertexContainerNames       = ["Muon1JpsiCandidates"],
                                                                 PassFlags                  = ["passed_Jpsi"] ))
        MUON1ThinningTools.append(MUON1Thin_vtxTrk)
                                                   
        ### Truth thinning
        if flags.Input.isMC:
            from DerivationFrameworkMCTruth.TruthDerivationToolsConfig import MenuTruthThinningCfg
            MUON1TruthThinningTool = acc.getPrimaryAndMerge(MenuTruthThinningCfg(flags,
                                                                                 name                            = "MUON1TruthThinningTool",
                                                                                 StreamName                      = kwargs['StreamName'],
                                                                                 WritePartons                    = False,
                                                                                 WriteHadrons                    = False,
                                                                                 WriteCHadrons                   = False,
                                                                                 WriteBHadrons                   = True,
                                                                                 WriteGeant                      = False,
                                                                                 WriteTauHad                     = False,
                                                                                 PartonPtThresh                  = -1.0,
                                                                                 WriteBSM                        = True,
                                                                                 WriteBosons                     = True,
                                                                                 WriteBosonProducts              = False,
                                                                                 WriteBSMProducts                = True,
                                                                                 WriteTopAndDecays               = False,
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
            MUON1ThinningTools.append(MUON1TruthThinningTool)

    # --------------------
    # The kernel algorithm
    # --------------------
    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel(name,
                                                                      SkimmingTools     = MUON1SkimmingTools, 
                                                                      ThinningTools     = MUON1ThinningTools))
    return acc


def MUON1Cfg(flags):
    stream_name = 'StreamDAOD_MUON1'
    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    MUON1TriggerListsHelper = TriggerListsHelper(flags)

 

    # Common augmentations
    acc.merge(MUON1KernelCfg(flags, name="MUON1Kernel", StreamName = stream_name, TriggerListsHelper = MUON1TriggerListsHelper))

    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    
    isoCones = [f"pt{cone}cone{size}_Nonprompt_All_MaxWeightTTVA_pt{pt}" for cone in ["", "var"] for size in range(20,50,10) for pt in [500, 1000] ] 
    isoCones += [f"{cone}{size}" for cone in ["topoetcone", "neflowisol"] for size in range(20,50,10)]
    
    decorationsID = ["TTVA_AMVFVertices" , "TTVA_AMVFWeights", "vx", "vy", "vz"]
    decorationsME = []
    decoartionsMuon = ["EnergyLossSigma"]

    decorationsID += isoCones
    decorationsME += isoCones
    CaloDeposDecors = ["CaloDeposits", "CaloElosses", "CaloDepType"]
    decorationsID += CaloDeposDecors
    decoartionsMuon += CaloDeposDecors
    tpExtrapolations = ["EtaTriggerPivot", "PhiTriggerPivot", "DecoratedPivotEtaPhi"]
    decorationsID += tpExtrapolations
    decoartionsMuon += tpExtrapolations
    
    
    MUON1SlimmingHelper = SlimmingHelper("MUON1SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
    MUON1SlimmingHelper.SmartCollections = ["EventInfo",
                                            "Electrons",
                                            "Photons",
                                            "Muons",
                                            "PrimaryVertices",
                                            "InDetTrackParticles",
                                            "AntiKt4EMTopoJets",
                                            "AntiKt4EMPFlowJets",
                                            "BTagging_AntiKt4EMPFlow",
                                            "BTagging_AntiKtVR30Rmax4Rmin02Track",
                                            "MET_Baseline_AntiKt4EMTopo",
                                            "MET_Baseline_AntiKt4EMPFlow",
                                            "TauJets",
                                            "TauJets_MuonRM",
                                            "DiTauJets",
                                            "DiTauJetsLowPt",
                                            "AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets",
                                            "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets",
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
    StaticContent += ["xAOD::VertexContainer#Muon1JpsiCandidates"]
    StaticContent += ["xAOD::VertexAuxContainer#Muon1JpsiCandidatesAux."+excludedVertexAuxData]
    
    MUON1SlimmingHelper.StaticContent = StaticContent
   
    # Extra content
    MUON1SlimmingHelper.ExtraVariables += ["AntiKt4EMTopoJets.DFCommonJets_QGTagger_truthjet_nCharged.DFCommonJets_QGTagger_truthjet_pt.DFCommonJets_QGTagger_truthjet_eta.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1.ConeExclBHadronsFinal.ConeExclCHadronsFinal.GhostBHadronsFinal.GhostCHadronsFinal.GhostBHadronsFinalCount.GhostBHadronsFinalPt.GhostCHadronsFinalCount.GhostCHadronsFinalPt",
                                           "AntiKt4EMPFlowJets.DFCommonJets_QGTagger_truthjet_nCharged.DFCommonJets_QGTagger_truthjet_pt.DFCommonJets_QGTagger_truthjet_eta.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1.ConeExclBHadronsFinal.ConeExclCHadronsFinal.GhostBHadronsFinal.GhostCHadronsFinal.GhostBHadronsFinalCount.GhostBHadronsFinalPt.GhostCHadronsFinalCount.GhostCHadronsFinalPt",
                                           "TruthPrimaryVertices.t.x.y.z",
                                           "PrimaryVertices.trackWeights",
                                           "MuonSegments.chiSquared.numberDoF.nPrecisionHits.x.y.z",
                                           "Muons." +".".join(decoartionsMuon),
                                           "InDetTrackParticles."+ ".".join(decorationsID),
                                           "ExtrapolatedMuonTrackParticles." +".".join(decorationsME),
                                           "CaloCalTopoClusters.calE.calEta.calM.calPhi.e_sampl.rawM.rawPhi.rawEta.rawE",
                                           "EventInfo.GenFiltHT.GenFiltMET.GenFiltHTinclNu.GenFiltPTZ.GenFiltFatJ",
                                           "TauJets.dRmax.etOverPtLeadTrk",
                                           "TauJets_MuonRM.dRmax.etOverPtLeadTrk",
                                           "CombinedMuonTrackParticles.vx.vy.vz",
                                           "MuonSpectrometerTrackParticles.vx.vy.vz",
                                           "HLT_xAOD__TrigMissingETContainer_TrigEFMissingET.ex.ey",
                                           "HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_mht.ex.ey"]

    # Truth content
    if flags.Input.isMC:
        MUON1SlimmingHelper.AllVariables += ["TruthEvents", "TruthParticles", "TruthVertices", "MuonTruthParticles"]

    # Trigger content
    MUON1SlimmingHelper.IncludeTriggerNavigation = False
    MUON1SlimmingHelper.IncludeJetTriggerContent = False
    MUON1SlimmingHelper.IncludeMuonTriggerContent = True
    MUON1SlimmingHelper.IncludeEGammaTriggerContent = False
    MUON1SlimmingHelper.IncludeJetTauEtMissTriggerContent = False
    MUON1SlimmingHelper.IncludeTauTriggerContent = False
    MUON1SlimmingHelper.IncludeEtMissTriggerContent = False
    MUON1SlimmingHelper.IncludeBJetTriggerContent = False
    MUON1SlimmingHelper.IncludeBPhysTriggerContent = False
    MUON1SlimmingHelper.IncludeMinBiasTriggerContent = False

    # Trigger matching
    # Run 2
    if flags.Trigger.EDMVersion == 2:
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = MUON1SlimmingHelper, 
                                         OutputContainerPrefix = "TrigMatch_", 
                                         TriggerList = MUON1TriggerListsHelper.Run2TriggerNamesTau)
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = MUON1SlimmingHelper, 
                                         OutputContainerPrefix = "TrigMatch_",
                                         TriggerList = MUON1TriggerListsHelper.Run2TriggerNamesNoTau)
    # Run 3, or Run 2 with navigation conversion
    if flags.Trigger.EDMVersion == 3 or (flags.Trigger.EDMVersion == 2 and flags.Trigger.doEDMVersionConversion):
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import AddRun3TrigNavSlimmingCollectionsToSlimmingHelper
        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(MUON1SlimmingHelper)
        ##################################################### 
        ## NOTE: This block is temporary, during validation of the doEDMVersionConversion flag.
        ## This adds a LOT of containers to the output! In order to help validate the conversion.
        ## It should be removed once doEDMVersionConversion goes into production use.
        if flags.Trigger.doEDMVersionConversion:   
            from DerivationFrameworkTrigger.TrigSlimmingHelper import addTrigEDMSetToOutput
            from AthenaCommon.Logging import logging
            msg = logging.getLogger('MUON1Cfg')
            msg.warn('doEDMVersionConversion is still in validation, WRITING FULL TRIGGER EDM TO THE DAOD!')
            addTrigEDMSetToOutput(flags, MUON1SlimmingHelper, "AODFULL")
            MUON1SlimmingHelper.AppendToDictionary.update({'HLTNav_R2ToR3Summary':'xAOD::TrigCompositeContainer','HLTNav_R2ToR3SummaryAux':'xAOD::TrigCompositeAuxContainer'})
            MUON1SlimmingHelper.AllVariables += ['HLTNav_R2ToR3Summary']
        ##
        #####################################################

    # Output stream    
    MUON1ItemList = MUON1SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_MUON1", ItemList=MUON1ItemList, AcceptAlgs=["MUON1Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_MUON1", AcceptAlgs=["MUON1Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData, MetadataCategory.TruthMetaData]))

    return acc

