# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#========================================================================
# HIGG9D1.py for J/psi+bbbar, J/psi+tautau and J/psi+diphoton
# This requires the flag "HIGG9D1" in Derivation_tf.py   
#========================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory
from AthenaCommon.Logging import logging
log_HIGG9D1 = logging.getLogger('HIGG9D1')

streamName = "StreamDAOD_HIGG9D1"

def HIGG9D1KernelCfg(flags, name='HIGG9D1Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for HIGG9D1"""
    acc = ComponentAccumulator()

    doLRT = flags.Tracking.doLargeD0
    if doLRT:
        log_HIGG9D1.info("flags.Tracking.doLargeD0 is True")
    else:
        log_HIGG9D1.info("flags.Tracking.doLargeD0 is False")

    # Adds primary vertex counts and track counts to EventInfo before they are thinned
    HIGG9D1_AugOriginalCounts = CompFactory.DerivationFramework.AugOriginalCounts(
       name              = "HIGG9D1_AugOriginalCounts",
       VertexContainer   = "PrimaryVertices",
       TrackContainer    = "InDetTrackParticles",
       TrackLRTContainer = "InDetLargeD0TrackParticles" if doLRT else "" )
    acc.addPublicTool(HIGG9D1_AugOriginalCounts)

    mainMuonInput = "StdWithLRTMuons" if doLRT else "Muons"
    mainIDInput   = "InDetWithLRTTrackParticles" if doLRT else "InDetTrackParticles"
    if doLRT:
        from DerivationFrameworkLLP.LLPToolsConfig import LRTMuonMergerAlg
        from AthenaConfiguration.Enums import LHCPeriod
        acc.merge(LRTMuonMergerAlg( flags,
                                    PromptMuonLocation    = "Muons",
                                    LRTMuonLocation       = "MuonsLRT",
                                    OutputMuonLocation    = mainMuonInput,
                                    CreateViewCollection  = True,
                                    UseRun3WP = flags.GeoModel.Run is LHCPeriod.Run3 ))
        from DerivationFrameworkInDet.InDetToolsConfig import InDetLRTMergeCfg
        acc.merge(InDetLRTMergeCfg( flags, OutputTrackParticleLocation = mainIDInput ))

    MuonToRelink = [ "Muons", "MuonsLRT" ] if doLRT else []
    TrkToRelink = ["InDetTrackParticles", "InDetLargeD0TrackParticles"] if doLRT else []

    from DerivationFrameworkBPhys.commonBPHYMethodsCfg import (BPHY_V0ToolCfg,  BPHY_InDetDetailedTrackSelectorToolCfg, BPHY_VertexPointEstimatorCfg, BPHY_TrkVKalVrtFitterCfg)
    V0Tools = acc.popToolsAndMerge(BPHY_V0ToolCfg(flags, "HIGG9D1"))
    vkalvrt = acc.popToolsAndMerge(BPHY_TrkVKalVrtFitterCfg(flags, "HIGG9D1"))
    acc.addPublicTool(vkalvrt)
    acc.addPublicTool(V0Tools)
    TrackSelector = acc.popToolsAndMerge(BPHY_InDetDetailedTrackSelectorToolCfg(flags, "HIGG9D1"))
    acc.addPublicTool(TrackSelector)
    vpest = acc.popToolsAndMerge(BPHY_VertexPointEstimatorCfg(flags, "HIGG9D1"))
    acc.addPublicTool(vpest)

    from JpsiUpsilonTools.JpsiUpsilonToolsConfig import PrimaryVertexRefittingToolCfg
    pvRefitter = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags))
    acc.addPublicTool(pvRefitter)

    HIGG9D1JpsiFinder = CompFactory.Analysis.JpsiFinder(
        name                        = "HIGG9D1JpsiFinder",
        muAndMu                     = True,
        muAndTrack                  = False,
        TrackAndTrack               = False,
        assumeDiMuons               = True,  # If true, will assume dimu hypothesis and use PDG value for mu mass
        trackThresholdPt            = 2400.,
        invMassLower                = 2600.0,
        invMassUpper                = 3500.0,
        Chi2Cut                     = 20.,
        oppChargesOnly              = True,
        atLeastOneComb              = True,
        useCombinedMeasurement      = False, # Only takes effect if combOnly=True    
        muonCollectionKey           = mainMuonInput,
        TrackParticleCollection     = mainIDInput,
        V0VertexFitterTool          = None, # V0 vertex fitter
        useV0Fitter                 = False, # if False a TrkVertexFitterTool will be used
        TrkVertexFitterTool         = vkalvrt, # VKalVrt vertex fitter
        TrackSelectorTool           = TrackSelector,
        VertexPointEstimator        = vpest,
        useMCPCuts                  = False )
    acc.addPublicTool(HIGG9D1JpsiFinder)

    HIGG9D1UpsiFinder = CompFactory.Analysis.JpsiFinder(
        name                        = "HIGG9D1UpsiFinder",
        muAndMu                     = True,
        muAndTrack                  = False,
        TrackAndTrack               = False,
        assumeDiMuons               = True, # If true, will assume dimu hypothesis and use PDG value for mu mass
        trackThresholdPt            = 2400.,
        invMassLower                = 8900.0,
        invMassUpper                = 9900.0,
        Chi2Cut                     = 20.,
        oppChargesOnly              = True,
        atLeastOneComb              = True,
        useCombinedMeasurement      = False, # Only takes effect if combOnly=True    
        muonCollectionKey           = mainMuonInput,
        TrackParticleCollection     = mainIDInput,
        V0VertexFitterTool          = None, # V0 vertex fitter
        useV0Fitter                 = False, # if False a TrkVertexFitterTool will be used
        TrkVertexFitterTool         = vkalvrt, # VKalVrt vertex fitter
        TrackSelectorTool           = TrackSelector,
        VertexPointEstimator        = vpest,
        useMCPCuts                  = False )
    acc.addPublicTool(HIGG9D1UpsiFinder)

    HIGG9D1_Jpsi = CompFactory.DerivationFramework.Reco_Vertex(
        name                   = "HIGG9D1_Jpsi",
        VertexSearchTool       = HIGG9D1JpsiFinder,
        OutputVtxContainerName = "HIGG9D1_JpsiCandidates",
        PVContainerName        = "PrimaryVertices",
        RefPVContainerName     = "HIGG9D1_JpsiRefittedPrimaryVertices",
        RefitPV                = True,
        MaxPVrefit             = 100,
        V0Tools                = V0Tools,
        RelinkTracks           = TrkToRelink,
        RelinkMuons            = MuonToRelink,
        PVRefitter             = pvRefitter,
        DoVertexType           = 7)
    acc.addPublicTool(HIGG9D1_Jpsi)

    HIGG9D1_Upsi = CompFactory.DerivationFramework.Reco_Vertex(
        name                   = "HIGG9D1_Upsi",
        VertexSearchTool       = HIGG9D1UpsiFinder,
        OutputVtxContainerName = "HIGG9D1_UpsiCandidates",
        PVContainerName        = "PrimaryVertices",
        RefPVContainerName     = "HIGG9D1_UpsiRefittedPrimaryVertices",
        RefitPV                = True,
        MaxPVrefit             = 100,
        V0Tools                = V0Tools,
        RelinkTracks           = TrkToRelink,
        RelinkMuons            = MuonToRelink,
        PVRefitter             = pvRefitter,
        DoVertexType           = 7)
    acc.addPublicTool(HIGG9D1_Upsi)

    from InDetConfig.InDetTrackSelectionToolConfig import InDetTrackSelectionTool_Loose_Cfg
    HIGG9D1_isoTrackSelTool = acc.popToolsAndMerge(InDetTrackSelectionTool_Loose_Cfg(
        flags,
        name          = "HIGG9D1_isoTrackSelTool",
        maxZ0SinTheta = 3.0,
        minPt         = 500.))

    from IsolationAlgs.IsoToolsConfig import TrackIsolationToolCfg
    HIGG9D1_TrackIsoTool = acc.popToolsAndMerge(TrackIsolationToolCfg(
        flags,
        name               = "HIGG9D1_TrackIsoTool",
        TrackSelectionTool = HIGG9D1_isoTrackSelTool))
    acc.addPublicTool(HIGG9D1_TrackIsoTool)

    HIGG9D1_JpsiVtxTrkIsoDecor = CompFactory.DerivationFramework.VertexTrackIsolation(
        name                            = "HIGG9D1_JpsiVtxTrkIsoDecor",
        TrackIsoTool                    = HIGG9D1_TrackIsoTool,
        TrackContainer                  = "InDetTrackParticles",
        InputVertexContainer            = "HIGG9D1_JpsiCandidates",
        FixElecExclusion                = False,
        IncludeV0                       = False)
    acc.addPublicTool(HIGG9D1_JpsiVtxTrkIsoDecor)

    HIGG9D1_UpsiVtxTrkIsoDecor = CompFactory.DerivationFramework.VertexTrackIsolation(
        name                            = "HIGG9D1_UpsiVtxTrkIsoDecor",
        TrackIsoTool                    = HIGG9D1_TrackIsoTool,
        TrackContainer                  = "InDetTrackParticles",
        InputVertexContainer            = "HIGG9D1_UpsiCandidates",
        FixElecExclusion                = False,
        IncludeV0                       = False)
    acc.addPublicTool(HIGG9D1_UpsiVtxTrkIsoDecor)

    # New subsequence
    from AthenaCommon.CFElements import seqAND
    acc.addSequence( seqAND("HIGG9D1Sequence") )

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(flags, TriggerListsHelper = kwargs['TriggerListsHelper']), sequenceName="HIGG9D1Sequence")

    ##=================
    ##  onia skimming
    ##=================
    HIGG9D1_onia_skim = CompFactory.DerivationFramework.AnyVertexSkimmingTool(name = "HIGG9D1_onia_skim", VertexContainerNames = ["HIGG9D1_JpsiCandidates", "HIGG9D1_UpsiCandidates"])
    acc.addPublicTool(HIGG9D1_onia_skim)

    HIGG9D1_onia_skimKernel = CompFactory.DerivationFramework.DerivationKernel(
        "HIGG9D1_onia_skimKernel",
        AugmentationTools = [ HIGG9D1_AugOriginalCounts, HIGG9D1_Jpsi, HIGG9D1_Upsi, HIGG9D1_JpsiVtxTrkIsoDecor, HIGG9D1_UpsiVtxTrkIsoDecor ],
        SkimmingTools     = [ HIGG9D1_onia_skim ])
    # Add skimming tool to subsequence
    acc.addEventAlgo(HIGG9D1_onia_skimKernel, sequenceName="HIGG9D1Sequence")

    ## FTAG augmentations - run b-tagging on PFlow jets
    from DerivationFrameworkFlavourTag.FtagDerivationConfig import JetCollectionsBTaggingCfg
    acc.merge(JetCollectionsBTaggingCfg(flags, ["AntiKt4EMPFlowJets"]), sequenceName="HIGG9D1Sequence")

    ##===========================
    ## bb / tautau / yy skimming
    ##===========================

    ## b-jets
    # https://indico.cern.ch/event/273466/contributions/616597/attachments/492353/680556/summary.pdf
    HIGG9D1_smallR_EMPFlow_2j_sel = "count(AntiKt4EMPFlowJets.pt > 18*GeV && abs(AntiKt4EMPFlowJets.eta) < 2.8) >= 2"
    HIGG9D1_smallR_EMPFlow_1j_sel = "count(AntiKt4EMPFlowJets.pt > 33*GeV && abs(AntiKt4EMPFlowJets.eta) < 2.8) >= 1"
    # https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/JetTagging/JetTagPerformanceCalibration/xAODBTaggingEfficiency/Root/BTaggingSelectionTool.cxx
    # fraction_c (0.2), fraction_tau (0.01) and cutvalue (0.844) from TDirectory "GN2v01/AntiKt4EMPFlowJets/FixedCutBEff_77" of
    # /cvmfs/atlas.cern.ch/repo/sw/database/GroupData/xAODBTaggingEfficiency/13TeV/MC20_2024-10-17_GN2v01_v1.root (run-2) and
    # /cvmfs/atlas.cern.ch/repo/sw/database/GroupData/xAODBTaggingEfficiency/13p6TeV/MC23_2024-10-17_GN2v01_v1.root (run-3)
    HIGG9D1_smallR_EMPFlow_1b_sel = "count(AntiKt4EMPFlowJets.pt > 18*GeV && abs(AntiKt4EMPFlowJets.eta) < 2.8 && log(BTagging_AntiKt4EMPFlow.GN2v01_pb/(0.2*BTagging_AntiKt4EMPFlow.GN2v01_pc + (1.-0.2-0.01)*BTagging_AntiKt4EMPFlow.GN2v01_pu + 0.01*BTagging_AntiKt4EMPFlow.GN2v01_ptau))>=0.844) >= 1"

    HIGG9D1_bjet_sel = "%s && %s && %s" % (HIGG9D1_smallR_EMPFlow_2j_sel, HIGG9D1_smallR_EMPFlow_1j_sel, HIGG9D1_smallR_EMPFlow_1b_sel)
    HIGG9D1_bb_skim = CompFactory.DerivationFramework.xAODStringSkimmingTool(name = "HIGG9D1_bb_skim", expression = HIGG9D1_bjet_sel)
    acc.addPublicTool(HIGG9D1_bb_skim)

    ## taus
    HIGG9D1_tauTrks = '(TauJets.nTracks + TauJets.nTracksIsolation >= 1 && TauJets.nTracks + TauJets.nTracksIsolation <= 8)'
    HIGG9D1_tauLead = '(TauJets.pt > 23.0*GeV || TauJets.ptFinalCalib > 23.0*GeV)'
    HIGG9D1_tauSubl = '(TauJets.pt > 18.0*GeV || TauJets.ptFinalCalib > 18.0*GeV)'
    HIGG9D1_tauId   = 'TauJets.DFTauLoose'
    HIGG9D1_tauReq0 = 'count( '+HIGG9D1_tauSubl+' && '+HIGG9D1_tauTrks+' ) >= 2'
    HIGG9D1_tauReq1 = 'count( '+HIGG9D1_tauSubl+' && '+HIGG9D1_tauTrks+' && '+HIGG9D1_tauId+' ) >= 1'
    HIGG9D1_tauReq2 = 'count( '+HIGG9D1_tauLead+' && '+HIGG9D1_tauTrks+' ) >= 1'
    HIGG9D1_tau_sel = "%s && %s && %s" % (HIGG9D1_tauReq0, HIGG9D1_tauReq1, HIGG9D1_tauReq2)

    HIGG9D1_tautau_skim = CompFactory.DerivationFramework.xAODStringSkimmingTool(name = "HIGG9D1_tautau_skim", expression = HIGG9D1_tau_sel)
    acc.addPublicTool(HIGG9D1_tautau_skim)

    ## diphoton
    from  DerivationFrameworkHiggs.SkimmingToolHIGG1Config import SkimmingToolHIGG1Cfg
    HIGG9D1_yy_skim = acc.popToolsAndMerge(SkimmingToolHIGG1Cfg(
        flags,
        name                                    = "HIGG9D1_yy_skim",
        RequireGRL                              = False,
        ReqireLArError                          = True,
        RequireTrigger                          = False,
        RequirePreselection                     = True,
        RequireKinematic                        = False,
        RequireQuality                          = False, # True: "Tight" ID; False: DFCommonPhotonsIsEMLoose
        RequireIsolation                        = False, # dummy
        RequireInvariantMass                    = False,
        IncludeSingleMergedElectronPreselection = False,
        IncludeSingleElectronPreselection       = False,
        IncludeDoubleElectronPreselection       = False,
        IncludeSingleMuonPreselection           = False,
        IncludeDoubleMuonPreselection           = False,
        IncludePhotonDoubleElectronPreselection = False,
        IncludePhotonMergedElectronPreselection = False,
        IncludeHighPtPhotonElectronPreselection = False,
        IncludeDoublePhotonPreselection         = True, # di-photon requirement
        MinimumPhotonPt                         = 4800.0,
        RemoveCrack                             = True,
        MaxEta                                  = 2.47))
    acc.addPublicTool(HIGG9D1_yy_skim)

    # Combine skims with OR
    HIGG9D1_bb_tautau_yy_skim = CompFactory.DerivationFramework.FilterCombinationOR(
        name = "HIGG9D1_bb_tautau_yy_skim",
        FilterList = [HIGG9D1_bb_skim, HIGG9D1_tautau_skim, HIGG9D1_yy_skim])
    acc.addPublicTool(HIGG9D1_bb_tautau_yy_skim)

    # Common calo decoration tools
    from DerivationFrameworkCalo.DerivationFrameworkCaloConfig import (CaloDecoratorKernelCfg, ClusterEnergyPerLayerDecoratorCfg)
    acc.merge(CaloDecoratorKernelCfg(flags), sequenceName="HIGG9D1Sequence")

    # Adding missing cluster energy decorators
    augmentationTools = []
    cluster_sizes = (3,5), (5,7), (7,7)
    for neta, nphi in cluster_sizes:
        cename = "ClusterEnergyPerLayerDecorator_%sx%s" % (neta, nphi)
        ClusterEnergyPerLayerDecorator = acc.popToolsAndMerge( ClusterEnergyPerLayerDecoratorCfg(flags, neta = neta, nphi=nphi, name=cename ))
        acc.addPublicTool(ClusterEnergyPerLayerDecorator)
        augmentationTools.append(ClusterEnergyPerLayerDecorator)

    HIGG9D1_bb_tautau_yy_skimKernel = CompFactory.DerivationFramework.DerivationKernel(
        "HIGG9D1_bb_tautau_yy_skimKernel",
        AugmentationTools = augmentationTools,
        SkimmingTools     = [ HIGG9D1_bb_tautau_yy_skim ])
    # Add skimming tool to subsequence
    acc.addEventAlgo(HIGG9D1_bb_tautau_yy_skimKernel, sequenceName="HIGG9D1Sequence")

    from IsolationAlgs.DerivationTrackIsoConfig import DerivationTrackIsoCfg
    acc.merge(DerivationTrackIsoCfg(flags, object_types=("Electrons", "Photons", "Muons"), sequenceName="HIGG9D1Sequence"))

    # IFF augmentation - Adding Lepton Taggers
    from LeptonTaggers.LeptonTaggersConfig import DecoratePLITAlgsCfg
    acc.merge(DecoratePLITAlgsCfg(flags), sequenceName="HIGG9D1Sequence")
    
    from IsolationSelection.IsolationSelectionConfig import IsoCloseByAlgsCfg
    contNames = [ "Muons", "Electrons", "Photons" ]
    acc.merge(IsoCloseByAlgsCfg(flags, isPhysLite = False, containerNames = contNames, useSelTools = True, stream_name = kwargs['StreamName']), sequenceName="HIGG9D1Sequence")

    # diphoton vertex augmentation
    from DerivationFrameworkEGamma.EGammaToolsConfig import PhotonVertexSelectionWrapperKernelCfg
    acc.merge(PhotonVertexSelectionWrapperKernelCfg(flags), sequenceName="HIGG9D1Sequence")
    from DerivationFrameworkHiggs.HIGG1D1CustomVertexConfig import DiphotonVertexDecoratorCfg
    DiphotonVertexDecorator = acc.popToolsAndMerge(DiphotonVertexDecoratorCfg(
        flags,
        MinimumPhotonPt    = 4800.0,
        DiphotonVertexName = "HIGG9D1_DiphotonPrimaryVertices"))
    acc.addPublicTool(DiphotonVertexDecorator)
    acc.addEventAlgo(CompFactory.DerivationFramework.CommonAugmentation(name = "DiphotonVertexAugmentation", AugmentationTools = [DiphotonVertexDecorator]), sequenceName="HIGG9D1Sequence")

    #================
    # Thinning tools
    #================
    # https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/DaodRecommendations
    from DerivationFrameworkInDet.InDetToolsConfig import (TrackParticleThinningCfg, MuonTrackParticleThinningCfg)
    HIGG9D1TPthinning_expression = "InDetTrackParticles.DFCommonTightPrimary && abs(DFCommonInDetTrackZ0AtPV)*sin(InDetTrackParticles.theta) < 3.0*mm && InDetTrackParticles.pt > 10*GeV"
    HIGG9D1TrackParticleThinningTool = acc.getPrimaryAndMerge(TrackParticleThinningCfg(
        flags,
        name                    = "HIGG9D1TrackParticleThinningTool",
        StreamName              = kwargs['StreamName'],
        SelectionString         = HIGG9D1TPthinning_expression,
        InDetTrackParticlesKey  = "InDetTrackParticles"))
    acc.addPublicTool(HIGG9D1TrackParticleThinningTool)

    # Include inner detector tracks associated with muons
    HIGG9D1MuonTPThinningTool = acc.getPrimaryAndMerge(MuonTrackParticleThinningCfg(
        flags,
        name                    = "HIGG9D1MuonTPThinningTool",
        StreamName              = kwargs['StreamName'],
        MuonKey                 = "Muons",
        InDetTrackParticlesKey  = "InDetTrackParticles"))
    acc.addPublicTool(HIGG9D1MuonTPThinningTool)

    # Include inner detector tracks associated with electonrs
    HIGG9D1ElectronTPThinningTool = CompFactory.DerivationFramework.EgammaTrackParticleThinning(
        name                   = "HIGG9D1ElectronTPThinningTool",
        SGKey                  = "Electrons",
        StreamName             = kwargs['StreamName'],
        InDetTrackParticlesKey = "InDetTrackParticles")
    acc.addPublicTool(HIGG9D1ElectronTPThinningTool)

    # Include inner detector tracks associated with taus
    from DerivationFrameworkInDet.InDetToolsConfig import TauTrackParticleThinningCfg
    HIGG9D1TauTPThinningTool = acc.getPrimaryAndMerge(TauTrackParticleThinningCfg(
        flags,
        name                   = "HIGG9D1TauTPThinningTool",
        StreamName             = kwargs['StreamName'],
        TauKey                 = "TauJets",
        InDetTrackParticlesKey = "InDetTrackParticles",
        DoTauTracksThinning    = True,
        TauTracksKey           = "TauTracks"))

    # ID tracks associated to vertices
    HIGG9D1_ThinVtxTracks = CompFactory.DerivationFramework.Thin_vtxTrk(
        name                       = "HIGG9D1_ThinVtxTracks",
        StreamName                 = kwargs['StreamName'],
        TrackParticleContainerName = "InDetTrackParticles",
        VertexContainerNames       = [ HIGG9D1_Jpsi.OutputVtxContainerName, HIGG9D1_Upsi.OutputVtxContainerName ],
        IgnoreFlags                = True )
    acc.addPublicTool(HIGG9D1_ThinVtxTracks)

    thinningTools = [HIGG9D1TrackParticleThinningTool,
                     HIGG9D1MuonTPThinningTool,
                     HIGG9D1ElectronTPThinningTool,
                     HIGG9D1TauTPThinningTool,
                     HIGG9D1_ThinVtxTracks]

    ### Truth thinning
    if flags.Input.isMC:
        from DerivationFrameworkMCTruth.TruthDerivationToolsConfig import MenuTruthThinningCfg
        HIGG9D1TruthTool = acc.getPrimaryAndMerge(MenuTruthThinningCfg(
            flags                        = flags,
            name                         = "HIGG9D1TruthTool",
            StreamName                   = kwargs['StreamName'],
            ParticlesKey                 = "TruthParticles",
            VerticesKey                  = "TruthVertices",
            EventsKey                    = "TruthEvents",
            WritePartons                 = False,
            WriteHadrons                 = True,
            WriteBHadrons                = True,
            WriteCHadrons                = True,
            WriteGeant                   = False,
            WriteTauHad                  = True,
            WriteBSM                     = True,
            WriteBosons                  = True,
            WriteBSMProducts             = True,
            WriteBosonProducts           = False,
            WriteTopAndDecays            = True,
            WriteEverything              = False,
            WriteAllLeptons              = True,
            WriteNotPhysical             = False,
            PreserveDescendants          = False,
            PreserveGeneratorDescendants = True,
            PreserveAncestors            = True))
        acc.addPublicTool(HIGG9D1TruthTool)
        thinningTools.append(HIGG9D1TruthTool)

        #===================================================
        # HEAVY FLAVOR CLASSIFICATION FOR ttbar+jets EVENTS
        #===================================================
        from DerivationFrameworkMCTruth.HFClassificationCommonConfig import HFClassificationCommonCfg
        acc.merge(HFClassificationCommonCfg(flags), sequenceName="HIGG9D1Sequence")

    # Apply thinning
    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel(name, ThinningTools = thinningTools), sequenceName="HIGG9D1Sequence")

    return acc

