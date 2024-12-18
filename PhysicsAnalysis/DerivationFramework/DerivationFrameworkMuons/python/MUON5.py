# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#====================================================================
# DAOD_MUON5.py
# This defines the component accumulator version of DAOD_MUON5 
# It requires the flag MUON5 in Derivation_tf.py   
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory
import AthenaCommon.SystemOfUnits as Units

def Muon5MumuSelectionCfg(flags, 
                      MuonContainer="Muons", 
                      IdTrackContainer="InDetTrackParticles",
                      applyTrigger = False):
    #copied from MUON1
    acc = ComponentAccumulator()
    from DerivationFrameworkMuons.MuonsToolsConfig import DiMuonTaggingAlgCfg
    ### Z mumu OC events
    acc.merge(DiMuonTaggingAlgCfg(flags,
                                  name                    = "DiMuonTaggingZmumuOC",
                                  Mu1PtMin                = 24*Units.GeV,
                                  Mu1AbsEtaMax            = 2.5,
                                  Mu1RequireQual          = True,
                                  Mu2PtMin                = 3.0*Units.GeV,
                                  Mu2RequireQual          = True,
                                  UseTrackProbe           = False,
                                  MuonContainerKey        = MuonContainer,
                                  TrackContainerKey       = IdTrackContainer, 
                                  InvariantMassLow        = 60*Units.GeV,
                                  IDTrackThinningConeSize = 0.4,
                                  applyTrigger            = applyTrigger, 
                                  BranchPrefix            = "Muon1ZmumuOC"))
    
    ### Z mumu SC events
    acc.merge(DiMuonTaggingAlgCfg(flags,
                                  name                    = "DiMuonTaggingZmumuSC",
                                  Mu1PtMin                = 24*Units.GeV,
                                  Mu1AbsEtaMax            = 2.5,
                                  Mu1RequireQual          = True,
                                  Mu2PtMin                = 3.0*Units.GeV,
                                  Mu2RequireQual          = True,
                                  UseTrackProbe           = False,
                                  MuonContainerKey        = MuonContainer,
                                  TrackContainerKey       = IdTrackContainer,
                                  OppositeCharge          = False,
                                  applyTrigger            = applyTrigger,
                                  InvariantMassLow        = 60*Units.GeV,
                                  BranchPrefix            = "Muon1ZmumuSC")) 
    return acc

def Muon5ElElSelectionCfg(flags):
    #copied from EGAM1
    acc = ComponentAccumulator()

    # ====================================================================
    # 3. di-electron invariant mass for events passing the Z->ee
    #    selection for the e efficiencies with tag and probe.
    #    Based on single e trigger, for reco (central) and ID SF(central)
    #
    #    1 tight e, central, pT>25 GeV
    #    1 e, central, pT>4 GeV
    #    opposite-sign + same-sign
    #    mee>50 GeV (cut applied in skimming step later)
    # ====================================================================

    requirement_tag = " && ".join(
        ["(Electrons.DFCommonElectronsLHMedium)", "(Electrons.pt > 24.5*GeV)"]
    )

    requirement_probe = " && ".join(
        ["(Electrons.DFCommonElectronsLHLoose)", "(Electrons.pt > 3.0*GeV)"]
    )

    acc.setPrivateTools(
        CompFactory.DerivationFramework.EGInvariantMassTool(
            name="EGAM1_ZEEMassTool3",
            Object1Requirements=requirement_tag,
            Object2Requirements=requirement_probe,
            StoreGateEntryName="EGAM1_DiElectronMass3",
            Mass1Hypothesis=0.511 * Units.MeV,
            Mass2Hypothesis=0.511 * Units.MeV,
            Container1Name="Electrons",
            Container2Name="Electrons",
            CheckCharge=False,
            DoTransverseMass=False,
            MinDeltaR=0.0,
        )
    )

    return acc


