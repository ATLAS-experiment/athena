# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#====================================================================
# BPHY3.py
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

BPHYDerivationName = "BPHY3"
streamName = "StreamDAOD_BPHY3"

def BPHY3Cfg(flags):
    from DerivationFrameworkBPhys.commonBPHYMethodsCfg import (BPHY_V0ToolCfg,  BPHY_InDetDetailedTrackSelectorToolCfg, BPHY_VertexPointEstimatorCfg, BPHY_TrkVKalVrtFitterCfg)
    from JpsiUpsilonTools.JpsiUpsilonToolsConfig import PrimaryVertexRefittingToolCfg
    acc = ComponentAccumulator()
    V0Tools = acc.popToolsAndMerge(BPHY_V0ToolCfg(flags, BPHYDerivationName))
    vkalvrt = acc.popToolsAndMerge(BPHY_TrkVKalVrtFitterCfg(flags, BPHYDerivationName))        # VKalVrt vertex fitter
    acc.addPublicTool(vkalvrt)
    acc.addPublicTool(V0Tools)
    trackselect = acc.popToolsAndMerge(BPHY_InDetDetailedTrackSelectorToolCfg(flags, BPHYDerivationName))
    acc.addPublicTool(trackselect)
    vpest = acc.popToolsAndMerge(BPHY_VertexPointEstimatorCfg(flags, BPHYDerivationName))
    acc.addPublicTool(vpest)
    BPHY3JpsiFinder = CompFactory.Analysis.JpsiFinder(
                    name                        = "BPHY3JpsiFinder",
                    muAndMu                     = False,
                    muAndTrack                  = False,
                    TrackAndTrack               = True,
                    assumeDiMuons               = False,    # If true, will assume dimu hypothesis and use PDG value for mu mass
                    invMassUpper                = 10000.0,
                    invMassLower                = 0.0,
                    Chi2Cut                     = 100.,
                    oppChargesOnly                = True,
                    atLeastOneComb              = False,
                    useCombinedMeasurement      = False, # Only takes effect if combOnly=True 
                    muonCollectionKey           = "Muons",
                    TrackParticleCollection     = "InDetTrackParticles",
                    V0VertexFitterTool          = None,             # V0 vertex fitter
                    useV0Fitter                 = False,                   # if False a TrkVertexFitterTool will be used
                    TrkVertexFitterTool         = vkalvrt,        # VKalVrt vertex fitter
                    TrackSelectorTool           = trackselect,
                    VertexPointEstimator        = vpest,
                    useMCPCuts                  = False,
                    track1Mass                  = 139.57, # Not very important, only used to calculate inv. mass cut, leave it loose here
                    track2Mass                  = 139.57)

    BPHY3_Reco_diTrk = CompFactory.DerivationFramework.Reco_Vertex(
                    name                   = "BPHY3_Reco_diTrk",
                    VertexSearchTool             = BPHY3JpsiFinder,
                    OutputVtxContainerName = "BPHY3VertexCandidates",
                    V0Tools                = V0Tools,
                    PVRefitter             = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
                    PVContainerName        = "PrimaryVertices",
                    RefPVContainerName     = "BPHY3RefittedPrimaryVertices")
    BPHY3_Select_PiPi = CompFactory.DerivationFramework.Select_onia2mumu(
                    name                  = "BPHY3_Select_PiPi",
                    HypothesisName        = "PiPi",
                    InputVtxContainerName = "BPHY3VertexCandidates",
                    V0Tools               = V0Tools,
                    TrkMasses             = [139.57,139.57],
                    VtxMassHypo           = 497.614,
                    MassMin               = 300.0,
                    MassMax               = 700.0,
                    Chi2Max               = 20)
    BPHY3_Select_PiK = CompFactory.DerivationFramework.Select_onia2mumu(
                    name                  = "BPHY3_Select_PiK",
                    HypothesisName        = "PiK",
                    InputVtxContainerName = "BPHY3VertexCandidates",
                    V0Tools               = V0Tools,
                    TrkMasses             = [139.57,493.677],
                    VtxMassHypo           = 892.,
                    MassMin               = 0.0,
                    MassMax               = 3500.0,
                    Chi2Max               = 10)
    BPHY3_Select_KPi = CompFactory.DerivationFramework.Select_onia2mumu(
                    name                  = "BPHY3_Select_KPi",
                    HypothesisName        = "KPi",
                    InputVtxContainerName = "BPHY3VertexCandidates",
                    V0Tools               = V0Tools,
                    TrkMasses             = [493.677,139.57],
                    VtxMassHypo           = 892.,
                    MassMin               = 0.0,
                    MassMax               = 3500.0,
                    Chi2Max               = 10)
    BPHY3_Select_KK = CompFactory.DerivationFramework.Select_onia2mumu(
                    name                  = "BPHY3_Select_KK",
                    HypothesisName        = "KK",
                    InputVtxContainerName = "BPHY3VertexCandidates",
                    V0Tools               = V0Tools,
                    TrkMasses             = [493.677,493.677],
                    VtxMassHypo           = 1019.461,
                    MassMin               = 0.0,
                    MassMax               = 1100.0,
                    Chi2Max               = 20)
    BPHY3_Select_PP = CompFactory.DerivationFramework.Select_onia2mumu(
                    name                  = "BPHY3_Select_PP",
                    HypothesisName        = "PP",
                    InputVtxContainerName = "BPHY3VertexCandidates",
                    V0Tools               = V0Tools,
                    TrkMasses             = [938.272,938.272],
                    VtxMassHypo           = 3096.916,
                    MassMin               = 2800.0,
                    MassMax               = 3600.0,
                    Chi2Max               = 1)

    expression = "count(BPHY3VertexCandidates.passed_PiPi) > 0 || count(BPHY3VertexCandidates.passed_KPi) > 0 || count(BPHY3VertexCandidates.passed_PiK) > 0 || count(BPHY3VertexCandidates.passed_KK) > 0 || count(BPHY3VertexCandidates.passed_PP) > 0"
    BPHY3_SelectEvent = CompFactory.DerivationFramework.xAODStringSkimmingTool(name = "BPHY3_SelectEvent",  expression = expression)
    BPHY3Thin_vtxTrk = CompFactory.DerivationFramework.Thin_vtxTrk(
                    name                       = "BPHY3Thin_vtxTrk",
                    TrackParticleContainerName = "InDetTrackParticles",
                    VertexContainerNames       = ["BPHY3VertexCandidates"],
                    StreamName = streamName,
                    PassFlags                  = ["passed_PiPi","passed_KPi","passed_PiK","passed_KK","passed_PP"])
    augCollections=[BPHY3_Reco_diTrk,BPHY3_Select_PiPi,BPHY3_Select_KPi,BPHY3_Select_PiK,BPHY3_Select_KK,BPHY3_Select_PP]
    skimCollections = [BPHY3_SelectEvent]
    BPHY3ThinningTools = [BPHY3Thin_vtxTrk]
    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel(
                    "BPHY3Kernel",
                     AugmentationTools = augCollections,
                     SkimmingTools     = skimCollections,
                     ThinningTools     = BPHY3ThinningTools))

    for t in augCollections +BPHY3ThinningTools +skimCollections : acc.addPublicTool(t)
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    BPHY3SlimmingHelper = SlimmingHelper("BPHY3SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
    from DerivationFrameworkBPhys.commonBPHYMethodsCfg import getDefaultAllVariables
    AllVariables  = getDefaultAllVariables()
    StaticContent = []
    
    ## primary vertices
    AllVariables += ["PrimaryVertices"]
    
    ## ID track particles
    AllVariables += ["InDetTrackParticles"]
    
    ## Vertex candidates 
    StaticContent += ["xAOD::VertexContainer#%s"        % BPHY3_Reco_diTrk.OutputVtxContainerName]
    ## we have to disable vxTrackAtVertex branch since it is not xAOD compatible
    StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY3_Reco_diTrk.OutputVtxContainerName]
    BPHY3SlimmingHelper.AllVariables = AllVariables
    BPHY3SlimmingHelper.StaticContent = StaticContent
    BPHY3ItemList = BPHY3SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_BPHY3", ItemList=BPHY3ItemList, AcceptAlgs=["BPHY3Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_BPHY3", AcceptAlgs=["BPHY3Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))
    acc.printConfig(withDetails=True, summariseProps=True, onlyComponents = [], printDefaults=True)
    return acc
