# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#### Low-level moments (clusters, FE, UFOs)
ClusterVariables = ["calE.calEta.calPhi.calM.rawE.rawEta.rawPhi.rawM.time.e_sampl"]

FlowElementVariables = ["pt.eta.phi.m.charge.signalType.chargedObjectLinks.otherObjectLinks",
                        "FE_ElectronLinks.FE_MuonLinks.FE_PhotonLinks.FE_TauLinks",
                        "GlobalFE_ElectronLinks.GlobalFE_MuonLinks.TCC_PhotonLinks.GlobalFE_TauLinks"]

UFOVariables = ["pt.eta.phi.m.signalType.otherObjectLinks.chargedObjectLinks"]

TrackingVariables = ["particleHypothesis.vx.vy.vz",
                     "btagIp_d0Uncertainty.btagIp_z0SinThetaUncertainty.btagIp_z0SinTheta.btagIp_trackMomentum.btagIp_trackDisplacement.btagIp_invalidIp",
                     "expectInnermostPixelLayerHit",
                     "numberOfInnermostPixelLayerHits.numberOfInnermostPixelLayerSharedHits.numberOfInnermostPixelLayerSplitHits.numberOfInnermostPixelLayerOutliers",
                     "numberOfNextToInnermostPixelLayerHits.numberOfNextToInnermostPixelLayerOutliers.numberOfNextToInnermostPixelLayerSharedHits.numberOfNextToInnermostPixelLayerSplitHits",
                     "numberOfPixelHits.numberOfPixelHoles.numberOfPixelSharedHits.numberOfPixelDeadSensors.numberOfPixelSplitHits.numberOfPixelSpoiltHits.numberOfPixelOutliers",
                     "numberOfSCTHits.numberOfSCTHoles.numberOfSCTSharedHits.numberOfSCTDeadSensors.numberOfSCTOutliers.numberOfSCTDoubleHoles.numberOfSCTSpoiltHits",
                     "leptonID.trackFitter.trackLink.trackProperties.AssoClustersUFO",
                     "ftagTruthOriginLabel.ftagTruthTypeLabel.ftagTruthVertexIndex.ftagTruthParentBarcode.ftagTruthBarcode",
                     "TTVA_AMVFVertices.TTVA_AMVFWeights"]

TrackingVariablesHGTD = ["HGTD_cluster_time", "HGTD_extrap_x", "HGTD_extrap_y"]

FELinks = ["Electrons.neutralGlobalFELinks.chargedGlobalFELinks",
           "Photons.neutralGlobalFELinks",
           "Muons.neutralGlobalFELinks.chargedGlobalFELinks"]

#### Substructure variables for R = 1.0 jets 
ExtraJSSVariables = ["C1.C2.C3.C4.D2",
                     "FoxWolfram0.FoxWolfram1.FoxWolfram2.FoxWolfram3.FoxWolfram4.FoxWolfram5",
                     "ThrustMaj.ThrustMin",
                     "L1.L2.L3.L4.L5.M2.M3.N2.N3",
                     "ECFG_2_1.ECFG_2_1_2",
                     "ECFG_3_1.ECFG_3_2.ECFG_3_1_1.ECFG_3_2_1.ECFG_3_2_2.ECFG_3_3_1.ECFG_3_3_2",
                     "ECFG_4_1.ECFG_4_2.ECFG_4_2_2.ECFG_4_4_1",
                     "Sphericity",
                     "Split34",
                     "ZCut23.ZCut34",
                     "rg.zg",
                     "Width.WidthPhi"]

### sub-structure ML taggers
JSSTaggersVariables = ["AntiKt4EMPFlowJets.QGTransformer_ConstScore"]
JSSTaggersVariables += ["AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets.TopTransformer_ConstScore"]
