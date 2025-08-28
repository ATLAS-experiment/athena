# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# configuration flags for the LLP derivations

from AthenaConfiguration.AthConfigFlags import AthConfigFlags


def createLLPDFConfigFlags():
    llpdcf = AthConfigFlags()
    llpdcf.addFlag("Derivation.LLP.saveFullTruth", False)
    llpdcf.addFlag("Derivation.LLP.doTrackSystematics", False)
    llpdcf.addFlag("Derivation.LLP.doMuSAValidation", False)
    return llpdcf

