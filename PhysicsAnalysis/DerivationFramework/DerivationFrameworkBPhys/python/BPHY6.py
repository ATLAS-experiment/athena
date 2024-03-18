# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#====================================================================
# BPHY6.py
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

BPHYDerivationName = "BPHY6"
streamName = "StreamDAOD_BPHY6"

def BPHY6Cfg(flags):
    from DerivationFrameworkBPhys.commonBPHYMethodsCfg import (BPHY_V0ToolCfg,  BPHY_InDetDetailedTrackSelectorToolCfg, BPHY_VertexPointEstimatorCfg, BPHY_TrkVKalVrtFitterCfg)
    from JpsiUpsilonTools.JpsiUpsilonToolsConfig import PrimaryVertexRefittingToolCfg
    acc = ComponentAccumulator()
    isSimulation = flags.Input.isMC
    # General Variables
    dimuon_chi2_max = 50.
    dimuon_mass_min = 100.
    dimuon_mass_max = 150e3

    V0Tools = acc.popToolsAndMerge(BPHY_V0ToolCfg(flags, BPHYDerivationName))
    vkalvrt = acc.popToolsAndMerge(BPHY_TrkVKalVrtFitterCfg(flags, BPHYDerivationName))        # VKalVrt vertex fitter
    acc.addPublicTool(vkalvrt)
    acc.addPublicTool(V0Tools)
    trackselect = acc.popToolsAndMerge(BPHY_InDetDetailedTrackSelectorToolCfg(flags, BPHYDerivationName))
    acc.addPublicTool(trackselect)
    vpest = acc.popToolsAndMerge(BPHY_VertexPointEstimatorCfg(flags, BPHYDerivationName))
    acc.addPublicTool(vpest)
    from TrkConfig.AtlasExtrapolatorConfig import InDetExtrapolatorCfg
    extrap = acc.popToolsAndMerge(InDetExtrapolatorCfg(flags))
    acc.addPublicTool(extrap)
    BPHY6_Extrap_Tool = CompFactory.DerivationFramework.MuonExtrapolationTool(name = "BPHY6_ExtrapolationTool", Extrapolator = extrap)
    BPHY6JpsiFinder = CompFactory.Analysis.JpsiFinder(
                name                        = "BPHY6JpsiFinder",
                muAndMu                     = True,
                muAndTrack                  = False,
                TrackAndTrack               = False,
                assumeDiMuons               = True,    # If true, will assume dimu hypothesis and use PDG value for mu mass
                invMassUpper                = dimuon_mass_max,
                invMassLower                = dimuon_mass_min,
                Chi2Cut                     = dimuon_chi2_max,
                oppChargesOnly              = True,
                atLeastOneComb              = True,
                useCombinedMeasurement      = False, # Only takes effect if combOnly=True 
                muonCollectionKey           = "Muons",
                TrackParticleCollection     = "InDetTrackParticles",
                V0VertexFitterTool          = None,             # V0 vertex fitter
                useV0Fitter                 = False,                   # if False a TrkVertexFitterTool will be used
                TrkVertexFitterTool         = vkalvrt,        # VKalVrt vertex fitter
                TrackSelectorTool           = trackselect,
                VertexPointEstimator        = vpest,
                useMCPCuts                  = False )

    BPHY6_Reco_mumu = CompFactory.DerivationFramework.Reco_Vertex(
                name                   = "BPHY6_Reco_mumu",
                VertexSearchTool             = BPHY6JpsiFinder,
                OutputVtxContainerName = "BPHY6OniaCandidates",
                V0Tools                = V0Tools,
                PVRefitter             = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
                PVContainerName        = "PrimaryVertices",
                RefPVContainerName     = "BPHY6RefittedPrimaryVertices")

    BPHY6_Select_Jpsi2mumu = CompFactory.DerivationFramework.Select_onia2mumu(
                name                  = "BPHY6_Select_Jpsi2mumu",
                HypothesisName        = "Jpsi",
                InputVtxContainerName = "BPHY6OniaCandidates",
                V0Tools               = V0Tools,
                VtxMassHypo           = 3096.916,
                MassMin               = 2700.0,
                MassMax               = 3500.0,
                Chi2Max               = 20)

    BPHY6_Select_Psi2mumu = CompFactory.DerivationFramework.Select_onia2mumu(
                name                  = "BPHY6_Select_Psi2mumu",
                HypothesisName        = "Psi",
                InputVtxContainerName = "BPHY6OniaCandidates",
                V0Tools               = V0Tools,
                VtxMassHypo           = 3686.09,
                MassMin               = 3200.0,
                MassMax               = 4200.0,
                Chi2Max               = 20)

    BPHY6_Select_Upsi2mumu = CompFactory.DerivationFramework.Select_onia2mumu(
                name                  = "BPHY6_Select_Upsi2mumu",
                HypothesisName        = "Upsi",
                InputVtxContainerName = "BPHY6OniaCandidates",
                V0Tools               = V0Tools,
                VtxMassHypo           = 9460.30,
                MassMin               = 8000.0,
                MassMax               = 12000.0,
                Chi2Max               = 20)
  
    BPHY6_Select_Bmumu2mumu = CompFactory.DerivationFramework.Select_onia2mumu(
                name                  = "BPHY6_Select_Bmumu2mumu",
                HypothesisName        = "Bmumu",
                InputVtxContainerName = "BPHY6OniaCandidates",
                V0Tools               = V0Tools,
                VtxMassHypo           = 5366.77,
                MassMin               = 4200.0,
                MassMax               = 8000.0,
                Chi2Max               = 20)
  
    BPHY6_Select_Zmumu2mumu = CompFactory.DerivationFramework.Select_onia2mumu(
                name                  = "BPHY6_Select_Zmumu2mumu",
                HypothesisName        = "Zmumu",
                InputVtxContainerName = "BPHY6OniaCandidates",
                V0Tools               = V0Tools,
                VtxMassHypo           = 91187.6,
                MassMin               = 60000.0,
                MassMax               = 120000.0,
                Chi2Max               = 20)

    BPHY6_Select_Onia2mumu = CompFactory.DerivationFramework.Select_onia2mumu(
                name                  = "BPHY6_Select_Onia2mumu",
                HypothesisName        = "Onia",
                InputVtxContainerName = "BPHY6OniaCandidates",
                V0Tools               = V0Tools,
                VtxMassHypo           = 3096.916,
                MassMin               = dimuon_mass_min,
                MassMax               = dimuon_mass_max,
                Chi2Max               = 20)

    trigger_list = [r'HLT_\d?mu\d+']

    BPHY6TrigSkimmingTool = CompFactory.DerivationFramework.TriggerSkimmingTool(   name     = "BPHY6TrigSkimmingTool",
                                                                TriggerListOR               = trigger_list )
    expression = "count(BPHY6OniaCandidates.passed_Onia) > 0 "
    BPHY6_SelectEvent = CompFactory.DerivationFramework.xAODStringSkimmingTool(name = "BPHY6_SelectEvent",
                                                                expression = expression)
    BPHY6Thin_vtxTrk = CompFactory.DerivationFramework.Thin_vtxTrk(
                name                       = "BPHY6Thin_vtxTrk",
                TrackParticleContainerName = "InDetTrackParticles",
                StreamName = streamName,
                VertexContainerNames       = ["BPHY6OniaCandidates"],
                PassFlags                  = ["passed_Onia"], )
    BPHY6MuonTPThinningTool = CompFactory.DerivationFramework.MuonTrackParticleThinning(name     = "BPHY6MuonTPThinningTool",
                                                                         MuonKey                 = "Muons",
                                                                         StreamName = streamName,
                                                                         InDetTrackParticlesKey  = "InDetTrackParticles")


    BPHY6ThinningTools = [BPHY6Thin_vtxTrk, BPHY6MuonTPThinningTool]
    if isSimulation:
       BPHY6TruthThinTool = CompFactory.DerivationFramework.GenericTruthThinning(name   = "BPHY6TruthThinTool",
                         ParticleSelectionString = "TruthParticles.pdgId == 443 || TruthParticles.pdgId == 100443 || TruthParticles.pdgId == 553 || TruthParticles.pdgId == 100553 || TruthParticles.pdgId == 200553 || TruthParticles.pdgId == 23 || TruthParticles.pdgId == 531 || TruthParticles.pdgId == 511 || TruthParticles.pdgId == 521 || TruthParticles.pdgId == 541",
                         PreserveDescendants     = True,
                         StreamName = streamName,
                         PreserveAncestors       = True)
       BPHY6ThinningTools.append(BPHY6TruthThinTool)

    SkimmingORTool = CompFactory.DerivationFramework.FilterCombinationOR("BPHY6SkimmingOR",
                                                        FilterList = [BPHY6_SelectEvent,BPHY6TrigSkimmingTool])

    augTools = [BPHY6_Reco_mumu, BPHY6_Select_Jpsi2mumu, BPHY6_Select_Psi2mumu, BPHY6_Select_Upsi2mumu,BPHY6_Select_Bmumu2mumu,
                                     BPHY6_Select_Zmumu2mumu,BPHY6_Select_Onia2mumu, BPHY6_Extrap_Tool]
    for t in  augTools + BPHY6ThinningTools + [SkimmingORTool] + [BPHY6_SelectEvent,BPHY6TrigSkimmingTool]: acc.addPublicTool(t)
    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel("BPHY6Kernel",
                AugmentationTools = augTools,
                SkimmingTools     =  [SkimmingORTool],
                ThinningTools     = BPHY6ThinningTools  ))

    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    BPHY6SlimmingHelper = SlimmingHelper("BPHY6SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
    
    # Needed for trigger objects
    BPHY6SlimmingHelper.IncludeMuonTriggerContent = True
    BPHY6SlimmingHelper.IncludeBPhysTriggerContent = True
    from DerivationFrameworkBPhys.commonBPHYMethodsCfg import getDefaultAllVariables
    AllVariables  = getDefaultAllVariables()
    StaticContent = []
    AllVariables += ["LVL1MuonRoIs"]
    
    ## primary vertices
    AllVariables += ["PrimaryVertices"]
    StaticContent += ["xAOD::VertexContainer#BPHY6RefittedPrimaryVertices"]
    StaticContent += ["xAOD::VertexAuxContainer#BPHY6RefittedPrimaryVerticesAux."]
    
    ## ID track particles
    AllVariables += ["InDetTrackParticles"]
    
    AllVariables += ["HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_Muon_EFID"]
    AllVariables += ["HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_Muon_IDTrig"]
    AllVariables += ["HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_Muon_FTF"]
    AllVariables += ["HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_Bphysics_FTF"]
    AllVariables += ["HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_Bphysics_IDTrig"]
    
    
    
    ## combined / extrapolated muon track particles 
    ## (note: for tagged muons there is no extra TrackParticle collection since the ID tracks
    ##        are store in InDetTrackParticles collection)
    AllVariables += ["CombinedMuonTrackParticles"]
    AllVariables += ["ExtrapolatedMuonTrackParticles"]
    AllVariables += ["MuonSpectrometerTrackParticles"]
    
    ## muon container
    AllVariables += ["Muons"]
    AllVariables += ["HLT_xAOD__L2StandAloneMuonContainer_MuonL2SAInfo"]
    AllVariables += ["HLT_xAOD__L2CombinedMuonContainer_MuonL2CBInfo"]
    AllVariables += ["HLT_xAOD__MuonContainer_MuonEFInfo"]
    
    
    AllVariables += ["HLT_xAOD__TrigBphysContainer_L2BMuMuXFex" ]
    AllVariables += ["HLT_xAOD__TrigBphysContainer_EFBMuMuXFex" ]
    AllVariables += ["HLT_xAOD__TrigBphysContainer_L2BMuMuFex"  ]
    AllVariables += ["HLT_xAOD__TrigBphysContainer_EFBMuMuFex"  ]
    AllVariables += ["HLT_xAOD__TrigBphysContainer_L2TrackMass" ]
    AllVariables += ["HLT_xAOD__TrigBphysContainer_EFTrackMass" ]
    AllVariables += ["HLT_xAOD__TrigBphysContainer_L2MultiMuFex"]
    AllVariables += ["HLT_xAOD__TrigBphysContainer_EFMultiMuFex"]
    
    
    ## Jpsi candidates 
    StaticContent += ["xAOD::VertexContainer#%s"        % BPHY6_Reco_mumu.OutputVtxContainerName]
    StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY6_Reco_mumu.OutputVtxContainerName]
    
    if isSimulation:
        AllVariables += ["TruthEvents","TruthParticles","TruthVertices","MuonTruthParticles"]
    BPHY6SlimmingHelper.AllVariables = AllVariables
    BPHY6SlimmingHelper.StaticContent = StaticContent
    BPHY6ItemList = BPHY6SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_BPHY6", ItemList=BPHY6ItemList, AcceptAlgs=["BPHY6Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_BPHY6", AcceptAlgs=["BPHY6Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))
    acc.printConfig(withDetails=True, summariseProps=True, onlyComponents = [], printDefaults=True, printComponentsOnly=False)
    return acc
