# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# configuration flags for the egamma derivations

from AthenaConfiguration.AthConfigFlags import AthConfigFlags


def createEGammaDFConfigFlags():
    egdcf = AthConfigFlags()
    egdcf.addFlag("Derivation.Egamma.doTrackThinning", True)
    egdcf.addFlag("Derivation.Egamma.doEventInfoSlimming", False)
    egdcf.addFlag("Derivation.Egamma.addTriggerMatching", False)
    egdcf.addFlag("Derivation.Egamma.addMissingCellInfo", True)
    egdcf.addFlag("Derivation.Egamma.addECIDS", True)
    # temporary - for fake photon bkg rejection studies
    egdcf.addFlag("Derivation.Egamma.addHLTJets", False)
    return egdcf
