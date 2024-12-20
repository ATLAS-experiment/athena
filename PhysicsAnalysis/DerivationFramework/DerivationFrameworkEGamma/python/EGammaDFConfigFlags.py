# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# configuration flags for the egamma derivations

from AthenaConfiguration.AthConfigFlags import AthConfigFlags


def createEGammaDFConfigFlags():
    egdcf = AthConfigFlags()
    # thin tracks
    egdcf.addFlag("Derivation.Egamma.doTrackThinning", True)
    # slim event info or save all variables
    egdcf.addFlag("Derivation.Egamma.doEventInfoSlimming", False)
    # run EgammaClussterCoreCellRecovery tool (cells removed by timing cut) or  not
    egdcf.addFlag("Derivation.Egamma.addMissingCellInfo", True)
    # run ElectronChargeIDSelector tool or not
    egdcf.addFlag("Derivation.Egamma.addECIDS", True)
    # add improved prompt lepton tagger inputs/output
    egdcf.addFlag("Derivation.Egamma.addPLITInputs", True)
    egdcf.addFlag("Derivation.Egamma.addPLITOutputs", True)
    # add HLT jet container (temporary for fake photon bkg rejection studies)
    egdcf.addFlag("Derivation.Egamma.addHLTJets", False)
    return egdcf