# Main algorithm config
def MUON5KernelCfg(flags, name='MUON5Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for MUON5"""
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
    diMuonSelAcc = Muon5MumuSelectionCfg(flags,
                                     MuonContainer= kwargs["MuonContainer"], 
                                     IdTrackContainer=kwargs["IdTrkContainer"])
    # ------------
    # Augmentation
    # ------------
    # Strings for applying cuts based on augmentations
    dimuon_flags = ["pass{flag}".format(flag = algo.BranchPrefix) for algo in diMuonSelAcc.getEventAlgos()]
    skimmingORs = [f"DIMU_{flag} > 0" for flag in dimuon_flags ]
    acc.merge(diMuonSelAcc)

    # ------------
    # Augmentation
    # ------------

    # Augmentation tools
    MUON5AugmentTools = []                                     
    muonThinFlags = []
    trkThinFlags = []
    
    Muon5ElElSelectionTool = acc.popToolsAndMerge(Muon5ElElSelectionCfg(flags))
    acc.addPublicTool(Muon5ElElSelectionTool)
    MUON5AugmentTools.append(Muon5ElElSelectionTool)
    skimmingORs.append("(count( EGAM1_DiElectronMass3 > 60.0*GeV ) >= 1)")

    
    ### isolation decorations
    from DerivationFrameworkMuons.TrackIsolationDecoratorConfig import TrackIsolationCfg
    acc.merge(TrackIsolationCfg(flags,TrackCollection="InDetTrackParticles", TrackSelections = trkThinFlags))
    acc.merge(TrackIsolationCfg(flags,TrackCollection="ExtrapolatedMuonTrackParticles"))

    from IsolationSelection.IsolationSelectionConfig import IsoCloseByAlgsCfg
    contNames = [ "Muons", "Electrons", "Photons" ]
    acc.merge(IsoCloseByAlgsCfg(flags, isPhysLite = False, containerNames = contNames, useSelTools = True, stream_name = kwargs['StreamName']))

    ### Calo deposits 
    from DerivationFrameworkMuons.MuonsToolsConfig import MuonCaloDepositAlgCfg
    acc.merge(MuonCaloDepositAlgCfg(flags,
                                    ContainerKey="Muons")) ### Decorate directly the muons
    acc.merge(MuonCaloDepositAlgCfg(flags, 
                                    name = "IdTrkCaloDepsitDecorator",
                                    ContainerKey="InDetTrackParticles")) ### Decorate the ID tracks
    ### Flavour tagging impact parameter decorators
    from BTagging.BTagTrackAugmenterAlgConfig import BTagTrackAugmenterAlgCfg
    acc.merge(BTagTrackAugmenterAlgCfg(
        flags,
        prefix="btagIp_",
        TrackCollection="InDetTrackParticles",
        PrimaryVertexCollectionName="PrimaryVertices"
    ))

    # --------
    # Skimming
    # --------
    
    #we are applying a Zmumu, Zee and a ttbar emu selection. At the moment tauJets selection is not included    
    #emu selection
    elReq1 = '(Electrons.pt > 25*GeV && abs(Electrons.eta) < 2.5 && Electrons.DFCommonElectronsLHMedium)'
    elReq2 = '(Electrons.pt > 3*GeV && abs(Electrons.eta) < 2.8 && Electrons.DFCommonElectronsLHLoose)'
    muReq1 = '(Muons.DFCommonMuonPassPreselection && Muons.DFCommonMuonPassIDCuts && Muons.pt>25*GeV && abs(Muons.eta) <2.5)'
    muReq2 = '(Muons.DFCommonMuonPassPreselection && Muons.DFCommonMuonPassIDCuts && Muons.pt>3*GeV && abs(Muons.eta) <2.8)'
    
    el1 = '(count('+elReq1+') >= 1)'
    el2 = '(count('+elReq2+') >= 1)'
    mu1 = '(count('+muReq1+') >= 1)'
    mu2 = '(count('+muReq2+') >= 1)'
    
    emuSel='(('+el1+"&&"+mu2+')'+'||'+'('+el2+"&&"+mu1+'))'
    skimmingORs.append(emuSel)
    
    lepSelection = '||'.join(skimmingORs)
    
    MUON5SkimmingTools = []
    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import xAODStringSkimmingToolCfg
    # if we are running on upgrade MC (no trigger), we disable the TDT inside the skimming tool. 
    # The TDT is not actually used as we skim only on offline objects. 
    SkimArgs={}
    if  not (flags.Reco.EnableTrigger or flags.Trigger.triggerConfig == 'INFILE'): 
        SkimArgs["TrigDecisionTool"] = ""
    MUON5SkimmingTool1 = acc.getPrimaryAndMerge(xAODStringSkimmingToolCfg(flags,
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
    MUON5TrackThinningTool = acc.getPrimaryAndMerge(TrackParticleThinningCfg(flags,
                                                                        name                    = "MUON5TrackThinningTool",
                                                                        StreamName              = kwargs['StreamName'],
                                                                        SelectionString         = "abs(DFCommonInDetTrackZ0AtPV) < 35.0 && abs(InDetTrackParticles.eta) < 3.2",
                                                                        InDetTrackParticlesKey  = "InDetTrackParticles")
                                                    )
    
    MUON5ThinningTools.append(MUON5TrackThinningTool)

    # Thinning tools
    from DerivationFrameworkCalo.DerivationFrameworkCaloConfig import CaloClusterThinningCfg
    from DerivationFrameworkCalo.CaloCellDFGetterConfig import thinCaloCellsForDFCfg
    from DerivationFrameworkMuons.MuonsToolsConfig import AnalysisMuonThinningAlgCfg
    
    # protect against missing forward tracks in Upgrade 
    fwdTracks = "InDetForwardTrackParticles"
    if fwdTracks not in flags.Input.Collections:
        fwdTracks = "" 
    acc.merge(AnalysisMuonThinningAlgCfg(flags,
                                         MuonPassFlags = muonThinFlags,
                                         TrkPassFlags = trkThinFlags,
                                         StreamName = kwargs['StreamName'], 
                                         IdTrkFwdThinning=fwdTracks))
    

    # keep topoclusters around muons
    MUON5ThinningTool1 = acc.getPrimaryAndMerge(CaloClusterThinningCfg(flags,
                                                                       name                    = "MUON5ThinningTool1",
                                                                       StreamName              = kwargs['StreamName'],
                                                                       SGKey                   = "Muons",
                                                                       SelectionString         = "Muons.pt>4*GeV",
                                                                       TopoClCollectionSGKey   = "CaloCalTopoClusters",
                                                                       ConeSize                = 0.5))
    MUON5ThinningTools.append(MUON5ThinningTool1)
    
    # keep egammaclusters around electrons
    MUON5ThinningTool2 = acc.getPrimaryAndMerge(CaloClusterThinningCfg(flags,
                                                                       name                    = "MUON5ThinningTool2",
                                                                       StreamName              = kwargs['StreamName'],
                                                                       SGKey                   = "Electrons",
                                                                       SelectionString         = "Electrons.pt>4*GeV",
                                                                       CaloClCollectionSGKey   = "egammaClusters",
                                                                       ConeSize                = 0.4))
    MUON5ThinningTools.append(MUON5ThinningTool2)

    # keep egammaclusters around photons
    MUON5ThinningTool3 = acc.getPrimaryAndMerge(CaloClusterThinningCfg(flags,
                                                                       name                    = "MUON5ThinningTool3",
                                                                       StreamName              = kwargs['StreamName'],
                                                                       SGKey                   = "Photons",
                                                                       SelectionString         = "Photons.pt>4*GeV",
                                                                       CaloClCollectionSGKey   = "egammaClusters",
                                                                       ConeSize                = 0.4))
    MUON5ThinningTools.append(MUON5ThinningTool3)
    
    ### cell thinning
    acc.merge(thinCaloCellsForDFCfg(flags,
                                    inputClusterKeys = ["MuonClusterCollection"],
                                    streamName       = kwargs['StreamName'],
                                    outputCellKey    = "DFMUONCellContainer"))
  
    ### Truth thinning
    if flags.Input.isMC:        
        from DerivationFrameworkMCTruth.TruthDerivationToolsConfig import MenuTruthThinningCfg
        MUON5TruthThinningTool = acc.getPrimaryAndMerge(MenuTruthThinningCfg(flags,
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
                                                                             WriteNotPhysical                = False,
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


def MUON5Cfg(flags):
    stream_name = 'StreamDAOD_MUON5'
    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    MUON5TriggerListsHelper = TriggerListsHelper(flags)

    # J/psi vertexing
    from DerivationFrameworkMuons.JPsiVertexFitSetupCfg import AddMCPJPsiVertexFitCfg
    acc.merge(AddMCPJPsiVertexFitCfg(flags, prefix = 'MUON5'))  

    # Common augmentations
    acc.merge(MUON5KernelCfg(flags, name="MUON5Kernel", StreamName = stream_name, TriggerListsHelper = MUON5TriggerListsHelper))
    
    #Adding Lepton Taggers
    from LeptonTaggers.LeptonTaggersConfig import DecorateImprovedPromptLeptonAlgsCfg, DecoratePLITAlgsCfg
    acc.merge(DecorateImprovedPromptLeptonAlgsCfg(flags))
    acc.merge(DecoratePLITAlgsCfg(flags))

    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    
    MUON5SlimmingHelper = SlimmingHelper("MUON5SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
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
                                            "MET_Baseline_AntiKt4EMPFlow",
                                          ]
    

    MUON5SlimmingHelper.AllVariables = [
                                        "egammaClusters",
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
    StaticContent += ["CaloCellContainer#DFMUONCellContainer"]
    StaticContent += ["CaloClusterCellLinkContainer#MuonClusterCollection_links"]

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
        "topoetconecoreConeEnergyCorrection"
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
                "ptcone40_Nonprompt_All_MaxWeightTTVA_pt1000",
                "msInnerMatchChi2",
            ]   
        )
    ]

    InDetTrackParticlesExtraContent = [
        ".".join(
            [
                "InDetTrackParticles","TTVA_AMVFVertices.TTVA_AMVFWeights",
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
    from LeptonTaggers.LeptonTaggersConfig import GetExtraImprovedPromptVariablesForDxAOD, GetExtraPLITVariablesForDxAOD
    MUON5SlimmingHelper.ExtraVariables += GetExtraImprovedPromptVariablesForDxAOD()
    MUON5SlimmingHelper.ExtraVariables += GetExtraPLITVariablesForDxAOD()
    
    from DerivationFrameworkEGamma.ElectronsCPDetailedContent import ElectronsCPDetailedContent
    MUON5SlimmingHelper.ExtraVariables += ElectronsCPDetailedContent
    from DerivationFrameworkEGamma.ElectronsCPDetailedContent import GSFTracksCPDetailedContent
    MUON5SlimmingHelper.ExtraVariables += GSFTracksCPDetailedContent

    # Truth content
    if flags.Input.isMC:
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
    MUON5SlimmingHelper.IncludeTauTriggerContent = False
    MUON5SlimmingHelper.IncludeEtMissTriggerContent = False
    MUON5SlimmingHelper.IncludeBJetTriggerContent = False
    MUON5SlimmingHelper.IncludeBPhysTriggerContent = False
    MUON5SlimmingHelper.IncludeMinBiasTriggerContent = False

    # Trigger matching
    # Run 2
    if flags.Trigger.EDMVersion == 2:
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = MUON5SlimmingHelper, 
                                               OutputContainerPrefix = "TrigMatch_", 
                                               TriggerList = MUON5TriggerListsHelper.Run2TriggerNamesTau)
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = MUON5SlimmingHelper, 
                                               OutputContainerPrefix = "TrigMatch_",
                                               TriggerList = MUON5TriggerListsHelper.Run2TriggerNamesNoTau)
    # Run 3, or Run 2 with navigation conversion
    if flags.Trigger.EDMVersion == 3 or (flags.Trigger.EDMVersion == 2 and flags.Trigger.doEDMVersionConversion):
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import AddRun3TrigNavSlimmingCollectionsToSlimmingHelper
        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(MUON5SlimmingHelper)
    
    # Output stream    
    MUON5ItemList = MUON5SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_MUON5", ItemList=MUON5ItemList, AcceptAlgs=["MUON5Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_MUON5", AcceptAlgs=["MUON5Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData, MetadataCategory.TruthMetaData]))
    return acc
