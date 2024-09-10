# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#====================================================================
# BPHY25.py
# Contact: xin.chen@cern.ch
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

BPHYDerivationName = "BPHY25"
streamName = "StreamDAOD_BPHY25"

def BPHY25Cfg(flags):
    from DerivationFrameworkBPhys.commonBPHYMethodsCfg import (BPHY_V0ToolCfg,  BPHY_InDetDetailedTrackSelectorToolCfg, BPHY_VertexPointEstimatorCfg, BPHY_TrkVKalVrtFitterCfg)
    from JpsiUpsilonTools.JpsiUpsilonToolsConfig import PrimaryVertexRefittingToolCfg
    acc = ComponentAccumulator()
    isSimulation = flags.Input.isMC

    doLRT = True
    # Adds primary vertex counts and track counts to EventInfo before they are thinned
    BPHY25_AugOriginalCounts = CompFactory.DerivationFramework.AugOriginalCounts(
       name              = "BPHY25_AugOriginalCounts",
       VertexContainer   = "PrimaryVertices",
       TrackContainer    = "InDetTrackParticles",
       TrackLRTContainer = "InDetLargeD0TrackParticles" if doLRT else "" )
    acc.addPublicTool(BPHY25_AugOriginalCounts)

    mainIDInput = "InDetWithLRTTrackParticles" if doLRT else "InDetTrackParticles"
    if doLRT:
        from DerivationFrameworkInDet.InDetToolsConfig import InDetLRTMergeCfg
        acc.merge(InDetLRTMergeCfg( flags,
                                    InputTrackParticleLocations = ["InDetTrackParticles", "InDetLargeD0TrackParticles"],
                                    OutputTrackParticleLocation = mainIDInput,
                                    CreateViewColllection       = True ))

    TrkToRelink = ["InDetTrackParticles", "InDetLargeD0TrackParticles"] if doLRT else ["InDetTrackParticles"]

    V0Tools = acc.popToolsAndMerge(BPHY_V0ToolCfg(flags, BPHYDerivationName))
    vkalvrt = acc.popToolsAndMerge(BPHY_TrkVKalVrtFitterCfg(flags, BPHYDerivationName)) # VKalVrt vertex fitter
    acc.addPublicTool(vkalvrt)
    acc.addPublicTool(V0Tools)
    trackselect = acc.popToolsAndMerge(BPHY_InDetDetailedTrackSelectorToolCfg(flags, BPHYDerivationName))
    acc.addPublicTool(trackselect)
    vpest = acc.popToolsAndMerge(BPHY_VertexPointEstimatorCfg(flags, BPHYDerivationName))
    acc.addPublicTool(vpest)
    pvrefitter = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags))
    acc.addPublicTool(pvrefitter)
    from TrkConfig.TrkV0FitterConfig import TrkV0VertexFitter_InDetExtrCfg
    v0fitter = acc.popToolsAndMerge(TrkV0VertexFitter_InDetExtrCfg(flags))
    acc.addPublicTool(v0fitter)
    from TrackToVertex.TrackToVertexConfig import InDetTrackToVertexCfg
    tracktovtxtool = acc.popToolsAndMerge(InDetTrackToVertexCfg(flags))
    acc.addPublicTool(tracktovtxtool)
    from TrkConfig.TrkVKalVrtFitterConfig import V0VKalVrtFitterCfg
    gammafitter = acc.popToolsAndMerge(V0VKalVrtFitterCfg(
        flags, BPHYDerivationName+"_GammaFitter",
        Robustness          = 6,
        usePhiCnst          = True,
        useThetaCnst        = True,
        InputParticleMasses = [0.511,0.511] ))
    acc.addPublicTool(gammafitter)
    from InDetConfig.InDetTrackSelectorToolConfig import V0InDetConversionTrackSelectorToolCfg
    v0trackselect = acc.popToolsAndMerge(V0InDetConversionTrackSelectorToolCfg(flags))
    acc.addPublicTool(v0trackselect)
    from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
    extrapolator = acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags))
    acc.addPublicTool(extrapolator)

    # mass limits and constants used in the following
    Jpsi_lo = 2600.0
    Jpsi_hi = 3500.0
    Psi_lo = 3350.0
    Psi_hi = 4200.0
    etac_lo = 2750.0
    etac_hi = 3150.0
    Dpm_lo = 1690.0
    Dpm_hi = 2050.0
    D0_lo = 1685.0
    D0_hi = 2045.0
    Dstarpm_lo = 1850.0
    Dstarpm_hi = 2150.0
    B_lo = 5000.0
    B_hi = 5550.0
    Bs0_lo = 5080.0
    Bs0_hi = 5650.0
    Bc_lo = 6000.0
    Bc_hi = 6550.0
    Ks_lo = 290.0
    Ks_hi = 690.0
    Ld_lo = 900.0
    Ld_hi = 1350.0
    Xi_lo = 1100.0
    Xi_hi = 1550.0
    Omg_lo = 1450.0
    Omg_hi = 1900.0
    Ldb0_lo = 5310.0
    Ldb0_hi = 5910.0
    Xib_lo = 5500.0
    Xib_hi = 6090.0
    Sigmabp_lo = 5500.0
    Sigmabp_hi = 6100.0
    Sigmabm_lo = 5510.0
    Sigmabm_hi = 6110.0

    Mumass = 105.658
    Pimass = 139.570
    Kmass = 493.677
    Ksmass = 497.611
    Protonmass = 938.2721
    Jpsimass = 3096.916
    Psi2Smass = 3686.10
    etacmass = 2984.1
    Dpmmass = 1869.66
    D0mass = 1864.84
    Dstarpmmass = 2010.26
    Bpmmass = 5279.34
    B0mass = 5279.66
    Bs0mass = 5366.92
    Bcmass = 6274.47
    Lambdamass = 1115.683
    Lambdab0mass = 5619.60
    Ximass = 1321.71
    Omegamass = 1672.45
    Xibmass = 5797.0
    Xib0mass = 5791.9
    Sigmabpmass = 5810.56
    Sigmabmmass = 5815.64

    BPHY25JpsiFinder = CompFactory.Analysis.JpsiFinder(
        name                        = "BPHY25JpsiFinder",
        muAndMu                     = True,
        muAndTrack                  = False,
        TrackAndTrack               = False,
        assumeDiMuons               = True,  # If true, will assume dimu hypothesis and use PDG value for mu mass
        trackThresholdPt            = 3800.,
        invMassLower                = Jpsi_lo,
        invMassUpper                = Psi_hi,
        Chi2Cut                     = 5., # NDF=1 if no mass constraint
        oppChargesOnly	            = True,
        atLeastOneComb              = True,
        useCombinedMeasurement      = False, # Only takes effect if combOnly=True	
        muonCollectionKey           = "Muons",
        TrackParticleCollection     = "InDetTrackParticles",
        V0VertexFitterTool          = None,
        useV0Fitter                 = False, # if False a TrkVertexFitterTool will be used
        TrkVertexFitterTool         = vkalvrt, # VKalVrt vertex fitter
        TrackSelectorTool           = trackselect,
        VertexPointEstimator        = vpest,
        useMCPCuts                  = False )
    acc.addPublicTool(BPHY25JpsiFinder)

    BPHY25_Reco_mumu = CompFactory.DerivationFramework.Reco_Vertex(
        name                   = "BPHY25_Reco_mumu",
        VertexSearchTool       = BPHY25JpsiFinder,
        OutputVtxContainerName = "BPHY25OniaCandidates",
        PVContainerName        = "PrimaryVertices",
        RelinkTracks           = TrkToRelink,
        V0Tools                = V0Tools,
        PVRefitter             = pvrefitter,
        RefitPV                = False,
        DoVertexType           = 0)

    # B+ -> J/psi K
    BPHY25Bpm_Jpsi1Trk = CompFactory.Analysis.JpsiPlus1Track(
        name                                = "BPHY25Bpm_Jpsi1Trk",
        pionHypothesis                      = False,
        kaonHypothesis                      = True,
        trkThresholdPt                      = 950.,
        trkMaxEta                           = 2.6,
        JpsiMassLower                       = Jpsi_lo,
        JpsiMassUpper                       = Jpsi_hi,
        TrkTrippletMassLower                = B_lo,
        TrkTrippletMassUpper                = B_hi,
        Chi2Cut                             = 5.,
        JpsiContainerKey                    = "BPHY25OniaCandidates",
        TrackParticleCollection             = "InDetTrackParticles",
        MuonsUsedInJpsi                     = "Muons",
        ExcludeJpsiMuonsOnly                = True,
        TrkVertexFitterTool                 = vkalvrt,
        TrackSelectorTool                   = trackselect,
        UseMassConstraint                   = False)
    acc.addPublicTool(BPHY25Bpm_Jpsi1Trk)

    # Bs0 -> J/psi + K K
    BPHY25Bs0_Jpsi2Trk = CompFactory.Analysis.JpsiPlus2Tracks(
        name                                = "BPHY25Bs0_Jpsi2Trk",
        kaonkaonHypothesis		    = True,
        pionpionHypothesis                  = False,
        kaonpionHypothesis                  = False,
        kaonprotonHypothesis                = False,
        trkThresholdPt			    = 760.,
        trkMaxEta		 	    = 2.6,
        oppChargesOnly                      = False,
        JpsiMassLower                       = Jpsi_lo,
        JpsiMassUpper                       = Jpsi_hi,
        TrkQuadrupletMassLower              = Bs0_lo,
        TrkQuadrupletMassUpper              = Bs0_hi,
        Chi2Cut                             = 5.,
        JpsiContainerKey                    = "BPHY25OniaCandidates",
        TrackParticleCollection             = "InDetTrackParticles",
        MuonsUsedInJpsi			    = "Muons",
        ExcludeJpsiMuonsOnly                = True,
        TrkVertexFitterTool		    = vkalvrt,
        TrackSelectorTool		    = trackselect,
        UseMassConstraint	            = False)
    acc.addPublicTool(BPHY25Bs0_Jpsi2Trk)

    # B0 -> J/psi + K pi
    BPHY25B0_Jpsi2Trk = CompFactory.Analysis.JpsiPlus2Tracks(
        name                                = "BPHY25B0_Jpsi2Trk",
        kaonkaonHypothesis		    = False,
        pionpionHypothesis                  = False,
        kaonpionHypothesis                  = True,
        kaonprotonHypothesis                = False,
        trkThresholdPt			    = 760.,
        trkMaxEta		 	    = 2.6,
        oppChargesOnly                      = False,
        JpsiMassLower                       = Jpsi_lo,
        JpsiMassUpper                       = Jpsi_hi,
        TrkQuadrupletMassLower              = B_lo,
        TrkQuadrupletMassUpper              = B_hi,
        Chi2Cut                             = 5.,
        JpsiContainerKey                    = "BPHY25OniaCandidates",
        TrackParticleCollection             = "InDetTrackParticles",
        MuonsUsedInJpsi			    = "Muons",
        ExcludeJpsiMuonsOnly                = True,
        TrkVertexFitterTool		    = vkalvrt,
        TrackSelectorTool		    = trackselect,
        UseMassConstraint	            = False)
    acc.addPublicTool(BPHY25B0_Jpsi2Trk)

    BPHY25ThreeTrackReco_Bpm = CompFactory.DerivationFramework.Reco_Vertex(
        name                     = "BPHY25ThreeTrackReco_Bpm",
        VertexSearchTool         = BPHY25Bpm_Jpsi1Trk,
        OutputVtxContainerName   = "BPHY25ThreeTrack_Bpm",
        PVContainerName          = "PrimaryVertices",
        RelinkTracks             = TrkToRelink,
        V0Tools                  = V0Tools,
        PVRefitter               = pvrefitter,
        RefitPV                  = False,
        DoVertexType             = 0)

    BPHY25FourTrackReco_Bs0 = CompFactory.DerivationFramework.Reco_Vertex(
        name                     = "BPHY25FourTrackReco_Bs0",
        VertexSearchTool         = BPHY25Bs0_Jpsi2Trk,
        OutputVtxContainerName   = "BPHY25FourTrack_Bs0",
        PVContainerName          = "PrimaryVertices",
        RelinkTracks             = TrkToRelink,
        V0Tools                  = V0Tools,
        PVRefitter               = pvrefitter,
        RefitPV                  = False,
        DoVertexType             = 0)

    BPHY25FourTrackReco_B0 = CompFactory.DerivationFramework.Reco_Vertex(
        name                     = "BPHY25FourTrackReco_B0",
        VertexSearchTool         = BPHY25B0_Jpsi2Trk,
        OutputVtxContainerName   = "BPHY25FourTrack_B0",
        PVContainerName          = "PrimaryVertices",
        RelinkTracks             = TrkToRelink,
        V0Tools                  = V0Tools,
        PVRefitter               = pvrefitter,
        RefitPV                  = False,
        DoVertexType             = 0)

    BPHY25Select_Jpsi              = CompFactory.DerivationFramework.Select_onia2mumu(
        name                       = "BPHY25Select_Jpsi",
        HypothesisName             = "Jpsi",
        InputVtxContainerName      = "BPHY25OniaCandidates",
        V0Tools                    = V0Tools,
        TrkMasses                  = [Mumass, Mumass],
        MassMin                    = Jpsi_lo,
        MassMax                    = Jpsi_hi,
        DoVertexType               = 0)

    BPHY25Select_Psi               = CompFactory.DerivationFramework.Select_onia2mumu(
        name                       = "BPHY25Select_Psi",
        HypothesisName             = "Psi",
        InputVtxContainerName      = "BPHY25OniaCandidates",
        V0Tools                    = V0Tools,
        TrkMasses                  = [Mumass, Mumass],
        MassMin                    = Psi_lo,
        MassMax                    = Psi_hi,
        DoVertexType               = 0)

    #################################################
    ## J/psi + 0 trk + Xi, Xi^- -> Lambda pi-      ##
    ## Psi + 0 trk + Xi, Xi^- -> Lambda pi-        ##
    ## J/psi + 0 trk + Omega, Omega^- -> Lambda K- ##
    ## Psi + 0 trk + Omega, Omega^- -> Lambda K-   ##
    #################################################

    list_disV_hypo = ["JpsiXi", "PsiXi", "JpsiOmg", "PsiOmg"]
    list_disV_jxHypo = ["Jpsi", "Psi", "Jpsi", "Psi"]
    list_disV_disVLo = [Xi_lo, Xi_lo, Omg_lo, Omg_lo]
    list_disV_disVHi = [Xi_hi, Xi_hi, Omg_hi, Omg_hi]
    list_disV_disVDau3Mass = [Pimass, Pimass, Kmass, Kmass]
    list_disV_jxMass = [Jpsimass, Psi2Smass, Jpsimass, Psi2Smass]
    list_disV_disVMass = [Ximass, Ximass, Omegamass, Omegamass]

    list_disV_obj = []
    for hypo in list_disV_hypo:
        list_disV_obj.append( CompFactory.DerivationFramework.JpsiXPlusDisplaced("BPHY25_"+hypo) )

    for i in range(len(list_disV_obj)):
        list_disV_obj[i].JXVertices               = "BPHY25OniaCandidates"
        list_disV_obj[i].JXVtxHypoNames           = [list_disV_jxHypo[i]]
        list_disV_obj[i].TrackParticleCollection  = mainIDInput
        list_disV_obj[i].RelinkTracks             = TrkToRelink
        if i == 0:
            # create V0 container for all following instances
            list_disV_obj[i].OutoutV0VtxCollection    = "V0Collection"
        else:
            list_disV_obj[i].V0Vertices               = "V0Collection"
        list_disV_obj[i].V0MassLowerCut           = Ld_lo
        list_disV_obj[i].V0MassUpperCut           = Ld_hi
        list_disV_obj[i].DisplacedMassLowerCut    = list_disV_disVLo[i]
        list_disV_obj[i].DisplacedMassUpperCut    = list_disV_disVHi[i]
        list_disV_obj[i].CascadeVertexCollections = ["BPHY25_"+list_disV_hypo[i]+"_CascadeVtx1_sub","BPHY25_"+list_disV_hypo[i]+"_CascadeVtx1","BPHY25_"+list_disV_hypo[i]+"_CascadeMainVtx"]
        list_disV_obj[i].HasJXSubVertex           = False
        list_disV_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
        list_disV_obj[i].V0Hypothesis             = "Lambda"
        list_disV_obj[i].MassCutGamma             = 10.
        list_disV_obj[i].Chi2CutGamma             = 3.
        list_disV_obj[i].LxyV0Cut                 = 10.
        list_disV_obj[i].LxyDisVtxCut             = 10.
        list_disV_obj[i].HypothesisName           = list_disV_hypo[i]
        list_disV_obj[i].NumberOfJXDaughters      = 2
        list_disV_obj[i].JXDaug1MassHypo          = Mumass
        list_disV_obj[i].JXDaug2MassHypo          = Mumass
        list_disV_obj[i].NumberOfDisVDaughters    = 3
        list_disV_obj[i].DisVDaug3MassHypo        = list_disV_disVDau3Mass[i]
        list_disV_obj[i].DisVDaug3MinPt           = 850.
        list_disV_obj[i].JpsiMass                 = list_disV_jxMass[i]
        list_disV_obj[i].V0Mass                   = Lambdamass
        list_disV_obj[i].DisVtxMass               = list_disV_disVMass[i]
        list_disV_obj[i].ApplyJpsiMassConstraint  = True
        list_disV_obj[i].ApplyV0MassConstraint    = True
        list_disV_obj[i].ApplyDisVMassConstraint  = True
        list_disV_obj[i].Chi2CutV0                = 5.
        list_disV_obj[i].Chi2CutDisV              = 5.
        list_disV_obj[i].Chi2Cut                  = 5.
        list_disV_obj[i].Trackd0Cut               = 2.5
        list_disV_obj[i].MaxJXCandidates          = 10
        list_disV_obj[i].MaxV0Candidates          = 20
        list_disV_obj[i].MaxDisVCandidates        = 30
        list_disV_obj[i].MaxMainVCandidates       = 30
        list_disV_obj[i].RefitPV                  = True
        list_disV_obj[i].MaxnPV                   = 50
        list_disV_obj[i].RefPVContainerName       = "BPHY25_"+list_disV_hypo[i]+"_RefPrimaryVertices"
        list_disV_obj[i].TrkVertexFitterTool      = vkalvrt
        list_disV_obj[i].V0VertexFitterTool       = v0fitter
        list_disV_obj[i].GammaFitterTool          = gammafitter
        list_disV_obj[i].PVRefitter               = pvrefitter
        list_disV_obj[i].V0Tools                  = V0Tools
        list_disV_obj[i].TrackToVertexTool        = tracktovtxtool
        list_disV_obj[i].V0TrackSelectorTool      = v0trackselect
        list_disV_obj[i].TrackSelectorTool        = trackselect
        list_disV_obj[i].Extrapolator             = extrapolator

    #############################
    ## J/psi + tracks + Lambda ##
    #############################

    list_trkLd_hypo = ["BpmLd", "Bs2KLd", "B0KpiLd", "B0piKLd"]
    list_trkLd_jxInput = ["BPHY25ThreeTrack_Bpm", "BPHY25FourTrack_Bs0", "BPHY25FourTrack_B0", "BPHY25FourTrack_B0"]
    list_trkLd_jxMass = [Bpmmass, Bs0mass, B0mass, B0mass]
    list_trkLd_jxDau3Mass = [Kmass, Kmass, Kmass, Pimass]
    list_trkLd_jxDau4Mass = [0, Kmass, Pimass, Kmass] # 0 is dummy
    list_trkLd_jxMassLo = [B_lo, Bs0_lo, B_lo, B_lo]
    list_trkLd_jxMassHi = [B_hi, Bs0_hi, B_hi, B_hi]

    list_trkLd_obj = []
    for hypo in list_trkLd_hypo:
        list_trkLd_obj.append( CompFactory.DerivationFramework.JpsiXPlusDisplaced("BPHY25_"+hypo) )

    for i in range(len(list_trkLd_obj)):
        list_trkLd_obj[i].JXVertices               = list_trkLd_jxInput[i]
        list_trkLd_obj[i].TrackParticleCollection  = mainIDInput
        list_trkLd_obj[i].RelinkTracks             = TrkToRelink
        list_trkLd_obj[i].V0Vertices               = "V0Collection"
        list_trkLd_obj[i].V0MassLowerCut           = Ld_lo
        list_trkLd_obj[i].V0MassUpperCut           = Ld_hi
        list_trkLd_obj[i].CascadeVertexCollections = ["BPHY25_"+list_trkLd_hypo[i]+"_CascadeVtx1","BPHY25_"+list_trkLd_hypo[i]+"_CascadeVtx2","BPHY25_"+list_trkLd_hypo[i]+"_CascadeMainVtx"]
        list_trkLd_obj[i].HasJXSubVertex           = True
        list_trkLd_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
        list_trkLd_obj[i].V0Hypothesis             = "Lambda"
        list_trkLd_obj[i].MassCutGamma             = 10.
        list_trkLd_obj[i].Chi2CutGamma             = 3.
        list_trkLd_obj[i].LxyV0Cut                 = 20.
        list_trkLd_obj[i].HypothesisName           = list_trkLd_hypo[i]
        if i == 0:
            list_trkLd_obj[i].NumberOfJXDaughters      = 3
        else:
            list_trkLd_obj[i].NumberOfJXDaughters      = 4
            list_trkLd_obj[i].JXDaug4MassHypo          = list_trkLd_jxDau4Mass[i]
        list_trkLd_obj[i].JXDaug1MassHypo          = Mumass
        list_trkLd_obj[i].JXDaug2MassHypo          = Mumass
        list_trkLd_obj[i].JXDaug3MassHypo          = list_trkLd_jxDau3Mass[i]
        list_trkLd_obj[i].JXPtOrdering             = True
        list_trkLd_obj[i].JXMassLowerCut           = list_trkLd_jxMassLo[i]
        list_trkLd_obj[i].JXMassUpperCut           = list_trkLd_jxMassHi[i]
        list_trkLd_obj[i].JpsiMassLowerCut         = Jpsi_lo
        list_trkLd_obj[i].JpsiMassUpperCut         = Jpsi_hi
        list_trkLd_obj[i].NumberOfDisVDaughters    = 2
        list_trkLd_obj[i].JXMass                   = list_trkLd_jxMass[i]
        list_trkLd_obj[i].JpsiMass                 = Jpsimass
        list_trkLd_obj[i].V0Mass                   = Lambdamass
        list_trkLd_obj[i].ApplyJXMassConstraint    = True
        list_trkLd_obj[i].ApplyJpsiMassConstraint  = True
        list_trkLd_obj[i].ApplyV0MassConstraint    = True
        list_trkLd_obj[i].Chi2CutV0                = 5.
        list_trkLd_obj[i].Chi2Cut                  = 5.
        list_trkLd_obj[i].Trackd0Cut               = 2.5
        list_trkLd_obj[i].MaxJXCandidates          = 20
        list_trkLd_obj[i].MaxV0Candidates          = 20
        list_trkLd_obj[i].MaxMainVCandidates       = 30
        list_trkLd_obj[i].RefitPV                  = True
        list_trkLd_obj[i].MaxnPV                   = 50
        list_trkLd_obj[i].RefPVContainerName       = "BPHY25_"+list_trkLd_hypo[i]+"_RefPrimaryVertices"
        list_trkLd_obj[i].TrkVertexFitterTool      = vkalvrt
        list_trkLd_obj[i].V0VertexFitterTool       = v0fitter
        list_trkLd_obj[i].GammaFitterTool          = gammafitter
        list_trkLd_obj[i].PVRefitter               = pvrefitter
        list_trkLd_obj[i].V0Tools                  = V0Tools
        list_trkLd_obj[i].TrackToVertexTool        = tracktovtxtool
        list_trkLd_obj[i].V0TrackSelectorTool      = v0trackselect
        list_trkLd_obj[i].TrackSelectorTool        = trackselect
        list_trkLd_obj[i].Extrapolator             = extrapolator

    ########################################
    ## X -> J/psi etac, etac -> Ks K+ pi- ##
    ## X -> J/psi etac, etac -> Ks pi+ K- ##
    ########################################

    list_jpsietac_hypo = ["JpsiEtac_Kpi", "JpsiEtac_piK"]
    list_jpsietac_extraTrk1Mass = [Kmass, Pimass]
    list_jpsietac_extraTrk2Mass = [Pimass, Kmass]

    list_jpsietac_obj = []
    for hypo in list_jpsietac_hypo:
        list_jpsietac_obj.append( CompFactory.DerivationFramework.JpsiXPlusDisplaced("BPHY25_"+hypo) )

    for i in range(len(list_jpsietac_obj)):
        list_jpsietac_obj[i].JXVertices                 = "BPHY25OniaCandidates"
        list_jpsietac_obj[i].JXVtxHypoNames             = ["Jpsi"]
        list_jpsietac_obj[i].TrackParticleCollection    = mainIDInput
        list_jpsietac_obj[i].RelinkTracks               = TrkToRelink
        list_jpsietac_obj[i].V0Vertices                 = "V0Collection"
        list_jpsietac_obj[i].V0MassLowerCut             = Ks_lo
        list_jpsietac_obj[i].V0MassUpperCut             = Ks_hi
        list_jpsietac_obj[i].CascadeVertexCollections   = ["BPHY25_"+list_jpsietac_hypo[i]+"_CascadeVtx1","BPHY25_"+list_jpsietac_hypo[i]+"_CascadeMainVtx"]
        list_jpsietac_obj[i].HasJXSubVertex             = False
        list_jpsietac_obj[i].VxPrimaryCandidateName     = "PrimaryVertices"
        list_jpsietac_obj[i].V0Hypothesis               = "Ks"
        list_jpsietac_obj[i].MassCutGamma               = 10.
        list_jpsietac_obj[i].Chi2CutGamma               = 3.
        list_jpsietac_obj[i].LxyV0Cut                   = 20.
        list_jpsietac_obj[i].HypothesisName             = list_jpsietac_hypo[i]
        list_jpsietac_obj[i].NumberOfJXDaughters        = 2
        list_jpsietac_obj[i].JXDaug1MassHypo            = Mumass
        list_jpsietac_obj[i].JXDaug2MassHypo            = Mumass
        list_jpsietac_obj[i].NumberOfDisVDaughters      = 2
        list_jpsietac_obj[i].ExtraTrack1MassHypo        = list_jpsietac_extraTrk1Mass[i]
        list_jpsietac_obj[i].ExtraTrack1MinPt           = 1430.
        list_jpsietac_obj[i].ExtraTrack2MassHypo        = list_jpsietac_extraTrk2Mass[i]
        list_jpsietac_obj[i].ExtraTrack2MinPt           = 1430.
        list_jpsietac_obj[i].V0ExtraMassLowerCut        = etac_lo
        list_jpsietac_obj[i].V0ExtraMassUpperCut        = etac_hi
        list_jpsietac_obj[i].MaxMesonCandidates         = 20
        list_jpsietac_obj[i].MesonPtOrdering            = True
        list_jpsietac_obj[i].V0ExtraMass                = etacmass
        list_jpsietac_obj[i].JpsiMass                   = Jpsimass
        list_jpsietac_obj[i].V0Mass                     = Ksmass
        list_jpsietac_obj[i].ApplyJpsiMassConstraint    = True
        list_jpsietac_obj[i].ApplyV0MassConstraint      = True
        list_jpsietac_obj[i].ApplyV0ExtraMassConstraint = True
        list_jpsietac_obj[i].Chi2CutV0                  = 4.
        list_jpsietac_obj[i].Chi2CutV0Extra             = 4.
        list_jpsietac_obj[i].Chi2Cut                    = 4.
        list_jpsietac_obj[i].Trackd0Cut                 = 2.5
        list_jpsietac_obj[i].MaxJXCandidates            = 10
        list_jpsietac_obj[i].MaxV0Candidates            = 20
        list_jpsietac_obj[i].MaxMainVCandidates         = 30
        list_jpsietac_obj[i].RefitPV                    = True
        list_jpsietac_obj[i].MaxnPV                     = 50
        list_jpsietac_obj[i].RefPVContainerName         = "BPHY25_"+list_jpsietac_hypo[i]+"_RefPrimaryVertices"
        list_jpsietac_obj[i].TrkVertexFitterTool        = vkalvrt
        list_jpsietac_obj[i].V0VertexFitterTool         = v0fitter
        list_jpsietac_obj[i].GammaFitterTool            = gammafitter
        list_jpsietac_obj[i].PVRefitter                 = pvrefitter
        list_jpsietac_obj[i].V0Tools                    = V0Tools
        list_jpsietac_obj[i].TrackToVertexTool          = tracktovtxtool
        list_jpsietac_obj[i].V0TrackSelectorTool        = v0trackselect
        list_jpsietac_obj[i].TrackSelectorTool          = trackselect
        list_jpsietac_obj[i].Extrapolator               = extrapolator

    ######################################################
    ## Bc+ -> J/psi D*+ Ks, D*+ -> D0 pi+ -> K- pi+ pi+ ##
    ## Bc+ -> J/psi D+ Ks, D+ -> K- pi+  pi+            ##
    ######################################################

    list_3bodyA_hypo = ["Bc_3body"]
    list_3bodyA_extraTrk1Mass = [Kmass]
    list_3bodyA_extraTrk2Mass = [Pimass]
    list_3bodyA_extraTrk3Mass = [Pimass]

    list_3bodyA_obj = []
    for hypo in list_3bodyA_hypo:
        list_3bodyA_obj.append( CompFactory.DerivationFramework.JpsiXPlusDisplaced("BPHY25_"+hypo) )

    for i in range(len(list_3bodyA_obj)):
        list_3bodyA_obj[i].JXVertices                 = "BPHY25OniaCandidates"
        list_3bodyA_obj[i].JXVtxHypoNames             = ["Jpsi"]
        list_3bodyA_obj[i].TrackParticleCollection    = mainIDInput
        list_3bodyA_obj[i].RelinkTracks               = TrkToRelink
        list_3bodyA_obj[i].V0Vertices                 = "V0Collection"
        list_3bodyA_obj[i].V0MassLowerCut             = Ks_lo
        list_3bodyA_obj[i].V0MassUpperCut             = Ks_hi
        list_3bodyA_obj[i].CascadeVertexCollections   = ["BPHY25_"+list_3bodyA_hypo[i]+"_CascadeVtx1","BPHY25_"+list_3bodyA_hypo[i]+"_CascadeVtx2","BPHY25_"+list_3bodyA_hypo[i]+"_CascadeMainVtx"]
        list_3bodyA_obj[i].VxPrimaryCandidateName     = "PrimaryVertices"
        list_3bodyA_obj[i].V0Hypothesis               = "Ks"
        list_3bodyA_obj[i].MassCutGamma               = 10.
        list_3bodyA_obj[i].Chi2CutGamma               = 3.
        list_3bodyA_obj[i].LxyV0Cut                   = 20.
        list_3bodyA_obj[i].HypothesisName             = list_3bodyA_hypo[i]
        list_3bodyA_obj[i].NumberOfJXDaughters        = 2
        list_3bodyA_obj[i].JXDaug1MassHypo            = Mumass
        list_3bodyA_obj[i].JXDaug2MassHypo            = Mumass
        list_3bodyA_obj[i].NumberOfDisVDaughters      = 2
        list_3bodyA_obj[i].ExtraTrack1MassHypo        = list_3bodyA_extraTrk1Mass[i]
        list_3bodyA_obj[i].ExtraTrack1MinPt           = 1810.
        list_3bodyA_obj[i].ExtraTrack2MassHypo        = list_3bodyA_extraTrk2Mass[i]
        list_3bodyA_obj[i].ExtraTrack2MinPt           = 1520.
        list_3bodyA_obj[i].ExtraTrack3MassHypo        = list_3bodyA_extraTrk3Mass[i]
        list_3bodyA_obj[i].ExtraTrack3MinPt           = 1520.
        list_3bodyA_obj[i].LxyDpmCut                  = 0.25
        list_3bodyA_obj[i].LxyD0Cut                   = 0.15
        list_3bodyA_obj[i].DpmMassLowerCut            = Dpm_lo
        list_3bodyA_obj[i].DpmMassUpperCut            = Dpm_hi
        list_3bodyA_obj[i].D0MassLowerCut             = D0_lo
        list_3bodyA_obj[i].D0MassUpperCut             = D0_hi
        list_3bodyA_obj[i].DstarpmMassLowerCut        = Dstarpm_lo
        list_3bodyA_obj[i].DstarpmMassUpperCut        = Dstarpm_hi
        list_3bodyA_obj[i].MaxMesonCandidates         = 30
        list_3bodyA_obj[i].MesonPtOrdering            = True
        list_3bodyA_obj[i].MassLowerCut               = Bc_lo
        list_3bodyA_obj[i].MassUpperCut               = Bc_hi
        list_3bodyA_obj[i].DpmMass                    = Dpmmass
        list_3bodyA_obj[i].D0Mass                     = D0mass
        list_3bodyA_obj[i].DstarpmMass                = Dstarpmmass
        list_3bodyA_obj[i].JpsiMass                   = Jpsimass
        list_3bodyA_obj[i].V0Mass                     = Ksmass
        list_3bodyA_obj[i].MainVtxMass                = Bcmass
        list_3bodyA_obj[i].ApplyJpsiMassConstraint    = True
        list_3bodyA_obj[i].ApplyV0MassConstraint      = True
        list_3bodyA_obj[i].ApplyDpmMassConstraint     = True
        list_3bodyA_obj[i].ApplyD0MassConstraint      = True
        list_3bodyA_obj[i].ApplyDstarpmMassConstraint = True
        list_3bodyA_obj[i].ApplyMainVMassConstraint   = True
        list_3bodyA_obj[i].Chi2CutV0                  = 4.
        list_3bodyA_obj[i].Chi2CutJXDpm               = 3.
        list_3bodyA_obj[i].Chi2CutJXDstarpm           = 3.
        list_3bodyA_obj[i].Chi2Cut                    = 4.
        list_3bodyA_obj[i].Trackd0Cut                 = 2.5
        list_3bodyA_obj[i].MaxJXCandidates            = 10
        list_3bodyA_obj[i].MaxV0Candidates            = 20
        list_3bodyA_obj[i].MaxMainVCandidates         = 30
        list_3bodyA_obj[i].RefitPV                    = True
        list_3bodyA_obj[i].MaxnPV                     = 50
        list_3bodyA_obj[i].RefPVContainerName         = "BPHY25_"+list_3bodyA_hypo[i]+"_RefPrimaryVertices"
        list_3bodyA_obj[i].TrkVertexFitterTool        = vkalvrt
        list_3bodyA_obj[i].V0VertexFitterTool         = v0fitter
        list_3bodyA_obj[i].GammaFitterTool            = gammafitter
        list_3bodyA_obj[i].PVRefitter                 = pvrefitter
        list_3bodyA_obj[i].V0Tools                    = V0Tools
        list_3bodyA_obj[i].TrackToVertexTool          = tracktovtxtool
        list_3bodyA_obj[i].V0TrackSelectorTool        = v0trackselect
        list_3bodyA_obj[i].TrackSelectorTool          = trackselect
        list_3bodyA_obj[i].Extrapolator               = extrapolator

    ##################################
    ## B- -> J/psi Lambda pbar      ##
    ## Xi_b- -> J/psi Lambda K-     ##
    ## Sigma_b+ -> J/psi Lambda pi+ ##
    ## Sigma_b- -> J/psi Lambda pi- ##
    ##################################

    list_3bodyB_hypo = ["Bpm_3body", "Xib_3body", "Sigmabp_3body", "Sigmabm_3body"]
    list_3bodyB_extraTrkMass = [Protonmass, Kmass, Pimass, Pimass]
    list_3bodyB_mainVtxMass = [Bpmmass, Xibmass, Sigmabpmass, Sigmabmmass]
    list_3bodyB_massLo = [B_lo, Xib_lo, Sigmabp_lo, Sigmabm_lo]
    list_3bodyB_massHi = [B_hi, Xib_hi, Sigmabp_hi, Sigmabm_hi]

    list_3bodyB_obj = []
    for hypo in list_3bodyB_hypo:
        list_3bodyB_obj.append( CompFactory.DerivationFramework.JpsiXPlusDisplaced("BPHY25_"+hypo) )

    for i in range(len(list_3bodyB_obj)):
        list_3bodyB_obj[i].JXVertices               = "BPHY25OniaCandidates"
        list_3bodyB_obj[i].JXVtxHypoNames           = ["Jpsi"]
        list_3bodyB_obj[i].TrackParticleCollection  = mainIDInput
        list_3bodyB_obj[i].RelinkTracks             = TrkToRelink
        list_3bodyB_obj[i].V0Vertices               = "V0Collection"
        list_3bodyB_obj[i].V0MassLowerCut           = Ld_lo
        list_3bodyB_obj[i].V0MassUpperCut           = Ld_hi
        list_3bodyB_obj[i].CascadeVertexCollections = ["BPHY25_"+list_3bodyB_hypo[i]+"_CascadeVtx1","BPHY25_"+list_3bodyB_hypo[i]+"_CascadeMainVtx"]
        list_3bodyB_obj[i].HasJXSubVertex           = False
        list_3bodyB_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
        list_3bodyB_obj[i].V0Hypothesis             = "Lambda"
        list_3bodyB_obj[i].MassCutGamma             = 10.
        list_3bodyB_obj[i].Chi2CutGamma             = 3.
        list_3bodyB_obj[i].LxyV0Cut                 = 20.
        list_3bodyB_obj[i].HypothesisName           = list_3bodyB_hypo[i]
        list_3bodyB_obj[i].NumberOfJXDaughters      = 2
        list_3bodyB_obj[i].JXDaug1MassHypo          = Mumass
        list_3bodyB_obj[i].JXDaug2MassHypo          = Mumass
        list_3bodyB_obj[i].NumberOfDisVDaughters    = 2
        list_3bodyB_obj[i].ExtraTrack1MassHypo      = list_3bodyB_extraTrkMass[i]
        list_3bodyB_obj[i].ExtraTrack1MinPt         = 480.
        list_3bodyB_obj[i].MassLowerCut             = list_3bodyB_massLo[i]
        list_3bodyB_obj[i].MassUpperCut             = list_3bodyB_massHi[i]
        list_3bodyB_obj[i].JpsiMass                 = Jpsimass
        list_3bodyB_obj[i].V0Mass                   = Lambdamass
        list_3bodyB_obj[i].MainVtxMass              = list_3bodyB_mainVtxMass[i]
        list_3bodyB_obj[i].ApplyJpsiMassConstraint  = True
        list_3bodyB_obj[i].ApplyV0MassConstraint    = True
        list_3bodyB_obj[i].ApplyMainVMassConstraint = True
        list_3bodyB_obj[i].Chi2CutV0                = 5.
        list_3bodyB_obj[i].Chi2Cut                  = 5.
        list_3bodyB_obj[i].Trackd0Cut               = 2.5
        list_3bodyB_obj[i].MaxJXCandidates          = 10
        list_3bodyB_obj[i].MaxV0Candidates          = 20
        list_3bodyB_obj[i].MaxMainVCandidates       = 30
        list_3bodyB_obj[i].RefitPV                  = True
        list_3bodyB_obj[i].MaxnPV                   = 50
        list_3bodyB_obj[i].RefPVContainerName       = "BPHY25_"+list_3bodyB_hypo[i]+"_RefPrimaryVertices"
        list_3bodyB_obj[i].TrkVertexFitterTool      = vkalvrt
        list_3bodyB_obj[i].V0VertexFitterTool       = v0fitter
        list_3bodyB_obj[i].GammaFitterTool          = gammafitter
        list_3bodyB_obj[i].PVRefitter               = pvrefitter
        list_3bodyB_obj[i].V0Tools                  = V0Tools
        list_3bodyB_obj[i].TrackToVertexTool        = tracktovtxtool
        list_3bodyB_obj[i].V0TrackSelectorTool      = v0trackselect
        list_3bodyB_obj[i].TrackSelectorTool        = trackselect
        list_3bodyB_obj[i].Extrapolator             = extrapolator

    ####################################################
    ## Xib^0 -> J/psi Xi^- pi+, Xi^- -> Lambda pi-    ##
    ## Lambdab^0 -> J/psi Xi^- K+, Xi^- -> Lambda pi- ##
    ####################################################

    list_3bodyC_hypo = ["Xib0_3body", "Lambdab0_3body"]
    list_3bodyC_extraTrkMass = [Pimass, Kmass]
    list_3bodyC_mainVtxMass = [Xib0mass, Lambdab0mass]
    list_3bodyC_massLo = [Xib_lo, Ldb0_lo]
    list_3bodyC_massHi = [Xib_hi, Ldb0_hi]

    list_3bodyC_obj = []
    for hypo in list_3bodyC_hypo:
        list_3bodyC_obj.append( CompFactory.DerivationFramework.JpsiXPlusDisplaced("BPHY25_"+hypo) )

    for i in range(len(list_3bodyC_obj)):
        list_3bodyC_obj[i].JXVertices               = "BPHY25OniaCandidates"
        list_3bodyC_obj[i].JXVtxHypoNames           = ["Jpsi"]
        list_3bodyC_obj[i].TrackParticleCollection  = mainIDInput
        list_3bodyC_obj[i].RelinkTracks             = TrkToRelink
        list_3bodyC_obj[i].V0Vertices               = "V0Collection"
        list_3bodyC_obj[i].V0MassLowerCut           = Ld_lo
        list_3bodyC_obj[i].V0MassUpperCut           = Ld_hi
        list_3bodyC_obj[i].DisplacedMassLowerCut    = Xi_lo
        list_3bodyC_obj[i].DisplacedMassUpperCut    = Xi_hi
        list_3bodyC_obj[i].CascadeVertexCollections = ["BPHY25_"+list_3bodyC_hypo[i]+"_CascadeVtx1_sub","BPHY25_"+list_3bodyC_hypo[i]+"_CascadeVtx1","BPHY25_"+list_3bodyC_hypo[i]+"_CascadeMainVtx"]
        list_3bodyC_obj[i].HasJXSubVertex           = False
        list_3bodyC_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
        list_3bodyC_obj[i].V0Hypothesis             = "Lambda"
        list_3bodyC_obj[i].MassCutGamma             = 10.
        list_3bodyC_obj[i].Chi2CutGamma             = 3.
        list_3bodyC_obj[i].LxyV0Cut                 = 10.
        list_3bodyC_obj[i].LxyDisVtxCut             = 10.
        list_3bodyC_obj[i].HypothesisName           = list_3bodyC_hypo[i]
        list_3bodyC_obj[i].NumberOfJXDaughters      = 2
        list_3bodyC_obj[i].JXDaug1MassHypo          = Mumass
        list_3bodyC_obj[i].JXDaug2MassHypo          = Mumass
        list_3bodyC_obj[i].NumberOfDisVDaughters    = 3
        list_3bodyC_obj[i].DisVDaug3MassHypo        = Pimass
        list_3bodyC_obj[i].DisVDaug3MinPt           = 480.
        list_3bodyC_obj[i].ExtraTrack1MassHypo      = list_3bodyC_extraTrkMass[i]
        list_3bodyC_obj[i].ExtraTrack1MinPt         = 480.
        list_3bodyC_obj[i].MassLowerCut             = list_3bodyC_massLo[i]
        list_3bodyC_obj[i].MassUpperCut             = list_3bodyC_massHi[i]
        list_3bodyC_obj[i].JpsiMass                 = Jpsimass
        list_3bodyC_obj[i].V0Mass                   = Lambdamass
        list_3bodyC_obj[i].DisVtxMass               = Ximass
        list_3bodyC_obj[i].MainVtxMass              = list_3bodyC_mainVtxMass[i]
        list_3bodyC_obj[i].ApplyJpsiMassConstraint  = True
        list_3bodyC_obj[i].ApplyV0MassConstraint    = True
        list_3bodyC_obj[i].ApplyDisVMassConstraint  = True
        list_3bodyC_obj[i].ApplyMainVMassConstraint = True
        list_3bodyC_obj[i].Chi2CutV0                = 5.
        list_3bodyC_obj[i].Chi2CutDisV              = 5.
        list_3bodyC_obj[i].Chi2Cut                  = 5.
        list_3bodyC_obj[i].Trackd0Cut               = 2.5
        list_3bodyC_obj[i].MaxJXCandidates          = 10
        list_3bodyC_obj[i].MaxV0Candidates          = 20
        list_3bodyC_obj[i].MaxDisVCandidates        = 30
        list_3bodyC_obj[i].MaxMainVCandidates       = 30
        list_3bodyC_obj[i].RefitPV                  = True
        list_3bodyC_obj[i].MaxnPV                   = 50
        list_3bodyC_obj[i].RefPVContainerName       = "BPHY25_"+list_3bodyC_hypo[i]+"_RefPrimaryVertices"
        list_3bodyC_obj[i].TrkVertexFitterTool      = vkalvrt
        list_3bodyC_obj[i].V0VertexFitterTool       = v0fitter
        list_3bodyC_obj[i].GammaFitterTool          = gammafitter
        list_3bodyC_obj[i].PVRefitter               = pvrefitter
        list_3bodyC_obj[i].V0Tools                  = V0Tools
        list_3bodyC_obj[i].TrackToVertexTool        = tracktovtxtool
        list_3bodyC_obj[i].V0TrackSelectorTool      = v0trackselect
        list_3bodyC_obj[i].TrackSelectorTool        = trackselect
        list_3bodyC_obj[i].Extrapolator             = extrapolator

    ###################################
    ## Bs0 -> J/psi Lambda Lambdabar ##
    ## Xib^0 -> J/psi Lambda Ks      ##
    ###################################

    list_2V0_hypo = ["Bs0_2V0", "Xib0_2V0"]
    list_2V0_v02hypo = ["Lambda", "Ks"]
    list_2V0_v02mass = [Lambdamass, Ksmass]
    list_2V0_v02MassLo = [Ld_lo, Ks_lo]
    list_2V0_v02MassHi = [Ld_hi, Ks_hi]
    list_2V0_mainVtxMass = [Bs0mass, Xib0mass]
    list_2V0_massLo = [Bs0_lo, Xib_lo]
    list_2V0_massHi = [Bs0_hi, Xib_hi]

    list_2V0_obj = []
    for hypo in list_2V0_hypo:
        list_2V0_obj.append( CompFactory.DerivationFramework.JpsiXPlus2V0("BPHY25_"+hypo) )

    for i in range(len(list_2V0_obj)):
        list_2V0_obj[i].JXVertices               = "BPHY25OniaCandidates"
        list_2V0_obj[i].JXVtxHypoNames           = ["Jpsi"]
        list_2V0_obj[i].TrackParticleCollection  = mainIDInput
        list_2V0_obj[i].RelinkTracks             = TrkToRelink
        list_2V0_obj[i].V0Vertices               = "V0Collection"
        list_2V0_obj[i].CascadeVertexCollections = ["BPHY25_"+list_2V0_hypo[i]+"_CascadeVtx1","BPHY25_"+list_2V0_hypo[i]+"_CascadeVtx2","BPHY25_"+list_2V0_hypo[i]+"_CascadeMainVtx"]
        list_2V0_obj[i].HasJXSubVertex           = False
        list_2V0_obj[i].HasJXV02SubVertex        = False
        list_2V0_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
        list_2V0_obj[i].V01Hypothesis            = "Lambda"
        list_2V0_obj[i].V01MassLowerCut          = Ld_lo
        list_2V0_obj[i].V01MassUpperCut          = Ld_hi
        list_2V0_obj[i].LxyV01Cut                = 20.
        list_2V0_obj[i].V02Hypothesis            = list_2V0_v02hypo[i]
        list_2V0_obj[i].V02MassLowerCut          = list_2V0_v02MassLo[i]
        list_2V0_obj[i].V02MassUpperCut          = list_2V0_v02MassHi[i]
        list_2V0_obj[i].LxyV02Cut                = 20.
        list_2V0_obj[i].MassCutGamma             = 10.
        list_2V0_obj[i].Chi2CutGamma             = 3.
        list_2V0_obj[i].HypothesisName           = list_2V0_hypo[i]
        list_2V0_obj[i].NumberOfJXDaughters      = 2
        list_2V0_obj[i].JXDaug1MassHypo          = Mumass
        list_2V0_obj[i].JXDaug2MassHypo          = Mumass
        list_2V0_obj[i].MassLowerCut             = list_2V0_massLo[i]
        list_2V0_obj[i].MassUpperCut             = list_2V0_massHi[i]
        list_2V0_obj[i].JpsiMass                 = Jpsimass
        list_2V0_obj[i].V01Mass                  = Lambdamass
        list_2V0_obj[i].V02Mass                  = list_2V0_v02mass[i]
        list_2V0_obj[i].MainVtxMass              = list_2V0_mainVtxMass[i]
        list_2V0_obj[i].ApplyJpsiMassConstraint  = True
        list_2V0_obj[i].ApplyV01MassConstraint   = True
        list_2V0_obj[i].ApplyV02MassConstraint   = True
        list_2V0_obj[i].ApplyMainVMassConstraint = True
        list_2V0_obj[i].Chi2CutV0                = 5.
        list_2V0_obj[i].Chi2Cut                  = 5.
        list_2V0_obj[i].Trackd0Cut               = 2.5
        list_2V0_obj[i].MaxJXCandidates          = 10
        list_2V0_obj[i].MaxV0Candidates          = 20
        list_2V0_obj[i].MaxMainVCandidates       = 30
        list_2V0_obj[i].RefitPV                  = True
        list_2V0_obj[i].MaxnPV                   = 50
        list_2V0_obj[i].RefPVContainerName       = "BPHY25_"+list_2V0_hypo[i]+"_RefPrimaryVertices"
        list_2V0_obj[i].TrkVertexFitterTool      = vkalvrt
        list_2V0_obj[i].V0VertexFitterTool       = v0fitter
        list_2V0_obj[i].GammaFitterTool          = gammafitter
        list_2V0_obj[i].PVRefitter               = pvrefitter
        list_2V0_obj[i].V0Tools                  = V0Tools
        list_2V0_obj[i].TrackToVertexTool        = tracktovtxtool
        list_2V0_obj[i].V0TrackSelectorTool      = v0trackselect
        list_2V0_obj[i].Extrapolator             = extrapolator

    ##############################################################
    ## Xi_bc^0 -> Lambda_b^0 + Ks, Lambda_b^0 -> J/psi + Lambda ##
    ##############################################################

    list_2V0_obj.append( CompFactory.DerivationFramework.JpsiXPlus2V0("BPHY25_Xibc_2V0") )
    idx = len(list_2V0_obj)-1
    list_2V0_obj[idx].JXVertices               = "BPHY25OniaCandidates"
    list_2V0_obj[idx].JXVtxHypoNames           = ["Jpsi"]
    list_2V0_obj[idx].TrackParticleCollection  = mainIDInput
    list_2V0_obj[idx].RelinkTracks             = TrkToRelink
    list_2V0_obj[idx].V0Vertices               = "V0Collection"
    list_2V0_obj[idx].CascadeVertexCollections = ["BPHY25_Xibc_2V0_CascadeVtx1","BPHY25_Xibc_2V0_CascadeVtx2","BPHY25_Xibc_2V0_CascadeVtx3","BPHY25_Xibc_2V0_CascadeMainVtx"]
    list_2V0_obj[idx].HasJXSubVertex           = True
    list_2V0_obj[idx].HasJXV02SubVertex        = True
    list_2V0_obj[idx].VxPrimaryCandidateName   = "PrimaryVertices"
    list_2V0_obj[idx].V01Hypothesis            = "Ks"
    list_2V0_obj[idx].V01MassLowerCut          = Ks_lo
    list_2V0_obj[idx].V01MassUpperCut          = Ks_hi
    list_2V0_obj[idx].LxyV01Cut                = 20.
    list_2V0_obj[idx].V02Hypothesis            = "Lambda"
    list_2V0_obj[idx].V02MassLowerCut          = Ld_lo
    list_2V0_obj[idx].V02MassUpperCut          = Ld_hi
    list_2V0_obj[idx].LxyV02Cut                = 20.
    list_2V0_obj[idx].MassCutGamma             = 10.
    list_2V0_obj[idx].Chi2CutGamma             = 3.
    list_2V0_obj[idx].HypothesisName           = "Xibc_2V0"
    list_2V0_obj[idx].NumberOfJXDaughters      = 2
    list_2V0_obj[idx].JXDaug1MassHypo          = Mumass
    list_2V0_obj[idx].JXDaug2MassHypo          = Mumass
    list_2V0_obj[idx].JXV02MassLowerCut        = Ldb0_lo
    list_2V0_obj[idx].JXV02MassUpperCut        = Ldb0_hi
    list_2V0_obj[idx].JpsiMass                 = Jpsimass
    list_2V0_obj[idx].V01Mass                  = Ksmass
    list_2V0_obj[idx].V02Mass                  = Lambdamass
    list_2V0_obj[idx].JXV02VtxMass             = Lambdab0mass
    list_2V0_obj[idx].ApplyJpsiMassConstraint  = True
    list_2V0_obj[idx].ApplyV01MassConstraint   = True
    list_2V0_obj[idx].ApplyV02MassConstraint   = True
    list_2V0_obj[idx].ApplyJXV02MassConstraint = True
    list_2V0_obj[idx].Chi2CutV0                = 5.
    list_2V0_obj[idx].Chi2Cut                  = 5.
    list_2V0_obj[idx].Trackd0Cut               = 2.5
    list_2V0_obj[idx].MaxJXCandidates          = 10
    list_2V0_obj[idx].MaxV0Candidates          = 20
    list_2V0_obj[idx].MaxMainVCandidates       = 30
    list_2V0_obj[idx].RefitPV                  = True
    list_2V0_obj[idx].MaxnPV                   = 50
    list_2V0_obj[idx].RefPVContainerName       = "BPHY25_Xibc_2V0_RefPrimaryVertices"
    list_2V0_obj[idx].TrkVertexFitterTool      = vkalvrt
    list_2V0_obj[idx].V0VertexFitterTool       = v0fitter
    list_2V0_obj[idx].GammaFitterTool          = gammafitter
    list_2V0_obj[idx].PVRefitter               = pvrefitter
    list_2V0_obj[idx].V0Tools                  = V0Tools
    list_2V0_obj[idx].TrackToVertexTool        = tracktovtxtool
    list_2V0_obj[idx].V0TrackSelectorTool      = v0trackselect
    list_2V0_obj[idx].Extrapolator             = extrapolator

    CascadeCollections = []
    RefPVContainers = []
    RefPVAuxContainers = []
    passedCandidates = []

    list_obj = list_disV_obj + list_trkLd_obj + list_jpsietac_obj + list_3bodyA_obj + list_3bodyB_obj + list_3bodyC_obj + list_2V0_obj
    #list_obj = list_disV_obj + list_trkLd_obj + list_jpsietac_obj + list_3bodyB_obj + list_3bodyC_obj + list_2V0_obj

    for obj in list_obj:
        CascadeCollections += obj.CascadeVertexCollections
        RefPVContainers += ["xAOD::VertexContainer#BPHY25_" + obj.HypothesisName + "_RefPrimaryVertices"]
        RefPVAuxContainers += ["xAOD::VertexAuxContainer#BPHY25_" + obj.HypothesisName + "_RefPrimaryVerticesAux."]
        passedCandidates += ["BPHY25_" + obj.HypothesisName + "_CascadeMainVtx"]

    BPHY25_SelectEvent = CompFactory.DerivationFramework.AnyVertexSkimmingTool(name = "BPHY25_SelectEvent", VertexContainerNames = passedCandidates)
    acc.addPublicTool(BPHY25_SelectEvent)

    augmentation_tools = [BPHY25_AugOriginalCounts, BPHY25_Reco_mumu, BPHY25ThreeTrackReco_Bpm, BPHY25FourTrackReco_Bs0, BPHY25FourTrackReco_B0, BPHY25Select_Jpsi, BPHY25Select_Psi] + list_obj
    for t in augmentation_tools : acc.addPublicTool(t)

    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel(
        "BPHY25Kernel",
        AugmentationTools = augmentation_tools,
        SkimmingTools     = [BPHY25_SelectEvent]
    ))

    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    BPHY25SlimmingHelper = SlimmingHelper("BPHY25SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
    from DerivationFrameworkBPhys.commonBPHYMethodsCfg import getDefaultAllVariables
    BPHY25_AllVariables  = getDefaultAllVariables()
    BPHY25_StaticContent = []

    # Needed for trigger objects
    BPHY25SlimmingHelper.IncludeMuonTriggerContent = True
    BPHY25SlimmingHelper.IncludeBPhysTriggerContent = True

    ## primary vertices
    BPHY25_AllVariables += ["PrimaryVertices"]
    BPHY25_StaticContent += RefPVContainers
    BPHY25_StaticContent += RefPVAuxContainers

    ## ID track particles
    BPHY25_AllVariables += ["InDetTrackParticles", "InDetLargeD0TrackParticles"]

    ## combined / extrapolated muon track particles 
    ## (note: for tagged muons there is no extra TrackParticle collection since the ID tracks
    ##        are stored in InDetTrackParticles collection)
    BPHY25_AllVariables += ["CombinedMuonTrackParticles", "ExtrapolatedMuonTrackParticles"]

    ## muon container
    BPHY25_AllVariables += ["Muons", "MuonSegments"]

    ## we have to disable vxTrackAtVertex branch since it is not xAOD compatible
    for cascade in CascadeCollections:
        BPHY25_StaticContent += ["xAOD::VertexContainer#%s" % cascade]
        BPHY25_StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % cascade]

    # Truth information for MC only
    if isSimulation:
        BPHY25_AllVariables += ["TruthEvents","TruthParticles","TruthVertices","MuonTruthParticles"]

    BPHY25SlimmingHelper.SmartCollections = ["Muons", "PrimaryVertices", "InDetTrackParticles", "InDetLargeD0TrackParticles"]
    BPHY25SlimmingHelper.AllVariables = BPHY25_AllVariables
    BPHY25SlimmingHelper.StaticContent = BPHY25_StaticContent

    BPHY25ItemList = BPHY25SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_BPHY25", ItemList=BPHY25ItemList, AcceptAlgs=["BPHY25Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_BPHY25", AcceptAlgs=["BPHY25Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))
    acc.printConfig(withDetails=True, summariseProps=True, onlyComponents = [], printDefaults=True, printComponentsOnly=False)
    return acc
