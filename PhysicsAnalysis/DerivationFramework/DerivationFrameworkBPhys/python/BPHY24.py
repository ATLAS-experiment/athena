# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#====================================================================
# BPHY24.py
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


BPHYDerivationName = "BPHY24"
streamName = "StreamDAOD_BPHY24"

def BPHY24Cfg(flags):

    # Lists for better code organization
    augsList          = [] # List of active augmentation tools
    skimList          = [] # List of active skimming algorithms
    thinList          = [] # List of active thinning algorithms
    outVtxList        = [] # List of reconstructed candidates to store
    outRePVList       = [] # List of candidates holding refitted primary vertices
    thinTrkVtxList    = [] # List of reconstructed candidates to use for the thinning of tracks from vertices
    thinPassFlagsList = [] # List of pass-flags in the reconstructed candidates to se for the thinning
    finalCandidateList = []

    from AthenaServices.PartPropSvcConfig import PartPropSvcCfg
    from DerivationFrameworkBPhys.commonBPHYMethodsCfg import (
        BPHY_V0ToolCfg, BPHY_InDetDetailedTrackSelectorToolCfg,
        BPHY_VertexPointEstimatorCfg, BPHY_TrkVKalVrtFitterCfg,
        AugOriginalCountsCfg)
    from JpsiUpsilonTools.JpsiUpsilonToolsConfig import PrimaryVertexRefittingToolCfg
    acc = ComponentAccumulator()
    PartPropSvcName = acc.getPrimaryAndMerge(PartPropSvcCfg(flags))
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
    
    bSkim = "(count(BPHY24JpsimmKshortCascadeSV1.Bd_mass) + count(BPHY24JpsieeKshortCascadeSV1.Bd_mass)) > 0"

    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import (
        xAODStringSkimmingToolCfg)
    BPHY24_Skim_Bcandidates = acc.getPrimaryAndMerge(xAODStringSkimmingToolCfg(
        flags, name  = "BPHY24_Skim_Bcandidates", expression = bSkim ))
    skimList += [ BPHY24_Skim_Bcandidates ]

    BPHY24_AugOriginalCounts = acc.popToolsAndMerge(
        AugOriginalCountsCfg(flags, name = "BPHY24_AugOriginalCounts"))
    augsList += [ BPHY24_AugOriginalCounts ]

    mainIDInput = "InDetTrackParticles"
    originalTrackCond = ["InDetTrackParticles", "GSFTrackParticles"]
    if flags.Tracking.doLargeD0:
        # LRT track merge
        from DerivationFrameworkInDet.InDetToolsConfig import InDetLRTMergeCfg
        acc.merge(InDetLRTMergeCfg(flags))

        mainIDInput = "InDetWithLRTTrackParticles"
        originalTrackCond += ["InDetLargeD0TrackParticles"]

    BPHY24_Finder_DiMuon = CompFactory.Analysis.JpsiFinder( name    = "BPHY24_Finder_DiMuon",
                                             muAndMu                     = True,
                                             muAndTrack                  = False,
                                             TrackAndTrack               = False,
                                             assumeDiMuons               = True,
                                             muonThresholdPt             = 3000.,
                                             higherPt                    = 3500.,
                                             invMassUpper                = 7000.,
                                             invMassLower                = 1.,
                                             Chi2Cut                     = 30.,
                                             oppChargesOnly              = False,
                                             allChargeCombinations       = True,
                                             atLeastOneComb              = True,
                                             useCombinedMeasurement      = False, # Only takes effect if combOnly=True
                                             muonCollectionKey           = "Muons",
                                             TrackParticleCollection     = "InDetTrackParticles",
                                             V0VertexFitterTool          = None,
                                             useV0Fitter                 = False,
                                             TrkVertexFitterTool         = vkalvrt,
                                             TrackSelectorTool           = trackselect,
                                             VertexPointEstimator        = vpest,
                                             useMCPCuts                  = False )


    BPHY24_SelectAndWrite_DiMuon = CompFactory.DerivationFramework.Reco_mumu( name   = "BPHY24_SelectAndWrite_DiMuon",
                                        JpsiFinder               = BPHY24_Finder_DiMuon,
                                        V0Tools                  = V0Tools,
                                        PVRefitter               = PVrefit,
                                        OutputVtxContainerName   = "BPHY24_DiMuon_Candidates",
                                        PVContainerName          = "PrimaryVertices",
                                        RefPVContainerName       = "SHOULDNOTBEUSED", # The container would be created if PV refit was requested (not needed at this point)
                                        DoVertexType             = 7 ) # Vertex type marking our own reconstruced secondary candidates

    augsList += [ BPHY24_SelectAndWrite_DiMuon ]
    # Final selection of the di-muon candidates
    thinTrkVtxList    += [ "BPHY24_DiMuon_Candidates" ]
    outVtxList        += [ "BPHY24_DiMuon_Candidates" ]
    thinPassFlagsList += [ "passed_Jpsi" ] # TODO: is this really needed?

    BPHY24_Select_DiMuons = CompFactory.DerivationFramework.Select_onia2mumu( name    = "BPHY24_Select_DiMuons",
                                           HypothesisName        = "Jpsi",
                                           InputVtxContainerName = "BPHY24_DiMuon_Candidates",
                                           V0Tools               = V0Tools,
                                           VtxMassHypo           = 3096.916, # used only for pseudo-proper decay time etc. calculations
                                           MassMax               = 10000., # loose cut to keep selection from BPHY24_Finder_DiMuon
                                           MassMin               = 0.,     # loose cut to keep selection from BPHY24_Finder_DiMuon
                                           Chi2Max               = 1000.,  # loose cut to keep selection from BPHY24_Finder_DiMuon (chi2, not chi2/NDF)
                                           DoVertexType          = 7 ) # Vertex type marking our own reconstruced secondary candidates   

    augsList += [ BPHY24_Select_DiMuons ]

    ElectronLHSelectorLHvloose = CompFactory.AsgElectronLikelihoodTool("ElectronLHSelectorLHvloose",
            primaryVertexContainer = "PrimaryVertices",
            ConfigFile="ElectronPhotonSelectorTools/offline/mc20_20210514/ElectronLikelihoodVeryLooseOfflineConfig2017_Smooth.conf")

    

    ElectronLHSelectorLHvloose_nod0 = CompFactory.AsgElectronLikelihoodTool("ElectronLHSelectorLHvloosenod0", primaryVertexContainer = "PrimaryVertices",
            ConfigFile="ElectronPhotonSelectorTools/offline/mc16_20190328_nod0/ElectronLikelihoodVeryLooseOfflineConfig2017_Smooth_nod0.conf")   # Still OK to use in Run3?


    # decorate electrons with the output of LH vloose (nod0)
    from DerivationFrameworkEGamma.EGammaToolsConfig import EGElectronLikelihoodToolWrapperCfg
    ElectronPassLHvloose = acc.addPublicTool(acc.popToolsAndMerge(EGElectronLikelihoodToolWrapperCfg(flags, name = "ElectronPassLHvloose",
                                            EGammaElectronLikelihoodTool = ElectronLHSelectorLHvloose,
                                            EGammaFudgeMCTool = "",
                                            CutType = "",
                                            StoreGateEntryName = "DFCommonElectronsLHVeryLoose",
                                            ContainerName = "Electrons",
                                            StoreTResult=False)))

    ElectronPassLHvloosenod0 = acc.addPublicTool(acc.popToolsAndMerge(EGElectronLikelihoodToolWrapperCfg(flags, name = "ElectronPassLHvloosenod0",
                                            EGammaElectronLikelihoodTool = ElectronLHSelectorLHvloose_nod0,
                                            EGammaFudgeMCTool = "",
                                            CutType = "",
                                            StoreGateEntryName = "DFCommonElectronsLHVeryLoosenod0",
                                            ContainerName = "Electrons",
                                            StoreTResult=False)))
    augsList += [ElectronPassLHvloose, ElectronPassLHvloosenod0]

    BPHY24DiElectronFinder = CompFactory.Analysis.JpsiFinder_ee(
        name                        = "BPHY24DiElectronFinder",
        elAndEl                     = True,
        elAndTrack                  = False,
        TrackAndTrack               = False,
        assumeDiElectrons           = True,
        elThresholdPt               = 4000.0,
        invMassUpper                = 7000.0,
        invMassLower                = 200.0,
        Chi2Cut                     = 30.,
        oppChargesOnly              = False,
        allChargeCombinations       = True,
        useElectronTrackMeasurement = True,
        electronCollectionKey       = "Electrons",
        TrackParticleCollection     = "GSFTrackParticles",
        useEgammaCuts               = True,
        V0VertexFitterTool          = None,
        useV0Fitter                 = False,
        TrkVertexFitterTool         = vkalvrt,
        TrackSelectorTool           = trackselect,
        VertexPointEstimator        = vpest,
        ElectronSelection             = "d0_or_nod0",
        PartPropSvc = PartPropSvcName
        )

    BPHY24_SelectAndWrite_DiElectron = CompFactory.DerivationFramework.Reco_mumu(
            name                   = "BPHY24_SelectAndWrite_DiElectron",
            JpsiFinder             = BPHY24DiElectronFinder,
            V0Tools                = V0Tools,
            PVRefitter             = PVrefit,
            OutputVtxContainerName = "BPHY24_DiElectron_Candidates",
            PVContainerName        = "PrimaryVertices",
            RefPVContainerName     = "SHOULDNOTBEUSED",
            DoVertexType           = 7
            )

    augsList += [ BPHY24_SelectAndWrite_DiElectron ]

    BPHY24_Select_DiElectrons = CompFactory.DerivationFramework.Select_onia2mumu(
            name                  = "BPHY24_Select_DiElectrons",
            HypothesisName        = "Jpsi",
            InputVtxContainerName = "BPHY24_DiElectron_Candidates",
            V0Tools               = V0Tools,
            VtxMassHypo           = 3096.916,
            MassMin               = 400.0,
            MassMax               = 7000.0,
            Chi2Max               = 30,
            DoVertexType          = 7
            )

    thinTrkVtxList    += [ "BPHY24_DiElectron_Candidates" ]
    outVtxList        += [ "BPHY24_DiElectron_Candidates" ]
 
    augsList += [ BPHY24_Select_DiElectrons ]

    V0ContainerName = "BPHY24RecoV0Candidates"
    KshortContainerName = "BPHY24RecoKshortCandidates"
    LambdaContainerName = "BPHY24RecoLambdaCandidates"
    LambdabarContainerName = "BPHY24RecoLambdabarCandidates"

    from DerivationFrameworkBPhys.V0ToolConfig import (
        BPHY_InDetV0FinderToolCfg, BPHY_Reco_V0FinderCfg)

    BPHY24_V0FinderTool = acc.popToolsAndMerge(BPHY_InDetV0FinderToolCfg(
        flags, BPHYDerivationName,
        TrackParticleCollection     = mainIDInput,
        V0ContainerName             = V0ContainerName,
        KshortContainerName         = KshortContainerName,
        LambdaContainerName         = LambdaContainerName,
        LambdabarContainerName      = LambdabarContainerName,
        RelinkTracks                = originalTrackCond,
        use_innerPixHits            = True,
        errmass                     = 99999,
        uksmin                      = 400,
        uksmax                      = 600,
        ksmin                       = 400,
        ksmax                       = 600,
        ulamin                      = -1,
        ulamax                      = -1,
        lamin                       = -1,
        lamax                       = -1,
        d0_cut                      = -1,
        max_d0_cut                  = 50,
        max_z0_cut                  = 250,
        ptTRT                       = 0,
        minConstrVertProb           = 0.000001,
        useBDT                      = True,
        BDTCut                      = 0.1,
        use_TrackSelector           = False))

    BPHY24_Reco_V0Finder = acc.popToolsAndMerge(BPHY_Reco_V0FinderCfg(
        flags, derivation = BPHYDerivationName,
        V0ContainerName = V0ContainerName,
        KshortContainerName = KshortContainerName,
        LambdaContainerName = LambdaContainerName,
        LambdabarContainerName = LambdabarContainerName,
        VxPrimaryCandidateName      = "BPHY24_DiMuon_Candidates",
        CheckVertexContainers = ['BPHY24_DiMuon_Candidates',
                                 'BPHY24_DiElectron_Candidates'],
        V0FinderTool = BPHY24_V0FinderTool))

    augsList += [BPHY24_Reco_V0Finder]
    outVtxList += ['BPHY24RecoKshortCandidates']
    outVtxList += ["BPHY24RecoV0Candidates"]
    thinTrkVtxList += ['BPHY24RecoKshortCandidates']
    thinPassFlagsList += [ "" ] # TODO: is this really needed?
    finalCandidateList += ["BPHY24RecoKshortCandidates"]
    finalCandidateList += ["BPHY24RecoV0Candidates"]
    from TrkConfig.TrkVKalVrtFitterConfig import JpsiV0VertexFitCfg
    JpsiV0VertexFit = acc.popToolsAndMerge(JpsiV0VertexFitCfg(flags))
    acc.addPublicTool(JpsiV0VertexFit)

    BPHY24JpsimmKshort          = CompFactory.DerivationFramework.JpsiPlusV0Cascade(
        name                    = "BPHY24mmKshort",
        V0Tools                 = V0Tools,
        HypothesisName          = "Bd",
        TrkVertexFitterTool     = JpsiV0VertexFit,
        PVRefitter              = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
        V0Hypothesis            = 310,
        JpsiMassLowerCut        = 1.,
        JpsiMassUpperCut        = 7000.,
        ApplyJpsiMassConstraint = False,
        V0MassLowerCut          = 400.,
        V0MassUpperCut          = 600.,
        MassLowerCut            = 4300.,
        MassUpperCut            = 6300.,
        RefitPV                 = True,
        RefPVContainerName      = "BPHY24RefittedPrimaryVertices_mm",
        JpsiVertices            = "BPHY24_DiMuon_Candidates",
        CascadeVertexCollections= ["BPHY24JpsimmKshortCascadeSV2", "BPHY24JpsimmKshortCascadeSV1"],
        V0Vertices              = "BPHY24RecoV0Candidates",
        V0TrackContainerName    = mainIDInput,
        RelinkTracks = originalTrackCond)

    augsList += [BPHY24JpsimmKshort]
    outVtxList += BPHY24JpsimmKshort.CascadeVertexCollections
    outVtxList += ["BPHY24RefittedPrimaryVertices_mm"]
    thinTrkVtxList += BPHY24JpsimmKshort.CascadeVertexCollections
    finalCandidateList += BPHY24JpsimmKshort.CascadeVertexCollections

    BPHY24JpsieeKshort          = CompFactory.DerivationFramework.JpsiPlusV0Cascade(
        name                    = "BPHY24eeKshort",
        V0Tools                 = V0Tools,
        HypothesisName          = "Bd",
        TrkVertexFitterTool     = JpsiV0VertexFit,
        PVRefitter              = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
        V0Hypothesis            = 310,
        JpsiMassLowerCut        = 100.,
        JpsiMassUpperCut        = 7000.,
        ApplyJpsiMassConstraint = False,
        V0MassLowerCut          = 400.,
        V0MassUpperCut          = 600.,
        MassLowerCut            = 4300.,
        MassUpperCut            = 6300.,
        JpsiTrackPDGID          = 11,
        JpsiTrackContainerName  = "GSFTrackParticles",
        RefitPV                 = True,
        RefPVContainerName      = "BPHY24RefittedPrimaryVertices_ee",
        JpsiVertices            = "BPHY24_DiElectron_Candidates",
        CascadeVertexCollections= ["BPHY24JpsieeKshortCascadeSV2", "BPHY24JpsieeKshortCascadeSV1"],
        V0Vertices              = "BPHY24RecoV0Candidates",
        V0TrackContainerName    = mainIDInput,
        RelinkTracks = originalTrackCond)

    augsList += [BPHY24JpsieeKshort]
    finalCandidateList += BPHY24JpsieeKshort.CascadeVertexCollections
    outVtxList += BPHY24JpsieeKshort.CascadeVertexCollections
    outVtxList += ["BPHY24RefittedPrimaryVertices_ee"]
    thinTrkVtxList += BPHY24JpsieeKshort.CascadeVertexCollections

    from TrackVertexAssociationTool.TrackVertexAssociationToolConfig import isoTTVAToolCfg
    TTVATool = acc.popToolsAndMerge(isoTTVAToolCfg(flags,
                                                  WorkingPoint = "Custom",
                                                  d0_cut = -1,
                                                  d0sig_cut = -1,
                                                  dzSinTheta_cut = -1,
                                                  doUsedInFit = False))
    acc.addPublicTool(TTVATool)

    from InDetConfig.InDetTrackSelectionToolConfig import isoTrackSelectionToolCfg
    TrackSelTool = acc.popToolsAndMerge(isoTrackSelectionToolCfg(flags,
                                                                maxZ0SinTheta= 2,
                                                                minPt= 1000,
                                                                CutLevel= "Loose"))
    acc.addPublicTool(TrackSelTool)

    from IsolationAlgs.IsoToolsConfig import TrackIsolationToolCfg
    TrackIsoTool = acc.popToolsAndMerge(TrackIsolationToolCfg(flags,
                                                              TrackSelectionTool = TrackSelTool,
                                                              TTVATool = TTVATool))
    acc.addPublicTool(TrackIsoTool)
    BPHY24TrackIsolationDecoratorBtoKee = CompFactory.DerivationFramework.VertexTrackIsolation(
      name                            = "BPHY24TrackIsolationDecoratorBtoKee",
      TrackIsoTool                    = TrackIsoTool,
      TrackContainer                  = "InDetTrackParticles",
      InputVertexContainer            = "BPHY24JpsieeKshortCascadeSV1",
      FixElecExclusion                = True,
      IncludeV0                       = True)
    BPHY24TrackIsolationDecoratorBtoKmumu = CompFactory.DerivationFramework.VertexTrackIsolation(
      name                            = "BPHY24TrackIsolationDecoratorBtoKmumu ",
      TrackIsoTool                    = TrackIsoTool,
      TrackContainer                  = "InDetTrackParticles",
      InputVertexContainer            = "BPHY24JpsimmKshortCascadeSV1",
      FixElecExclusion                = False,
      IncludeV0                       = True)
    
    BPHY24TrackIsolationDecoratorJpsiee = CompFactory.DerivationFramework.VertexTrackIsolation(
      name                            = "BPHY24TrackIsolationDecoratorJpsiee",
      TrackIsoTool                    = TrackIsoTool,
      TrackContainer                  = "InDetTrackParticles",
      InputVertexContainer            = "BPHY24_DiElectron_Candidates",
      FixElecExclusion                = True,
      IncludeV0                       = False)
    
    BPHY24TrackIsolationDecoratorJpsimumu = CompFactory.DerivationFramework.VertexTrackIsolation(
      name                            = "BPHY24TrackIsolationDecoratorJpsimumu",
      TrackIsoTool                    = TrackIsoTool,
      TrackContainer                  = "InDetTrackParticles",
      InputVertexContainer            = "BPHY24_DiMuon_Candidates",
      FixElecExclusion                = False,
      IncludeV0                       = False)

    augsList += [ BPHY24TrackIsolationDecoratorBtoKee,
              BPHY24TrackIsolationDecoratorBtoKmumu,
              BPHY24TrackIsolationDecoratorJpsiee,
              BPHY24TrackIsolationDecoratorJpsimumu]

    if flags.Trigger.EDMVersion >= 0:
        trigger_list = [ # Pure muon triggers
            "HLT_mu11_mu6_bDimu",
            "HLT_mu11_mu6_bDimu2700",
            "HLT_mu11_mu6_bDimu_L1LFV-MU11",
            "HLT_mu11_mu6_bDimu2700_L1LFV-MU11",
            "HLT_mu11_mu6_bBmumuxv2",
            "HLT_mu10_mu6_bBmumuxv2",
            "HLT_mu10_mu6_bBmumuxv2_delayed",
            "HLT_2mu6_bBmumuxv2",
            "HLT_2mu6_bBmumuxv2_delayed",
            "HLT_2mu6_bBmumuxv2_L1LFV-MU6",
            "HLT_mu6_mu4_bBmumuxv2",
            "HLT_mu6_mu4_bBmumuxv2_delayed",
            "HLT_2mu4_bBmumuxv2",
            "HLT_mu11_mu6_bJpsimumu",
            "HLT_2mu10_bJpsimumu",
            "HLT_mu11_mu6_bJpsimumu_L1LFV-MU11",
            "HLT_2mu6_bJpsimumu_L1BPH-2M9-2MU6_BPH-2DR15-2MU6",
            "HLT_2mu6_bJpsimumu_delayed_L1BPH-2M9-2MU6_BPH-2DR15-2MU6",
            "HLT_2mu10_bJpsimumu_noL2",
            "HLT_mu10_mu6_bJpsimumu",
            "HLT_mu10_mu6_bJpsimumu_delayed",
            "HLT_2mu6_bJpsimumu",
            "HLT_2mu6_bJpsimumu_delayed",
            "HLT_mu6_mu4_bJpsimumu",
            "HLT_mu6_mu4_bJpsimumu_Lxy0_L1BPH-2M9-MU6MU4_BPH-0DR15-MU6MU4",
            "HLT_2mu4_bJpsimumu",
            "HLT_mu6_2mu4_bJpsi",
            "HLT_mu6_2mu4_bJpsi_delayed",
            "HLT_2mu14",
            "HLT_2mu10",
            # dielectron triggers
            "HLT_2e5_lhvloose_nod0_bBeexM6000t",  #37,143,877  inb
            "HLT_e5_lhvloose_nod0_bBeexM6000t",  #37,143,877
            "HLT_e5_lhvloose_nod0_bBeexM6000t_2mu4_nomucomb_L1BPH-0DR3-EM7J15_2MU4",   #37,312,506
            "HLT_e5_lhvloose_nod0_bBeexM6000t_mu6_nomucomb_L1BPH-0DR3-EM7J15_MU6",   #27,041,892
            "HLT_e5_lhvloose_nod0_bBeexM6000_mu6_nomucomb_L1BPH-0DR3-EM7J15_MU6",   #149,100
            "HLT_e9_lhloose_bBeexM2700_2mu4_nomucomb_L1BPH-0DR3-EM7J15_2MU4",   #2,681,764
            "HLT_e9_lhloose_bBeexM2700_mu6_nomucomb_L1BPH-0DR3-EM7J15_MU6",   #1,979,362
            "HLT_e9_lhloose_bBeexM6000_2mu4_nomucomb_L1BPH-0DR3-EM7J15_2MU4",   #3,359,105
            "HLT_e9_lhloose_bBeexM6000_mu6_nomucomb_L1BPH-0DR3-EM7J15_MU6",   #2,426,663
            "HLT_e9_lhloose_e5_lhloose_bBeexM2700_2mu4_nomucomb_L1BPH-0M9-EM7-EM5_2MU4",   #2,950,935
            "HLT_e9_lhloose_e5_lhloose_bBeexM2700_mu6_nomucomb_L1BPH-0M9-EM7-EM5_MU6",   #2,928,030
            "HLT_e9_lhloose_e5_lhloose_bBeexM6000_2mu4_nomucomb_L1BPH-0M9-EM7-EM5_2MU4",   #3,647,507
            "HLT_e9_lhloose_e5_lhloose_bBeexM6000_mu6_nomucomb_L1BPH-0M9-EM7-EM5_MU6",   #3,605,371
            "HLT_e9_lhvloose_nod0_e5_lhvloose_nod0_bBeexM6000t_2mu4_nomucomb_L1BPH-0M9-EM7-EM5_2MU4",   #40,169,436
            "HLT_e9_lhvloose_nod0_e5_lhvloose_nod0_bBeexM6000t_mu6_nomucomb_L1BPH-0M9-EM7-EM5_MU6",   #37,312,506
            "HLT_e9_lhvloose_nod0_e5_lhvloose_nod0_bBeexM6000_mu6_nomucomb_L1BPH-0M9-EM7-EM5_MU6",   #677,340
        ]

        BPHY24TrigSkimmingTool = CompFactory.DerivationFramework.TriggerSkimmingTool(
            name = "BPHY24TrigSkimmingTool",
            TriggerListOR = trigger_list,
            TriggerListORHLTOnly = ["HLT_2e5_lhvloose_nod0_bBeexM6000t","HLT_e5_lhvloose_nod0_bBeexM6000t"] )
        acc.addPublicTool(BPHY24TrigSkimmingTool)
        skimList += [BPHY24TrigSkimmingTool]

    # ID tracks
    BPHY24_Thin_VtxTracks = CompFactory.DerivationFramework.Thin_vtxTrk( name     = "BPHY24_Thin_VtxTracks",
                                                                StreamName = streamName,
                                                                TrackParticleContainerName = "InDetTrackParticles",
                                                                VertexContainerNames       = finalCandidateList,
                                                                IgnoreFlags                = True )
                                                                # PassFlags                  = thinPassFlagsList )
    thinList += [ BPHY24_Thin_VtxTracks ]
  
    # LRT ID tracks
    if flags.Tracking.doLargeD0:
        BPHY24_Thin_VtxTracks_LRT = CompFactory.DerivationFramework.Thin_vtxTrk(
            name     = "BPHY24_Thin_VtxTracks_LRT",
            StreamName = streamName,
            TrackParticleContainerName = "InDetLargeD0TrackParticles",
            VertexContainerNames       = finalCandidateList,
            IgnoreFlags                = True )
        thinList += [ BPHY24_Thin_VtxTracks_LRT ]
    
    # GSF tracks
    BPHY24_Thin_VtxTracks_GSF = CompFactory.DerivationFramework.Thin_vtxTrk( name    = "BPHY24_Thin_VtxTracks_GSF",
                                                                StreamName = streamName,
                                                                TrackParticleContainerName = "GSFTrackParticles",
                                                                VertexContainerNames       = finalCandidateList,
                                                                IgnoreFlags                = True )
    thinList += [ BPHY24_Thin_VtxTracks_GSF ]
    
    # Muons (TODO: thinning not used muons or something else ?)
    BPHY24_Thin_Muons = CompFactory.DerivationFramework.MuonTrackParticleThinning( name   = "BPHY24_Thin_Muons",
                                                                          MuonKey                = "Muons",
                                                                          StreamName = streamName,
                                                                          InDetTrackParticlesKey = "InDetTrackParticles" )
    thinList += [ BPHY24_Thin_Muons ]
    
    # Electrons
    BPHY24_Thin_Egamma = CompFactory.DerivationFramework.EgammaTrackParticleThinning(
          name                   = "BPHY24_Thin_Egamma",
          SGKey                  = "Electrons",
          StreamName = streamName,
          InDetTrackParticlesKey = mainIDInput)
    thinList += [BPHY24_Thin_Egamma]
    
       # Primary vertices
    BPHY24_Thin_PV = CompFactory.DerivationFramework.BPhysPVThinningTool( name                 = "BPHY24_Thin_PV",
                                                                 CandidateCollections = finalCandidateList,
                                                                 StreamName = streamName,
                                                                 KeepPVTracks         = True )
    thinList += [ BPHY24_Thin_PV ]

    if isSimulation:

      #Decorate Truth Particles with track parameters
      from InDetPhysValMonitoring.InDetPhysValDecorationConfig import InDetPhysValTruthDecoratorAlgCfg
      acc.merge(InDetPhysValTruthDecoratorAlgCfg(flags, BPHYDerivationName))
    
      # Keep all muons and electrons
      keepParticles = ('abs(TruthParticles.pdgId) == 11 || ' # mu
                       'abs(TruthParticles.pdgId) == 13')    # e
      # Keep only the potentially signal b-hadrons
      
      keepParticles += (' || '
                          'abs(TruthParticles.pdgId) == 511 || ' # B0
                          'abs(TruthParticles.pdgId) == 513 || ' # B0*
                          'abs(TruthParticles.pdgId) == 515')    # B0**
      
    
      BPHY24_Thin_TruthHadrons = CompFactory.DerivationFramework.GenericTruthThinning( name                    = "BPHY24_Thin_TruthHadrons",
                                                                          ParticleSelectionString = keepParticles,
                                                                          PreserveDescendants     = True,
                                                                          StreamName = streamName,
                                                                          PreserveAncestors       = True)
      thinList += [ BPHY24_Thin_TruthHadrons ]
    
      # Save also neutrinos and b-quarks, without their decay trees
      BPHY24_Thin_TruthQuarks = CompFactory.DerivationFramework.GenericTruthThinning( name                    = "BPHY24_Thin_TruthQuarks",
                                                                         ParticleSelectionString = ('abs(TruthParticles.pdgId) ==  5 || '
                                                                                                    'abs(TruthParticles.pdgId) == 12 || abs(TruthParticles.pdgId) == 14' ),
                                                                         PreserveDescendants     = False,
                                                                         StreamName = streamName,
                                                                         PreserveAncestors       = False)
      thinList += [ BPHY24_Thin_TruthQuarks ]

    for t in  augsList + skimList + thinList: acc.addPublicTool(t)
    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel("BPHY24Kernel",
                                                     AugmentationTools = augsList,
                                                     #OutputLevel = DEBUG,
                                                     #Only skim if not MC
                                                     SkimmingTools     = skimList,
                                                     ThinningTools     = thinList))


    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    BPHY24SlimmingHelper = SlimmingHelper("BPHY24SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from AthenaConfiguration.Enums import MetadataCategory
    from DerivationFrameworkBPhys.commonBPHYMethodsCfg import getDefaultAllVariables
    AllVariables  = getDefaultAllVariables()
    StaticContent = []
    ExtraVariables = []
    
    # Smart collections
    # What is the difference w.r.t. adding them into AllVariables?
    # AB Answer: SmarCollections trims commonly unused variables.
    BPHY24SlimmingHelper.SmartCollections = [ "Electrons", "Muons", "InDetTrackParticles", "InDetLargeD0TrackParticles" ]
    
    # Full combined muon-ID tracks
    AllVariables += ["InDetLargeD0TrackParticles"]
    AllVariables += [ "CombinedMuonTrackParticles" ]
    AllVariables += [ "ExtrapolatedMuonTrackParticles" ]
    ExtraVariables += [ "Muons.etaLayer1Hits.etaLayer2Hits.etaLayer3Hits.etaLayer4Hits.phiLayer1Hits.phiLayer2Hits.phiLayer3Hits.phiLayer4Hits",
                    "Muons.numberOfTriggerEtaLayers.numberOfPhiLayers",
                    "InDetTrackParticles.numberOfTRTHits.numberOfTRTHighThresholdHits.vx.vy.vz",
                    "InDetLargeD0TrackParticles.numberOfTRTHits.numberOfTRTHighThresholdHits.vx.vy.vz",
                    "PrimaryVertices.chiSquared.covariance", "Electrons.deltaEta1.DFCommonElectronsLHVeryLoosenod0",
                    "egammaClusters.calE.calEta.calPhi.e_sampl.eta_sampl.etaCalo.phiCalo.ETACALOFRAME.PHICALOFRAME",
                    "HLT_xAOD__ElectronContainer_egamma_ElectronsAuxDyn.charge" ]

    # Include also trigger objects
    # DONE: Test it works (HLT objects appear/not-present)

    BPHY24SlimmingHelper.IncludeMuonTriggerContent  = True
    BPHY24SlimmingHelper.IncludeEgammaTriggerContent  = True
    BPHY24SlimmingHelper.IncludeBPhysTriggerContent = True
    
    # Include primary vertices
    AllVariables  += [ "PrimaryVertices" ]
    print("BPHY24: List of refitted-PV output: ", outRePVList)
    for i in outRePVList:
      StaticContent += [ "xAOD::VertexContainer#%s"        % i ]
      StaticContent += [ "xAOD::VertexAuxContainer#%sAux." % i ]
    
    # B-vertexing output
    print("BPHY24: List of B-vertexing output: ", outVtxList)
    for i in outVtxList:
      StaticContent += [ "xAOD::VertexContainer#%s"                        % i ]
      StaticContent += [ "xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % i ]
    
    print("BPHY24: Full list of B-augmentation: ", StaticContent)
    
    # Truth information for MC only
    if isSimulation:
      AllVariables += [ "TruthEvents",
                        "TruthParticles",
                        "TruthVertices",
                        "MuonTruthParticles" ]
    AllVariables = list(set(AllVariables)) # remove duplicates
    
    BPHY24SlimmingHelper.AllVariables = AllVariables
    BPHY24SlimmingHelper.StaticContent = StaticContent
    BPHY24SlimmingHelper.ExtraVariables = ExtraVariables
    BPHY24ItemList = BPHY24SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_BPHY24", ItemList=BPHY24ItemList, AcceptAlgs=["BPHY24Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_BPHY24", AcceptAlgs=["BPHY24Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))
    acc.printConfig(withDetails=True, summariseProps=True, onlyComponents = [], printDefaults=True)
    return acc
