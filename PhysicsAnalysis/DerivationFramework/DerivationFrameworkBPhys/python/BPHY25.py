# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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
    from AthenaServices.PartPropSvcConfig import PartPropSvcCfg
    from DerivationFrameworkBPhys.commonBPHYMethodsCfg import (BPHY_V0ToolCfg,  BPHY_InDetDetailedTrackSelectorToolCfg, BPHY_VertexPointEstimatorCfg, BPHY_TrkVKalVrtFitterCfg)
    from JpsiUpsilonTools.JpsiUpsilonToolsConfig import PrimaryVertexRefittingToolCfg
    acc = ComponentAccumulator()
    acc.getPrimaryAndMerge(PartPropSvcCfg(flags))
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
        acc.merge(InDetLRTMergeCfg( flags, OutputTrackParticleLocation = mainIDInput ))

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
    Dpm_lo = 1700.0
    Dpm_hi = 2039.0
    B_lo = 5080.0
    B_hi = 5480.0
    Bc_lo = 5900.0
    Bc_hi = 6700.0
    Ks_lo = 430.0
    Ks_hi = 565.0
    Ld_lo = 1030.0
    Ld_hi = 1200.0
    Xi_lo = 1260.0
    Xi_hi = 1383.0
    Omg_lo = 1600.0
    Omg_hi = 1745.0

    Mumass = 105.658
    Pimass = 139.570
    Kmass = 493.677
    Ksmass = 497.611
    Jpsimass = 3096.916
    Dpmmass = 1869.66
    B0mass = 5279.66
    Bcmass = 6274.47
    Lambdamass = 1115.683
    Ximass = 1321.71
    Omegamass = 1672.45

    BPHY25JpsiFinder = CompFactory.Analysis.JpsiFinder(
        name                        = "BPHY25JpsiFinder",
        muAndMu                     = True,
        muAndTrack                  = False,
        TrackAndTrack               = False,
        assumeDiMuons               = True,  # If true, will assume dimu hypothesis and use PDG value for mu mass
        trackThresholdPt            = 3800.,
        invMassLower                = Jpsi_lo,
        invMassUpper                = Jpsi_hi,
        Chi2Cut                     = 4., # NDF=1 if no mass constraint
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

    # Bc -> J/psi pi
    BPHY25Bc_Jpsi1Trk = CompFactory.Analysis.JpsiPlus1Track(
        name                                = "BPHY25Bc_Jpsi1Trk",
        pionHypothesis                      = True,
        kaonHypothesis                      = False,
        trkThresholdPt                      = 3320.,
        trkMaxEta                           = 2.6,
        JpsiMassLower                       = Jpsi_lo,
        JpsiMassUpper                       = Jpsi_hi,
        TrkTrippletMassLower                = Bc_lo-100,
        TrkTrippletMassUpper                = Bc_hi+100,
        BMassLower                          = Bc_lo,
        BMassUpper                          = Bc_hi,
        Chi2Cut                             = 4.,
        JpsiContainerKey                    = "BPHY25OniaCandidates",
        TrackParticleCollection             = "InDetTrackParticles",
        MuonsUsedInJpsi                     = "Muons",
        ExcludeJpsiMuonsOnly                = True,
        TrkVertexFitterTool                 = vkalvrt,
        TrackSelectorTool                   = trackselect,
        UseMassConstraint                   = True)
    acc.addPublicTool(BPHY25Bc_Jpsi1Trk)

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
        TrkQuadrupletMassLower              = B_lo-100,
        TrkQuadrupletMassUpper              = B_hi+100,
        BMassLower                          = B_lo,
        BMassUpper                          = B_hi,
        DiTrackMassLower                    = 500.,
        DiTrackMassUpper                    = 1100.,
        Chi2Cut                             = 4.,
        JpsiContainerKey                    = "BPHY25OniaCandidates",
        TrackParticleCollection             = "InDetTrackParticles",
        MuonsUsedInJpsi			    = "Muons",
        ExcludeJpsiMuonsOnly                = True,
        TrkVertexFitterTool		    = vkalvrt,
        TrackSelectorTool		    = trackselect,
        UseMassConstraint	            = True)
    acc.addPublicTool(BPHY25B0_Jpsi2Trk)

    ######################
    ## Bc+ -> J/psi pi+ ##
    ######################
    BPHY25ThreeTrackReco_Bc = CompFactory.DerivationFramework.Reco_Vertex(
        name                     = "BPHY25ThreeTrackReco_Bc",
        VertexSearchTool         = BPHY25Bc_Jpsi1Trk,
        OutputVtxContainerName   = "BPHY25_BcToJpsiPi",
        PVContainerName          = "PrimaryVertices",
        RefPVContainerName       = "BPHY25_BcToJpsiPi_RefPrimaryVertices",
        RelinkTracks             = TrkToRelink,
        V0Tools                  = V0Tools,
        PVRefitter               = pvrefitter,
        RefitPV                  = True,
        MaxPVrefit               = 100,
        DoVertexType             = 7)

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

    #####################################################
    ## Xi_b^- -> J/psi Xi-, Xi^- -> Lambda pi-         ##
    ## Omega_b^- -> J/psi Omega-, Omega^- -> Lambda K- ##
    #####################################################

    list_disV_hypo = ["JpsiXi", "JpsiOmg"]
    list_disV_jxHypo = ["Jpsi", "Jpsi"]
    list_disV_disVLo = [Xi_lo, Omg_lo]
    list_disV_disVHi = [Xi_hi, Omg_hi]
    list_disV_disVDau3Mass = [Pimass, Kmass]
    list_disV_disVDaug3MinPt = [650., 750.]
    list_disV_jxMass = [Jpsimass, Jpsimass]
    list_disV_disVMass = [Ximass, Omegamass]

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
            list_disV_obj[i].OutputV0VtxCollection    = "V0Collection"
        else:
            list_disV_obj[i].V0Vertices               = "V0Collection"
        list_disV_obj[i].UseImprovedMass          = True
        list_disV_obj[i].LambdaMassLowerCut       = Ld_lo
        list_disV_obj[i].LambdaMassUpperCut       = Ld_hi
        list_disV_obj[i].KsMassLowerCut           = Ks_lo
        list_disV_obj[i].KsMassUpperCut           = Ks_hi
        list_disV_obj[i].DisplacedMassLowerCut    = list_disV_disVLo[i]
        list_disV_obj[i].DisplacedMassUpperCut    = list_disV_disVHi[i]
        list_disV_obj[i].MassLowerCut             = 0.
        list_disV_obj[i].MassUpperCut             = 12200.
        list_disV_obj[i].CascadeVertexCollections = ["BPHY25_"+list_disV_hypo[i]+"_CascadeVtx1_sub","BPHY25_"+list_disV_hypo[i]+"_CascadeVtx1","BPHY25_"+list_disV_hypo[i]+"_CascadeMainVtx"]
        list_disV_obj[i].HasJXSubVertex           = False
        list_disV_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
        list_disV_obj[i].V0Hypothesis             = "Lambda"
        list_disV_obj[i].MassCutGamma             = 10.
        list_disV_obj[i].Chi2CutGamma             = 3.
        list_disV_obj[i].LxyV0Cut                 = 5.
        list_disV_obj[i].LxyDisVtxCut             = 5.
        list_disV_obj[i].HypothesisName           = list_disV_hypo[i]
        list_disV_obj[i].NumberOfJXDaughters      = 2
        list_disV_obj[i].JXDaug1MassHypo          = Mumass
        list_disV_obj[i].JXDaug2MassHypo          = Mumass
        list_disV_obj[i].NumberOfDisVDaughters    = 3
        list_disV_obj[i].DisVDaug3MassHypo        = list_disV_disVDau3Mass[i]
        list_disV_obj[i].DisVDaug3MinPt           = list_disV_disVDaug3MinPt[i]
        list_disV_obj[i].JpsiMass                 = list_disV_jxMass[i]
        list_disV_obj[i].LambdaMass               = Lambdamass
        list_disV_obj[i].KsMass                   = Ksmass
        list_disV_obj[i].DisVtxMass               = list_disV_disVMass[i]
        list_disV_obj[i].ApplyJpsiMassConstraint  = True
        list_disV_obj[i].ApplyV0MassConstraint    = True
        list_disV_obj[i].ApplyDisVMassConstraint  = True
        list_disV_obj[i].ApplyMainVMassConstraint = False
        list_disV_obj[i].Chi2CutV0                = 4.
        list_disV_obj[i].Chi2CutDisV              = 4.
        list_disV_obj[i].Chi2Cut                  = 4.
        list_disV_obj[i].Trackd0Cut               = 3.0
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

    #################################
    ## B0(J/psi + K + pi) + Lambda ##
    ## B0(J/psi + K + pi) + Ks     ##
    #################################

    list_trkLd_hypo = ["B0KpiLd", "B0piKLd"]
    list_trkLd_jxInput = ["BPHY25FourTrack_B0", "BPHY25FourTrack_B0"]
    list_trkLd_jxMass = [B0mass, B0mass]
    list_trkLd_jxDau3Mass = [Kmass, Pimass]
    list_trkLd_jxDau4Mass = [Pimass, Kmass]
    list_trkLd_jxMassLo = [B_lo, B_lo]
    list_trkLd_jxMassHi = [B_hi, B_hi]

    list_trkLd_obj = []
    for hypo in list_trkLd_hypo:
        list_trkLd_obj.append( CompFactory.DerivationFramework.JpsiXPlusDisplaced("BPHY25_"+hypo) )

    for i in range(len(list_trkLd_obj)):
        list_trkLd_obj[i].JXVertices               = list_trkLd_jxInput[i]
        list_trkLd_obj[i].TrackParticleCollection  = mainIDInput
        list_trkLd_obj[i].RelinkTracks             = TrkToRelink
        list_trkLd_obj[i].V0Vertices               = "V0Collection"
        list_trkLd_obj[i].UseImprovedMass          = True
        list_trkLd_obj[i].LambdaMassLowerCut       = Ld_lo
        list_trkLd_obj[i].LambdaMassUpperCut       = Ld_hi
        list_trkLd_obj[i].KsMassLowerCut           = Ks_lo
        list_trkLd_obj[i].KsMassUpperCut           = Ks_hi
        list_trkLd_obj[i].MassLowerCut             = 0.
        list_trkLd_obj[i].MassUpperCut             = 9400.
        list_trkLd_obj[i].CascadeVertexCollections = ["BPHY25_"+list_trkLd_hypo[i]+"_CascadeVtx1","BPHY25_"+list_trkLd_hypo[i]+"_CascadeVtx2","BPHY25_"+list_trkLd_hypo[i]+"_CascadeMainVtx"]
        list_trkLd_obj[i].HasJXSubVertex           = True
        list_trkLd_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
        list_trkLd_obj[i].V0Hypothesis             = "Lambda/Ks"
        list_trkLd_obj[i].MassCutGamma             = 10.
        list_trkLd_obj[i].Chi2CutGamma             = 3.
        list_trkLd_obj[i].LxyV0Cut                 = 5.
        list_trkLd_obj[i].HypothesisName           = list_trkLd_hypo[i]
        list_trkLd_obj[i].NumberOfJXDaughters      = 4
        list_trkLd_obj[i].JXDaug1MassHypo          = Mumass
        list_trkLd_obj[i].JXDaug2MassHypo          = Mumass
        list_trkLd_obj[i].JXDaug3MassHypo          = list_trkLd_jxDau3Mass[i]
        list_trkLd_obj[i].JXDaug4MassHypo          = list_trkLd_jxDau4Mass[i]
        list_trkLd_obj[i].JXPtOrdering             = False
        list_trkLd_obj[i].JXMassLowerCut           = list_trkLd_jxMassLo[i]
        list_trkLd_obj[i].JXMassUpperCut           = list_trkLd_jxMassHi[i]
        list_trkLd_obj[i].JpsiMassLowerCut         = Jpsi_lo
        list_trkLd_obj[i].JpsiMassUpperCut         = Jpsi_hi
        list_trkLd_obj[i].DisplacedMassLowerCut    = 500.
        list_trkLd_obj[i].DisplacedMassUpperCut    = 1100.
        list_trkLd_obj[i].NumberOfDisVDaughters    = 2
        list_trkLd_obj[i].JXMass                   = list_trkLd_jxMass[i]
        list_trkLd_obj[i].JpsiMass                 = Jpsimass
        list_trkLd_obj[i].LambdaMass               = Lambdamass
        list_trkLd_obj[i].KsMass                   = Ksmass
        list_trkLd_obj[i].ApplyJXMassConstraint    = True
        list_trkLd_obj[i].ApplyJpsiMassConstraint  = True
        list_trkLd_obj[i].ApplyV0MassConstraint    = True
        list_trkLd_obj[i].ApplyMainVMassConstraint = False
        list_trkLd_obj[i].Chi2CutV0                = 4.
        list_trkLd_obj[i].Chi2Cut                  = 4.
        list_trkLd_obj[i].Trackd0Cut               = 3.0
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

    ##########################################
    ## Bc+ -> J/psi D+ Ks, D+ -> K- pi+ pi+ ##
    ##########################################

    list_3bodyA_hypo = ["Bc3body"]
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
        list_3bodyA_obj[i].UseImprovedMass            = True
        list_3bodyA_obj[i].LambdaMassLowerCut         = Ld_lo
        list_3bodyA_obj[i].LambdaMassUpperCut         = Ld_hi
        list_3bodyA_obj[i].KsMassLowerCut             = Ks_lo
        list_3bodyA_obj[i].KsMassUpperCut             = Ks_hi
        list_3bodyA_obj[i].MassLowerCut               = 5900.
        list_3bodyA_obj[i].MassUpperCut               = 6650.
        list_3bodyA_obj[i].PostMassLowerCut           = 6190.
        list_3bodyA_obj[i].PostMassUpperCut           = 6360.
        list_3bodyA_obj[i].CascadeVertexCollections   = ["BPHY25_"+list_3bodyA_hypo[i]+"_CascadeVtx1","BPHY25_"+list_3bodyA_hypo[i]+"_CascadeVtx2","BPHY25_"+list_3bodyA_hypo[i]+"_CascadeMainVtx"]
        list_3bodyA_obj[i].CascadeVertexCollectionsMVC= ["BPHY25_"+list_3bodyA_hypo[i]+"_CascadeVtx1_mvc","BPHY25_"+list_3bodyA_hypo[i]+"_CascadeVtx2_mvc","BPHY25_"+list_3bodyA_hypo[i]+"_CascadeMainVtx_mvc"]
        list_3bodyA_obj[i].VxPrimaryCandidateName     = "PrimaryVertices"
        list_3bodyA_obj[i].V0Hypothesis               = "Ks"
        list_3bodyA_obj[i].MassCutGamma               = 10.
        list_3bodyA_obj[i].Chi2CutGamma               = 3.
        list_3bodyA_obj[i].LxyV0Cut                   = 5.
        list_3bodyA_obj[i].HypothesisName             = list_3bodyA_hypo[i]
        list_3bodyA_obj[i].NumberOfJXDaughters        = 2
        list_3bodyA_obj[i].JXDaug1MassHypo            = Mumass
        list_3bodyA_obj[i].JXDaug2MassHypo            = Mumass
        list_3bodyA_obj[i].NumberOfDisVDaughters      = 2
        list_3bodyA_obj[i].ExtraTrack1MassHypo        = list_3bodyA_extraTrk1Mass[i]
        list_3bodyA_obj[i].ExtraTrack1MinPt           = 2000.
        list_3bodyA_obj[i].ExtraTrack2MassHypo        = list_3bodyA_extraTrk2Mass[i]
        list_3bodyA_obj[i].ExtraTrack2MinPt           = 1610.
        list_3bodyA_obj[i].ExtraTrack3MassHypo        = list_3bodyA_extraTrk3Mass[i]
        list_3bodyA_obj[i].ExtraTrack3MinPt           = 1230.
        list_3bodyA_obj[i].LxyDpmCut                  = 0.12
        list_3bodyA_obj[i].DpmMassLowerCut            = Dpm_lo
        list_3bodyA_obj[i].DpmMassUpperCut            = Dpm_hi
        list_3bodyA_obj[i].MaxMesonCandidates         = 30
        list_3bodyA_obj[i].MesonPtOrdering            = True
        list_3bodyA_obj[i].DpmMass                    = Dpmmass
        list_3bodyA_obj[i].JpsiMass                   = Jpsimass
        list_3bodyA_obj[i].LambdaMass                 = Lambdamass
        list_3bodyA_obj[i].KsMass                     = Ksmass
        list_3bodyA_obj[i].MainVtxMass                = Bcmass
        list_3bodyA_obj[i].ApplyJpsiMassConstraint    = True
        list_3bodyA_obj[i].ApplyV0MassConstraint      = True
        list_3bodyA_obj[i].ApplyDpmMassConstraint     = True
        list_3bodyA_obj[i].ApplyMainVMassConstraint   = False
        list_3bodyA_obj[i].DoPostMainVContrFit        = True
        list_3bodyA_obj[i].Chi2CutV0                  = 4.
        list_3bodyA_obj[i].Chi2CutDpm                 = 3.
        list_3bodyA_obj[i].Chi2Cut                    = 4.
        list_3bodyA_obj[i].Trackd0Cut                 = 3.0
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

    # BcToJpsiPi:
    Collections        = [ "BPHY25_BcToJpsiPi" ]
    RefPVContainers    = [ "xAOD::VertexContainer#BPHY25_BcToJpsiPi_RefPrimaryVertices" ]
    RefPVAuxContainers = [ "xAOD::VertexAuxContainer#BPHY25_BcToJpsiPi_RefPrimaryVerticesAux." ]
    passedCandidates   = [ "BPHY25_BcToJpsiPi" ]

    list_obj = list_disV_obj + list_trkLd_obj + list_3bodyA_obj

    for obj in list_obj:
        Collections += obj.CascadeVertexCollections
        if obj.ApplyMainVMassConstraint is False and obj.DoPostMainVContrFit is True:
            Collections += obj.CascadeVertexCollectionsMVC
        RefPVContainers += ["xAOD::VertexContainer#BPHY25_" + obj.HypothesisName + "_RefPrimaryVertices"]
        RefPVAuxContainers += ["xAOD::VertexAuxContainer#BPHY25_" + obj.HypothesisName + "_RefPrimaryVerticesAux."]
        passedCandidates += ["BPHY25_" + obj.HypothesisName + "_CascadeMainVtx"]

    BPHY25_SelectEvent = CompFactory.DerivationFramework.AnyVertexSkimmingTool(name = "BPHY25_SelectEvent", VertexContainerNames = passedCandidates)
    acc.addPublicTool(BPHY25_SelectEvent)

    augmentation_tools = [BPHY25_AugOriginalCounts, BPHY25_Reco_mumu, BPHY25ThreeTrackReco_Bc, BPHY25FourTrackReco_B0, BPHY25Select_Jpsi] + list_obj
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
    for collection in Collections:
        BPHY25_StaticContent += ["xAOD::VertexContainer#%s" % collection]
        BPHY25_StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % collection]

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
