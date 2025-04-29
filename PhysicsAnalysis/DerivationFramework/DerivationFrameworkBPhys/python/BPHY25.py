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
    B_lo = 5080.0
    B_hi = 5480.0
    Ks_lo = 430.0
    Ks_hi = 565.0
    Ld_lo = 1030.0
    Ld_hi = 1200.0
    Xi_lo = 1260.0
    Xi_hi = 1383.0
    Omg_lo = 1600.0
    Omg_hi = 1745.0
    Xib_lo = 5500.0
    Xib_hi = 6090.0
    Ldb0_lo = 5310.0
    Ldb0_hi = 5910.0

    Mumass = 105.658
    Pimass = 139.570
    Kmass = 493.677
    Ksmass = 497.611
    Protonmass = 938.2721
    Jpsimass = 3096.916
    B0mass = 5279.66
    Lambdamass = 1115.683
    Ximass = 1321.71
    Omegamass = 1672.45
    Lambdab0mass = 5619.60

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

    # B+ -> J/psi K
    BPHY25Bpm_Jpsi1Trk = CompFactory.Analysis.JpsiPlus1Track(
        name                                = "BPHY25Bpm_Jpsi1Trk",
        pionHypothesis                      = False,
        kaonHypothesis                      = True,
        trkThresholdPt                      = 950.,
        trkMaxEta                           = 2.6,
        JpsiMassLower                       = Jpsi_lo,
        JpsiMassUpper                       = Jpsi_hi,
        TrkTrippletMassLower                = B_lo-100,
        TrkTrippletMassUpper                = B_hi+100,
        BMassLower                          = B_lo,
        BMassUpper                          = B_hi,
        Chi2Cut                             = 4.,
        JpsiContainerKey                    = "BPHY25OniaCandidates",
        TrackParticleCollection             = "InDetTrackParticles",
        MuonsUsedInJpsi                     = "Muons",
        ExcludeJpsiMuonsOnly                = True,
        TrkVertexFitterTool                 = vkalvrt,
        TrackSelectorTool                   = trackselect,
        UseMassConstraint                   = True)
    acc.addPublicTool(BPHY25Bpm_Jpsi1Trk)

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
        list_disV_obj[i].MassUpperCut             = 20000.
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

    #################
    ## B+ + Lambda ##
    #################
    list_BpmLd_hypo = ["BpmLd"]
    list_BpmLd_jxInput = ["BPHY25ThreeTrack_Bpm"]
    list_BpmLd_jxDau3Mass = [Kmass]
    list_BpmLd_jxMassLo = [B_lo]
    list_BpmLd_jxMassHi = [B_hi]

    list_BpmLd_obj = []
    for hypo in list_BpmLd_hypo:
        list_BpmLd_obj.append( CompFactory.DerivationFramework.JpsiXPlusDisplaced("BPHY25_"+hypo) )

    for i in range(len(list_BpmLd_obj)):
        list_BpmLd_obj[i].JXVertices               = list_BpmLd_jxInput[i]
        list_BpmLd_obj[i].TrackParticleCollection  = mainIDInput
        list_BpmLd_obj[i].RelinkTracks             = TrkToRelink
        list_BpmLd_obj[i].V0Vertices               = "V0Collection"
        list_BpmLd_obj[i].UseImprovedMass          = True
        list_BpmLd_obj[i].LambdaMassLowerCut       = Ld_lo
        list_BpmLd_obj[i].LambdaMassUpperCut       = Ld_hi
        list_BpmLd_obj[i].KsMassLowerCut           = Ks_lo
        list_BpmLd_obj[i].KsMassUpperCut           = Ks_hi
        list_BpmLd_obj[i].JXMassLowerCut           = list_BpmLd_jxMassLo[i]
        list_BpmLd_obj[i].JXMassUpperCut           = list_BpmLd_jxMassHi[i]
        list_BpmLd_obj[i].JpsiMassLowerCut         = Jpsi_lo
        list_BpmLd_obj[i].JpsiMassUpperCut         = Jpsi_hi
        list_BpmLd_obj[i].MassLowerCut             = 6200.
        list_BpmLd_obj[i].MassUpperCut             = 150000.
        list_BpmLd_obj[i].CascadeVertexCollections = ["BPHY25_"+list_BpmLd_hypo[i]+"_CascadeVtx1","BPHY25_"+list_BpmLd_hypo[i]+"_CascadeVtx2","BPHY25_"+list_BpmLd_hypo[i]+"_CascadeMainVtx"]
        list_BpmLd_obj[i].HasJXSubVertex           = True
        list_BpmLd_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
        list_BpmLd_obj[i].V0Hypothesis             = "Lambda"
        list_BpmLd_obj[i].MassCutGamma             = 10.
        list_BpmLd_obj[i].Chi2CutGamma             = 3.
        list_BpmLd_obj[i].LxyV0Cut                 = 5.
        list_BpmLd_obj[i].HypothesisName           = list_BpmLd_hypo[i]
        list_BpmLd_obj[i].NumberOfJXDaughters      = 3
        list_BpmLd_obj[i].JXDaug1MassHypo          = Mumass
        list_BpmLd_obj[i].JXDaug2MassHypo          = Mumass
        list_BpmLd_obj[i].JXDaug3MassHypo          = list_BpmLd_jxDau3Mass[i]
        list_BpmLd_obj[i].JXPtOrdering             = False
        list_BpmLd_obj[i].NumberOfDisVDaughters    = 2
        list_BpmLd_obj[i].JpsiMass                 = Jpsimass
        list_BpmLd_obj[i].LambdaMass               = Lambdamass
        list_BpmLd_obj[i].KsMass                   = Ksmass
        list_BpmLd_obj[i].ApplyJpsiMassConstraint  = True
        list_BpmLd_obj[i].ApplyJXMassConstraint    = False
        list_BpmLd_obj[i].ApplyV0MassConstraint    = False
        list_BpmLd_obj[i].ApplyMainVMassConstraint = False
        list_BpmLd_obj[i].Chi2CutV0                = 4.
        list_BpmLd_obj[i].Chi2Cut                  = 4.
        list_BpmLd_obj[i].Trackd0Cut               = 3.0
        list_BpmLd_obj[i].MaxJXCandidates          = 20
        list_BpmLd_obj[i].MaxV0Candidates          = 20
        list_BpmLd_obj[i].MaxMainVCandidates       = 30
        list_BpmLd_obj[i].RefitPV                  = True
        list_BpmLd_obj[i].MaxnPV                   = 50
        list_BpmLd_obj[i].RefPVContainerName       = "BPHY25_"+list_BpmLd_hypo[i]+"_RefPrimaryVertices"
        list_BpmLd_obj[i].TrkVertexFitterTool      = vkalvrt
        list_BpmLd_obj[i].V0VertexFitterTool       = v0fitter
        list_BpmLd_obj[i].GammaFitterTool          = gammafitter
        list_BpmLd_obj[i].PVRefitter               = pvrefitter
        list_BpmLd_obj[i].V0Tools                  = V0Tools
        list_BpmLd_obj[i].TrackToVertexTool        = tracktovtxtool
        list_BpmLd_obj[i].V0TrackSelectorTool      = v0trackselect
        list_BpmLd_obj[i].TrackSelectorTool        = trackselect
        list_BpmLd_obj[i].Extrapolator             = extrapolator

    #################################
    ## B0(J/psi + K + pi) + Lambda ##
    #################################

    list_B0Ld_hypo = ["B0KpiLd", "B0piKLd"]
    list_B0Ld_jxInput = ["BPHY25FourTrack_B0", "BPHY25FourTrack_B0"]
    list_B0Ld_jxMass = [B0mass, B0mass]
    list_B0Ld_jxDau3Mass = [Kmass, Pimass]
    list_B0Ld_jxDau4Mass = [Pimass, Kmass]
    list_B0Ld_jxMassLo = [B_lo, B_lo]
    list_B0Ld_jxMassHi = [B_hi, B_hi]

    list_B0Ld_obj = []
    for hypo in list_B0Ld_hypo:
        list_B0Ld_obj.append( CompFactory.DerivationFramework.JpsiXPlusDisplaced("BPHY25_"+hypo) )

    for i in range(len(list_B0Ld_obj)):
        list_B0Ld_obj[i].JXVertices               = list_B0Ld_jxInput[i]
        list_B0Ld_obj[i].TrackParticleCollection  = mainIDInput
        list_B0Ld_obj[i].RelinkTracks             = TrkToRelink
        list_B0Ld_obj[i].V0Vertices               = "V0Collection"
        list_B0Ld_obj[i].UseImprovedMass          = True
        list_B0Ld_obj[i].LambdaMassLowerCut       = Ld_lo
        list_B0Ld_obj[i].LambdaMassUpperCut       = Ld_hi
        list_B0Ld_obj[i].KsMassLowerCut           = Ks_lo
        list_B0Ld_obj[i].KsMassUpperCut           = Ks_hi
        list_B0Ld_obj[i].JXMassLowerCut           = list_B0Ld_jxMassLo[i]
        list_B0Ld_obj[i].JXMassUpperCut           = list_B0Ld_jxMassHi[i]
        list_B0Ld_obj[i].JpsiMassLowerCut         = Jpsi_lo
        list_B0Ld_obj[i].JpsiMassUpperCut         = Jpsi_hi
        list_B0Ld_obj[i].MassLowerCut             = 0.
        list_B0Ld_obj[i].MassUpperCut             = 150000.
        list_B0Ld_obj[i].CascadeVertexCollections = ["BPHY25_"+list_B0Ld_hypo[i]+"_CascadeVtx1","BPHY25_"+list_B0Ld_hypo[i]+"_CascadeVtx2","BPHY25_"+list_B0Ld_hypo[i]+"_CascadeMainVtx"]
        list_B0Ld_obj[i].HasJXSubVertex           = True
        list_B0Ld_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
        list_B0Ld_obj[i].V0Hypothesis             = "Lambda"
        list_B0Ld_obj[i].MassCutGamma             = 10.
        list_B0Ld_obj[i].Chi2CutGamma             = 3.
        list_B0Ld_obj[i].LxyV0Cut                 = 5.
        list_B0Ld_obj[i].HypothesisName           = list_B0Ld_hypo[i]
        list_B0Ld_obj[i].NumberOfJXDaughters      = 4
        list_B0Ld_obj[i].JXDaug1MassHypo          = Mumass
        list_B0Ld_obj[i].JXDaug2MassHypo          = Mumass
        list_B0Ld_obj[i].JXDaug3MassHypo          = list_B0Ld_jxDau3Mass[i]
        list_B0Ld_obj[i].JXDaug4MassHypo          = list_B0Ld_jxDau4Mass[i]
        list_B0Ld_obj[i].JXPtOrdering             = False
        list_B0Ld_obj[i].NumberOfDisVDaughters    = 2
        list_B0Ld_obj[i].JXMass                   = list_B0Ld_jxMass[i]
        list_B0Ld_obj[i].JpsiMass                 = Jpsimass
        list_B0Ld_obj[i].LambdaMass               = Lambdamass
        list_B0Ld_obj[i].KsMass                   = Ksmass
        list_B0Ld_obj[i].ApplyJXMassConstraint    = True
        list_B0Ld_obj[i].ApplyJpsiMassConstraint  = True
        list_B0Ld_obj[i].ApplyV0MassConstraint    = False
        list_B0Ld_obj[i].ApplyMainVMassConstraint = False
        list_B0Ld_obj[i].Chi2CutV0                = 4.
        list_B0Ld_obj[i].Chi2Cut                  = 4.
        list_B0Ld_obj[i].Trackd0Cut               = 3.0
        list_B0Ld_obj[i].MaxJXCandidates          = 20
        list_B0Ld_obj[i].MaxV0Candidates          = 20
        list_B0Ld_obj[i].MaxMainVCandidates       = 30
        list_B0Ld_obj[i].RefitPV                  = True
        list_B0Ld_obj[i].MaxnPV                   = 50
        list_B0Ld_obj[i].RefPVContainerName       = "BPHY25_"+list_B0Ld_hypo[i]+"_RefPrimaryVertices"
        list_B0Ld_obj[i].TrkVertexFitterTool      = vkalvrt
        list_B0Ld_obj[i].V0VertexFitterTool       = v0fitter
        list_B0Ld_obj[i].GammaFitterTool          = gammafitter
        list_B0Ld_obj[i].PVRefitter               = pvrefitter
        list_B0Ld_obj[i].V0Tools                  = V0Tools
        list_B0Ld_obj[i].TrackToVertexTool        = tracktovtxtool
        list_B0Ld_obj[i].V0TrackSelectorTool      = v0trackselect
        list_B0Ld_obj[i].TrackSelectorTool        = trackselect
        list_B0Ld_obj[i].Extrapolator             = extrapolator

    #############################
    ## B0(J/psi + K + pi) + Ks ##
    #############################

    list_B0Ks_hypo = ["B0KpiKs", "B0piKKs"]
    list_B0Ks_jxInput = ["BPHY25FourTrack_B0", "BPHY25FourTrack_B0"]
    list_B0Ks_jxMass = [B0mass, B0mass]
    list_B0Ks_jxDau3Mass = [Kmass, Pimass]
    list_B0Ks_jxDau4Mass = [Pimass, Kmass]
    list_B0Ks_jxMassLo = [B_lo, B_lo]
    list_B0Ks_jxMassHi = [B_hi, B_hi]

    list_B0Ks_obj = []
    for hypo in list_B0Ks_hypo:
        list_B0Ks_obj.append( CompFactory.DerivationFramework.JpsiXPlusDisplaced("BPHY25_"+hypo) )

    for i in range(len(list_B0Ks_obj)):
        list_B0Ks_obj[i].JXVertices               = list_B0Ks_jxInput[i]
        list_B0Ks_obj[i].TrackParticleCollection  = mainIDInput
        list_B0Ks_obj[i].RelinkTracks             = TrkToRelink
        list_B0Ks_obj[i].V0Vertices               = "V0Collection"
        list_B0Ks_obj[i].UseImprovedMass          = True
        list_B0Ks_obj[i].LambdaMassLowerCut       = Ld_lo
        list_B0Ks_obj[i].LambdaMassUpperCut       = Ld_hi
        list_B0Ks_obj[i].KsMassLowerCut           = Ks_lo
        list_B0Ks_obj[i].KsMassUpperCut           = Ks_hi
        list_B0Ks_obj[i].JXMassLowerCut           = list_B0Ks_jxMassLo[i]
        list_B0Ks_obj[i].JXMassUpperCut           = list_B0Ks_jxMassHi[i]
        list_B0Ks_obj[i].JpsiMassLowerCut         = Jpsi_lo
        list_B0Ks_obj[i].JpsiMassUpperCut         = Jpsi_hi
        list_B0Ks_obj[i].MassLowerCut             = 0.
        list_B0Ks_obj[i].MassUpperCut             = 150000.
        list_B0Ks_obj[i].CascadeVertexCollections = ["BPHY25_"+list_B0Ks_hypo[i]+"_CascadeVtx1","BPHY25_"+list_B0Ks_hypo[i]+"_CascadeVtx2","BPHY25_"+list_B0Ks_hypo[i]+"_CascadeMainVtx"]
        list_B0Ks_obj[i].HasJXSubVertex           = True
        list_B0Ks_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
        list_B0Ks_obj[i].V0Hypothesis             = "Ks"
        list_B0Ks_obj[i].MassCutGamma             = 10.
        list_B0Ks_obj[i].Chi2CutGamma             = 3.
        list_B0Ks_obj[i].LxyV0Cut                 = 5.
        list_B0Ks_obj[i].HypothesisName           = list_B0Ks_hypo[i]
        list_B0Ks_obj[i].NumberOfJXDaughters      = 4
        list_B0Ks_obj[i].JXDaug1MassHypo          = Mumass
        list_B0Ks_obj[i].JXDaug2MassHypo          = Mumass
        list_B0Ks_obj[i].JXDaug3MassHypo          = list_B0Ks_jxDau3Mass[i]
        list_B0Ks_obj[i].JXDaug4MassHypo          = list_B0Ks_jxDau4Mass[i]
        list_B0Ks_obj[i].JXPtOrdering             = False
        list_B0Ks_obj[i].NumberOfDisVDaughters    = 2
        list_B0Ks_obj[i].JXMass                   = list_B0Ks_jxMass[i]
        list_B0Ks_obj[i].JpsiMass                 = Jpsimass
        list_B0Ks_obj[i].LambdaMass               = Lambdamass
        list_B0Ks_obj[i].KsMass                   = Ksmass
        list_B0Ks_obj[i].ApplyJXMassConstraint    = True
        list_B0Ks_obj[i].ApplyJpsiMassConstraint  = True
        list_B0Ks_obj[i].ApplyV0MassConstraint    = True
        list_B0Ks_obj[i].ApplyMainVMassConstraint = False
        list_B0Ks_obj[i].Chi2CutV0                = 5.
        list_B0Ks_obj[i].Chi2Cut                  = 5.
        list_B0Ks_obj[i].Trackd0Cut               = 3.0
        list_B0Ks_obj[i].MaxJXCandidates          = 20
        list_B0Ks_obj[i].MaxV0Candidates          = 20
        list_B0Ks_obj[i].MaxMainVCandidates       = 30
        list_B0Ks_obj[i].RefitPV                  = True
        list_B0Ks_obj[i].MaxnPV                   = 50
        list_B0Ks_obj[i].RefPVContainerName       = "BPHY25_"+list_B0Ks_hypo[i]+"_RefPrimaryVertices"
        list_B0Ks_obj[i].TrkVertexFitterTool      = vkalvrt
        list_B0Ks_obj[i].V0VertexFitterTool       = v0fitter
        list_B0Ks_obj[i].GammaFitterTool          = gammafitter
        list_B0Ks_obj[i].PVRefitter               = pvrefitter
        list_B0Ks_obj[i].V0Tools                  = V0Tools
        list_B0Ks_obj[i].TrackToVertexTool        = tracktovtxtool
        list_B0Ks_obj[i].V0TrackSelectorTool      = v0trackselect
        list_B0Ks_obj[i].TrackSelectorTool        = trackselect
        list_B0Ks_obj[i].Extrapolator             = extrapolator

    ###############################
    ## B- -> J/psi Lambda pbar   ##
    ## Xi_b- -> J/psi Lambda pi- ##
    ###############################

    list_3bodyA_hypo = ["Bpm3body", "Xibpm3body"]
    list_3bodyA_extraTrkMass = [Protonmass, Pimass]
    list_3bodyA_extraTrkPt = [480.0, 760.0]
    list_3bodyA_massLo = [B_lo, Xib_lo]
    list_3bodyA_massHi = [B_hi, Xib_hi]

    list_3bodyA_obj = []
    for hypo in list_3bodyA_hypo:
        list_3bodyA_obj.append( CompFactory.DerivationFramework.JpsiXPlusDisplaced("BPHY25_"+hypo) )

    for i in range(len(list_3bodyA_obj)):
        list_3bodyA_obj[i].JXVertices               = "BPHY25OniaCandidates"
        list_3bodyA_obj[i].JXVtxHypoNames           = ["Jpsi"]
        list_3bodyA_obj[i].TrackParticleCollection  = mainIDInput
        list_3bodyA_obj[i].RelinkTracks             = TrkToRelink
        list_3bodyA_obj[i].V0Vertices               = "V0Collection"
        list_3bodyA_obj[i].UseImprovedMass          = True
        list_3bodyA_obj[i].LambdaMassLowerCut       = Ld_lo
        list_3bodyA_obj[i].LambdaMassUpperCut       = Ld_hi
        list_3bodyA_obj[i].KsMassLowerCut           = Ks_lo
        list_3bodyA_obj[i].KsMassUpperCut           = Ks_hi
        list_3bodyA_obj[i].JpsiMassLowerCut         = Jpsi_lo
        list_3bodyA_obj[i].JpsiMassUpperCut         = Jpsi_hi
        list_3bodyA_obj[i].MassLowerCut             = list_3bodyA_massLo[i]
        list_3bodyA_obj[i].MassUpperCut             = list_3bodyA_massHi[i]
        list_3bodyA_obj[i].CascadeVertexCollections = ["BPHY25_"+list_3bodyA_hypo[i]+"_CascadeVtx1","BPHY25_"+list_3bodyA_hypo[i]+"_CascadeMainVtx"]
        list_3bodyA_obj[i].HasJXSubVertex           = False
        list_3bodyA_obj[i].HasJXV0SubVertex         = False
        list_3bodyA_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
        list_3bodyA_obj[i].V0Hypothesis             = "Lambda"
        list_3bodyA_obj[i].MassCutGamma             = 10.
        list_3bodyA_obj[i].Chi2CutGamma             = 3.
        list_3bodyA_obj[i].LxyV0Cut                 = 5.
        list_3bodyA_obj[i].HypothesisName           = list_3bodyA_hypo[i]
        list_3bodyA_obj[i].NumberOfJXDaughters      = 2
        list_3bodyA_obj[i].JXDaug1MassHypo          = Mumass
        list_3bodyA_obj[i].JXDaug2MassHypo          = Mumass
        list_3bodyA_obj[i].NumberOfDisVDaughters    = 2
        list_3bodyA_obj[i].ExtraTrack1MassHypo      = list_3bodyA_extraTrkMass[i]
        list_3bodyA_obj[i].ExtraTrack1MinPt         = list_3bodyA_extraTrkPt[i]
        list_3bodyA_obj[i].JpsiMass                 = Jpsimass
        list_3bodyA_obj[i].LambdaMass               = Lambdamass
        list_3bodyA_obj[i].KsMass                   = Ksmass
        list_3bodyA_obj[i].ApplyJpsiMassConstraint  = True
        list_3bodyA_obj[i].ApplyV0MassConstraint    = True
        list_3bodyA_obj[i].ApplyMainVMassConstraint = False
        list_3bodyA_obj[i].Chi2CutV0                = 4.
        list_3bodyA_obj[i].Chi2Cut                  = 4.
        list_3bodyA_obj[i].Trackd0Cut               = 3.0
        list_3bodyA_obj[i].MaxJXCandidates          = 10
        list_3bodyA_obj[i].MaxV0Candidates          = 20
        list_3bodyA_obj[i].MaxMainVCandidates       = 30
        list_3bodyA_obj[i].RefitPV                  = True
        list_3bodyA_obj[i].MaxnPV                   = 50
        list_3bodyA_obj[i].RefPVContainerName       = "BPHY25_"+list_3bodyA_hypo[i]+"_RefPrimaryVertices"
        list_3bodyA_obj[i].TrkVertexFitterTool      = vkalvrt
        list_3bodyA_obj[i].V0VertexFitterTool       = v0fitter
        list_3bodyA_obj[i].GammaFitterTool          = gammafitter
        list_3bodyA_obj[i].PVRefitter               = pvrefitter
        list_3bodyA_obj[i].V0Tools                  = V0Tools
        list_3bodyA_obj[i].TrackToVertexTool        = tracktovtxtool
        list_3bodyA_obj[i].V0TrackSelectorTool      = v0trackselect
        list_3bodyA_obj[i].TrackSelectorTool        = trackselect
        list_3bodyA_obj[i].Extrapolator             = extrapolator

    ###########################
    ## Xi_b- -> Lambdab0 pi- ##
    ###########################

    list_RefA_hypo = ["XibpmRef"]
    list_RefA_extraTrkMass = [Pimass]
    list_RefA_extraTrkPt = [760.0]
    list_RefA_massLo = [Xib_lo]
    list_RefA_massHi = [Xib_hi]

    list_RefA_obj = []
    for hypo in list_RefA_hypo:
        list_RefA_obj.append( CompFactory.DerivationFramework.JpsiXPlusDisplaced("BPHY25_"+hypo) )

    for i in range(len(list_RefA_obj)):
        list_RefA_obj[i].JXVertices               = "BPHY25OniaCandidates"
        list_RefA_obj[i].JXVtxHypoNames           = ["Jpsi"]
        list_RefA_obj[i].TrackParticleCollection  = mainIDInput
        list_RefA_obj[i].RelinkTracks             = TrkToRelink
        list_RefA_obj[i].V0Vertices               = "V0Collection"
        list_RefA_obj[i].UseImprovedMass          = True
        list_RefA_obj[i].LambdaMassLowerCut       = Ld_lo
        list_RefA_obj[i].LambdaMassUpperCut       = Ld_hi
        list_RefA_obj[i].KsMassLowerCut           = Ks_lo
        list_RefA_obj[i].KsMassUpperCut           = Ks_hi
        list_RefA_obj[i].JpsiMassLowerCut         = Jpsi_lo
        list_RefA_obj[i].JpsiMassUpperCut         = Jpsi_hi
        list_RefA_obj[i].MassLowerCut             = list_RefA_massLo[i]
        list_RefA_obj[i].MassUpperCut             = list_RefA_massHi[i]
        list_RefA_obj[i].CascadeVertexCollections = ["BPHY25_"+list_RefA_hypo[i]+"_CascadeVtx1","BPHY25_"+list_RefA_hypo[i]+"_CascadeVtx2","BPHY25_"+list_RefA_hypo[i]+"_CascadeMainVtx"]
        list_RefA_obj[i].HasJXSubVertex           = False
        list_RefA_obj[i].HasJXV0SubVertex         = True
        list_RefA_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
        list_RefA_obj[i].V0Hypothesis             = "Lambda"
        list_RefA_obj[i].MassCutGamma             = 10.
        list_RefA_obj[i].Chi2CutGamma             = 3.
        list_RefA_obj[i].LxyV0Cut                 = 5.
        list_RefA_obj[i].HypothesisName           = list_RefA_hypo[i]
        list_RefA_obj[i].NumberOfJXDaughters      = 2
        list_RefA_obj[i].JXDaug1MassHypo          = Mumass
        list_RefA_obj[i].JXDaug2MassHypo          = Mumass
        list_RefA_obj[i].NumberOfDisVDaughters    = 2
        list_RefA_obj[i].ExtraTrack1MassHypo      = list_RefA_extraTrkMass[i]
        list_RefA_obj[i].ExtraTrack1MinPt         = list_RefA_extraTrkPt[i]
        list_RefA_obj[i].JpsiMass                 = Jpsimass
        list_RefA_obj[i].LambdaMass               = Lambdamass
        list_RefA_obj[i].KsMass                   = Ksmass
        list_RefA_obj[i].ApplyJpsiMassConstraint  = True
        list_RefA_obj[i].ApplyV0MassConstraint    = True
        list_RefA_obj[i].ApplyJXV0MassConstraint  = False
        list_RefA_obj[i].ApplyMainVMassConstraint = False
        list_RefA_obj[i].Chi2CutV0                = 4.
        list_RefA_obj[i].Chi2Cut                  = 4.
        list_RefA_obj[i].Trackd0Cut               = 3.0
        list_RefA_obj[i].MaxJXCandidates          = 10
        list_RefA_obj[i].MaxV0Candidates          = 20
        list_RefA_obj[i].MaxMainVCandidates       = 30
        list_RefA_obj[i].RefitPV                  = True
        list_RefA_obj[i].MaxnPV                   = 50
        list_RefA_obj[i].RefPVContainerName       = "BPHY25_"+list_RefA_hypo[i]+"_RefPrimaryVertices"
        list_RefA_obj[i].TrkVertexFitterTool      = vkalvrt
        list_RefA_obj[i].V0VertexFitterTool       = v0fitter
        list_RefA_obj[i].GammaFitterTool          = gammafitter
        list_RefA_obj[i].PVRefitter               = pvrefitter
        list_RefA_obj[i].V0Tools                  = V0Tools
        list_RefA_obj[i].TrackToVertexTool        = tracktovtxtool
        list_RefA_obj[i].V0TrackSelectorTool      = v0trackselect
        list_RefA_obj[i].TrackSelectorTool        = trackselect
        list_RefA_obj[i].Extrapolator             = extrapolator

    ####################################################
    ## Xib^0 -> J/psi Xi^- pi+, Xi^- -> Lambda pi-    ##
    ## Lambdab^0 -> J/psi Xi^- K+, Xi^- -> Lambda pi- ##
    ####################################################

    list_3bodyB_hypo = ["Xib03body", "Lambdab03body"]
    list_3bodyB_extraTrkMass = [Pimass, Kmass]
    list_3bodyB_extraTrkPt = [660.0, 660.0]
    list_3bodyB_massLo = [Xib_lo, Ldb0_lo]
    list_3bodyB_massHi = [Xib_hi, Ldb0_hi]

    list_3bodyB_obj = []
    for hypo in list_3bodyB_hypo:
        list_3bodyB_obj.append( CompFactory.DerivationFramework.JpsiXPlusDisplaced("BPHY25_"+hypo) )

    for i in range(len(list_3bodyB_obj)):
        list_3bodyB_obj[i].JXVertices               = "BPHY25OniaCandidates"
        list_3bodyB_obj[i].JXVtxHypoNames           = ["Jpsi"]
        list_3bodyB_obj[i].TrackParticleCollection  = mainIDInput
        list_3bodyB_obj[i].RelinkTracks             = TrkToRelink
        list_3bodyB_obj[i].V0Vertices               = "V0Collection"
        list_3bodyB_obj[i].UseImprovedMass          = True
        list_3bodyB_obj[i].LambdaMassLowerCut       = Ld_lo
        list_3bodyB_obj[i].LambdaMassUpperCut       = Ld_hi
        list_3bodyB_obj[i].KsMassLowerCut           = Ks_lo
        list_3bodyB_obj[i].KsMassUpperCut           = Ks_hi
        list_3bodyB_obj[i].JpsiMassLowerCut         = Jpsi_lo
        list_3bodyB_obj[i].JpsiMassUpperCut         = Jpsi_hi
        list_3bodyB_obj[i].DisplacedMassLowerCut    = Xi_lo
        list_3bodyB_obj[i].DisplacedMassUpperCut    = Xi_hi
        list_3bodyB_obj[i].MassLowerCut             = list_3bodyB_massLo[i]
        list_3bodyB_obj[i].MassUpperCut             = list_3bodyB_massHi[i]
        list_3bodyB_obj[i].CascadeVertexCollections = ["BPHY25_"+list_3bodyB_hypo[i]+"_CascadeVtx1_sub","BPHY25_"+list_3bodyB_hypo[i]+"_CascadeVtx1","BPHY25_"+list_3bodyB_hypo[i]+"_CascadeMainVtx"]
        list_3bodyB_obj[i].HasJXSubVertex           = False
        list_3bodyB_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
        list_3bodyB_obj[i].V0Hypothesis             = "Lambda"
        list_3bodyB_obj[i].MassCutGamma             = 10.
        list_3bodyB_obj[i].Chi2CutGamma             = 3.
        list_3bodyB_obj[i].LxyV0Cut                 = 5.
        list_3bodyB_obj[i].LxyDisVtxCut             = 5.
        list_3bodyB_obj[i].HypothesisName           = list_3bodyB_hypo[i]
        list_3bodyB_obj[i].NumberOfJXDaughters      = 2
        list_3bodyB_obj[i].JXDaug1MassHypo          = Mumass
        list_3bodyB_obj[i].JXDaug2MassHypo          = Mumass
        list_3bodyB_obj[i].NumberOfDisVDaughters    = 3
        list_3bodyB_obj[i].DisVDaug3MassHypo        = Pimass
        list_3bodyB_obj[i].DisVDaug3MinPt           = 650.
        list_3bodyB_obj[i].ExtraTrack1MassHypo      = list_3bodyB_extraTrkMass[i]
        list_3bodyB_obj[i].ExtraTrack1MinPt         = list_3bodyB_extraTrkPt[i]
        list_3bodyB_obj[i].JpsiMass                 = Jpsimass
        list_3bodyB_obj[i].LambdaMass               = Lambdamass
        list_3bodyB_obj[i].KsMass                   = Ksmass
        list_3bodyB_obj[i].DisVtxMass               = Ximass
        list_3bodyB_obj[i].ApplyJpsiMassConstraint  = True
        list_3bodyB_obj[i].ApplyV0MassConstraint    = True
        list_3bodyB_obj[i].ApplyDisVMassConstraint  = True
        list_3bodyB_obj[i].ApplyMainVMassConstraint = False
        list_3bodyB_obj[i].Chi2CutV0                = 4.
        list_3bodyB_obj[i].Chi2CutDisV              = 4.
        list_3bodyB_obj[i].Chi2Cut                  = 4.
        list_3bodyB_obj[i].Trackd0Cut               = 3.0
        list_3bodyB_obj[i].MaxJXCandidates          = 10
        list_3bodyB_obj[i].MaxV0Candidates          = 20
        list_3bodyB_obj[i].MaxDisVCandidates        = 30
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

    #################
    ## J/psi + 2V0 ##
    #################

    list_2V0A_hypo = ["Jpsi2V0A"]

    list_2V0A_obj = []
    for hypo in list_2V0A_hypo:
        list_2V0A_obj.append( CompFactory.DerivationFramework.JpsiXPlus2V0("BPHY25_"+hypo) )

    for i in range(len(list_2V0A_obj)):
        list_2V0A_obj[i].JXVertices               = "BPHY25OniaCandidates"
        list_2V0A_obj[i].JXVtxHypoNames           = ["Jpsi"]
        list_2V0A_obj[i].TrackParticleCollection  = mainIDInput
        list_2V0A_obj[i].RelinkTracks             = TrkToRelink
        list_2V0A_obj[i].V0Vertices               = "V0Collection"
        list_2V0A_obj[i].UseImprovedMass          = True
        list_2V0A_obj[i].LambdaMassLowerCut       = Ld_lo
        list_2V0A_obj[i].LambdaMassUpperCut       = Ld_hi
        list_2V0A_obj[i].KsMassLowerCut           = Ks_lo
        list_2V0A_obj[i].KsMassUpperCut           = Ks_hi
        list_2V0A_obj[i].JpsiMassLowerCut         = Jpsi_lo
        list_2V0A_obj[i].JpsiMassUpperCut         = Jpsi_hi
        list_2V0A_obj[i].MassLowerCut             = 0.
        list_2V0A_obj[i].MassUpperCut             = 150000.
        list_2V0A_obj[i].CascadeVertexCollections = ["BPHY25_"+list_2V0A_hypo[i]+"_CascadeVtx1","BPHY25_"+list_2V0A_hypo[i]+"_CascadeVtx2","BPHY25_"+list_2V0A_hypo[i]+"_CascadeMainVtx"]
        list_2V0A_obj[i].HasJXSubVertex           = False
        list_2V0A_obj[i].HasJXV02SubVertex        = False
        list_2V0A_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
        list_2V0A_obj[i].V01Hypothesis            = "Lambda/Ks"
        list_2V0A_obj[i].V02Hypothesis            = "Lambda/Ks"
        list_2V0A_obj[i].LxyV01Cut                = 5.
        list_2V0A_obj[i].LxyV02Cut                = 5.
        list_2V0A_obj[i].MassCutGamma             = 10.
        list_2V0A_obj[i].Chi2CutGamma             = 3.
        list_2V0A_obj[i].HypothesisName           = list_2V0A_hypo[i]
        list_2V0A_obj[i].NumberOfJXDaughters      = 2
        list_2V0A_obj[i].JXDaug1MassHypo          = Mumass
        list_2V0A_obj[i].JXDaug2MassHypo          = Mumass
        list_2V0A_obj[i].JpsiMass                 = Jpsimass
        list_2V0A_obj[i].LambdaMass               = Lambdamass
        list_2V0A_obj[i].KsMass                   = Ksmass
        list_2V0A_obj[i].ApplyJpsiMassConstraint  = True
        list_2V0A_obj[i].ApplyV01MassConstraint   = True
        list_2V0A_obj[i].ApplyV02MassConstraint   = True
        list_2V0A_obj[i].ApplyMainVMassConstraint = False
        list_2V0A_obj[i].Chi2CutV0                = 4.
        list_2V0A_obj[i].Chi2Cut                  = 4.
        list_2V0A_obj[i].Trackd0Cut               = 3.0
        list_2V0A_obj[i].MaxJXCandidates          = 10
        list_2V0A_obj[i].MaxV0Candidates          = 20
        list_2V0A_obj[i].MaxMainVCandidates       = 30
        list_2V0A_obj[i].RefitPV                  = True
        list_2V0A_obj[i].MaxnPV                   = 50
        list_2V0A_obj[i].RefPVContainerName       = "BPHY25_"+list_2V0A_hypo[i]+"_RefPrimaryVertices"
        list_2V0A_obj[i].TrkVertexFitterTool      = vkalvrt
        list_2V0A_obj[i].V0VertexFitterTool       = v0fitter
        list_2V0A_obj[i].GammaFitterTool          = gammafitter
        list_2V0A_obj[i].PVRefitter               = pvrefitter
        list_2V0A_obj[i].V0Tools                  = V0Tools
        list_2V0A_obj[i].TrackToVertexTool        = tracktovtxtool
        list_2V0A_obj[i].V0TrackSelectorTool      = v0trackselect
        list_2V0A_obj[i].Extrapolator             = extrapolator

    list_2V0B_hypo        = ["Jpsi2V0B1", "Jpsi2V0B2"]
    list_2V0B_v02hypo     = ["Lambda", "Ks"]
    list_2V0B_jxv02mass   = [Lambdab0mass, B0mass]
    list_2V0B_jxv02massLo = [Ldb0_lo, B_lo]
    list_2V0B_jxv02massHi = [Ldb0_hi, B_hi]

    list_2V0B_obj = []
    for hypo in list_2V0B_hypo:
        list_2V0B_obj.append( CompFactory.DerivationFramework.JpsiXPlus2V0("BPHY25_"+hypo) )

    for i in range(len(list_2V0B_obj)):
        list_2V0B_obj[i].JXVertices               = "BPHY25OniaCandidates"
        list_2V0B_obj[i].JXVtxHypoNames           = ["Jpsi"]
        list_2V0B_obj[i].TrackParticleCollection  = mainIDInput
        list_2V0B_obj[i].RelinkTracks             = TrkToRelink
        list_2V0B_obj[i].V0Vertices               = "V0Collection"
        list_2V0B_obj[i].UseImprovedMass          = True
        list_2V0B_obj[i].LambdaMassLowerCut       = Ld_lo
        list_2V0B_obj[i].LambdaMassUpperCut       = Ld_hi
        list_2V0B_obj[i].KsMassLowerCut           = Ks_lo
        list_2V0B_obj[i].KsMassUpperCut           = Ks_hi
        list_2V0B_obj[i].JpsiMassLowerCut         = Jpsi_lo
        list_2V0B_obj[i].JpsiMassUpperCut         = Jpsi_hi
        list_2V0B_obj[i].JXV02MassLowerCut        = list_2V0B_jxv02massLo[i]
        list_2V0B_obj[i].JXV02MassUpperCut        = list_2V0B_jxv02massHi[i]
        list_2V0B_obj[i].MassLowerCut             = 0.
        list_2V0B_obj[i].MassUpperCut             = 150000.
        list_2V0B_obj[i].CascadeVertexCollections = ["BPHY25_"+list_2V0B_hypo[i]+"_CascadeVtx1","BPHY25_"+list_2V0B_hypo[i]+"_CascadeVtx2","BPHY25_"+list_2V0B_hypo[i]+"_CascadeVtx3","BPHY25_"+list_2V0B_hypo[i]+"_CascadeMainVtx"]
        list_2V0B_obj[i].HasJXSubVertex           = True
        list_2V0B_obj[i].HasJXV02SubVertex        = True
        list_2V0B_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
        list_2V0B_obj[i].V01Hypothesis            = "Lambda/Ks"
        list_2V0B_obj[i].V02Hypothesis            = list_2V0B_v02hypo[i]
        list_2V0B_obj[i].LxyV01Cut                = 5.
        list_2V0B_obj[i].LxyV02Cut                = 5.
        list_2V0B_obj[i].MassCutGamma             = 10.
        list_2V0B_obj[i].Chi2CutGamma             = 3.
        list_2V0B_obj[i].HypothesisName           = list_2V0B_hypo[i]
        list_2V0B_obj[i].NumberOfJXDaughters      = 2
        list_2V0B_obj[i].JXDaug1MassHypo          = Mumass
        list_2V0B_obj[i].JXDaug2MassHypo          = Mumass
        list_2V0B_obj[i].JpsiMass                 = Jpsimass
        list_2V0B_obj[i].LambdaMass               = Lambdamass
        list_2V0B_obj[i].KsMass                   = Ksmass
        list_2V0B_obj[i].JXV02VtxMass             = list_2V0B_jxv02mass[i]
        list_2V0B_obj[i].ApplyJpsiMassConstraint  = True
        list_2V0B_obj[i].ApplyV01MassConstraint   = True
        list_2V0B_obj[i].ApplyV02MassConstraint   = True
        list_2V0B_obj[i].ApplyJXV02MassConstraint = True
        list_2V0B_obj[i].ApplyMainVMassConstraint = False
        list_2V0B_obj[i].Chi2CutV0                = 4.
        list_2V0B_obj[i].Chi2Cut                  = 4.
        list_2V0B_obj[i].Trackd0Cut               = 3.0
        list_2V0B_obj[i].MaxJXCandidates          = 10
        list_2V0B_obj[i].MaxV0Candidates          = 20
        list_2V0B_obj[i].MaxMainVCandidates       = 30
        list_2V0B_obj[i].RefitPV                  = True
        list_2V0B_obj[i].MaxnPV                   = 50
        list_2V0B_obj[i].RefPVContainerName       = "BPHY25_"+list_2V0B_hypo[i]+"_RefPrimaryVertices"
        list_2V0B_obj[i].TrkVertexFitterTool      = vkalvrt
        list_2V0B_obj[i].V0VertexFitterTool       = v0fitter
        list_2V0B_obj[i].GammaFitterTool          = gammafitter
        list_2V0B_obj[i].PVRefitter               = pvrefitter
        list_2V0B_obj[i].V0Tools                  = V0Tools
        list_2V0B_obj[i].TrackToVertexTool        = tracktovtxtool
        list_2V0B_obj[i].V0TrackSelectorTool      = v0trackselect
        list_2V0B_obj[i].Extrapolator             = extrapolator

    BPHY25Rev_JpsiXi = CompFactory.DerivationFramework.ReVertex(
        name                       = "BPHY25Rev_JpsiXi",
        InputVtxContainerName      = "BPHY25_JpsiXi_CascadeMainVtx",
        TrackContainerName         = mainIDInput,
        RelinkTracks               = TrkToRelink,
        TrackIndices               = [ 0, 1 ],
        UseMassConstraint          = True,
        VertexMass                 = Jpsimass,
        MassInputParticles         = [Mumass, Mumass],
        Chi2Cut                    = 25.,
        TrkVertexFitterTool	   = vkalvrt,
        PVRefitter                 = pvrefitter,
        V0Tools                    = V0Tools,
        RefitPV                    = False,
        PVContainerName            = "BPHY25_JpsiXi_RefPrimaryVertices",
        OutputVtxContainerName     = "BPHY25_JpsiXi_JpsiVtx")

    BPHY25Rev_JpsiOmg = CompFactory.DerivationFramework.ReVertex(
        name                       = "BPHY25Rev_JpsiOmg",
        InputVtxContainerName      = "BPHY25_JpsiOmg_CascadeMainVtx",
        TrackContainerName         = mainIDInput,
        RelinkTracks               = TrkToRelink,
        TrackIndices               = [ 0, 1 ],
        UseMassConstraint          = True,
        VertexMass                 = Jpsimass,
        MassInputParticles         = [Mumass, Mumass],
        Chi2Cut                    = 25.,
        TrkVertexFitterTool	   = vkalvrt,
        PVRefitter                 = pvrefitter,
        V0Tools                    = V0Tools,
        RefitPV                    = False,
        PVContainerName            = "BPHY25_JpsiOmg_RefPrimaryVertices",
        OutputVtxContainerName     = "BPHY25_JpsiOmg_JpsiVtx")

    BPHY25Rev_BpmLd = CompFactory.DerivationFramework.ReVertex(
        name                       = "BPHY25Rev_BpmLd",
        InputVtxContainerName      = "BPHY25_BpmLd_CascadeVtx2",
        TrackContainerName         = mainIDInput,
        RelinkTracks               = TrkToRelink,
        TrackIndices               = [ 0, 1, 2 ],
        SubVertexTrackIndices      = [ 1, 2 ],
        UseMassConstraint          = True,
        SubVertexMass              = Jpsimass,
        MassInputParticles         = [Mumass, Mumass, Kmass],
        Chi2Cut                    = 25.,
        TrkVertexFitterTool	   = vkalvrt,
        PVRefitter                 = pvrefitter,
        V0Tools                    = V0Tools,
        RefitPV                    = False,
        PVContainerName            = "BPHY25_BpmLd_RefPrimaryVertices",
        OutputVtxContainerName     = "BPHY25_BpmLd_BpmVtx")

    BPHY25Rev_B0KpiLd = CompFactory.DerivationFramework.ReVertex(
        name                       = "BPHY25Rev_B0KpiLd",
        InputVtxContainerName      = "BPHY25_B0KpiLd_CascadeVtx2",
        TrackContainerName         = mainIDInput,
        RelinkTracks               = TrkToRelink,
        TrackIndices               = [ 0, 1, 2, 3 ],
        SubVertexTrackIndices      = [ 1, 2 ],
        UseMassConstraint          = True,
        VertexMass                 = B0mass,
        SubVertexMass              = Jpsimass,
        MassInputParticles         = [Mumass, Mumass, Kmass, Pimass],
        Chi2Cut                    = 25.,
        TrkVertexFitterTool	   = vkalvrt,
        PVRefitter                 = pvrefitter,
        V0Tools                    = V0Tools,
        RefitPV                    = False,
        PVContainerName            = "BPHY25_B0KpiLd_RefPrimaryVertices",
        OutputVtxContainerName     = "BPHY25_B0KpiLd_B0Vtx")

    BPHY25Rev_B0piKLd = CompFactory.DerivationFramework.ReVertex(
        name                       = "BPHY25Rev_B0piKLd",
        InputVtxContainerName      = "BPHY25_B0piKLd_CascadeVtx2",
        TrackContainerName         = mainIDInput,
        RelinkTracks               = TrkToRelink,
        TrackIndices               = [ 0, 1, 2, 3 ],
        SubVertexTrackIndices      = [ 1, 2 ],
        UseMassConstraint          = True,
        VertexMass                 = B0mass,
        SubVertexMass              = Jpsimass,
        MassInputParticles         = [Mumass, Mumass, Pimass, Kmass],
        Chi2Cut                    = 25.,
        TrkVertexFitterTool	   = vkalvrt,
        PVRefitter                 = pvrefitter,
        V0Tools                    = V0Tools,
        RefitPV                    = False,
        PVContainerName            = "BPHY25_B0piKLd_RefPrimaryVertices",
        OutputVtxContainerName     = "BPHY25_B0piKLd_B0Vtx")

    BPHY25Rev_B0KpiKs = CompFactory.DerivationFramework.ReVertex(
        name                       = "BPHY25Rev_B0KpiKs",
        InputVtxContainerName      = "BPHY25_B0KpiKs_CascadeVtx2",
        TrackContainerName         = mainIDInput,
        RelinkTracks               = TrkToRelink,
        TrackIndices               = [ 0, 1, 2, 3 ],
        SubVertexTrackIndices      = [ 1, 2 ],
        UseMassConstraint          = True,
        VertexMass                 = B0mass,
        SubVertexMass              = Jpsimass,
        MassInputParticles         = [Mumass, Mumass, Kmass, Pimass],
        Chi2Cut                    = 25.,
        TrkVertexFitterTool	   = vkalvrt,
        PVRefitter                 = pvrefitter,
        V0Tools                    = V0Tools,
        RefitPV                    = False,
        PVContainerName            = "BPHY25_B0KpiKs_RefPrimaryVertices",
        OutputVtxContainerName     = "BPHY25_B0KpiKs_B0Vtx")

    BPHY25Rev_B0piKKs = CompFactory.DerivationFramework.ReVertex(
        name                       = "BPHY25Rev_B0piKKs",
        InputVtxContainerName      = "BPHY25_B0piKKs_CascadeVtx2",
        TrackContainerName         = mainIDInput,
        RelinkTracks               = TrkToRelink,
        TrackIndices               = [ 0, 1, 2, 3 ],
        SubVertexTrackIndices      = [ 1, 2 ],
        UseMassConstraint          = True,
        VertexMass                 = B0mass,
        SubVertexMass              = Jpsimass,
        MassInputParticles         = [Mumass, Mumass, Pimass, Kmass],
        Chi2Cut                    = 25.,
        TrkVertexFitterTool	   = vkalvrt,
        PVRefitter                 = pvrefitter,
        V0Tools                    = V0Tools,
        RefitPV                    = False,
        PVContainerName            = "BPHY25_B0piKKs_RefPrimaryVertices",
        OutputVtxContainerName     = "BPHY25_B0piKKs_B0Vtx")

    BPHY25Rev_Bpm3body = CompFactory.DerivationFramework.ReVertex(
        name                       = "BPHY25Rev_Bpm3body",
        InputVtxContainerName      = "BPHY25_Bpm3body_CascadeMainVtx",
        TrackContainerName         = mainIDInput,
        RelinkTracks               = TrkToRelink,
        TrackIndices               = [ 0, 1, 2 ],
        SubVertexTrackIndices      = [ 1, 2 ],
        UseMassConstraint          = True,
        SubVertexMass              = Jpsimass,
        MassInputParticles         = [Mumass, Mumass, Protonmass],
        Chi2Cut                    = 25.,
        TrkVertexFitterTool	   = vkalvrt,
        PVRefitter                 = pvrefitter,
        V0Tools                    = V0Tools,
        RefitPV                    = False,
        PVContainerName            = "BPHY25_Bpm3body_RefPrimaryVertices",
        OutputVtxContainerName     = "BPHY25_Bpm3body_JpsiPVtx")

    BPHY25Rev_Xibpm3body = CompFactory.DerivationFramework.ReVertex(
        name                       = "BPHY25Rev_Xibpm3body",
        InputVtxContainerName      = "BPHY25_Xibpm3body_CascadeMainVtx",
        TrackContainerName         = mainIDInput,
        RelinkTracks               = TrkToRelink,
        TrackIndices               = [ 0, 1, 2 ],
        SubVertexTrackIndices      = [ 1, 2 ],
        UseMassConstraint          = True,
        SubVertexMass              = Jpsimass,
        MassInputParticles         = [Mumass, Mumass, Pimass],
        Chi2Cut                    = 25.,
        TrkVertexFitterTool	   = vkalvrt,
        PVRefitter                 = pvrefitter,
        V0Tools                    = V0Tools,
        RefitPV                    = False,
        PVContainerName            = "BPHY25_Xibpm3body_RefPrimaryVertices",
        OutputVtxContainerName     = "BPHY25_Xibpm3body_JpsiPiVtx")

    BPHY25Rev_XibpmRef = CompFactory.DerivationFramework.ReVertex(
        name                       = "BPHY25Rev_XibpmRef",
        InputVtxContainerName      = "BPHY25_XibpmRef_CascadeVtx2",
        TrackContainerName         = mainIDInput,
        RelinkTracks               = TrkToRelink,
        TrackIndices               = [ 0, 1 ],
        UseMassConstraint          = True,
        VertexMass                 = Jpsimass,
        MassInputParticles         = [Mumass, Mumass],
        Chi2Cut                    = 25.,
        TrkVertexFitterTool	   = vkalvrt,
        PVRefitter                 = pvrefitter,
        V0Tools                    = V0Tools,
        RefitPV                    = False,
        PVContainerName            = "BPHY25_XibpmRef_RefPrimaryVertices",
        OutputVtxContainerName     = "BPHY25_XibpmRef_JpsiVtx")

    BPHY25Rev_Xib03body = CompFactory.DerivationFramework.ReVertex(
        name                       = "BPHY25Rev_Xib03body",
        InputVtxContainerName      = "BPHY25_Xib03body_CascadeMainVtx",
        TrackContainerName         = mainIDInput,
        RelinkTracks               = TrkToRelink,
        TrackIndices               = [ 0, 1 ],
        UseMassConstraint          = True,
        VertexMass                 = Jpsimass,
        MassInputParticles         = [Mumass, Mumass],
        Chi2Cut                    = 25.,
        TrkVertexFitterTool	   = vkalvrt,
        PVRefitter                 = pvrefitter,
        V0Tools                    = V0Tools,
        RefitPV                    = False,
        PVContainerName            = "BPHY25_Xib03body_RefPrimaryVertices",
        OutputVtxContainerName     = "BPHY25_Xib03body_JpsiVtx")

    BPHY25Rev_Lambdab03body = CompFactory.DerivationFramework.ReVertex(
        name                       = "BPHY25Rev_Lambdab03body",
        InputVtxContainerName      = "BPHY25_Lambdab03body_CascadeMainVtx",
        TrackContainerName         = mainIDInput,
        RelinkTracks               = TrkToRelink,
        TrackIndices               = [ 0, 1 ],
        UseMassConstraint          = True,
        VertexMass                 = Jpsimass,
        MassInputParticles         = [Mumass, Mumass],
        Chi2Cut                    = 25.,
        TrkVertexFitterTool	   = vkalvrt,
        PVRefitter                 = pvrefitter,
        V0Tools                    = V0Tools,
        RefitPV                    = False,
        PVContainerName            = "BPHY25_Lambdab03body_RefPrimaryVertices",
        OutputVtxContainerName     = "BPHY25_Lambdab03body_JpsiVtx")

    BPHY25Rev_Jpsi2V0A = CompFactory.DerivationFramework.ReVertex(
        name                       = "BPHY25Rev_Jpsi2V0A",
        InputVtxContainerName      = "BPHY25_Jpsi2V0A_CascadeMainVtx",
        TrackContainerName         = mainIDInput,
        RelinkTracks               = TrkToRelink,
        TrackIndices               = [ 0, 1 ],
        UseMassConstraint          = True,
        VertexMass                 = Jpsimass,
        MassInputParticles         = [Mumass, Mumass],
        Chi2Cut                    = 25.,
        TrkVertexFitterTool	   = vkalvrt,
        PVRefitter                 = pvrefitter,
        V0Tools                    = V0Tools,
        RefitPV                    = False,
        PVContainerName            = "BPHY25_Jpsi2V0A_RefPrimaryVertices",
        OutputVtxContainerName     = "BPHY25_Jpsi2V0A_JpsiVtx")

    BPHY25Rev_Jpsi2V0B1 = CompFactory.DerivationFramework.ReVertex(
        name                       = "BPHY25Rev_Jpsi2V0B1",
        InputVtxContainerName      = "BPHY25_Jpsi2V0B1_CascadeVtx3",
        TrackContainerName         = mainIDInput,
        RelinkTracks               = TrkToRelink,
        TrackIndices               = [ 0, 1 ],
        UseMassConstraint          = True,
        VertexMass                 = Jpsimass,
        MassInputParticles         = [Mumass, Mumass],
        Chi2Cut                    = 25.,
        TrkVertexFitterTool	   = vkalvrt,
        PVRefitter                 = pvrefitter,
        V0Tools                    = V0Tools,
        RefitPV                    = False,
        PVContainerName            = "BPHY25_Jpsi2V0B1_RefPrimaryVertices",
        OutputVtxContainerName     = "BPHY25_Jpsi2V0B1_JpsiVtx")

    BPHY25Rev_Jpsi2V0B2 = CompFactory.DerivationFramework.ReVertex(
        name                       = "BPHY25Rev_Jpsi2V0B2",
        InputVtxContainerName      = "BPHY25_Jpsi2V0B2_CascadeVtx3",
        TrackContainerName         = mainIDInput,
        RelinkTracks               = TrkToRelink,
        TrackIndices               = [ 0, 1 ],
        UseMassConstraint          = True,
        VertexMass                 = Jpsimass,
        MassInputParticles         = [Mumass, Mumass],
        Chi2Cut                    = 25.,
        TrkVertexFitterTool	   = vkalvrt,
        PVRefitter                 = pvrefitter,
        V0Tools                    = V0Tools,
        RefitPV                    = False,
        PVContainerName            = "BPHY25_Jpsi2V0B2_RefPrimaryVertices",
        OutputVtxContainerName     = "BPHY25_Jpsi2V0B2_JpsiVtx")

    Collections        = [ ]
    RefPVContainers    = [ ]
    RefPVAuxContainers = [ ]
    passedCandidates   = [ ]

    list_obj = list_disV_obj + list_BpmLd_obj + list_B0Ld_obj + list_B0Ks_obj + list_3bodyA_obj + list_RefA_obj + list_3bodyB_obj + list_2V0A_obj + list_2V0B_obj
    list_rev = [BPHY25Rev_JpsiXi, BPHY25Rev_JpsiOmg, BPHY25Rev_BpmLd, BPHY25Rev_B0KpiLd, BPHY25Rev_B0piKLd, BPHY25Rev_B0KpiKs, BPHY25Rev_B0piKKs, BPHY25Rev_Bpm3body, BPHY25Rev_Xibpm3body, BPHY25Rev_XibpmRef, BPHY25Rev_Xib03body, BPHY25Rev_Lambdab03body, BPHY25Rev_Jpsi2V0A, BPHY25Rev_Jpsi2V0B1, BPHY25Rev_Jpsi2V0B2]

    for obj in list_obj:
        Collections += obj.CascadeVertexCollections
        RefPVContainers += ["xAOD::VertexContainer#BPHY25_" + obj.HypothesisName + "_RefPrimaryVertices"]
        RefPVAuxContainers += ["xAOD::VertexAuxContainer#BPHY25_" + obj.HypothesisName + "_RefPrimaryVerticesAux."]
        passedCandidates += ["BPHY25_" + obj.HypothesisName + "_CascadeMainVtx"]

    for obj in list_rev:
        Collections += [ obj.OutputVtxContainerName ]

    BPHY25_SelectEvent = CompFactory.DerivationFramework.AnyVertexSkimmingTool(name = "BPHY25_SelectEvent", VertexContainerNames = passedCandidates)
    acc.addPublicTool(BPHY25_SelectEvent)

    augmentation_tools = [BPHY25_AugOriginalCounts, BPHY25_Reco_mumu, BPHY25FourTrackReco_B0, BPHY25ThreeTrackReco_Bpm, BPHY25Select_Jpsi] + list_obj + list_rev
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
    acc.printConfig(withDetails=True, summariseProps=True, onlyComponents = [], printDefaults=True)
    return acc
