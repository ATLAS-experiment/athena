# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#====================================================================
# BPHY10.py
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

BPHYDerivationName = "BPHY10"
streamName = "StreamDAOD_BPHY10"

def BPHY10Cfg(flags):
    from DerivationFrameworkBPhys.commonBPHYMethodsCfg import (BPHY_V0ToolCfg,  BPHY_InDetDetailedTrackSelectorToolCfg, BPHY_VertexPointEstimatorCfg, BPHY_TrkVKalVrtFitterCfg)
    from JpsiUpsilonTools.JpsiUpsilonToolsConfig import PrimaryVertexRefittingToolCfg

    acc = ComponentAccumulator()
    isSimulation = flags.Input.isMC
    BPHY10_AugOriginalCounts = CompFactory.DerivationFramework.AugOriginalCounts(
                              name = "BPHY10_AugOriginalCounts",
                              VertexContainer = "PrimaryVertices",
                              TrackContainer = "InDetTrackParticles" )
    V0Tools = acc.popToolsAndMerge(BPHY_V0ToolCfg(flags, BPHYDerivationName))
    vkalvrt = acc.popToolsAndMerge(BPHY_TrkVKalVrtFitterCfg(flags, BPHYDerivationName))        # VKalVrt vertex fitter
    acc.addPublicTool(vkalvrt)
    acc.addPublicTool(V0Tools)
    trackselect = acc.popToolsAndMerge(BPHY_InDetDetailedTrackSelectorToolCfg(flags, BPHYDerivationName))
    acc.addPublicTool(trackselect)
    vpest = acc.popToolsAndMerge(BPHY_VertexPointEstimatorCfg(flags, BPHYDerivationName))
    acc.addPublicTool(vpest)
    BPHY10JpsiFinder = CompFactory.Analysis.JpsiFinder(
                              name                        = "BPHY10JpsiFinder",
                              muAndMu                     = True,
                              muAndTrack                  = False,
                              TrackAndTrack               = False,
                              assumeDiMuons               = True,
                              invMassUpper                = 4000.0,
                              invMassLower                = 2600.0,
                              Chi2Cut                     = 200.,
                              oppChargesOnly              = True,
                              combOnly                    = True,
                              atLeastOneComb              = False,
                              useCombinedMeasurement      = False, # Only takes effect if combOnly=True
                              muonCollectionKey           = "Muons",
                              TrackParticleCollection     = "InDetTrackParticles",
                              V0VertexFitterTool          = None,             # V0 vertex fitter
                              useV0Fitter                 = False,                   # if False a TrkVertexFitterTool will be used
                              TrkVertexFitterTool         = vkalvrt,        # VKalVrt vertex fitter
                              TrackSelectorTool           = trackselect,
                              VertexPointEstimator        = vpest,
                              useMCPCuts                  = False)

    BPHY10JpsiSelectAndWrite   = CompFactory.DerivationFramework.Reco_Vertex(
                              name                   = "BPHY10JpsiSelectAndWrite",
                              VertexSearchTool       = BPHY10JpsiFinder,
                              OutputVtxContainerName = "BPHY10JpsiCandidates",
                              PVContainerName        = "PrimaryVertices",
                              V0Tools                = V0Tools,
                              PVRefitter             = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
                              RefPVContainerName     = "SHOULDNOTBEUSED",
                              DoVertexType = 1)
    BPHY10_Select_Jpsi2mumu = CompFactory.DerivationFramework.Select_onia2mumu(
                              name                  = "BPHY10_Select_Jpsi2mumu",
                              HypothesisName        = "Jpsi",
                              InputVtxContainerName = "BPHY10JpsiCandidates",
                              V0Tools               = V0Tools,
                              VtxMassHypo           = 3096.916,
                              MassMin               = 2600.0,
                              MassMax               = 4000.0,
                              Chi2Max               = 200,
                              DoVertexType =1)

    BPHY10BdJpsiKst = CompFactory.Analysis.JpsiPlus2Tracks(
                             name                    = "BPHY10BdJpsiKst",
                             kaonkaonHypothesis      = False,
                             pionpionHypothesis      = False,
                             kaonpionHypothesis      = True,
                             trkThresholdPt          = 500.0,
                             trkMaxEta           = 3.0,
                             BThresholdPt            = 5000.,
                             BMassLower              = 4300.0,
                             BMassUpper          = 6300.0,
                             JpsiContainerKey        = "BPHY10JpsiCandidates",
                             TrackParticleCollection = "InDetTrackParticles",
                             ExcludeCrossJpsiTracks  = False,   #setting this to False rejects the muons from J/psi candidate
                             TrkVertexFitterTool     = vkalvrt,
                             TrackSelectorTool       = trackselect,
                             UseMassConstraint       = True,
                             Chi2Cut                 = 10.0,
                             DiTrackPt               = 500.,
                             TrkQuadrupletMassLower  = 3500.0,
                             TrkQuadrupletMassUpper  = 6800.0,
                             FinalDiTrackPt          = 500.
                             )
    BPHY10V0ContainerName = "BPHY10RecoV0Candidates"
    BPHY10KshortContainerName = "BPHY10RecoKshortCandidates"
    BPHY10LambdaContainerName = "BPHY10RecoLambdaCandidates"
    BPHY10LambdabarContainerName = "BPHY10RecoLambdabarCandidates"

    BPHY10BdKstSelectAndWrite  = CompFactory.DerivationFramework.Reco_Vertex(
                                    name                   = "BPHY10BdKstSelectAndWrite",
                                    VertexSearchTool     = BPHY10BdJpsiKst,
                                    OutputVtxContainerName = "BPHY10BdJpsiKstCandidates",
                                    PVContainerName        = "PrimaryVertices",
                                    V0Tools                = V0Tools,
                                    PVRefitter             = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
                                    RefPVContainerName     = "BPHY10RefittedPrimaryVertices1",
                                    RefitPV                = True,
                                    MaxPVrefit             = 10000,
                                    DoVertexType = 7)

    BPHY10_Select_Bd2JpsiKst = CompFactory.DerivationFramework.Select_onia2mumu(
                                   name                       = "BPHY10_Select_Bd2JpsiKst",
                                   HypothesisName             = "Bd",
                                   InputVtxContainerName      = "BPHY10BdJpsiKstCandidates",
                                   V0Tools                    = V0Tools,
                                   TrkMasses                  = [105.658, 105.658, 493.677, 139.570],
                                   VtxMassHypo                = 5279.6,
                                   MassMin                    = 100.0,      #no mass cuts here
                                   MassMax                    = 100000.0,   #no mass cuts here
                                   Chi2Max                    = 200)

    BPHY10_Select_Bd2JpsiKstbar = CompFactory.DerivationFramework.Select_onia2mumu(
                                   name                       = "BPHY10_Select_Bd2JpsiKstbar",
                                   HypothesisName             = "Bdbar",
                                   InputVtxContainerName      = "BPHY10BdJpsiKstCandidates",
                                   V0Tools                    = V0Tools,
                                   TrkMasses                  = [105.658, 105.658, 139.570, 493.677],
                                   VtxMassHypo                = 5279.6,
                                   MassMin                    = 100.0,      #no mass cuts here
                                   MassMax                    = 100000.0,   #no mass cuts here
                                   Chi2Max                    = 200)

    from DerivationFrameworkBPhys.V0ToolConfig import BPHY_Reco_V0FinderCfg
    BPHY10_Reco_V0Finder = acc.popToolsAndMerge(BPHY_Reco_V0FinderCfg(
        flags, derivation = BPHYDerivationName,
        V0ContainerName = BPHY10V0ContainerName,
        KshortContainerName = BPHY10KshortContainerName,
        LambdaContainerName = BPHY10LambdaContainerName,
        LambdabarContainerName = BPHY10LambdabarContainerName,
        CheckVertexContainers = ['BPHY10JpsiCandidates']))

    from TrkConfig.TrkVKalVrtFitterConfig import JpsiV0VertexFitCfg
    JpsiV0VertexFit = acc.popToolsAndMerge(JpsiV0VertexFitCfg(flags))
    acc.addPublicTool(JpsiV0VertexFit)

    BPHY10JpsiKshort  = CompFactory.DerivationFramework.JpsiPlusV0Cascade(
                                  name                    = "BPHY10JpsiKshort",
                                  V0Tools                 = V0Tools,
                                  HypothesisName          = "Bd",
                                  TrkVertexFitterTool     = JpsiV0VertexFit,
                                  V0Hypothesis            = 310,
                                  PVRefitter             = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
                                  JpsiMassLowerCut        = 2800.,
                                  JpsiMassUpperCut        = 4000.,
                                  V0MassLowerCut          = 400.,
                                  V0MassUpperCut          = 600.,
                                  MassLowerCut            = 4300.,
                                  MassUpperCut            = 6300.,
                                  RefitPV                 = True,
                                  RefPVContainerName      = "BPHY10RefittedPrimaryVertices2",
                                  JpsiVertices            = "BPHY10JpsiCandidates",
                                  CascadeVertexCollections= ["BPHY10JpsiKshortCascadeSV2", "BPHY10JpsiKshortCascadeSV1"],
                                  V0Vertices              = BPHY10V0ContainerName)

    BPHY10JpsiLambda   = CompFactory.DerivationFramework.JpsiPlusV0Cascade(
                                  name                    = "BPHY10JpsiLambda",
                                  V0Tools                 = V0Tools,
                                  HypothesisName          = "Lambda_b",
                                  TrkVertexFitterTool     = JpsiV0VertexFit,
                                  PVRefitter             = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
                                  V0Hypothesis            = 3122,
                                  JpsiMassLowerCut        = 2800.,
                                  JpsiMassUpperCut        = 4000.,
                                  V0MassLowerCut          = 1050.,
                                  V0MassUpperCut          = 1250.,
                                  MassLowerCut            = 4600.,
                                  MassUpperCut            = 6600.,
                                  RefitPV                 = True,
                                  RefPVContainerName      = "BPHY10RefittedPrimaryVertices3",
                                  JpsiVertices            = "BPHY10JpsiCandidates",
                                  CascadeVertexCollections= ["BPHY10JpsiLambdaCascadeSV2", "BPHY10JpsiLambdaCascadeSV1"],
                                  V0Vertices              = BPHY10V0ContainerName)

    BPHY10JpsiLambdabar         = CompFactory.DerivationFramework.JpsiPlusV0Cascade(
                                  name                    = "BPHY10JpsiLambdabar",
                                  HypothesisName          = "Lambda_bbar",
                                  V0Tools                 = V0Tools,
                                  TrkVertexFitterTool     = JpsiV0VertexFit,
                                  PVRefitter             = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
                                  V0Hypothesis            = -3122,
                                  JpsiMassLowerCut        = 2800.,
                                  JpsiMassUpperCut        = 4000.,
                                  V0MassLowerCut          = 1050.,
                                  V0MassUpperCut          = 1250.,
                                  MassLowerCut            = 4600.,
                                  MassUpperCut            = 6600.,
                                  RefitPV                 = True,
                                  RefPVContainerName      = "BPHY10RefittedPrimaryVertices4",
                                  JpsiVertices            = "BPHY10JpsiCandidates",
                                  CascadeVertexCollections= ["BPHY10JpsiLambdabarCascadeSV2", "BPHY10JpsiLambdabarCascadeSV1"],
                                  V0Vertices              = BPHY10V0ContainerName)

    CascadeCollections = []
    CascadeCollections += BPHY10JpsiKshort.CascadeVertexCollections
    CascadeCollections += BPHY10JpsiLambda.CascadeVertexCollections
    CascadeCollections += BPHY10JpsiLambdabar.CascadeVertexCollections

    if not isSimulation: #Only Skim Data
       BPHY10_SelectBdJpsiKstEvent = CompFactory.DerivationFramework.xAODStringSkimmingTool(
                    name = "BPHY10_SelectBdJpsiKstEvent",
                    expression = "(count(BPHY10BdJpsiKstCandidates.passed_Bd > 0) + count(BPHY10BdJpsiKstCandidates.passed_Bdbar > 0)) >0")

       BPHY10_cascadeCheck = CompFactory.DerivationFramework.AnyVertexSkimmingTool("BPHY10_AnyVertexSkimmingTool",
                                                                        VertexContainerNames =CascadeCollections,
                                                                        UseHandles = True )
       BPHY10SkimmingOR = CompFactory.DerivationFramework.FilterCombinationOR(
                                "BPHY10SkimmingOR",
                                FilterList = [BPHY10_cascadeCheck, BPHY10_SelectBdJpsiKstEvent])
       acc.addPublicTool(BPHY10_cascadeCheck)
       acc.addPublicTool(BPHY10_SelectBdJpsiKstEvent)
       acc.addPublicTool(BPHY10SkimmingOR)


    augTools = [BPHY10JpsiSelectAndWrite,  BPHY10_Select_Jpsi2mumu,
                          BPHY10BdKstSelectAndWrite, BPHY10_Select_Bd2JpsiKst, BPHY10_Select_Bd2JpsiKstbar,
                          BPHY10_Reco_V0Finder, BPHY10JpsiKshort, BPHY10JpsiLambda, BPHY10JpsiLambdabar,
                          BPHY10_AugOriginalCounts]
    for t in  augTools : acc.addPublicTool(t)
    #from AthenaCommon.Constants import DEBUG
    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel("BPHY10Kernel",
                                                     AugmentationTools = augTools,
                                                     #OutputLevel = DEBUG,
                                                     #Only skim if not MC
                                                     SkimmingTools     = [BPHY10SkimmingOR] if not isSimulation else [],
                                                     ThinningTools     = []))
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    BPHY10SlimmingHelper = SlimmingHelper("BPHY10SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
    from DerivationFrameworkBPhys.commonBPHYMethodsCfg import getDefaultAllVariables
    AllVariables  = getDefaultAllVariables()
    StaticContent = []

    # Needed for trigger objects
    BPHY10SlimmingHelper.IncludeMuonTriggerContent  = True
    BPHY10SlimmingHelper.IncludeBPhysTriggerContent = True

    ## primary vertices
    AllVariables  += ["PrimaryVertices"]

    for x in range(1,5):
       StaticContent += ["xAOD::VertexContainer#BPHY10RefittedPrimaryVertices%s"   %     str(x)]
       StaticContent += ["xAOD::VertexAuxContainer#BPHY10RefittedPrimaryVertices%sAux." % str(x)]

    ## ID track particles
    AllVariables += ["InDetTrackParticles"]

    ## combined / extrapolated muon track particles
    ## (note: for tagged muons there is no extra TrackParticle collection since the ID tracks
    ##        are store in InDetTrackParticles collection)
    AllVariables += ["CombinedMuonTrackParticles"]
    AllVariables += ["ExtrapolatedMuonTrackParticles"]

    ## muon container
    AllVariables += ["Muons", "MuonsLRT"]


    ## Jpsi candidates
    StaticContent += ["xAOD::VertexContainer#%s"        %                 BPHY10JpsiSelectAndWrite.OutputVtxContainerName]
    ## we have to disable vxTrackAtVertex branch since it is not xAOD compatible
    StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY10JpsiSelectAndWrite.OutputVtxContainerName]

    StaticContent += ["xAOD::VertexContainer#%s"        %                 BPHY10BdKstSelectAndWrite.OutputVtxContainerName]
    StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY10BdKstSelectAndWrite.OutputVtxContainerName]

    StaticContent += ["xAOD::VertexContainer#%s"        %                 BPHY10V0ContainerName]
    StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY10V0ContainerName]
    StaticContent += ["xAOD::VertexContainer#%s"        %                 BPHY10KshortContainerName]
    StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY10KshortContainerName]
    StaticContent += ["xAOD::VertexContainer#%s"        %                 BPHY10LambdaContainerName]
    StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY10LambdaContainerName]
    StaticContent += ["xAOD::VertexContainer#%s"        %                 BPHY10LambdabarContainerName]
    StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY10LambdabarContainerName]

    for cascades in CascadeCollections:
       StaticContent += ["xAOD::VertexContainer#%s"   %     cascades]
       StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % cascades]

    # Tagging information (in addition to that already requested by usual algorithms)
    AllVariables += ["GSFTrackParticles", "MuonSpectrometerTrackParticles" ]

    # Truth information for MC only
    if isSimulation:
        AllVariables += ["TruthEvents","TruthParticles","TruthVertices","MuonTruthParticles"]

    AllVariables = list(set(AllVariables)) # remove duplicates

    BPHY10SlimmingHelper.AllVariables = AllVariables
    BPHY10SlimmingHelper.StaticContent = StaticContent
    BPHY10SlimmingHelper.SmartCollections = []
    BPHY10ItemList = BPHY10SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_BPHY10", ItemList=BPHY10ItemList, AcceptAlgs=["BPHY10Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_BPHY10", AcceptAlgs=["BPHY10Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))
    acc.printConfig(withDetails=True, summariseProps=True, onlyComponents = [], printDefaults=True, printComponentsOnly=False)
    return acc
