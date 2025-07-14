# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaCommon.SystemOfUnits import GeV

def createTrigJetConfigFlags():
    flags = AthConfigFlags()

    # PFO-muon removal option for the full-scan hadronic signatures.
    # Options are:
    #   "None": Do no PFO-muon removal
    #   "Calo": Use the calo-tagging tools from the muon slice
    #   "Iso" : Use the mainly isolation-based selections based on the MET associator package
    flags.addFlag("Trigger.FSHad.PFOMuonRemoval", "Calo",
                  help='PFO-muon removal option: None, Calo, Iso)')

    flags.addFlag("Trigger.FSHad.PFOMuonRemovalMinPt", 10 * GeV,
                  help='minimum pT threshold to use for the muon removal')

    flags.addFlag("Trigger.FSTrk.doJetRestrictedVertexSort", False,
                  help='use tracks in jets for computing sumpt2 for vertex sorting')

    flags.addFlag('Trigger.Jet.doJetSuperPrecisionTracking', False,
                  help='enable precision tracking in jet super-ROI before fast b-tagging (EMTopo jets)')

    flags.addFlag("Trigger.Jet.fastbtagPFlow", True,
                  help='enable fast b-tagging for all fully calibrated HLT PFlow jets')

    flags.addFlag("Trigger.Jet.fastbtagVertex", True,
                  help='enable the addition of the super ROI PV to the b-tagging')

    flags.addFlag("Trigger.Jet.doVRJets", False,
                  help='enable the addition of the VR track jet reconstruction sequence')

    # chooses calibration config file for HLT small-R jets
    # mapping in: Reconstruction/Jet/JetCalibTools/python/JetCalibToolsConfig.py
    # All calib keys for HLT jets have to start with "Trig" otherwise the JetCalibTool config fails!
    flags.addFlag("Trigger.Jet.pflowCalibKey", lambda prevFlags: "TrigHIUPC" if 'HI' in prevFlags.Trigger.triggerMenuSetup else "TrigR22Prerec",
                  help='calibration config file for HLT small-R jets')

    flags.addFlag("Trigger.Jet.emtopoCalibKey", "TrigLS2",
                  help='calibration config file for HLT small-R jets')

    flags.addFlag("Trigger.Jet.pflowLJCalibKey", "TrigSoftDrop",
                  help='calibration config file for HLT large-R PFlow jets')

    # chooses config directory for jet vertex tagger with neural network
    flags.addFlag("Trigger.Jet.nnJVTConfigDir", lambda prevFlags: "JetPileupTag/NNJvt/HLT-2025-05-21",
                  help='config directory containing cut files and neural network for HLT nnjvt')

    flags.addFlag("Trigger.Jet.PFlowTolerance", 1e-2,
                  help='tolerance in STEP Propagator')
    
    flags.addFlag("Trigger.Jet.TrackVtxAssocWP", "Custom", # offline default is "Nonprompt_All_MaxWeight"
                  help='working point for the TVA algorithm')

    flags.addFlag("Trigger.Jet.LowPtFilter", lambda prevFlags: 'HI' in prevFlags.Trigger.triggerMenuSetup,
                  help='apply low pT filter on antiKt4 jets (used for HI UPC jet reco)')

    return flags

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN2
    flags.lock()
    flags.loadAllDynamicFlags()
    flags.dump("Trigger.(Jet|FSTrk|FSHad)",evaluate=True)
