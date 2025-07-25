# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import unittest
from AthenaConfiguration.AthConfigFlags import AthConfigFlags
import AthenaCommon.SystemOfUnits as Units
from AthenaConfiguration.Enums import LHCPeriod

def createTauConfigFlags():
    tau_cfg = AthConfigFlags()

    tau_cfg.addFlag("Tau.doTauRec", True)
    tau_cfg.addFlag("Tau.ThinTaus", True)
    tau_cfg.addFlag("Tau.doEarlyStopping", True)

    # Switches for enabling/disabling some tools
    tau_cfg.addFlag("Tau.doTJVA", True)
    tau_cfg.addFlag("Tau.doPi0Clus", True)
    tau_cfg.addFlag("Tau.doPanTau", True)
    tau_cfg.addFlag("Tau.doRNNTrackClass", True)
    tau_cfg.addFlag("Tau.doTauDiscriminant", True)
    tau_cfg.addFlag("Tau.doTauNeutralFourVecNNRegression", True)
    tau_cfg.addFlag("Tau.associateLRT", False)
    tau_cfg.addFlag("Tau.isStandalone", False)
    # Classify Large Radius Tracks in tau track classifier
    tau_cfg.addFlag("Tau.classifyLRT", False)

    # Settings common to Run2 and Run3
    tau_cfg.addFlag("Tau.SeedMinPt", 0.0*Units.GeV)
    tau_cfg.addFlag("Tau.SeedMaxEta", lambda pcf: 2.5 if pcf.GeoModel.Run <= LHCPeriod.Run3 else 4.0)
    # FIXME: MaxNTracks is not used, drop at the next occasion
    tau_cfg.addFlag("Tau.MaxNTracks", -1)
    tau_cfg.addFlag("Tau.RemoveDupeCoreTracks", True)
    tau_cfg.addFlag("Tau.doTJVATiebreak", True)
    tau_cfg.addFlag("Tau.useGhostTracks", True)
    tau_cfg.addFlag("Tau.ghostTrackDR", 0.25)
    tau_cfg.addFlag("Tau.shotPtCut_1Photon", [430.*Units.MeV, 300.*Units.MeV, 9999999.*Units.MeV, 330.*Units.MeV, 350.*Units.MeV])
    tau_cfg.addFlag("Tau.shotPtCut_2Photons", [10000.*Units.MeV, 10000.*Units.MeV, 9999999.*Units.MeV, 10000.*Units.MeV, 10000.*Units.MeV])
    tau_cfg.addFlag("Tau.Pi0ScoreConfig", "TauPi0BDTWeights.root")
    tau_cfg.addFlag("Tau.pi0EtCuts", [2800.*Units.MeV, 2700.*Units.MeV, 2500.*Units.MeV, 2900.*Units.MeV, 2700.*Units.MeV])
    tau_cfg.addFlag("Tau.pi0MVACuts_1prong", [0.45, 0.37, 0.39, 0.40, 0.38])
    tau_cfg.addFlag("Tau.pi0MVACuts_mprong", [0.73, 0.69, 0.58, 0.69, 0.67])

    # Run2 settings and calibration files
    tau_cfg.addFlag("Tau.tauRecToolsCVMFSPath", "tauRecTools/R22_preprod")
    tau_cfg.addFlag("Tau.tauRNNTrackClassConfig", "RNNTrackClassifier_2021-07-19_14-25-14_90_25_30.json")
    tau_cfg.addFlag("Tau.CalibrateLCConfig", "CaloTES_R22_Round2.5.root")
    tau_cfg.addFlag("Tau.CombinedTESConfig", "CombinedTES_R22_Round2.5.root")
    tau_cfg.addFlag("Tau.MvaTESConfig0p", "MvaTES_0p_R23.root")
    tau_cfg.addFlag("Tau.MvaTESConfig", "MvaTES_R23.root")
    tau_cfg.addFlag("Tau.MinPt0p", 9.25*Units.GeV)
    tau_cfg.addFlag("Tau.MinPt", 6.75*Units.GeV)
    tau_cfg.addFlag("Tau.MinPtDAOD", 13*Units.GeV)
    tau_cfg.addFlag("Tau.TauJetRNNConfig", ["tauid_rnn_1p_R22_v1.json", "tauid_rnn_2p_R22_v1.json", "tauid_rnn_3p_R22_v1.json"])
    tau_cfg.addFlag("Tau.TauJetRNNWPConfig", ["tauid_rnnWP_1p_R22_v0.root", "tauid_rnnWP_2p_R22_v0.root", "tauid_rnnWP_3p_R22_v0.root"])
    tau_cfg.addFlag("Tau.TauEleRNNConfig", ["taueveto_rnn_config_1P_r22.json", "taueveto_rnn_config_3P_r22.json"])
    tau_cfg.addFlag("Tau.TauEleRNNWPConfig", ["taueveto_rnn_flat_1P_r22.root", "taueveto_rnn_flat_3P_r22.root"])
    tau_cfg.addFlag("Tau.DecayModeNNClassifierConfig", "NNDecayMode_R22_v1.json")
    tau_cfg.addFlag("Tau.NeutralFourVecNNRegressionConfig1p1n", "NNDecayMode_R22_v1.json") # using the decay mode classifier network as a placeholder for testing
    tau_cfg.addFlag("Tau.NeutralFourVecNNRegressionConfig1pXn", "NNDecayMode_R22_v1.json") # using the decay mode classifier network as a placeholder for testing
    tau_cfg.addFlag("Tau.NeutralFourVecNNRegressionConfig3pXn", "NNDecayMode_R22_v1.json") # using the decay mode classifier network as a placeholder for testing
    tau_cfg.addFlag("Tau.TauEleRNNWPfix", ["rnneveto_mc16d_flat_1p_fix.root", "rnneveto_mc16d_flat_3p_fix.root"])
    # R22 DeepSet tau ID tune with track RNN scores
    tau_cfg.addFlag("Tau.TauJetDeepSetConfig", ["tauid_R22_1p_trk_dpst_notrkfakeRNN.json", "tauid_R22_2p_trk_dpst.json", "tauid_R22_3p_trk_dpst.json"])
    tau_cfg.addFlag("Tau.TauJetDeepSetWP", ["model_R22_1p_trk_dpst_notrkfakeRNN.root", "model_R22_2p_trk_dpst.root", "model_R22_3p_trk_dpst.root"])
    # R22 DeepSet tau ID tune without track RNN scores, for now define a second set of flags, but ultimately we'll choose one and drop the other
    tau_cfg.addFlag("Tau.TauJetDeepSetConfig_v2", ["tauid_1p_R22_dpst_noTrackScore.json", "tauid_2p_R22_dpst_noTrackScore.json", "tauid_3p_R22_dpst_noTrackScore.json"])
    tau_cfg.addFlag("Tau.TauJetDeepSetWP_v2", ["model_1p_R22_dpst_noTrackScore.root", "model_2p_R22_dpst_noTrackScore.root", "model_3p_R22_dpst_noTrackScore.root"])
    # GNTau ID tune file (need to add another version for noAux)
    tau_cfg.addFlag("Tau.TauGNNConfig", ["GNTau_pruned_MC23.onnx","GNTau_trunc_MC23.onnx"])
    tau_cfg.addFlag("Tau.TauGNNWP",
                    [ 
                        ["GNTauNAprune_flat_model_1p.root", "GNTauNAprune_flat_model_2p.root", "GNTauNAprune_flat_model_3p.root"],
                        ["GNTauNAtrunc_flat_model_1p.root", "GNTauNAtrunc_flat_model_2p.root", "GNTauNAtrunc_flat_model_3p.root"]
                    ])
    tau_cfg.addFlag("Tau.GNTauScoreName", ["GNTauScore_v0prune","GNTauScore_v1trunc"])
    tau_cfg.addFlag("Tau.GNTauTransScoreName", ["GNTauScoreSigTrans_v0prune","GNTauScoreSigTrans_v1trunc"])
    tau_cfg.addFlag("Tau.GNTauMaxTracks", [30,10])
    tau_cfg.addFlag("Tau.GNTauMaxClusters", [20,6])
    tau_cfg.addFlag("Tau.GNTauNodeNameTau", "GN2TauNoAux_pb")
    tau_cfg.addFlag("Tau.GNTauNodeNameJet", "GN2TauNoAux_pu")
    tau_cfg.addFlag("Tau.GNTauDecorWPNames", 
                    [
                        ["GNTauVL_v0prune", "GNTauL_v0prune", "GNTauM_v0prune", "GNTauT_v0prune"],
                        ["GNTauVL_v1trunc", "GNTauL_v1trunc", "GNTauM_v1trunc", "GNTauT_v1trunc"]
                    ])


    # PanTau config flags
    from PanTauAlgs.PanTauConfigFlags import createPanTauConfigFlags
    tau_cfg.addFlagsCategory("Tau.PanTau", createPanTauConfigFlags, prefix=True)

    # create 2 flag categories, for standard taus and electron-subtracted taus
    tau_cfg.addFlagsCategory("Tau.TauRec", createTauRecConfigFlags, prefix=True)
    tau_cfg.addFlagsCategory("Tau.TauEleRM", createTauEleRMConfigFlags, prefix=True)
    # define ActiveConfig in TauConfigFlags.py so it exists for client code like DerivationFramework that don't want to define it via cloneAndReplace
    # FIXME: this looks more like a hack than good design, maybe dropping Tau.ActiveConfig and using Tau.TauRec as active config would be better?
    tau_cfg.addFlagsCategory("Tau.ActiveConfig", createTauRecConfigFlags, prefix=True)

    # e-had boosted ditaus, aka electron-subtracted taus
    tau_cfg.addFlag("Tau.doTauEleRMRec", True)
    # had-had boosted ditaus
    tau_cfg.addFlag("Tau.doDiTauRec", True)

    return tau_cfg


