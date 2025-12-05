# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#====================================================================
# BPHY22.py
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory


BPHYDerivationName = "BPHY22"
streamName = "StreamDAOD_BPHY22"

def BPHY22Cfg(flags):
    from AthenaServices.PartPropSvcConfig import PartPropSvcCfg
    from DerivationFrameworkBPhys.commonBPHYMethodsCfg import (
        BPHY_V0ToolCfg, BPHY_InDetDetailedTrackSelectorToolCfg,
        BPHY_VertexPointEstimatorCfg, BPHY_TrkVKalVrtFitterCfg,
        AugOriginalCountsCfg)
    from JpsiUpsilonTools.JpsiUpsilonToolsConfig import PrimaryVertexRefittingToolCfg
    acc = ComponentAccumulator()
    acc.getPrimaryAndMerge(PartPropSvcCfg(flags))
    isSimulation = flags.Input.isMC
    V0Tools = acc.popToolsAndMerge(BPHY_V0ToolCfg(flags, BPHYDerivationName))
    vkalvrt = acc.popToolsAndMerge(BPHY_TrkVKalVrtFitterCfg(flags, BPHYDerivationName))        # VKalVrt vertex fitter
    acc.addPublicTool(vkalvrt)
    acc.addPublicTool(V0Tools)
    trackselect = acc.popToolsAndMerge(BPHY_InDetDetailedTrackSelectorToolCfg(flags, BPHYDerivationName))
    acc.addPublicTool(trackselect)
    vpest = acc.popToolsAndMerge(BPHY_VertexPointEstimatorCfg(flags, BPHYDerivationName))
    acc.addPublicTool(vpest)
    PVrefit = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags))
    acc.addPublicTool(PVrefit)

    BPHY22MuPiFinder = CompFactory.Analysis.JpsiFinder(
          name                       = "BPHY22MuPiFinder",
          muAndMu                    = False,
          muAndTrack                 = True,  #need doTagAndProbe flag
          TrackAndTrack              = False,
          assumeDiMuons              = False,
          muonThresholdPt            = 2700,
          trackThresholdPt           = 250.0, # MeV
          invMassUpper               = 8200.0,
          invMassLower               = 200.0,
          Chi2Cut                    = 10.,
          oppChargesOnly             = False,
          allChargeCombinations      = True,
          atLeastOneComb             = False, # True by default
          useCombinedMeasurement     = False, # Only takes effect if combOnly=True
          muonCollectionKey          = "Muons",
          TrackParticleCollection    = "InDetTrackParticles",
          V0VertexFitterTool         = None,             # V0 vertex fitter
          useV0Fitter                = False,                   # if False a TrkVertexFitterTool will be used
          TrkVertexFitterTool        = vkalvrt,        # VKalVrt vertex fitter
          TrackSelectorTool          = trackselect,
          VertexPointEstimator       = vpest,
          useMCPCuts                 = False,
          doTagAndProbe              = True, #won't work with all/same charges combs
          forceTagAndProbe           = True) #force T&P to work with any charges combs

    BPHY22MuPiSelectAndWrite = CompFactory.DerivationFramework.Reco_Vertex(
              name                   = "BPHY22MuPiSelectAndWrite",
              VertexSearchTool       = BPHY22MuPiFinder,
              OutputVtxContainerName = "BPHY22MuPiCandidates",
              V0Tools                = V0Tools,
              PVRefitter             = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
              PVContainerName        = "PrimaryVertices",
              RefPVContainerName     = "SHOULDNOTBEUSED")


    BPHY22DiTrkFinder = CompFactory.Analysis.JpsiFinder(
           name                       = "BPHY22DiTrkFinder",
           muAndMu                    = False,
           muAndTrack                 = False,
           TrackAndTrack              = True,
           assumeDiMuons              = False,    # If true, will assume dimu hypothesis and use PDG value for mu mass
           trackThresholdPt           = 900,
           invMassUpper               = 2100.0,
           invMassLower               = 275,
           Chi2Cut                    = 20., #chi2
           oppChargesOnly             = True,
           atLeastOneComb             = False,
           useCombinedMeasurement     = False, # Only takes effect if combOnly=True
           muonCollectionKey          = "Muons",
           TrackParticleCollection    = "InDetTrackParticles",
           V0VertexFitterTool         = None,             # V0 vertex fitter
           useV0Fitter                = False,                   # if False a TrkVertexFitterTool will be used
           TrkVertexFitterTool        = vkalvrt,        # VKalVrt vertex fitter
           TrackSelectorTool          = trackselect,
           VertexPointEstimator       = vpest,
           useMCPCuts                 = False,
           track1Mass                 = 139.571, # Not very important, only used to calculate inv. mass cut, leave it loose here
           track2Mass                 = 139.571)

    BPHY22DiTrkSelectAndWrite = CompFactory.DerivationFramework.Reco_Vertex(
           name                   = "BPHY22DiTrkSelectAndWrite",
           VertexSearchTool       = BPHY22DiTrkFinder,
           OutputVtxContainerName = "BPHY22DiTrkCandidates",
           PVContainerName        = "PrimaryVertices",
           V0Tools                = V0Tools,
           PVRefitter             = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
           RefPVContainerName     = "SHOULDNOTBEUSED",
           CheckCollections       = True,
           CheckVertexContainers  = ['BPHY22MuPiCandidates'])


    BMuDstVertexFit = acc.popToolsAndMerge(BPHY_TrkVKalVrtFitterCfg(flags, BPHYDerivationName, CascadeCnstPrecision = 1e-6))
    
    BPHY22MuDpst = CompFactory.DerivationFramework.MuPlusDpstCascade(
        name                     = "BPHY22MuDpst",
        HypothesisName           = "B",
        TrkVertexFitterTool      = BMuDstVertexFit,
        DxHypothesis             = 421, # MC PID for D0
        ApplyD0MassConstraint    = True,
        MuPiMassLowerCut         = 200.,
        MuPiMassUpperCut         = 8200.,
        V0Tools                  = V0Tools,
        PVRefitter               = PVrefit,
        D0MassLowerCut           = 1864.83 - 200.,
        D0MassUpperCut           = 1864.83 + 200.,
        DstMassLowerCut          = 2010.26 - 300.,
        DstMassUpperCut          = 2010.26 + 300.,
        DstMassUpperCutAft       = 2010.26 + 25., #mass cut after cascade fit
        MassLowerCut             = 0.,
        MassUpperCut             = 12500.,
        Chi2Cut                  = 5, #chi2/ndf
        RefitPV                  = True,
        RefPVContainerName       = "BPHY22RefittedPrimaryVertices",
        MuPiVertices             = "BPHY22MuPiCandidates",
        CascadeVertexCollections = ["BMuDpstCascadeSV2", "BMuDpstCascadeSV1"],
        D0Vertices               = "BPHY22DiTrkCandidates",
        DoVertexType             = 15 )

    BPHY22Dh3Finder = CompFactory.Analysis.JpsiPlus1Track(
        name                    = "BPHY22Dh3Finder",
        pionHypothesis          = True,     #false by default
        kaonHypothesis          = False,    #true by default
        trkThresholdPt          = 900.0,
        trkMaxEta               = 2.7, # is this value fine?? default would be 102.5
        BThresholdPt            = 2000.0,
        BMassUpper              = 2100.0, # What is this??
        BMassLower              = 500.0,
        TrkDeltaZ               = 20.,
        TrkTrippletMassUpper    = 2200, #2100
        TrkTrippletMassLower    = 500,
        TrkQuadrupletPt         = 2000,
        JpsiContainerKey        = "BPHY22DiTrkCandidates",
        TrackParticleCollection = "InDetTrackParticles",
        MuonsUsedInJpsi         = "NONE", # ?
        ExcludeCrossJpsiTracks  = False,
        TrkVertexFitterTool     = vkalvrt,
        TrackSelectorTool       = trackselect,
        UseMassConstraint       = False,
        Chi2Cut                 = 7) #Cut on chi2/Ndeg_of_freedom 5->7

    BPHY22Dh3SelectAndWrite = CompFactory.DerivationFramework.Reco_Vertex(
           name                   = "BPHY22Dh3SelectAndWrite",
           VertexSearchTool     = BPHY22Dh3Finder,
           V0Tools                = V0Tools,
           PVRefitter             = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
           OutputVtxContainerName = "BPHY22Dh3Candidates",
           PVContainerName        = "PrimaryVertices",
           RefPVContainerName     = "SHOULDNOTBEUSED",
           MaxPVrefit             = 1000)

    BMuDxVertexFit = acc.popToolsAndMerge(BPHY_TrkVKalVrtFitterCfg(flags, BPHYDerivationName+"BMuDxVertexFit", CascadeCnstPrecision = 1e-6))

    BPHY22MuDs = CompFactory.DerivationFramework.MuPlusDsCascade(
           name                        = "BPHY22MuDs",
           HypothesisName              = "B",
           TrkVertexFitterTool         = BMuDxVertexFit,
               V0Tools                 = V0Tools,
               PVRefitter               = PVrefit,
           DxHypothesis                = 431,
           ApplyDxMassConstraint       = False,
           DxMassLowerCut              = 1968.28 - 300.,
           DxMassUpperCut              = 1968.28 + 200.,
           MassLowerCut                = 1000,
           MassUpperCut                = 12500,
           Chi2Cut                     = 10,
           RefitPV                     = True,
           combOnly                    = True,
           TrackSelectorTool           = trackselect,
           useMCPCuts                  = False,
           muonThresholdPt             = 2700,
           muonCollectionKey           = "Muons",
           useCombinedMeasurement      = False, # Only takes effect if combOnly=True
           RefPVContainerName          = "BPHY22RefittedPrimaryVertices",
           CascadeVertexCollections    = ["BMuDsCascadeSV2", "BMuDsCascadeSV1"],
           DxVertices                  = "BPHY22Dh3Candidates")

    BPHY22MuDp = CompFactory.DerivationFramework.MuPlusDsCascade(
          name                        = "BPHY22MuDp",
          HypothesisName              = "B",
          TrkVertexFitterTool         = BMuDxVertexFit,
              V0Tools                 = V0Tools,
              PVRefitter               = PVrefit,
          DxHypothesis                = 411,
          ApplyDxMassConstraint       = False,
          DxMassLowerCut              = 1869.59 - 180.,
          DxMassUpperCut              = 1869.59 + 250.,
          MassLowerCut                = 1000,
          MassUpperCut                = 12500,
          Chi2Cut                     = 10,
          RefitPV                     = True,
          combOnly                    = True,
          TrackSelectorTool           = trackselect,
          useMCPCuts                  = False,
          muonThresholdPt             = 2700,
          muonCollectionKey           = "Muons",
          useCombinedMeasurement      = False, # Only takes effect if combOnly=True
          RefPVContainerName          = "BPHY22RefittedPrimaryVertices",
          CascadeVertexCollections    = ["BMuDpCascadeSV2", "BMuDpCascadeSV1"],
          DxVertices                  = "BPHY22Dh3Candidates")

    BPHY22MuLambdaC = CompFactory.DerivationFramework.MuPlusDsCascade(
          name                        = "BPHY22MuLambdaC",
          HypothesisName              = "B",
          TrkVertexFitterTool         = BMuDxVertexFit,
              V0Tools                 = V0Tools,
              PVRefitter               = PVrefit,
          DxHypothesis                = 4122,
          ApplyDxMassConstraint       = False,
          DxMassLowerCut              = 2286.46 - 200,
          DxMassUpperCut              = 2286.46 + 220,
          MassLowerCut                = 1000,
          MassUpperCut                = 12500,
          Chi2Cut                     = 10,
          RefitPV                     = True,
          combOnly                    = True,
          TrackSelectorTool           = trackselect,
          useMCPCuts                  = False,
          muonThresholdPt             = 2700,
          muonCollectionKey           = "Muons",
          useCombinedMeasurement      = False, # Only takes effect if combOnly=True
          RefPVContainerName          = "BPHY22RefittedPrimaryVertices",
          CascadeVertexCollections    = ["BMuLambdaCCascadeSV2", "BMuLambdaCCascadeSV1"],
          DxVertices                  = "BPHY22Dh3Candidates")

    CascadeCollections = []
    CascadeCollections += BPHY22MuDpst.CascadeVertexCollections
    CascadeCollections += BPHY22MuDp.CascadeVertexCollections
    CascadeCollections += BPHY22MuDs.CascadeVertexCollections
    CascadeCollections += BPHY22MuLambdaC.CascadeVertexCollections

    BPHY22_AugOriginalCounts = acc.popToolsAndMerge(
        AugOriginalCountsCfg(flags, name = "BPHY22_AugOriginalCounts"))
    
    if not isSimulation: #Only Skim Data
        BPHY22_SelectBMuDxEvent = CompFactory.DerivationFramework.AnyVertexSkimmingTool("BPHY22_AnyVertexSkimmingTool",
                                                                        VertexContainerNames =CascadeCollections,
                                                                        UseHandles = True )
        acc.addPublicTool(BPHY22_SelectBMuDxEvent)

        #====================================================================
        # Make event selection based on an OR of the input skimming tools
        #====================================================================
           
        BPHY22SkimmingOR = CompFactory.DerivationFramework.FilterCombinationOR(
            "BPHY22SkimmingOR",
            FilterList = [BPHY22_SelectBMuDxEvent] )
        acc.addPublicTool(BPHY22SkimmingOR)

    augTools = [BPHY22MuPiSelectAndWrite, #BPHY22_Select_MuPi,
                         BPHY22DiTrkSelectAndWrite, #BPHY22_Select_D0, BPHY22_Select_D0b,
                         BPHY22Dh3SelectAndWrite, #BPHY22_Select_Ds, BPHY22_Select_Dp, BPHY22_Select_Dm, BPHY22_Select_LambdaCp, BPHY22_Select_LambdaCm,
                         BPHY22MuDpst,
                         BPHY22MuDs,
                         BPHY22MuDp,
                         BPHY22MuLambdaC,
                         BPHY22_AugOriginalCounts]
    for t in  augTools : acc.addPublicTool(t)
    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel("BPHY22Kernel",
                                                    AugmentationTools = augTools,
                                                    #Only skim if not MC
                                                    SkimmingTools     = [BPHY22SkimmingOR] if not isSimulation else [],
                                                    ThinningTools     = []))

    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    BPHY22SlimmingHelper = SlimmingHelper("BPHY22SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)

    # Needed for trigger objects
    BPHY22SlimmingHelper.IncludeMuonTriggerContent  = True
    BPHY22SlimmingHelper.IncludeBPhysTriggerContent = True
    from DerivationFrameworkBPhys.commonBPHYMethodsCfg import getDefaultAllVariables
    AllVariables  = getDefaultAllVariables()
    StaticContent = []
    ## primary vertices
    AllVariables  += ["PrimaryVertices"]
    StaticContent += ["xAOD::VertexContainer#BPHY22RefittedPrimaryVertices"]
    StaticContent += ["xAOD::VertexAuxContainer#BPHY22RefittedPrimaryVerticesAux."]
    
    ## ID track particles
    AllVariables += ["InDetTrackParticles"]
    
    ## combined / extrapolated muon track particles
    ## (note: for tagged muons there is no extra TrackParticle collection since the ID tracks
    ##        are store in InDetTrackParticles collection)
    AllVariables += ["CombinedMuonTrackParticles"]
    AllVariables += ["ExtrapolatedMuonTrackParticles"]
    
    ## muon container
    AllVariables += ["Muons"]
    
    ## Jpsi candidates
    StaticContent += ["xAOD::VertexContainer#%s"        %                 BPHY22MuPiSelectAndWrite.OutputVtxContainerName]
    ## we have to disable vxTrackAtVertex branch since it is not xAOD compatible
    StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY22MuPiSelectAndWrite.OutputVtxContainerName]
    
    ## B+>mu D_(s)+/-, mu D*+/- and mu Lambda_c+/- candidates
    for cascades in CascadeCollections:
       StaticContent += ["xAOD::VertexContainer#%s"   %     cascades]
       StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % cascades]
    
    # Tagging information (in addition to that already requested by usual algorithms)
    AllVariables += ["MuonSpectrometerTrackParticles" ]
    
    # Truth information for MC only
    if isSimulation:
        AllVariables += ["TruthEvents","TruthParticles","TruthVertices","MuonTruthParticles"]
    
    AllVariables = list(set(AllVariables)) # remove duplicates
    BPHY22SlimmingHelper.AllVariables = AllVariables
    BPHY22SlimmingHelper.StaticContent = StaticContent
    BPHY22ItemList = BPHY22SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_BPHY22", ItemList=BPHY22ItemList, AcceptAlgs=["BPHY22Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_BPHY22", AcceptAlgs=["BPHY22Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))
    acc.printConfig(withDetails=True, summariseProps=True, onlyComponents = [], printDefaults=True)
    return acc