def HIGG9D1Cfg(flags):
    log_HIGG9D1.info('****************** STARTING HIGG9D1 ******************')

    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    HIGG9D1TriggerListsHelper = TriggerListsHelper(flags)

    # dedicated augmentations
    acc.merge(HIGG9D1KernelCfg(flags, name="HIGG9D1Kernel", StreamName = streamName, TriggerListsHelper = HIGG9D1TriggerListsHelper))

    # =============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper

    HIGG9D1SlimmingHelper = SlimmingHelper("HIGG9D1SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
    HIGG9D1SlimmingHelper.SmartCollections = [
        "EventInfo",
        "Electrons",
        "Photons",
        "Muons",
        "PrimaryVertices",
        "InDetTrackParticles",
        "AntiKt4EMTopoJets",
        "AntiKt4EMPFlowJets",
        "BTagging_AntiKt4EMPFlow",
        "AntiKt4EMPFlowJets_FTAG",
        "MET_Baseline_AntiKt4EMTopo",
        "MET_Baseline_AntiKt4EMPFlow",
        "TauJets",
        "TauJets_MuonRM",
        "DiTauJets",
        "DiTauJetsLowPt",
        "AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets",
        "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets",
        "AntiKtVR30Rmax4Rmin02PV0TrackJets"]

    if flags.Tau.TauEleRM_isAvailable:
        HIGG9D1SlimmingHelper.SmartCollections.append("TauJets_EleRM")

    from DerivationFrameworkBPhys.commonBPHYMethodsCfg import getDefaultAllVariables
    AllVariables  = getDefaultAllVariables()
    AllVariables += ["PrimaryVertices"]
    AllVariables += ["CombinedMuonTrackParticles"]
    AllVariables += ["ExtrapolatedMuonTrackParticles"]

    doLRT = flags.Tracking.doLargeD0
    AllVariables += ["InDetTrackParticles", "InDetLargeD0TrackParticles"] if doLRT else ["InDetTrackParticles"]
    AllVariables += ["Muons", "MuonsLRT"] if doLRT else ["Muons"]
    AllVariables += ["MuonSegments"]

    # Additional content (shallow copy of PrimaryVertices)
    HIGG9D1SlimmingHelper.AppendToDictionary.update({
        "HIGG9D1_DiphotonPrimaryVertices" : "xAOD::VertexContainer",
        "HIGG9D1_DiphotonPrimaryVerticesAux" : "xAOD::ShallowAuxContainer"
    })
    AllVariables += ["HIGG9D1_DiphotonPrimaryVertices"]

    HIGG9D1_Jpsi = acc.getPublicTool("HIGG9D1_Jpsi")
    HIGG9D1_Upsi = acc.getPublicTool("HIGG9D1_Upsi")

    StaticContent = []
    if HIGG9D1_Jpsi.RefitPV:
        StaticContent += [ "xAOD::VertexContainer#%s"                        % HIGG9D1_Jpsi.RefPVContainerName ]
        StaticContent += [ "xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % HIGG9D1_Jpsi.RefPVContainerName ]
    if HIGG9D1_Upsi.RefitPV:
        StaticContent += [ "xAOD::VertexContainer#%s"                        % HIGG9D1_Upsi.RefPVContainerName ]
        StaticContent += [ "xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % HIGG9D1_Upsi.RefPVContainerName ]
    StaticContent += [ "xAOD::VertexContainer#%s"                        % HIGG9D1_Jpsi.OutputVtxContainerName ]
    StaticContent += [ "xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % HIGG9D1_Jpsi.OutputVtxContainerName ]
    StaticContent += [ "xAOD::VertexContainer#%s"                        % HIGG9D1_Upsi.OutputVtxContainerName ]
    StaticContent += [ "xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % HIGG9D1_Upsi.OutputVtxContainerName ]

    excludedVertexAuxData = "-vxTrackAtVertex.-MvfFitInfo.-isInitialized.-VTAV"
    StaticContent += ["xAOD::VertexContainer#SoftBVrtClusterTool_Tight_Vertices"]
    StaticContent += ["xAOD::VertexAuxContainer#SoftBVrtClusterTool_Tight_VerticesAux." + excludedVertexAuxData]
    StaticContent += ["xAOD::VertexContainer#SoftBVrtClusterTool_Medium_Vertices"]
    StaticContent += ["xAOD::VertexAuxContainer#SoftBVrtClusterTool_Medium_VerticesAux." + excludedVertexAuxData]
    StaticContent += ["xAOD::VertexContainer#SoftBVrtClusterTool_Loose_Vertices"]
    StaticContent += ["xAOD::VertexAuxContainer#SoftBVrtClusterTool_Loose_VerticesAux." + excludedVertexAuxData]

    HIGG9D1SlimmingHelper.StaticContent = StaticContent

    # Extra content
    HIGG9D1SlimmingHelper.ExtraVariables += [
        "AntiKt4EMTopoJets.DFCommonJets_QGTagger_truthjet_nCharged.DFCommonJets_QGTagger_truthjet_pt.DFCommonJets_QGTagger_truthjet_eta.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1.ConeExclBHadronsFinal.ConeExclCHadronsFinal.GhostBHadronsFinal.GhostCHadronsFinal.GhostBHadronsFinalCount.GhostBHadronsFinalPt.GhostCHadronsFinalCount.GhostCHadronsFinalPt.IsoFixedCone5PtPUsub",
        "AntiKt4EMPFlowJets.DFCommonJets_QGTagger_truthjet_nCharged.DFCommonJets_QGTagger_truthjet_pt.DFCommonJets_QGTagger_truthjet_eta.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1.ConeExclBHadronsFinal.ConeExclCHadronsFinal.GhostBHadronsFinal.GhostCHadronsFinal.GhostBHadronsFinalCount.GhostBHadronsFinalPt.GhostCHadronsFinalCount.GhostCHadronsFinalPt.GhostPartons.isJvtHS.isJvtPU.IsoFixedCone5PtPUsub",
        "InDetTrackParticles.TTVA_AMVFVertices.TTVA_AMVFWeights.eProbabilityHT.numberOfTRTHits.numberOfTRTOutliers",
        "EventInfo.GenFiltHT.GenFiltMET.GenFiltHTinclNu.GenFiltPTZ.GenFiltFatJ.HF_Classification.HF_SimpleClassification",
        "TauJets.dRmax.etOverPtLeadTrk",
        "TauJets_MuonRM.dRmax.etOverPtLeadTrk",
        "HLT_xAOD__TrigMissingETContainer_TrigEFMissingET.ex.ey",
        "HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_mht.ex.ey"]
    if flags.Tau.TauEleRM_isAvailable:
        HIGG9D1SlimmingHelper.ExtraVariables += ["TauJets_EleRM.dRmax.etOverPtLeadTrk"]

    # needed for photon ID
    from DerivationFrameworkEGamma.PhotonsCPDetailedContent import PhotonsCPDetailedContent
    HIGG9D1SlimmingHelper.ExtraVariables += PhotonsCPDetailedContent
        
    # FTAG Xbb extra content
    extraList = []
    for tagger in ["GN2Xv01", "GN2Xv02"]:
        for score in ["phbb", "phcc", "ptop", "pqcd"]:
            extraList.append(f"{tagger}_{score}")
    HIGG9D1SlimmingHelper.ExtraVariables += ["AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets." + ".".join(extraList)]

    # Large-Radius jet regression extra content
    extraListReg = []
    modelName = "bJR10v00"
    for score in ["mass", "pt"]:
        extraListReg.append(f"{modelName}_{score}")
    HIGG9D1SlimmingHelper.ExtraVariables += ["AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets." + ".".join(extraListReg)]
        
    # Truth extra content
    if flags.Input.isMC:
        from DerivationFrameworkMCTruth.MCTruthCommonConfig import addTruth3ContentToSlimmerTool
        addTruth3ContentToSlimmerTool(HIGG9D1SlimmingHelper)
        HIGG9D1SlimmingHelper.ExtraVariables += ["Electrons.TruthLink","Muons.TruthLink","Photons.TruthLink","AntiKt4TruthDressedWZJets.IsoFixedCone5Pt","TruthPrimaryVertices.t.x.y.z"]

        AllVariables += ["TruthEvents","TruthParticles","TruthVertices","MuonTruthParticles","TruthPrimaryVertices"]
        AllVariables += ["TruthLHEParticles","TruthHFWithDecayParticles","TruthHFWithDecayVertices","TruthCharm","TruthPileupParticles","InTimeAntiKt4TruthJets","OutOfTimeAntiKt4TruthJets","AntiKt4TruthDressedWZJets","AntiKt4TruthWZJets"]

        from DerivationFrameworkMCTruth.MCTruthCommonConfig import AddTauAndDownstreamParticlesCfg
        acc.merge(AddTauAndDownstreamParticlesCfg(flags))
        AllVariables += ["TruthTausWithDecayParticles","TruthTausWithDecayVertices"]

    # Extra isolation content
    EgammaExtraContent= [
        "ptcone20","ptcone30","ptcone40", "ptvarcone20", "ptvarcone30", "ptvarcone40", "topoetcone20", "topoetcone30",
        "neflowisol20", "neflowisol30", "neflowisol40",
        "ptvarcone20_Nonprompt_All_MaxWeightTTVA_pt500" ,"ptvarcone20_Nonprompt_All_MaxWeightTTVA_pt1000",
        "ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt500","ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt1000",
        "ptvarcone40_Nonprompt_All_MaxWeightTTVA_pt500","ptvarcone40_Nonprompt_All_MaxWeightTTVA_pt1000",
        "ptcone20_Nonprompt_All_MaxWeightTTVA_pt500", "ptcone20_Nonprompt_All_MaxWeightTTVA_pt1000",
        "ptcone30_Nonprompt_All_MaxWeightTTVA_pt500", "ptcone30_Nonprompt_All_MaxWeightTTVA_pt1000",
        "ptcone40_Nonprompt_All_MaxWeightTTVA_pt500", "ptcone40_Nonprompt_All_MaxWeightTTVA_pt1000",
        "topoetconecoreConeEnergyCorrection", "isoSelIsOK", "topoetcone20_CloseByCorr"
    ]

    ElectronsExtraContent = [ ".".join( ["Electrons", "deltaPhiRescaled2", "deltaPhiFromLastMeasurement", "originalTrackParticle", "ptvarcone30_Nonprompt_All_MaxWeightTTVALooseCone_pt1000", "ptcone20_Nonprompt_All_MaxWeightTTVALooseCone_pt1000", "ptvarcone30_Nonprompt_All_MaxWeightTTVALooseCone_pt1000_CloseByCorr", "ptcone20_Nonprompt_All_MaxWeightTTVALooseCone_pt1000_CloseByCorr"] + EgammaExtraContent ) ]

    PhotonsExtraContent = [ ".".join(["Photons", "topoetcone40", "topoetcone40_CloseByCorr", "ptcone20_CloseByCorr"] + EgammaExtraContent ) ]

    MuonsExtraContent = [ ".".join( [
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
        "msInnerMatchChi2", "isoSelIsOK", "ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt500_CloseByCorr",
        "ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt1000_CloseByCorr", "neflowisol20_CloseByCorr", "topoetcone20_CloseByCorr"
    ] ) ]

    HIGG9D1SlimmingHelper.ExtraVariables += ElectronsExtraContent
    HIGG9D1SlimmingHelper.ExtraVariables += PhotonsExtraContent
    HIGG9D1SlimmingHelper.ExtraVariables += MuonsExtraContent

    from IsolationSelection.IsolationSelectionConfig import setupIsoCloseBySlimmingVariables
    setupIsoCloseBySlimmingVariables(HIGG9D1SlimmingHelper)

    from LeptonTaggers.LeptonTaggersConfig import GetExtraPLITVariablesForDxAOD
    HIGG9D1SlimmingHelper.ExtraVariables += GetExtraPLITVariablesForDxAOD()
    
    HIGG9D1SlimmingHelper.AllVariables = AllVariables

    # Trigger content
    HIGG9D1SlimmingHelper.IncludeTriggerNavigation = False
    HIGG9D1SlimmingHelper.IncludeJetTriggerContent = False
    HIGG9D1SlimmingHelper.IncludeMuonTriggerContent = False
    HIGG9D1SlimmingHelper.IncludeEGammaTriggerContent = False
    HIGG9D1SlimmingHelper.IncludeTauTriggerContent = False
    HIGG9D1SlimmingHelper.IncludeEtMissTriggerContent = False
    HIGG9D1SlimmingHelper.IncludeBJetTriggerContent = False
    HIGG9D1SlimmingHelper.IncludeBPhysTriggerContent = False
    HIGG9D1SlimmingHelper.IncludeMinBiasTriggerContent = False
    # Compact b-jet trigger matching info
    HIGG9D1SlimmingHelper.IncludeBJetTriggerByYearContent = True

    # Trigger matching
    # Run 2
    if flags.Trigger.EDMVersion == 2:
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = HIGG9D1SlimmingHelper, 
                                               OutputContainerPrefix = "TrigMatch_", 
                                               TriggerList = HIGG9D1TriggerListsHelper.Run2TriggerNamesTau)
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = HIGG9D1SlimmingHelper, 
                                               OutputContainerPrefix = "TrigMatch_",
                                               TriggerList = HIGG9D1TriggerListsHelper.Run2TriggerNamesNoTau)
    # Run 3, or Run 2 with navigation conversion
    if flags.Trigger.EDMVersion == 3 or (flags.Trigger.EDMVersion == 2 and flags.Trigger.doEDMVersionConversion):
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import AddRun3TrigNavSlimmingCollectionsToSlimmingHelper
        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(HIGG9D1SlimmingHelper)

    # L1 trigger objects
    from DerivationFrameworkPhys.TriggerMatchingCommonConfig import getDataYear
    if getDataYear(flags) >= 2024:
        # Run 3 with Phase I jet RoIs.
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddjFexRoIsToSlimmingHelper
        AddjFexRoIsToSlimmingHelper(SlimmingHelper = HIGG9D1SlimmingHelper)
    elif getDataYear(flags) >= 2015:
        # Run 2 and early Run 3, legacy L1 RoIs
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddLegacyL1JetRoIsToSlimmingHelper
        AddLegacyL1JetRoIsToSlimmingHelper(SlimmingHelper = HIGG9D1SlimmingHelper)

    # Output stream    
    HIGG9D1ItemList = HIGG9D1SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_HIGG9D1", ItemList=HIGG9D1ItemList, AcceptAlgs=["HIGG9D1Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_HIGG9D1", AcceptAlgs=["HIGG9D1Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData, MetadataCategory.TruthMetaData]))
    acc.printConfig(withDetails=True, summariseProps=True, onlyComponents = [], printDefaults=True, printComponentsOnly=False)
    return acc