def createTauRecConfigFlags():
    from AthenaConfiguration.AthConfigFlags import AthConfigFlags
    flags = AthConfigFlags()
    flags.addFlag("prefix", "")

    # Output containers
    flags.addFlag("TauJets",                "TauJets")
    flags.addFlag("TauTracks",              "TauTracks")
    flags.addFlag("TauShotClusters",        "TauShotClusters")
    flags.addFlag("TauShotClustersLinks",   "TauShotClusters_links")
    flags.addFlag("TauShotPFOs",            "TauShotParticleFlowObjects")
    flags.addFlag("TauPi0Clusters",         "TauPi0Clusters")
    flags.addFlag("TauPi0ClustersLinks",    "TauPi0Clusters_links")
    flags.addFlag("TauHadronicPFOs",        "TauHadronicParticleFlowObjects")
    flags.addFlag("TauNeutralPFOs",         "TauNeutralParticleFlowObjects")
    flags.addFlag("TauChargedPFOs",         "TauChargedParticleFlowObjects")
    flags.addFlag("TauSecondaryVertices",   "TauSecondaryVertices")
    flags.addFlag("TauFinalPi0s",           "TauFinalPi0s")

    # Transient containers
    flags.addFlag("TauJets_tmp", "TauJets_tmp")
    flags.addFlag("TauCommonPi0Cells", "TauCommonPi0Cells")
    flags.addFlag("TauPi0Clusters_tmp", "TauPi0Clusters_tmp")

    # Input containers
    flags.addFlag("VertexCollection", "PrimaryVertices")
    flags.addFlag("TrackCollection", "InDetTrackParticles")
    flags.addFlag("SeedJetCollection", "AntiKt4LCTopoJets")
    flags.addFlag("LargeD0TrackCollection", "InDetLargeD0TrackParticles")
    flags.addFlag("EventShapeCollection", "Kt4LCTopoOriginEventShape")

    # Electron-subtracted tau flags appearing in standard tau reconstruction
    flags.addFlag("inTauEleRM", False)
    flags.addFlag("RemoveElectronCells",        False)
    flags.addFlag("RemovedElectronClusters",    "")

    return flags


