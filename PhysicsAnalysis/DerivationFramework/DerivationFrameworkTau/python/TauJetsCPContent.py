# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#! Note: Please check the TauJets_MuonRMCPContent and TauJets_EleRMCPContent if any new containers is added here. 

TauJetsCPContent = [
    "TauJets",
    "TauJetsAux.pt.eta.phi.m.ptFinalCalib.etaFinalCalib.ptTauEnergyScale.etaTauEnergyScale.charge.nChargedTracks.nIsolatedTracks.nAllTracks.isTauFlags.PanTau_DecayMode.NNDecayMode.NNDecayModeProb_1p0n.NNDecayModeProb_1p1n.NNDecayModeProb_1pXn.NNDecayModeProb_3p0n.RNNJetScore.RNNJetScoreSigTrans.JetDeepSetScore.JetDeepSetScoreTrans.JetDeepSetVeryLoose.JetDeepSetLoose.JetDeepSetMedium.JetDeepSetTight.JetDeepSetScore_v2.JetDeepSetScoreTrans_v2.JetDeepSetVeryLoose_v2.JetDeepSetLoose_v2.JetDeepSetMedium_v2.JetDeepSetTight_v2.RNNEleScore.RNNEleScoreSigTrans_v1.EleRNNLoose_v1.EleRNNMedium_v1.EleRNNTight_v1.tauTrackLinks.vertexLink.secondaryVertexLink.neutralPFOLinks.pi0PFOLinks.truthParticleLink.truthJetLink.trackWidth.centFrac.etOverPtLeadTrk.innerTrkAvgDist.absipSigLeadTrk.SumPtTrkFrac.EMPOverTrkSysP.ptRatioEflowApprox.mEflowApprox.dRmax.trFlightPathSig.massTrkSys.leadTrackProbNNorHT.EMFracFixed.etHotShotWinOverPtLeadTrk.hadLeakFracFixed.PSFrac.ClustersMeanCenterLambda.ClustersMeanFirstEngDens.ClustersMeanPresamplerFrac.GNTauScore.GNTauScoreSigTrans_v0.GNTauVL_v0.GNTauL_v0.GNTauM_v0.GNTauT_v0",
    "TauTracks",
    "TauTracksAux.pt.eta.phi.flagSet.trackLinks.rnn_chargedScore.rnn_isolationScore.rnn_conversionScore",
    "InDetTrackParticles",
    "InDetTrackParticlesAux.phi.vertexLink.theta.qOverP.truthParticleLink.truthMatchProbability",
    "TauSecondaryVertices",
    "TauSecondaryVerticesAux.x.y.z.covariance.trackParticleLinks",
    "TauNeutralParticleFlowObjects",
    "TauNeutralParticleFlowObjectsAux.pt.eta.phi.m",
    "TruthTaus",
    "TruthTausAux.pt_vis_neutral_pions.pt_vis_neutral_others.numCharged.classifierParticleType.classifierParticleOrigin.IsHadronicTau.numNeutralPion.numNeutral.numChargedPion.pt_vis.eta_vis.phi_vis.m_vis",
]

