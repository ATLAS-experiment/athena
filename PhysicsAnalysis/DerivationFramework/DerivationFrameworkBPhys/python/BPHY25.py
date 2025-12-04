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
    Ldb0_lo = 5310.0
    Ldb0_hi = 5910.0

    Mumass = 105.658
    Pimass = 139.570
    Kmass = 493.677
    Ksmass = 497.611
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
        trackThresholdPt            = 3400.,
        invMassLower                = 500.,
        invMassUpper                = 4500.,
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
        RefitPV                = True,
        MaxPVrefit             = 500,
        RefPVContainerName     = "BPHY25_mumuRefittedPrimaryVertices",
        DoVertexType           = 7)

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
        RefitPV                  = True,
        MaxPVrefit               = 500,
        RefPVContainerName       = "BPHY25_B0RefittedPrimaryVertices",
        DoVertexType             = 7)

    BPHY25ThreeTrackReco_Bpm = CompFactory.DerivationFramework.Reco_Vertex(
        name                     = "BPHY25ThreeTrackReco_Bpm",
        VertexSearchTool         = BPHY25Bpm_Jpsi1Trk,
        OutputVtxContainerName   = "BPHY25ThreeTrack_Bpm",
        PVContainerName          = "PrimaryVertices",
        RelinkTracks             = TrkToRelink,
        V0Tools                  = V0Tools,
        PVRefitter               = pvrefitter,
        RefitPV                  = True,
        MaxPVrefit               = 500,
        RefPVContainerName       = "BPHY25_BpmRefittedPrimaryVertices",
        DoVertexType             = 7)

    BPHY25Select_Jpsi              = CompFactory.DerivationFramework.Select_onia2mumu(
        name                       = "BPHY25Select_Jpsi",
        HypothesisName             = "Jpsi",
        InputVtxContainerName      = "BPHY25OniaCandidates",
        V0Tools                    = V0Tools,
        TrkMasses                  = [Mumass, Mumass],
        MassMin                    = Jpsi_lo,
        MassMax                    = Jpsi_hi,
        DoVertexType               = 0)

    BPHY25Select_mumu              = CompFactory.DerivationFramework.Select_onia2mumu(
        name                       = "BPHY25Select_mumu",
        HypothesisName             = "mumu",
        InputVtxContainerName      = "BPHY25OniaCandidates",
        V0Tools                    = V0Tools,
        TrkMasses                  = [Mumass, Mumass],
        MassMin                    = 500.,
        MassMax                    = 4500.,
        DoVertexType               = 0)

    #####################################################
    ## Xi_b^- -> J/psi Xi-, Xi^- -> Lambda pi-         ##
    ## Omega_b^- -> J/psi Omega-, Omega^- -> Lambda K- ##
    #####################################################

    list_disV_hypo = ["JpsiXi", "JpsiOmg", "mumuXi"]
    list_disV_jxHypo = ["Jpsi", "Jpsi", "mumu"]
    list_disV_jxCons = [True, True, False]
    list_disV_disVLo = [Xi_lo, Omg_lo, Xi_lo]
    list_disV_disVHi = [Xi_hi, Omg_hi, Xi_hi]
    list_disV_disVDau3Mass = [Pimass, Kmass, Pimass]
    list_disV_disVDaug3MinPt = [650., 750., 650.]
    list_disV_jxMass = [Jpsimass, Jpsimass, -9999.]
    list_disV_disVMass = [Ximass, Omegamass, Ximass]

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
        list_disV_obj[i].LxyV0Cut                 = 1.
        list_disV_obj[i].LxyDisVtxCut             = 2.
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
        list_disV_obj[i].ApplyJpsiMassConstraint  = list_disV_jxCons[i]
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
        list_disV_obj[i].RefitPV                  = False
        list_disV_obj[i].PVContainerName          = "BPHY25_mumuRefittedPrimaryVertices"
        list_disV_obj[i].DoCascadeFitWithPV       = 2
        list_disV_obj[i].FirstDecayAtPV           = False
        list_disV_obj[i].TrkVertexFitterTool      = vkalvrt
        list_disV_obj[i].V0VertexFitterTool       = v0fitter
        list_disV_obj[i].GammaFitterTool          = gammafitter
        list_disV_obj[i].PVRefitter               = pvrefitter
        list_disV_obj[i].V0Tools                  = V0Tools
        list_disV_obj[i].TrackToVertexTool        = tracktovtxtool
        list_disV_obj[i].V0TrackSelectorTool      = v0trackselect
        list_disV_obj[i].TrackSelectorTool        = trackselect
        list_disV_obj[i].Extrapolator             = extrapolator

    #############
    ## B+ + V0 ##
    #############
    list_BpmV0_hypo = ["BpmLd", "BpmKs"]
    list_BpmV0_jxInput = ["BPHY25ThreeTrack_Bpm", "BPHY25ThreeTrack_Bpm"]
    list_BpmV0_jxDau3Mass = [Kmass, Kmass]
    list_BpmV0_jxMassLo = [B_lo, B_lo]
    list_BpmV0_jxMassHi = [B_hi, B_hi]
    list_BpmV0_v0hypo = ["Lambda", "Ks"]

    list_BpmV0_obj = []
    for hypo in list_BpmV0_hypo:
        list_BpmV0_obj.append( CompFactory.DerivationFramework.JpsiXPlusDisplaced("BPHY25_"+hypo) )

    for i in range(len(list_BpmV0_obj)):
        list_BpmV0_obj[i].JXVertices               = list_BpmV0_jxInput[i]
        list_BpmV0_obj[i].TrackParticleCollection  = mainIDInput
        list_BpmV0_obj[i].RelinkTracks             = TrkToRelink
        list_BpmV0_obj[i].V0Vertices               = "V0Collection"
        list_BpmV0_obj[i].UseImprovedMass          = True
        list_BpmV0_obj[i].LambdaMassLowerCut       = Ld_lo
        list_BpmV0_obj[i].LambdaMassUpperCut       = Ld_hi
        list_BpmV0_obj[i].KsMassLowerCut           = Ks_lo
        list_BpmV0_obj[i].KsMassUpperCut           = Ks_hi
        list_BpmV0_obj[i].JXMassLowerCut           = list_BpmV0_jxMassLo[i]
        list_BpmV0_obj[i].JXMassUpperCut           = list_BpmV0_jxMassHi[i]
        list_BpmV0_obj[i].JpsiMassLowerCut         = Jpsi_lo
        list_BpmV0_obj[i].JpsiMassUpperCut         = Jpsi_hi
        list_BpmV0_obj[i].MassLowerCut             = 0.
        list_BpmV0_obj[i].MassUpperCut             = 150000.
        list_BpmV0_obj[i].CascadeVertexCollections = ["BPHY25_"+list_BpmV0_hypo[i]+"_CascadeVtx1","BPHY25_"+list_BpmV0_hypo[i]+"_CascadeVtx2","BPHY25_"+list_BpmV0_hypo[i]+"_CascadeMainVtx"]
        list_BpmV0_obj[i].HasJXSubVertex           = True
        list_BpmV0_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
        list_BpmV0_obj[i].V0Hypothesis             = list_BpmV0_v0hypo[i]
        list_BpmV0_obj[i].MassCutGamma             = 10.
        list_BpmV0_obj[i].Chi2CutGamma             = 3.
        list_BpmV0_obj[i].LxyV0Cut                 = 3.
        list_BpmV0_obj[i].HypothesisName           = list_BpmV0_hypo[i]
        list_BpmV0_obj[i].NumberOfJXDaughters      = 3
        list_BpmV0_obj[i].JXDaug1MassHypo          = Mumass
        list_BpmV0_obj[i].JXDaug2MassHypo          = Mumass
        list_BpmV0_obj[i].JXDaug3MassHypo          = list_BpmV0_jxDau3Mass[i]
        list_BpmV0_obj[i].JXPtOrdering             = False
        list_BpmV0_obj[i].NumberOfDisVDaughters    = 2
        list_BpmV0_obj[i].JpsiMass                 = Jpsimass
        list_BpmV0_obj[i].LambdaMass               = Lambdamass
        list_BpmV0_obj[i].KsMass                   = Ksmass
        list_BpmV0_obj[i].ApplyJXMassConstraint    = True
        list_BpmV0_obj[i].ApplyJpsiMassConstraint  = True
        list_BpmV0_obj[i].ApplyV0MassConstraint    = True
        list_BpmV0_obj[i].ApplyMainVMassConstraint = False
        list_BpmV0_obj[i].Chi2CutV0                = 4.
        list_BpmV0_obj[i].Chi2Cut                  = 4.
        list_BpmV0_obj[i].Trackd0Cut               = 3.0
        list_BpmV0_obj[i].MaxJXCandidates          = 20
        list_BpmV0_obj[i].MaxV0Candidates          = 20
        list_BpmV0_obj[i].MaxMainVCandidates       = 30
        list_BpmV0_obj[i].RefitPV                  = False
        list_BpmV0_obj[i].PVContainerName          = "BPHY25_BpmRefittedPrimaryVertices"
        list_BpmV0_obj[i].DoCascadeFitWithPV       = 2
        list_BpmV0_obj[i].FirstDecayAtPV           = True
        list_BpmV0_obj[i].TrkVertexFitterTool      = vkalvrt
        list_BpmV0_obj[i].V0VertexFitterTool       = v0fitter
        list_BpmV0_obj[i].GammaFitterTool          = gammafitter
        list_BpmV0_obj[i].PVRefitter               = pvrefitter
        list_BpmV0_obj[i].V0Tools                  = V0Tools
        list_BpmV0_obj[i].TrackToVertexTool        = tracktovtxtool
        list_BpmV0_obj[i].V0TrackSelectorTool      = v0trackselect
        list_BpmV0_obj[i].TrackSelectorTool        = trackselect
        list_BpmV0_obj[i].Extrapolator             = extrapolator

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
        list_B0Ld_obj[i].LxyV0Cut                 = 3.
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
        list_B0Ld_obj[i].ApplyV0MassConstraint    = True
        list_B0Ld_obj[i].ApplyMainVMassConstraint = False
        list_B0Ld_obj[i].Chi2CutV0                = 4.
        list_B0Ld_obj[i].Chi2Cut                  = 4.
        list_B0Ld_obj[i].Trackd0Cut               = 3.0
        list_B0Ld_obj[i].MaxJXCandidates          = 20
        list_B0Ld_obj[i].MaxV0Candidates          = 20
        list_B0Ld_obj[i].MaxMainVCandidates       = 30
        list_B0Ld_obj[i].RefitPV                  = False
        list_B0Ld_obj[i].PVContainerName          = "BPHY25_B0RefittedPrimaryVertices"
        list_B0Ld_obj[i].DoCascadeFitWithPV       = 2
        list_B0Ld_obj[i].FirstDecayAtPV           = True
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
        list_B0Ks_obj[i].LxyV0Cut                 = 3.
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
        list_B0Ks_obj[i].MainVtxMass              = 5839.88 # Bs2
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
        list_B0Ks_obj[i].RefitPV                  = False
        list_B0Ks_obj[i].PVContainerName          = "BPHY25_B0RefittedPrimaryVertices"
        list_B0Ks_obj[i].DoCascadeFitWithPV       = 2
        list_B0Ks_obj[i].FirstDecayAtPV           = True
        list_B0Ks_obj[i].TrkVertexFitterTool      = vkalvrt
        list_B0Ks_obj[i].V0VertexFitterTool       = v0fitter
        list_B0Ks_obj[i].GammaFitterTool          = gammafitter
        list_B0Ks_obj[i].PVRefitter               = pvrefitter
        list_B0Ks_obj[i].V0Tools                  = V0Tools
        list_B0Ks_obj[i].TrackToVertexTool        = tracktovtxtool
        list_B0Ks_obj[i].V0TrackSelectorTool      = v0trackselect
        list_B0Ks_obj[i].TrackSelectorTool        = trackselect
        list_B0Ks_obj[i].Extrapolator             = extrapolator

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
        list_2V0A_obj[i].LxyV01Cut                = 2.
        list_2V0A_obj[i].LxyV02Cut                = 2.
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
        list_2V0A_obj[i].RefitPV                  = False
        list_2V0A_obj[i].PVContainerName          = "BPHY25_mumuRefittedPrimaryVertices"
        list_2V0A_obj[i].DoCascadeFitWithPV       = 2
        list_2V0A_obj[i].FirstDecayAtPV           = False
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
        list_2V0B_obj[i].LxyV01Cut                = 2.
        list_2V0B_obj[i].LxyV02Cut                = 2.
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
        list_2V0B_obj[i].RefitPV                  = False
        list_2V0B_obj[i].PVContainerName          = "BPHY25_mumuRefittedPrimaryVertices"
        list_2V0B_obj[i].DoCascadeFitWithPV       = 2
        list_2V0B_obj[i].FirstDecayAtPV           = True
        list_2V0B_obj[i].TrkVertexFitterTool      = vkalvrt
        list_2V0B_obj[i].V0VertexFitterTool       = v0fitter
        list_2V0B_obj[i].GammaFitterTool          = gammafitter
        list_2V0B_obj[i].PVRefitter               = pvrefitter
        list_2V0B_obj[i].V0Tools                  = V0Tools
        list_2V0B_obj[i].TrackToVertexTool        = tracktovtxtool
        list_2V0B_obj[i].V0TrackSelectorTool      = v0trackselect
        list_2V0B_obj[i].Extrapolator             = extrapolator

    Collections        = [ ]
    RefPVContainers    = [ ]
    RefPVAuxContainers = [ ]
    passedCandidates   = [ ]

    list_obj = list_disV_obj + list_BpmV0_obj + list_B0Ld_obj + list_B0Ks_obj + list_2V0A_obj + list_2V0B_obj

    for obj in list_obj:
        Collections += obj.CascadeVertexCollections
        passedCandidates += ["BPHY25_" + obj.HypothesisName + "_CascadeMainVtx"]

    RefPVContainers += [
        "xAOD::VertexContainer#BPHY25_mumuRefittedPrimaryVertices",
        "xAOD::VertexContainer#BPHY25_BpmRefittedPrimaryVertices",
        "xAOD::VertexContainer#BPHY25_B0RefittedPrimaryVertices"
    ]
    RefPVAuxContainers += [
        "xAOD::VertexAuxContainer#BPHY25_mumuRefittedPrimaryVerticesAux.",
        "xAOD::VertexAuxContainer#BPHY25_BpmRefittedPrimaryVerticesAux.",
        "xAOD::VertexAuxContainer#BPHY25_B0RefittedPrimaryVerticesAux."
    ]

    BPHY25_SelectEvent = CompFactory.DerivationFramework.AnyVertexSkimmingTool(name = "BPHY25_SelectEvent", VertexContainerNames = passedCandidates)
    acc.addPublicTool(BPHY25_SelectEvent)

    augmentation_tools = [BPHY25_AugOriginalCounts, BPHY25_Reco_mumu, BPHY25FourTrackReco_B0, BPHY25ThreeTrackReco_Bpm, BPHY25Select_Jpsi, BPHY25Select_mumu] + list_obj
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
