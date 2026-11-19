# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
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
    from DerivationFrameworkBPhys.CommonBPhysMethodsConfig import (
        BPHY_V0ToolCfg, BPHY_InDetDetailedTrackSelectorToolCfg,
        BPHY_VertexPointEstimatorCfg, BPHY_TrkVKalVrtFitterCfg,
        AugOriginalCountsCfg, Reco_VertexCfg, Select_onia2mumuCfg, JpsiXPlus2V0Cfg,
        JpsiXPlusDisplacedCfg)
    from JpsiUpsilonTools.JpsiUpsilonToolsConfig import PrimaryVertexRefittingToolCfg
    acc = ComponentAccumulator()
    isSimulation = flags.Input.isMC

    augmentation_tools = []
    # Adds primary vertex counts and track counts to EventInfo before they are thinned
    augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
        AugOriginalCountsCfg(flags, name = "BPHY25_AugOriginalCounts"))))

    doLRT = flags.Tracking.doLargeD0
    mainIDInput = "InDetWithLRTTrackParticles" if doLRT else "InDetTrackParticles"
    if doLRT:
        from DerivationFrameworkInDet.InDetToolsConfig import InDetLRTMergeCfg
        acc.merge(InDetLRTMergeCfg( flags, OutputTrackParticleLocation = mainIDInput ))

    TrkToRelink = ["InDetTrackParticles", "InDetLargeD0TrackParticles"] if doLRT else ["InDetTrackParticles"]

    # Configure common Public Tools
    V0Tools = acc.addPublicTool(acc.popToolsAndMerge(BPHY_V0ToolCfg(flags, BPHYDerivationName)))
    vkalvrt = acc.addPublicTool(acc.popToolsAndMerge(BPHY_TrkVKalVrtFitterCfg(flags, BPHYDerivationName))) # VKalVrt vertex fitter
    trackselect = acc.addPublicTool(acc.popToolsAndMerge(BPHY_InDetDetailedTrackSelectorToolCfg(flags, BPHYDerivationName)))
    pvrefitter = acc.addPublicTool(acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)))
    from TrkConfig.TrkV0FitterConfig import TrkV0VertexFitter_InDetExtrCfg
    v0fitter = acc.addPublicTool(acc.popToolsAndMerge(TrkV0VertexFitter_InDetExtrCfg(flags)))
    from TrackToVertex.TrackToVertexConfig import InDetTrackToVertexCfg
    tracktovtxtool = acc.addPublicTool(acc.popToolsAndMerge(InDetTrackToVertexCfg(flags)))
    from TrkConfig.TrkVKalVrtFitterConfig import V0VKalVrtFitterCfg
    gammafitter = acc.addPublicTool(acc.popToolsAndMerge(V0VKalVrtFitterCfg(
        flags, BPHYDerivationName+"_GammaFitter",
        Robustness          = 6,
        usePhiCnst          = True,
        useThetaCnst        = True,
        InputParticleMasses = [0.511,0.511] )))
    from InDetConfig.InDetTrackSelectorToolConfig import V0InDetConversionTrackSelectorToolCfg
    v0trackselect = acc.addPublicTool(acc.popToolsAndMerge(V0InDetConversionTrackSelectorToolCfg(flags)))
    from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
    extrapolator = acc.addPublicTool(acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags)))

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

    augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
        Reco_VertexCfg(flags,
                       name = "BPHY25_Reco_mumu",
                       VertexSearchTool = CompFactory.Analysis.JpsiFinder(
                           name = "BPHY25JpsiFinder",
                           muAndMu = True,
                           muAndTrack = False,
                           TrackAndTrack = False,
                           assumeDiMuons = True,  # If true, will assume dimu hypothesis and use PDG value for mu mass
                           trackThresholdPt = 3400.,
                           invMassLower = 500.,
                           invMassUpper = 4500.,
                           Chi2Cut = 4., # NDF=1 if no mass constraint
                           oppChargesOnly = True,
                           atLeastOneComb = True,
                           useCombinedMeasurement = False, # Only takes effect if combOnly=True
                           muonCollectionKey = "Muons",
                           TrackParticleCollection = "InDetTrackParticles",
                           TrkVertexFitterTool = vkalvrt, # PublicToolHandle
                           TrackSelectorTool = trackselect, # PublicToolHandle
                           VertexPointEstimator = acc.addPublicTool(acc.popToolsAndMerge(BPHY_VertexPointEstimatorCfg(flags, BPHYDerivationName))), # PublicToolHandle
                           useMCPCuts = False),
                       OutputVtxContainerName = "BPHY25OniaCandidates",
                       PVContainerName = "PrimaryVertices",
                       RelinkTracks = TrkToRelink,
                       V0Tools = V0Tools, # PublicToolHandle
                       PVRefitter = pvrefitter, #PublicToolHandle
                       RefitPV = True,
                       MaxPVrefit = 500,
                       RefPVContainerName = "BPHY25_mumuRefittedPrimaryVertices",
                       DoVertexType = 7)
    )))

    # B0 -> J/psi + K pi
    augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
        Reco_VertexCfg(flags,
                       name = "BPHY25FourTrackReco_B0",
                       VertexSearchTool = CompFactory.Analysis.JpsiPlus2Tracks(
                           name = "BPHY25B0_Jpsi2Trk",
                           kaonkaonHypothesis = False,
                           pionpionHypothesis = False,
                           kaonpionHypothesis = True,
                           kaonprotonHypothesis = False,
                           trkThresholdPt = 760.,
                           trkMaxEta = 2.6,
                           oppChargesOnly = False,
                           JpsiMassLower = Jpsi_lo,
                           JpsiMassUpper = Jpsi_hi,
                           TrkQuadrupletMassLower = B_lo-100,
                           TrkQuadrupletMassUpper = B_hi+100,
                           BMassLower = B_lo,
                           BMassUpper = B_hi,
                           DiTrackMassLower = 500.,
                           DiTrackMassUpper = 1100.,
                           Chi2Cut = 4.,
                           JpsiContainerKey = "BPHY25OniaCandidates",
                           TrackParticleCollection = "InDetTrackParticles",
                           MuonsUsedInJpsi = "Muons",
                           ExcludeJpsiMuonsOnly = True,
                           TrkVertexFitterTool = vkalvrt, # PublicToolHandle
                           TrackSelectorTool = trackselect, # PublicToolHandle
                           UseMassConstraint = True),
                       OutputVtxContainerName = "BPHY25FourTrack_B0",
                       PVContainerName = "PrimaryVertices",
                       RelinkTracks = TrkToRelink,
                       V0Tools = V0Tools, # PublicToolHandle
                       PVRefitter = pvrefitter, #PublicToolHandle
                       RefitPV = True,
                       MaxPVrefit = 500,
                       RefPVContainerName = "BPHY25_B0RefittedPrimaryVertices",
                       DoVertexType = 7)
    )))

    # B+ -> J/psi K
    augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
        Reco_VertexCfg(flags,
                       name = "BPHY25ThreeTrackReco_Bpm",
                       VertexSearchTool = CompFactory.Analysis.JpsiPlus1Track(
                           name = "BPHY25Bpm_Jpsi1Trk",
                           pionHypothesis = False,
                           kaonHypothesis = True,
                           trkThresholdPt = 950.,
                           trkMaxEta = 2.6,
                           JpsiMassLower = Jpsi_lo,
                           JpsiMassUpper = Jpsi_hi,
                           TrkTrippletMassLower = B_lo-100,
                           TrkTrippletMassUpper = B_hi+100,
                           BMassLower = B_lo,
                           BMassUpper = B_hi,
                           Chi2Cut = 4.,
                           JpsiContainerKey = "BPHY25OniaCandidates",
                           TrackParticleCollection = "InDetTrackParticles",
                           MuonsUsedInJpsi = "Muons",
                           ExcludeJpsiMuonsOnly = True,
                           TrkVertexFitterTool = vkalvrt, # PublicToolHandle
                           TrackSelectorTool = trackselect, # PublicToolHandle
                           UseMassConstraint = True),
                       OutputVtxContainerName = "BPHY25ThreeTrack_Bpm",
                       PVContainerName = "PrimaryVertices",
                       RelinkTracks = TrkToRelink,
                       V0Tools = V0Tools, # PublicToolHandle
                       PVRefitter = pvrefitter, #PublicToolHandle
                       RefitPV = True,
                       MaxPVrefit = 500,
                       RefPVContainerName = "BPHY25_BpmRefittedPrimaryVertices",
                       DoVertexType = 7)
    )))

    augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
        Select_onia2mumuCfg(flags,
                            name = "BPHY25Select_Jpsi",
                            HypothesisName = "Jpsi",
                            InputVtxContainerName = "BPHY25OniaCandidates",
                            V0Tools = V0Tools, # PublicToolHandle
                            TrkMasses = [Mumass, Mumass],
                            MassMin = Jpsi_lo,
                            MassMax = Jpsi_hi,
                            DoVertexType = 0)
    )))

    augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
        Select_onia2mumuCfg(flags,
                            name = "BPHY25Select_mumu",
                            HypothesisName = "mumu",
                            InputVtxContainerName = "BPHY25OniaCandidates",
                            V0Tools = V0Tools, # PublicToolHandle
                            TrkMasses = [Mumass, Mumass],
                            MassMin = 500.,
                            MassMax = 4500.,
                            DoVertexType = 0)
    )))

    Collections = [ ]
    passedCandidates   = [ ]

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

    for i, hypo in enumerate(list_disV_hypo):
        DisVArgs = { }
        DisVArgs.setdefault("JXVertices", "BPHY25OniaCandidates")
        DisVArgs.setdefault("JXVtxHypoNames", [list_disV_jxHypo[i]])
        DisVArgs.setdefault("TrackParticleCollection", mainIDInput)
        DisVArgs.setdefault("RelinkTracks", TrkToRelink)
        if i == 0:
            # create V0 container for all following instances
            DisVArgs.setdefault("OutputV0VtxCollection", "V0Collection")
        else:
            DisVArgs.setdefault("V0Vertices", "V0Collection")
        DisVArgs.setdefault("UseImprovedMass", True)
        DisVArgs.setdefault("LambdaMassLowerCut", Ld_lo)
        DisVArgs.setdefault("LambdaMassUpperCut", Ld_hi)
        DisVArgs.setdefault("KsMassLowerCut", Ks_lo)
        DisVArgs.setdefault("KsMassUpperCut", Ks_hi)
        DisVArgs.setdefault("DisplacedMassLowerCut", list_disV_disVLo[i])
        DisVArgs.setdefault("DisplacedMassUpperCut", list_disV_disVHi[i])
        DisVArgs.setdefault("MassLowerCut", 0.)
        DisVArgs.setdefault("MassUpperCut", 20000.)
        DisVArgs.setdefault("CascadeVertexCollections", ["BPHY25_"+hypo+"_CascadeVtx1_sub","BPHY25_"+hypo+"_CascadeVtx1","BPHY25_"+hypo+"_CascadeMainVtx"])
        DisVArgs.setdefault("HasJXSubVertex", False)
        DisVArgs.setdefault("VxPrimaryCandidateName", "PrimaryVertices")
        DisVArgs.setdefault("V0Hypothesis", "Lambda")
        DisVArgs.setdefault("MassCutGamma", 10.)
        DisVArgs.setdefault("Chi2CutGamma", 3.)
        DisVArgs.setdefault("LxyV0Cut", 1.)
        DisVArgs.setdefault("LxyDisVtxCut", 2.)
        DisVArgs.setdefault("HypothesisName", hypo)
        DisVArgs.setdefault("NumberOfJXDaughters", 2)
        DisVArgs.setdefault("JXDaug1MassHypo", Mumass)
        DisVArgs.setdefault("JXDaug2MassHypo", Mumass)
        DisVArgs.setdefault("NumberOfDisVDaughters", 3)
        DisVArgs.setdefault("DisVDaug3MassHypo", list_disV_disVDau3Mass[i])
        DisVArgs.setdefault("DisVDaug3MinPt", list_disV_disVDaug3MinPt[i])
        DisVArgs.setdefault("JpsiMass", list_disV_jxMass[i])
        DisVArgs.setdefault("LambdaMass", Lambdamass)
        DisVArgs.setdefault("KsMass", Ksmass)
        DisVArgs.setdefault("DisVtxMass", list_disV_disVMass[i])
        DisVArgs.setdefault("ApplyJpsiMassConstraint", list_disV_jxCons[i])
        DisVArgs.setdefault("ApplyV0MassConstraint", True)
        DisVArgs.setdefault("ApplyDisVMassConstraint", True)
        DisVArgs.setdefault("ApplyMainVMassConstraint", False)
        DisVArgs.setdefault("Chi2CutV0", 4.)
        DisVArgs.setdefault("Chi2CutDisV", 4.)
        DisVArgs.setdefault("Chi2Cut", 4.)
        DisVArgs.setdefault("Trackd0Cut", 3.0)
        DisVArgs.setdefault("MaxJXCandidates", 10)
        DisVArgs.setdefault("MaxV0Candidates", 20)
        DisVArgs.setdefault("MaxDisVCandidates", 30)
        DisVArgs.setdefault("MaxMainVCandidates", 30)
        DisVArgs.setdefault("RefitPV", False)
        DisVArgs.setdefault("PVContainerName", "BPHY25_mumuRefittedPrimaryVertices")
        DisVArgs.setdefault("DoCascadeFitWithPV", 2)
        DisVArgs.setdefault("FirstDecayAtPV", False)
        DisVArgs.setdefault("TrkVertexFitterTool", vkalvrt) # PublicToolHandle
        DisVArgs.setdefault("V0VertexFitterTool", v0fitter) # PublicToolHandle
        DisVArgs.setdefault("GammaFitterTool", gammafitter) # PublicToolHandle
        DisVArgs.setdefault("PVRefitter", pvrefitter) # PublicToolHandle
        DisVArgs.setdefault("V0Tools", V0Tools) # PublicToolHandle
        DisVArgs.setdefault("TrackToVertexTool", tracktovtxtool) #PublicToolHandle
        DisVArgs.setdefault("V0TrackSelectorTool", v0trackselect) #PublicToolHandle
        DisVArgs.setdefault("TrackSelectorTool", trackselect) # PublicToolHandle
        DisVArgs.setdefault("Extrapolator", extrapolator) # PublicToolHandle
        augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
            JpsiXPlusDisplacedCfg(flags, name = "BPHY25_"+hypo, **DisVArgs)
        )))
        Collections += DisVArgs["CascadeVertexCollections"]
        passedCandidates += ["BPHY25_" + hypo + "_CascadeMainVtx"]

    #############
    ## B+ + V0 ##
    #############
    list_BpmV0_hypo = ["BpmLd", "BpmKs"]
    list_BpmV0_jxInput = ["BPHY25ThreeTrack_Bpm", "BPHY25ThreeTrack_Bpm"]
    list_BpmV0_jxDau3Mass = [Kmass, Kmass]
    list_BpmV0_jxMassLo = [B_lo, B_lo]
    list_BpmV0_jxMassHi = [B_hi, B_hi]
    list_BpmV0_v0hypo = ["Lambda", "Ks"]

    for i, hypo in enumerate(list_BpmV0_hypo):
        BpmV0Args = { }
        BpmV0Args.setdefault("JXVertices", list_BpmV0_jxInput[i])
        BpmV0Args.setdefault("TrackParticleCollection", mainIDInput)
        BpmV0Args.setdefault("RelinkTracks", TrkToRelink)
        BpmV0Args.setdefault("V0Vertices", "V0Collection")
        BpmV0Args.setdefault("UseImprovedMass", True)
        BpmV0Args.setdefault("LambdaMassLowerCut", Ld_lo)
        BpmV0Args.setdefault("LambdaMassUpperCut", Ld_hi)
        BpmV0Args.setdefault("KsMassLowerCut", Ks_lo)
        BpmV0Args.setdefault("KsMassUpperCut", Ks_hi)
        BpmV0Args.setdefault("JXMassLowerCut", list_BpmV0_jxMassLo[i])
        BpmV0Args.setdefault("JXMassUpperCut", list_BpmV0_jxMassHi[i])
        BpmV0Args.setdefault("JpsiMassLowerCut", Jpsi_lo)
        BpmV0Args.setdefault("JpsiMassUpperCut", Jpsi_hi)
        BpmV0Args.setdefault("MassLowerCut", 0.)
        BpmV0Args.setdefault("MassUpperCut", 150000.)
        BpmV0Args.setdefault("CascadeVertexCollections", ["BPHY25_"+hypo+"_CascadeVtx1","BPHY25_"+hypo+"_CascadeVtx2","BPHY25_"+hypo+"_CascadeMainVtx"])
        BpmV0Args.setdefault("HasJXSubVertex", True)
        BpmV0Args.setdefault("VxPrimaryCandidateName", "PrimaryVertices")
        BpmV0Args.setdefault("V0Hypothesis", list_BpmV0_v0hypo[i])
        BpmV0Args.setdefault("MassCutGamma", 10.)
        BpmV0Args.setdefault("Chi2CutGamma", 3.)
        BpmV0Args.setdefault("LxyV0Cut", 3.)
        BpmV0Args.setdefault("HypothesisName", hypo)
        BpmV0Args.setdefault("NumberOfJXDaughters", 3)
        BpmV0Args.setdefault("JXDaug1MassHypo", Mumass)
        BpmV0Args.setdefault("JXDaug2MassHypo", Mumass)
        BpmV0Args.setdefault("JXDaug3MassHypo", list_BpmV0_jxDau3Mass[i])
        BpmV0Args.setdefault("JXPtOrdering", False)
        BpmV0Args.setdefault("NumberOfDisVDaughters", 2)
        BpmV0Args.setdefault("JpsiMass", Jpsimass)
        BpmV0Args.setdefault("LambdaMass", Lambdamass)
        BpmV0Args.setdefault("KsMass", Ksmass)
        BpmV0Args.setdefault("ApplyJXMassConstraint", True)
        BpmV0Args.setdefault("ApplyJpsiMassConstraint", True)
        BpmV0Args.setdefault("ApplyV0MassConstraint", True)
        BpmV0Args.setdefault("ApplyMainVMassConstraint", False)
        BpmV0Args.setdefault("Chi2CutV0", 4.)
        BpmV0Args.setdefault("Chi2Cut", 4.)
        BpmV0Args.setdefault("Trackd0Cut", 3.0)
        BpmV0Args.setdefault("MaxJXCandidates", 20)
        BpmV0Args.setdefault("MaxV0Candidates", 20)
        BpmV0Args.setdefault("MaxMainVCandidates", 30)
        BpmV0Args.setdefault("RefitPV", False)
        BpmV0Args.setdefault("PVContainerName", "BPHY25_BpmRefittedPrimaryVertices")
        BpmV0Args.setdefault("DoCascadeFitWithPV", 2)
        BpmV0Args.setdefault("FirstDecayAtPV", True)
        BpmV0Args.setdefault("TrkVertexFitterTool", vkalvrt) # PublicToolHandle
        BpmV0Args.setdefault("V0VertexFitterTool", v0fitter) # PublicToolHandle
        BpmV0Args.setdefault("GammaFitterTool", gammafitter) # PublicToolHandle
        BpmV0Args.setdefault("PVRefitter", pvrefitter) # PublicToolHandle
        BpmV0Args.setdefault("V0Tools", V0Tools) # PublicToolHandle
        BpmV0Args.setdefault("TrackToVertexTool", tracktovtxtool) #PublicToolHandle
        BpmV0Args.setdefault("V0TrackSelectorTool", v0trackselect) #PublicToolHandle
        BpmV0Args.setdefault("TrackSelectorTool", trackselect) # PublicToolHandle
        BpmV0Args.setdefault("Extrapolator", extrapolator) # PublicToolHandle
        augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
            JpsiXPlusDisplacedCfg(flags, name = "BPHY25_"+hypo, **BpmV0Args)
        )))
        Collections += BpmV0Args["CascadeVertexCollections"]
        passedCandidates += ["BPHY25_" + hypo + "_CascadeMainVtx"]

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

    for i, hypo in enumerate(list_B0Ld_hypo):
        B0LdArgs = { }
        B0LdArgs.setdefault("JXVertices", list_B0Ld_jxInput[i])
        B0LdArgs.setdefault("TrackParticleCollection", mainIDInput)
        B0LdArgs.setdefault("RelinkTracks", TrkToRelink)
        B0LdArgs.setdefault("V0Vertices", "V0Collection")
        B0LdArgs.setdefault("UseImprovedMass", True)
        B0LdArgs.setdefault("LambdaMassLowerCut", Ld_lo)
        B0LdArgs.setdefault("LambdaMassUpperCut", Ld_hi)
        B0LdArgs.setdefault("KsMassLowerCut", Ks_lo)
        B0LdArgs.setdefault("KsMassUpperCut", Ks_hi)
        B0LdArgs.setdefault("JXMassLowerCut", list_B0Ld_jxMassLo[i])
        B0LdArgs.setdefault("JXMassUpperCut", list_B0Ld_jxMassHi[i])
        B0LdArgs.setdefault("JpsiMassLowerCut", Jpsi_lo)
        B0LdArgs.setdefault("JpsiMassUpperCut", Jpsi_hi)
        B0LdArgs.setdefault("MassLowerCut", 0.)
        B0LdArgs.setdefault("MassUpperCut", 150000.)
        B0LdArgs.setdefault("CascadeVertexCollections", ["BPHY25_"+hypo+"_CascadeVtx1","BPHY25_"+hypo+"_CascadeVtx2","BPHY25_"+hypo+"_CascadeMainVtx"])
        B0LdArgs.setdefault("HasJXSubVertex", True)
        B0LdArgs.setdefault("VxPrimaryCandidateName", "PrimaryVertices")
        B0LdArgs.setdefault("V0Hypothesis", "Lambda")
        B0LdArgs.setdefault("MassCutGamma", 10.)
        B0LdArgs.setdefault("Chi2CutGamma", 3.)
        B0LdArgs.setdefault("LxyV0Cut", 3.)
        B0LdArgs.setdefault("HypothesisName", hypo)
        B0LdArgs.setdefault("NumberOfJXDaughters", 4)
        B0LdArgs.setdefault("JXDaug1MassHypo", Mumass)
        B0LdArgs.setdefault("JXDaug2MassHypo", Mumass)
        B0LdArgs.setdefault("JXDaug3MassHypo", list_B0Ld_jxDau3Mass[i])
        B0LdArgs.setdefault("JXDaug4MassHypo", list_B0Ld_jxDau4Mass[i])
        B0LdArgs.setdefault("JXPtOrdering", False)
        B0LdArgs.setdefault("NumberOfDisVDaughters", 2)
        B0LdArgs.setdefault("JXMass", list_B0Ld_jxMass[i])
        B0LdArgs.setdefault("JpsiMass", Jpsimass)
        B0LdArgs.setdefault("LambdaMass", Lambdamass)
        B0LdArgs.setdefault("KsMass", Ksmass)
        B0LdArgs.setdefault("ApplyJXMassConstraint", True)
        B0LdArgs.setdefault("ApplyJpsiMassConstraint", True)
        B0LdArgs.setdefault("ApplyV0MassConstraint", True)
        B0LdArgs.setdefault("ApplyMainVMassConstraint", False)
        B0LdArgs.setdefault("Chi2CutV0", 4.)
        B0LdArgs.setdefault("Chi2Cut", 4.)
        B0LdArgs.setdefault("Trackd0Cut", 3.0)
        B0LdArgs.setdefault("MaxJXCandidates", 20)
        B0LdArgs.setdefault("MaxV0Candidates", 20)
        B0LdArgs.setdefault("MaxMainVCandidates", 30)
        B0LdArgs.setdefault("RefitPV", False)
        B0LdArgs.setdefault("PVContainerName", "BPHY25_B0RefittedPrimaryVertices")
        B0LdArgs.setdefault("DoCascadeFitWithPV", 2)
        B0LdArgs.setdefault("FirstDecayAtPV", True)
        B0LdArgs.setdefault("TrkVertexFitterTool", vkalvrt) # PublicToolHandle
        B0LdArgs.setdefault("V0VertexFitterTool", v0fitter) # PublicToolHandle
        B0LdArgs.setdefault("GammaFitterTool", gammafitter) # PublicToolHandle
        B0LdArgs.setdefault("PVRefitter", pvrefitter) # PublicToolHandle
        B0LdArgs.setdefault("V0Tools", V0Tools) # PublicToolHandle
        B0LdArgs.setdefault("TrackToVertexTool", tracktovtxtool) #PublicToolHandle
        B0LdArgs.setdefault("V0TrackSelectorTool", v0trackselect) #PublicToolHandle
        B0LdArgs.setdefault("TrackSelectorTool", trackselect) # PublicToolHandle
        B0LdArgs.setdefault("Extrapolator", extrapolator) # PublicToolHandle
        augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
            JpsiXPlusDisplacedCfg(flags, name = "BPHY25_"+hypo, **B0LdArgs)
        )))
        Collections += B0LdArgs["CascadeVertexCollections"]
        passedCandidates += ["BPHY25_" + hypo + "_CascadeMainVtx"]

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

    for i, hypo in enumerate(list_B0Ks_hypo):
        B0KsArgs = { }
        B0KsArgs.setdefault("JXVertices",  list_B0Ks_jxInput[i])
        B0KsArgs.setdefault("TrackParticleCollection",  mainIDInput)
        B0KsArgs.setdefault("RelinkTracks",  TrkToRelink)
        B0KsArgs.setdefault("V0Vertices",  "V0Collection")
        B0KsArgs.setdefault("UseImprovedMass",  True)
        B0KsArgs.setdefault("LambdaMassLowerCut",  Ld_lo)
        B0KsArgs.setdefault("LambdaMassUpperCut",  Ld_hi)
        B0KsArgs.setdefault("KsMassLowerCut",  Ks_lo)
        B0KsArgs.setdefault("KsMassUpperCut",  Ks_hi)
        B0KsArgs.setdefault("JXMassLowerCut",  list_B0Ks_jxMassLo[i])
        B0KsArgs.setdefault("JXMassUpperCut",  list_B0Ks_jxMassHi[i])
        B0KsArgs.setdefault("JpsiMassLowerCut",  Jpsi_lo)
        B0KsArgs.setdefault("JpsiMassUpperCut",  Jpsi_hi)
        B0KsArgs.setdefault("MassLowerCut",  0.)
        B0KsArgs.setdefault("MassUpperCut",  150000.)
        B0KsArgs.setdefault("CascadeVertexCollections",  ["BPHY25_"+hypo+"_CascadeVtx1","BPHY25_"+hypo+"_CascadeVtx2","BPHY25_"+hypo+"_CascadeMainVtx"])
        B0KsArgs.setdefault("HasJXSubVertex",  True)
        B0KsArgs.setdefault("VxPrimaryCandidateName",  "PrimaryVertices")
        B0KsArgs.setdefault("V0Hypothesis",  "Ks")
        B0KsArgs.setdefault("MassCutGamma",  10.)
        B0KsArgs.setdefault("Chi2CutGamma",  3.)
        B0KsArgs.setdefault("LxyV0Cut",  3.)
        B0KsArgs.setdefault("HypothesisName",  hypo)
        B0KsArgs.setdefault("NumberOfJXDaughters",  4)
        B0KsArgs.setdefault("JXDaug1MassHypo",  Mumass)
        B0KsArgs.setdefault("JXDaug2MassHypo",  Mumass)
        B0KsArgs.setdefault("JXDaug3MassHypo",  list_B0Ks_jxDau3Mass[i])
        B0KsArgs.setdefault("JXDaug4MassHypo",  list_B0Ks_jxDau4Mass[i])
        B0KsArgs.setdefault("JXPtOrdering",  False)
        B0KsArgs.setdefault("NumberOfDisVDaughters",  2)
        B0KsArgs.setdefault("JXMass",  list_B0Ks_jxMass[i])
        B0KsArgs.setdefault("JpsiMass",  Jpsimass)
        B0KsArgs.setdefault("LambdaMass",  Lambdamass)
        B0KsArgs.setdefault("KsMass",  Ksmass)
        B0KsArgs.setdefault("MainVtxMass",  5839.88) # Bs2
        B0KsArgs.setdefault("ApplyJXMassConstraint",  True)
        B0KsArgs.setdefault("ApplyJpsiMassConstraint",  True)
        B0KsArgs.setdefault("ApplyV0MassConstraint",  True)
        B0KsArgs.setdefault("ApplyMainVMassConstraint",  False)
        B0KsArgs.setdefault("Chi2CutV0",  5.)
        B0KsArgs.setdefault("Chi2Cut",  5.)
        B0KsArgs.setdefault("Trackd0Cut",  3.0)
        B0KsArgs.setdefault("MaxJXCandidates",  20)
        B0KsArgs.setdefault("MaxV0Candidates",  20)
        B0KsArgs.setdefault("MaxMainVCandidates",  30)
        B0KsArgs.setdefault("RefitPV",  False)
        B0KsArgs.setdefault("PVContainerName",  "BPHY25_B0RefittedPrimaryVertices")
        B0KsArgs.setdefault("DoCascadeFitWithPV",  2)
        B0KsArgs.setdefault("FirstDecayAtPV",  True)
        B0KsArgs.setdefault("TrkVertexFitterTool",  vkalvrt) # PublicToolHandle
        B0KsArgs.setdefault("V0VertexFitterTool",  v0fitter) # PublicToolHandle
        B0KsArgs.setdefault("GammaFitterTool",  gammafitter)
        B0KsArgs.setdefault("PVRefitter",  pvrefitter) # PublicToolHandle
        B0KsArgs.setdefault("V0Tools",  V0Tools) # PublicToolHandle
        B0KsArgs.setdefault("TrackToVertexTool",  tracktovtxtool) # PublicToolHandle
        B0KsArgs.setdefault("V0TrackSelectorTool",  v0trackselect)
        B0KsArgs.setdefault("TrackSelectorTool",  trackselect) # PublicToolHandle
        B0KsArgs.setdefault("Extrapolator",  extrapolator)
        augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
            JpsiXPlusDisplacedCfg(flags, name = "BPHY25_"+hypo, **B0KsArgs)
        )))
        Collections += B0KsArgs["CascadeVertexCollections"]
        passedCandidates += ["BPHY25_" + hypo + "_CascadeMainVtx"]

    #################
    ## J/psi + 2V0 ##
    #################

    list_2V0A_hypo = ["Jpsi2V0A"]

    for i, hypo in enumerate(list_2V0A_hypo):
        TwoV0AArgs = { }
        TwoV0AArgs.setdefault("JXVertices",  "BPHY25OniaCandidates")
        TwoV0AArgs.setdefault("JXVtxHypoNames",  ["Jpsi"])
        TwoV0AArgs.setdefault("TrackParticleCollection",  mainIDInput)
        TwoV0AArgs.setdefault("RelinkTracks",  TrkToRelink)
        TwoV0AArgs.setdefault("V0Vertices",  "V0Collection")
        TwoV0AArgs.setdefault("UseImprovedMass",  True)
        TwoV0AArgs.setdefault("LambdaMassLowerCut",  Ld_lo)
        TwoV0AArgs.setdefault("LambdaMassUpperCut",  Ld_hi)
        TwoV0AArgs.setdefault("KsMassLowerCut",  Ks_lo)
        TwoV0AArgs.setdefault("KsMassUpperCut",  Ks_hi)
        TwoV0AArgs.setdefault("JpsiMassLowerCut",  Jpsi_lo)
        TwoV0AArgs.setdefault("JpsiMassUpperCut",  Jpsi_hi)
        TwoV0AArgs.setdefault("MassLowerCut",  0.)
        TwoV0AArgs.setdefault("MassUpperCut",  150000.)
        TwoV0AArgs.setdefault("CascadeVertexCollections",  ["BPHY25_"+hypo+"_CascadeVtx1","BPHY25_"+hypo+"_CascadeVtx2","BPHY25_"+hypo+"_CascadeMainVtx"])
        TwoV0AArgs.setdefault("HasJXSubVertex",  False)
        TwoV0AArgs.setdefault("HasJXV02SubVertex",  False)
        TwoV0AArgs.setdefault("VxPrimaryCandidateName",  "PrimaryVertices")
        TwoV0AArgs.setdefault("V01Hypothesis",  "Lambda/Ks")
        TwoV0AArgs.setdefault("V02Hypothesis",  "Lambda/Ks")
        TwoV0AArgs.setdefault("LxyV01Cut",  2.)
        TwoV0AArgs.setdefault("LxyV02Cut",  2.)
        TwoV0AArgs.setdefault("MassCutGamma",  10.)
        TwoV0AArgs.setdefault("Chi2CutGamma",  3.)
        TwoV0AArgs.setdefault("HypothesisName",  hypo)
        TwoV0AArgs.setdefault("NumberOfJXDaughters",  2)
        TwoV0AArgs.setdefault("JXDaug1MassHypo",  Mumass)
        TwoV0AArgs.setdefault("JXDaug2MassHypo",  Mumass)
        TwoV0AArgs.setdefault("JpsiMass",  Jpsimass)
        TwoV0AArgs.setdefault("LambdaMass",  Lambdamass)
        TwoV0AArgs.setdefault("KsMass",  Ksmass)
        TwoV0AArgs.setdefault("ApplyJpsiMassConstraint",  True)
        TwoV0AArgs.setdefault("ApplyV01MassConstraint",  True)
        TwoV0AArgs.setdefault("ApplyV02MassConstraint",  True)
        TwoV0AArgs.setdefault("ApplyMainVMassConstraint",  False)
        TwoV0AArgs.setdefault("Chi2CutV0",  4.)
        TwoV0AArgs.setdefault("Chi2Cut",  4.)
        TwoV0AArgs.setdefault("Trackd0Cut",  3.0)
        TwoV0AArgs.setdefault("MaxJXCandidates",  10)
        TwoV0AArgs.setdefault("MaxV0Candidates",  20)
        TwoV0AArgs.setdefault("MaxMainVCandidates",  30)
        TwoV0AArgs.setdefault("RefitPV",  False)
        TwoV0AArgs.setdefault("PVContainerName",  "BPHY25_mumuRefittedPrimaryVertices")
        TwoV0AArgs.setdefault("DoCascadeFitWithPV",  2)
        TwoV0AArgs.setdefault("FirstDecayAtPV",  False)
        TwoV0AArgs.setdefault("TrkVertexFitterTool",  vkalvrt) # PublicToolHandle
        TwoV0AArgs.setdefault("V0VertexFitterTool",  v0fitter) # PublicToolHandle
        TwoV0AArgs.setdefault("GammaFitterTool",  gammafitter)
        TwoV0AArgs.setdefault("PVRefitter",  pvrefitter) # PublicToolHandle
        TwoV0AArgs.setdefault("V0Tools",  V0Tools) # PublicToolHandle
        TwoV0AArgs.setdefault("TrackToVertexTool",  tracktovtxtool) # PublicToolHandle
        TwoV0AArgs.setdefault("V0TrackSelectorTool",  v0trackselect)
        TwoV0AArgs.setdefault("Extrapolator",  extrapolator)
        augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
            JpsiXPlus2V0Cfg(flags, name = "BPHY25_"+hypo, **TwoV0AArgs)
        )))
        Collections += TwoV0AArgs["CascadeVertexCollections"]
        passedCandidates += ["BPHY25_" + hypo + "_CascadeMainVtx"]

    list_2V0B_hypo = ["Jpsi2V0B1", "Jpsi2V0B2"]
    list_2V0B_v02hypo = ["Lambda", "Ks"]
    list_2V0B_jxv02mass = [Lambdab0mass, B0mass]
    list_2V0B_jxv02massLo = [Ldb0_lo, B_lo]
    list_2V0B_jxv02massHi = [Ldb0_hi, B_hi]

    for i, hypo in enumerate(list_2V0B_hypo):
        TwoV0BArgs = { }
        TwoV0BArgs.setdefault("JXVertices",  "BPHY25OniaCandidates")
        TwoV0BArgs.setdefault("JXVtxHypoNames",  ["Jpsi"])
        TwoV0BArgs.setdefault("TrackParticleCollection",  mainIDInput)
        TwoV0BArgs.setdefault("RelinkTracks",  TrkToRelink)
        TwoV0BArgs.setdefault("V0Vertices",  "V0Collection")
        TwoV0BArgs.setdefault("UseImprovedMass",  True)
        TwoV0BArgs.setdefault("LambdaMassLowerCut",  Ld_lo)
        TwoV0BArgs.setdefault("LambdaMassUpperCut",  Ld_hi)
        TwoV0BArgs.setdefault("KsMassLowerCut",  Ks_lo)
        TwoV0BArgs.setdefault("KsMassUpperCut",  Ks_hi)
        TwoV0BArgs.setdefault("JpsiMassLowerCut",  Jpsi_lo)
        TwoV0BArgs.setdefault("JpsiMassUpperCut",  Jpsi_hi)
        TwoV0BArgs.setdefault("JXV02MassLowerCut",  list_2V0B_jxv02massLo[i])
        TwoV0BArgs.setdefault("JXV02MassUpperCut",  list_2V0B_jxv02massHi[i])
        TwoV0BArgs.setdefault("MassLowerCut",  0.)
        TwoV0BArgs.setdefault("MassUpperCut",  150000.)
        TwoV0BArgs.setdefault("CascadeVertexCollections",  ["BPHY25_"+hypo+"_CascadeVtx1","BPHY25_"+hypo+"_CascadeVtx2","BPHY25_"+hypo+"_CascadeVtx3","BPHY25_"+hypo+"_CascadeMainVtx"])
        TwoV0BArgs.setdefault("HasJXSubVertex",  True)
        TwoV0BArgs.setdefault("HasJXV02SubVertex",  True)
        TwoV0BArgs.setdefault("VxPrimaryCandidateName",  "PrimaryVertices")
        TwoV0BArgs.setdefault("V01Hypothesis",  "Lambda/Ks")
        TwoV0BArgs.setdefault("V02Hypothesis",  list_2V0B_v02hypo[i])
        TwoV0BArgs.setdefault("LxyV01Cut",  2.)
        TwoV0BArgs.setdefault("LxyV02Cut",  2.)
        TwoV0BArgs.setdefault("MassCutGamma",  10.)
        TwoV0BArgs.setdefault("Chi2CutGamma",  3.)
        TwoV0BArgs.setdefault("HypothesisName",  hypo)
        TwoV0BArgs.setdefault("NumberOfJXDaughters",  2)
        TwoV0BArgs.setdefault("JXDaug1MassHypo",  Mumass)
        TwoV0BArgs.setdefault("JXDaug2MassHypo",  Mumass)
        TwoV0BArgs.setdefault("JpsiMass",  Jpsimass)
        TwoV0BArgs.setdefault("LambdaMass",  Lambdamass)
        TwoV0BArgs.setdefault("KsMass",  Ksmass)
        TwoV0BArgs.setdefault("JXV02VtxMass",  list_2V0B_jxv02mass[i])
        TwoV0BArgs.setdefault("ApplyJpsiMassConstraint",  True)
        TwoV0BArgs.setdefault("ApplyV01MassConstraint",  True)
        TwoV0BArgs.setdefault("ApplyV02MassConstraint",  True)
        TwoV0BArgs.setdefault("ApplyJXV02MassConstraint",  True)
        TwoV0BArgs.setdefault("ApplyMainVMassConstraint",  False)
        TwoV0BArgs.setdefault("Chi2CutV0",  4.)
        TwoV0BArgs.setdefault("Chi2Cut",  4.)
        TwoV0BArgs.setdefault("Trackd0Cut",  3.0)
        TwoV0BArgs.setdefault("MaxJXCandidates",  10)
        TwoV0BArgs.setdefault("MaxV0Candidates",  20)
        TwoV0BArgs.setdefault("MaxMainVCandidates",  30)
        TwoV0BArgs.setdefault("RefitPV",  False)
        TwoV0BArgs.setdefault("PVContainerName",  "BPHY25_mumuRefittedPrimaryVertices")
        TwoV0BArgs.setdefault("DoCascadeFitWithPV",  2)
        TwoV0BArgs.setdefault("FirstDecayAtPV",  True)
        TwoV0BArgs.setdefault("TrkVertexFitterTool",  vkalvrt) # PublicToolHandle
        TwoV0BArgs.setdefault("V0VertexFitterTool",  v0fitter) # PublicToolHandle
        TwoV0BArgs.setdefault("GammaFitterTool",  gammafitter)
        TwoV0BArgs.setdefault("PVRefitter",  pvrefitter) # PublicToolHandle
        TwoV0BArgs.setdefault("V0Tools",  V0Tools) # PublicToolHandle
        TwoV0BArgs.setdefault("TrackToVertexTool",  tracktovtxtool) # PublicToolHandle
        TwoV0BArgs.setdefault("V0TrackSelectorTool",  v0trackselect)
        TwoV0BArgs.setdefault("Extrapolator",  extrapolator)
        augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
            JpsiXPlus2V0Cfg(flags, name = "BPHY25_"+hypo, **TwoV0BArgs)
        )))
        Collections += TwoV0BArgs["CascadeVertexCollections"]
        passedCandidates += ["BPHY25_" + hypo + "_CascadeMainVtx"]

    RefPVContainers = [
        "xAOD::VertexContainer#BPHY25_mumuRefittedPrimaryVertices",
        "xAOD::VertexContainer#BPHY25_BpmRefittedPrimaryVertices",
        "xAOD::VertexContainer#BPHY25_B0RefittedPrimaryVertices"
    ]
    RefPVAuxContainers = [
        "xAOD::VertexAuxContainer#BPHY25_mumuRefittedPrimaryVerticesAux.",
        "xAOD::VertexAuxContainer#BPHY25_BpmRefittedPrimaryVerticesAux.",
        "xAOD::VertexAuxContainer#BPHY25_B0RefittedPrimaryVerticesAux."
    ]

    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel(
        "BPHY25Kernel",
        AugmentationTools = augmentation_tools,
        SkimmingTools     = [acc.addPublicTool(CompFactory.DerivationFramework.AnyVertexSkimmingTool(name = "BPHY25_SelectEvent", VertexContainerNames = passedCandidates))]
    ))

    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    BPHY25SlimmingHelper = SlimmingHelper("BPHY25SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
    from DerivationFrameworkBPhys.CommonBPhysMethodsConfig import getDefaultAllVariables
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