def createTauEleRMConfigFlags():
    flags = createTauRecConfigFlags()
    flags.prefix                     = "EleRM_"
    _output_suffix                   = "_EleRM"

    # Output containers
    flags.TauJets                    = f"TauJets{_output_suffix}"
    flags.TauTracks                  = f"TauTracks{_output_suffix}"
    flags.TauShotClusters            = f"TauShotClusters{_output_suffix}"
    flags.TauShotClustersLinks       = f"TauShotClusters{_output_suffix}_links"
    flags.TauShotPFOs                = f"TauShotParticleFlowObjects{_output_suffix}"
    flags.TauPi0Clusters             = f"TauPi0Clusters{_output_suffix}"
    flags.TauPi0ClustersLinks        = f"TauPi0Clusters{_output_suffix}_links"
    flags.TauHadronicPFOs            = f"TauHadronicParticleFlowObjects{_output_suffix}"
    flags.TauNeutralPFOs             = f"TauNeutralParticleFlowObjects{_output_suffix}"
    flags.TauChargedPFOs             = f"TauChargedParticleFlowObjects{_output_suffix}"
    flags.TauSecondaryVertices       = f"TauSecondaryVertices{_output_suffix}"
    flags.TauFinalPi0s               = f"TauFinalPi0s{_output_suffix}"

    # Transient containers
    flags.TauJets_tmp                = "TauJets_tmp_EleRM"
    flags.TauCommonPi0Cells          = "TauCommonPi0Cells_EleRM"
    flags.TauPi0Clusters_tmp         = "TauPi0Clusters_tmp_EleRM"
    
    # Input containers
    flags.TrackCollection            = "InDetTrackParticles_EleRM"
    flags.SeedJetCollection          = "AntiKt4LCTopoJets_EleRM"
    flags.EventShapeCollection       = "EleRM_Kt4LCTopoOriginEventShape"

    # Electron-subtracted tau flags appearing in standard tau reconstruction
    flags.inTauEleRM                 = True
    flags.RemoveElectronCells        = True
    flags.RemovedElectronClusters    = "RemovedClusters_EleRM"

    # Electron-subtracted tau specific flags
    flags.addFlag("EleRM_ElectronWorkingPoint", "Medium")
    flags.addFlag("RemovedElectronTracks",      "RemovedTracks_EleRM")
    flags.addFlag("CaloCalTopoClusters_EleRM",  "CaloCalTopoClusters_EleRM")
    flags.addFlag("LCOriginTopoClusters_EleRM", "LCOriginTopoClusters_EleRM")
    flags.addFlag("LCTopoOrigin_EleRM",         "LCTopoOrigin_EleRM")
    flags.addFlag("EleRM_CheckingConeSize",     0.6)

    return flags


# Self test

class TestTauRecConfigFlags(unittest.TestCase):
    def runTest(self):
        createTauConfigFlags()


if __name__ == "__main__":
    unittest.main()
