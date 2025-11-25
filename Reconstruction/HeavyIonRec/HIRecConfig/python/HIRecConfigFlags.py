# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.Enums import HIMode

def createHIRecConfigFlags():
  flags=AthConfigFlags()
  
  flags.addFlag("HeavyIon.doGlobal", True)
  flags.addFlag("HeavyIon.Global.doEventShapeSummary", True)
  flags.addFlag("HeavyIon.Global.EventShape", "HIEventShape")

  flags.addFlag("HeavyIon.doJet", True)
  flags.addFlag("HeavyIon.Jet.doTrackJetSeed", True)
  flags.addFlag("HeavyIon.Jet.ApplyTowerEtaPhiCorrection", lambda prevFlags: prevFlags.Reco.HIMode is HIMode.HI)
  flags.addFlag("HeavyIon.Jet.HarmonicsForSubtraction", lambda prevFlags: [2, 3, 4] if prevFlags.Reco.HIMode is HIMode.HI else [])
  flags.addFlag("HeavyIon.Jet.SeedPtMin", lambda prevFlags: 25000 if prevFlags.Reco.HIMode is HIMode.HI else 8000)
  flags.addFlag("HeavyIon.Jet.RecoOutputPtMin", lambda prevFlags: 25000 if prevFlags.Reco.HIMode is HIMode.HI else 8000)
  flags.addFlag("HeavyIon.Jet.TrackJetPtMin", lambda prevFlags: 7000 if prevFlags.Reco.HIMode is HIMode.HI else 4000)
  flags.addFlag("HeavyIon.Jet.HIClusterGeoWeightFile", "auto")
  flags.addFlag("HeavyIon.Jet.ClusterKey", lambda prevFlags: "DFHIClusters" if prevFlags.HeavyIon.isDerivation else "HIClusters")
  flags.addFlag("HeavyIon.Jet.Internal.ClusterKey", lambda prevFlags: "DFHIClusters_temp" if prevFlags.HeavyIon.isDerivation else "HIClusters_temp")
  flags.addFlag("HeavyIon.Jet.WriteHIClusters", lambda prevFlags: prevFlags.Reco.HIMode is not HIMode.UPC)
  flags.addFlag("HeavyIon.Jet.RValues", [2,4])#this are the R's we want to reconstruct
  flags.addFlag("HeavyIon.Jet.CaliRValues", ["2","3","4","10"])#this are the R's that are supported for calibration, if not listed then cali R=0.4 is picked

  flags.addFlag("HeavyIon.Egamma.doSubtractedClusters", lambda prevFlags: prevFlags.Reco.HIMode is HIMode.HI)
  flags.addFlag("HeavyIon.Egamma.EventShape", "HIEventShape_iter_egamma")
  flags.addFlag("HeavyIon.Egamma.SubtractedCells", "SubtractedCells")
  flags.addFlag("HeavyIon.Egamma.UncalibCaloTopoCluster", "SubtractedCaloTopoClusters")
  flags.addFlag("HeavyIon.Egamma.EgammaTopoCluster", "SubtractedEgammaTopoClusters")
  flags.addFlag("HeavyIon.Egamma.CaloTopoCluster", lambda prevFlags: "SubtractedCaloCalTopoClusters" if prevFlags.Calo.TopoCluster.doTopoClusterLocalCalib else prevFlags.HeavyIon.Egamma.UncalibCaloTopoCluster)

  flags.addFlag("HeavyIon.redoTracking", True)
  flags.addFlag("HeavyIon.redoEgamma", True)
  # derivation flags
  flags.addFlag("HeavyIon.isDerivation", False)
  flags.addFlag("HeavyIon.HIJetPrefix", lambda prevFlags: "DF" if prevFlags.HeavyIon.isDerivation else "")
  flags.addFlag("HeavyIon.doHIBTagging", True) # to get flavour tagging running on DFAntiKt4HIJets collection in derivations
  flags.addFlag("HeavyIon.FTagModifiers", lambda prevFlags: ["QGTagging", "NNJVT"] if prevFlags.HeavyIon.doHIBTagging else [] )
  flags.addFlag("HeavyIon.FTagTruthModifiers", lambda prevFlags: ["JetGhostLabel", "JetGhostInitialLabel", "PartonTruthLabel", "JetDeltaRInitialLabel:5000", "JetDeltaRLabel:5000"] if prevFlags.HeavyIon.doHIBTagging and prevFlags.Input.isMC else [] ) 
  flags.addFlag("HeavyIon.MinTrackPt", 0.9) # minimal reco track pT (in GeV) to be stored in derivation

  # expand as needed
  return flags
